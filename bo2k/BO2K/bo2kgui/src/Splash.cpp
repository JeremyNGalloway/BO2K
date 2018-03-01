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

// Splash.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplash dialog


CSplash::CSplash(CWnd* pParent /*=NULL*/)
	: CDialog(CSplash::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSplash)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSplash::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplash)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSplash, CDialog)
	//{{AFX_MSG_MAP(CSplash)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplash message handlers

BOOL CSplash::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;
	GetDlgItem(IDC_SPLASH)->GetClientRect(&r);
	MoveWindow((GetSystemMetrics(SM_CXSCREEN)-r.right)/2,
		(GetSystemMetrics(SM_CYSCREEN)-r.bottom)/2,
		(r.right-r.left),(r.bottom-r.top),TRUE);

	SetTimer(69,3000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSplash::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==69) {
		EndDialog(0);	
	}
	CDialog::OnTimer(nIDEvent);
}

void CSplash::OnLButtonDown(UINT nFlags, CPoint point) 
{
	EndDialog(0);	
	CDialog::OnLButtonDown(nFlags, point);
}

void CSplash::OnRButtonDown(UINT nFlags, CPoint point) 
{
	EndDialog(0);	
	CDialog::OnRButtonDown(nFlags, point);
}
