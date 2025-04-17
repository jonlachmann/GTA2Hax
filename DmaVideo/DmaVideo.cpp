#include "stdafx.h"
#include "DmaVideo.hpp"
#include "logger.hpp"
#include "..\d3ddll\d3ddll.hpp"
#include <DINPUT.H>
#include <ddraw.h>
#include <math.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "imgui.h"
#include "examples/directx9_example/imgui_impl_dx9.h"

struct IDirectDraw7Vtbl
{
    HRESULT(__stdcall* QueryInterface)(IDirectDraw7* This, const IID* const riid, LPVOID* ppvObj);
    ULONG(__stdcall* AddRef)(IDirectDraw7* This);
    ULONG(__stdcall* Release)(IDirectDraw7* This);
    HRESULT(__stdcall* Compact)(IDirectDraw7* This);
    HRESULT(__stdcall* CreateClipper)(IDirectDraw7* This, DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*);
    HRESULT(__stdcall* CreatePalette)(IDirectDraw7* This, DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE*, IUnknown*);
    HRESULT(__stdcall* CreateSurface)(IDirectDraw7* This, LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7*, IUnknown*);
    HRESULT(__stdcall* DuplicateSurface)(IDirectDraw7* This, LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7*);
    HRESULT(__stdcall* EnumDisplayModes)(IDirectDraw7* This, DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
    HRESULT(__stdcall* EnumSurfaces)(IDirectDraw7* This, DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7);
    HRESULT(__stdcall* FlipToGDISurface)(IDirectDraw7* This);
    HRESULT(__stdcall* GetCaps)(IDirectDraw7* This, LPDDCAPS, LPDDCAPS);
    HRESULT(__stdcall* GetDisplayMode)(IDirectDraw7* This, LPDDSURFACEDESC2);
    HRESULT(__stdcall* GetFourCCCodes)(IDirectDraw7* This, LPDWORD, LPDWORD);
    HRESULT(__stdcall* GetGDISurface)(IDirectDraw7* This, LPDIRECTDRAWSURFACE7*);
    HRESULT(__stdcall* GetMonitorFrequency)(IDirectDraw7* This, LPDWORD);
    HRESULT(__stdcall* GetScanLine)(IDirectDraw7* This, LPDWORD);
    HRESULT(__stdcall* GetVerticalBlankStatus)(IDirectDraw7* This, LPBOOL);
    HRESULT(__stdcall* Initialize)(IDirectDraw7* This, GUID*);
    HRESULT(__stdcall* RestoreDisplayMode)(IDirectDraw7* This);
    HRESULT(__stdcall* SetCooperativeLevel)(IDirectDraw7* This, HWND, DWORD);
    HRESULT(__stdcall* SetDisplayMode)(IDirectDraw7* This, DWORD, DWORD, DWORD, DWORD, DWORD);
    HRESULT(__stdcall* WaitForVerticalBlank)(IDirectDraw7* This, DWORD, HANDLE);
    HRESULT(__stdcall* GetAvailableVidMem)(IDirectDraw7* This, LPDDSCAPS2, LPDWORD, LPDWORD);
    HRESULT(__stdcall* GetSurfaceFromDC)(IDirectDraw7* This, HDC, LPDIRECTDRAWSURFACE7*);
    HRESULT(__stdcall* RestoreAllSurfaces)(IDirectDraw7* This);
    HRESULT(__stdcall* TestCooperativeLevel)(IDirectDraw7* This);
    HRESULT(__stdcall* GetDeviceIdentifier)(IDirectDraw7* This, LPDDDEVICEIDENTIFIER2, DWORD);
    HRESULT(__stdcall* StartModeTest)(IDirectDraw7* This, LPSIZE, DWORD, DWORD);
    HRESULT(__stdcall* EvaluateMode)(IDirectDraw7* This, DWORD, DWORD*);
};

struct IDirectDrawClipperVtbl
{
    HRESULT(__stdcall* QueryInterface)(IDirectDrawClipper* This, const IID* const riid, LPVOID* ppvObj);
    ULONG(__stdcall* AddRef)(IDirectDrawClipper* This);
    ULONG(__stdcall* Release)(IDirectDrawClipper* This);
    HRESULT(__stdcall* GetClipList)(IDirectDrawClipper* This, LPRECT, LPRGNDATA, LPDWORD);
    HRESULT(__stdcall* GetHWnd)(IDirectDrawClipper* This, HWND*);
    HRESULT(__stdcall* Initialize)(IDirectDrawClipper* This, LPDIRECTDRAW, DWORD);
    HRESULT(__stdcall* IsClipListChanged)(IDirectDrawClipper* This, BOOL*);
    HRESULT(__stdcall* SetClipList)(IDirectDrawClipper* This, LPRGNDATA, DWORD);
    HRESULT(__stdcall* SetHWnd)(IDirectDrawClipper* This, DWORD, HWND);
};

class StubDirectDrawClipper
{
  private:
    IDirectDrawClipperVtbl* lpVtbl; // Dynamically allocated in the constructor
    ULONG refCount; // Reference count for AddRef/Release

    // Static method implementations for the vtable functions
    static HRESULT __stdcall QueryInterface(IDirectDrawClipper* This, const IID* const riid, LPVOID* ppvObj)
    {
        printf("Stub: QueryInterface called\n");
        if (ppvObj == NULL)
            return E_POINTER;
        *ppvObj = This; // For simplicity, return the same object
        reinterpret_cast<StubDirectDrawClipper*>(This)->AddRef(This);
        return S_OK;
    }

    static ULONG __stdcall AddRef(IDirectDrawClipper* This)
    {
        StubDirectDrawClipper* stub = reinterpret_cast<StubDirectDrawClipper*>(This);
        printf("Stub: AddRef called\n");
        return ++stub->refCount;
    }

    static ULONG __stdcall Release(IDirectDrawClipper* This)
    {
        StubDirectDrawClipper* stub = reinterpret_cast<StubDirectDrawClipper*>(This);
        printf("Stub: Release called\n");
        if (--stub->refCount == 0)
        {
            delete stub;
            return 0;
        }
        return stub->refCount;
    }

    static HRESULT __stdcall GetClipList(IDirectDrawClipper* This, LPRECT lpRect, LPRGNDATA lpRgnData, LPDWORD lpdwSize)
    {
        printf("Stub: GetClipList called\n");
        return S_OK;
    }

    static HRESULT __stdcall GetHWnd(IDirectDrawClipper* This, HWND* lphWnd)
    {
        printf("Stub: GetHWnd called\n");
        return S_OK;
    }

    static HRESULT __stdcall Initialize(IDirectDrawClipper* This, LPDIRECTDRAW lpDD, DWORD dwFlags)
    {
        printf("Stub: Initialize called\n");
        return S_OK;
    }

    static HRESULT __stdcall IsClipListChanged(IDirectDrawClipper* This, BOOL* lpChanged)
    {
        printf("Stub: IsClipListChanged called\n");
        return S_OK;
    }

    static HRESULT __stdcall SetClipList(IDirectDrawClipper* This, LPRGNDATA lpClipList, DWORD dwFlags)
    {
        printf("Stub: SetClipList called\n");
        return S_OK;
    }

    static HRESULT __stdcall SetHWnd(IDirectDrawClipper* This, DWORD dwFlags, HWND hWnd)
    {
        printf("Stub: SetHWnd called\n");
        return S_OK;
    }

  public:
    // Constructor
    StubDirectDrawClipper() : refCount(1)
    {
        lpVtbl = new IDirectDrawClipperVtbl();
        lpVtbl->QueryInterface = &StubDirectDrawClipper::QueryInterface;
        lpVtbl->AddRef = &StubDirectDrawClipper::AddRef;
        lpVtbl->Release = &StubDirectDrawClipper::Release;
        lpVtbl->GetClipList = &StubDirectDrawClipper::GetClipList;
        lpVtbl->GetHWnd = &StubDirectDrawClipper::GetHWnd;
        lpVtbl->Initialize = &StubDirectDrawClipper::Initialize;
        lpVtbl->IsClipListChanged = &StubDirectDrawClipper::IsClipListChanged;
        lpVtbl->SetClipList = &StubDirectDrawClipper::SetClipList;
        lpVtbl->SetHWnd = &StubDirectDrawClipper::SetHWnd;
    }

    // Destructor
    ~StubDirectDrawClipper()
    {
        delete lpVtbl;
    }

    // Accessor for the IDirectDrawClipper interface
    IDirectDrawClipper* GetInterface()
    {
        return reinterpret_cast<IDirectDrawClipper*>(this);
    }
};

class IDirectDraw7Stub
{
  public:
    IDirectDraw7Vtbl* vtbl;

    IDirectDraw7Stub()
    {
        vtbl = new IDirectDraw7Vtbl(); // todo: don't leak
        //        ZeroMemory(vtbl, sizeof(*vtbl));
        memset(vtbl, 0xfe, sizeof(*vtbl));

        // Initialize vtable methods
        vtbl->QueryInterface = QueryInterfaceStub;
        vtbl->AddRef = AddRefStub;
        vtbl->Release = ReleaseStub;
        vtbl->Compact = CompactStub;
        vtbl->CreateClipper = CreateClipperStub;
        vtbl->CreatePalette = CreatePaletteStub;
        vtbl->CreateSurface = CreateSurfaceStub;
        vtbl->DuplicateSurface = DuplicateSurfaceStub;
        vtbl->EnumDisplayModes = EnumDisplayModesStub;
        vtbl->EnumSurfaces = EnumSurfacesStub;
        vtbl->FlipToGDISurface = FlipToGDISurfaceStub;
        vtbl->GetCaps = GetCapsStub;
        vtbl->GetDisplayMode = GetDisplayModeStub;
        vtbl->GetFourCCCodes = GetFourCCCodesStub;
        vtbl->GetGDISurface = GetGDISurfaceStub;
        vtbl->GetMonitorFrequency = GetMonitorFrequencyStub;
        vtbl->GetScanLine = GetScanLineStub;
        vtbl->GetVerticalBlankStatus = GetVerticalBlankStatusStub;
        vtbl->Initialize = InitializeStub;
        vtbl->RestoreDisplayMode = RestoreDisplayModeStub;
        vtbl->SetCooperativeLevel = SetCooperativeLevelStub;
        vtbl->SetDisplayMode = SetDisplayModeStub;
        vtbl->WaitForVerticalBlank = WaitForVerticalBlankStub;
        vtbl->GetAvailableVidMem = GetAvailableVidMemStub;
        vtbl->GetSurfaceFromDC = GetSurfaceFromDCStub;
        vtbl->RestoreAllSurfaces = RestoreAllSurfacesStub;
        vtbl->TestCooperativeLevel = TestCooperativeLevelStub;
        vtbl->GetDeviceIdentifier = GetDeviceIdentifierStub;
        vtbl->StartModeTest = StartModeTestStub;
        vtbl->EvaluateMode = EvaluateModeStub;
    }

