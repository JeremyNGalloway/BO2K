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

// omCtlMsg.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "ComCtlMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ComCtlMsg dialog


ComCtlMsg::ComCtlMsg(CWnd* pParent /*=NULL*/)
	: CDialog(ComCtlMsg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ComCtlMsg)
		// NOTE: the ClassWizard will add member initialization here

	//}}AFX_DATA_INIT
}


void ComCtlMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ComCtlMsg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ComCtlMsg, CDialog)
	//{{AFX_MSG_MAP(ComCtlMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ComCtlMsg message handlers

BOOL ComCtlMsg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_HL_COMCTL.SetURL("ftp://ftp.microsoft.com/Softlib/MSLFILES/COM32UPD.EXE");
	m_HL_COMCTL.SubclassDlgItem(IDC_COMCTLUPD,this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
