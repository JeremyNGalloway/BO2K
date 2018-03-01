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

#ifndef __INC_IOHANDLER_H
#define __INC_IOHANDLER_H

#include<windows.h>

#define MAX_IO_HANDLERS 8
#define INVALID_IO_HANDLER ((IO_HANDLER *) NULL)

// Socket class

#pragma pack(push, 1)

// IO Handler Structure

typedef struct {
	char *(__cdecl *pQuery)(void);
	int (__cdecl *pInsert)(void);
	int (__cdecl *pRemove)(void);
	void *(__cdecl *pListen)(char *svTarget);
	void *(__cdecl *pConnect)(char *svTarget);
	void *(__cdecl *pAccept)(void *pInternal, char *svAddr, int nMaxLen);
	int (__cdecl *pClose)(void *pInternal);
	int (__cdecl *pRecv)(void *pInternal, BYTE **pInData, int *pnInDataLen);
	int (__cdecl *pSend)(void *pInternal, BYTE *pData, int nDataLen);
	void  (__cdecl *pFree)(void *pInternal, BYTE *pBuffer);
	int (__cdecl *pGetConnectAddr)(void *pInternal, char *svTarget, int nMaxLen);
} IO_HANDLER;

class CIOSocket {
private:
	IO_HANDLER *m_pHandler;
	void *m_pData;
	char m_svRmtAddr[256];

public:
	CIOSocket(IO_HANDLER *pHandler);
	virtual ~CIOSocket();
	virtual int Listen(char *svTarget);
	virtual int Connect(char *svTarget);
	virtual CIOSocket *Accept(void);
	virtual int Close(void);
	virtual int Recv(BYTE **pInData, int *pnInDataLen);
	virtual int Send(BYTE *pData, int nDataLen);
	virtual void Free(BYTE *pBuffer);
	virtual int GetRemoteAddr(char *svAddr,int nMaxLen);
	virtual int GetConnectAddr(char *svAddr,int nMaxLen);
};


#pragma pack(pop)

// IO Handler Manager Functions

class CIOHandler {

private:
	IO_HANDLER *m_IOHandler[MAX_IO_HANDLERS];

public:
	CIOHandler();
	virtual ~CIOHandler();
	virtual int Insert(IO_HANDLER *handler);
	virtual int Remove(int handlernum);
	virtual char *Query(int nHandler);
	virtual int GetHandlerCount(void);
	virtual IO_HANDLER *GetHandler(int nHandler);
	virtual IO_HANDLER *GetHandlerByID(char *svID);
};

#endif