// CaptureVideo.cpp : implementation file
//
//--------------------------------------------------------------------------//
// Copyright DILLIP KUMAR KARA 2004
// You may do whatever you want with this code, as long as you include this
// copyright notice in your implementation files.
// Comments and bug Reports: codeguru_bank@yahoo.com
//--------------------------------------------------------------------------//

#include "stdafx.h"
#include "LiveVideo.h"
#include "m_dshow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCaptureVideo

CCaptureVideo::CCaptureVideo()
{
	// Initialization 
	m_hApp = NULL;
	m_dwGraphRegister = 0;
	nAnalogCount = 0; // Counting Analog devices
	m_pVW = NULL;
	m_pMC = NULL;
	m_pME = NULL;
	m_pGraph = NULL;  // IFilterGraph2 provides AddSourceFileForMoniker()
	m_pCapture = NULL;
	pSrcF = NULL;

	/// For Webcam
	m_pVWMoniker = NULL;
	m_pMCMoniker = NULL;
	m_pMEMoniker = NULL;
	m_pCaptureMoniker = NULL;
	m_pFilterGraph = NULL;

}

CCaptureVideo::~CCaptureVideo()
{
}


BEGIN_MESSAGE_MAP(CCaptureVideo, CWnd)
	//{{AFX_MSG_MAP(CCaptureVideo)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCaptureVideo message handlers
HRESULT CCaptureVideo::InitializeVideo(HWND hWnd)
{
	HRESULT hr = NULL;
	m_hApp = hWnd;
	bDevCheck = FALSE;
	nVSourceCount = 0;
	vType = 0;

	//COM Library Intialization
	if (FAILED(CoInitialize(NULL)))	/*, COINIT_APARTMENTTHREADED)))*/
	{
		AfxMessageBox("CoInitialize Failed!\r\n");
		return hr;
	}

	return hr;
}

void CCaptureVideo::UnIntializeVideo()
{
	if (vType == 1) // If WebCam
		CloseInterfacesMoniker();
	CloseInterfaces();
	CoUninitialize();
	bVideo = FALSE;
}


HRESULT CCaptureVideo::GetInterfaces()
{
	HRESULT hr;

	if (m_pGraph == NULL) {
		// Create the filter graph
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			IID_IGraphBuilder, (void **)&m_pGraph);
		if (FAILED(hr))
			return hr;
	}
	else
		return E_FAIL;

	if (m_pCapture == NULL) {
		// Create the capture graph builder
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
		if (FAILED(hr))
			return hr;
	}
	else
		return E_FAIL;

	// Obtain interfaces for media control and Video Window
	hr = m_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMC);
	if (FAILED(hr))
		return hr;

	hr = m_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVW);
	if (FAILED(hr))
		return hr;

	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_pME);
	if (FAILED(hr))
		return hr;

	// Set the window handle used to process graph events
	hr = m_pME->SetNotifyWindow((OAHWND)m_hApp, WM_GRAPHNOTIFY, 0);

	return hr;
}

