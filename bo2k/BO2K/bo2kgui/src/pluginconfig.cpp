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

// pluginconfig.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "MainFrm.h"
#include "pluginconfig.h"
#include "dll_load.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPluginConfig dialog


CPluginConfig::CPluginConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CPluginConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPluginConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPluginConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginConfig)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginConfig, CDialog)
	//{{AFX_MSG_MAP(CPluginConfig)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(TVN_SELCHANGED, IDC_OPTIONVARIABLES, OnSelchangedOptionvariables)
	ON_BN_CLICKED(IDC_BOOL_DISABLED, OnBoolDisabled)
	ON_BN_CLICKED(IDC_BOOL_ENABLED, OnBoolEnabled)
	ON_BN_CLICKED(IDC_SETVALUE, OnSetvalue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginConfig message handlers

BOOL CPluginConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ImgList.Create(IDB_IMGLIST,16,1,RGB(0,255,0));

	CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
	pLC->InsertColumn(0,"Plugin Name",LVCFMT_LEFT,100,-1);
	pLC->InsertColumn(1,"Version",LVCFMT_LEFT,48,-1);
	pLC->InsertColumn(2,"BO2K Ver",LVCFMT_LEFT,60,-1);
	pLC->InsertColumn(3,"Description",LVCFMT_LEFT,200,-1);
	pLC->SetImageList(&m_ImgList,LVSIL_SMALL);
	pLC->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);
	pTC->SetImageList(&m_ImgList,TVSIL_NORMAL);

	int i,count;
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();
	count=pFrm->m_arrPluginInfo.GetSize();
	for(i=0;i<count;i++) {
		CString str;
		PLUGIN_INFO pi;
		
		pi=pFrm->m_arrPluginInfo[i];

		pLC->InsertItem(i,pi.svFilename,0);
		str.Format("%d.%d",pi.wVersionHi,pi.wVersionLo);
		pLC->SetItem(i,1,LVIF_TEXT,str,0,0,0,0);
		str.Format("%d.%d",pi.wBOVersionHi,pi.wBOVersionLo);
		pLC->SetItem(i,2,LVIF_TEXT,str,0,0,0,0);
		pLC->SetItem(i,3,LVIF_TEXT,pi.svDescription,0,0,0,0);
	}

	UpdateVariableList();

	EnableControls(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginConfig::EnableControls(BOOL bEnable)
{
	// Deselect All List/Tree Controls
	
	((CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES))->SelectItem(NULL);
	((CListCtrl *)GetDlgItem(IDC_PLUGINS))->SetSelectionMark(-1);

	// Enable/Disable Controls
	
	GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SETVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_ENABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_DISABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_GROUP)->EnableWindow(FALSE);
}

BOOL CPluginConfig::IsValidPlugin(LPCSTR svPath)
{
	HMODULE hMod;
	BOOL bAltLoad=FALSE;
	
	hMod=LoadLibrary(svPath);
	if(hMod==NULL) {
		bAltLoad=TRUE;
		hMod=LoadDLL(svPath);
		if(hMod==NULL) return FALSE;
	}

	FARPROC proc=GetDLLProcAddress(hMod,"InstallPlugin");
	
	if(bAltLoad) FreeDLL(hMod);
	else FreeLibrary(hMod);

	return (proc==NULL)?FALSE:TRUE;
}

void CPluginConfig::UpdateVariableList()
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);
	pTC->DeleteAllItems();

	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();
	
	// Add options for executable
	
	pFrm->m_arrVarInfo.RemoveAll();
	
	char *base=(char *)AfxGetInstanceHandle();
	PIMAGE_OPTIONAL_HEADER poh=(PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(base);
	pFrm->AddVariables(base, poh->SizeOfImage, -1);

	// Add options for all plugins
	
	int i,count;
	count=pFrm->m_arrPluginInfo.GetSize();
	for(i=0;i<count;i++) {
		pFrm->AddVariables((char *)pFrm->m_arrPluginInfo[i].pPluginImage,pFrm->m_arrPluginInfo[i].dwPluginLen,i);
	}
	
	
	// Add option variables to tree control
	count=pFrm->m_arrVarInfo.GetSize();
	for(i=0;i<count;i++) {
		VARIABLE_INFO vi=pFrm->m_arrVarInfo.GetAt(i);
		HTREEITEM hti;

		// Find category item
		hti=pTC->GetRootItem();
		while(hti!=NULL) {
			if(pTC->GetItemText(hti).Compare(vi.svCategory)==0) break;
			hti=pTC->GetNextSiblingItem(hti);
		}
		if(hti==NULL) {
			if(vi.nPlugin==-1) {
				hti=pTC->InsertItem(vi.svCategory,4,5);
			} else {
				hti=pTC->InsertItem(vi.svCategory,6,7);
			}
			pTC->SetItemData(hti,0xFFFFFFFF);
		}
		int nImg;
		if(vi.VarType=='B') nImg=3;
		else if(vi.VarType=='N') nImg=2;
		else if(vi.VarType=='S') nImg=1;

		hti=pTC->InsertItem(vi.svVarName,nImg,nImg,hti);
		pTC->SetItemData(hti,i);
	}
}

