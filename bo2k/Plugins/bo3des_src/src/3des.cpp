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
#include<stdio.h>
#include<encryption.h>
#include<config.h>
#include "global.h"
#include "md5.h"

// ---------- Tables defined in the Data Encryption Standard documents ----------------------------

// Initial permutation IP
static BYTE ip[] = {
	58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6,
    64, 56, 48, 40, 32, 24, 16,  8,
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7
};

// Final permutation IP^-1
static BYTE fp[] = {
	40,  8, 48, 16, 56, 24, 64, 32,
    39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30,
    37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28,
    35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26,
    33,  1, 41,  9, 49, 17, 57, 25
};

// permuted choice table (key)
static BYTE pc1[] = {
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,	
    63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
};

// number left rotations of pc1
static BYTE totrot[] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

// permuted choice key (table)
static BYTE pc2[] = {
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// The s boxes
static BYTE si[8][64] = {
	// S1
	{  14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
		0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
		4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
       15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 },
		
    // S2
	{  15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
		3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
		0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	   13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 },
	
	// S3
	{  10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	   13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	   13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	    1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 },
	
	// S4
	{   7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	   13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	   10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	    3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 },
	
	// S5 
	{   2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
       14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	    4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	   11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 },
	
	// S6 
	{  12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	   10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	    9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	    4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 },
	
	// S7
	{   4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	   13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	    1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	    6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 },
	
	// S8
	{  13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	    1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	    7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	    2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 }
};

// 32-bit permutation function P used on the output of the S-boxes
static BYTE p32i[] = {
	16,  7, 20, 21,
    29, 12, 28, 17,
     1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
    32, 27,  3,  9,
    19, 13, 30,  6,
    22, 11,  4, 25
};

// ---- Global Variables ------------------------------------------------

char g_szTripleDESOptions[]="<**CFG**>3DES\0"
                             "S[45]:3DES Key String=\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

ENCRYPTION_ENGINE g_TripleDESengine={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

DWORD sp[8][64];              // Combined S and P boxes
BYTE iperm[16][16][8];       // Initial permutations
BYTE fperm[16][16][8];       // Final permutations
int bytebit[] = { 0200,0100,040,020,010,04,02,01 };
int nibblebit[] = { 010,04,02,01 };
	
// ---- Structures ------------------------------------------------------

typedef struct {
	BYTE kn1[16][8];     // 8 6-bit subkeys for each of 16 initialized by des_setkey()
	BYTE kn2[16][8];     // 8 6-bit subkeys for each of 16 initialized by des_setkey()
	BYTE kn3[16][8];     // 8 6-bit subkeys for each of 16 initialized by des_setkey()
	BYTE enc_lastblock[8]; // last encrypted block
	BYTE dec_lastblock[8]; // last decrypted block
	BYTE enc_key[46];
	BYTE dec_key[46];
} TRIPLEDES_DATA;

// ---- Function Prototypes ---------------------------------------------

int __cdecl TripleDES_Insert(void);
int __cdecl TripleDES_Remove(void);
char * __cdecl TripleDES_Query(void);

void * __cdecl TripleDES_Startup(void);
int __cdecl TripleDES_Shutdown(void *pInternal);
int __cdecl TripleDES_SetEncryptKey(void *pInternal, char *pKey);
int __cdecl TripleDES_SetDecryptKey(void *pInternal, char *pKey);
char * __cdecl TripleDES_GetEncryptKey(void *pInternal);
char * __cdecl TripleDES_GetDecryptKey(void *pInternal);
BYTE * __cdecl TripleDES_Encrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
BYTE * __cdecl TripleDES_Decrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen);
int __cdecl TripleDES_CreateNewKeys(void *pInternal);
void __cdecl TripleDES_Free(void *pInternal, BYTE *pBuffer);

 
// ---- Function Declarations -------------------------------------------

#ifndef  BIG_ENDIAN
// Byte swap a long 
DWORD byteswap(DWORD x)
{
	char *cp,tmp;
	cp = (char *)&x;
	tmp = cp[3];
	cp[3] = cp[0];
	cp[0] = tmp;
	tmp = cp[2];
	cp[2] = cp[1];
	cp[1] = tmp;
	return x;
}
#endif

// ---- initialize a perm array ----
static void perminit(BYTE perm[16][16][8], BYTE p[64])
{
	int i,j,k,l,m;
	
	// Clear the permutation array
	for (i=0; i<16; i++) {
		for (j=0; j<16; j++) {
			// Clear permutation
			for (k=0; k<8; k++) {
				perm[i][j][k]=0;
			}

			// each input nibble position
			for (i=0; i<16; i++) {
				// each possible input nibble
				for (j = 0; j < 16; j++) {
					// each output bit position
					for (k = 0; k < 64; k++) {
						// where does this bit come from
						l = p[k] - 1; 
						
						// does it come from input posn
						if ((l >> 2) != i) continue;     // if not, bit k is 0
						
						// any such bit in input?
						if (!(j & nibblebit[l & 3])) continue;     

						// which bit is this in the byte?
						m = k & 07;   
						perm[i][j][k>>3] |= (char)bytebit[m];
					}
				}
			}
		}
	}
}

// ---- Initialize the lookup table for the combined S and P boxes ----
static void spinit()
{
	BYTE pbox[32];
	int p,i,s,j,rowcol;
	DWORD val;
	
	// Compute pbox, the inverse of p32i.
	// This is easier to work with
	
	for(p=0;p<32;p++){
		for(i=0;i<32;i++){
			if(p32i[i]-1 == p){
				pbox[p] = (char)i;
				break;
			}
		}
	}

	// For each S-box
	for(s = 0; s < 8; s++) {  
		// For each possible input
		for(i=0; i<64; i++) {
			val = 0;
			
			// The row number is formed from the first and last
			// bits; the column number is from the middle 4
			
			rowcol = (i & 32) | ((i & 1) ? 16 : 0) | ((i >> 1) & 0xf);
			for(j=0;j<4;j++) {       // For each output bit
				if(si[s][rowcol] & (8 >> j)){
					val |= 1L << (31 - pbox[4*s + j]);
				}
			}
			sp[s][i] = val;
		}
	}
}


// permute: takes an input block, passes it through a permutation
//          (if desmode == 0) and returns an output block
void des_permute(BYTE *inblock, BYTE perm[16][16][8], BYTE *outblock)
{
	int i,j;
	BYTE *ib,*ob,*p,*q;

	// Clear Output block
	memset(outblock, 0, 8*sizeof(BYTE));
	
	// Perform permutation
	ib = inblock;
	for (j = 0; j < 16; j += 2, ib++) { // for each input nibble
		ob = outblock;
		p = perm[j][(*ib >> 4) & 017];
		q = perm[j + 1][*ib & 017];
		for (i = 8; i != 0; i--){   // and each output byte 
			*ob++ |= *p++ | *q++;   // OR the masks together
		}
	}
}

// The nonlinear function f(r,k), the heart of DES
static DWORD f(DWORD r, BYTE subkey[8])
{
	DWORD rval,rt;
	
	// Run E(R) ^ K through the combined S & P boxes
	// This code takes advantage of a convenient regularity in
	// E, namely that each group of 6 bits in E(R) feeding
	// a single S-box is a contiguous segment of R.
	
	rt = (r >> 1) | ((r & 1) ? 0x80000000 : 0);
	rval = 0;
	rval |= sp[0][((rt >> 26) ^ *subkey++) & 0x3f];
	rval |= sp[1][((rt >> 22) ^ *subkey++) & 0x3f];
	rval |= sp[2][((rt >> 18) ^ *subkey++) & 0x3f];
	rval |= sp[3][((rt >> 14) ^ *subkey++) & 0x3f];
	rval |= sp[4][((rt >> 10) ^ *subkey++) & 0x3f];
	rval |= sp[5][((rt >> 6) ^ *subkey++) & 0x3f];
	rval |= sp[6][((rt >> 2) ^ *subkey++) & 0x3f];
	rt = (r << 1) | ((r & 0x80000000) ? 1 : 0);
	rval |= sp[7][(rt ^ *subkey) & 0x3f];
	
	return rval;
}

// round: Do one DES cipher round 
void des_round(int num, DWORD *block, BYTE kn[16][8])
{
	// The rounds are numbered from 0 to 15. On even rounds
	// the right half is fed to f() and the result exclusive-ORs
	// the left half; on odd rounds the reverse is done.
	
	if(num & 1)
		block[1] ^= f(block[0],kn[num]);
	else 
		block[0] ^= f(block[1],kn[num]);
}

// In-place encryption of 64-bit block
void des_endes(BYTE *block, BYTE kn[16][8])
{
	int i;
	DWORD work[2], tmp;

	// Initial Permutation
	des_permute(block, iperm, (BYTE *)work);   
#ifndef BIG_ENDIAN
	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);
#endif
	
	// Do the 16 rounds
	for (i=0; i<16; i++) des_round(i,work, kn);
	
	// Left/right half swap
	tmp = work[0];
	work[0] = work[1];
	work[1] = tmp;

	// Inverse initial permutation
#ifndef BIG_ENDIAN
	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);
