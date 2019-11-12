// LiveVideo.h : main header file for the LIVEVIDEO application
//

#if !defined(AFX_LIVEVIDEO_H__70943714_3422_4F6A_B79A_DD84D6EECF17__INCLUDED_)
#define AFX_LIVEVIDEO_H__70943714_3422_4F6A_B79A_DD84D6EECF17__INCLUDED_

#include <afxwin.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoApp:
// See LiveVideo.cpp for the implementation of this class
//

class CLiveVideoApp : public CWinApp
{
public:
	CLiveVideoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLiveVideoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLiveVideoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIVEVIDEO_H__70943714_3422_4F6A_B79A_DD84D6EECF17__INCLUDED_)
