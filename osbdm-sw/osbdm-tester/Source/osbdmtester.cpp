//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------
USEFORM("otmain.cpp", otfrm);
USERES("osbdmtester.res");
USEFORM("ot_about.cpp", AboutBox);
USEUNIT("osbdm_usb.c");
USEUNIT("osbdm_JM60.cpp");
USELIB("libusb.lib");
USEFORM("C:\Program Files\Common Files\Borland Shared\Images\Buttons\othelp.cpp", othelpf);
USEFORM("SimpleTerm.cpp", TermFrm);
//---------------------------------------------------------------------------
bool AlreadyRunning(String myname, bool popupmessage);
HANDLE hMutex;
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application->Initialize();
	Application->Title = "OSBDM Tester";
	if(AlreadyRunning("OSBDM Tester", true) == true) return 1;	// exit if program already running

		Application->CreateForm(__classid(Totfrm), &otfrm);
		Application->CreateForm(__classid(TAboutBox), &AboutBox);
		Application->CreateForm(__classid(Tothelpf), &othelpf);
		Application->CreateForm(__classid(TTermFrm), &TermFrm);
		Application->Run();

	//	Close mutex which keeps more than one instance of program from starting
	if(hMutex)
		CloseHandle(hMutex);

	return 0;
}
//---------------------------------------------------------------------
// See if this program is already running
bool AlreadyRunning(String myname, bool popupmessage){

	//Declare security attributes
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;

	//Initialize security descriptor
	BOOL bGotSA = FALSE;
	SECURITY_DESCRIPTOR sd;
	if(InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{
		//Set security descriptor (Null DACL)
		if(SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE))
		{
			bGotSA = TRUE;
			sa.lpSecurityDescriptor = &sd;
		}
	}

	//Create named mutex
	BOOL bSecondInstance;
	String pMutexName = "Global\\" + myname + "-03448840-B10A-1127-BC36-006052890974";

	int nErr;
	if(bGotSA)
	{
		//Got security descriptor
		hMutex = CreateMutex(&sa, FALSE, pMutexName.c_str());
		nErr = ::GetLastError();
		bSecondInstance = hMutex && nErr == ERROR_ALREADY_EXISTS;
	}
	else
	{
		//No security descriptor
		hMutex = CreateMutex(NULL, FALSE, pMutexName.c_str());
		nErr = ::GetLastError();
		bSecondInstance = (hMutex && nErr == ERROR_ALREADY_EXISTS) || nErr == ERROR_ACCESS_DENIED;
	}


	if(bSecondInstance)	{
		// More than one instance of this app is running, Show message and exit
		if(popupmessage)	Application->MessageBox("Program already running", "ERROR", MB_OK);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
