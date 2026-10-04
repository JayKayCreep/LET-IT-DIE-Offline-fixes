#define WIN32_LEAN_AND_MEAN
#include "engine_fade.h"
#include <cstdlib>
void Check(bool pass,const char* text) { if (!pass) { fprintf(stderr,"FAIL: %s\n",text);exit(1); } }
int main() {
    Hasher hash;unsigned char abc[]={'a','b','c'};
    Check(hash.Start() && hash.Add(abc,3) && hash.Finish()==L"BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD","SHA256 known vector");
    AutomaticMipFadeFix rejected;
    Check(!rejected.Initialize() && !rejected.value,"self executable rejected without writing");
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x1004A000,MEM_RESERVE,PAGE_READWRITE));
    Check(memory!=nullptr,"synthetic image reservation");
    for (uintptr_t region : {uintptr_t(0),uintptr_t(0x686000),uintptr_t(0x886000),uintptr_t(0x2783000)})
        Check(VirtualAlloc(memory+region,4096,MEM_COMMIT,PAGE_READWRITE)!=nullptr,"synthetic image page committed");
    auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(memory);dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=128;
    auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(memory+128);
    nt->Signature=IMAGE_NT_SIGNATURE;nt->FileHeader.Machine=IMAGE_FILE_MACHINE_AMD64;
    nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR64_MAGIC;nt->OptionalHeader.SizeOfImage=0x1004A000;
    const unsigned char signature1[]={0xf3,0x0f,0x11,0x05,0x00,0xc4,0x0f,0x02};
    const unsigned char signature2[]={0xf3,0x0f,0x10,0x05,0xfb,0xc8,0xef,0x01};
    memcpy(memory+0x686cc4,signature1,sizeof(signature1));memcpy(memory+0x8867c9,signature2,sizeof(signature2));
    auto bits=reinterpret_cast<LONG*>(memory+MipFadeRva);*bits=AutomaticMipFadeFix::EnabledBits;
    const uintptr_t base=reinterpret_cast<uintptr_t>(memory);
    Check(CompatibleMipFade(GetCurrentProcess(),base),"synthetic signatures/header accepted");
    memory[0x686cc4]^=1;
    Check(!rejected.ConnectVerified(base) && *bits==AutomaticMipFadeFix::EnabledBits,"tampered signature rejected without write");
    memory[0x686cc4]^=1;*bits=0;
    Check(!rejected.ConnectVerified(base) && !*bits,"unexpected baseline rejected without write");
    *bits=AutomaticMipFadeFix::EnabledBits;DWORD protection=0,ignored=0;
    Check(VirtualProtect(bits,sizeof(LONG),PAGE_READONLY,&protection)!=0,"read-only control page");
    Check(!rejected.ConnectVerified(base),"read-only page rejected");
    VirtualProtect(bits,sizeof(LONG),protection,&ignored);
    AutomaticMipFadeFix control;
    Check(control.ConnectVerified(base),"verified synthetic writable control accepted");
    Check(control.Apply() && *bits==AutomaticMipFadeFix::DisabledBits,"automatic disable applies native -1");
    Check(control.Apply() && *bits==AutomaticMipFadeFix::DisabledBits,"automatic apply is idempotent");
    AutomaticMipFadeFix alreadyDisabled;
    Check(alreadyDisabled.ConnectVerified(base) && alreadyDisabled.Apply(),"native disabled baseline accepted");
    *bits=AutomaticMipFadeFix::EnabledBits;
    Check(control.Apply() && *bits==AutomaticMipFadeFix::DisabledBits,"lifecycle reinitialization can reapply known enabled value");
    *bits=0;
    Check(!control.Apply() && !*bits,"unexpected conflicting value left unchanged");
    AutomaticMipFadeFix unavailable;
    Check(!unavailable.Apply(),"rejected/unconnected control cannot write");
    VirtualFree(memory,0,MEM_RELEASE);
    puts("PASS: SHA256, wrong-image rejection, header/signature gate, writable-page and baseline checks, automatic disable, idempotency, lifecycle reapplication, conflicting writer protection. Synthetic memory only.");
}