void CPluginConfig::OnInsert() 
{
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();
	CFileDialog cfd(TRUE,".dll",NULL,OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,"DLL Files (*.dll)|*.dll|All Files (*.*)|*.*||",this);
	cfd.m_ofn.lpstrTitle="Insert BO2K Plugin";
	
	if(cfd.DoModal()==IDOK) {
		if(IsValidPlugin(cfd.GetPathName())) {
			int nNum;
			CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
			CString str;
			PLUGIN_INFO pi;
			
			nNum=pFrm->AddPlugin(cfd.GetPathName());
			
			pi=pFrm->m_arrPluginInfo[nNum];
			
			pLC->InsertItem(nNum,pi.svFilename,0);
			str.Format("%d.%d",pi.wVersionHi,pi.wVersionLo);
			pLC->SetItem(nNum,1,LVIF_TEXT,str,0,0,0,0);
			str.Format("%d.%d",pi.wBOVersionHi,pi.wBOVersionLo);
			pLC->SetItem(nNum,2,LVIF_TEXT,str,0,0,0,0);
			pLC->SetItem(nNum,3,LVIF_TEXT,pi.svDescription,0,0,0,0);
	
			UpdateVariableList();
		} else {
			AfxMessageBox("The specified file is not a valid BO2K plugin");
		}
	}
}

void CPluginConfig::OnRemove() 
{
	int nNum;
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();
	CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));

	nNum=pLC->GetSelectionMark();
	if(nNum<0) return;

	CString str;

	pLC->DeleteItem(nNum);
	nNum=pFrm->RemovePlugin(nNum);
	
	UpdateVariableList();
}




void CPluginConfig::OnSelchangedOptionvariables(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTREEVIEW *pNMTreeView = (NMTREEVIEW*)pNMHDR;
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();

	// Hide all option controls
	GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SETVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_ENABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_DISABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_GROUP)->EnableWindow(FALSE);

	if(pNMTreeView->itemNew.hItem!=NULL) {
		int nVar=pNMTreeView->itemNew.lParam;
		if(nVar!=0xFFFFFFFF) {	
			if(pFrm->m_arrVarInfo[nVar].VarType=='B') {
				GetDlgItem(IDC_BOOL_GROUP)->EnableWindow();
				GetDlgItem(IDC_BOOL_ENABLED)->EnableWindow();
				GetDlgItem(IDC_BOOL_DISABLED)->EnableWindow();
				
				if(atoi(pFrm->m_arrVarInfo[nVar].svVarValue)) {
					((CButton *)GetDlgItem(IDC_BOOL_ENABLED))->SetCheck(1);
					((CButton *)GetDlgItem(IDC_BOOL_DISABLED))->SetCheck(0);
				} else {
					((CButton *)GetDlgItem(IDC_BOOL_DISABLED))->SetCheck(1);
					((CButton *)GetDlgItem(IDC_BOOL_ENABLED))->SetCheck(0);
				}
				
			} else if(pFrm->m_arrVarInfo[nVar].VarType=='N') {
				GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow();
				GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow();
				GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow();
				GetDlgItem(IDC_SETVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(pFrm->m_arrVarInfo[nVar].svVarValue);
				GetDlgItem(IDC_STR_NEWVALUE)->SetWindowText("");
				((CEdit *)GetDlgItem(IDC_STR_CURRENTVALUE))->SetLimitText(pFrm->m_arrVarInfo[nVar].nStrLen);
				((CEdit *)GetDlgItem(IDC_STR_NEWVALUE))->SetLimitText(pFrm->m_arrVarInfo[nVar].nStrLen);
				GetDlgItem(IDC_STR_CURRENTVALUE)->ModifyStyle(0,ES_NUMBER);
				GetDlgItem(IDC_STR_NEWVALUE)->ModifyStyle(0,ES_NUMBER);
			} else if(pFrm->m_arrVarInfo[nVar].VarType=='S') {
				GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow();
				GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow();
				GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow();
				GetDlgItem(IDC_SETVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(pFrm->m_arrVarInfo[nVar].svVarValue);
				GetDlgItem(IDC_STR_NEWVALUE)->SetWindowText("");
				((CEdit *)GetDlgItem(IDC_STR_CURRENTVALUE))->SetLimitText(pFrm->m_arrVarInfo[nVar].nStrLen);
				((CEdit *)GetDlgItem(IDC_STR_NEWVALUE))->SetLimitText(pFrm->m_arrVarInfo[nVar].nStrLen);
				GetDlgItem(IDC_STR_CURRENTVALUE)->ModifyStyle(ES_NUMBER,0);
				GetDlgItem(IDC_STR_NEWVALUE)->ModifyStyle(ES_NUMBER,0);				
			}
		}
	}

	*pResult = 0;
}

void CPluginConfig::OnBoolDisabled() 
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		pFrm->m_arrVarInfo[nVar].svVarValue[0]='0';

		// Set into image
		VARIABLE_INFO vi=pFrm->m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=pFrm->m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)AfxGetInstanceHandle();
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));

	}
}

void CPluginConfig::OnBoolEnabled() 
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		pFrm->m_arrVarInfo[nVar].svVarValue[0]='1';

		// Set into image
		VARIABLE_INFO vi=pFrm->m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=pFrm->m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)AfxGetInstanceHandle();
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));
	}
}

void CPluginConfig::OnSetvalue()
{
	CMainFrame *pFrm=(CMainFrame *)AfxGetMainWnd();
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		CString str;
		GetDlgItem(IDC_STR_NEWVALUE)->GetWindowText(str);
		if(pFrm->m_arrVarInfo[nVar].VarType=='N') {
			int nValue=atoi(str);
			if((nValue>pFrm->m_arrVarInfo[nVar].nNumHi) ||
			   (nValue<pFrm->m_arrVarInfo[nVar].nNumLo)) {
				AfxMessageBox("Value is out of range.");
				return;
			}
		}
		lstrcpyn(pFrm->m_arrVarInfo[nVar].svVarValue,str,256);
		GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(str);
	
		// Set into image
		VARIABLE_INFO vi=pFrm->m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=pFrm->m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)AfxGetInstanceHandle();
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));
	}	
}
