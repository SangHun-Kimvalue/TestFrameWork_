#include "stdafx.h"
#include "MF_Capture.h"


MF_Capture::MF_Capture()
{
}


MF_Capture::~MF_Capture()
{
}


HRESULT CreateVideoCaptureDevice(IMFMediaSource **ppSource)
{
	*ppSource = NULL;

	UINT32 count = 0;

	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;
	// Create an attribute store to hold the search criteria.
	HRESULT hr = MFCreateAttributes(&pConfig, 1);

	// Request video capture devices.
	if (SUCCEEDED(hr))
	{
		hr = pConfig->SetGUID(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
		);
	}

	// Enumerate the devices,
	if (SUCCEEDED(hr))
	{
		hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
	}

	// Create a media source for the first device in the list.
	if (SUCCEEDED(hr))
	{
		if (count > 0)
		{
			hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(ppSource));
		}
		else
		{
			hr = NULL;
		}
	}

	for (DWORD i = 0; i < count; i++)
	{
		ppDevices[i]->Release();
	}
	CoTaskMemFree(ppDevices);
	return hr;
}

void DebugShowDeviceNames(IMFActivate **ppDevices, UINT count)
{
	for (DWORD i = 0; i < count; i++)
	{
		HRESULT hr = S_OK;
		WCHAR *szFriendlyName = NULL;

		// Try to get the display name.
		UINT32 cchName;
		hr = ppDevices[i]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			&szFriendlyName, &cchName);

		if (SUCCEEDED(hr))
		{
			OutputDebugString(szFriendlyName);
			OutputDebugString(L"\n");
		}
		CoTaskMemFree(szFriendlyName);
	}
}


HRESULT CreateAudioCaptureDevice(PCWSTR *pszEndPointID, IMFMediaSource **ppSource)
{
	*ppSource = NULL;

	IMFAttributes *pAttributes = NULL;
	IMFMediaSource *pSource = NULL;

	HRESULT hr = MFCreateAttributes(&pAttributes, 2);

	// Set the device type to audio.
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetGUID(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID
		);
	}

	// Set the endpoint ID.
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetString(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID,
			(LPCWSTR)pszEndPointID
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = MFCreateDeviceSource(pAttributes, ppSource);
	}

	SafeRelease(&pAttributes);
	return hr;
}