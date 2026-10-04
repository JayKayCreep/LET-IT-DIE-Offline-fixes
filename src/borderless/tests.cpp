#include "d3d9_proxy.cpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

void Check(bool pass, const char* message) { if (!pass) { fprintf(stderr,"FAIL: %s\n",message); exit(1); } }
HRESULT STDMETHODCALLTYPE Failure(IDirect3DDevice9*,D3DPRESENT_PARAMETERS* p) { p->BackBufferWidth=99; return D3DERR_DEVICELOST; }
HRESULT STDMETHODCALLTYPE Success(IDirect3DDevice9*,D3DPRESENT_PARAMETERS* p) { p->BackBufferCount=2; return S_OK; }
int main() {
    const HWND hwnd=reinterpret_cast<HWND>(static_cast<uintptr_t>(0x1234));
    for (BOOL windowed : {FALSE,TRUE}) {
        D3DPRESENT_PARAMETERS p{};
        p.Windowed=windowed; p.BackBufferWidth=1280; p.BackBufferHeight=720;
        p.BackBufferFormat=D3DFMT_X8R8G8B8; p.FullScreen_RefreshRateInHz=144;
        p.BackBufferCount=1; p.MultiSampleType=D3DMULTISAMPLE_4_SAMPLES; p.MultiSampleQuality=2;
        p.SwapEffect=D3DSWAPEFFECT_DISCARD; p.EnableAutoDepthStencil=TRUE;
        p.AutoDepthStencilFormat=D3DFMT_D24S8; p.PresentationInterval=D3DPRESENT_INTERVAL_ONE; p.Flags=D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
        auto original=p;
        Check(MakeWindowed(p,hwnd,2560,1440),"valid transform");
        Check(p.Windowed && p.hDeviceWindow==hwnd && p.BackBufferWidth==2560 && p.BackBufferHeight==1440,"mode and dimensions");
        Check(p.BackBufferFormat==D3DFMT_UNKNOWN && p.FullScreen_RefreshRateInHz==0,"windowed format/refresh");
        Check(p.MultiSampleType==original.MultiSampleType && p.MultiSampleQuality==original.MultiSampleQuality &&
            p.SwapEffect==original.SwapEffect && p.EnableAutoDepthStencil==original.EnableAutoDepthStencil &&
            p.AutoDepthStencilFormat==original.AutoDepthStencilFormat && p.Flags==original.Flags &&
            p.PresentationInterval==original.PresentationInterval && p.BackBufferCount==original.BackBufferCount,"unrelated rendering fields preserved");
        auto adjusted=p;
        auto caller=original;
        Check(ForwardReset(Failure,nullptr,caller,adjusted)==D3DERR_DEVICELOST,"failure forwarded");
        Check(memcmp(&caller,&original,sizeof(caller))==0,"failed reset preserves caller parameters");
        adjusted=p;
        Check(ForwardReset(Success,nullptr,caller,adjusted)==S_OK && caller.Windowed && caller.BackBufferCount==2,"successful reset outputs forwarded");
    }
    D3DPRESENT_PARAMETERS invalid{},before{};
    Check(!MakeWindowed(invalid,nullptr,1920,1080) && !MakeWindowed(invalid,hwnd,0,1080),"invalid target rejected");
    Check(memcmp(&invalid,&before,sizeof(invalid))==0,"invalid transform leaves inputs unchanged");
    void* table1[119]{}; table1[16]=reinterpret_cast<void*>(&Failure);
    void* table2[119]{}; table2[16]=reinterpret_cast<void*>(&Success);
    struct Object { void** table; } first{table1}, shared{table1}, second{table2};
    Check(HookReset(reinterpret_cast<IDirect3DDevice9*>(&first)),"first reset hook");
    Check(HookReset(reinterpret_cast<IDirect3DDevice9*>(&shared)),"shared table idempotency");
    Check(HookReset(reinterpret_cast<IDirect3DDevice9*>(&second)),"separate table hook");
    Check(resetEntries && resetEntries->original==Success && resetEntries->next && resetEntries->next->original==Failure && !resetEntries->next->next,"per-table originals retained");
    table1[16]=reinterpret_cast<void*>(&Success);
    Check(!HookReset(reinterpret_cast<IDirect3DDevice9*>(&first)),"later hook conflict detected");
    HWND hidden=CreateWindowExW(WS_EX_CLIENTEDGE,L"STATIC",L"Borderless validation",
        WS_OVERLAPPEDWINDOW,30,40,320,240,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    Check(hidden!=nullptr,"hidden test window created");
    const HWND foreground=GetForegroundWindow();
    RECT bounds{},actual{};
    Check(Bounds(hidden,bounds),"owned window monitor bounds");
    CorrectWindow(hidden);
    Check(GetWindowRect(hidden,&actual) && EqualRect(&bounds,&actual),"hidden window fills selected monitor");
    Check(!IsWindowVisible(hidden),"correction does not show hidden window");
    Check(GetForegroundWindow()==foreground,"correction preserves foreground application");
    Check((GetWindowLongPtrW(hidden,GWL_STYLE)&(WS_CAPTION|WS_THICKFRAME))==0,"window frame removed");
    Check((GetWindowLongPtrW(hidden,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)==0,"extended frame removed");
    Check((GetWindowLongPtrW(hidden,GWL_EXSTYLE)&WS_EX_TOPMOST)==0,"correction does not make window topmost");
    CorrectWindow(hidden);
    Check(GetWindowRect(hidden,&actual) && EqualRect(&bounds,&actual),"repeat correction retains bounds");
    table2[16]=reinterpret_cast<void*>(&BorderlessReset);
    D3DPRESENT_PARAMETERS reset{};reset.hDeviceWindow=hidden;reset.Windowed=FALSE;
    Check(BorderlessReset(reinterpret_cast<IDirect3DDevice9*>(&second),&reset)==S_OK && reset.Windowed,
        "reset callback uses actual owned window and preserves success");
    DestroyWindow(hidden);
    puts("PASS: display conversion, field preservation, reset success/failure, per-table callbacks, hook conflict detection, hidden-window borderless sizing and foreground preservation.");
}
