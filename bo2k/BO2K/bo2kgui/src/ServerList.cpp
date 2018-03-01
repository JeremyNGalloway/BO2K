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

// ServerList.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "ServerList.h"
#include "NewMachineDlg.h"
#include "MainFrm.h"
#include "BOWDoc.h"
#include "BODialogBar.h"
#include "BOServerDlg.h"

#define TIMER_UPDATE 1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerList dialog


CServerList::CServerList(CWnd* pParent /*=NULL*/)
	: CGradientDialog(CServerList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nUpdateTimer=0;
}

CServerList::~CServerList()
{

}

void CServerList::CleanUpAndClose(void)
{
	KillTimer(m_nUpdateTimer);
	DeleteAllItems();
	DestroyWindow();
}


void CServerList::DoDataExchange(CDataExchange* pDX)
{
	CGradientDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerList)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CServerList, CGradientDialog)
	//{{AFX_MSG_MAP(CServerList)
	ON_BN_CLICKED(ID_DETAILS, OnDetails)
	ON_BN_CLICKED(ID_NEWMACHINE, OnNewmachine)
	ON_BN_CLICKED(ID_DELETEMACHINE, OnDeletemachine)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, ID_SERVERLIST, OnItemchangedServerlist)
	ON_NOTIFY(LVN_KEYDOWN, ID_SERVERLIST, OnKeydownServerlist)
	ON_NOTIFY(NM_DBLCLK, ID_SERVERLIST, OnDblclkServerlist)
	ON_BN_CLICKED(IDC_DOCKING, OnAllowDocking)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_EDITMACHINE, OnEditmachine)
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerList message handlers


void CServerList::OnCancel()
{

}

void CServerList::OnOK()
{

}


void CServerList::OnDetails() 
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	CButton *details=(CButton *)GetDlgItem(ID_DETAILS);
	
	DWORD dwStyle=GetWindowLong(list->GetSafeHwnd(),GWL_STYLE);

	dwStyle &= ~LVS_TYPEMASK;
	if(details->GetCheck()) dwStyle |= LVS_REPORT;
	else dwStyle |= LVS_ICON;

	SetWindowLong(list->GetSafeHwnd(),GWL_STYLE,dwStyle);
	list->RedrawWindow();
}

void CServerList::OnNewmachine() 
{
	CString strSvrAddr;
	CNewMachineDlg nmd;
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);

	if(nmd.DoModal()==IDOK) {
		// Add to CBOWDoc

		CBOWDoc *cbd;
		SERVER_INFO *pSvr;
 		cbd=(CBOWDoc *) ((CMainFrame *)theApp.GetMainWnd())->GetActiveDocument();

		pSvr=(SERVER_INFO *) cbd->InsertServer(nmd.m_strMachineName,nmd.m_strSelectedIO,nmd.m_strSvrAddr,nmd.m_strSelectedEnc,nmd.m_strSelectedAuth);
		if(pSvr==NULL) return;
		
		// Synchronize List Control
		SynchToDocument(cbd);
	}
	
}

void CServerList::OnEditmachine() 
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	CNewMachineDlg nmd;
	CBOWDoc *cbd;
	SERVER_INFO *pSvr;
	char textbuf[512];

	cbd=(CBOWDoc *) ((CMainFrame *)theApp.GetMainWnd())->GetActiveDocument();

	int i;
	for(i=list->GetItemCount()-1;i>=0;i--) {
		if(list->GetItemState(i,LVIS_SELECTED)==LVIS_SELECTED) break;
	}
	if(i<0) return;


	LVITEM lvi;
	lvi.iItem=i;
	lvi.iSubItem=0;
	lvi.mask=LVIF_TEXT | LVIF_PARAM;
	lvi.pszText=textbuf;
	lvi.cchTextMax=512;
	list->GetItem(&lvi);
	pSvr=(SERVER_INFO *)lvi.lParam;

	if(pSvr->pServerDlg->m_bIsConnected) {
		AfxMessageBox("The server command window is currently connected.\nDisconnect it first before editing the server parameters.",MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return;
	}

	nmd.m_strMachineName=pSvr->svName;
	nmd.m_strSvrAddr=pSvr->svAddress;
	nmd.m_strSelectedIO=pSvr->svConnectionType;
	nmd.m_strSelectedEnc=pSvr->svEncryption;
	nmd.m_strSelectedAuth=pSvr->svAuthentication;
	if(nmd.DoModal()==IDOK) {
		cbd->EditServer(pSvr,nmd.m_strMachineName,nmd.m_strSelectedIO,nmd.m_strSvrAddr,nmd.m_strSelectedEnc,nmd.m_strSelectedAuth);
		SynchToDocument(cbd);
	}

}

