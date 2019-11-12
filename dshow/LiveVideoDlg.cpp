// LiveVideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LiveVideo.h"
#include "LiveVideoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoDlg dialog

CLiveVideoDlg::CLiveVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLiveVideoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLiveVideoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLiveVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLiveVideoDlg)
	DDX_Control(pDX, IDC_STATIC_SCREEN, m_staticScreen);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLiveVideoDlg, CDialog)
	//{{AFX_MSG_MAP(CLiveVideoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED,IDC_RADIO1,IDC_RADIO4,OnSelectVideo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveVideoDlg message handlers

BOOL CLiveVideoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_btnSVideo = (CButton*)GetDlgItem(IDC_RADIO1) ;
	m_btnComposite = (CButton*)GetDlgItem(IDC_RADIO2) ;
	m_btnTVTuner = (CButton*)GetDlgItem(IDC_RADIO3) ;
	m_btnWebcam = (CButton*)GetDlgItem(IDC_RADIO4) ;
	
	m_btnWebcam->SetCheck(1) ;

	//Retrive the window Handle of the Video Screen
	HWND hWnd = m_staticScreen.GetSafeHwnd() ;	
	
	// Video Intialization 
	HRESULT hr = capVideo.InitializeVideo(hWnd) ;

	if(FAILED(hr))
		AfxMessageBox("Couldn't capture Video.!") ;
	else
		capVideo.StartWebcam() ;  // Set Webcam as Default

	CenterWindow() ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLiveVideoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLiveVideoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLiveVideoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



BOOL CLiveVideoDlg::PreTranslateMessage(MSG* pMsg) 
{
	if((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
    {
        // Escape key was hit: do whatever you want
        return TRUE;
    }
	if((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
    {
        // Enter key was hit: do whatever you want
        return TRUE;
    }	
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CLiveVideoDlg::OnSelectVideo(UINT nID)
{
	ASSERT(nID >= IDC_RADIO1 && nID <= IDC_RADIO4) ;
	int nVideo = nID - IDC_RADIO1 ;

	switch(nVideo)
	{
	case 0:// S-Video
		capVideo.StartSVideo() ;
		break ;
	case 1:// Composite Video
		capVideo.StartCompositeVideo() ;
		break ;
	case 2:// TV Tuner
		capVideo.StartTVTuner() ;
		break ;
	case 3:// Webcam 
		capVideo.StartWebcam() ;
		break ;
	}
}

void CLiveVideoDlg::OnCancel() 
{
	// UnIntialize the Video
	capVideo.UnIntializeVideo() ;	
	CDialog::OnCancel();
}

void CLiveVideoDlg::OnStop() 
{
	capVideo.StopCapture() ;	
	m_btnSVideo->SetCheck(0) ;
	m_btnComposite->SetCheck(0) ;
	m_btnTVTuner->SetCheck(0) ;
	m_btnWebcam->SetCheck(0) ;
}

void CLiveVideoDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	// UnIntialize the Video
	capVideo.UnIntializeVideo() ;	
	
}