#endif	
	des_permute((BYTE *)work, fperm, block);
}

// In-place decryption of 64-bit block 
void des_dedes(BYTE *block, BYTE kn[16][8])
{
	int i;
	DWORD work[2], tmp;

	// Initial permutation
	des_permute(block,iperm,(BYTE *)work);    
#ifndef BIG_ENDIAN
	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);
#endif

	// Left/right half swap
	tmp = work[0];
	work[0] = work[1];
	work[1] = tmp;
	
	// Do the 16 rounds in reverse order
	for (i=15; i >= 0; i--)
		des_round(i,work, kn);

	// Inverse initial permutation
#ifndef BIG_ENDIAN
	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);
#endif
	des_permute((BYTE *)work,fperm,block);    
}

// setkey:
// initializes key schedule array
// key is 64 bits (will use only 56)
void des_setkey(BYTE *key, BYTE kn[16][8])
{
	BYTE pc1m[56];              // place to modify pc1 into 
	BYTE pcr[56];               // place to rotate pc1 into 
	register int i,j,l,m;
	
	// Clear key schedule
	for (i=0; i<16; i++) {
		for (j=0; j<8; j++) {
			kn[i][j]=0;
		}
	}
		 
	// Convert pc1 to bits of key 
	for(j=0; j<56; j++) {  
		l=pc1[j]-1;                     // integer bit location  
		m = l & 07;                     // find bit              
		pc1m[j]= (char)((key[l>>3] &    // find which key byte l is in 
			bytebit[m])                 // and which bit of that byte 
			? 1 : 0);                   // and store 1-bit result
	}
	
	// Key chunk for each iteration
    for (i=0; i<16; i++) { 
		
		// Rotate pc1 the right amount
		for (j=0; j<56; j++)    
			pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];		
		
		// Rotate left and right halves independently
		for (j=0; j<48; j++) {   // select bits individually
			
			// check bit that goes to kn[j]
			if (pcr[pc2[j]-1]) {
				// mask it in if it's there
				l= j % 6;
				kn[i][j/6] |= (BYTE)(bytebit[l] >> 2);
			}
		}
	}

}

