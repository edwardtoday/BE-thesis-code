#include "sampvid.h"
#include <initguid.h>
#if(1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

// GUIDs

DEFINE_GUID(CLSID_SampleRenderer,
0x4d4b1600, 0x33ac, 0x11cf, 0xbf, 0x30, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);

DEFINE_GUID(CLSID_SampleQuality,
0xdb76d7f0, 0x97cc, 0x11cf, 0xa0, 0x96, 0x00, 0x80, 0x5f, 0x6c, 0xab, 0x82);


LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE9      g_pTexture   = NULL; // Our texture
HINSTANCE               hInstance    = 0;
HWND                    g_hWnd       = 0;
HRESULT					hr = 0;

D3DFORMAT               g_TextureFormat;

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,           // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
    L"Input",                   // Name of the pin
    FALSE,                      // Is pin rendered
    FALSE,                      // Is an output pin
    FALSE,                      // Ok for no pins
    FALSE,                      // Allowed many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of pin types
    &sudPinTypes                // Details for pins
};

const AMOVIESETUP_FILTER sudSampVid =
{
    &CLSID_SampleRenderer,      // Filter CLSID
    L"Sample Video Renderer",   // Filter name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
    { L"Sample Video Renderer"
    , &CLSID_SampleRenderer
    , CVideoRenderer::CreateInstance
    , NULL
    , &sudSampVid }
  ,
    { L"Quality Property Page"
    , &CLSID_SampleQuality
    , CQualityProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// CreateInstance
//
// This goes in the factory template table to create new filter instances
//
CUnknown * WINAPI CVideoRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CVideoRenderer(NAME("Sample Video Renderer"),pUnk,phr);

} // CreateInstance


#pragma warning(disable:4355)

//
// Constructor
//
CVideoRenderer::CVideoRenderer(TCHAR *pName,
                               LPUNKNOWN pUnk,
                               HRESULT *phr) :

    CBaseVideoRenderer(CLSID_SampleRenderer,pName,pUnk,phr),
    m_InputPin(NAME("Video Pin"),this,&m_InterfaceLock,phr,L"Input"),
    m_ImageAllocator(this,NAME("Sample video allocator"),phr),
    m_VideoText(NAME("Video properties"),GetOwner(),phr,&m_InterfaceLock,this),
    m_ImagePalette(this,&m_VideoText,&m_DrawImage),
    m_DrawImage(&m_VideoText)
{
    // Store the video input pin
    m_pInputPin = &m_InputPin;

    // Reset the current video size

    m_VideoSize.cx = 0;
    m_VideoSize.cy = 0;

	 m_lVidWidth = 320;
	 m_lVidHeight = 240;
	 m_lVidPitch = 20;
    // Initialise the window and control interfaces

    m_VideoText.SetControlVideoPin(&m_InputPin);
    m_VideoText.SetControlWindowPin(&m_InputPin);

} // (Constructor)


//
// Destructor
//
CVideoRenderer::~CVideoRenderer()
{
    m_pInputPin = NULL;

} // (Destructor)


//
// CheckMediaType
//
// Check the proposed video media type
//
HRESULT CVideoRenderer::CheckMediaType(const CMediaType *pmtIn)
{
    return m_Display.CheckMediaType(pmtIn);

} // CheckMediaType


//
// GetPin
//
// We only support one input pin and it is numbered zero
//
CBasePin *CVideoRenderer::GetPin(int n)
{
    ASSERT(n == 0);
    if (n != 0) {
        return NULL;
    }

    // Assign the input pin if not already done so

    if (m_pInputPin == NULL) {
        m_pInputPin = &m_InputPin;
    }

    return m_pInputPin;

} // GetPin


//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP CVideoRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);

    } else if (riid == IID_IVideoWindow) {
        return m_VideoText.NonDelegatingQueryInterface(riid,ppv);

    } else if (riid == IID_IBasicVideo) {
        return m_VideoText.NonDelegatingQueryInterface(riid,ppv);
    }

    return CBaseVideoRenderer::NonDelegatingQueryInterface(riid,ppv);

} // NonDelegatingQueryInterface


