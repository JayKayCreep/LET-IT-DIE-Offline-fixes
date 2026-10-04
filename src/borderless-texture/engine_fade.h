#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <memory>
constexpr wchar_t ExpectedHash[]=L"716046F09F398A61B359CC1DD08366F3FD1D867605949D6729E25C3A07A4DBAF";
constexpr uintptr_t MipFadeRva=0x27830CC;
struct HandleCloser { void operator()(void* p) const { if (p && p!=INVALID_HANDLE_VALUE) CloseHandle(p); } };
using Handle=std::unique_ptr<void,HandleCloser>;
std::wstring LastErrorText() { return L"Windows error "+std::to_wstring(GetLastError()); }
template<class T> bool Read(HANDLE process,uintptr_t address,T& value) {
    SIZE_T received=0;
    return ReadProcessMemory(process,reinterpret_cast<const void*>(address),&value,sizeof(value),&received) && received==sizeof(value);
}

struct Hasher {
    BCRYPT_ALG_HANDLE algorithm=nullptr;
    BCRYPT_HASH_HANDLE hash=nullptr;
    std::vector<unsigned char> object;
    ~Hasher() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm,0); }
    bool Start() {
        if (BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0) return false;
        DWORD length=0,received=0;
        if (BCryptGetProperty(algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&length),sizeof(length),&received,0)<0) return false;
        object.resize(length);
        return BCryptCreateHash(algorithm,&hash,object.data(),length,nullptr,0,0)>=0;
    }
    bool Add(unsigned char* bytes,ULONG count) { return BCryptHashData(hash,bytes,count,0)>=0; }
    std::wstring Finish() {
        unsigned char digest[32]{};
        if (BCryptFinishHash(hash,digest,sizeof(digest),0)<0) return {};
        std::wstring result;
        for (unsigned char byte:digest) { wchar_t pair[3]{}; swprintf_s(pair,L"%02X",byte); result+=pair; }
        return result;
    }
};
std::wstring FileHash(const wchar_t* path) {
    Handle file(CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if (file.get()==INVALID_HANDLE_VALUE) return {};
    Hasher hasher;
    if (!hasher.Start()) return {};
    unsigned char buffer[65536];
    DWORD count=0;
    for (;;) {
        if (!ReadFile(file.get(),buffer,sizeof(buffer),&count,nullptr)) return {};
        if (!count) break;
        if (!hasher.Add(buffer,count)) return {};
    }
    return hasher.Finish();
}

bool CompatibleMipFade(HANDLE process,uintptr_t base) {
    IMAGE_DOS_HEADER dos{};
    if (!Read(process,base,dos) || dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<=0 || dos.e_lfanew>1048576) return false;
    IMAGE_NT_HEADERS64 nt{};
    if (!Read(process,base+static_cast<uintptr_t>(dos.e_lfanew),nt) || nt.Signature!=IMAGE_NT_SIGNATURE ||
        nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 || nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC || nt.OptionalHeader.SizeOfImage!=0x1004A000) return false;
    struct Signature { uintptr_t rva; std::array<unsigned char,8> bytes; };
    const Signature signatures[]={
        {0x686cc4,{0xf3,0x0f,0x11,0x05,0x00,0xc4,0x0f,0x02}},
        {0x8867c9,{0xf3,0x0f,0x10,0x05,0xfb,0xc8,0xef,0x01}}
    };
    for (const auto& signature:signatures) {
        std::array<unsigned char,8> actual{};
        if (!Read(process,base+signature.rva,actual) || actual!=signature.bytes) return false;
    }
    return true;
}

struct AutomaticMipFadeFix {
    volatile LONG* value=nullptr;
    static constexpr LONG EnabledBits=0x3F800000;
    static constexpr LONG DisabledBits=static_cast<LONG>(0xBF800000u);
    bool ConnectVerified(uintptr_t base) {
        if (!CompatibleMipFade(GetCurrentProcess(),base)) return false;
        auto candidate=reinterpret_cast<volatile LONG*>(base+MipFadeRva);
        MEMORY_BASIC_INFORMATION page{};LONG current=0;
        if (!VirtualQuery(const_cast<LONG*>(candidate),&page,sizeof(page)) || page.State!=MEM_COMMIT ||
            page.Protect!=PAGE_READWRITE || page.AllocationBase!=reinterpret_cast<void*>(base) ||
            !Read(GetCurrentProcess(),base+MipFadeRva,current) ||
            (current!=EnabledBits && current!=DisabledBits)) return false;
        value=candidate;return true;
    }
    bool Initialize() {
        wchar_t path[32768]{};HMODULE module=GetModuleHandleW(nullptr);
        DWORD count=GetModuleFileNameW(module,path,32768);
        return count && count<32768 && FileHash(path)==ExpectedHash &&
            ConnectVerified(reinterpret_cast<uintptr_t>(module));
    }
    bool Apply() const {
        if (!value) return false;
        // Accept only the native command's two values; leave any other writer alone.
        LONG previous=InterlockedCompareExchange(value,DisabledBits,EnabledBits);
        return previous==EnabledBits || previous==DisabledBits;
    }
};