void des_str_to_key(BYTE *str, BYTE *key)
{
	int i;
	key[0] = (str[0]>>1);
	key[1] = ((str[0]&0x01)<<6) | (str[1]>>2);
	key[2] = ((str[1]&0x03)<<5) | (str[2]>>3);
	key[3] = ((str[2]&0x07)<<4) | (str[3]>>4);
	key[4] = ((str[3]&0x0F)<<3) | (str[4]>>5);
	key[5] = ((str[4]&0x1F)<<2) | (str[5]>>6);
	key[6] = ((str[5]&0x3F)<<1) | (str[6]>>7);
	key[7] = (str[6]&0x7F);
	for (i=0;i<8;i++)
		key[i] = (BYTE)(key[i]<<1);
}

void xorblock(BYTE *block, BYTE *xorbuf)
{
	register int i;
	for(i=0;i<8;i++) {
		block[i]^=xorbuf[i];
	}
}


int __cdecl TripleDES_Insert(void)
{
	spinit();
	perminit(iperm, ip);
	perminit(fperm, fp);

	return 0;
}

int __cdecl TripleDES_Remove(void)
{
	
	
	return 0;
}

char * __cdecl TripleDES_Query(void)
{
	return "3DES: BO2K Triple-DES Encryption";
}


void * __cdecl TripleDES_Startup(void)
{
	char *svKey=GetCfgStr(g_szTripleDESOptions,"3DES Key String");
	if(svKey==NULL) return NULL;
	if(svKey[0]=='\0') return NULL;

	TRIPLEDES_DATA *data;
	data=(TRIPLEDES_DATA *)malloc(sizeof(TRIPLEDES_DATA));
	if(data==NULL) return NULL;
	
	TripleDES_SetEncryptKey(data,svKey);
	TripleDES_SetDecryptKey(data,svKey);
	
	return data;
}

