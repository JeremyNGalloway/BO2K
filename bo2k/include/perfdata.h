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

#ifndef __INC_PERFDATA_H
#define __INC_PERFDATA_H

typedef PERF_DATA_BLOCK             PERF_DATA,      *PPERF_DATA; 
typedef PERF_OBJECT_TYPE            PERF_OBJECT,    *PPERF_OBJECT; 
typedef PERF_INSTANCE_DEFINITION    PERF_INSTANCE,  *PPERF_INSTANCE; 
typedef PERF_COUNTER_DEFINITION     PERF_COUNTER,   *PPERF_COUNTER; 

 
DWORD   GetPerfData (HKEY       hPerfKey, 
                     LPTSTR     szObjectIndex, 
                     PPERF_DATA *ppData, 
                     DWORD      *pDataSize); 
 
DWORD   GetPerfTitleSz (HKEY       hKeyMachine, 
                        HKEY       hKeyPerf, 
                        LPTSTR     *TitleBuffer, 
                        LPTSTR     *TitleSz[], 
                        DWORD      *TitleLastIdx); 

 
PPERF_OBJECT    FirstObject (PPERF_DATA pData); 
PPERF_OBJECT    NextObject (PPERF_OBJECT pObject); 
PPERF_OBJECT    FindObject (PPERF_DATA pData, DWORD TitleIndex); 
PPERF_OBJECT    FindObjectN (PPERF_DATA pData, DWORD N); 

PPERF_INSTANCE  FirstInstance (PPERF_OBJECT pObject); 
PPERF_INSTANCE  NextInstance (PPERF_INSTANCE pInst); 
PPERF_INSTANCE  FindInstanceN (PPERF_OBJECT pObject, DWORD N); 
PPERF_INSTANCE  FindInstanceParent (PPERF_INSTANCE pInst, PPERF_DATA pData); 
LPTSTR          InstanceName (PPERF_INSTANCE pInst); 
 
PPERF_COUNTER   FirstCounter (PPERF_OBJECT pObject); 
PPERF_COUNTER   NextCounter (PPERF_COUNTER pCounter); 
PPERF_COUNTER   FindCounter (PPERF_OBJECT pObject, DWORD TitleIndex); 
PVOID           CounterData (PPERF_INSTANCE pInst, PPERF_COUNTER pCount); 

#endif