//
// GetPages
//
// Return the CLSIDs for the property pages we support
//
STDMETHODIMP CVideoRenderer::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    pPages->pElems[0] = CLSID_SampleQuality;
    return NOERROR;

} // GetPages


//
// DoRenderSample
//
// Have the drawing object render the current image
//
HRESULT CVideoRenderer::DoRenderSample(IMediaSample *pSample)
{
	   BYTE  *pBmpBuffer, *pTxtBuffer; // Bitmap buffer, texture buffer
    LONG  lTxtPitch;                // Pitch of bitmap, texture

    BYTE  * pbS = NULL;
    DWORD * pdwS = NULL;
    DWORD * pdwD = NULL;
    UINT row, col, dwordWidth;
    
    CheckPointer(pSample,E_POINTER);
    CheckPointer(g_pTexture,E_UNEXPECTED);

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    if (FAILED(g_pTexture->LockRect(0, &d3dlr, 0, 0)))
        return E_FAIL;
    
    // Get the texture buffer & pitch
    pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    lTxtPitch = d3dlr.Pitch;
    
	pdwS = ( DWORD*)pBmpBuffer;
    pdwD = ( DWORD*)pTxtBuffer;
    dwordWidth = m_lVidWidth / 4;

	 for( row = 0; row< (UINT)m_lVidHeight; row++)
     {
			for( col = 0; col < dwordWidth; col ++ )
				 pdwD[col] =  0;
	 }
    // Copy the bits    

    //if (1) 
    //{
    //    // Instead of copying data bytewise, we use DWORD alignment here.
    //    // We also unroll loop by copying 4 pixels at once.
    //    //
    //    // original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
    //    //
    //    // aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
    //    //
    //    // We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
    //    // below, bitwise operations do exactly this.

    //    dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
    //                                  // (pixel by 3 bytes over sizeof(DWORD))

    //    for( row = 0; row< (UINT)m_lVidHeight; row++)
    //    {
    //        pdwS = ( DWORD*)pBmpBuffer;
    //        pdwD = ( DWORD*)pTxtBuffer;

    //        for( col = 0; col < dwordWidth; col ++ )
    //        {
    //            pdwD[0] =  pdwS[0] | 0xFF000000;
    //            pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
    //            pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
    //            pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
    //            pdwD +=4;
    //            pdwS +=3;
    //        }

    //        // we might have remaining (misaligned) bytes here
    //        pbS = (BYTE*) pdwS;
    //        for( col = 0; col < (UINT)m_lVidWidth % 4; col++)
    //        {
    //            *pdwD = 0xFF000000     |
    //                    (pbS[2] << 16) |
    //                    (pbS[1] <<  8) |
    //                    (pbS[0]);
    //            pdwD++;
    //            pbS += 3;           
    //        }

    //        pBmpBuffer  += m_lVidPitch;
    //        pTxtBuffer += lTxtPitch;
    //    }// for rows
    //}


    // Unlock the Texture
    if (FAILED(g_pTexture->UnlockRect(0)))
        return E_FAIL;
    
    return S_OK;
	//return m_DrawImage.DrawImage(pMediaSample);

} // DoRenderSample


//
// PrepareRender
//
// Overriden to realise the palette before drawing. We used to have to realise
// the palette on every frame because we could never be sure of receiving top
// level messages like WM_PALETTECHANGED. However the plug in distributor will
// now make sure we get these but we still have to do this because otherwise
// we may not find the palette being realised before the thread comes to draw

void CVideoRenderer::PrepareRender()
{
    // Realise the palette on this thread
    m_VideoText.DoRealisePalette();

} // PrepareRender