HRESULT CCaptureVideo::FindCaptureDevice(IBaseFilter **ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter * pSrc = NULL;
	CComPtr <IMoniker> pMoniker = NULL;
	ULONG cFetched;

	if (!ppSrcFilter)
		return E_POINTER;

	// Create the system device enumerator  
	CComPtr <ICreateDevEnum> pDevEnum = NULL;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
		return hr;
	}

	// Create an enumerator for the video capture devices
	CComPtr <IEnumMoniker> pClassEnum = NULL;

	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
		return hr;
	}

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if (pClassEnum == NULL)
	{
		//       MessageBox(m_hApp,TEXT("No video capture device was detected.\r\n\r\n")
		//                TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
		//                TEXT("to be installed and working properly.  The sample will now close."),
		//                TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
		return E_FAIL;
	}

	// Use the first video capture device on the device list.
	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.

	//To Find the Analog Capture Devices....

	BOOL         Found;
	IPin        *pP = 0;
	IEnumPins   *pins = 0;
	ULONG        n;
	PIN_INFO     pinInfo;
	Found = FALSE;
	IKsPropertySet *pKs = 0;
	GUID guid;
	DWORD dw;
	BOOL fMatch = FALSE;

	//while(S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
	if (S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
	{
		// Bind Moniker to a filter object
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			DisplayMesg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
			return hr;
		}
		if (SUCCEEDED(pSrc->EnumPins(&pins)))
		{
			while (!Found && (S_OK == pins->Next(1, &pP, &n)))
			{
				vType++;
				if (S_OK == pP->QueryPinInfo(&pinInfo))
				{

					if (pinInfo.dir == PINDIR_INPUT)
					{
						// is this pin an ANALOGVIDEOIN input pin?
						if (pP->QueryInterface(IID_IKsPropertySet,
							(void **)&pKs) == S_OK)
						{
							if (pKs->Get(AMPROPSETID_Pin,
								AMPROPERTY_PIN_CATEGORY, NULL, 0,
								&guid, sizeof(GUID), &dw) == S_OK)
							{
								if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
									fMatch = TRUE;
							}
							pKs->Release();
						}

						if (fMatch)
						{
							Found = TRUE;
							bDevCheck = Found;
							break;
						}
					}
					pinInfo.pFilter->Release();
				}
				pP->Release();
			}
			pins->Release();
		}
	}

	// Copy the found filter pointer to the output parameter.
	// Do NOT Release() the reference, since it will still be used
	// by the calling function.
	pSrcF = pSrc;
	*ppSrcFilter = pSrc;

	return hr;
}

HRESULT CCaptureVideo::SetupVideoWindow()
{
	HRESULT hr;

	// Set the video window to be a child of the main window
	hr = m_pVW->put_Owner((OAHWND)m_hApp);
	if (FAILED(hr))
		return hr;

	// Set video window style
	hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr))
		return hr;

	hr = m_pVW->put_Width(320);
	hr = m_pVW->put_Height(240);

	// Use helper function to position video window in client rect 
	// of main application window
	ResizeVideoWindow();

	// Make the video window visible, now that it is properly positioned
	hr = m_pVW->put_Visible(OATRUE);
	if (FAILED(hr))
		return hr;
	hr = m_pVW->put_MessageDrain((OAHWND)m_hApp);

	return hr;
}

HRESULT CCaptureVideo::ChangePreviewState(int nShow)
{
	HRESULT hr = S_OK;

	// If the media control interface isn't ready, don't call it
	if (!m_pMC)
		return S_OK;

	if (nShow)
	{
		if (m_psCurrent != RUNNING)
		{
			// Start previewing video data
			hr = m_pMC->Run();
			m_psCurrent = RUNNING;
		}
	}
	else
	{
		// Stop previewing video data
		hr = m_pMC->StopWhenReady();
		m_psCurrent = STOPPED;
	}

	return hr;
}

HRESULT CCaptureVideo::HandleGraphEvent()
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr = S_OK;

	if (!m_pME)
		return E_POINTER;

	while (SUCCEEDED(m_pME->GetEvent(&evCode, (LONG_PTR *)&evParam1,
		(LONG_PTR *)&evParam2, 0)))
	{
		//
		// Free event parameters to prevent memory leaks associated with
		// event parameter data.  While this application is not interested
		// in the received events, applications should always process them.
		//
		hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);
	}

	return hr;
}

HRESULT CCaptureVideo::CaptureVideo()
{
	HRESULT hr;
	IBaseFilter *pSrcFilter = NULL;

	// Get DirectShow interfaces
	hr = GetInterfaces();
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = m_pCapture->SetFiltergraph(m_pGraph);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
		return hr;
	}

	// Use the system device enumerator and class enumerator to find
	// a video capture/preview device, such as a desktop USB video camera.

	hr = FindCaptureDevice(&pSrcFilter);
	if (FAILED(hr))
	{
		// Don't display a message because FindCaptureDevice will handle it
		return hr;
	}

	if (nAnalogCount == 0)
		AfxMessageBox("Capture device is not detected.!");

	if (bDevCheck == FALSE)
	{
		return E_FAIL;
	}


	// Add Capture filter to our graph.
	hr = m_pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
			TEXT("If you have a working video capture device, please make sure\r\n")
			TEXT("that it is connected and is not being used by another application.\r\n\r\n"), hr);
		pSrcFilter->Release();
		return hr;
	}

	// Render the preview pin on the video capture filter
	// Use this instead of m_pGraph->RenderFile
	hr = m_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
		pSrcFilter, NULL, NULL);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
			TEXT("The capture device may already be in use by another application.\r\n\r\n")
			TEXT("The sample will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}

	// Now that the filter has been added to the graph and we have
	// rendered its stream, we can release this reference to the filter.
	pSrcFilter->Release();

	// Set video window style and position
	hr = SetupVideoWindow();
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
		return hr;
	}