int __cdecl TripleDES_Shutdown(void *pInternal)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	free(data);
	return 0;
}

int __cdecl TripleDES_SetEncryptKey(void *pInternal, char *svKey)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	MD5_CTX ctx;
	BYTE digest1[16],digest2[16];
	BYTE key[8];
	DWORD dwLen;
	char svKeyStr1[]="1 h4v3 4 b1g butt 4nd my butt sm3llz";
	char svKeyStr2[]="4nd I l1k3 t0 sm3ll my 0wn butt.";
		
	dwLen=lstrlen(svKey);
	lstrcpyn((char *)data->enc_key,svKey,46);

	// Create two digests to form the key
	MD5Init(&ctx);
	MD5Update(&ctx,(BYTE *)svKeyStr1,lstrlen(svKeyStr1));
	MD5Update(&ctx,(BYTE *)svKey,dwLen);
	MD5Final(digest1,&ctx);

	MD5Init(&ctx);
	MD5Update(&ctx,(BYTE *)svKeyStr2,lstrlen(svKeyStr2));
	MD5Update(&ctx,(BYTE *)svKey,dwLen);
	MD5Final(digest2,&ctx);

	// Convert first schedule
	des_str_to_key((BYTE *)digest1,key);
	des_setkey(key,data->kn1);
	// Convert second schedule
	des_str_to_key((BYTE *)digest1+7,key);
	des_setkey(key,data->kn2);
	// Convert third schedule
	des_str_to_key((BYTE *)digest2,key);
	des_setkey(key,data->kn3);

	memset(data->enc_lastblock,0,8);

	return 0;
}

int __cdecl TripleDES_SetDecryptKey(void *pInternal, char *svKey)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	MD5_CTX ctx;
	BYTE digest1[16],digest2[16];
	BYTE key[8];
	DWORD dwLen;
	char svKeyStr1[]="1 h4v3 4 b1g butt 4nd my butt sm3llz";
	char svKeyStr2[]="4nd I l1k3 t0 sm3ll my 0wn butt.";
		
	dwLen=lstrlen(svKey);
	lstrcpyn((char *)data->dec_key,svKey,46);

	// Create two digests to form the key
	MD5Init(&ctx);
	MD5Update(&ctx,(BYTE *)svKeyStr1,lstrlen(svKeyStr1));
	MD5Update(&ctx,(BYTE *)svKey,dwLen);
	MD5Final(digest1,&ctx);

	MD5Init(&ctx);
	MD5Update(&ctx,(BYTE *)svKeyStr2,lstrlen(svKeyStr2));
	MD5Update(&ctx,(BYTE *)svKey,dwLen);
	MD5Final(digest2,&ctx);

	// Convert first schedule
	des_str_to_key((BYTE *)digest1,key);
	des_setkey(key,data->kn1);
	// Convert second schedule
	des_str_to_key((BYTE *)digest1+7,key);
	des_setkey(key,data->kn2);
	// Convert third schedule
	des_str_to_key((BYTE *)digest2,key);
	des_setkey(key,data->kn3);

	memset(data->dec_lastblock,0,8);
	return 0;
}

char * __cdecl TripleDES_GetEncryptKey(void *pInternal)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	
	return (char *) data->enc_key;
}

char * __cdecl TripleDES_GetDecryptKey(void *pInternal)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	
	return (char *) data->dec_key;
}

