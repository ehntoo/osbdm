/* OSBDM-JM60 Windows USB Library
 * Copyright (C) 2009  Freescale
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//----------------------------------------------------------------------------
//
//
//  FILE
//
//      osbdm_dll.c
// 
//
//  DESCRIPTION
//
//		OSBDM USB driver dll
//
//
//----------------------------------------------------------------------------
//

#include <windows.h>
#include "osbdm_usb.h"
#include "log.h"

/*********************************************************************************************/
/* DllMain					                                                                 */
/*********************************************************************************************/

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			open_log_file();

			osbdm_usb_init();			//initialise USB 
			#ifdef LOG
/*				GetModuleFileName(NULL,path,sizeof(path));
				charp=strchr(path,'\\');
				if (charp!=NULL) {
					while (strchr(charp+1,'\\')) charp=strchr(charp+1,'\\');	// find the last backslash 
					strcpy(charp+1,"OpenSourceBDM_dll.log");
					log_file=fopen(path,"wb");
				}
				print("Log file path: %s\r\n",path);
				print("Open Source BDM DLL v%1d.%1d. Compiled on %s, %s.\r\n",opensourcebdm_DLL_VERSION/16,opensourcebdm_DLL_VERSION&0x0f,__DATE__,__TIME__);
				time(&time_now);
				print("Log file created on: %s\r", ctime(&time_now));
*/
			#endif
            break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			close_log_file();
			#ifdef LOG
/*				time(&time_now);
				print("End of log file: %s\r", ctime(&time_now));
				fclose(log_file);
*/
			#endif
            break;
    }
    return TRUE;
}