//
// Active
//
// The auto show flag is used to have the window shown automatically when we
// change state. We do this only when moving to paused or running, when there
// is no outstanding EC_USERABORT set and when the window is not already up
// This can be changed through the IVideoWindow interface AutoShow property.
// If the window is not currently visible then we are showing it because of
// a state change to paused or running, in which case there is no point in
// the video window sending an EC_REPAINT as we're getting an image anyway
//
HRESULT CVideoRenderer::Active()
{
    HWND hwnd = m_VideoText.GetWindowHWND();
    NOTE("AutoShowWindow");

    if(m_VideoText.IsAutoShowEnabled() == TRUE)
    {
        if(m_bAbort == FALSE)
        {
            if(IsWindowVisible(hwnd) == FALSE)
            {
                NOTE("Executing AutoShowWindow");
                SetRepaintStatus(FALSE);

                m_VideoText.PerformanceAlignWindow();
                m_VideoText.DoShowWindow(SW_SHOWNORMAL);
                m_VideoText.DoSetWindowForeground(TRUE);
            }
        }
    }

    return CBaseVideoRenderer::Active();

} // Active


//
// SetMediaType
//
// We store a copy of the media type used for the connection in the renderer
// because it is required by many different parts of the running renderer
// This can be called when we come to draw a media sample that has a format
// change with it. We normally delay type changes until they are really due
// for rendering otherwise we will change types too early if the source has
// allocated a queue of samples. In our case this isn't a problem because we
// only ever receive one sample at a time so it's safe to change immediately
//
HRESULT CVideoRenderer::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    HRESULT hr = NOERROR;
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    CMediaType StoreFormat(m_mtIn);

    // Fill out the optional fields in the VIDEOINFOHEADER

    m_mtIn = *pmt;
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_mtIn.Format();
    m_Display.UpdateFormat(pVideoInfo);

    // We set the new palette before completing so that the method can look
    // at the old RGB colours we used and compare them with the new set, if
    // they're all identical colours we don't need to change the palette

    hr = m_ImagePalette.PreparePalette(&m_mtIn,&StoreFormat,NULL);
    if (FAILED(hr)) {
        return hr;
    }

    // Complete the initialisation

    m_DrawImage.NotifyMediaType(&m_mtIn);
    m_ImageAllocator.NotifyMediaType(&m_mtIn);
    return NOERROR;

} // SetMediaType


//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and lets us to reset the connection flag held by the base class renderer
// The filter object may be hanging onto an image to use for refreshing the
// video window so that must be freed (the allocator decommit may be waiting
// for that image to return before completing) then we must also uninstall
// any palette we were using, reset anything set with the control interfaces
// then set our overall state back to disconnected ready for the next time

HRESULT CVideoRenderer::BreakConnect()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    // Check we are in a valid state

    HRESULT hr = CBaseVideoRenderer::BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }

    // The window is not used when disconnected
    IPin *pPin = m_InputPin.GetConnected();
    if (pPin) 
        SendNotifyWindow(pPin,NULL);

    // The base class break connect disables us from sending any EC_REPAINT
    // events which is important otherwise when we come down here to remove
    // our palette we end up painting the window again - which in turn sees
    // there is no image to draw and would otherwise send a redundant event

    m_ImagePalette.RemovePalette();
    m_mtIn.ResetFormatBuffer();

    return NOERROR;

} // BreakConnect