#ifdef REGISTER_FILTERGRAPH
	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
	hr = AddGraphToRot(m_pGraph, &m_dwGraphRegister);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		m_dwGraphRegister = 0;
	}
#endif

	// Start previewing video data
	hr = m_pMC->Run();
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
		return hr;
	}

	// Remember current state
	m_psCurrent = RUNNING;

	return S_OK;
}

HRESULT CCaptureVideo::AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	WCHAR wsz[128];
	HRESULT hr;

	if (!pUnkGraph || !pdwRegister)
		return E_POINTER;

	if (FAILED(GetRunningObjectTable(0, &pROT)))
		return E_FAIL;

	wsprintfW(wsz, L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph,
		GetCurrentProcessId());

	hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr))
	{
		// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
		// to the object.  Using this flag will cause the object to remain
		// registered until it is explicitly revoked with the Revoke() method.
		//
		// Not using this flag means that if GraphEdit remotely connects
		// to this graph and then GraphEdit exits, this object registration 
		// will be deleted, causing future attempts by GraphEdit to fail until
		// this application is restarted or until the graph is registered again.
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pMoniker, pdwRegister);
		pMoniker->Release();
	}

	pROT->Release();
	return hr;
}

void CCaptureVideo::RemoveGraphFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;

	if (SUCCEEDED(GetRunningObjectTable(0, &pROT)))
	{
		pROT->Revoke(pdwRegister);
		pROT->Release();
	}
}

void CCaptureVideo::CloseInterfaces()
{
	// Stop previewing data
	if (m_pMC)
		m_pMC->StopWhenReady();

	m_psCurrent = STOPPED;

	// Stop receiving events
	if (m_pME)
		m_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Relinquish ownership (IMPORTANT!) of the video window.
	// Failing to call put_Owner can lead to assert failures within
	// the video renderer, as it still assumes that it has a valid
	// parent window.
	if (m_pVW)
	{
		m_pVW->put_Visible(OAFALSE);
		m_pVW->put_Owner(NULL);
	}

#ifdef REGISTER_FILTERGRAPH
	// Remove filter graph from the running object table   
	if (m_dwGraphRegister)
		RemoveGraphFromRot(m_dwGraphRegister);
#endif

	// Release DirectShow interfaces
	SAFE_RELEASE(m_pMC);
	SAFE_RELEASE(m_pME);
	SAFE_RELEASE(m_pVW);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pCapture);
}

void CCaptureVideo::CloseInterfacesMoniker()
{
	// Stop previewing data
	if (m_pMCMoniker)
		m_pMCMoniker->StopWhenReady();

	// Stop receiving events
	if (m_pMEMoniker)
		m_pMEMoniker->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	if (m_pVWMoniker)
	{
		m_pVWMoniker->put_Visible(OAFALSE);
		m_pVWMoniker->put_Owner(NULL);
	}

#ifdef REGISTER_FILTERGRAPH
	// Remove filter graph from the running object table   
	if (m_dwGraphRegister)
		RemoveGraphFromRotMoniker(m_dwGraphRegister);
#endif

	// Release DirectShow interfaces
	SAFE_RELEASE(m_pMCMoniker);
	SAFE_RELEASE(m_pMEMoniker);
	SAFE_RELEASE(m_pVWMoniker);
	SAFE_RELEASE(m_pCaptureMoniker);
	SAFE_RELEASE(m_pFilterGraph);
}

