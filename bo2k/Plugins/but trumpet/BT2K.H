// Butt Trumpet 2000
//  Brian Enigma

#ifndef __BT2K_H__
#define __BT2K_H__

#include<windows.h>
#include<bo2k/plugins.h>
#include<bo2k/bocomreg.h>
#include<bo2k/auth.h>
#include<bo2k/iohandler.h>
#include<bo2k/encryption.h>

// ---------------- Global Linkage Variables -----------------
extern CEncryptionHandler				*g_pEncHandler;
extern CIOHandler						*g_pIOHandler;
extern CAuthHandler						*g_pAuthHandler;
extern TYPEOF_RegisterCommand			*RegisterCommand;
extern TYPEOF_UnregisterCommand			*UnregisterCommand;
extern TYPEOF_RegisterClientMenu		*RegisterClientMenu;
extern TYPEOF_UnregisterClientMenu		*UnregisterClientMenu;
extern TYPEOF_IssueAuthCommandRequest	*IssueAuthCommandRequest;
extern TYPEOF_IssueAuthCommandReply		*IssueAuthCommandReply;
extern TYPEOF_ConnectAuthSocket			*ConnectAuthSocket;
extern TYPEOF_ListenAuthSocket			*ListenAuthSocket;
extern TYPEOF_InteractiveConnect		*InteractiveConnect;
extern TYPEOF_InteractiveListen 		*InteractiveListen;
// ------------- External Implementations ------------------
extern BOOL GetCfgBool(char *cfgstr,char *key);

// ------------- Function Implementations ------------------
#define MAX_THREADS 64

extern HINSTANCE g_hInstance;
extern char g_szBT2KOptions[];
extern BOOL g_bActive;
extern long g_nNumThreads;

// internal functions
void debug(const char* msg);
void myIP(char *result);
DWORD WINAPI btWorkerThread(LPVOID lpParameter);
BOOL announce(void);
int Send( SOCKET s, const char *lpszBuff, int nLen, int nFlags );
int Receive( SOCKET s, LPTSTR lpszBuff, int nLenMax, int nFlags, LPCTSTR lpszReplyCode );
char g_szIPList[17*6+1];
char* g_pszDebugString;
int g_nDebugCommand;
int g_nDebugClearCommand;
int g_nDebugResendCommand;

// command definitions
int CmdProc_DebugCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_DebugClearCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_DebugResendCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
DWORD WINAPI btWorkerThread(LPVOID lpParameter);


#endif
