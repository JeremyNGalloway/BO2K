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

// Wizard1.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "Wizard1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard1 dialog


CWizard1::CWizard1(CWnd* pParent /*=NULL*/)
	: CDialog(CWizard1::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWizard1)
	m_bShowWizard = FALSE;
	//}}AFX_DATA_INIT
}


void CWizard1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizard1)
	DDX_Check(pDX, IDC_SHOWWIZARD, m_bShowWizard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizard1, CDialog)
	//{{AFX_MSG_MAP(CWizard1)
	ON_BN_CLICKED(IDC_SHOWWIZARD, OnShowwizard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizard1 message handlers

void CWizard1::OnShowwizard() 
{
	BOOL bWiz=IsDlgButtonChecked(IDC_SHOWWIZARD);
	AfxGetApp()->WriteProfileInt("Startup","Wizard",bWiz);
}
