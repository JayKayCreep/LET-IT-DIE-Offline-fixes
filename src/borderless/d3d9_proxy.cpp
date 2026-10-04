#define WIN32_LEAN_AND_MEAN
#include "presentation.h"
#include <new>

// LET IT DIE borderless-only wrapper. No polling, logging or game-address patches.
namespace {
HMODULE realModule = nullptr;
INIT_ONCE realOnce = INIT_ONCE_STATIC_INIT;

BOOL CALLBACK LoadRuntime(PINIT_ONCE, PVOID, PVOID*) {
    wchar_t path[MAX_PATH]{};
    UINT size = GetSystemDirectoryW(path, MAX_PATH);
    if (!size || size >= MAX_PATH - 12) return FALSE;
    wcscat_s(path, L"\\d3d9.dll");
    realModule = LoadLibraryW(path);
    return realModule != nullptr;
}
template<class T> T Real(const char* name) {
    if (!InitOnceExecuteOnce(&realOnce, LoadRuntime, nullptr, nullptr)) return nullptr;
    return reinterpret_cast<T>(GetProcAddress(realModule, name));
}

bool Bounds(HWND window, RECT& bounds) {
    DWORD process = 0;
    if (!IsWindow(window) || !GetWindowThreadProcessId(window, &process) || process != GetCurrentProcessId()) return false;
    MONITORINFO info{sizeof(info)};
    if (!GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &info)) return false;
    bounds = info.rcMonitor;
    return bounds.right > bounds.left && bounds.bottom > bounds.top;
}

bool Prepare(D3DPRESENT_PARAMETERS& p, HWND fallback) {
    HWND window = p.hDeviceWindow ? p.hDeviceWindow : fallback;
    RECT bounds{};
    return Bounds(window, bounds) && MakeWindowed(p, window,
        static_cast<UINT>(bounds.right - bounds.left), static_cast<UINT>(bounds.bottom - bounds.top));
}

// Never activate, foreground, unminimize, or make the game topmost.
// Only write window state when a correction is actually needed.
void CorrectWindow(HWND window) {
    RECT desired{}, actual{};
    if (!Bounds(window, desired) || IsIconic(window) || !GetWindowRect(window, &actual)) return;
    LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
    LONG_PTR extended = GetWindowLongPtrW(window, GWL_EXSTYLE);
    LONG_PTR newStyle = (style & ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) | WS_POPUP;
    LONG_PTR newExtended = extended & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE);
    const bool styleChange = style != newStyle || extended != newExtended;
    const bool sizeChange = !EqualRect(&desired, &actual);
    if (!styleChange && !sizeChange) return;
    if (style != newStyle) {
        SetLastError(0);
        if (!SetWindowLongPtrW(window, GWL_STYLE, newStyle) && GetLastError()) return;
    }
    if (extended != newExtended) {
        SetLastError(0);
        if (!SetWindowLongPtrW(window, GWL_EXSTYLE, newExtended) && GetLastError()) return;
    }
    UINT flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_ASYNCWINDOWPOS;
    if (styleChange) flags |= SWP_FRAMECHANGED;
    SetWindowPos(window, nullptr, desired.left, desired.top,
        desired.right-desired.left, desired.bottom-desired.top, flags);
}

using ResetFn = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
struct ResetEntry { void** table; ResetFn original; ResetEntry* next; };
ResetEntry* resetEntries = nullptr;
SRWLOCK resetLock = SRWLOCK_INIT;

HRESULT STDMETHODCALLTYPE BorderlessReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* parameters) {
    if (!device || !parameters) return D3DERR_INVALIDCALL;
    void** table = *reinterpret_cast<void***>(device);
    ResetFn original = nullptr;
    AcquireSRWLockShared(&resetLock);
    for (auto entry = resetEntries; entry; entry = entry->next) if (entry->table == table) { original = entry->original; break; }
    ReleaseSRWLockShared(&resetLock);
    if (!original) return D3DERR_INVALIDCALL;
    D3DPRESENT_PARAMETERS adjusted = *parameters;
    D3DDEVICE_CREATION_PARAMETERS creation{};
    if (!adjusted.hDeviceWindow && FAILED(device->GetCreationParameters(&creation))) return D3DERR_INVALIDCALL;
    if (!Prepare(adjusted, creation.hFocusWindow)) return D3DERR_INVALIDCALL;
    HWND window = adjusted.hDeviceWindow;
    HRESULT result = ForwardReset(original, device, *parameters, adjusted);
    if (SUCCEEDED(result)) CorrectWindow(window);
    return result;
}