void CServerList::OnDeletemachine() 
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	CBOWDoc *cbd;
	SERVER_INFO *pSvr;

	cbd=(CBOWDoc *) ((CMainFrame *)theApp.GetMainWnd())->GetActiveDocument();

	int i;
	for(i=list->GetItemCount()-1;i>=0;i--) {
		if(list->GetItemState(i,LVIS_SELECTED)==LVIS_SELECTED) {
			pSvr=(SERVER_INFO *)list->GetItemData(i);
			DeleteItem(i);
			cbd->DeleteServer(pSvr);
		}
	}
}

void CServerList::DeleteItem(int idx)
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);

	SERVER_INFO *pSvr;
	pSvr=(SERVER_INFO *) list->GetItemData(idx);
		
	pSvr->pServerDlgBDB->Hide();

	pSvr->pServerDlg->DestroyWindow();
	delete pSvr->pServerDlg;
	pSvr->pServerDlg=NULL;

	pSvr->pServerDlgBDB->DestroyWindow();
	delete pSvr->pServerDlgBDB;
	pSvr->pServerDlgBDB=NULL;

	list->DeleteItem(idx);
}

void CServerList::DeleteAllItems()
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	int i;
	if(list!=NULL) {
		for(i=list->GetItemCount()-1;i>=0;i--) {
			DeleteItem(i);
		}
	}
}

void CServerList::OnSize(UINT nType, int cx, int cy) 
{
	CGradientDialog::OnSize(nType, cx, cy);

	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	if(list) {
		RECT r;
		int newsize;
		int i;
			
		// Move window
		GetClientRect(&r);
		list->MoveWindow(36,26,(r.right-r.left)-40,(r.bottom-r.top)-30);

		// Resize columns
		list->GetClientRect(&r);
		newsize=r.right-r.left;
	
		for(i=0;i<5;i++) {
			int newwidth=newsize/5;
			if(newwidth<100) newwidth=100;
			list->SetColumnWidth(i,newwidth);
		}
	}
}

BOOL CServerList::OnInitDialog() 
{
	CGradientDialog::OnInitDialog();

	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);

	m_NormalImageList.Create(IDB_NORMAL_BOCOMPS,32,1,RGB(255,0,0));
	list->SetImageList(&m_NormalImageList,LVSIL_NORMAL);
	
	m_SmallImageList.Create(IDB_LITTLE_BOCOMPS,16,1,RGB(255,0,0));
	list->SetImageList(&m_SmallImageList,LVSIL_SMALL);

	// Size columns
	RECT r;
	list->GetClientRect(&r);
	int newwidth=(r.right-r.left)/5;
	if(newwidth<100) newwidth=100;

	list->InsertColumn(0,"Machine",LVCFMT_LEFT,newwidth,0);
	list->InsertColumn(1,"Address",LVCFMT_LEFT,newwidth,1);
	list->InsertColumn(2,"Connection",LVCFMT_LEFT,newwidth,2);
	list->InsertColumn(3,"Encryption",LVCFMT_LEFT,newwidth,3);
	list->InsertColumn(4,"Authentication",LVCFMT_LEFT,newwidth,3);
	list->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	CButton *nmb=(CButton *)GetDlgItem(ID_NEWMACHINE);
	nmb->SetIcon(theApp.LoadIcon(IDI_NEWMACHINE));
	
	CButton *dmb=(CButton *)GetDlgItem(ID_DELETEMACHINE);
	dmb->SetIcon(theApp.LoadIcon(IDI_DELETEMACHINE));	
	dmb->EnableWindow(FALSE);

	CButton *emb=(CButton *)GetDlgItem(ID_EDITMACHINE);
	emb->SetIcon(theApp.LoadIcon(IDI_EDITMACHINE));
	emb->EnableWindow(FALSE);

	CButton *dok=(CButton *)GetDlgItem(IDC_DOCKING);
	dok->SetCheck(1);
	
	CButton *ddt=(CButton *)GetDlgItem(ID_DETAILS);
	ddt->SetCheck(1);

	m_nUpdateTimer=SetTimer(TIMER_UPDATE,500,NULL);
	if(m_nUpdateTimer==0) return FALSE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServerList::OnItemchangedServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CListCtrl *list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	
	CButton *del=(CButton *)GetDlgItem(ID_DELETEMACHINE);
	if(list->GetSelectedCount()==0) del->EnableWindow(FALSE);
	else del->EnableWindow(TRUE);
	
	CButton *edt=(CButton *)GetDlgItem(ID_EDITMACHINE);
	if(list->GetSelectedCount()==1) edt->EnableWindow(TRUE);
	else edt->EnableWindow(FALSE);
	
	*pResult = 0;
}


