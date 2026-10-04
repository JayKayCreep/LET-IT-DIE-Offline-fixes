    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* p) override { return real_->RegisterSoftwareDevice(p); }
    UINT STDMETHODCALLTYPE GetAdapterCount() override { return real_->GetAdapterCount(); }
    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT a,DWORD f,D3DADAPTER_IDENTIFIER9* p) override { return real_->GetAdapterIdentifier(a,f,p); }
    UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT a,D3DFORMAT f) override { return real_->GetAdapterModeCount(a,f); }
    HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT a,D3DFORMAT f,UINT m,D3DDISPLAYMODE* p) override { return real_->EnumAdapterModes(a,f,m,p); }
    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT a,D3DDISPLAYMODE* p) override { return real_->GetAdapterDisplayMode(a,p); }
    HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT a,D3DDEVTYPE t,D3DFORMAT af,D3DFORMAT bf,BOOL w) override { return real_->CheckDeviceType(a,t,af,bf,w); }
    HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT a,D3DDEVTYPE t,D3DFORMAT af,DWORD u,D3DRESOURCETYPE r,D3DFORMAT cf) override { return real_->CheckDeviceFormat(a,t,af,u,r,cf); }
    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT a,D3DDEVTYPE t,D3DFORMAT f,BOOL w,D3DMULTISAMPLE_TYPE m,DWORD* q) override { return real_->CheckDeviceMultiSampleType(a,t,f,w,m,q); }
    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT a,D3DDEVTYPE t,D3DFORMAT af,D3DFORMAT rt,D3DFORMAT ds) override { return real_->CheckDepthStencilMatch(a,t,af,rt,ds); }
    HRESULT STDMETHODCALLTYPE CheckDeviceFormatConversion(UINT a,D3DDEVTYPE t,D3DFORMAT s,D3DFORMAT d) override { return real_->CheckDeviceFormatConversion(a,t,s,d); }
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT a,D3DDEVTYPE t,D3DCAPS9* p) override { return real_->GetDeviceCaps(a,t,p); }
    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT a) override { return real_->GetAdapterMonitor(a); }
