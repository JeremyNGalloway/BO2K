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

#include<windows.h>
#include<tlhelp32.h>

#include<osversion.h>
#include<nt_pviewer.h>
#include<pviewer.h>
#include<functions.h>

		
PROCESSINFO *CreateProcListSnapshot(char *svName)
{
	HANDLE hSnap;
	PROCESSINFO *pProcCur;
	THREADINFO *pThreadCur;
	PROCESSINFO phd;
	PROCESSENTRY32 pe;
	THREADENTRY32 te;
	
	if(svName!=NULL) {
		if(svName[0]=='\0') svName=NULL;
	}
	
	if(g_bIsWinNT || (svName!=NULL)) {
		if(NtProcList_ConnectComputer(svName)==-1) return NULL;
		return NtProcList_BuildSnapShot();
	}
	else {
		// Create Toolhelp Snapshot
		hSnap=pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD,0);
		if(hSnap==INVALID_HANDLE_VALUE) return NULL;

		phd.next=NULL;

		// -------------------------- Get Process Information
		pe.dwSize=sizeof(PROCESSENTRY32);
		pProcess32First(hSnap,&pe);
		pProcCur=&phd;
		do {
			// Insert process into process list
			pProcCur->next=(PROCESSINFO *)malloc(sizeof(PROCESSINFO));
			pProcCur=pProcCur->next;
			if(pProcCur==NULL) return NULL;

			// Fill in information
			pProcCur->dwProcID=pe.th32ProcessID;
			lstrcpyn(pProcCur->svApp,strrchr(pe.szExeFile,'\\')+1,MAX_PATH+1);
			char *dot;
			dot=strrchr(pProcCur->svApp,'.');
			if(dot!=NULL) *dot='\0';

			pProcCur->pThread=NULL;
						
		} while(pProcess32Next(hSnap,&pe));

		pProcCur->next=NULL;

		// -------------------------- Get Thread Information
		te.dwSize=sizeof(THREADENTRY32);
		pThread32First(hSnap,&te);
		do {
			// Find process that owns this thread
			pProcCur=phd.next;
			while(pProcCur!=NULL) {
				if(pProcCur->dwProcID==te.th32OwnerProcessID) break;
				pProcCur=pProcCur->next;
			}
			if(pProcCur!=NULL) {
				// Add thread to thread list
				pThreadCur=(THREADINFO *)malloc(sizeof(THREADINFO));
				if(pThreadCur==NULL) return NULL;
				pThreadCur->next=pProcCur->pThread;
				pProcCur->pThread=pThreadCur;

				// Fill in information
				pThreadCur->dwThreadID=te.th32ThreadID;
			}
		} while(pThread32Next(hSnap,&te));

		// Free Toolhelp Snapshot and return
		CloseHandle(hSnap);
		return phd.next;
	}

	return NULL;
}

void DestroyProcListSnapshot(PROCESSINFO *pSS)
{
	PROCESSINFO *pi,*ptmp;
	THREADINFO *ti,*ttmp;

	pi=pSS;
	while(pi!=NULL) {
		ti=pi->pThread;
		while(ti!=NULL) {
			ttmp=ti->next;
			free(ti);
			ti=ttmp;
		}
		ptmp=pi->next;
		free(pi);
		pi=ptmp;
	}
}
