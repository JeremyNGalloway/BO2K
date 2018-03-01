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

#include <windows.h> 
#include <winperf.h> 
#include <perfdata.h>
 

// FirstObject() - Returns pointer to the first object in pData. 
//                 If pData is NULL then NULL is returned. 

PPERF_OBJECT FirstObject (PPERF_DATA pData) 
{ 
    if (pData) 
        return ((PPERF_OBJECT) ((PBYTE) pData + pData->HeaderLength)); 
    else 
        return NULL; 
} 
 
 
 
// NextObject() - Returns pointer to the next object following pObject. 
//	              If pObject is the last object, bogus data maybe returned. 
//                The caller should do the checking. 
//                If pObject is NULL, then NULL is returned. 

PPERF_OBJECT NextObject (PPERF_OBJECT pObject) 
{ 
    if (pObject) 
        return ((PPERF_OBJECT) ((PBYTE) pObject + pObject->TotalByteLength)); 
    else 
        return NULL; 
} 
 
 
 
 
// FindObject() - Returns pointer to object with TitleIndex.
//                If not found, NULL is returned. 

PPERF_OBJECT FindObject (PPERF_DATA pData, DWORD TitleIndex) 
{	
	PPERF_OBJECT pObject; 
	DWORD i = 0; 
	
    if (pObject = FirstObject (pData)) {
        while (i < pData->NumObjectTypes) { 
            if (pObject->ObjectNameTitleIndex == TitleIndex) 
                return pObject; 
			
            pObject = NextObject (pObject); 
            i++; 
		} 
	}
		
	return NULL; 
} 
 
// FindObjectN() - Find the Nth object in pData.  
//                 If not found, NULL is returned. 0 <= N < NumObjectTypes. 

PPERF_OBJECT FindObjectN (PPERF_DATA pData, DWORD N) 
{ 
	PPERF_OBJECT pObject; 
	DWORD        i = 0; 
	
    if (!pData) 
        return NULL; 
    else if (N >= pData->NumObjectTypes) 
        return NULL; 
    else 
	{ 
        pObject = FirstObject (pData); 
		
        while (i != N) 
		{ 
            pObject = NextObject (pObject); 
            i++; 
		} 
		
        return pObject; 
	} 
}
