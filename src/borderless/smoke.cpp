#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <initializer_list>
int wmain(int count,wchar_t** args) {
    if (count!=2) return 1;
    HMODULE module=LoadLibraryW(args[1]);
    if (!module) { printf("FAIL: LoadLibrary %lu\n",GetLastError()); return 1; }
    for (const char* name : {"Direct3DCreate9","D3DPERF_BeginEvent","D3DPERF_EndEvent","D3DPERF_SetOptions"}) {
        if (!GetProcAddress(module,name)) { printf("FAIL: missing %s\n",name); return 1; }
    }
    auto create=reinterpret_cast<IDirect3D9* (WINAPI*)(UINT)>(GetProcAddress(module,"Direct3DCreate9"));
    IDirect3D9* d3d=create(D3D_SDK_VERSION);
    if (!d3d) { puts("FAIL: Direct3DCreate9 returned null"); return 1; }
    IUnknown* identity=nullptr;
    HRESULT hr=d3d->QueryInterface(__uuidof(IUnknown),reinterpret_cast<void**>(&identity));
    if (FAILED(hr) || identity!=static_cast<IUnknown*>(d3d)) { puts("FAIL: COM identity"); return 1; }
    identity->Release();
    printf("PASS: DLL loaded, four exports resolved, real D3D9 created, COM identity retained, adapters=%u. No window/device created.\n",d3d->GetAdapterCount());
    d3d->Release();
    FreeLibrary(module);
}