    ~IDirectDraw7Stub()
    {
        delete vtbl;
    }

  private:
    // Stub method implementations
    static HRESULT __stdcall QueryInterfaceStub(IDirectDraw7* This, const IID* const riid, LPVOID* ppvObj)
    {
        printf("QueryInterface called\n");
        if (ppvObj)
            ZeroMemory(ppvObj, sizeof(*ppvObj));
        return DD_OK;
    }

    static ULONG __stdcall AddRefStub(IDirectDraw7* This)
    {
        printf("AddRef called\n");
        return 1;
    }

    static ULONG __stdcall ReleaseStub(IDirectDraw7* This)
    {
        printf("Release called\n");
        return 0;
    }

    static HRESULT __stdcall CompactStub(IDirectDraw7* This)
    {
        printf("Compact called\n");
        return DD_OK;
    }

    static HRESULT __stdcall CreateClipperStub(IDirectDraw7* This, DWORD flags, LPDIRECTDRAWCLIPPER* clipper, IUnknown* outerUnknown)
    {
        printf("CreateClipper called\n");
        if (clipper)
        {
            *clipper = (IDirectDrawClipper*)new StubDirectDrawClipper();
        }
        return DD_OK;
    }

    static HRESULT __stdcall CreatePaletteStub(IDirectDraw7* This,
                                               DWORD flags,
                                               LPPALETTEENTRY entries,
                                               LPDIRECTDRAWPALETTE* palette,
                                               IUnknown* outerUnknown)
    {
        printf("CreatePalette called\n");
        if (palette)
            ZeroMemory(palette, sizeof(*palette));
        return DD_OK;
    }

    static HRESULT __stdcall CreateSurfaceStub(IDirectDraw7* This,
                                               LPDDSURFACEDESC2 desc,
                                               LPDIRECTDRAWSURFACE7* surface,
                                               IUnknown* outerUnknown);

    static HRESULT __stdcall DuplicateSurfaceStub(IDirectDraw7* This, LPDIRECTDRAWSURFACE7 src, LPDIRECTDRAWSURFACE7* dest)
    {
        printf("DuplicateSurface called\n");
        if (dest)
            ZeroMemory(dest, sizeof(*dest));
        return DD_OK;
    }

    static HRESULT __stdcall EnumDisplayModesStub(IDirectDraw7* This,
                                                  DWORD flags,
                                                  LPDDSURFACEDESC2 desc,
                                                  LPVOID context,
                                                  LPDDENUMMODESCALLBACK2 callback)
    {
        printf("EnumDisplayModes called\n");
        return DD_OK;
    }

    static HRESULT __stdcall EnumSurfacesStub(IDirectDraw7* This,
                                              DWORD flags,
                                              LPDDSURFACEDESC2 desc,
                                              LPVOID context,
                                              LPDDENUMSURFACESCALLBACK7 callback)
    {
        printf("EnumSurfaces called\n");
        return DD_OK;
    }