BYTE * __cdecl TripleDES_Encrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	BYTE *buf;
	BYTE encblock[8];
	int nOutBufLen,i;

	if(nBufLen&7) nOutBufLen=(nBufLen&~7)+8+8;
	else nOutBufLen=nBufLen+8;

	buf=(BYTE *)malloc(nOutBufLen);
	if(buf==NULL) return NULL;
	
	memset(buf,0,nOutBufLen);
	memcpy(buf,pBuffer,nBufLen);
	*(int *)(buf+(nOutBufLen-8))=nBufLen;
	
	memcpy(encblock,data->enc_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		xorblock(buf+i,encblock);
		des_endes(buf+i,data->kn1);
		memcpy(encblock,buf+i,8);
	}
	memcpy(encblock,data->enc_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		xorblock(buf+i,encblock);
		des_dedes(buf+i,data->kn2);
		memcpy(encblock,buf+i,8);
	}
	memcpy(encblock,data->enc_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		xorblock(buf+i,encblock);
		des_endes(buf+i,data->kn3);
		memcpy(encblock,buf+i,8);
	}
	memcpy(data->enc_lastblock,encblock,8);
	
	*pnOutBufLen=nOutBufLen;
	return buf;
}

BYTE * __cdecl TripleDES_Decrypt(void *pInternal, BYTE *pBuffer,int nBufLen,int *pnOutBufLen)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	BYTE *buf;
	int nOutBufLen,i;
	BYTE decblock[8],decblock2[8],decblocklast[8];

	if(nBufLen&7) nOutBufLen=(nBufLen&~7)+8;
	else nOutBufLen=nBufLen;

	buf=(BYTE *)malloc(nOutBufLen);
	if(buf==NULL) return NULL;
	
	memset(buf,0,nOutBufLen);
	memcpy(buf,pBuffer,nBufLen);

	memcpy(decblocklast,buf+(nOutBufLen-8),8);

	memcpy(decblock,data->dec_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		memcpy(decblock2,buf+i,8);
		des_dedes(buf+i,data->kn3);
		xorblock(buf+i,decblock);
		memcpy(decblock,decblock2,8);
	}
	memcpy(decblock,data->dec_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		memcpy(decblock2,buf+i,8);
		des_endes(buf+i,data->kn2);
		xorblock(buf+i,decblock);
		memcpy(decblock,decblock2,8);
	}
	memcpy(decblock,data->dec_lastblock,8);
	for(i=0;i<nOutBufLen;i+=8) {
		memcpy(decblock2,buf+i,8);
		des_dedes(buf+i,data->kn1);
		xorblock(buf+i,decblock);
		memcpy(decblock,decblock2,8);
	}

	memcpy(data->dec_lastblock,decblocklast,8);

	int nRealLen=*(int *)(buf+(nOutBufLen-8));

	*pnOutBufLen=nRealLen;
	return buf;
}

int __cdecl TripleDES_CreateNewKeys(void *pInternal)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;	
	
	char *svKey=GetCfgStr(g_szTripleDESOptions,"3DES Key String");

	TripleDES_SetEncryptKey(data,svKey);
	TripleDES_SetDecryptKey(data,svKey);

	return 0;
}

void __cdecl TripleDES_Free(void *pInternal, BYTE *pBuffer)
{
	TRIPLEDES_DATA *data=(TRIPLEDES_DATA *)pInternal;
	
	free(pBuffer);
}



ENCRYPTION_ENGINE *GetTripleDESEngine(void)
{
	g_TripleDESengine.pInsert=TripleDES_Insert;
	g_TripleDESengine.pRemove=TripleDES_Remove;
	g_TripleDESengine.pQuery=TripleDES_Query;
	
	g_TripleDESengine.pStartup=TripleDES_Startup;
	g_TripleDESengine.pShutdown=TripleDES_Shutdown;
	g_TripleDESengine.pSetEncryptKey=TripleDES_SetEncryptKey;
	g_TripleDESengine.pSetDecryptKey=TripleDES_SetDecryptKey;
	g_TripleDESengine.pGetEncryptKey=TripleDES_GetEncryptKey;
	g_TripleDESengine.pGetDecryptKey=TripleDES_GetDecryptKey;
	g_TripleDESengine.pEncrypt=TripleDES_Encrypt;
	g_TripleDESengine.pDecrypt=TripleDES_Decrypt;
	g_TripleDESengine.pCreateNewKeys=TripleDES_CreateNewKeys;
	g_TripleDESengine.pFree=TripleDES_Free;
	
	return &g_TripleDESengine;
}


