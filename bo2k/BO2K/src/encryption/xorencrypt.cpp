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

#include<windows.h>
#include<encryption.h>
#include<config.h>

#define LROTL(dw,c) (((DWORD)dw<<c) | ((DWORD)dw>>(32-c)))

#ifdef NDEBUG
char g_szXOREncryptOptions[]="<**CFG**>XOR\0"
                             "S[45]:XOR Key=\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#else
char g_szXOREncryptOptions[]="<**CFG**>XOR\0"
                             "S[45]:XOR Key=No rest for the wicked.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#endif

ENCRYPTION_ENGINE g_XORengine={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

typedef struct {
	DWORD dwXORKey;
	char svXORKey[46];
} XORENCRYPT_DATA;

int __cdecl XOREncrypt_Insert(void);
int __cdecl XOREncrypt_Remove(void);
char * __cdecl XOREncrypt_Query(void);

void * __cdecl XOREncrypt_Startup(void);
int __cdecl XOREncrypt_Shutdown(void *pInternal);
int __cdecl XOREncrypt_SetEncryptKey(void *pInternal, char *pKey);
int __cdecl XOREncrypt_SetDecryptKey(void *pInternal, char *pKey);
char * __cdecl XOREncrypt_GetEncryptKey(void *pInternal);
char * __cdecl XOREncrypt_GetDecryptKey(void *pInternal);
BYTE * __cdecl XOREncrypt_Encrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
BYTE * __cdecl XOREncrypt_Decrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
int __cdecl XOREncrypt_CreateNewKeys(void *pInternal);
void __cdecl XOREncrypt_Free(void *pInternal, BYTE *pBuffer);


int __cdecl XOREncrypt_Insert(void)
{
	char *svKey=GetCfgStr(g_szXOREncryptOptions,"XOR Key");
	if(svKey==NULL) return -1;

	return 0;
}

int __cdecl XOREncrypt_Remove(void)
{
	return 0;
}

char * __cdecl XOREncrypt_Query(void)
{
	return "XOR: BO2K Simple XOR Encryption";
}


void * __cdecl XOREncrypt_Startup(void)
{
	char *svKey=GetCfgStr(g_szXOREncryptOptions,"XOR Key");
	if(svKey==NULL) return NULL;
	if(svKey[0]=='\0') return NULL;

	XORENCRYPT_DATA *data;
	data=(XORENCRYPT_DATA *)malloc(sizeof(XORENCRYPT_DATA));
	if(data==NULL) return NULL;
	
	XOREncrypt_SetEncryptKey((void *)data,svKey);

	return data;
}

int __cdecl XOREncrypt_Shutdown(void *pInternal)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	free(data);
	return 0;
}

int __cdecl XOREncrypt_SetEncryptKey(void *pInternal, char *svKey)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;

	char *svKeyPtr=svKey;
	DWORD key;
	key=0xCDC31337;
	while((*svKeyPtr)!=0) {
		key=LROTL(key,8);
		key+=(*svKeyPtr);
		svKeyPtr++;
	}

	data->dwXORKey=key;
	lstrcpyn(data->svXORKey,svKey,46);

	return 0;
}

int __cdecl XOREncrypt_SetDecryptKey(void *pInternal, char *svKey)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	
	char *svKeyPtr=svKey;
	DWORD key;
	key=0xCDC31337;
	while((*svKeyPtr)!=0) {
		key=LROTL(key,8);
		key+=(*svKeyPtr);
		svKeyPtr++;
	}

	data->dwXORKey=key;
	lstrcpyn(data->svXORKey,svKey,46);
	
	return 0;
}

char * __cdecl XOREncrypt_GetEncryptKey(void *pInternal)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	
	return data->svXORKey;
}

char * __cdecl XOREncrypt_GetDecryptKey(void *pInternal)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	
	return data->svXORKey;
}

BYTE * __cdecl XOREncrypt_Encrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	BYTE *buf;
	int i;

	buf=(BYTE *)malloc(nBufLen);
	if(buf==NULL) return NULL;

	for(i=0;i<nBufLen;i++) {
		buf[i]=(pBuffer[i] ^ (BYTE)(data->dwXORKey >> (8*(i&3))) );
	}

	*pnOutBufLen=nBufLen;
	return buf;
}

BYTE * __cdecl XOREncrypt_Decrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;
	BYTE *buf;
	int i;

	buf=(BYTE *)malloc(nBufLen);
	if(buf==NULL) return NULL;

	for(i=0;i<nBufLen;i++) {
		buf[i]=(pBuffer[i] ^ (BYTE)(data->dwXORKey >> (8*(i&3))) );
	}

	*pnOutBufLen=nBufLen;
	return buf;
}

int __cdecl XOREncrypt_CreateNewKeys(void *pInternal)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;

	return 0;
}

void __cdecl XOREncrypt_Free(void *pInternal, BYTE *pBuffer)
{
	XORENCRYPT_DATA *data=(XORENCRYPT_DATA *)pInternal;

	free(pBuffer);
}



ENCRYPTION_ENGINE *GetXOREncryptionEngine(void)
{
	g_XORengine.pInsert=XOREncrypt_Insert;
	g_XORengine.pRemove=XOREncrypt_Remove;
	g_XORengine.pQuery=XOREncrypt_Query;
	
	g_XORengine.pStartup=XOREncrypt_Startup;
	g_XORengine.pShutdown=XOREncrypt_Shutdown;
	g_XORengine.pSetEncryptKey=XOREncrypt_SetEncryptKey;
	g_XORengine.pSetDecryptKey=XOREncrypt_SetDecryptKey;
	g_XORengine.pGetEncryptKey=XOREncrypt_GetEncryptKey;
	g_XORengine.pGetDecryptKey=XOREncrypt_GetDecryptKey;
	g_XORengine.pEncrypt=XOREncrypt_Encrypt;
	g_XORengine.pDecrypt=XOREncrypt_Decrypt;
	g_XORengine.pCreateNewKeys=XOREncrypt_CreateNewKeys;
	g_XORengine.pFree=XOREncrypt_Free;
	
	return &g_XORengine;
}


