// ******************************************************************
// *
// *    .,-:::::    .,::      .::::::::.    .,::      .:
// *  ,;;;'````'    `;;;,  .,;;  ;;;'';;'   `;;;,  .,;; 
// *  [[[             '[[,,[['   [[[__[[\.    '[[,,[['  
// *  $$$              Y$$$P     $$""""Y$$     Y$$$P    
// *  `88bo,__,o,    oP"``"Yo,  _88o,,od8P   oP"``"Yo,  
// *    "YUMMMMMP",m"       "Mm,""YUMMMP" ,m"       "Mm,
// *
// *   Cxbx->Win32->CxbxKrnl->EmuXG.cpp
// *
// *  This file is part of the Cxbx project.
// *
// *  Cxbx and Cxbe are free software; you can redistribute them
// *  and/or modify them under the terms of the GNU General Public
// *  License as published by the Free Software Foundation; either
// *  version 2 of the license, or (at your option) any later version.
// *
// *  This program is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *  GNU General Public License for more details.
// *
// *  You should have recieved a copy of the GNU General Public License
// *  along with this program; see the file COPYING.
// *  If not, write to the Free Software Foundation, Inc.,
// *  59 Temple Place - Suite 330, Bostom, MA 02111-1307, USA.
// *
// *  (c) 2002-2003 Aaron Robinson <caustik@caustik.com>
// *
// *  All rights reserved
// *
// ******************************************************************
#define _CXBXKRNL_INTERNAL
#define _XBOXKRNL_LOCAL_

#undef FIELD_OFFSET     // prevent macro redefinition warnings
#include <windows.h>

// ******************************************************************
// * prevent name collisions
// ******************************************************************
namespace xntdll
{
    #include "xntdll.h"
};

// ******************************************************************
// * prevent name collisions
// ******************************************************************
namespace xapi
{
    #include "EmuXapi.h"
};

#include "Emu.h"
#include "EmuFS.h"

// ******************************************************************
// * prevent name collisions
// ******************************************************************
namespace xdirectx
{
    #include "xdirectx.h"
};

// ******************************************************************
// * prevent name collisions
// ******************************************************************
namespace xg
{
    #include "EmuXG.h"
};

// ******************************************************************
// * func: EmuXGIsSwizzledFormat
// ******************************************************************
PVOID WINAPI xg::EmuXGIsSwizzledFormat
(
    xdirectx::D3DFORMAT Format
)
{
    // ******************************************************************
    // * debug trace
    // ******************************************************************
    #ifdef _DEBUG_TRACE
    {
        EmuSwapFS();   // Win2k/XP FS
        printf("EmuXapi (0x%X): EmuXGIsSwizzledFormat\n"
               "(\n"
               "   Format              : 0x%.08X\n"
               ");\n",
               GetCurrentThreadId(), Format);
        EmuSwapFS();   // Xbox FS
    }
    #endif

    return FALSE;
}

// ******************************************************************
// * func: EmuXGUnswizzleRect
// ******************************************************************
VOID WINAPI xg::EmuXGUnswizzleRect
(
    PVOID           pSrcBuff,
    DWORD           dwWidth,
    DWORD           dwHeight,
    DWORD           dwDepth,
    PVOID           pDstBuff,
    DWORD           dwPitch,
    RECT            rSrc,
    POINT           poDst,
    DWORD           dwBPP
)
{
	DWORD dwOffsetU = 0, dwMaskU = 0;
    DWORD dwOffsetV = 0, dwMaskV = 0;
    DWORD dwOffsetW = 0, dwMaskW = 0;

	DWORD i = 1;
	DWORD j = 1;

	while( (i >= dwWidth) || (i >= dwHeight) || (i >= dwDepth) )
    {
        if(i < dwWidth)
        {
			dwMaskU |= j;
			j<<=1;
		}

        if(i < dwHeight)
        {
			dwMaskV |= j;
			j<<=1;
		}

        if(i < dwDepth)
        {
			dwMaskW |= j;   
            j<<=1;  
        }

        i<<=1;
	}

    DWORD dwSU = 0;
	DWORD dwSV = 0;
	DWORD dwSW = 0;
	DWORD dwMaskMax=0;

	// get the biggest mask
	if(dwMaskU > dwMaskV)
		dwMaskMax=dwMaskU;
	else
		dwMaskMax=dwMaskV;
	if(dwMaskW > dwMaskMax)
		dwMaskMax=dwMaskW;

	for(i = 1; i <= dwMaskMax; i<<=1)
    {
		if(i<=dwMaskU)
        {
			if(dwMaskU & i) dwSU |= (dwOffsetU & i);
			else            dwOffsetU<<=1;
		}

        if(i<=dwMaskV)
        {
			if(dwMaskV & i) dwSV |= (dwOffsetV & i);
			else            dwOffsetV<<=1;
		}
		
        if(i<=dwMaskW)
        {
			if(dwMaskW & i) dwSW |= (dwOffsetW & i);
			else            dwOffsetW<<=1;
		}
	}

	DWORD dwW = dwSW;
	DWORD dwV = dwSV;
	DWORD dwU = dwSU;

	for(DWORD z=0; z<dwDepth; z++) 
	{
		dwV = dwSV;

		for(DWORD y=0; y<dwHeight; y++) 
		{
			dwU = dwSU;

			for (DWORD x=0; x<dwWidth; x++) 
			{
				memcpy(pDstBuff, &((BYTE*)pSrcBuff)[(dwU|dwV|dwW)*dwBPP], dwBPP);
				pDstBuff=(PVOID)(((DWORD)pDstBuff)+dwBPP);

				dwU = (dwU - dwMaskU) & dwMaskU;
			}
			pDstBuff=(PVOID)(((DWORD)pDstBuff)+(dwPitch-dwWidth*dwBPP));
			dwV = (dwV - dwMaskV) & dwMaskV;
		}
		dwW = (dwW - dwMaskW) & dwMaskW;
	}
}