LRESULT CCaptureVideo::ClearInterfaces(WPARAM wp, LPARAM lp)
{
	if (vType == 1) // If WebCam
		CloseInterfacesMoniker();
	CloseInterfaces();
	bVideo = FALSE;
	return 0L;
}

HRESULT CCaptureVideo::GetInterfacesMoniker()
{
	HRESULT hr;

	if (m_pFilterGraph == NULL)
	{
		// Create the filter graph
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			IID_IFilterGraph2, (void **)&m_pFilterGraph);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		return E_FAIL;
	}

	if (m_pCaptureMoniker == NULL)
	{
		// Create the capture graph builder
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **)&m_pCaptureMoniker);
		if (FAILED(hr))
			return hr;
	}
	else
		return E_FAIL;

	// Obtain interfaces for media control and Video Window
	hr = m_pFilterGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMCMoniker);
	if (FAILED(hr))
		return hr;

	hr = m_pFilterGraph->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVWMoniker);
	if (FAILED(hr))
		return hr;

	hr = m_pFilterGraph->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_pMEMoniker);
	if (FAILED(hr))
		return hr;

	// Set the window handle used to process graph events
	hr = m_pMEMoniker->SetNotifyWindow((OAHWND)m_hApp, WM_GRAPHNOTIFY, 0);

	return hr;
}

HRESULT CCaptureVideo::CaptureVideoByMoniker()
{
	HRESULT hr;
	IMoniker *pMoniker = NULL;

	// Get DirectShow interfaces
	hr = GetInterfacesMoniker();
	if (FAILED(hr))
	{
		//Monikor Message
		DisplayMesg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
		return hr;
	}

	// Use the system device enumerator and class enumerator to find
	// a moniker that represents a video capture/preview device, 
	// such as a desktop USB video camera.
	hr = FindCaptureDeviceMoniker(&pMoniker);
	if (FAILED(hr))
	{
		//AfxMessageBox("Webcam is not detected.!") ;
		return hr;
	}

	if (nAnalogCount == nVSourceCount)
	{
		AfxMessageBox("Webcam(USB) is not detected.!");
		return hr;
	}

	hr = AddCaptureMonikerToGraph(pMoniker);

	if (FAILED(hr))
	{
		/*      DisplayMesg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
				  TEXT("If you have a working video capture device, please make sure\r\n")
				  TEXT("that it is connected and is not being used by another application.\r\n\r\n")
				  TEXT("The sample will now close."), hr);*/
		return hr;
	}

	// Release the IMoniker interface for the capture source filter
	pMoniker->Release();

	// Set video window style and position
	hr = SetupVideoWindowMoniker();
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
		return hr;
	}

#ifdef REGISTER_FILTERGRAPH
	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
	hr = AddGraphToRotMoniker(m_pFilterGraph, &m_dwGraphRegister);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		m_dwGraphRegister = 0;
	}
#endif

	// Start previewing video data
	hr = m_pMCMoniker->Run();
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
		return hr;
	}
	return S_OK;
}

HRESULT CCaptureVideo::AddCaptureMonikerToGraph(IMoniker *pMoniker)
{
	USES_CONVERSION;

	HRESULT hr;
	IBaseFilter *pBaseFilter = 0;

	// Get the display name of the moniker
	LPOLESTR strMonikerName = 0;
	hr = pMoniker->GetDisplayName(NULL, NULL, &strMonikerName);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't get moniker's display name!  hr=0x%x"), hr);
		return hr;
	}

#ifdef DEBUG
	// Get a human-readable string for evaluation during debugging
	TCHAR szMonikerName[256];
	_tcsncpy(szMonikerName, W2T(strMonikerName), 255);
	szMonikerName[255] = 0;     // Null-terminate
	 //DisplayMesg(TEXT("Moniker: %s\r\n"), szMonikerName);