void CServerList::OnKeydownServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVKEYDOWN* pLVKeyDown = (NMLVKEYDOWN*)pNMHDR;
	CButton *butt;

	switch(pLVKeyDown->wVKey) {
	case VK_DELETE:
		butt=(CButton *)GetDlgItem(ID_DELETEMACHINE);
		if(butt->IsWindowEnabled()) OnDeletemachine();
		break;
	case VK_INSERT:
		OnNewmachine();
		break;
	}
	
	*pResult = 0;
}

void CServerList::OnDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CListCtrl *list=(CListCtrl *)GetDlgItem(pNMHDR->idFrom);
	
	int i;
	if(list->GetSelectedCount()>0) {
		for(i=(list->GetItemCount()-1);i>=0;i--) {
			if(list->GetItemState(i,LVIS_SELECTED)==LVIS_SELECTED) {
				LVITEM lvi;
				SERVER_INFO *si;
				char svText[512];
				
				// Get item information
				lvi.iItem=i;
				lvi.iSubItem=0;
				lvi.mask=LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
				lvi.pszText=svText;
				lvi.cchTextMax=512;
				list->GetItem(&lvi);

				si=(SERVER_INFO *) lvi.lParam;
				if(si->pServerDlgBDB->IsWindowVisible()) {
					si->pServerDlgBDB->Hide();
				} else {
					si->pServerDlgBDB->Show();				
				}

			}
		}
	}

	*pResult = 0;
}

void CServerList::OnAllowDocking() 
{
	CButton *btn;
	btn=(CButton *)GetDlgItem(IDC_DOCKING);
	if(btn->GetCheck()) {
		((CBODialogBar *)GetParent())->EnableDocking(CBRS_ALIGN_ANY);	
	} else {
		((CBODialogBar *)GetParent())->EnableDocking(0);
		((CBODialogBar *)GetParent())->m_pDockContext->ToggleDocking();
	}
}

