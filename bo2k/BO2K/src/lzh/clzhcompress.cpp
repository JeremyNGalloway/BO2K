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
#include<limits.h>
#include<lzhcompress.h>


void CLzhCompress::InitTree(void)  /* initialize trees */
{
    short  i;
	
    for (i = N + 1; i <= N + 256; i++)
        rson[i] = NIL;        /* root */
    for (i = 0; i < N; i++)
        dad[i] = NIL;         /* node */
}

void CLzhCompress::InsertNode(short r)  /* insert to tree */
{
    short  i, p, cmp;
    unsigned char  *key;
    unsigned c;
	
    cmp = 1;
    key = &text_buf[r];
    p = N + 1 + key[0];
    rson[r] = lson[r] = NIL;
    match_length = 0;
    for(;;) {
        if (cmp >= 0) {
            if (rson[p] != NIL)
                p = rson[p];
            else {
                rson[p] = r;
                dad[r] = p;
                return;
            }
        } else {
            if (lson[p] != NIL)
                p = lson[p];
            else {
                lson[p] = r;
                dad[r] = p;
                return;
            }
        }
        for (i = 1; i < F; i++)
            if ((cmp = key[i] - text_buf[p + i]) != 0)
                break;
			if (i > THRESHOLD) {
				if (i > match_length) {
					match_position = ((r - p) & (N - 1)) - 1;
					if ((match_length = i) >= F)
						break;
				}
				if (i == match_length) {
					if ((c = ((r - p) & (N-1)) - 1) < (WORD)match_position) {
						match_position = c;
					}
				}
			}
    }
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
        rson[dad[p]] = r;
    else
        lson[dad[p]] = r;
    dad[p] = NIL; /* remove p */
}

