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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__720D769D_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_MAINFRM_H__720D769D_A7E5_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include"ServerList.h"
#include"plugins.h"
#include"BODialogBar.h"
#include"GoodMenuBar.h"
#include"trayicon.h"

typedef struct {
	TYPEOF_ClientMenu *pProc;
	int nId;
} PLUGINMENUITEM;

#include <plugins.h>

typedef struct {
	BYTE *pPluginImage;
	DWORD dwPluginLen;
	char svFilename[MAX_PATH+1];
	char svDescription[256];
	WORD wVersionLo;
	WORD wVersionHi;
	WORD wBOVersionLo;
	WORD wBOVersionHi;
} PLUGIN_INFO;

typedef struct {
	int nPlugin;
	DWORD dwPos;
	char VarType; // B, N, S
	int nNumLo;
	int nNumHi;
	int nStrLen;
	char svVarValue[256];
	char svVarName[256];
	char svCategory[256];
} VARIABLE_INFO;



class CMainFrame : public CCJFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	
//	CTrayIcon *m_pTrayIcon;
//	BOOL m_bSysTray;
	BOOL m_bGradients;
	BOOL m_bScreenReader;
	CArray<PLUGINMENUITEM, PLUGINMENUITEM> m_arrPluginMenuItems;

	
// Attributes
public:
	CServerList *m_pWndServerList;

	CArray<PLUGIN_INFO, PLUGIN_INFO> m_arrPluginInfo;
	CArray<VARIABLE_INFO, VARIABLE_INFO> m_arrVarInfo;
	
// Operations
public:
	int RegisterClientMenu(LPCSTR szCategory, LPCSTR szComName, TYPEOF_ClientMenu *pProc);
	int UnregisterClientMenu(LPCSTR szCategory, LPCSTR szComName);

	int InitializePlugins();
	int TerminatePlugins();
	int AddPlugin(CString strPath);
	int RemovePlugin(int nPlugin);
	void AddVariables(char *pBuffer, DWORD dwLen, int nPlugin);
	int SavePluginConfig(int nNum);
	int LoadPluginConfig(int nNum);

	int InitializeToolbars();
	int InitializeDockingWindows();
	int DoDialogModal(CDialog *dlg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  
	// control bar embedded members
	CCoolMenuManager m_menuManager;
	CGoodMenuBar m_menuBar;
	CCJToolBar m_wndTBMain;
	CStatusBar  m_wndStatusBar;
	CBODialogBar m_wndServerListBDB;

	
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnServerlist();
	afx_msg void OnUpdateServerlist(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPlugins();
	afx_msg void OnGradients();
	afx_msg void OnUpdateGradients(CCmdUI* pCmdUI);
	afx_msg	void OnMenuType();
	afx_msg void OnUpdateMenuType(CCmdUI* pCmdUI);
	afx_msg void OnServerNew();
	afx_msg void OnUpdateServerNew(CCmdUI* pCmdUI);
	afx_msg void OnServerEdit();
	afx_msg void OnUpdateServerEdit(CCmdUI* pCmdUI);
	afx_msg void OnServerDelete();
	afx_msg void OnUpdateServerDelete(CCmdUI* pCmdUI);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnSysTray();
	afx_msg void OnUpdateSysTray(CCmdUI* pCmdUI);
//	afx_msg LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg void OnPluginMenu(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


extern CIOHandler *g_pIOHandler;
extern CEncryptionHandler *g_pEncryptionHandler;
extern CAuthHandler *g_pAuthHandler;

extern int RegisterClientMenu(LPCSTR szCategory, LPCSTR szComName, TYPEOF_ClientMenu *pProc);
extern int UnregisterClientMenu(LPCSTR szCategory, LPCSTR szComName);
extern int InteractiveConnect(HWND hParent,LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth);
extern int InteractiveListen(HWND hParent,LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__720D769D_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
