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

#if !defined(AFX_PLUGINCONFIG_H__3C09D785_EB98_11D2_ADC5_0000F8084273__INCLUDED_)
#define AFX_PLUGINCONFIG_H__3C09D785_EB98_11D2_ADC5_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// pluginconfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPluginConfig dialog

class CPluginConfig : public CDialog
{
// Construction
public:
	CPluginConfig(CWnd* pParent = NULL);   // standard constructor

	void EnableControls(BOOL bEnable);
	BOOL IsValidPlugin(LPCSTR svPath);
	void UpdateVariableList();

// Dialog Data
	//{{AFX_DATA(CPluginConfig)
	enum { IDD = IDD_PLUGINS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImageList m_ImgList;

	// Generated message map functions
	//{{AFX_MSG(CPluginConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnInsert();
	afx_msg void OnRemove();
	afx_msg void OnSelchangedOptionvariables(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBoolDisabled();
	afx_msg void OnBoolEnabled();
	afx_msg void OnSetvalue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINCONFIG_H__3C09D785_EB98_11D2_ADC5_0000F8084273__INCLUDED_)
