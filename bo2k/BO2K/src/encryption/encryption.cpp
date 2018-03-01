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

// Command Input/Output Interface
#include<windows.h>
#include<encryption.h>

//
///////
//////////////////// Encryption Handler /////////////////////////
///////
//


CEncryptionHandler::CEncryptionHandler()
{
	int i;
	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++)
		m_EncryptionEngine[i]=INVALID_ENCRYPTION_ENGINE;
}

CEncryptionHandler::~CEncryptionHandler()
{
	int i;
	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		if(m_EncryptionEngine[i]!=INVALID_ENCRYPTION_ENGINE) {
			m_EncryptionEngine[i]->pRemove();
			m_EncryptionEngine[i]=INVALID_ENCRYPTION_ENGINE;
		}
	}
}

int CEncryptionHandler::Insert(ENCRYPTION_ENGINE *engine)
{	
	int i;

	if(engine->pInsert==NULL) return -1;
	if(engine->pInsert()==-1) return -1;

	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		if(m_EncryptionEngine[i]==INVALID_ENCRYPTION_ENGINE) break;
	}
	if(i==MAX_ENCRYPTION_ENGINES) return -1;
	
	m_EncryptionEngine[i]=engine;
	
	return i;
}

int CEncryptionHandler::GetEngineCount(void)
{
	int count,i;

	count=0;
	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		if(m_EncryptionEngine[i]==INVALID_ENCRYPTION_ENGINE) count++;
	}
	return count;
}
	
ENCRYPTION_ENGINE *CEncryptionHandler::GetEngine(int nEngine)
{
	if(nEngine<0 || nEngine>=MAX_ENCRYPTION_ENGINES) return NULL;
	return m_EncryptionEngine[nEngine];
}

ENCRYPTION_ENGINE *CEncryptionHandler::GetEngineByID(char *svID)
{
	int i;
	
	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		char *svQuery=Query(i);
		if(svQuery!=NULL) {
			if(strnicmp(svQuery,svID,min(lstrlen(svID),lstrlen(svQuery)))==0)
				return m_EncryptionEngine[i];
		}
	}

	return NULL;
}


char *CEncryptionHandler::Query(int nEngine)
{
	if(nEngine<0 || nEngine>=MAX_ENCRYPTION_ENGINES) return NULL;
	if(m_EncryptionEngine[nEngine]==INVALID_ENCRYPTION_ENGINE) return NULL;
	if(m_EncryptionEngine[nEngine]->pQuery==NULL) return NULL;
	
	return m_EncryptionEngine[nEngine]->pQuery();
}


int CEncryptionHandler::Remove(int nEngine)
{
	int ret;

	if(nEngine<0 || nEngine>=MAX_ENCRYPTION_ENGINES) return NULL;
	if(m_EncryptionEngine[nEngine]==INVALID_ENCRYPTION_ENGINE) return NULL;
	if(m_EncryptionEngine[nEngine]->pRemove==NULL) return NULL;
	
	ret=m_EncryptionEngine[nEngine]->pRemove();
	
	m_EncryptionEngine[nEngine]=INVALID_ENCRYPTION_ENGINE;
	
	return ret;
}


//
///////
//////////////////// Encryption Engine /////////////////////////
///////
//



CEncryptionEngine::CEncryptionEngine(ENCRYPTION_ENGINE *pEngine)
{
	m_pEngine=pEngine;
	m_pData=NULL;
}

CEncryptionEngine::~CEncryptionEngine()
{

}

char *CEncryptionEngine::Query(void)
{
	return m_pEngine->pQuery();
}

int CEncryptionEngine::Startup(void)
{
	if(m_pData!=NULL) return -1;
	if(m_pEngine->pStartup==NULL) return 0;
	m_pData=m_pEngine->pStartup();
	if(m_pData==NULL) return -1;
	return 0;
}

int CEncryptionEngine::Shutdown(void)
{
	int ret;
	if(m_pData==NULL) return -1;
	if(m_pEngine->pShutdown==NULL) return 0;
	ret=m_pEngine->pShutdown(m_pData);
	if(ret>=0) m_pData=NULL;

	return ret;
}

int CEncryptionEngine::SetEncryptKey(char *svKey)
{
	if(m_pData==NULL) return -1;
	if(m_pEngine->pSetEncryptKey==NULL) return 0;
	return m_pEngine->pSetEncryptKey(m_pData,svKey);
}

int CEncryptionEngine::SetDecryptKey(char *svKey)
{
	if(m_pData==NULL) return -1;
	if(m_pEngine->pSetDecryptKey==NULL) return 0;
	return m_pEngine->pSetDecryptKey(m_pData,svKey);
}

char *CEncryptionEngine::GetEncryptKey(void)
{
	if(m_pData==NULL) return NULL;
	if(m_pEngine->pGetEncryptKey==NULL) return NULL;
	return m_pEngine->pGetEncryptKey(m_pData);
}

char *CEncryptionEngine::GetDecryptKey(void)
{
	if(m_pData==NULL) return NULL;
	if(m_pEngine->pGetDecryptKey==NULL) return NULL;
	return m_pEngine->pGetDecryptKey(m_pData);
}

BYTE *CEncryptionEngine::Encrypt(BYTE *pBuffer,int nBufLen, int *pnOutBufLen)
{
	if(m_pData==NULL) return NULL;
	if(m_pEngine->pEncrypt==NULL) return NULL;
	return m_pEngine->pEncrypt(m_pData,pBuffer,nBufLen,pnOutBufLen);
}

BYTE *CEncryptionEngine::Decrypt(BYTE *pBuffer,int nBufLen, int *pnOutBufLen)
{
	if(m_pData==NULL) return NULL;
	if(m_pEngine->pDecrypt==NULL) return NULL;
	return m_pEngine->pDecrypt(m_pData,pBuffer,nBufLen,pnOutBufLen);
}

int CEncryptionEngine::CreateNewKeys(void)
{
	if(m_pData==NULL) return -1;
	if(m_pEngine->pCreateNewKeys==NULL) return -1;
	return m_pEngine->pCreateNewKeys(m_pData);
}

void CEncryptionEngine::Free(BYTE *pBuffer)
{
	if(m_pData==NULL) return;
	if(m_pEngine->pFree==NULL) return;
	m_pEngine->pFree(m_pData, pBuffer);
}