//
// CompleteConnect
//
// When we complete connection we need to see if the video has changed sizes
// If it has then we activate the window and reset the source and destination
// rectangles. If the video is the same size then we bomb out early. By doing
// this we make sure that temporary disconnections such as when we go into a
// fullscreen mode do not cause unnecessary property changes. The basic ethos
// is that all properties should be persistent across connections if possible
//
HRESULT CVideoRenderer::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    CBaseVideoRenderer::CompleteConnect(pReceivePin);
    m_DrawImage.ResetPaletteVersion();

    // Has the video size changed between connections

    VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_mtIn.Format();
    if (pVideoInfo->bmiHeader.biWidth == m_VideoSize.cx) 
    {
        if (pVideoInfo->bmiHeader.biHeight == m_VideoSize.cy) 
        {
            return NOERROR;
        }
    }

    // Pass the video window handle upstream
    HWND hwnd = m_VideoText.GetWindowHWND();
    NOTE1("Sending EC_NOTIFY_WINDOW %x",hwnd);

    SendNotifyWindow(pReceivePin,hwnd);

    // Set them for the current video dimensions

    m_DrawImage.SetDrawContext();
    m_VideoSize.cx = pVideoInfo->bmiHeader.biWidth;
    m_VideoSize.cy = pVideoInfo->bmiHeader.biHeight;

    m_VideoText.SetDefaultSourceRect();
    m_VideoText.SetDefaultTargetRect();
    m_VideoText.OnVideoSizeChange();
    m_VideoText.ActivateWindow();

    return NOERROR;

} // CompleteConnect


//
// OnReceiveFirstSample
//
// Use the image just delivered to display a poster frame
//
void CVideoRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);

    DoRenderSample(pMediaSample);

} // OnReceiveFirstSample


// Constructor

CVideoInputPin::CVideoInputPin(TCHAR *pObjectName,
                               CVideoRenderer *pRenderer,
                               CCritSec *pInterfaceLock,
                               HRESULT *phr,
                               LPCWSTR pPinName) :

    CRendererInputPin(pRenderer,phr,pPinName),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pInterfaceLock)
{
    ASSERT(m_pRenderer);
    ASSERT(pInterfaceLock);

} // (Constructor)


//
// GetAllocator
//
// This overrides the CBaseInputPin virtual method to return our allocator
// we create to pass shared memory DIB buffers that GDI can directly access
// When NotifyAllocator is called it sets the current allocator in the base
// input pin class (m_pAllocator), this is what GetAllocator should return
// unless it is NULL in which case we return the allocator we would like
//
STDMETHODIMP CVideoInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    // Has an allocator been set yet in the base class

    if (m_pAllocator == NULL) 
    {
        m_pAllocator = &m_pRenderer->m_ImageAllocator;
        m_pAllocator->AddRef();
    }

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;

    return NOERROR;

} // GetAllocator


//
// NotifyAllocator
//
// The COM specification says any two IUnknown pointers to the same object
// should always match which provides a way for us to see if they are using
// our DIB allocator or not. Since we are only really interested in equality
// and our object always hands out the same IMemAllocator interface we can
// just see if the pointers match. If they are we set a flag in the main
// renderer as the window needs to know whether it can do fast rendering
//
STDMETHODIMP
CVideoInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

    // Make sure the base class gets a look

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
    if (FAILED(hr))
        return hr;

    // Whose allocator is the source going to use?

    m_pRenderer->m_DrawImage.NotifyAllocator(FALSE);
    if (pAllocator == &m_pRenderer->m_ImageAllocator)
        m_pRenderer->m_DrawImage.NotifyAllocator(TRUE);

    return NOERROR;

} // NotifyAllocator


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterSever
//
// Handle the registration of this filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


HRESULT InitD3D(HWND hwnd)
{
    HRESULT hr;

    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Get the current desktop display mode, so we can set up a back
    // buffer of the same format
    D3DDISPLAYMODE d3ddm;

    if ( FAILED( hr = g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
    {
        Msg(TEXT("Could not read adapter display mode!  hr=0x%x"), hr);
        return hr;
    }

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
                               D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                               &d3dpp, &g_pd3dDevice );                                     


    hr = g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
    hr = g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

    // Add filtering
    hr = g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    hr = g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    // set maximum ambient light
    hr = g_pd3dDevice->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255));

    // Turn off culling
    hr = g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    hr = g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn on the zbuffer
    hr = g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Set texture states
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );


    return hr;
}

void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(NULL, szBuffer, TEXT("DirectShow Texture3D Sample"), 
               MB_OK | MB_ICONERROR);
}
