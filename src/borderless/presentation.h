#pragma once
#include <windows.h>
#include <d3d9.h>

// Only presentation parameters are changed. Engine render targets are untouched.
inline bool MakeWindowed(D3DPRESENT_PARAMETERS& p, HWND window, UINT width, UINT height) {
    if (!window || !width || !height) return false;
    p.Windowed = TRUE;
    p.hDeviceWindow = window;
    p.BackBufferWidth = width;
    p.BackBufferHeight = height;
    p.BackBufferFormat = D3DFMT_UNKNOWN;
    p.FullScreen_RefreshRateInHz = 0;
    return true;
}

using ResetFunction = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
inline HRESULT ForwardReset(ResetFunction original, IDirect3DDevice9* device,
    D3DPRESENT_PARAMETERS& caller, D3DPRESENT_PARAMETERS& adjusted) {
    HRESULT result = original(device, &adjusted);
    if (SUCCEEDED(result)) caller = adjusted;
    return result;
}
