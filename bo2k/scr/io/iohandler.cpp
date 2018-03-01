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
#include<iohandler.h>
#include<commnet.h>

//                                               //
//////////////// IO Handler Class /////////////////
//                                               //

CIOHandler::CIOHandler()
{
	int i;
	for(i=0;i<MAX_IO_HANDLERS;i++)
		m_IOHandler[i]=INVALID_IO_HANDLER;
}

CIOHandler::~CIOHandler()
{
	int i;
	for(i=0;i<MAX_IO_HANDLERS;i++) {
		if(m_IOHandler[i]!=INVALID_IO_HANDLER) {
			m_IOHandler[i]->pRemove();
			m_IOHandler[i]=INVALID_IO_HANDLER;
		}
	}
}

int CIOHandler::Insert(IO_HANDLER *handler)
{	
	int i;

	if(handler->pInsert==NULL) return -1;
	if(handler->pInsert()==-1) return -1;

	for(i=0;i<MAX_IO_HANDLERS;i++) {
		if(m_IOHandler[i]==INVALID_IO_HANDLER) break;
	}
	if(i==MAX_IO_HANDLERS) return -1;
	
	m_IOHandler[i]=handler;
	
	return i;
}

int CIOHandler::GetHandlerCount(void)
{
	int count,i;

	count=0;
	for(i=0;i<MAX_IO_HANDLERS;i++) {
		if(m_IOHandler[i]!=INVALID_IO_HANDLER) count++;
	}
	return count;
}
	
IO_HANDLER *CIOHandler::GetHandler(int nHandler)
{
	if(nHandler<0 || nHandler>=MAX_IO_HANDLERS) return NULL;
	return m_IOHandler[nHandler];
}

IO_HANDLER *CIOHandler::GetHandlerByID(char *svID)
{
	int i;

	for(i=0;i<MAX_IO_HANDLERS;i++) {
		char *svQuery=Query(i);
		if(svQuery!=NULL) {
			if(strnicmp(svQuery,svID,min(lstrlen(svID),lstrlen(svQuery)))==0)
				return m_IOHandler[i];
		}
	}

	return NULL;
}


char *CIOHandler::Query(int nHandler)
{
	if(nHandler<0 || nHandler>=MAX_IO_HANDLERS) return NULL;
	if(m_IOHandler[nHandler]==INVALID_IO_HANDLER) return NULL;
	if(m_IOHandler[nHandler]->pQuery==NULL) return NULL;
	
	return m_IOHandler[nHandler]->pQuery();
}


int CIOHandler::Remove(int nHandler)
{
	int ret;

	if(nHandler<0 || nHandler>=MAX_IO_HANDLERS) return NULL;
	if(m_IOHandler[nHandler]==INVALID_IO_HANDLER) return NULL;
	if(m_IOHandler[nHandler]->pRemove==NULL) return NULL;
	
	ret=m_IOHandler[nHandler]->pRemove();
	
	m_IOHandler[nHandler]=INVALID_IO_HANDLER;
	
	return ret;
}



//                                               //
//////////////// Socket Class /////////////////////
//                                               //


CIOSocket::CIOSocket(IO_HANDLER *pHandler)
{
	m_pHandler=pHandler;
	m_pData=NULL;
	m_svRmtAddr[0]=0;
}

CIOSocket::~CIOSocket()
{

}

int CIOSocket::Listen(char *svTarget)
{
	if(m_pHandler->pListen==NULL) return -1;
	if(m_pData!=NULL) return -1;

	m_pData=m_pHandler->pListen(svTarget);
	if(m_pData==NULL) return -1;

	return 0;
}

int CIOSocket::Connect(char *svTarget)
{
	if(m_pHandler->pConnect==NULL) return -1;
	if(m_pData!=NULL) return -1;

	m_pData=m_pHandler->pConnect(svTarget);
	if(m_pData==NULL) return -1;

	lstrcpyn(m_svRmtAddr,svTarget,256);

	return 0;
}

CIOSocket *CIOSocket::Accept(void)
{
	CIOSocket *cios;
	if(m_pHandler->pAccept==NULL) return NULL;
	if(m_pData==NULL) return NULL;

	void *data;
	char svAddr[256];
	data=m_pHandler->pAccept(m_pData,svAddr,256);
	if(data==NULL) return NULL;

	cios=new CIOSocket(m_pHandler);
	cios->m_pData=data;
	lstrcpyn(cios->m_svRmtAddr,svAddr,256);

	return cios;
}

int CIOSocket::Close(void)
{
	int ret;
	if(m_pHandler->pClose==NULL) return -1;
	if(m_pData==NULL) return 0;
	
	ret=m_pHandler->pClose(m_pData);
	if(ret<0) return ret;

	m_svRmtAddr[0]='\0';

	m_pData=NULL;
	return ret;
}	

int CIOSocket::Recv(BYTE **pInData, int *pnInDataLen)
{
	if(m_pHandler->pRecv==NULL) return -1;
	if(m_pData==NULL) {
		*pInData=NULL;
		*pnInDataLen=0;
		return -1;
	}
	
	return m_pHandler->pRecv(m_pData,pInData,pnInDataLen);
}

int CIOSocket::Send(BYTE *pData, int nDataLen)
{
	if(m_pHandler->pSend==NULL) return -1;
	if(m_pData==NULL) return -1;
	
	return m_pHandler->pSend(m_pData,pData,nDataLen);
}

void CIOSocket::Free(BYTE *pBuffer)
{
	if(m_pHandler->pFree==NULL) return;
	if(m_pData==NULL) return;
	
	m_pHandler->pFree(m_pData,pBuffer);
}

int CIOSocket::GetRemoteAddr(char *svAddr,int nMaxLen)
{
	lstrcpyn(svAddr,m_svRmtAddr,nMaxLen);
	return lstrlen(svAddr);
}

int CIOSocket::GetConnectAddr(char *svAddr,int nMaxLen)
{
	if(m_pHandler->pGetConnectAddr==NULL) return -1;
	if(m_pData==NULL) return -1;

	return m_pHandler->pGetConnectAddr(m_pData,svAddr,nMaxLen);
}