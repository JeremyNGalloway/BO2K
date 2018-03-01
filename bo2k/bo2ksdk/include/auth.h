/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This file is free software, and not subject to GNU Public License
	restrictions; you can redistribute it and/or modify it in any way 
	you see fit. This file is suitable for inclusion in a derivative
	work, regardless of license on the work or availability of source code
	to the work. If you redistribute this file, you must leave this
	header intact.
    
	This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

	The author of this program may be contacted at dildog@l0pht.com. */

#ifndef __INC_AUTH_H
#define __INC_AUTH_H

#include<windows.h>
#include<iohandler.h>
#include<encryption.h>

#define MAX_AUTH_HANDLERS 8
#define INVALID_AUTH_HANDLER ((AUTH_HANDLER *) NULL)

// Socket class

#pragma pack(push, 1)

// IO Handler Structure

typedef struct {
	int (__cdecl *pInsert)(void);
	int (__cdecl *pRemove)(void);
	char *(__cdecl *pQuery)(void);
	void *(__cdecl *pOnListen)(CIOSocket *pSock, CEncryptionEngine *pEnc, int nUserId);
	void *(__cdecl *pOnConnect)(CIOSocket *pSock, CEncryptionEngine *pEnc, int nUserId);
	void *(__cdecl *pOnAccept)(void *pInternal, CIOSocket *pSock, CEncryptionEngine *pEnc);
	int (__cdecl *pGetUserID)(void *pInternal);
	int (__cdecl *pOnClose)(void *pInternal);
	int (__cdecl *pOnRecv)(void *pInternal, CEncryptionEngine *pEnc, BYTE *pData, int nDataLen, BYTE **pInData, int *pnInDataLen);
	int (__cdecl *pOnSend)(void *pInternal, CEncryptionEngine *pEnc, BYTE *pData, int nDataLen, BYTE **pOutData, int *pnOutDataLen);
	void (__cdecl *pFree)(void *pInternal, BYTE *pBuffer);
	BOOL (_cdecl *pValidateCommand)(int nUserId, int nCommand);
} AUTH_HANDLER;

class CAuthSocket {
private:
	
	void *m_pData;

public:
	AUTH_HANDLER *m_pHandler;
	IO_HANDLER *m_pIOH;
	ENCRYPTION_ENGINE *m_pEE;
	CIOSocket *m_pSock;
	CEncryptionEngine *m_pEnc;

	CAuthSocket(AUTH_HANDLER *pHandler, IO_HANDLER *pIOH, ENCRYPTION_ENGINE *pEE);
	virtual ~CAuthSocket();
	virtual int Listen(char *svTarget, int nUserId);
	virtual int Connect(char *svTarget, int nUserId);
	virtual CAuthSocket *Accept(void);
	virtual int GetUserID(void);
	virtual int Close(void);
	virtual int Recv(BYTE **pInData, int *pnInDataLen);
	virtual int Send(BYTE *pData, int nDataLen);
	virtual void Free(BYTE *pBuffer);
	virtual int GetRemoteAddr(char *svAddr,int nMaxLen);
	virtual int GetConnectAddr(char *svAddr,int nMaxLen);
	virtual AUTH_HANDLER *GetAuthHandler(void);
};

#pragma pack(pop)

// Authentiction Handler Manager Functions

class CAuthHandler {

private:
	AUTH_HANDLER *m_AuthHandler[MAX_AUTH_HANDLERS];

public:
	CAuthHandler();
	virtual ~CAuthHandler();
	virtual int Insert(AUTH_HANDLER *handler);
	virtual int Remove(int handlernum);
	virtual char *Query(int nHandler);
	virtual int GetHandlerCount(void);
	virtual AUTH_HANDLER *GetHandler(int nHandler);
	virtual AUTH_HANDLER *GetHandlerByID(char *svID);
};

typedef int (INTERACTIVE_CONNECT)(HWND hParent,LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth);
typedef int (INTERACTIVE_LISTEN)(HWND hParent,LPCSTR svBindStr,LPCSTR svNetMod,LPCSTR svEncryption,LPCSTR svAuth,char *svRBindStr,char *svRNetMod,char *svREncryption,char *svRAuth);

#ifdef __BO2KSERVER__

int IssueAuthCommandRequest(CAuthSocket *cas_from, int command, int comid, int nArg1, char *svArg2, char *svArg3);
int IssueAuthCommandReply(CAuthSocket *cas_from, int comid, int nReplyCode, char *svReply);
CAuthSocket *ConnectAuthSocket(INTERACTIVE_CONNECT *pIC, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth);
CAuthSocket *ListenAuthSocket(INTERACTIVE_LISTEN *pIL, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth);

#endif


#endif