#endif

	// Create a bind context needed for working with the moniker
	IBindCtx *pContext = 0;
	hr = CreateBindCtx(0, &pContext);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't create a bind context for moniker!  hr=0x%x"), hr);
		return hr;
	}

	hr = m_pFilterGraph->AddSourceFilterForMoniker(pMoniker, pContext,
		strMonikerName, &pBaseFilter);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed in AddSourceFilterForMoniker()!  hr=0x%x"), hr);
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = m_pCaptureMoniker->SetFiltergraph(m_pFilterGraph);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
		return hr;
	}
	// Render the preview pin on the video capture filter
	// Use this instead of m_pGraph->RenderFile
	hr = m_pCaptureMoniker->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
		pBaseFilter, NULL, NULL);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't render capture stream.  ")
			TEXT("The device may already be in use.\r\n\r\nhr=0x%x"), hr);

		return hr;
	}

	SAFE_RELEASE(pContext);
	SAFE_RELEASE(pBaseFilter);

	return hr;
}

HRESULT CCaptureVideo::FindCaptureDeviceMoniker(IMoniker **ppMoniker)
{
	HRESULT hr;
	ULONG cFetched;

	if (!ppMoniker)
		return E_POINTER;

	// Create the system device enumerator
	CComPtr <ICreateDevEnum> pDevEnum = NULL;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
		return hr;
	}

	// Create an enumerator for the video capture devices
	CComPtr <IEnumMoniker> pClassEnum = NULL;

	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		DisplayMesg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
		return hr;
	}

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if (pClassEnum == NULL)
	{
		AfxMessageBox("No video capture device was detected.", MB_OK | MB_ICONINFORMATION);
		return E_FAIL;
	}

	// Use the first video capture device on the device list.
	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.

	/// For finding Digital Capture Devices ...

	BOOL         Found;
	IPin        *pP = 0;
	IEnumPins   *pins = 0;
	ULONG        n;
	PIN_INFO     pinInfo;
	Found = FALSE;
	IKsPropertySet *pKs = 0;
	GUID guid;
	DWORD dw;
	BOOL fMatch = FALSE;
	IMoniker *pMoniker = NULL;
	IBaseFilter *pSrc;

	while (S_OK == (pClassEnum->Next(1, ppMoniker, &cFetched)))
	{
		pMoniker = ppMoniker[0];

		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			DisplayMesg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
			return hr;
		}
		else
			pSrcF = pSrc;

		if (SUCCEEDED(pSrc->EnumPins(&pins)))
		{
			while (!Found && (S_OK == pins->Next(1, &pP, &n)))
			{
				if (S_OK == pP->QueryPinInfo(&pinInfo))
				{
					if (pinInfo.dir == PINDIR_INPUT)
					{
						// is this pin an ANALOGVIDEOIN input pin?
						if (pP->QueryInterface(IID_IKsPropertySet,
							(void **)&pKs) == S_OK)
						{
							if (pKs->Get(AMPROPSETID_Pin,
								AMPROPERTY_PIN_CATEGORY, NULL, 0,
								&guid, sizeof(GUID), &dw) == S_OK)
							{
								if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
									fMatch = TRUE;
							}
							pKs->Release();
						}

						if (fMatch)
						{
							Found = TRUE;
							bDevCheck = Found;
							nAnalogCount++;
							break;
						}

					}
					pinInfo.pFilter->Release();
				}
				pP->Release();
			}
			pins->Release();
		}

		nVSourceCount++;

	} // End of While Loop

	return S_OK;;
}

HRESULT CCaptureVideo::HandleGraphEventMoniker()
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr = S_OK;

	if (!m_pMEMoniker)
		return E_POINTER;

	// Process all queued events
	while (SUCCEEDED(m_pMEMoniker->GetEvent(&evCode, (LONG_PTR *)&evParam1,
		(LONG_PTR *)&evParam2, 0)))
	{
		// Insert event processing code here, if desired
		switch (evCode)
		{
			// When the user closes the capture window, close the app.
		case EC_DEVICE_LOST:
		case EC_ERRORABORT:
		case EC_USERABORT:
			DisplayMesg(TEXT("Received an error event: 0x%x.  Closing app.\0"), evCode);
			::PostMessage(m_hApp, WM_CLOSE, 0, 0);
			break;
		}
		hr = m_pMEMoniker->FreeEventParams(evCode, evParam1, evParam2);
	}

	return hr;
}

