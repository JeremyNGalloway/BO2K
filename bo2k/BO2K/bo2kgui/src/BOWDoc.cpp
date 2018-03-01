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

// BOWDoc.cpp : implementation of the CBOWDoc class
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "MainFrm.h"
#include "BOWDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAGIC_DWORD 0xCDC31337

/////////////////////////////////////////////////////////////////////////////
// CBOWDoc

IMPLEMENT_DYNCREATE(CBOWDoc, CDocument)

BEGIN_MESSAGE_MAP(CBOWDoc, CDocument)
	//{{AFX_MSG_MAP(CBOWDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBOWDoc construction/destruction

CBOWDoc::CBOWDoc()
{
	// TODO: add one-time construction code here
	m_pHeadSI=NULL;
}

CBOWDoc::~CBOWDoc()
{
}

void *CBOWDoc::InsertServer(const char *svName,
	                        const char *svConnectionType,
					        const char *svAddress,
						    const char *svEncryption,
						    const char *svAuthentication)
{
	SERVER_INFO *si,*prev;

	si=(SERVER_INFO *)malloc(sizeof(SERVER_INFO));
	if(si==NULL) return NULL;

	memset(si,0,sizeof(SERVER_INFO));
	
	// Permanent Stuff
	lstrcpyn(si->svName,svName,64);
	lstrcpyn(si->svConnectionType,svConnectionType,64);
	lstrcpyn(si->svAddress,svAddress,512);
	lstrcpyn(si->svEncryption,svEncryption,64);
	lstrcpyn(si->svAuthentication,svAuthentication,64);
	
	// Temporary Stuff
	si->bOpened=FALSE;

	si->next=NULL;
	if(m_pHeadSI==NULL) {
		m_pHeadSI=si;
	} else {
		for(prev=m_pHeadSI;prev->next!=NULL;prev=prev->next);
		prev->next=si;
	}

	SetModifiedFlag(TRUE);

	return si;
}

void CBOWDoc::EditServer(SERVER_INFO *si,
						 const char *svName,
	                     const char *svConnectionType,
					     const char *svAddress,
						 const char *svEncryption,
						 const char *svAuthentication)
{
	// Permanent Stuff
	lstrcpyn(si->svName,svName,64);
	lstrcpyn(si->svConnectionType,svConnectionType,64);
	lstrcpyn(si->svAddress,svAddress,512);
	lstrcpyn(si->svEncryption,svEncryption,64);
	lstrcpyn(si->svAuthentication,svAuthentication,64);
	
	SetModifiedFlag(TRUE);
}

		
void CBOWDoc::DeleteServer(void *psi)
{
	SERVER_INFO *si;
	si=(SERVER_INFO *)psi;

	if(m_pHeadSI==si) {
		m_pHeadSI=si->next;
	}
	else {
		SERVER_INFO *prev;
		prev=m_pHeadSI;
		while((prev->next)!=si) prev=prev->next;
		prev->next=si->next;
	}
	free(si);

	SetModifiedFlag(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CBOWDoc serialization

void CBOWDoc::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{	
		// TODO: add storing code here
	} else {
		// TODO: add loading code here

	}
}

/////////////////////////////////////////////////////////////////////////////
// CBOWDoc diagnostics

#ifdef _DEBUG
void CBOWDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBOWDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBOWDoc commands

BOOL CBOWDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	HANDLE out;
	out=CreateFile(lpszPathName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(out!=INVALID_HANDLE_VALUE) {
		SERVER_INFO *si;
		DWORD dwBytes;
		DWORD magic=MAGIC_DWORD;
		si=m_pHeadSI;
		WriteFile(out,&magic,sizeof(DWORD),&dwBytes,NULL);
		while(si!=NULL) {
			WriteFile(out,si->svName,64,&dwBytes,NULL);
			WriteFile(out,si->svConnectionType,64,&dwBytes,NULL);
			WriteFile(out,si->svAddress,512,&dwBytes,NULL);
			WriteFile(out,si->svEncryption,64,&dwBytes,NULL);
			WriteFile(out,si->svAuthentication,64,&dwBytes,NULL);
			si=si->next;
		}
		CloseHandle(out);
	}
	
	SetModifiedFlag(FALSE);
	
	return TRUE;
}

BOOL CBOWDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	char svName[64];
	char svConnectionType[64];
	char svAddress[512];
	char svEncryption[64];
	char svAuthentication[64];

	DeleteContents();

	HANDLE in;
	in=CreateFile(lpszPathName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(in!=INVALID_HANDLE_VALUE) {
		DWORD dwBytes,dwCount,dwMagic;
		dwCount=GetFileSize(in,NULL);
		ReadFile(in,&dwMagic,sizeof(DWORD),&dwBytes,NULL);
		dwCount-=dwBytes;
		if(dwMagic!=MAGIC_DWORD) {
			CloseHandle(in);
			return FALSE;
		}
		while(dwCount>0) {
			ReadFile(in,svName,64,&dwBytes,NULL);
			dwCount-=dwBytes;
			ReadFile(in,svConnectionType,64,&dwBytes,NULL);
			dwCount-=dwBytes;
			ReadFile(in,svAddress,512,&dwBytes,NULL);
			dwCount-=dwBytes;
			ReadFile(in,svEncryption,64,&dwBytes,NULL);
			dwCount-=dwBytes;
			ReadFile(in,svAuthentication,64,&dwBytes,NULL);
			dwCount-=dwBytes;

			InsertServer(svName,svConnectionType,svAddress,svEncryption,svAuthentication);
		}
		
		CloseHandle(in);
	}
	
	SetModifiedFlag(FALSE);

	return TRUE;
}

void CBOWDoc::DeleteContents() 
{
	theApp.NewDocumentFile();

	if(m_pHeadSI!=NULL) {
		SERVER_INFO *psi;
		while(m_pHeadSI!=NULL) {
			psi=m_pHeadSI->next;
			free(m_pHeadSI);
			m_pHeadSI=psi;
		}
	}

	CDocument::DeleteContents();
}