void CLzhCompress::DeleteNode(short p)  /* remove from tree */
{
    short  q;
	
    if (dad[p] == NIL)
        return;         /* not registered */
    if (rson[p] == NIL)
        q = lson[p];
    else {
		if (lson[p] == NIL)
			q = rson[p];
		else {
			q = lson[p];
			if (rson[q] != NIL) {
				do {
					q = rson[q];
				} while (rson[q] != NIL);
				rson[dad[q]] = lson[q];
				dad[lson[q]] = dad[q];
				lson[q] = lson[p];
				dad[lson[p]] = q;
			}
			rson[q] = rson[p];
			dad[rson[p]] = q;
		}
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = NIL;
}

/* Huffman coding */
/* table for encoding and decoding the upper 6 bits of position */
/* for encoding */
BYTE p_len[64] = {
    0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

BYTE p_code[64] = {
    0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
		0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
		0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
		0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
		0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
		0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
		0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
		0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* for decoding */
BYTE d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
		0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
		0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
		0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
		0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
		0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
		0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
		0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
		0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
		0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
		0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
		0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
		0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
		0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
		0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
		0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

BYTE d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};


WORD CLzhCompress::GetBit(void)    /* get one bit */
{
    WORD i;
	
    while (getlen <= 8) {
        if ((i = fnc_getc())==0xFFFF) i = 0;
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 1;
    getlen--;
    return ((i & 0x8000) >> 15);
}

WORD CLzhCompress::GetByte(void)   /* get one byte */
{
    WORD i;
	
    while (getlen <= 8) {
        if ((i = fnc_getc())==0xFFFF) i = 0;
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 8;
    getlen -= 8;
    return ((i & 0xff00) >> 8);
}

void CLzhCompress::Putcode(short l, WORD c)     /* output c bits of code */
{
    putbuf |= c >> putlen;
    if ((putlen += l) >= 8) {
        if(fnc_putc(putbuf >> 8)==-1) return;
        
        if((putlen -= 8) >= 8) {
            if(fnc_putc(putbuf)==-1) return;
      
            codesize += 2;
            putlen -= 8;
            putbuf = c << (l - putlen);
        } else {
            putbuf <<= 8;
            codesize++;
        }
    }
}


/* initialization of tree */

void CLzhCompress::StartHuff(void)
{
    short i, j;
	
    for (i = 0; i < N_CHAR; i++) {
        freq[i] = 1;
        son[i] = i + T;
        prnt[i + T] = i;
    }
    i = 0; j = N_CHAR;
    while (j <= R) {
        freq[j] = freq[i] + freq[i + 1];
        son[j] = i;
        prnt[i] = prnt[i + 1] = j;
        i += 2; j++;
    }
    freq[T] = 0xffff;
    prnt[R] = 0;
}


/* reconstruction of tree */

void CLzhCompress::reconst(void)
{
    short i, j, k;
    WORD f, l;
	
    /* collect leaf nodes in the first half of the table */
    /* and replace the freq by (freq + 1) / 2. */
    j = 0;
    for (i = 0; i < T; i++) {
        if (son[i] >= T) {
            freq[j] = (freq[i] + 1) / 2;
            son[j] = son[i];
            j++;
        }
    }
    /* begin constructing tree by connecting sons */
    for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
        k = i + 1;
        f = freq[j] = freq[i] + freq[k];
        for (k = j - 1; f < freq[k]; k--);
        k++;
        l = (j - k) * 2;
        memmove(&freq[k + 1], &freq[k], l);
        freq[k] = f;
        memmove(&son[k + 1], &son[k], l);
        son[k] = i;
    }
    /* connect prnt */
    for (i = 0; i < T; i++) {
        if ((k = son[i]) >= T) {
            prnt[k] = i;
        } else {
            prnt[k] = prnt[k + 1] = i;
        }
    }
}


/* increment frequency of given code by one, and update tree */

void CLzhCompress::update(short c)
{
    short i, j, k, l;
	
    if (freq[R] == MAX_FREQ) {
        reconst();
    }
    c = prnt[c + T];
    do {
        k = ++freq[c];
		
        /* if the order is disturbed, exchange nodes */
        if ((WORD)k > freq[l = c + 1]) {
            while ((WORD)k > freq[++l]);
            l--;
            freq[c] = freq[l];
            freq[l] = k;
			
            i = son[c];
            prnt[i] = l;
            if (i < T) prnt[i + 1] = l;
			
            j = son[l];
            son[l] = i;
			
            prnt[j] = c;
            if (j < T) prnt[j + 1] = c;
            son[c] = j;
			
            c = l;
        }
    } while ((c = prnt[c]) != 0); /* repeat up to root */
}


void CLzhCompress::EncodeChar(WORD c)
{
    WORD i;
    short j, k;
	
    i = 0;
    j = 0;
    k = prnt[c + T];
	
    /* travel from leaf to root */
    do {
        i >>= 1;
		
        /* if node's address is odd-numbered, choose bigger brother node */
        if (k & 1) i += 0x8000;
		
        j++;
    } while ((k = prnt[k]) != R);
    Putcode(j, i);
    code = i;
    len = j;
    update(c);
}

void CLzhCompress::EncodePosition(WORD c)
{
    WORD i;
	
    /* output upper 6 bits by table lookup */
    i = c >> 6;
    Putcode(p_len[i], (WORD)p_code[i] << 8);
	
    /* output lower 6 bits verbatim */
    Putcode(6, (c & 0x3f) << 10);
}

void CLzhCompress::EncodeEnd(void)
{
    if (putlen) {
        if (fnc_putc(putbuf >> 8) == -1) {
			return;
        }
        codesize++;
    }
}

short CLzhCompress::DecodeChar(void)
{
    WORD c;
	
    c = son[R];
	
    /* travel from root to leaf, */
    /* choosing the smaller child node (son[]) if the read bit is 0, */
    /* the bigger (son[]+1} if 1 */
    while (c < T) {
        c += GetBit();
        c = son[c];
    }
    c -= T;
    update(c);
    return (short)c;
}

short CLzhCompress::DecodePosition(void)
{
    WORD i, j, c;
	
    /* recover upper 6 bits from table */
    i = GetByte();
    c = (WORD)d_code[i] << 6;
    j = d_len[i];
	
    /* read lower 6 bits verbatim */
    j -= 2;
    while (j--) {
        i = (i << 1) + GetBit();
    }
    return (short)(c | (i & 0x3f));
}

/* compression */

void CLzhCompress::Encode(void)  /* compression */
{
    short  i, c, len, r, s, last_match_length;
	
	codesize=0;
    fnc_putc((BYTE)(textsize & 0xff));
    fnc_putc((BYTE)((textsize & 0xff00) >> 8));
    fnc_putc((BYTE)((textsize & 0xff0000L) >> 16));
    fnc_putc((BYTE)((textsize & 0xff000000L) >> 24));
    
	if (textsize == 0)
        return;

    textsize = 0;           /* rewind and re-read */
    StartHuff();
    InitTree();
    s = 0;
    r = N - F;
    for (i = s; i < r; i++)
        text_buf[i] = 0x20;
    for (len = 0; len < F && ((c = fnc_getc()) != -1); len++)
        text_buf[r + len] = (unsigned char)c;
    textsize = len;
    for (i = 1; i <= F; i++)
        InsertNode(r - i);
    InsertNode(r);
    do {
        if (match_length > len)
            match_length = len;
        if (match_length <= THRESHOLD) {
            match_length = 1;
            EncodeChar(text_buf[r]);
        } else {
            EncodeChar(255 - THRESHOLD + match_length);
            EncodePosition(match_position);
        }
        last_match_length = match_length;
        for (i = 0; i < last_match_length &&
			((c = fnc_getc()) != -1); i++) {
            DeleteNode(s);
            text_buf[s] = (unsigned char)c;
            if (s < F - 1)
                text_buf[s + N] = (unsigned char)c;
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            InsertNode(r);
        }
        while (i++ < last_match_length) {
            DeleteNode(s);
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            if (--len) InsertNode(r);
        }
    } while (len > 0);

    EncodeEnd();
}

void CLzhCompress::Decode(void)  /* recover */
{
    short  i, j, k, r, c;
    unsigned long int  count;
	
    codesize=0;
	textsize = (fnc_getc());
    textsize |= (((unsigned long)fnc_getc()) << 8);
    textsize |= (((unsigned long)fnc_getc()) << 16);
    textsize |= (((unsigned long)fnc_getc()) << 24);
    
	if (textsize == 0)
        return;
    StartHuff();
    for (i = 0; i < N - F; i++)
        text_buf[i] = 0x20;
    r = N - F;
    for (count = 0; count < textsize; ) {
        c = DecodeChar();
        if (c < 256) {
            if(fnc_putc(c)==-1) {
                return;
            }
            text_buf[r++] = (unsigned char)c;
            r &= (N - 1);
            count++;
        } else {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - 255 + THRESHOLD;
            for (k = 0; k < j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                if(fnc_putc(c)==-1) {
                    return;
                }
                text_buf[r++] = (unsigned char)c;
                r &= (N - 1);
                count++;
            }
        }
        
    }

}

int CLzhCompress::fnc_read_file(BYTE *pBuffer, int nLen)
{
	DWORD dwBytes;
	dwBytes=0;
	ReadFile(hFileIn,pBuffer,nLen,&dwBytes,NULL);
	return dwBytes;
}

int CLzhCompress::fnc_write_file(BYTE *pBuffer, int nLen)
{
	DWORD dwBytes;
	dwBytes=0;
	WriteFile(hFileOut,pBuffer,nLen,&dwBytes,NULL);
	return dwBytes;
}

int CLzhCompress::fnc_read_memory(BYTE *pBuffer, int nLen)
{
	int nCount;
	nCount=nLen;
	if((nCount+nMemInOff)>nMemInSize)
		nCount=nMemInSize-nMemInOff;
	
	memcpy(pBuffer,pMemIn+nMemInOff,nCount);
	nMemInOff+=nCount;
	
	return nCount;
}

int CLzhCompress::fnc_write_memory(BYTE *pBuffer, int nLen)
{
	int nCount;
	nCount=nLen;
	if((nCount+nMemOutOff)>nMemOutSize)
		nCount=nMemOutSize-nMemOutOff;
	
	memcpy(pMemOut+nMemOutOff,pBuffer,nCount);
	nMemOutOff+=nCount;
	
	return nCount;
}

int CLzhCompress::fnc_write(BYTE *ptr, int len)
{
	if(freeze_type==0)
		return fnc_write_file(ptr,len);
	else if(freeze_type==1)
		return fnc_write_memory(ptr,len);
	return 0;
}

int CLzhCompress::fnc_read(BYTE *ptr, int len)
{
	if(freeze_type==0)
		return fnc_read_file(ptr,len);
	else if(freeze_type==1)
		return fnc_read_memory(ptr,len);
	return 0;
}

int CLzhCompress::fnc_getc(void)
{
	int val;
	
	val=0;
	if(fnc_read((BYTE *)&val,1)!=1) return -1;
	
	return val;
}

int CLzhCompress::fnc_putc(int val)
{
	if(fnc_write((BYTE *)&val,1)!=1) return -1;
	
	return (int) 0;
}


int CLzhCompress::lzh_freeze_file(char *szInFile, char *szOutFile)
{
	hFileIn=CreateFile(szInFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFileIn==INVALID_HANDLE_VALUE) return -1;
	hFileOut=CreateFile(szOutFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFileOut==INVALID_HANDLE_VALUE) {
		CloseHandle(hFileIn);
		return -1;
	}
	
	getbuf=0;
	getlen=0;
	putbuf=0;
	putlen=0;
	textsize=GetFileSize(hFileIn,NULL);
    freeze_type=0;
	
	Encode();
	
	CloseHandle(hFileOut);
	CloseHandle(hFileIn);
	return 0;
}

int CLzhCompress::lzh_freeze_memory(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen)
{
	pMemIn=(BYTE *)pInBuffer;
	nMemInSize=nInBufLen;
	pMemOut=(BYTE *)pOutBuf;
	nMemOutSize=nOutBufLen;
	nMemInOff=0;
	nMemOutOff=0;
	
	getbuf=0;
	getlen=0;
	putbuf=0;
	putlen=0;
	textsize=nInBufLen;
    freeze_type=1;
	
	Encode();

	return 0;
}


int CLzhCompress::lzh_melt_file(char *szInFile, char *szOutFile)
{
	hFileIn=CreateFile(szInFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFileIn==INVALID_HANDLE_VALUE) return -1;
	hFileOut=CreateFile(szOutFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFileOut==INVALID_HANDLE_VALUE) {
		CloseHandle(hFileIn);
		return -1;
	}
	
	getbuf=0;
	getlen=0;
	putbuf=0;
	putlen=0;
	freeze_type=0;
	
	Decode();
	
	CloseHandle(hFileOut);
	CloseHandle(hFileIn);
	return 0;
}

int CLzhCompress::lzh_melt_memory(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen)
{
	pMemIn=(BYTE *)pInBuffer;
	nMemInSize=nInBufLen;
	pMemOut=(BYTE *)pOutBuf;
	nMemOutSize=nOutBufLen;
	nMemInOff=0;
	nMemOutOff=0;
	
	getbuf=0;
	getlen=0;
	putbuf=0;
	putlen=0;
	freeze_type=1;
	
	Decode();

	return 0;
}