HRESULT CCaptureVideo::SetupVideoWindowMoniker()
{
	HRESULT hr;

	// Set the video window to be a child of the main window
	hr = m_pVWMoniker->put_Owner((OAHWND)m_hApp);
	if (FAILED(hr))
		return hr;

	// Set video window style
	hr = m_pVWMoniker->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr))
		return hr;

	ResizeVideoWindowMoniker();

	// Make the video window visible, now that it is properly positioned
	hr = m_pVWMoniker->put_Visible(OATRUE);
	if (FAILED(hr))
		return hr;

	hr = m_pVWMoniker->put_MessageDrain((OAHWND)m_hApp);

	return hr;
}

HRESULT CCaptureVideo::AddGraphToRotMoniker(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	WCHAR wsz[128];
	HRESULT hr;

	if (!pUnkGraph || !pdwRegister)
		return E_POINTER;

	if (FAILED(GetRunningObjectTable(0, &pROT)))
		return E_FAIL;

	wsprintfW(wsz, L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph,
		GetCurrentProcessId());

	hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr))
	{
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pMoniker, pdwRegister);
		pMoniker->Release();
	}

	pROT->Release();
	return hr;
}

void CCaptureVideo::RemoveGraphFromRotMoniker(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;

	if (SUCCEEDED(GetRunningObjectTable(0, &pROT)))
	{
		pROT->Revoke(pdwRegister);
		pROT->Release();
	}
}

HRESULT CCaptureVideo::OnGraphNotify(WPARAM wp, LPARAM lp)
{
	HandleGraphEvent();
	return 0;
}

void CCaptureVideo::DisplayMesg(TCHAR *szFormat, ...)
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

	::MessageBox(NULL, szBuffer, TEXT("LiveVideo Message"), MB_OK | MB_ICONERROR);

}

void CCaptureVideo::StartSVideo()
{
	if (vType == 1) // If WebCam
		CloseInterfacesMoniker();
	if (m_pCapture == NULL)
	{
		HRESULT hr = CaptureVideo();
		if (FAILED(hr))
		{
			CloseInterfaces();
			return;
		}
	}

	vType = 0; // Initializing current capture device to Video

	IAMCrossbar *pX;
	//unsigned short a =10;
	//MessageBox(NULL,&a,&a, 1);

	if (m_pCapture == NULL)
		return;

	if (pSrcF == NULL)
		return;

	HRESULT hr;
	hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Interleaved, pSrcF,
		IID_IAMCrossbar, (void **)&pX);
	if (hr != NOERROR)
		hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, pSrcF,
			IID_IAMCrossbar, (void **)&pX);

	LONG lInpin, lOutpin;
	hr = pX->get_PinCounts(&lOutpin, &lInpin);
	//CString str; str.Format("%ld    %ld",l2,l1);
	//MessageBox(str);

	BOOL IPin = TRUE; LONG pIndex = 0, pRIndex = 0, pType = 0;

	while (pIndex < lInpin)
	{
		hr = pX->get_CrossbarPinInfo(IPin, pIndex, &pRIndex, &pType);

		/*if( pType == PhysConn_Video_Tuner)
				break;
				pIndex++;*/
		if (pType == PhysConn_Video_SVideo)
			break;
		pIndex++;
	}

	BOOL OPin = FALSE; LONG pOIndex = 0, pORIndex = 0, pOType = 0;
	while (pOIndex < lOutpin)
	{
		hr = pX->get_CrossbarPinInfo(OPin, pOIndex, &pORIndex, &pOType);
		if (pOType == PhysConn_Video_VideoDecoder)
			break;
	}

	pX->Route(pOIndex, pIndex);

	pX->Release();
}