void CServerList::SynchToDocument(CBOWDoc *cbd)
{
	SERVER_INFO *si;
	CListCtrl *pList;
	int idx;

	pList=(CListCtrl *) GetDlgItem(ID_SERVERLIST);

	// Add items that should be there and update those that are already there
	si=cbd->m_pHeadSI;
	while(si!=NULL) {
		BOOL bNew;
		LVITEM lvi;
		LVFINDINFO lvfi;
		lvfi.flags=LVFI_PARAM;
		lvfi.lParam=(LPARAM) si;
	
		idx=pList->FindItem(&lvfi,-1);
		if(idx==-1) {
			idx=0;
			lvi.mask=LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lvi.pszText=si->svName;
			lvi.iImage=0;
			lvi.iItem=idx;
			lvi.iSubItem=0;
			lvi.lParam=(LPARAM) si;
			pList->InsertItem(&lvi);
			bNew=TRUE;
		} else {
			lvi.mask=LVIF_TEXT;
			lvi.pszText=si->svName;
			lvi.iItem=idx;
			lvi.iSubItem=0;

			pList->SetItem(&lvi);
			bNew=FALSE;
		}
		
		lvi.mask=LVIF_TEXT;
		lvi.pszText=si->svAddress;
		lvi.iItem=idx;
		lvi.iSubItem=1;
		pList->SetItem(&lvi);

		lvi.mask=LVIF_TEXT;
		lvi.pszText=si->svConnectionType;
		lvi.iItem=idx;
		lvi.iSubItem=2;
		pList->SetItem(&lvi);

		lvi.mask=LVIF_TEXT;
		lvi.pszText=si->svEncryption;
		lvi.iItem=idx;
		lvi.iSubItem=3;
		pList->SetItem(&lvi);

		lvi.mask=LVIF_TEXT;
		lvi.pszText=si->svAuthentication;
		lvi.iItem=idx;
		lvi.iSubItem=4;
		pList->SetItem(&lvi);


		// Create cool dialog bar
		if(bNew) {
			CSize sizeNormal(422,399);
			CString title("Server Command Client");
			si->pServerDlgBDB=new CBODialogBar;
			si->pServerDlgBDB->SetNormalSize(sizeNormal);
			si->pServerDlgBDB->Create(this->GetParentFrame(),IDD_BOSERVERDLG,title,sizeNormal);
			si->pServerDlgBDB->SetBarStyle((si->pServerDlgBDB->GetBarStyle() & ~CBRS_ALIGN_ANY) | CBRS_TOOLTIPS | CBRS_BORDER_ANY | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
			si->pServerDlgBDB->EnableDockingOnSizeBar(CBRS_ALIGN_ANY);
			
			si->pServerDlg=new CBOServerDlg();
			si->pServerDlg->SetServerInfo(si);
			si->pServerDlg->Create(CBOServerDlg::IDD,si->pServerDlgBDB);
			si->pServerDlg->UseGradient(m_bGradient);

			si->pServerDlgBDB->SetDialog(si->pServerDlg);
		
			si->pServerDlgBDB->GetDockingFrame()->FloatControlBar(si->pServerDlgBDB, CPoint((GetSystemMetrics(SM_CXSCREEN)-422)/2,(GetSystemMetrics(SM_CYSCREEN)-399)/2));
			//si->pServerDlgBDB->Hide();
		}
		else {
			si->pServerDlg->SetServerInfo(si);
			si->pServerDlg->UpdateDialog();
			si->pServerDlg->Invalidate();
			si->pServerDlg->UpdateWindow();
		}

		si=si->next;
	}
}

void CServerList::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==TIMER_UPDATE) {
		UpdateListStatus();
	}

	CGradientDialog::OnTimer(nIDEvent);
}

void CServerList::UpdateListStatus(void)
{
	CListCtrl *list;
	
	list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	
	int i;
	for(i=(list->GetItemCount()-1);i>=0;i--) {
		LVITEM lvi;
		SERVER_INFO *si;
		char svText[512];
		
		// Get item information
		lvi.iItem=i;
		lvi.iSubItem=0;
		lvi.mask=LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
		lvi.pszText=svText;
		lvi.cchTextMax=512;
		list->GetItem(&lvi);
		
		si=(SERVER_INFO *) lvi.lParam;
		lvi.mask=LVIF_IMAGE;
		if(!si->pServerDlgBDB->IsFloating()) {
			if(si->pServerDlgBDB->m_hWnd!=NULL) {
				if(si->pServerDlgBDB->IsWindowVisible()) lvi.iImage=1;
				else lvi.iImage=0;
			}
		} else {
			if(si->pServerDlg->m_hWnd!=NULL) {
				if(si->pServerDlg->GetParent()->GetParent()->GetParent()->IsWindowVisible()) lvi.iImage=1;
				else lvi.iImage=0;
			}
		}
		list->SetItem(&lvi);
	}
}

void CServerList::UseGradient(BOOL bUse) 
{
	CListCtrl *list;
	list=(CListCtrl *)GetDlgItem(ID_SERVERLIST);
	
	int i;
	for(i=(list->GetItemCount()-1);i>=0;i--) {
		LVITEM lvi;
		SERVER_INFO *si;
		char svText[512];
		
		// Get item information
		lvi.iItem=i;
		lvi.iSubItem=0;
		lvi.mask=LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
		lvi.pszText=svText;
		lvi.cchTextMax=512;
		list->GetItem(&lvi);
		
		si=(SERVER_INFO *) lvi.lParam;
		
		si->pServerDlg->UseGradient(bUse);
		si->pServerDlg->Invalidate();
		si->pServerDlg->UpdateWindow();
	}
	
	if(bUse)
		SetGradient(GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_3DHILIGHT),2);
	else
		SetGradient();

	Invalidate();
	UpdateWindow();
}
