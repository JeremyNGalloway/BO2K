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

/*
 *  HistoryEdit.h
 *
 *  Description:
 *    CHistoryEdit interface
 *
 *    A CEdit subclass that allows you to display a scrolling history
 *    of text entries.
 *
 *  Author:
 *    Ravi Bhavnani (ravib@datablast.net)
 *
 *  Revision History:
 *    15 Mar 1998   rab   Original version
 */

#ifndef _HistoryEdit_h_
#define _HistoryEdit_h_

/////////////////////////////////////////////////////////////////////////////
// CHistoryEdit window

class CHistoryEdit : public CEdit
{
// Construction
public:
	CHistoryEdit();

// Attributes
public:
					
// Operations
public:
  void  AppendString (CString str);
  BOOL  IsSelectable() { return m_bSelectable; }
  void  AllowSelection (BOOL bAllowSelect) { m_bSelectable = bAllowSelect; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHistoryEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHistoryEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
  BOOL  m_bSelectable;                          // flag: user can select text in control
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif

// End HistoryEdit.h