void CCaptureVideo::StartCompositeVideo()
{
	if (vType == 1)	//If Webcam
		CloseInterfacesMoniker();

	if (m_pCapture == NULL)
	{
		HRESULT hr = CaptureVideo();
		if (FAILED(hr))
		{
			CloseInterfaces();
			return;
		}
	}

	vType = 0; // Initializing current capture device to Video

	IAMCrossbar *pX;

	if (m_pCapture == NULL)
		return;

	if (pSrcF == NULL)
		return;

	HRESULT hr;
	hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Interleaved, pSrcF,
		IID_IAMCrossbar, (void **)&pX);
	if (hr != NOERROR)
		hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, pSrcF,
			IID_IAMCrossbar, (void **)&pX);

	LONG lInpin, lOutpin;
	hr = pX->get_PinCounts(&lOutpin, &lInpin);
	//CString str; str.Format("%ld    %ld",l2,l1);
	//MessageBox(str);

	BOOL IPin = TRUE; LONG pIndex = 0, pRIndex = 0, pType = 0;

	while (pIndex < lInpin)
	{
		hr = pX->get_CrossbarPinInfo(IPin, pIndex, &pRIndex, &pType);

		if (pType == PhysConn_Video_Composite)
			break;
		pIndex++;
	}

	BOOL OPin = FALSE; LONG pOIndex = 0, pORIndex = 0, pOType = 0;
	while (pOIndex < lOutpin)
	{
		hr = pX->get_CrossbarPinInfo(OPin, pOIndex, &pORIndex, &pOType);
		if (pOType == PhysConn_Video_VideoDecoder)
			break;
	}

	pX->Route(pOIndex, pIndex);

	pX->Release();
}

void CCaptureVideo::StartWebcam()
{
	// If Previous mode is Video then Close all interfaces
	if (vType == 0)
		CloseInterfaces();
	vType = 1; // Initializing current capture device to WebCam

	// Initializing Interfaces and Capture using WebCam
	HRESULT hr = CaptureVideoByMoniker();
	if (FAILED(hr))
	{
		//AfxMessageBox("No Webcam Found.!") ;
		return;
	}
}

void CCaptureVideo::StartTVTuner()
{
	if (vType == 1)	//If Webcam
		CloseInterfacesMoniker();

	if (m_pCapture == NULL)
	{
		HRESULT hr = CaptureVideo();
		if (FAILED(hr))
		{
			CloseInterfaces();
			return;
		}
	}

	vType = 0; // Initializing current capture device to Video
	IAMCrossbar *pX;

	if (m_pCapture == NULL)
		return;

	if (pSrcF == NULL)
		return;

	HRESULT hr;
	hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Interleaved, pSrcF,
		IID_IAMCrossbar, (void **)&pX);
	if (hr != NOERROR)
		hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, pSrcF,
			IID_IAMCrossbar, (void **)&pX);

	LONG lInpin, lOutpin;
	hr = pX->get_PinCounts(&lOutpin, &lInpin);

	BOOL IPin = TRUE; LONG pIndex = 0, pRIndex = 0, pType = 0;

	while (pIndex < lInpin)
	{
		hr = pX->get_CrossbarPinInfo(IPin, pIndex, &pRIndex, &pType);

		if (pType == PhysConn_Video_Tuner)
			break;
		pIndex++;
	}

	BOOL OPin = FALSE; LONG pOIndex = 0, pORIndex = 0, pOType = 0;
	while (pOIndex < lOutpin)
	{
		hr = pX->get_CrossbarPinInfo(OPin, pOIndex, &pORIndex, &pOType);
		if (pOType == PhysConn_Video_VideoDecoder)
			break;
	}

	pX->Route(pOIndex, pIndex);


	pX->Release();
}

void CCaptureVideo::StopCapture()
{
	if (vType == 1) // If WebCam
		CloseInterfacesMoniker();
	CloseInterfaces();
	bVideo = FALSE;
	//CoUninitialize();
}

void CCaptureVideo::ResizeVideoWindowMoniker()
{
	// Resize the video preview window to match owner window size
	if (m_pVWMoniker)
	{
		CRect rc;
		// Make the preview video fill our window
		::GetClientRect(m_hApp, &rc);
		m_pVWMoniker->SetWindowPosition(0, 0, rc.right, rc.bottom);
	}
}

void CCaptureVideo::ResizeVideoWindow()
{
	// Resize the video preview window to match owner window size
	if (m_pVW)
	{
		RECT rc;

		// Make the preview video fill our window
		::GetClientRect(m_hApp, &rc);
		m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
	}
}