bool HookReset(IDirect3DDevice9* device) {
    // Slot 16 is IDirect3DDevice9::Reset in the public COM ABI, not a game offset.
    // Keep the callback valid until process exit, without a background thread.
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&BorderlessReset), &self)) return false;
    void** table = *reinterpret_cast<void***>(device);
    AcquireSRWLockExclusive(&resetLock);
    for (auto entry = resetEntries; entry; entry = entry->next) {
        if (entry->table == table) {
            bool intact = table[16] == reinterpret_cast<void*>(&BorderlessReset);
            ReleaseSRWLockExclusive(&resetLock); return intact;
        }
    }
    auto entry = new (std::nothrow) ResetEntry{table, reinterpret_cast<ResetFn>(table[16]), resetEntries};
    if (!entry) { ReleaseSRWLockExclusive(&resetLock); return false; }
    DWORD oldProtection = 0;
    if (!VirtualProtect(&table[16], sizeof(void*), PAGE_READWRITE, &oldProtection)) {
        delete entry; ReleaseSRWLockExclusive(&resetLock); return false;
    }
    resetEntries = entry;
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&table[16]), reinterpret_cast<void*>(&BorderlessReset));
    DWORD ignored = 0;
    VirtualProtect(&table[16], sizeof(void*), oldProtection, &ignored);
    ReleaseSRWLockExclusive(&resetLock);
    return true;
}

class Direct3D9Proxy final : public IDirect3D9 {
    IDirect3D9* real_;
public:
    explicit Direct3D9Proxy(IDirect3D9* real) : real_(real) {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** out) override {
        if (!out) return E_POINTER;
        if (id == __uuidof(IUnknown) || id == __uuidof(IDirect3D9)) { *out=this; AddRef(); return S_OK; }
        return real_->QueryInterface(id,out);
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return real_->AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { ULONG refs=real_->Release(); if (!refs) delete this; return refs; }
#include "d3d9_forwarders.inc"
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT adapter, D3DDEVTYPE type, HWND focus, DWORD flags,
        D3DPRESENT_PARAMETERS* parameters, IDirect3DDevice9** output) override {
        if (!parameters || !output) return D3DERR_INVALIDCALL;
        *output=nullptr;
        D3DPRESENT_PARAMETERS adjusted=*parameters;
        if (!Prepare(adjusted,focus)) return D3DERR_INVALIDCALL;
        HWND window=adjusted.hDeviceWindow;
        HRESULT result=real_->CreateDevice(adapter,type,focus,flags,&adjusted,output);
        if (SUCCEEDED(result)) {
            if (!HookReset(*output)) {
                (*output)->Release(); *output=nullptr; return E_FAIL;
            }
            *parameters=adjusted;
            CorrectWindow(window);
        }
        return result;
    }
};
}

IDirect3D9* WINAPI Direct3DCreate9(UINT sdk) {
    auto function=Real<IDirect3D9* (WINAPI*)(UINT)>("Direct3DCreate9");
    if (!function) return nullptr;
    IDirect3D9* real=function(sdk);
    if (!real) return nullptr;
    auto proxy=new (std::nothrow) Direct3D9Proxy(real);
    if (!proxy) real->Release();
    return proxy;
}
int WINAPI D3DPERF_BeginEvent(D3DCOLOR color,LPCWSTR name) {
    auto f=Real<int (WINAPI*)(D3DCOLOR,LPCWSTR)>("D3DPERF_BeginEvent"); return f ? f(color,name) : -1;
}
int WINAPI D3DPERF_EndEvent() {
    auto f=Real<int (WINAPI*)()>("D3DPERF_EndEvent"); return f ? f() : -1;
}
void WINAPI D3DPERF_SetOptions(DWORD options) {
    auto f=Real<void (WINAPI*)(DWORD)>("D3DPERF_SetOptions"); if (f) f(options);
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if (reason==DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(instance);
    return TRUE;
}
