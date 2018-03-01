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

// Wizard6.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "Wizard6.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard6 dialog


CWizard6::CWizard6(CWnd* pParent /*=NULL*/)
	: CDialog(CWizard6::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWizard6)
	m_strPassword = _T("");
	m_nLetterCount = 0;
	//}}AFX_DATA_INIT
}


void CWizard6::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizard6)
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_LETTERCOUNT, m_nLetterCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizard6, CDialog)
	//{{AFX_MSG_MAP(CWizard6)
	ON_BN_CLICKED(IDBACK, OnBack)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizard6 message handlers

void CWizard6::OnBack() 
{
	EndDialog(IDBACK);		
}

void CWizard6::OnChangePassword() 
{
	CString txt;
	int nLen;

	GetDlgItemText(IDC_PASSWORD,txt);
	nLen=txt.GetLength();
	SetDlgItemInt(IDC_LETTERCOUNT,nLen,TRUE);
	if(nLen>=m_nMinLetterCount &&
		nLen<=m_nMaxLetterCount) {
		GetDlgItem(IDOK)->EnableWindow();
	} else {
		if(nLen<=m_nMaxLetterCount) {
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		} else {
			SetDlgItemText(IDC_PASSWORD,txt.Left(m_nMaxLetterCount));
		}
	}
}

BOOL CWizard6::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	OnChangePassword();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