    static HRESULT __stdcall FlipToGDISurfaceStub(IDirectDraw7* This)
    {
        printf("FlipToGDISurface called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetCapsStub(IDirectDraw7* This, LPDDCAPS caps, LPDDCAPS capsMore)
    {
        printf("GetCaps called\n");
        //if (caps) ZeroMemory(caps, sizeof(*caps));
        //if (capsMore) ZeroMemory(capsMore, sizeof(*capsMore));
        return DD_OK;
    }

    static HRESULT __stdcall GetDisplayModeStub(IDirectDraw7* This, LPDDSURFACEDESC2 desc)
    {
        printf("GetDisplayMode called\n");
        if (desc)
            ZeroMemory(desc, sizeof(*desc));
        return DD_OK;
    }

    static HRESULT __stdcall GetFourCCCodesStub(IDirectDraw7* This, LPDWORD codes, LPDWORD count)
    {
        printf("GetFourCCCodes called\n");
        if (codes)
            ZeroMemory(codes, sizeof(*codes));
        if (count)
            ZeroMemory(count, sizeof(*count));
        return DD_OK;
    }

    static HRESULT __stdcall GetGDISurfaceStub(IDirectDraw7* This, LPDIRECTDRAWSURFACE7* surface)
    {
        printf("GetGDISurface called\n");
        if (surface)
            ZeroMemory(surface, sizeof(*surface));
        return DD_OK;
    }

    static HRESULT __stdcall GetMonitorFrequencyStub(IDirectDraw7* This, LPDWORD frequency)
    {
        printf("GetMonitorFrequency called\n");
        if (frequency)
            ZeroMemory(frequency, sizeof(*frequency));
        return DD_OK;
    }

    static HRESULT __stdcall GetScanLineStub(IDirectDraw7* This, LPDWORD scanLine)
    {
        printf("GetScanLine called\n");
        if (scanLine)
            ZeroMemory(scanLine, sizeof(*scanLine));
        return DD_OK;
    }

    static HRESULT __stdcall GetVerticalBlankStatusStub(IDirectDraw7* This, LPBOOL status)
    {
        printf("GetVerticalBlankStatus called\n");
        if (status)
            ZeroMemory(status, sizeof(*status));
        return DD_OK;
    }

    static HRESULT __stdcall InitializeStub(IDirectDraw7* This, GUID* guid)
    {
        printf("Initialize called\n");
        return DD_OK;
    }

    static HRESULT __stdcall RestoreDisplayModeStub(IDirectDraw7* This)
    {
        printf("RestoreDisplayMode called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetCooperativeLevelStub(IDirectDraw7* This, HWND hwnd, DWORD level)
    {
        printf("SetCooperativeLevel called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetDisplayModeStub(IDirectDraw7* This, DWORD width, DWORD height, DWORD bpp, DWORD refreshRate, DWORD flags)
    {
        printf("SetDisplayMode called\n");
        return DD_OK;
    }

    static HRESULT __stdcall WaitForVerticalBlankStub(IDirectDraw7* This, DWORD flags, HANDLE handle)
    {
        printf("WaitForVerticalBlank called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetAvailableVidMemStub(IDirectDraw7* This, LPDDSCAPS2 caps, LPDWORD total, LPDWORD free)
    {
        printf("GetAvailableVidMem called\n");
        if (total)
            ZeroMemory(total, sizeof(*total));
        if (free)
            ZeroMemory(free, sizeof(*free));
        return DD_OK;
    }

    static HRESULT __stdcall GetSurfaceFromDCStub(IDirectDraw7* This, HDC hdc, LPDIRECTDRAWSURFACE7* surface)
    {
        printf("GetSurfaceFromDC called\n");
        if (surface)
            ZeroMemory(surface, sizeof(*surface));
        return DD_OK;
    }

    static HRESULT __stdcall RestoreAllSurfacesStub(IDirectDraw7* This)
    {
        printf("RestoreAllSurfaces called\n");
        return DD_OK;
    }

    static HRESULT __stdcall TestCooperativeLevelStub(IDirectDraw7* This)
    {
        printf("TestCooperativeLevel called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetDeviceIdentifierStub(IDirectDraw7* This, LPDDDEVICEIDENTIFIER2 identifier, DWORD flags)
    {
        printf("GetDeviceIdentifier called\n");
        if (identifier)
            ZeroMemory(identifier, sizeof(*identifier));
        return DD_OK;
    }

    static HRESULT __stdcall StartModeTestStub(IDirectDraw7* This, LPSIZE sizes, DWORD count, DWORD flags)
    {
        printf("StartModeTest called\n");
        return DD_OK;
    }

    static HRESULT __stdcall EvaluateModeStub(IDirectDraw7* This, DWORD mode, DWORD* flags)
    {
        printf("EvaluateMode called\n");
        if (flags)
            ZeroMemory(flags, sizeof(*flags));
        return DD_OK;
    }
};

static IDirectDraw7Stub gDirectDraw7Stub;

struct IDirectDrawSurface7Vtbl
{
    HRESULT(__stdcall* QueryInterface)(IDirectDrawSurface7* This, const IID* const riid, LPVOID* ppvObj);
    ULONG(__stdcall* AddRef)(IDirectDrawSurface7* This);
    ULONG(__stdcall* Release)(IDirectDrawSurface7* This);
    HRESULT(__stdcall* AddAttachedSurface)(IDirectDrawSurface7* This, LPDIRECTDRAWSURFACE7);
    HRESULT(__stdcall* AddOverlayDirtyRect)(IDirectDrawSurface7* This, LPRECT);
    HRESULT(__stdcall* Blt)(IDirectDrawSurface7* This, LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
    HRESULT(__stdcall* BltBatch)(IDirectDrawSurface7* This, LPDDBLTBATCH, DWORD, DWORD);
    HRESULT(__stdcall* BltFast)(IDirectDrawSurface7* This, DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
    HRESULT(__stdcall* DeleteAttachedSurface)(IDirectDrawSurface7* This, DWORD, LPDIRECTDRAWSURFACE7);
    HRESULT(__stdcall* EnumAttachedSurfaces)(IDirectDrawSurface7* This, LPVOID, LPDDENUMSURFACESCALLBACK7);
    HRESULT(__stdcall* EnumOverlayZOrders)(IDirectDrawSurface7* This, DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7);
    HRESULT(__stdcall* Flip)(IDirectDrawSurface7* This, LPDIRECTDRAWSURFACE7, DWORD);
    HRESULT(__stdcall* GetAttachedSurface)(IDirectDrawSurface7* This, LPDDSCAPS2, LPDIRECTDRAWSURFACE7*);
    HRESULT(__stdcall* GetBltStatus)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* GetCaps)(IDirectDrawSurface7* This, LPDDSCAPS2);
    HRESULT(__stdcall* GetClipper)(IDirectDrawSurface7* This, LPDIRECTDRAWCLIPPER*);
    HRESULT(__stdcall* GetColorKey)(IDirectDrawSurface7* This, DWORD, LPDDCOLORKEY);
    HRESULT(__stdcall* GetDC)(IDirectDrawSurface7* This, HDC*);
    HRESULT(__stdcall* GetFlipStatus)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* GetOverlayPosition)(IDirectDrawSurface7* This, LPLONG, LPLONG);
    HRESULT(__stdcall* GetPalette)(IDirectDrawSurface7* This, LPDIRECTDRAWPALETTE*);
    HRESULT(__stdcall* GetPixelFormat)(IDirectDrawSurface7* This, LPDDPIXELFORMAT);
    HRESULT(__stdcall* GetSurfaceDesc)(IDirectDrawSurface7* This, LPDDSURFACEDESC2);
    HRESULT(__stdcall* Initialize)(IDirectDrawSurface7* This, LPDIRECTDRAW, LPDDSURFACEDESC2);
    HRESULT(__stdcall* IsLost)(IDirectDrawSurface7* This);
    HRESULT(__stdcall* Lock)(IDirectDrawSurface7* This, LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
    HRESULT(__stdcall* ReleaseDC)(IDirectDrawSurface7* This, HDC);
    HRESULT(__stdcall* Restore)(IDirectDrawSurface7* This);
    HRESULT(__stdcall* SetClipper)(IDirectDrawSurface7* This, LPDIRECTDRAWCLIPPER);
    HRESULT(__stdcall* SetColorKey)(IDirectDrawSurface7* This, DWORD, LPDDCOLORKEY);
    HRESULT(__stdcall* SetOverlayPosition)(IDirectDrawSurface7* This, LONG, LONG);
    HRESULT(__stdcall* SetPalette)(IDirectDrawSurface7* This, LPDIRECTDRAWPALETTE);
    HRESULT(__stdcall* Unlock)(IDirectDrawSurface7* This, LPRECT);
    HRESULT(__stdcall* UpdateOverlay)(IDirectDrawSurface7* This, LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX);
    HRESULT(__stdcall* UpdateOverlayDisplay)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* UpdateOverlayZOrder)(IDirectDrawSurface7* This, DWORD, LPDIRECTDRAWSURFACE7);
    HRESULT(__stdcall* GetDDInterface)(IDirectDrawSurface7* This, LPVOID*);
    HRESULT(__stdcall* PageLock)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* PageUnlock)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* SetSurfaceDesc)(IDirectDrawSurface7* This, LPDDSURFACEDESC2, DWORD);
    HRESULT(__stdcall* SetPrivateData)(IDirectDrawSurface7* This, const GUID* const, LPVOID, DWORD, DWORD);
    HRESULT(__stdcall* GetPrivateData)(IDirectDrawSurface7* This, const GUID* const, LPVOID, LPDWORD);
    HRESULT(__stdcall* FreePrivateData)(IDirectDrawSurface7* This, const GUID* const);
    HRESULT(__stdcall* GetUniquenessValue)(IDirectDrawSurface7* This, LPDWORD);
    HRESULT(__stdcall* ChangeUniquenessValue)(IDirectDrawSurface7* This);
    HRESULT(__stdcall* SetPriority)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* GetPriority)(IDirectDrawSurface7* This, LPDWORD);
    HRESULT(__stdcall* SetLOD)(IDirectDrawSurface7* This, DWORD);
    HRESULT(__stdcall* GetLOD)(IDirectDrawSurface7* This, LPDWORD);
};

class IDirectDrawSurface7Stub
{
  public:
    IDirectDrawSurface7Vtbl* vtbl;
    std::vector<BYTE> mData;
    u32 mWidth;
    u32 mHeight;

    IDirectDrawSurface7Stub(s32 w, s32 h) {
        mWidth = w;
        mHeight = h;

        vtbl = new IDirectDrawSurface7Vtbl(); // Allocate memory for vtable
        InitializeVtbl(vtbl);                 // Call the common initializer
    }

    IDirectDrawSurface7Stub() {
        mWidth = 0;
        mHeight = 0;

        vtbl = new IDirectDrawSurface7Vtbl(); // Allocate memory for vtable
        InitializeVtbl(vtbl);                 // Call the common initializer
    }

    ~IDirectDrawSurface7Stub()
    {
        delete vtbl;
    }

  private:
    void InitializeVtbl(IDirectDrawSurface7Vtbl* vtbl) {
        if (!vtbl) return;

        memset(vtbl, 0xfe, sizeof(*vtbl)); // Initialize memory for the vtable with a dummy pattern

        vtbl->QueryInterface = QueryInterfaceStub;
        vtbl->AddRef = AddRefStub;
        vtbl->Release = ReleaseStub;
        vtbl->AddAttachedSurface = AddAttachedSurfaceStub;
        vtbl->AddOverlayDirtyRect = AddOverlayDirtyRectStub;
        vtbl->Blt = BltStub;
        vtbl->BltBatch = BltBatchStub;
        vtbl->BltFast = BltFastStub;
        vtbl->DeleteAttachedSurface = DeleteAttachedSurfaceStub;
        vtbl->EnumAttachedSurfaces = EnumAttachedSurfacesStub;
        vtbl->EnumOverlayZOrders = EnumOverlayZOrdersStub;
        vtbl->Flip = FlipStub;
        vtbl->GetAttachedSurface = GetAttachedSurfaceStub;
        vtbl->GetBltStatus = GetBltStatusStub;
        vtbl->GetCaps = GetCapsStub;
        vtbl->GetClipper = GetClipperStub;
        vtbl->GetColorKey = GetColorKeyStub;
        vtbl->GetDC = GetDCStub;
        vtbl->GetFlipStatus = GetFlipStatusStub;
        vtbl->GetOverlayPosition = GetOverlayPositionStub;
        vtbl->GetPalette = GetPaletteStub;
        vtbl->GetPixelFormat = GetPixelFormatStub;
        vtbl->GetSurfaceDesc = GetSurfaceDescStub;
        vtbl->Initialize = InitializeStub;
        vtbl->IsLost = IsLostStub;
        vtbl->Lock = LockStub;
        vtbl->ReleaseDC = ReleaseDCStub;
        vtbl->Restore = RestoreStub;
        vtbl->SetClipper = SetClipperStub;
        vtbl->SetColorKey = SetColorKeyStub;
        vtbl->SetOverlayPosition = SetOverlayPositionStub;
        vtbl->SetPalette = SetPaletteStub;
        vtbl->Unlock = UnlockStub;
        vtbl->UpdateOverlay = UpdateOverlayStub;
        vtbl->UpdateOverlayDisplay = UpdateOverlayDisplayStub;
        vtbl->UpdateOverlayZOrder = UpdateOverlayZOrderStub;
        vtbl->GetDDInterface = GetDDInterfaceStub;
        vtbl->PageLock = PageLockStub;
        vtbl->PageUnlock = PageUnlockStub;
        vtbl->SetSurfaceDesc = SetSurfaceDescStub;
        vtbl->SetPrivateData = SetPrivateDataStub;
        vtbl->GetPrivateData = GetPrivateDataStub;
        vtbl->FreePrivateData = FreePrivateDataStub;
        vtbl->GetUniquenessValue = GetUniquenessValueStub;
        vtbl->ChangeUniquenessValue = ChangeUniquenessValueStub;
        vtbl->SetPriority = SetPriorityStub;
        vtbl->GetPriority = GetPriorityStub;
        vtbl->SetLOD = SetLODStub;
        vtbl->GetLOD = GetLODStub;
    }


    // Below are the stub implementations for each method.

    static HRESULT __stdcall QueryInterfaceStub(IDirectDrawSurface7* This, const IID* const riid, LPVOID* ppvObj)
    {
        printf("QueryInterface called\n");
        if (ppvObj)
            ZeroMemory(ppvObj, sizeof(*ppvObj));
        return DD_OK;
    }

    static ULONG __stdcall AddRefStub(IDirectDrawSurface7* This)
    {
        printf("AddRef called\n");
        return 1;
    }

    static ULONG __stdcall ReleaseStub(IDirectDrawSurface7* This)
    {
        printf("Release called\n");
        return 0;
    }

    static HRESULT __stdcall AddAttachedSurfaceStub(IDirectDrawSurface7* This, LPDIRECTDRAWSURFACE7 surface)
    {
        printf("AddAttachedSurface called\n");
        return DD_OK;
    }

    static HRESULT __stdcall AddOverlayDirtyRectStub(IDirectDrawSurface7* This, LPRECT rect)
    {
        printf("AddOverlayDirtyRect called\n");
        return DD_OK;
    }

    static HRESULT __stdcall BltStub(IDirectDrawSurface7* This,
                                     LPRECT destRect,
                                     LPDIRECTDRAWSURFACE7 srcSurface,
                                     LPRECT srcRect,
                                     DWORD flags,
                                     LPDDBLTFX bltFx)
    {
        static int cnt = 0;
        cnt++;
        printf("Blt called %d\n", cnt);


        return DD_OK;
    }

    static HRESULT __stdcall BltBatchStub(IDirectDrawSurface7* This, LPDDBLTBATCH bltBatch, DWORD count, DWORD flags)
    {
        printf("BltBatch called\n");
        return DD_OK;
    }

    static HRESULT __stdcall BltFastStub(IDirectDrawSurface7* This,
                                         DWORD x,
                                         DWORD y,
                                         LPDIRECTDRAWSURFACE7 srcSurface,
                                         LPRECT srcRect,
                                         DWORD trans)
    {
        printf("BltFast called\n");
        return DD_OK;
    }

    static HRESULT __stdcall DeleteAttachedSurfaceStub(IDirectDrawSurface7* This, DWORD flags, LPDIRECTDRAWSURFACE7 surface)
    {
        printf("DeleteAttachedSurface called\n");
        return DD_OK;
    }

    static HRESULT __stdcall EnumAttachedSurfacesStub(IDirectDrawSurface7* This, LPVOID context, LPDDENUMSURFACESCALLBACK7 callback)
    {
        printf("EnumAttachedSurfaces called\n");
        return DD_OK;
    }

    static HRESULT __stdcall EnumOverlayZOrdersStub(IDirectDrawSurface7* This,
                                                    DWORD flags,
                                                    LPVOID context,
                                                    LPDDENUMSURFACESCALLBACK7 callback)
    {
        printf("EnumOverlayZOrders called\n");
        return DD_OK;
    }

    static HRESULT __stdcall FlipStub(IDirectDrawSurface7* This, LPDIRECTDRAWSURFACE7 targetOverride, DWORD flags)
    {
        printf("Flip called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetAttachedSurfaceStub(IDirectDrawSurface7* This, LPDDSCAPS2 caps, LPDIRECTDRAWSURFACE7* surface)
    {
        printf("GetAttachedSurface called\n");
        if (surface)
            ZeroMemory(surface, sizeof(*surface));
        return DD_OK;
    }

    static HRESULT __stdcall GetBltStatusStub(IDirectDrawSurface7* This, DWORD flags)
    {
        printf("GetBltStatus called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetCapsStub(IDirectDrawSurface7* This, LPDDSCAPS2 caps)
    {
        printf("GetCaps called\n");
        if (caps)
            ZeroMemory(caps, sizeof(*caps));
        return DD_OK;
    }

    static HRESULT __stdcall GetClipperStub(IDirectDrawSurface7* This, LPDIRECTDRAWCLIPPER* clipper)
    {
        printf("GetClipper called\n");
        if (clipper)
            ZeroMemory(clipper, sizeof(*clipper));
        return DD_OK;
    }

    static HRESULT __stdcall GetColorKeyStub(IDirectDrawSurface7* This, DWORD flags, LPDDCOLORKEY colorKey)
    {
        printf("GetColorKey called\n");
        if (colorKey)
            ZeroMemory(colorKey, sizeof(*colorKey));
        return DD_OK;
    }

    static HRESULT __stdcall GetDCStub(IDirectDrawSurface7* This, HDC* hdc)
    {
        printf("GetDC called\n");
        if (hdc)
            ZeroMemory(hdc, sizeof(*hdc));
        return DD_OK;
    }

    static HRESULT __stdcall GetFlipStatusStub(IDirectDrawSurface7* This, DWORD flags)
    {
        printf("GetFlipStatus called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetOverlayPositionStub(IDirectDrawSurface7* This, LPLONG x, LPLONG y)
    {
        printf("GetOverlayPosition called\n");
        if (x)
            ZeroMemory(x, sizeof(*x));
        if (y)
            ZeroMemory(y, sizeof(*y));
        return DD_OK;
    }

    static HRESULT __stdcall GetPaletteStub(IDirectDrawSurface7* This, LPDIRECTDRAWPALETTE* palette)
    {
        printf("GetPalette called\n");
        if (palette)
            ZeroMemory(palette, sizeof(*palette));
        return DD_OK;
    }

    static HRESULT __stdcall GetPixelFormatStub(IDirectDrawSurface7* This, LPDDPIXELFORMAT format) {
        printf("GetPixelFormat called\n");

        if (format) {
            // Initialize the DDPIXELFORMAT structure to zero
            ZeroMemory(format, sizeof(*format));

            // Set required fields for RGB555 format
            format->dwSize = sizeof(DDPIXELFORMAT); 

            // Set required fields for ARGB888 format
            format->dwSize = sizeof(DDPIXELFORMAT); // Size of the structure
            format->dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS; // Indicating it's an RGB format with alpha
            format->dwRGBBitCount = 32;             // 32 bits per pixel (ARGB888)

            // Masks for each color component
            format->dwRBitMask = 0x00FF0000;        // Red mask (8 bits, shifted 16 bits left)
            format->dwGBitMask = 0x0000FF00;        // Green mask (8 bits, shifted 8 bits left)
            format->dwBBitMask = 0x000000FF;        // Blue mask (8 bits)
            format->dwRGBAlphaBitMask = 0xFF000000; // Alpha mask (8 bits, shifted 24 bits left)
        }

        printf("Pixel format set to RGB32\n");
        return DD_OK;
    }


    static HRESULT __stdcall GetSurfaceDescStub(IDirectDrawSurface7* This, LPDDSURFACEDESC2 desc)
    {
        printf("GetSurfaceDesc called\n");
        if (desc)
            ZeroMemory(desc, sizeof(*desc));
        return DD_OK;
    }

    static HRESULT __stdcall InitializeStub(IDirectDrawSurface7* This, LPDIRECTDRAW dd, LPDDSURFACEDESC2 desc)
    {
        printf("Initialize called\n");
        return DD_OK;
    }

    static HRESULT __stdcall IsLostStub(IDirectDrawSurface7* This)
    {
        printf("IsLost called\n");
        return DD_OK;
    }

    static HRESULT __stdcall LockStub(IDirectDrawSurface7* This, LPRECT rect, LPDDSURFACEDESC2 desc, DWORD flags, HANDLE handle)
    {
        //printf("Lock called\n");

        if (rect)
        {
            printf("Rect: left=%ld, top=%ld, right=%ld, bottom=%ld\n", 
               rect->left, rect->top, rect->right, rect->bottom);
        }

        if (desc)
        {
            IDirectDrawSurface7Stub* pThis = (IDirectDrawSurface7Stub*)This;

            desc->dwSize = sizeof(DDSURFACEDESC2);
            desc->dwFlags = DDSD_PITCH | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_LPSURFACE;

            desc->lPitch = pThis->mWidth*4; //pThis->mWidth ;
            pThis->mData.resize(pThis->mWidth * pThis->mHeight * 4);
            desc->lpSurface = &pThis->mData[0];

            desc->dwWidth = pThis->mWidth; // Set the width
            desc->dwHeight = pThis->mHeight; // Set the height

            desc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            desc->ddpfPixelFormat.dwFlags = DDPF_RGB;
            desc->ddpfPixelFormat.dwRGBBitCount = 32; // Example: 32-bit color depth

            desc->ddsCaps.dwCaps = DDSCAPS_TEXTURE; // Set texture capabilities
        }
        //printf("Surface locked: width=%lu, height=%lu, pitch=%ld\n", desc->dwWidth, desc->dwHeight, desc->lPitch);

        return DD_OK;
    }

    static HRESULT __stdcall ReleaseDCStub(IDirectDrawSurface7* This, HDC hdc)
    {
        printf("ReleaseDC called\n");
        return DD_OK;
    }

    static HRESULT __stdcall RestoreStub(IDirectDrawSurface7* This)
    {
        printf("Restore called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetClipperStub(IDirectDrawSurface7* This, LPDIRECTDRAWCLIPPER clipper)
    {
        printf("SetClipper called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetColorKeyStub(IDirectDrawSurface7* This, DWORD flags, LPDDCOLORKEY colorKey)
    {
        printf("SetColorKey called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetOverlayPositionStub(IDirectDrawSurface7* This, LONG x, LONG y)
    {
        printf("SetOverlayPosition called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetPaletteStub(IDirectDrawSurface7* This, LPDIRECTDRAWPALETTE palette)
    {
        printf("SetPalette called\n");
        return DD_OK;
    }

    static void SaveVectorToFile(const std::vector<BYTE>& data, const char* filename) {
        // Step 1: Open the file in binary write mode
        FILE* file = fopen(filename, "wb");
        if (!file) {
            printf("Error: Could not open file '%s' for writing.\n", filename);
            return;
        }

        // Step 2: Write the vector's contents to the file
        size_t written = fwrite(&data[0], sizeof(BYTE), data.size(), file);
        if (written != data.size()) {
            printf("Error: Failed to write all data to '%s'.\n", filename);
        } else {
            printf("Data successfully written to '%s'.\n", filename);
        }

        // Step 3: Close the file
        fclose(file);
    }


    static HRESULT __stdcall UnlockStub(IDirectDrawSurface7* This, LPRECT rect);

    static HRESULT __stdcall UpdateOverlayStub(IDirectDrawSurface7* This,
                                               LPRECT srcRect,
                                               LPDIRECTDRAWSURFACE7 destSurface,
                                               LPRECT destRect,
                                               DWORD flags,
                                               LPDDOVERLAYFX overlayFx)
    {
        printf("UpdateOverlay called\n");
        return DD_OK;
    }

    static HRESULT __stdcall UpdateOverlayDisplayStub(IDirectDrawSurface7* This, DWORD flags)
    {
        printf("UpdateOverlayDisplay called\n");
        return DD_OK;
    }

    static HRESULT __stdcall UpdateOverlayZOrderStub(IDirectDrawSurface7* This, DWORD flags, LPDIRECTDRAWSURFACE7 refSurface)
    {
        printf("UpdateOverlayZOrder called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetDDInterfaceStub(IDirectDrawSurface7* This, LPVOID* interfacePtr)
    {
        printf("GetDDInterface called\n");
        if (interfacePtr)
        {
            // TODO
            *interfacePtr = &gDirectDraw7Stub;
        }
        return DD_OK;
    }

    static HRESULT __stdcall PageLockStub(IDirectDrawSurface7* This, DWORD flags)
    {
        printf("PageLock called\n");
        return DD_OK;
    }

    static HRESULT __stdcall PageUnlockStub(IDirectDrawSurface7* This, DWORD flags)
    {
        printf("PageUnlock called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetSurfaceDescStub(IDirectDrawSurface7* This, LPDDSURFACEDESC2 desc, DWORD flags)
    {
        printf("SetSurfaceDesc called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetPrivateDataStub(IDirectDrawSurface7* This, const GUID* const guid, LPVOID data, DWORD dataSize, DWORD flags)
    {
        printf("SetPrivateData called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetPrivateDataStub(IDirectDrawSurface7* This, const GUID* const guid, LPVOID data, LPDWORD dataSize)
    {
        printf("GetPrivateData called\n");
        if (data)
            ZeroMemory(data, *dataSize);
        return DD_OK;
    }

    static HRESULT __stdcall FreePrivateDataStub(IDirectDrawSurface7* This, const GUID* const guid)
    {
        printf("FreePrivateData called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetUniquenessValueStub(IDirectDrawSurface7* This, LPDWORD value)
    {
        printf("GetUniquenessValue called\n");
        if (value)
            ZeroMemory(value, sizeof(*value));
        return DD_OK;
    }

    static HRESULT __stdcall ChangeUniquenessValueStub(IDirectDrawSurface7* This)
    {
        printf("ChangeUniquenessValue called\n");
        return DD_OK;
    }

    static HRESULT __stdcall SetPriorityStub(IDirectDrawSurface7* This, DWORD priority)
    {
        printf("SetPriority called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetPriorityStub(IDirectDrawSurface7* This, LPDWORD priority)
    {
        printf("GetPriority called\n");
        if (priority)
            ZeroMemory(priority, sizeof(*priority));
        return DD_OK;
    }

    static HRESULT __stdcall SetLODStub(IDirectDrawSurface7* This, DWORD lod)
    {
        printf("SetLOD called\n");
        return DD_OK;
    }

    static HRESULT __stdcall GetLODStub(IDirectDrawSurface7* This, LPDWORD lod)
    {
        printf("GetLOD called\n");
        if (lod)
            ZeroMemory(lod, sizeof(*lod));
        return DD_OK;
    }
};

HRESULT __stdcall IDirectDraw7Stub::CreateSurfaceStub(IDirectDraw7* This,
                                                      LPDDSURFACEDESC2 desc,
                                                      LPDIRECTDRAWSURFACE7* surface,
                                                      IUnknown* outerUnknown)
{
    printf("CreateSurface called\n");

    if (desc)
    {
        // Debug print the details of the surface description
        printf("Surface Description Details:\n");
        printf("  dwSize: %lu\n", desc->dwSize);
        printf("  dwFlags: 0x%08lx\n", desc->dwFlags);
        if (desc->dwFlags & DDSD_WIDTH)
        {
            printf("  Width: %lu\n", desc->dwWidth);
        }
        if (desc->dwFlags & DDSD_HEIGHT)
        {
            printf("  Height: %lu\n", desc->dwHeight);
        }
        if (desc->dwFlags & DDSD_PITCH)
        {
            printf("  Pitch: %lu\n", desc->lPitch);
        }
        if (desc->dwFlags & DDSD_PIXELFORMAT)
        {
            printf("  Pixel Format:\n");
            printf("    dwSize: %lu\n", desc->ddpfPixelFormat.dwSize);
            printf("    dwFlags: 0x%08lx\n", desc->ddpfPixelFormat.dwFlags);
            printf("    Bit Depth: %lu bits\n", desc->ddpfPixelFormat.dwRGBBitCount);
        }
        if (desc->dwFlags & DDSD_CAPS)
        {
            printf("  Caps: 0x%08lx\n", desc->ddsCaps.dwCaps);
        }
    }
    else
    {
        printf("  Surface description is NULL\n");
    }

    if (surface)
    {
        IDirectDrawSurface7Stub* t = new IDirectDrawSurface7Stub();
        t->mWidth = desc->dwWidth;
        t->mHeight = desc->dwHeight;

        *surface = (IDirectDrawSurface7*)t;

        printf("Surface Width: %lu, Surface Height: %lu\n", t->mWidth, t->mHeight);

        printf("  Surface successfully created.\n");
    }
    else
    {
        printf("  Surface pointer is NULL. Unable to create surface.\n");
    }

    return DD_OK;
}


// ===========================================

static HINSTANCE gHinstance;
static SVideoFunctions* gVideoFuncs_100FFF8;

#define FLAG_HARDWARE_RENDERING 0x40

static s32 gNumDisplayModes = 0;
static s32 gDisplayModeIdx = 0;
static s32 gBitCountFilter = 0;
static std::vector<SDisplayMode> gDisplayModes;

#pragma comment(lib, "d3d9.lib")
#include <d3d9.h>

Renderer::Renderer() : mHwnd(NULL), g_pD3D(NULL), g_pD3DDevice(NULL), mVbo(NULL), mDeviceReady(false), mPresentParams(NULL), mOGW(640.0f), mOGH(640.0f), mCurH(1.0f), mCurW(1.0f), mFmvTexture(NULL)
{
    mPresentParams = new D3DPRESENT_PARAMETERS;
    ZeroMemory(mPresentParams, sizeof(D3DPRESENT_PARAMETERS));

}

Renderer::~Renderer()
{
    ImGui_ImplDX9_Shutdown();
    //ImGui::DestroyContext();

    FreeTextures();
    FreeVBO();

    if (mFmvTexture)
    {
        mFmvTexture->Release();
    }

    if (g_pD3DDevice)
    {
        g_pD3DDevice->Release();
    }

    if (g_pD3D)
    {
        g_pD3D->Release();
    }

    delete mPresentParams;
}

void Renderer::InitD3D(HWND hWnd, s32 ogW, s32 ogH)
{
    if (g_pD3D)
    {
        return;
    }

    printf("InitD3D\n");

    mHwnd = hWnd;
    mOGW = static_cast<float>(ogW);
    mOGH = static_cast<float>(ogH);

    FixWindow();
    PostMessage(mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    SetForegroundWindow(mHwnd);
    SetWindowText(mHwnd, "GTA2 (DX9 MrSapps/CriminalRETeam)");

    GetCurWH();

    // Create the D3D object.
    g_pD3D = ::Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL)
    {
        VerifyHR(E_FAIL, "Direct3DCreate9 returned null");
    }

    // Set our display mode to Windowed
    mPresentParams->Windowed = TRUE;
    // Discard old frames
    mPresentParams->SwapEffect = D3DSWAPEFFECT_DISCARD;
    // Set the device window to our window pointer passed as a parameter
    mPresentParams->hDeviceWindow = hWnd;

    mPresentParams->BackBufferFormat = D3DFMT_X8R8G8B8;
    mPresentParams->BackBufferWidth = mCurW;
    mPresentParams->BackBufferHeight = mCurH;

    // create a device class using this information and the info from the mPresentParams stuct
    VerifyHR(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                  D3DDEVTYPE_HAL,
                                  hWnd,
                                  D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                  mPresentParams,
                                  &g_pD3DDevice),
             "CreateDevice");
    if (g_pD3DDevice == NULL)
    {
        VerifyHR(E_FAIL, "CreateDevice returned null device");
    }

    // Default view port should be OK

    ImGui_ImplDX9_Init(mHwnd, g_pD3DDevice);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.5f; // Scale UI globally


    VerifyHR(g_pD3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX), "SetFVF");

    CreateVBO();

    g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    mDeviceReady = true;
}

// 10.5.exe address
typedef void(__stdcall* T_Input_ReleaseMouse)();
static T_Input_ReleaseMouse pInput_ReleaseMouse = NULL;

void Renderer::BeginScene()
{
    HINSTANCE exeInstance = GetModuleHandle(NULL);

    // Do this here rather than the msg loop because sometimes the mouse gets locked and its
    // a long time before we get a window msg, where as this is always called often
    if (pInput_ReleaseMouse)
    {
        pInput_ReleaseMouse();
    }

    if (!mDeviceReady)
    {
        return;
    }
    //printf("BeginScene\n");

    // Begin our scene
    VerifyHR(g_pD3DDevice->BeginScene(), "BeginScene");

    ImGui_ImplDX9_NewFrame();

    // Somehow only in wine this gets turned off... so just turn it back on ya mad lad
    g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

void Renderer::EndScene()
{
    if (!mDeviceReady)
    {
        return;
    }
    //printf("EndScene\n");

    ImGui::Render();

    // End our scene
    VerifyHR(g_pD3DDevice->EndScene(), "EndScene");
}

void Renderer::Clear(u8 r, u8 g, u8 b, s32 left, s32 top, s32 right, s32 bottom)
{
    if (!mDeviceReady)
    {
        return;
    }

    const bool useRect = !(left == 0 && top == 0 && right == 0 && bottom == 0);
    const D3DRECT rect = {left, top, right, bottom};

    // Clear the target buffer
    VerifyHR(g_pD3DDevice->Clear(useRect ? 1 : 0, useRect ? &rect : NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(r, g, b), 1.0f, 0),
             "Clear device");
}

static float mapValue(float oldValue, float oldMax, float newMax) 
{
    return (oldValue / oldMax) * newMax;
}

void Renderer::Draw(const Renderer::CUSTOMVERTEX* pVerts, u32 vertsCount, IDirect3DTexture9* pTexture)
{
    if (!mDeviceReady || vertsCount > 4)
    {
        return;
    }

    Renderer::CUSTOMVERTEX tmp[4];
    memcpy(tmp, pVerts, sizeof(Renderer::CUSTOMVERTEX) * vertsCount);

    //printf("Draw\n");
    s32 i;
    for (i = 0; i < vertsCount; i++)
    {
        tmp[i].x = mapValue(pVerts[i].x, mOGW, mCurW);
        tmp[i].y = mapValue(pVerts[i].y, mOGH, mCurH);
    }

    CUSTOMVERTEX* pBuffer = NULL;
    VerifyHR(mVbo->Lock(0, 0, (void**)&pBuffer, D3DLOCK_DISCARD), "VBO lock");
    memcpy(pBuffer, tmp, vertsCount * sizeof(CUSTOMVERTEX));
    VerifyHR(mVbo->Unlock(), "VBO unlock");

    VerifyHR(g_pD3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX), "SetFVF");

    VerifyHR(g_pD3DDevice->SetTexture(0, pTexture), "SetTexture");

    if (vertsCount == 4)
    {
        VerifyHR(g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2), "DrawPrimitive D3DPT_TRIANGLEFAN");
    }
    else if (vertsCount == 3)
    {
        VerifyHR(g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1), "DrawPrimitive D3DPT_TRIANGLELIST");
    }
    else
    {
        VerifyHR(E_FAIL, "Can only handle 3 or 4 vertices");
    }
}

static void set_pixel(u32* pPixels, u32 pitch, s32 x, s32 y, u32 pixel)
{
    u32* const target_pixel = (pPixels) + (y * (pitch / sizeof(u32))) + (x);
    *target_pixel = pixel;
}

static u32 convertRGB565ToRGB888(u16 rgb565)
{
    // Extract the 5-bit red, 6-bit green, and 5-bit blue components
    u16 red565 = (rgb565 >> 11) & 0x1F; // Red is in the top 5 bits
    u16 green565 = (rgb565 >> 5) & 0x3F; // Green is in the middle 6 bits
    u16 blue565 = rgb565 & 0x1F; // Blue is in the lowest 5 bits

    // Expand to 8-bit red, green, and blue
    u8 red = (red565 * 255) / 31; // Map 5-bit to 8-bit
    u8 green = (green565 * 255) / 63; // Map 6-bit to 8-bit
    u8 blue = (blue565 * 255) / 31; // Map 5-bit to 8-bit

    // Combine into a 32-bit RGB888 value
    u32 rgb888 = (red << 16) | (green << 8) | blue;
    return rgb888;
}

static u32 ConvertRGB555toRGB888(u16 rgb555)
{
    // Extract RGB555 channels
    u8 red = (rgb555 & 0x7C00) >> 10; // 5 bits for red
    u8 green = (rgb555 & 0x03E0) >> 5; // 5 bits for green
    u8 blue = rgb555 & 0x001F; // 5 bits for blue

    // Scale 5-bit values to 8-bit values
    u8 red888 = (red << 3) | (red >> 2); // Scale: shift left and fill bits
    u8 green888 = (green << 3) | (green >> 2);
    u8 blue888 = (blue << 3) | (blue >> 2);

    // Combine into RGB888 format
    u32 rgb888 = (red888 << 16) | (green888 << 8) | blue888;
    return rgb888;
}

static u32 setAlpha(u32 rgb888, u32 alpha)
{
    u32 argb8888 = (alpha << 24) | rgb888; // Add alpha as the most significant 8 bits
    return argb8888;
}

IDirect3DTexture9* Renderer::SetTexture(IDirect3DTexture9* pExisting,
                                        u32 w,
                                        u32 h,
                                        const u8* pPixelData,
                                        const u16* pPalData,
                                        const s32* pitch)
{
    if (pExisting)
    {
        pExisting->Release();
    }

    // Create empty IDirect3DTexture9*
    IDirect3DTexture9* pTexture = NULL;
    VerifyHR(g_pD3DDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, 0), "CreateTexture");

    D3DLOCKED_RECT outRect;
    VerifyHR(pTexture->LockRect(0, &outRect, NULL, D3DLOCK_DISCARD), "LockRect");
    s32 rowPitch = outRect.Pitch;
    u32* pixels = (u32*)outRect.pBits;
    //    printf("Pitch %d ptr %X w %d h %d\n", rowPitch, pixels, w, h);

    bool stop = false;
    u32 sourcePixelIndex = 0;
    for (s32 y = 0; y < h; ++y)
    {
        for (s32 x = 0; x < w; ++x)
        {
            if (pPixelData && pPalData)
            {
                // TODO: HACK - og game seems to pass a buffer smaller than w*h ??
                u8 pixel_idx = 0;
                __try
                {
                    pixel_idx = pPixelData[sourcePixelIndex];
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    stop = true;
                    break;
                }

                sourcePixelIndex++;
                //pixels[x + (rowPitch / sizeof(u32)) * y] = pixel_idx;
                u32 pixelVal = convertRGB565ToRGB888(pPalData[pixel_idx]);
                u32 withAlpha = setAlpha(pixelVal, pixel_idx == 0 ? 0 : 255);
                set_pixel(pixels, rowPitch, x, y, withAlpha);
            }
            else if (pPixelData && pitch)
            {
                const u8* pSrc = pPixelData;
                pSrc += *pitch;

                u32 pixelVal = ((u16*)pSrc)[sourcePixelIndex];

                pixelVal = ConvertRGB555toRGB888(pixelVal); // TODO : RGB555
                // convertRGB565ToRGB888(pPalData[pixel_idx]);
                u32 withAlpha = setAlpha(pixelVal, 255);

                s32 reverseY = h - 1 - y; // Bottom-to-top
                set_pixel(pixels, rowPitch, x, reverseY, withAlpha);
                // pitch
                //                pixels[x + (rowPitch / sizeof(u32)) * y] = 0x00ff00ff;

                sourcePixelIndex++;

                // TODO: Handle normal case where no pixel data is passed
            }
            else
            {
                set_pixel(pixels, rowPitch, x, y, 0);
            }
        }
        if (stop)
        {
            break;
        }

        if (!pitch)
        {
            // always move in 256 increments ??
            const u32 val = 256 - w;
            sourcePixelIndex += val;
        }
    }
    VerifyHR(pTexture->UnlockRect(0), "Unlock rect");

    return pTexture;
}

IDirect3DTexture9* Renderer::SetFmvTexture(
                                     u32 w,
                                     u32 h,
                                     u32 wOffset,
                                     u32 hOffset,
                                     const u32* pPixelData)
{
    if (!mFmvTexture)
    {
        // Create the texture with a fixed resolution of 640x480
        VerifyHR(g_pD3DDevice->CreateTexture(640, 480, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mFmvTexture, 0), "CreateTexture FMV");
    }

    D3DLOCKED_RECT outRect;
    VerifyHR(mFmvTexture->LockRect(0, &outRect, NULL, D3DLOCK_DISCARD), "LockRect FMV");

    s32 rowPitch = outRect.Pitch;          // Number of bytes per row in the locked texture
    u32* pixels = (u32*)outRect.pBits;    // Pointer to the locked texture memory

    // Loop over the destination texture (640x480 range)
    for (s32 destY = 0; destY < 480; ++destY)
    {
        for (s32 destX = 0; destX < 640; ++destX)
        {
            // Calculate the corresponding source coordinates
            s32 sourceX = destX + wOffset;
            s32 sourceY = destY + hOffset;

            // Ensure the source coordinates are within bounds of the source texture dimensions (w/h)
            if (sourceX >= 0 && sourceX < w &&
                sourceY >= 0 && sourceY < h)
            {
                // Calculate the source index in pPixelData
                u32 sourcePixelIndex = sourceY * w + sourceX;

                // Convert the 16-bit RGB555 pixel to 32-bit RGB888
                u32 pixelVal = pPixelData[sourcePixelIndex];

                // Write the converted pixel into the destination texture
                set_pixel(pixels, rowPitch, destX, destY, pixelVal);
            }
        }
    }

    VerifyHR(mFmvTexture->UnlockRect(0), "Unlock rect FMV");

    DrawFmvTexture();

    return mFmvTexture;
}

void Renderer::DrawFmvTexture()
{
    CUSTOMVERTEX vertices[4] = {
        { 0.0f, 0.0f, 0.5f, 1.0f, 0xFFFFFFFF, 0x00000000, 0.0f, 0.0f },   // Top-left (u=0, v=0)
        { 640.0f, 0.0f, 0.5f, 1.0f, 0xFFFFFFFF, 0x00000000, 1.0f, 0.0f }, // Top-right (u=1, v=0)
        { 640.0f, 480.0f, 0.5f, 1.0f, 0xFFFFFFFF, 0x00000000, 1.0f, 1.0f }, // Bottom-right (u=1, v=1)
        { 0.0f, 480.0f, 0.5f, 1.0f, 0xFFFFFFFF, 0x00000000, 0.0f, 1.0f }  // Bottom-left (u=0, v=1)
    };
    Draw(vertices, 4, mFmvTexture);
}

void Renderer::FreeTextureDelayed(IDirect3DTexture9* pTexture)
{
    if (pTexture)
    {
        mFreeLaterTextures.push_back(pTexture);
    }
}

void Renderer::Present()
{
    if (!mDeviceReady)
    {
        HRESULT hr = g_pD3DDevice->TestCooperativeLevel();
        if (hr == S_OK)
        {
            printf("TestCooperativeLevel S_OK (??)\n");
            mDeviceReady = true;
        }
        else if (hr == D3DERR_DEVICENOTRESET)
        {
            // Release resources in D3DPOOL_DEFAULT.
            ImGui_ImplDX9_InvalidateDeviceObjects();
            FreeVBO();

            hr = g_pD3DDevice->Reset(mPresentParams);
            if (SUCCEEDED(hr))
            {
                printf("Device reset done\n");
                mDeviceReady = true;
                CreateVBO();
            }
            else
            {
                VerifyHR(hr, "Reset");
            }
        }
        else if (hr == D3DERR_DEVICELOST)
        {
            printf("Still in D3DERR_DEVICELOST\n");
        }
        else
        {
            VerifyHR(hr, "TestCooperativeLevel");
        }
    }
    else
    {
        // No active texture
        VerifyHR(g_pD3DDevice->SetTexture(0, NULL), "SetTexture");

        // Display our scene
        const HRESULT hr = g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
        if (hr == D3DERR_DEVICELOST)
        {
            mDeviceReady = false;
            printf("Device lost, waiting for D3DERR_DEVICENOTRESET state\n");
        }
        else
        {
            VerifyHR(hr, "Present");
        }

        // Free textures that are no longer in use
        FreeTextures();
    }
}

static bool IsAlphaBlendEnabled(u32 states)
{
    return states & 0x200 ? true : false; // ALPHA_BLEND_MASK
}

static bool IsModulateAlphaEnabled(u32 states)
{
    return states & 0x180 ? true : false; // MODULATE_ALPHA_MASK
}

void Renderer::SetRenderStates(u32 states)
{
    if (IsAlphaBlendEnabled(states))
    {
        g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else if (IsModulateAlphaEnabled(states))
    {
        g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
    else
    {
        g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
}

void Renderer::TrackTexture(STexture* pTexture)
{
    mTrackedTextures.insert(pTexture);
}

void Renderer::UnTrackTexture(STexture* pTexture)
{
    std::set<STexture*>::iterator it = mTrackedTextures.find(pTexture);
    if (it != mTrackedTextures.end())
    {
        mTrackedTextures.erase(it);
    }
}

void Renderer::Resize(s32 w, s32 h)
{
    mPresentParams->BackBufferWidth = w;
    mPresentParams->BackBufferHeight = h;
    GetCurWH();

    ImGui_ImplDX9_InvalidateDeviceObjects();
    FreeVBO();
    VerifyHR(g_pD3DDevice->SetTexture(0, NULL), "SetTexture");

    const HRESULT hr = g_pD3DDevice->Reset(mPresentParams);
    if (hr == D3DERR_DEVICELOST)
    {
        mDeviceReady = false;
        printf("Resize reset device lost\n");
    }
    else if (hr == S_OK)
    {
        printf("Resize reset OK\n");
        CreateVBO();
        ImGui_ImplDX9_CreateDeviceObjects();
    }
    else
    {
        VerifyHR(hr, "Resize Reset");
    }
}


void Renderer::VerifyHR(HRESULT hr, const char* msg)
{ 
    if (!SUCCEEDED(hr))
    {
        char buffer[512];
        sprintf(buffer, "Function failed with 0x%X msg: %s", hr, msg);
        printf("%s\n", buffer);
        ::MessageBox(mHwnd, buffer, "GTA2 Error", MB_OK | MB_ICONERROR);
        exit(-1);
    }
}

void Renderer::FreeTextures()
{
    if (!mFreeLaterTextures.empty())
    {
        printf("mFreeLaterTextures %d mTrackedTextures %d\n", mFreeLaterTextures.size(), mTrackedTextures.size());
    }

    for (size_t i = 0; i < mFreeLaterTextures.size(); i++)
    {
        const ULONG refCount = mFreeLaterTextures[i]->Release();
        if (refCount != 0)
        {
            VerifyHR(E_FAIL, "Freed texture still in use");
        }
    }
    mFreeLaterTextures.clear();
}

void Renderer::CreateVBO()
{
    FreeVBO();
    VerifyHR(g_pD3DDevice->CreateVertexBuffer(4 * sizeof(Renderer::CUSTOMVERTEX),
                                              D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                              D3DFVF_CUSTOMVERTEX,
                                              D3DPOOL_DEFAULT,
                                              &mVbo,
                                              NULL),
             "CreateVertexBuffer");
    VerifyHR(g_pD3DDevice->SetStreamSource(0, mVbo, 0, sizeof(Renderer::CUSTOMVERTEX)), "SetStreamSource VBO");
}

void Renderer::FreeVBO()
{
    if (mVbo)
    {
        VerifyHR(g_pD3DDevice->SetStreamSource(0, NULL, 0, 0), "Unbind VBO");

        const ULONG refCount = mVbo->Release();
        if (refCount != 0)
        {
            VerifyHR(E_FAIL, "Freed VBO still in use");
        }
        mVbo = NULL;
    }
}

static bool modifyingStyle = false;

static void AddBorderAndButtons(HWND hwnd)
{
    modifyingStyle = true;

    // Get current style
    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    // Add window styles for border and buttons
    style |= WS_OVERLAPPEDWINDOW; // Includes WS_BORDER, WS_CAPTION, WS_SYSMENU, WS_MINIMIZEBOX, WS_MAXIMIZEBOX

    // Set updated style
    SetWindowLong(hwnd, GWL_STYLE, style);

    // Apply the changes by calling SetWindowPos
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    // Force the window to update its frame and apply the new style
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Request a redraw to ensure the window updates visually
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

    modifyingStyle = false;

    while (!ShowCursor(TRUE))
    {
    }
}

static WNDPROC originalProc = NULL;

static LRESULT CALLBACK CustomWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

enum SubClassState
{
    SubClassNone,
    SubClassed,
    UnSubClassed,
};
static SubClassState gSubClassState = SubClassNone;

static void SubclassWindow(HWND hwnd)
{
    if (gSubClassState == UnSubClassed || gSubClassState == SubClassNone)
    {
        originalProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)CustomWindowProc);

        if (!originalProc)
        {
            printf("Failed to subclass window!\n");
        }
        gSubClassState = SubClassed;
    }
}

static void UnsubclassWindow(HWND hwnd)
{
    if (gSubClassState == SubClassed)
    {
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)originalProc);
        gSubClassState = UnSubClassed;
    }
}

static DmaVideoDataWrapper* gWrapper = NULL;

static LRESULT CALLBACK CustomWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (pInput_ReleaseMouse)
    {
        pInput_ReleaseMouse();
    }

    switch (msg)
    {
        case WM_STYLECHANGED:
            if (!modifyingStyle)
            {
                AddBorderAndButtons(hwnd);
                printf("WM_STYLECHANGED!\n");
                printf("Good bye subclass\n");
                UnsubclassWindow(hwnd);
            }
            return 0;

        case WM_SIZING:
            printf("WM_SIZING - call def proc\n");
            if (gWrapper)
            {
                gWrapper->mRenderer.Present();
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);

        default:
            break;
    }
    return CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
}

void Renderer::FixWindow()
{
    AddBorderAndButtons(mHwnd);
    SubclassWindow(mHwnd);
}

void Renderer::GetCurWH()
{
    RECT rect;
    GetClientRect(mHwnd, &rect);

    const int windowWidth = rect.right - rect.left;
    const int windowHeight = rect.bottom - rect.top;

    // Output the dimensions
    printf("GetCurWH Window Width: %d, Window Height: %d\n", windowWidth, windowHeight);

    mCurW = static_cast<float>(windowWidth);
    mCurH = static_cast<float>(windowHeight);
}

DmaVideoDataWrapper::DmaVideoDataWrapper()
{
    memset(&mVideo, 0, sizeof(SVideo));

    int width = GetSystemMetrics(SM_CXSCREEN); // Screen width in pixels
    int height = GetSystemMetrics(SM_CYSCREEN); // Screen height in pixels

    // Bink attempts to "directly write" to the rect of the window using the entire
    // screen area as the buffer
    mVideo.field_134_SurfacePrimary = new IDirectDrawSurface7Stub(width, height);
}

DmaVideoDataWrapper::~DmaVideoDataWrapper()
{
    delete mVideo.field_134_SurfacePrimary;
}

SVideo* CC Vid_Init_SYS(s32 param1, u16 param2_flags)
{
    printf("Vid_Init_SYS\n");

    DmaVideoDataWrapper* wrapper = new DmaVideoDataWrapper();

    wrapper->mVideo.field_7C_self_dll_handle = gHinstance;
    wrapper->mVideo.field_84_from_initDLL = gVideoFuncs_100FFF8;
    wrapper->mVideo.field_0 = 1;
    wrapper->mVideo.field_78 = param1;
    wrapper->mVideo.field_14_display_mode_count_2_q = 1;
    wrapper->mVideo.field_18_num_guids = 2;
    wrapper->mVideo.field_4_flags = param2_flags & FLAG_HARDWARE_RENDERING | 0x200;

    Vid_FindFirstMode(&wrapper->mVideo, 32);
    while (Vid_FindNextMode(&wrapper->mVideo) != 0)
    {
    }

    gWrapper = wrapper;
    return &wrapper->mVideo;
}

s32 CC Vid_CheckMode(SVideo* pVideoDriver, s32 width, s32 height, s32 rgbBitCount)
{
    printf("Vid_CheckMode %dx%d bp %d\n", width, height, rgbBitCount);

    if (!pVideoDriver)
    {
        return 0;
    }

    s32 i = 0;
    for (i = 0; i < gDisplayModes.size(); i++)
    {
        const SDisplayMode& m = gDisplayModes[i];
        if (m.field_8_width == width && m.field_C_height == height)
        {
            printf("Found with idx %d\n", m.field_0_display_mode_idx);
            return m.field_0_display_mode_idx;
        }
    }

    return 0;
}

SDevice* CC Vid_FindDevice(SVideo* pVideoDriver, s32 deviceId)
{
    printf("Vid_FindDevice %d\n", deviceId);

    static SDevice hack;
    hack.field_4_flags = 1;
    return &hack;
}

SDisplayMode* CC Vid_FindMode(SVideo* pVideoDriver, s32 modeId)
{
    printf("Vid_FindMode %d\n", modeId);

    if (!pVideoDriver || modeId == -2 && pVideoDriver->field_34_active_device_id > 1)
    {
        return 0;
    }

    return 0;
}

s32 CC Vid_FindFirstMode(SVideo* pVideoDriver, s32 rgbBitCountFilter)
{
    printf("Vid_FindFirstMode filter = %d\n", rgbBitCountFilter);

    if (!pVideoDriver)
    {
        return 0;
    }

    gNumDisplayModes = 1;
    printf("gNumDisplayModes = %d\n", gNumDisplayModes);
    gBitCountFilter = rgbBitCountFilter;
    gDisplayModeIdx = 0;
    gDisplayModes.clear();
    gDisplayModes.reserve(gNumDisplayModes);
    return Vid_FindNextMode(pVideoDriver);
}

s32 CC Vid_FindNextMode(SVideo* pVideoDriver)
{
    printf("Vid_FindNextMode\n");

    if (!pVideoDriver || gDisplayModeIdx >= gNumDisplayModes)
    {
        return 0;
    }

    gDisplayModeIdx++; // 0 isn't valid

    SDisplayMode tmp;
    memset(&tmp, 0, sizeof(tmp));

    tmp.field_0_display_mode_idx = gDisplayModeIdx;
    tmp.field_4_deviceId = 0;
    tmp.field_8_width = 640;
    tmp.field_C_height = 480;

    // TOOD: Hack, calc properly if these are actually used
    tmp.field_10_pitch = 640;
    tmp.field_14_rgb_bit_count = 32;
    printf("Vid_FindNextMode add %dx%d\n", 640, 480);

    gDisplayModes.push_back(tmp);

    return gDisplayModeIdx;
}

void CC Vid_CloseScreen(SVideo* pVideo)
{
    TRACE_ENTRYEXIT;
}

s32 CC Vid_SetDevice(SVideo* pVideoDriver, s32 deviceId)
{
    TRACE_ENTRYEXIT;
    return 0;
}

s32 CC Vid_SetMode(SVideo* pVideoDriver, HWND hWnd, s32 modeId)
{
    printf("Vid_SetMode %d\n", modeId);

    DmaVideoDataWrapper* pWrapper = (DmaVideoDataWrapper*)pVideoDriver;
    if (pWrapper)
    {
        const SDisplayMode& mode = gDisplayModes[modeId-1];
        printf("Vid_SetMode %dx%d\n", mode.field_8_width, mode.field_C_height);
        pWrapper->mRenderer.InitD3D(hWnd, mode.field_8_width, mode.field_C_height);
    }

    return 0;
}

void CC Vid_GrabSurface(SVideo* pVideoDriver)
{
    TRACE_ENTRYEXIT;

    // Lock the screen buffer/restore if lost
}

void CC Vid_ReleaseSurface(SVideo* pVideoDriver)
{
    TRACE_ENTRYEXIT;

    // Unlock the screen buffer/restore if lost
}

void CC Vid_FlipBuffers(SVideo* pVideo)
{
    //TRACE_ENTRYEXIT;

    // Call flip/blt surface to the primary surface via pVideo->field_4C0_hwn
    DmaVideoDataWrapper* pWrapper = (DmaVideoDataWrapper*)pVideo;
    if (pWrapper)
    {
        pWrapper->mRenderer.Present();
    }
}

    HRESULT __stdcall IDirectDrawSurface7Stub::UnlockStub(IDirectDrawSurface7* This, LPRECT rect)
    {
        static int cnt = 0;
        cnt++;
        printf("Unlock called %d\n", cnt);

        if (gWrapper)
        {
            gWrapper->mRenderer.BeginScene();

            RECT clientRect;
            POINT topLeft;
            if (GetClientRect(gWrapper->mRenderer.GetHwnd(), &clientRect)) 
            {
                // Convert the top-left corner of the client area to screen coordinates
                topLeft.x = clientRect.left;
                topLeft.y = clientRect.top;

                if (ClientToScreen(gWrapper->mRenderer.GetHwnd(), &topLeft)) 
                {
                   // printf("Top-left pixel of client area: X=%ld, Y=%ld\n", topLeft.x, topLeft.y);
                }

                // Note the FMV looks slightly offset, yet its actually correct its just the way the FMV has been recorded
                IDirectDrawSurface7Stub* pThis = (IDirectDrawSurface7Stub*)This;
                IDirect3DTexture9* pTexture = gWrapper->mRenderer.SetFmvTexture(pThis->mWidth, pThis->mHeight, topLeft.x, topLeft.y, (u32*)&pThis->mData[0]);

            }

            gWrapper->mRenderer.EndScene();
            gWrapper->mRenderer.Present();
        }

        /*
        if (cnt == 95)
        {
            // y = 852
            // x = 1600
            IDirectDrawSurface7Stub* pThis = (IDirectDrawSurface7Stub*)This;
            printf("wh %dx%d len %d\n", pThis->mWidth, pThis->mHeight, pThis->mData.size());
            SaveVectorToFile(pThis->mData, "test.dat");
            abort();  
        }*/
        return DD_OK;
    }

void CC Vid_ShutDown_SYS(SVideo* pVideoDriver)
{
    TRACE_ENTRYEXIT;

    // Restore display mode, free DX, free display modes and devices

    DmaVideoDataWrapper* pWrapper = (DmaVideoDataWrapper*)pVideoDriver;
    delete pWrapper;
}

s32 CC Vid_EnableWrites(SVideo* pVideoDriver)
{
    printf("Vid_EnableWrites\n");

    //pVideoDriver->field_50_surface_pixels_ptr = buffer;
    //pVideoDriver->field_54_surface_pixels_pitch = 640 * 2;

    // Sets
    // pVideoDriver->field_50_surface_pixels_ptr = pVideoDriver->field_13C_DDSurfaceDesc7.lpSurface;
    // pVideoDriver->field_54_surface_pixels_pitch = pVideoDriver->field_13C_DDSurfaceDesc7.lPitch;
    return 0;
}

s32 CC Vid_DisableWrites(SVideo* pVideoDriver)
{
    printf("Vid_DisableWrites\n");

    // Sets
    //pVideoDriver->field_50_surface_pixels_ptr = 0;
    //pVideoDriver->field_4_flags &= 0xFD;

    return 0;
}

s32 CC Vid_GetSurface(SVideo* pVideoDriver)
{
    //printf("Vid_GetSurface\n");
    //TRACE_ENTRYEXIT;

    //Vid_GrabSurface();
    //Vid_DisableWrites();

    return 0;
}

s32 CC Vid_FreeSurface(SVideo* pVideoDriver)
{
    //printf("Vid_FreeSurface\n");
    //TRACE_ENTRYEXIT;

    // Unlocks primary surface, doesn't actually free it

    return 0;
}

s32 CC Vid_ClearScreen(SVideo* pVideoDriver, u8 aR, u8 aG, u8 aB, s32 aLeft, s32 aTop, s32 aRight, s32 aBottom)
{
    DmaVideoDataWrapper* pWrapper = (DmaVideoDataWrapper*)pVideoDriver;
    if (pWrapper)
    {
        pWrapper->mRenderer.Clear(aR, aG, aB, aLeft, aTop, aRight, aBottom);
    }
    return 0;
}

s32 CC Vid_SetGamma(SVideo* pVideoDriver, f32 gR, f32 gG, f32 gB)
{
    TRACE_ENTRYEXIT;
    return 0;
}

IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam);

s32 CC Vid_WindowProc(SVideo* pVideoDriver, HWND hwnd, DWORD uMsg, WPARAM wParam, LPARAM lParam)
{
    if (pInput_ReleaseMouse)
    {
        pInput_ReleaseMouse();
    }

    if (ImGui_ImplDX9_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return 0; // Let ImGui handle the input
    }

    switch (uMsg)
    {
        case WM_ENTERSIZEMOVE:
            printf("WM_ENTERSIZEMOVE - add subclass\n");
            SubclassWindow(hwnd);
            break;

        case WM_EXITSIZEMOVE:
            printf("WM_EXITSIZEMOVE - remove subclass\n");
            UnsubclassWindow(hwnd);
            break;

        // This will be hit when sized without the user resizing the window
        case WM_SIZE:
            //printf("Resize\n");
            AddBorderAndButtons(hwnd);
            if (wParam != SIZE_MINIMIZED && gWrapper)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                gWrapper->mRenderer.Resize(rect.right - rect.left, rect.bottom - rect.top);
            }
            break;

        default:
            break;
    }
    return 0;
}

#pragma comment(lib, "version.lib") // Link version.lib for file version functions

struct ExecutableVersion {
    DWORD major;
    DWORD minor;
    DWORD build;
    DWORD revision;
};

static ExecutableVersion GetExecutableVersion(HINSTANCE hInstance) {
    ExecutableVersion version = {0, 0, 0, 0};

    // Step 1: Get the full file path from HINSTANCE
    char filePath[MAX_PATH];
    if (!GetModuleFileName(hInstance, filePath, MAX_PATH)) {
        printf("Error: Failed to get file path.\n");
        return version;
    }

    // Step 2: Get the size of the version information
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSize(filePath, &handle);
    if (size == 0) {
        printf("Error: Failed to get version info size.\n");
        return version;
    }

    // Step 3: Retrieve the version information
    BYTE* versionInfo = new BYTE[size];
    if (!GetFileVersionInfo(filePath, handle, size, versionInfo)) {
        printf("Error: Failed to retrieve version information.\n");
        delete[] versionInfo;
        return version;
    }

    // Step 4: Extract the fixed file info
    VS_FIXEDFILEINFO* fileInfo = NULL;
    UINT len = 0;
    if (VerQueryValue(versionInfo, "\\", (LPVOID*)&fileInfo, &len) && fileInfo) {
        // Populate the version structure
        version.major = HIWORD(fileInfo->dwFileVersionMS);
        version.minor = LOWORD(fileInfo->dwFileVersionMS);
        version.build = HIWORD(fileInfo->dwFileVersionLS);
        version.revision = LOWORD(fileInfo->dwFileVersionLS);
    } else {
        printf("Error: Failed to query version value.\n");
    }

    // Cleanup
    delete[] versionInfo;

    return version;
}


s32 CC Vid_InitDLL(HINSTANCE hInstance, SVideoFunctions* pVidFuncs)
{
    TRACE_ENTRYEXIT;

    HINSTANCE exeHInstance = GetModuleHandle(NULL);
    ExecutableVersion version = GetExecutableVersion(exeHInstance);

    // Output the version using printf
    printf("Executable Version: %d.%d.%d.%d\n", 
           version.major, version.minor, version.build, version.revision);

    if (version.major == 10 && version.minor == 5)
    {
        printf("Found 10.5 version executable\n");
        const FARPROC fnIsDecompBuild = GetProcAddress(GetModuleHandle(NULL), "GetGlobalsRegistry");
        if (!fnIsDecompBuild)
        {
            pInput_ReleaseMouse = (T_Input_ReleaseMouse)0x5D7C70;
        }
        else
        {
            printf("This executable is a decomp build which has the var at a diff address, do not hook\n");
        }
    }

    gHinstance = hInstance;
    gVideoFuncs_100FFF8 = pVidFuncs;

    return 0;
}

static SVidVersion gVersionInfo = {0x3FE7AE14, // TODO: Reverse
                                   "Reimplementation of DMA Video (DX9 implementation). Version 1.8.1"};

SVidVersion* CC Vid_GetVersion()
{
    TRACE_ENTRYEXIT;
    return &gVersionInfo;
}
