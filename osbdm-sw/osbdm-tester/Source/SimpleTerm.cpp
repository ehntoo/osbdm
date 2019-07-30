//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "SimpleTerm.h"
#include "osbdm_JM60.h"
#include "otmain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTermFrm *TermFrm;

#define XON                 17  // Disable TX via software
#define XOFF                19  // Enable TX via software

#define SREC_MAX_BUF	1024	// maximum characters per srecord line

//---------------------------------------------------------------------------
__fastcall TTermFrm::TTermFrm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TTermFrm::FormShow(TObject *Sender){
	STimer->Interval = 2;	// timer interval in milliseconds
	STimer->Enabled = true;	// start serial timer
}
//---------------------------------------------------------------------------
void __fastcall TTermFrm::FormClose(TObject *Sender, TCloseAction &Action){
	STimer->Enabled = false;	// stop serial timer
}
//---------------------------------------------------------------------------
// Handle Key pressed in Terminal window
void __fastcall TTermFrm::MemoKeyDown(TObject *Sender, WORD &Key, TShiftState Shift){
	// A key was just pressed and released
	switch(Key){
		case VK_RETURN: // if ENTER key
			osbdm_sci_write1('\n');	// send line feed to serial port
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTermFrm::MemoKeyPress(TObject *Sender, char &Key){
	osbdm_sci_write1(Key);	// send to serial port
	Key=0;
}
//---------------------------------------------------------------------------
// Handle Timer event - check for serial input
void __fastcall TTermFrm::STimerTimer(TObject *Sender){
	STimer->Enabled = false;
	// read serial port
	if(osbdm_sci_read(usb_data2) == 0){
//		Memo->Lines->Add("hello universe");	// add a new line
//		Memo->SelText = " and me";	// add text at current position
		char c = usb_data2[1] - 2;	// data count
		if(c != 0){	// if data received
			usb_data2[c+2] = 0;  // null terminate
			Memo->SelText = String((char *)usb_data2+2);	// add text at current position
		}
	}
//	CurrentLine = Memo->Lines->Strings[Memo->CaretPos.y];
	STimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTermFrm::BitBtn1Click(TObject *Sender)
{
	if(!OpenDialog1->Execute())	return;
	if(FileExists(OpenDialog1->FileName)){
		// send this file
		SendFile(OpenDialog1->FileName);
	}

}
//---------------------------------------------------------------------------
// returns the number of characters received
// returns 0 if none OR error
// data received will be at usb_data2+2
char TTermFrm::PollForSerial(){
	if(osbdm_sci_read(usb_data2) == 0){
		char c = usb_data2[1] - 2;	// data count
		if(c != 0){	// if data received
			usb_data2[c+2] = 0;  // null terminate stream
			return c;
		}
		return 0;
	}
	return 0;	// error
}
//---------------------------------------------------------------------------

void TTermFrm::SendFile(String filepath){
	char b;
	FILE *fin;
	char rbuf[SREC_MAX_BUF+1];
	char *ptr;
	int count, lcount;

	// open the text file
	if ((fin = fopen(filepath.c_str(), "rt")) == NULL){
		werror("Cannot open file %s", filepath);
		return;
	}

	STimer->Enabled = false;	// disable automatic read timer

	for( ; ;){
		// read a line of data from the file
		fgets(rbuf, SREC_MAX_BUF, fin); // read a line from the file
		if(feof(fin))	break;

		// handle newline character
		ptr = strchr(rbuf,'\n'); 		// find first newline character
		if(ptr != NULL){
			*ptr = '\r';	// replace with a carriage return character (what the bootloader is looking for)
			*(ptr+1) = 0;	// end string right after that
		}

		lcount = strlen(rbuf);	// total number of bytes to send
		ptr = rbuf;				// point to start of data

		// send data in blocks no greater than the max packet size till done with this line
		for( ; ;){
			count = packet_data_size(2, 8, lcount);	// determine size of this data block
			osbdm_sci_write(count, ptr);	// send the data
			ptr += count;

			lcount -= count;	// take number sent off count
			if(lcount < 1)	break;	// stop if all data sent
		}

		// wait for XON character to be received
		for( ; ; ){
			b=0;
			char c = PollForSerial();	// check for incomming serial data (XON/XOFF)
			if(c){
				for(char n=0; n < c; n++){
					b=*(usb_data2+2+n);
					if(b=='*'){	// display pacing characters on the terminal
						Memo->SelText = "*";	// add text at current position
						break;
					}
					if(b==XON)	break;		// do this if we care about the first one
											// if it's commented out, XON must be the last one received
				}
				if(b==XON)	break;
				if(b=='*')	break;	// this pacing character might be used
			}
		}
	}
	fclose(fin);
	STimer->Enabled = true;	// re-enable automatic read timer
}


/*

	for(; debug_stopflag == false; ){

		fgets(rbuf, CFG_MAX_BUF, fin); // read a line from the file
		if(feof(fin))	break;

		// handle newline character
		ptr = strchr(rbuf,'\n'); 		// find first newline character
		if(ptr != NULL)	*ptr = 0;		// remove it and end string there

		// display the line
		Commands->Lines->Add(rbuf);

		// handle comment character
		if(rbuf[0] == ';')	continue;	// fast ignore commented out lines
		ptr = strchr(rbuf,';');			// find any other EOL comment characters
		if(ptr != NULL)	*ptr = 0;		// ignore anything after if found

		last_command = 0;
		if(rbuf[0])
			ProcessCommand(rbuf, true);	// process the command line with arguments
	}

	fclose(fin);
	return 0;
*/
