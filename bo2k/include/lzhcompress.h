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

#ifndef __INC_LZHCOMPRESS_H
#define __INC_LZHCOMPRESS_H

#include<windows.h>
#include<limits.h>

#define N       4096    /* buffer size */
#define F       60  /* lookahead buffer size */
#define THRESHOLD   2
#define NIL     N   /* leaf of tree */
#define N_CHAR      (256 - THRESHOLD + F)
                /* kinds of characters (character code = 0..N_CHAR-1) */
#define T       (N_CHAR * 2 - 1)    /* size of table */
#define R       (T - 1)         /* position of root */
#define MAX_FREQ    0x8000      /* updates tree when the */

class CLzhCompress {
private:
	
	unsigned long int  textsize, codesize;

	// LZSS compression

	BYTE text_buf[N + F - 1];
	short match_position, match_length, lson[N + 1], rson[N + 257], dad[N + 1];

	// Huffman coding

	WORD freq[T + 1];	/* frequency table */
	short prnt[T + N_CHAR];	/* pointers to parent nodes, except for the */
							/* elements [T..T + N_CHAR - 1] which are used to get */
							/* the positions of leaves corresponding to the codes. */
	short son[T];				/* pointers to child nodes (son[], son[] + 1) */

	WORD getbuf;
	BYTE getlen;
	
	WORD putbuf;
	BYTE putlen;

	WORD code, len;


	// Various read/write things
	HANDLE hFileIn, hFileOut;
	BYTE *pMemIn, *pMemOut;
	int nMemInSize, nMemOutSize;
	int nMemInOff, nMemOutOff;
	int freeze_type;

protected:

	void InitTree(void);				// initialize trees 
	void InsertNode(short r);			// insert to tree 
	void DeleteNode(short p);			// remove from tree 
	WORD GetBit(void);				// get one bit 
	WORD GetByte(void);				// get one byte 
	void Putcode(short l, WORD c); // output c bits of code 
	void StartHuff(void);
	void reconst(void);
	void update(short c);
	void EncodeChar(WORD c);
	void EncodePosition(WORD c);
	void EncodeEnd(void);
	short DecodeChar(void);
	short DecodePosition(void);
	void Encode(void);				// compression
	void Decode(void);				// recover 

	int fnc_read_file(BYTE *pBuffer, int nLen);
	int fnc_write_file(BYTE *pBuffer, int nLen);
	int fnc_read_memory(BYTE *pBuffer, int nLen);
	int fnc_write_memory(BYTE *pBuffer, int nLen);
	int fnc_write(BYTE *ptr, int len);
	int fnc_read(BYTE *ptr, int len);

	int fnc_getc(void);
	int fnc_putc(int val);
	int lzh_freeze(void);
	
public:
	int lzh_freeze_file(char *szInFile, char *szOutFile);
	int lzh_freeze_memory(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen);
	int lzh_melt_file(char *szInFile, char *szOutFile);
	int lzh_melt_memory(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen);
};

#endif











