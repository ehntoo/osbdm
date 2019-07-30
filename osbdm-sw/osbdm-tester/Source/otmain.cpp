//---------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <sys\stat.h>
#include <dir.h>
#pragma hdrstop

#include "otmain.h"
#include "ot_about.h"
#include "othelp.h"
#include "commands.h"
#include "osbdm_usb.h"
#include "osbdm_JM60.h"
#include "SimpleTerm.h"

//---------------------------------------------------------------------
#pragma resource "*.dfm"
Totfrm *otfrm;


//#define TERMINAL_VERSION	1

const String ProgramDir = ExtractFilePath(Application->ExeName);


int	BDMStat=0;	// buffers current bdm status
bool cmd_script;	// set to true if command executed in "script" mode
int	TesterTime;

bool debug_stopflag;

unsigned char outbuf[MAX_DATA_SIZE+2];	// temporary buffers
unsigned char inbuf[MAX_DATA_SIZE+2];

// global variables used by S-Record Loader
unsigned long StartAddr, EndAddr,  wAddr, bAddr, lastS1, lastS2, srTotalBytes;

// global variables set by command line options
bool CL_InitBDM = 1;	// clear to NOT initialize BDM on startup
bool TERM_MODE = 0;		// clear terminal mode flag

String CurrentCommandline = "";
bool CommandKeyPressed;

//---------------------------------------------------------------------
__fastcall Totfrm::Totfrm(TComponent *AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall Totfrm::FormShow(TObject *Sender)
{

#ifdef TERMINAL_VERSION
	CL_InitBDM = 0;	// do not initialize BDM
	TERM_MODE = 1;	// switch to terminal mode
	TermFrm->Show();	// open terminal window
#else
	// set global variables from command line
	// parameter 0 is always the full path to the program executable
	for (int i=0;i<=ParamCount();i++){
		String p = LowerCase(ParamStr(i));	// get parameter
		if(p == "noinit"){
			CL_InitBDM = 0;	// do not initialize BDM
		}
		if(p == "term"){
			CL_InitBDM = 0;	// do not initialize BDM
			TERM_MODE = 1;	// switch to terminal mode
			TermFrm->Show();	// open terminal window
		}
	}
#endif
	TargetBox->ItemIndex = 0;	// select the default target
/*	if(otfrm->TargetBox->Text == "DSC"){
		oprint("Target is DSC");
	}
*/

	// open BDM connection
	if(int rval=OpenBDM(0)){
		werror("Cannot open USB/BDM connection - Error code: %x", rval);
		Close();
	}

	// if startup file exists, run it
	OpenDialog->InitialDir = ProgramDir;
	const String Sfile = ProgramDir + "startup.tscript";
	if (FileExists(Sfile)){
		ProcessTargetConfig(Sfile);
	}

}
//---------------------------------------------------------------------------
void __fastcall Totfrm::FormClose(TObject *Sender, TCloseAction &Action)
{
	CloseBDM(0); // close any BDM connections that were opened
}

//---------------------------------------------------------------------------
void __fastcall Totfrm::TermButtonClick(TObject *Sender){
	TERM_MODE = 1;	// switch to terminal mode
	TermFrm->Show();	// open terminal window
}

//---------------------------------------------------------------------------
// Load and execute a test script file
// return 0 on success
int Totfrm::RunScript(char *fpath){
	if(fpath == 0){
		if(!OpenDialog->Execute())	return 1;
	}else{
		OpenDialog->FileName = String(fpath);
	}
	if(FileExists(OpenDialog->FileName)){
		// process this file
		if(ProcessTargetConfig(OpenDialog->FileName) == 0){
			AgainButton->Enabled = true;
			return 0;
		}
	}
	return 1;
}

void __fastcall Totfrm::FileOpen1Execute(TObject *Sender){
	RunScript(0);	// prompt for a script file and execute it
}
void __fastcall Totfrm::AgainButtonClick(TObject *Sender)
{
	ProcessTargetConfig(OpenDialog->FileName);		// process this file

}

//---------------------------------------------------------------------------

void __fastcall Totfrm::FileSave1Execute(TObject *Sender)
{
	SaveDialog->Execute();
}

//---------------------------------------------------------------------------
void __fastcall Totfrm::FileExit1Execute(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall Totfrm::HelpAbout1Execute(TObject *Sender)
{
	AboutBox->ShowModal();
}
//---------------------------------------------------------------------------
// output a simple formatted messagebox
void werror (const char *message, ...){
	VA_OPEN (args, message);
	VA_FIXEDARG (args, const char *, message);
	AnsiString str;

	str.vprintf(message, args);
	Application->MessageBox(str.c_str(), "ERROR", MB_OK);
	VA_CLOSE (args);
}
//---------------------------------------------------------------------------
// output a formatted string to the Output window pane
// may want to route this to a text file (log) instead in some cases
void oprintf(const char *format, ...){
	VA_OPEN (args, format);
	VA_FIXEDARG (args, const char *, format);
	AnsiString str;

	str.vprintf(format, args);
	otfrm->Output->Lines->Add(str);
	VA_CLOSE (args);
}
void oprint(String str){
	otfrm->Output->Lines->Add(str);
}


/*---------------------------------------------------------------------------
 OpenBDM
 -------
 Initialize the BDM if it's not already open (BDMStat>0)
 Input:
	force: 1 = re-initialize and open no matter what
 Returns:
	0 = BDM open and ready for use
*/
int OpenBDM(char force){
	int i;

	if(force==1 || BDMStat == 0){
		i=osbdm_connect();		// initialize usb and search for an osbdm device
		if(i<1)	return 0x100;
		i=osbdm_open(0);		// open first device found
		if(i>0) return 0x200 + i;
		if(CL_InitBDM){	// if not overridden by command line
			if(osbdm_init())	return 0x300;	// initialize bdm
//	unsigned long val = osbdm_read32(MEM_REG, 0);	// read a 32-bit value
//	oprintf("D0 = %08x", val);

		}
	}
	BDMStat = 1;
	return 0;
}
//---------------------------------------------------------------------------
// close the BDM if it's open
void CloseBDM(char force){
	if(force==1 || BDMStat != 0){
		osbdm_close();	// close the open BDM device
	}
	BDMStat = 0;
}

//---------------------------------------------------------------------------
// Handle Key pressed in Command window
void __fastcall Totfrm::CommandsKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	// save the current line
	CurrentCommandline = Commands->Lines->Strings[Commands->CaretPos.y];
	CommandKeyPressed = true;

	// A key was just pressed and released
	switch(Key){
		case VK_RETURN: // if ENTER key
			String CurrentLine = "";
			// get the current line
			CurrentLine = Commands->Lines->Strings[Commands->CaretPos.y];
			if(CurrentLine != NULL){
				// send the line just entered to the command parser for processing
				ProcessCommand(CurrentLine.c_str(), false);
			}
			break;
	}

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void cmd_help(int argc, char **argv);

void cmd_test(int argc, char **argv);
void cmd_testmem(int argc, char **argv);
void cmd_testflash(int argc, char **argv);
void cmd_delay(int argc, char **argv);
void cmd_waitforkey(int argc, char **argv);
void cmd_log(int argc, char **argv);
void cmd_set_target(int argc, char **argv);
//void cmd_writectrl(int argc, char **argv);
void cmd_write8(int argc, char **argv);
void cmd_write16(int argc, char **argv);
void cmd_write32(int argc, char **argv);
void cmd_halt(int argc, char **argv);
void cmd_resethalt(int argc, char **argv);
void cmd_resetrun(int argc, char **argv);
void cmd_writeaddressreg(int argc, char **argv);
void cmd_writedatareg(int argc, char **argv);
void cmd_read8(int argc, char **argv);
void cmd_read16(int argc, char **argv);
void cmd_read32(int argc, char **argv);
void cmd_memorydump(int argc, char **argv);
void cmd_memorydumpw(int argc, char **argv);
void cmd_memorydumpl(int argc, char **argv);
void cmd_blockfill8(int argc, char **argv);
void cmd_blockfill16(int argc, char **argv);
void cmd_blockfill32(int argc, char **argv);
void cmd_go(int argc, char **argv);
void cmd_gobk(int argc, char **argv);
void cmd_step(int argc, char **argv);
void cmd_regmodify(int argc, char **argv);
void cmd_regdisplay(int argc, char **argv);
void cmd_cregmodify(int argc, char **argv);
void cmd_cregdisplay(int argc, char **argv);
void cmd_dregmodify(int argc, char **argv);
void cmd_dregdisplay(int argc, char **argv);
void cmd_reset(int argc, char **argv);
void cmd_stat(int argc, char **argv);
void cmd_unsecure(int argc, char **argv);
void cmd_exit(int argc, char **argv);
void cmd_load(int argc, char **argv);
void cmd_load16(int argc, char **argv);
void cmd_run(int argc, char **argv);
void cmd_pc(int argc, char **argv);
void cmd_bdminit(int argc, char **argv);
void cmd_sci_read(int argc, char **argv);
void cmd_sci_write(int argc, char **argv);

void cmd_fwrite16(int argc, char **argv);
void cmd_fread16(int argc, char **argv);

void cmd_gspeed(int argc, char **argv);
void cmd_sspeed(int argc, char **argv);


CFG_CMD CFG_CMDTAB[] =
{
	{"test",0,cmd_test,0,0},
	{"testmem",3,cmd_testmem,"Test Memory","<8/16/32> <Start> <End>"},
	{"testflash",2,cmd_testflash,0,0},
	{"delay",1,cmd_delay,0,0},
	{"waitforkey",0,cmd_waitforkey,0,0},
	{"log",1,cmd_log,0,0},
	{"target",1,cmd_set_target,0,0},
	{"run",0,cmd_run,"Execute Test Script","[<filepath>]"},
	{"load",0,cmd_load,"Load Memory from S-Rec","[<filepath>]"},
	{"load16",1,cmd_load16,0,0},
	{"exit",0,cmd_exit,0,0},

	{"help",0,cmd_help,0,0},

	{"bdminit",0,cmd_bdminit,0,0},
	{"reset",0,cmd_reset,"Target Reset",""},
	{"resethalt",0,cmd_resethalt,0,0},
	{"resetrun",0,cmd_resetrun,0,0},
	{"pc",1,cmd_pc,"Set Program Counter","<addr1>"},
	{"go",0,cmd_go,"Execute from current PC",""},
	{"step",0,cmd_step,"Execute 1 instruction",""},
	{"halt",0,cmd_halt,"Halt execution",""},
	{"stat",0,cmd_stat,"Read various satus values",""},
	{"gspeed",0,cmd_gspeed,"get target clock speed",""},
	{"sspeed",1,cmd_sspeed,"set target clock speed","<16-bit>"},

	{"mmb",2,cmd_write8,"Modify Byte","<addr1> <8-bit> [x/p]"},
	{"mm",2,cmd_write8,0,0},
	{"wb",2,cmd_write8,0,0},
	{"writemem.b",2,cmd_write8,0,0},

	{"mmw",2,cmd_write16,"Modify Word","<addr1> <16-bit> [x/p]"},
	{"ww",2,cmd_write16,0,0},
	{"writemem.w",2,cmd_write16,0,0},

	{"mml",2,cmd_write32,"Modify Long","<addr1> <32-bit> [x/p]"},
	{"wl",2,cmd_write32,0,0},
	{"writemem.l",2,cmd_write32,0,0},

	{"mmr",2,cmd_regmodify,"Modify Register","<D0-7,A0-7> <32-bit>"},
	{"rm",2,cmd_regmodify,0,0},

	{"mmc",2,cmd_cregmodify,"Modify Control Reg","<addr1> <32-bit>"},
	{"rmc",2,cmd_cregmodify,0,0},

	{"mmd",2,cmd_dregmodify,"Modify Debug Reg","<DRc> <32-bit>"},
	{"rmd",2,cmd_dregmodify,0,0},


	{"db",1,cmd_read8,"Display Byte","<addr1> [x/p]"},
	{"rb",1,cmd_read8,0,0},

	{"dw",1,cmd_read16,"Display Word","<addr1> [x/p]"},
	{"rw",1,cmd_read16,0,0},

	{"dl",1,cmd_read32,"Display Long","<addr1> [x/p]"},
	{"rl",1,cmd_read32,0,0},

	{"dr",1,cmd_regdisplay,"Display Register","<D0-7,A0-7>"},
	{"rd",1,cmd_regdisplay,0,0},

	{"dc",1,cmd_cregdisplay,"Display Control Reg","<addr1>"},
	{"rdc",1,cmd_cregdisplay,0,0},

	{"dd",1,cmd_dregdisplay,"Display Debug Reg","<DRc>"},
	{"rdd",1,cmd_dregdisplay,0,0},


	{"md",0,cmd_memorydump,"Memory Dump","[<addr>] [size]] [x/p]"},
	{"mdw",0,cmd_memorydumpw,"Memory Dump Word","[<addr>] [size] [x/p]"},
	{"mdl",0,cmd_memorydumpl,"Memory Dump Long","[<addr>] [size] [x/p]"},


	{"unsecure",0,cmd_unsecure,"Unsecure and Erase flash",""},

	{"rsci",0,cmd_sci_read,"Read sci port",""},
	{"wsci",1,cmd_sci_write,"Write to sci port","<string>"},

	{"fmw",2,cmd_fwrite16,"Write 16-bit to flash","<addr1> <16-bit> [p/x]"},
	{"fdw",1,cmd_fread16,"Display 16-bit from flash","<addr1> [p/x]"},


/*	{"bf",3,cmd_blockfill8,"Block Fill","<first> <last> <8-bit>"},
	{"bfw",3,cmd_blockfill16,"Block Fill","<first> <last> <16-bit>"},
	{"bfl",3,cmd_blockfill32,"Block Fill","<first> <last> <32-bit>"},
*/


/*
	{"writecontrolreg",2,cmd_writectrl,0,0},
	{"writeaddressreg",2,cmd_writeaddressreg,0,0},
	{"writedatareg",2,cmd_writedatareg,0,0},

	{"load",0,cmd_load,"Load Memory from File","[<filepath>]"},
	{"gobk",0,cmd_gobk,0,0},

	#ifdef HCS0812  // if HCS0812 BDM
	{"ss",0,cmd_ss,"SCI Status",""},
	{"so",1,cmd_so,"SCI Output","string"},
	{"si",0,cmd_si,"SCI Input",""},
	#endif
*/

};
const int CFG_NUM_CMD = CFG_CMDTAB_SIZE;

// these are global variables that may be used to save values from the command functions
unsigned long int cmd_val;
char *cmd_ptr;
unsigned long int cmd_addr_save;	// global variable to hold an address
unsigned long int cmd_data_save;	// global variable to hold a data variable

int last_command = 0;	// last command entered, used if user presses ENTER key on a blank line


//---------------------------------------------------------------------------
/* Process Command Line
   --------------------
 Returns 0 if file processed OK, otherwise returns 1
*/
int ProcessCommand(char *cbuf, bool ScriptMode){
	int argc;
	char *argv[CFG_MAX_ARGS+1];
	char delims[] = " ,\t";

	cmd_script = ScriptMode;	// set script mode true or false

	// get each word out of the string
	argc=0;
	argv[argc] = strtok(cbuf, delims);	// get a pointer to a word from the string
	while(argv[argc] != NULL){
		if(++argc > CFG_MAX_ARGS) break;
		argv[argc] = strtok(NULL, delims);	// get a pointer to a word from the string
	}
	if(argc > 0){
		// search table of commands to see if this is a command line we recognise
		for (int i=0; i < CFG_NUM_CMD; i++){
			if(strcmpi(CFG_CMDTAB[i].cmd, argv[0]) == 0){	// if command found
				if((argc-1) >= CFG_CMDTAB[i].min_args){		// and it has enough arguments
					last_command = i+1;	// save command number in case user wants to repeat
					oprintf(" ");	// put a blank line before each command output to make it more readable
					CFG_CMDTAB[i].func(argc,argv);			// execute its function
					return 0;
				}
				else{
					Application->MessageBox("Invalid Arguments. Type Help for command syntax.", argv[0], MB_OK);
					return 1;
				}
			}
		}
		// invalid command entered
		oprintf("-- Unknown Command");	
		cmd_help(0,0);
	}
//--- disable repeat command function till it's fully debugged.  Should be changed to default OFF and only turn it on for commands that use it
/*	else{
		// if we user previously issued a command that can be repeated
		if(last_command){
			argc=1;	// don't pass any new arguments
			oprintf(" ");	// put a blank line before each command output to make it more readable
			CFG_CMDTAB[last_command-1].func(argc,argv);			// execute its function
		}
	}
*/	
	return 0;
}

//---------------------------------------------------------------------------
/* Process Target Script File
   --------------------------
 Returns 0 if file processed OK, otherwise returns 1

*/
char Totfrm::ProcessTargetConfig(AnsiString Cfname){
	FILE *fin;
	char rbuf[CFG_MAX_BUF+1];
	char *ptr;

	debug_stopflag = false;

	if ((fin = fopen(Cfname.c_str(), "rt")) == NULL){
		werror("Cannot open file %s", Cfname);
		return 1;
	}

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
}

void cmd_help(int argc, char **argv){
	for (int i=0; i < CFG_NUM_CMD; i++){
		if(!CFG_CMDTAB[i].desc)	continue;	// don't display help if command not meant to be typed
		oprintf("%-28s %s %s", CFG_CMDTAB[i].desc, CFG_CMDTAB[i].cmd, CFG_CMDTAB[i].syntax);
	}
}

void cmd_error(){
	oprintf("ERROR - command failed");
}

// dumps memory to the display for testing
void testdump(char *m, int count, unsigned char *s){
	String str;

	if(m != 0){
		str.sprintf("%s(%d): ", m, count);
	}
	for(int i=0; i<count; i++, s++){
		str.cat_sprintf("%02x ", *s);
	}
	oprint(str);
}
/*
unsigned int ByteSwap16(unsigned int val){
	unsigned char h = (val >> 8);	// shift high to low and save
	val = (val & 0xFF);	// mask high
	val <<= 8;			// shift low to high
	return(val + h);	// return them merged
}
unsigned int ByteSwap32(unsigned long val){
	unsigned long a, b, c, d;

	a = val << 24;
	b = (val << 8) & 0x00FF0000;
	c = (val >> 8) & 0x0000FF00;
	d = val >> 24;

	return(a + b + c + d);
}
*/

/*------------------------------------------------------------------------------------
   return a 32-bit value taking 4 bytes from an 8-bit buffer, LSB first
*/
unsigned long getbuf4(unsigned char *buf){
	unsigned long val=0;
	int i;
	for(i=0; i<4; i++){
		val <<=8;
		val += *(buf++);
	}

	return val;
}

// Delay a number of milliseconds using the TesterTimer
void TesterTimerDelay(unsigned long int MilliSec){
	otfrm->TesterTimer->Interval = MilliSec;
	TesterTime=0;
	otfrm->TesterTimer->Enabled = true;
	while(TesterTime==0){
		Application->ProcessMessages();
		if(debug_stopflag == true)	break;
	}
}

// wait for a number of milliseconds - unstoppable
void ForceDelay(unsigned long int MilliSec){
	debug_stopflag = false;
	TesterTimerDelay(MilliSec);
}
// wait for a number of milliseconds
void cmd_delay(int argc, char **argv){
	TesterTimerDelay(StrToInt(AnsiString(argv[1])));
	last_command = 0;	// don't let this command repeat
}

// wait for user to press a key
void cmd_waitforkey(int argc, char **argv){
	CommandKeyPressed = false;
	while(CommandKeyPressed == false){
		Application->ProcessMessages();
	}
	last_command = 0;	// don't let this command repeat
}

// display strings in the output window
void cmd_log(int argc, char **argv){
	String s="";
	for(int i=1; i < argc; i++){
		s += String(argv[i]) + " ";
	}
	oprint(s);

}
// change the current target type
void cmd_set_target(int argc, char **argv){
	otfrm->TargetBox->Text = String(argv[1]);
	oprintf("Target changed to: %s", otfrm->TargetBox->Text);
}

void cmd_bdminit(int argc, char **argv){
	if(osbdm_init()){	// initialize bdm
		werror("Cannot open BDM connection");
	}
}
// set Program Counter
void cmd_pc(int argc, char **argv){
	unsigned long int val = StrToInt("0x"+AnsiString(argv[1]));

	osbdm_write32(MEM_CREG, 0x80F, val);	// write PC
	val = osbdm_read32(MEM_CREG, 0x80F);	// read PC
	oprintf("PC = %08x", val);
}

// get status information
void cmd_stat(int argc, char **argv){
	int i;

	if(osbdm_status(inbuf)) return oprint("ERROR: cannot get status");
	unsigned long val = getbuf4(inbuf+2);
	oprintf("BDM CSR: %08x", val);
	oprintf("PST = %02x", *(inbuf+7));
	val = osbdm_read32(MEM_CREG, 0x80F);	// read PC
	oprintf("PC = %08x", val);

}
// get target clock speed
void cmd_gspeed(int argc, char **argv){

	if(osbdm_target_get_speed(inbuf)) return oprint("ERROR: cannot read target speed");
	unsigned long val = getbuf4(inbuf+2);

//	oprintf("Received: %08X", *(unsigned long *)(inbuf+2));
	oprintf("Received: %08X", val);

}
// set target clock speed
// Speed is in MHz, Floating point Decimal
void cmd_sspeed(int argc, char **argv){

//	unsigned long int val = StrToInt("0x"+AnsiString(argv[1]));
	float val = StrToFloat(AnsiString(argv[1]));
	if(osbdm_target_set_speed(val)) return oprint("ERROR: cannot set target speed");
}

#define	RESET_SOFT_BDM	0	// Soft Reset to BDM mode
#define	RESET_SOFT		1	// Soft Reset to normal (running) mode
#define	RESET_HARD_BDM	2	// hard reset to BDM (halted) mode
#define	RESET_HARD		3	// hard reset to normal (running) mode
#define	RESET_POR_BDM	4	// voltage reset to BDM (halted) mode
#define	RESET_POR		5	// voltage reset normal (running) mode


/* reset target
	Mode:	0 soft reset to BDM mode,
			1 soft reset to Running mode (default)
			2 hard reset to BDM mode,
			3 hard reset to Running mode,
			4 voltage reset to BDM mode (bdm must supply power)
*/

void cmd_reset(int argc, char **argv){
	const char *STATSTR[] = {"Reset OK", "Cannot Reset"};
	int i;
	unsigned char rtype = RESET_SOFT_BDM;	// default reset type

	if(argc > 1){	// if arguments
		// get reset type
		rtype = (unsigned char) StrToInt("0x"+AnsiString(argv[1]));
	}

	i = osbdm_target_reset(rtype);	// reset target
	oprintf(" - %s -", STATSTR[i]);
}

// reset in BDM mode - using soft reset
void cmd_resethalt(int argc, char **argv){
	const char *STATSTR[] = {"Reset OK", "Cannot Reset"};
	int i;

	i = osbdm_target_reset(RESET_SOFT_BDM);	// reset target
	oprintf(" - %s -", STATSTR[i]);
}

// reset in run mode - using soft reset
void cmd_resetrun(int argc, char **argv){
	const char *STATSTR[] = {"Reset OK", "Cannot Reset"};
	int i;

	i = osbdm_target_reset(RESET_SOFT);	// reset target
	oprintf(" - %s -", STATSTR[i]);
}

void cmd_halt(int argc, char **argv){
	const char *STATSTR[] = {"Halted OK", "Cannot Halt"};
	int i;

	i = osbdm_target_halt();
	oprintf(" - %s -", STATSTR[i]);
}
void cmd_go(int argc, char **argv){
	const char *STATSTR[] = {"Go OK", "Cannot Go"};
	int i;

/*	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	*(unsigned long int *)outbuf = 0x80F;

	// set program counter
	if(osbdm_write_block(MEM_CREG, 32, address, outbuf, 4)) return oprintf("Cannot set PC");
*/

	i = osbdm_target_go();	// reset target
	oprintf(" - %s -", STATSTR[i]);
}
void cmd_step(int argc, char **argv){
	const char *STATSTR[] = {"Step OK", "Cannot Step"};
	int i;
	i = osbdm_target_step();	// reset target
	oprintf(" - %s -", STATSTR[i]);
}

// exit program, useful for scripts
void cmd_exit(int argc, char **argv){
	otfrm->Close();
}

// execute a test script file
void cmd_run(int argc, char **argv){

	if(argc < 2){
		otfrm->RunScript(0);	// prompt for a script file and execute it
	}else{
		otfrm->RunScript(argv[1]);	// execute argument file script
	}
}

// load memory from s-rec file
void cmd_load(int argc, char **argv){
	char *fpath;

	if(argc < 2){
		fpath = 0;  // no path argument
	}else{
		fpath = argv[1];
	}
	LoadFile(fpath, 21);
	oprint("Done.");
}


// Write a signle SRecord line to 16-bit program memory (currently only DSC use)
// command line should be a single string to be written (like "s2049ab09800d880")
// example:  load16 S10780001234567800   <-- this will write 4 bytes "12345678" to address 8000
void cmd_load16(int argc, char **argv){
	char *srbuf;

	srbuf = argv[1];
	LoadFile(srbuf, 0x32);
}

//---------------------------------------------------------------------------
// Memory Modify - Byte

// test: wb 40100022 5
void cmd_write8(int argc, char **argv){
	int i;

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3 ){	// if memory type argument
		if(argv[3][0] == 'p' || argv[3][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	outbuf[0] = StrToInt("0x"+AnsiString(argv[2]));

	i = osbdm_write_block(mtype, 8, address, outbuf, 1);	// write 1, 8-bit value
	oprintf(" - wrote %02x to %x", outbuf[0], address);	// display result

/*
	if(cmd_script==false){	// if not in script mode, display output
		cmd_val = read8(address);   // value saved in global: cmd_val
		oprintf("[%x] = %02x", address, cmd_val);
	}
*/
}
//---------------------------------------------------------------------------
// Memory Read - Byte

// test: rb 40100022
void cmd_read8(int argc, char **argv){
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 2){	// if memory type argument
		if(argv[2][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	if(osbdm_read_block(mtype, 8, address, inbuf, 1)) 	// read 1, 8-bit value
		return oprint("ERROR receiving data");
//	testdump("Received", 1, inbuf);
	oprintf("Read: %02X from address %x", *inbuf, address);

}

//---------------------------------------------------------------------------
// Memory Modify - Word

// test: ww 40100022 a573
void cmd_write16(int argc, char **argv){
	int i;

	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	*(unsigned int *)outbuf = StrToInt("0x"+AnsiString(argv[2]));

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3 ){	// if memory type argument
		if(argv[3][0] == 'p' || argv[3][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	// swap the bytes if this is a DSC target
	// this was done so the Big-Endian JM60 could have data it's way
	// also this may change in future firmware 
	if(otfrm->TargetBox->Text == "DSC"){
		char c = outbuf[0];
		outbuf[0] = outbuf[1];
		outbuf[1] = c;
	}

	i = osbdm_write_block(mtype, 16, address, outbuf, 2);	// write 1, 16-bit value
	oprintf(" - wrote %04x to %x", *(unsigned int *)outbuf, address);	// display result
}
//---------------------------------------------------------------------------
// Memory Read - Word

// test: rw 40100022
void cmd_read16(int argc, char **argv){
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 2){	// if memory type argument
		if(argv[2][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	if(osbdm_read_block(mtype, 16, address, inbuf, 2)) 	// read 1, 16-bit value
		return oprint("ERROR receiving data");
//	testdump("Received", 2, inbuf);
//	oprintf("Received: %04X", *(unsigned short *)inbuf);
	oprintf("Read: %04X from address %x", *(unsigned short *)inbuf, address);
}


unsigned long int ByteSwap32(unsigned long val){
	return(	(val&0xFF000000)>>24|
			(val&0x00FF0000)>>8 |
			(val&0x0000FF00)<<8 |
			(val&0x000000FF)<<24 );
}

//---------------------------------------------------------------------------
// Memory Modify - Long

// test: wl 40100022 12345678
void cmd_write32(int argc, char **argv){
	int i;
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long int val = StrToInt("0x"+AnsiString(argv[2]));

	// if this is a DSC target, swap the bytes
	// this was done so the Big-Endian JM60 could have data it's way
	// also this may change in future firmware
	if(otfrm->TargetBox->Text == "DSC"){
		val = ByteSwap32(val);
	}

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3 ){	// if memory type argument
		if(argv[3][0] == 'p' || argv[3][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	osbdm_write32(mtype, address, val);	// write 1, 32-bit value
	oprintf(" - wrote %08x to %x", val, address);	// display result
}
//---------------------------------------------------------------------------
// Memory Read - Long

// test: rl 40100022
void cmd_read32(int argc, char **argv){
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));

	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 2){	// if memory type argument
		if(argv[2][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	if(osbdm_read_block(mtype, 32, address, inbuf, 4)) 	// read 1, 16-bit value
		return oprint("ERROR receiving data");
//	oprintf("Received: %08X", *(unsigned long *)inbuf);
	oprintf("Read: %08X from address %x", *(unsigned long *)inbuf, address);
}

/*

// NOTE: -----> the following functions need optomized for size
// move most of the code below to subfunctions....
// use bdm fill commands

// Block Fill
void cmd_blockfill8(int argc, char **argv){
	const char *STATSTR[] = {"Data Written OK", "ERROR Writing Data"};
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long int address2 = StrToInt("0x"+AnsiString(argv[2]));
	unsigned char value = StrToInt("0x"+AnsiString(argv[3]));

	unsigned char data[4];
	unsigned long count;
	if(address2 > address)	count = address2 - address;
	else					count = address - address2;

	if(count == 0)	return;

	data[0] = value;

	oprintf("filling %x-%x with %x...", address, address2, value);
	int i = osbdm_write_fill(MEM_RAM, 8, address, data, count);
	oprintf(" - %s -", STATSTR[i]);
}
void cmd_blockfill16(int argc, char **argv){
	const char *STATSTR[] = {"Data Written OK", "ERROR Writing Data"};
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long int address2 = StrToInt("0x"+AnsiString(argv[2]));
	unsigned int value = StrToInt("0x"+AnsiString(argv[3]));

	unsigned char data[4];
	unsigned long count;
	if(address2 > address)	count = address2 - address;
	else					count = address - address2;

	if(count == 0)	return;

	data[0] = (value >> 8);
	data[1] = (value & 0xFF);

	oprintf("filling %x-%x with %x...", address, address2, value);
	int i = osbdm_write_fill(MEM_RAM, 16, address, data, count);
	oprintf(" - %s -", STATSTR[i]);
}
void cmd_blockfill32(int argc, char **argv){
	const char *STATSTR[] = {"Data Written OK", "ERROR Writing Data"};
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long int address2 = StrToInt("0x"+AnsiString(argv[2]));
	unsigned long int value = StrToInt("0x"+AnsiString(argv[3]));

	unsigned char data[4];
	unsigned long count;
	if(address2 > address)	count = address2 - address;
	else					count = address - address2;

	if(count == 0)	return;

	data[0] = (value >> 24);
	data[1] = (value >> 16);
	data[2] = (value >> 8);
	data[3] = (value & 0xFF);

	oprintf("filling %x-%x with %x...", address, address2, value);
	int i = osbdm_write_fill(MEM_RAM, 32, address, data, count);
	oprintf(" - %s -", STATSTR[i]);
}
*/

//---------------------------------------------------------------------------
// Register Display
void cmd_regdisplay(int argc, char **argv){
	int i;
	unsigned long address, val;

	// figure out register address
	if(argv[1][1] < '0' || argv[1][1] > '7') return oprint("ERROR:  use A0 or D7 for example");
	address = argv[1][1] - '0';

	if(argv[1][0] == 'A' || argv[1][0] == 'a'){
		address += 8;
	}

//	i = osbdm_read_block(MEM_REG, 32, address, inbuf, 4);	// write 1, 32-bit value
//	if(i) return oprintf("error reading registers");
//	val = *(unsigned long *)inbuf;

	val = osbdm_read32(MEM_REG, address);	// read a 32-bit value
	oprintf("%s = %08x", argv[1], val);
}

//---------------------------------------------------------------------------
// Register Modify - Long

// test: wl 40100022 12345678
void cmd_regmodify(int argc, char **argv){
	int i;
	unsigned long address;

	// figure out register address
	if(argv[1][1] < '0' || argv[1][1] > '7') return oprint("ERROR:  use A0 or D7 for example");
	address = argv[1][1] - '0';

	if(argv[1][0] == 'A' || argv[1][0] == 'a'){
		address += 8;
	}

	// get value to write
//	*(unsigned long int *)outbuf = StrToInt("0x"+AnsiString(argv[2]));
//	i = osbdm_write_block(MEM_REG, 32, address, outbuf, 4);	// write 1, 32-bit value

	unsigned long val = StrToInt("0x"+AnsiString(argv[2]));
	i = osbdm_write32(MEM_REG, address, val);

	oprintf(" - wrote %08x to %x", val, address);	// display result
}


//---------------------------------------------------------------------------
// Control Register Modify - Long

// test: wl 40100022 12345678
void cmd_cregmodify(int argc, char **argv){
	int i;
	unsigned long int address = (unsigned long int ) StrToInt("0x"+AnsiString(argv[1]));
	address &= 0x0000FFFF;

	// get value to write
//	*(unsigned long int *)outbuf = StrToInt("0x"+AnsiString(argv[2]));
//	i = osbdm_write_block(MEM_CREG, 32, address, outbuf, 4);

	unsigned long val = StrToInt("0x"+AnsiString(argv[2]));
	i = osbdm_write32(MEM_CREG, address, val);	// write control register
	oprintf(" - wrote %08x to %x", val, address);	// display result
}
//---------------------------------------------------------------------------
// Control Register Display
void cmd_cregdisplay(int argc, char **argv){
	int i;
	unsigned long val;
	unsigned long int address = (unsigned long int) StrToInt("0x"+AnsiString(argv[1]));

	i = osbdm_read_block(MEM_CREG, 32, address, inbuf, 4);	// read 1, 32-bit value
	if(i) return oprintf("error reading register");

	val = *(unsigned long *)(inbuf);
	oprintf("%04x = %08x", address, val);
}

//---------------------------------------------------------------------------
// Debug Register Modify
void cmd_dregmodify(int argc, char **argv){
	const char *STATSTR[] = {"Register Written OK", "ERROR Writing Data"};
	int i;
	unsigned long address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long val = StrToInt("0x"+AnsiString(argv[2]));

	i = osbdm_write32(MEM_DREG, address, val);	// write debug register

	oprintf(" - %s -", STATSTR[i]);	// display result
}
//---------------------------------------------------------------------------
// Debug Register Display
void cmd_dregdisplay(int argc, char **argv){
	int i;
	unsigned long val;
	unsigned long address = StrToInt("0x"+AnsiString(argv[1]));

	i = osbdm_read_block(MEM_DREG, 32, address, inbuf, 4);	// read 1, 32-bit value
	if(i) return oprintf("error reading register");

	val = getbuf4(inbuf);
	oprintf("%02x = %08x", address, val);
}


// dumps memory to the display for testing
void linedump(char width, unsigned long addr, int count, unsigned char *s){
	String str;
	str.printf("%04x:%04x  ", (addr>>16), addr & 0xFFFF);
	switch(width){
		case 16:
			for(int i=0; i<count; i+=2, s+=2){
				str.cat_sprintf("%04X ", *(unsigned short *)s);
			}
			break;
		case 32:
			for(int i=0; i<count; i+=4, s+=4){
				str.cat_sprintf("%08X ", *(unsigned long *)s);
			}
			break;
		default:
			for(int i=0; i<count; i++, s++){
				str.cat_sprintf("%02x ", *s);
			}
	}
	oprint(str);
}

// memory dump
int memory_dump(unsigned char mtype, unsigned char width, unsigned long addr, int bytes_per_row, unsigned long size){

	int rows;
	unsigned long addr2;
	unsigned char *ptr;

	// calculate number of rows to display
	if(size > bytes_per_row){
		rows = (int) (size / bytes_per_row);
		if(size % bytes_per_row) ++rows;	// incriment if remainder

	}else{
		rows=1;
	}

	// allocate buffer to hold data
   unsigned char* buffer= (unsigned char*)malloc(size * sizeof(unsigned char));
   if(buffer == NULL) return 1;

	// clear buffer so it displays FF for unread data
// doesn't work...	for(int i=size; i>0; i--)	buffer[i-1] = 0xff;

	// read data
	if(osbdm_read_block(mtype, width, addr, buffer, size)){
		free(buffer);
		return 2;
	}

	// display data dump
	addr2 = addr;
	ptr = buffer;
	int bytes_left = size;
	for(int rcount=0; rcount < rows; rcount++){
		if(bytes_left < bytes_per_row) bytes_per_row = bytes_left;
		linedump(width, addr2, bytes_per_row, ptr);
		bytes_left -= bytes_per_row;
		addr2 += bytes_per_row;
		ptr += bytes_per_row;
		// need fix for displaying outside buffer, this is a quick test function
	}

	cmd_addr_save = addr + size;	// save next address row
	free(buffer);
}

// memory dump - 8-bit
void cmd_memorydump(int argc, char **argv){
	unsigned long addr, size;

	if(argc > 1){	// if address arguments
		addr = StrToInt("0x"+AnsiString(argv[1]));
		if(argc > 2){
			size = StrToInt("0x"+AnsiString(argv[2]));
		}else{
			size = 0x100;
		}
	}else{
		addr = cmd_addr_save;
	}
	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3){	// if memory type argument
		if(argv[3][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	memory_dump(mtype, 8, addr, 0x10, size);

}

// <addr> [size]
void cmd_memorydumpl(int argc, char **argv){
	unsigned long addr, size;

	if(argc > 1){	// if address arguments
		addr = StrToInt("0x"+AnsiString(argv[1]));
		if(argc > 2){
			size = StrToInt("0x"+AnsiString(argv[2]));
		}else{
			size = 0x100;
		}
	}else{
		addr = cmd_addr_save;
	}
	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3){	// if memory type argument
		if(argv[3][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	memory_dump(mtype, 32, addr, 0x14, size);
}
// <addr> [size]
void cmd_memorydumpw(int argc, char **argv){
	unsigned long addr, size;

	if(argc > 1){	// if address arguments
		addr = StrToInt("0x"+AnsiString(argv[1]));
		if(argc > 2){
			size = StrToInt("0x"+AnsiString(argv[2]));
		}else{
			size = 0x100;
		}
	}else{
		addr = cmd_addr_save;
	}
	unsigned char mtype = MEM_RAM;	// default memory type is ram
	if( argc > 3){	// if memory type argument
		if(argv[3][0] == 'p' || argv[2][0] == 'P'){
			mtype = MEM_P;	// memory type is P
		}
	}

	memory_dump(mtype, 16, addr, 0x14, size);
}


void __fastcall Totfrm::TesterTimerTimer(TObject *Sender)
{
	TesterTimer->Enabled = false;
	++TesterTime;
}

//---------------------------------------------------------------------------
// Write a block of memory, read it back and return 0 if it read back the same

int osbdm_write_verify(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size) {
	osbdm_write_block(type, width, addr, data, size);	// write block
	osbdm_read_block(type, width, addr, inbuf, size); 	// read it back
	for(int i=0; i< size; i++){
		if(inbuf[i] != data[i]){
			oprintf("FAILED --- at address %X - wrote: %02X read %02x", addr+i, data[i], inbuf[i]);
			return 1;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------
// Test Memory

// testmem 8 80 FF
void cmd_testmem(int argc, char **argv){

	unsigned char size = StrToInt(AnsiString(argv[1]));
	unsigned long int saddr = StrToInt("0x"+AnsiString(argv[2]));
	unsigned long int eaddr = StrToInt("0x"+AnsiString(argv[3]));
	unsigned long int bytesleft = eaddr - saddr;

	oprintf("%d bit Test from %x - %x ...", size, saddr, eaddr);

	for(int i=0; i< MAX_DATA_SIZE; i++)	outbuf[i] = i;	// fill buffer

	for(unsigned long i=saddr; i<eaddr; ){
		if(bytesleft <= MAX_DATA_SIZE){
			if(osbdm_write_verify(MEM_RAM, size, saddr, outbuf, bytesleft) != 0) return;	// write block
			break;
		}
		if(osbdm_write_verify(MEM_RAM, size, saddr, outbuf, MAX_DATA_SIZE) != 0) return;	// write block
		saddr += MAX_DATA_SIZE;
		bytesleft -= MAX_DATA_SIZE;
	}
	oprintf("Test Passed.");
}

//---------------------------------------------------------------------------
// Test FLASH Memory (16-bit only)

// testflash 100 180
void cmd_testflash(int argc, char **argv){

	unsigned long int saddr = StrToInt("0x"+AnsiString(argv[1]));
	unsigned long int eaddr = StrToInt("0x"+AnsiString(argv[2]));
	unsigned long int bytesleft = eaddr - saddr;

	oprintf("Flash Test from %x - %x ...", saddr, eaddr);

	for(int i=0; i< MAX_DATA_SIZE; i++)	outbuf[i] = i;	// fill buffer

	for(unsigned long i=saddr; i<eaddr; ){
		if(bytesleft <= MAX_DATA_SIZE){
			if(osbdm_write_verify(MEM_P_FLASH, 16, saddr, outbuf, bytesleft) != 0) return;	// write block
			break;
		}
		if(osbdm_write_verify(MEM_P_FLASH, 16, saddr, outbuf, MAX_DATA_SIZE) != 0) return;	// write block
		saddr += MAX_DATA_SIZE;
		bytesleft -= MAX_DATA_SIZE;
	}
	oprintf("Test Passed.");
}

// unsecure and erase flash
void cmd_unsecure(int argc, char **argv){
	const char *STATSTR[] = {"Unsecure completed OK", "Unsecure FAILED"};
	int i;

	i = osbdm_flash_unsecure(0x17, 5, 0x5F);	// Coldire unsecure for 5213 board
	oprintf(" - %s -", STATSTR[i]);
}

//---------------------------------------------------------------------------

void __fastcall Totfrm::HelpClick(TObject *Sender)
{
	const String Hfile = ProgramDir + "osbdmtester.rtf";

	// if file exists, load it
	if (FileExists(Hfile)){
		othelpf->RichEdit1->Lines->LoadFromFile(Hfile);
		othelpf->Caption = "OSBDM Tester Help";
		othelpf->Show();	// display it
	}
}
//---------------------------------------------------------------------------
// StrHexNum converts a series of characters in a string to an integer
unsigned short StrHexNum(char *ptr, char bytes){
	return StrToInt("0x"+AnsiString(ptr).SubString(1,bytes*2));
}
// Write a signle SRecord line to memory on the board
// srbuf = buffer that holds a single string to be written (like "s2049ab09800d880")
char WriteSRecLine(char *srbuf){
	return LoadFile(srbuf, 0x31);
}

//---------------------------------------------------------------------------
/*	LoadFile
	--------
	Read an S-Record file and process it in various ways

	Returns 0 if file processed OK, otherwise returns 1

 ProcessType:   0 = just return with the Global variables set
				20 = read one line at a time and WRITE each to the board - RAM
				21 = same as 20 but doesn't display status info
				26 = read one line at a time and WRITE each to the board - FLASH
				27 = same as 26 but doesn't display status info
				31 = same as 21 except fpath is a pointer to a single string to be written (like "s2049ab09800d880")
				32 = same as 31 except writes to 16-bit program memory (ie. DSC)

 Global variables set:
	StartAddr = lowest data address read from file
	EndAddr = highest data address read from file

	see also: dump_section of elf.cpp
*/
char LoadFile(char *fpath, char ProcessType){
	FILE *fin;
	char *RSbptr;
	char srbuf[256];
	char swbuf[256];
	Byte SRectype, srcount;
	String AddressMessage;
	String FilePath;

	char fdrive[MAXDRIVE];
	char fdir[MAXDIR];
	char fname[MAXFILE];
	char fext[MAXEXT];
	int flags;

	// if no file provided, prompt the user for one
	if(fpath == 0){
		// show dialog box to get file name from user
		otfrm->OpenDialog->FilterIndex = 2;	// filter program files (.s19, hex)
		if (otfrm->OpenDialog->Execute()){
			FilePath = otfrm->OpenDialog->FileName;
			fpath = FilePath.c_str();
		}
		else	return 1;
	}
	if(ProcessType != 0x31 && ProcessType != 0x32){
		// split file path into its components
		flags=fnsplit(fpath,fdrive,fdir,fname,fext);

		// determine file type by its extension
		if(flags & EXTENSION){
			// S-Record File
			if(!strcmpi(fext,".s19")){
				if ((fin = fopen(fpath, "rt")) == NULL){
					werror("Cannot open file: %s", fpath);
					return 1;
				}
			}
			else{
				werror("Invalid file extension: %s", fext);
				return 1;
			}
		}
		else{
			werror("Invalid file extension: %s", fext);
			return 1;
		}
	}


	if(ProcessType == 0)	 srTotalBytes = 0;

	StartAddr = 0xFFFFFFFF;
	EndAddr = 0; lastS1=0; lastS2=0;
//	barcount=0;

	if(cmd_script==false){	// if not in script mode
		oprintf("Loading %s", fpath);
		Application->ProcessMessages();
	}

	while((ProcessType == 0x31) || (ProcessType == 0x32) || (!feof(fin))){
		if(ProcessType != 0x31 && ProcessType != 0x32)		fgets(srbuf, 255, fin);
		else						strcpy(srbuf, fpath);

		if(srbuf[0] != 'S'){
			Application->MessageBox("Not a valid S-Record file", "ERROR", MB_OK);
			fclose(fin);
			return 1;
		}
		SRectype = srbuf[1];        // get record type

		if(SRectype == '9' || SRectype == '7' || SRectype == '8'){        // if end of file record, finish
			break;
		}
		else if(SRectype == '0'){        // if header record, ignore
			continue;
		}
		else if(SRectype == '1'){        // if s1 record
			wAddr = StrHexNum(srbuf+4, 2);  // get address
			RSbptr = srbuf+8;
			srcount = (char) ( StrHexNum(srbuf+2, 1) -3 );  // get data count
			if(lastS1 < wAddr) lastS1 = (unsigned long) (wAddr-1) + srcount;
		}
		else if(SRectype == '2'){        // if s2 record
			wAddr = StrHexNum(srbuf+4, 1);  // get high address
			wAddr *= 0x10000;
			wAddr += StrHexNum(srbuf+6, 2);  // get low address
			RSbptr = srbuf+10;
			srcount = (char) ( StrHexNum(srbuf+2, 1) -4 );  // get data count
			if(lastS2 < wAddr) lastS2 = (unsigned long) (wAddr-1) + srcount;
		}
		else if(SRectype == '3'){        // if s3 record
			wAddr = StrHexNum(srbuf+4, 2);  // get high address
			wAddr *= 0x10000;
			wAddr += StrHexNum(srbuf+8, 2);  // get low address
			RSbptr = srbuf+12;
			srcount = (char) ( StrHexNum(srbuf+2, 1) -5 );  // get data count
		}
		else{
			Application->MessageBox("This is not a supported S-Record format", "ERROR", MB_OK);
			fclose(fin);
			return 1;
		}

		if(StartAddr > wAddr)   StartAddr = wAddr;	// adjust lowest address global

		if(ProcessType == 0){
			wAddr += srcount;
			srTotalBytes += srcount;
		}
		else{
			bAddr = wAddr;	// save start address for this record block
			// read each data byte into a memory array
			for(char bcnt=0; bcnt < srcount; bcnt++, RSbptr+=2){
				swbuf[bcnt] = StrHexNum(RSbptr, 1);   // store data byte
				++wAddr;
			}
			if(ProcessType == 0x31){
//				writeblock(bAddr, srcount, swbuf);	// Write block of data to memory
				osbdm_write_block(MEM_RAM, 8, bAddr, swbuf, srcount);
				break;	// stop if only writing one line
			}
			if(ProcessType == 0x32){
				osbdm_write_block(MEM_P, 16, bAddr, swbuf, srcount);
				break;	// stop if only writing one line
			}
			if(ProcessType >= 20){
				if((ProcessType & 0x01) == 0){
					oprintf("Writing %x..", bAddr);
				}
/*				if(ProcessType > 25){
					if(WriteFlashBlock(bAddr, swbuf, srcount) == 0){
						fclose(fin);	// error
						return 1;
					}
				}
				else{
*/
					// write big block of data
//oprintf("addr: %08x  count=%d", bAddr, srcount);
//					if(osbdm_write_block(MEM_RAM, 8, bAddr, swbuf, srcount) != 0){
					if(osbdm_write_block(MEM_RAM, 32, bAddr, swbuf, srcount) != 0){
						oprint("ERROR writing block");
						return 1;
					}

//				}

/*				if(CommandMode == false){
					barcount+=srcount;
					UpdateWriteFileBar(barcount, srTotalBytes);
				}
*/
			}
		}
		if(EndAddr < wAddr) EndAddr = (unsigned long) (wAddr-1);
	}
	if(ProcessType != 0x31)	fclose(fin);

	if(ProcessType > 9 && ProcessType < 20){
		StartAddr &= 0xFFFF00;
		EndAddr |= 0x0000FF;
		wAddr = bAddr = StartAddr;
	}
	return 0;
}


// read serial data
void cmd_sci_read(int argc, char **argv){
	int c;
	String str;

	if(osbdm_sci_read(inbuf)) return cmd_error();	// read serial port
	c = inbuf[1] - 2;	// data count
	oprintf("%d received", c);
	if(c){
		for(int i=0; i<c; i++){
//			str.cat_sprintf("%02x ", *(inbuf+2+i));
			str.cat_sprintf("%c", *(inbuf+2+i));
		}
		oprint(str);
	}
}

// write serial data
void cmd_sci_write(int argc, char **argv){

	int len = strlen(argv[1]);
	if(len > 62) len=62;	// max size of data to send
	if(osbdm_sci_write(len, argv[1])) return cmd_error();	// send data to serial port
	oprintf("%d bytes sent", len);
}

//---------------------------------------------------------------------------
// 16-bit Flash memory write

// test: ww 40100022 a573
void cmd_fwrite16(int argc, char **argv){
	int i;
	unsigned char mtype;

	mtype = MEM_P_FLASH;	// default memory type is Program Flash
	if( argc > 3 ){	// if memory type argument
		if(argv[3][0] == 'x' || argv[3][0] == 'X'){
			mtype = MEM_X_FLASH;	// memory type is X Flash
		}
	}

	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	*(unsigned int *)outbuf = StrToInt("0x"+AnsiString(argv[2]));

	// swap the bytes if this is a DSC target
	// this was done so the Big-Endian JM60 could have data it's way
	// also this may change in future firmware 
	if(otfrm->TargetBox->Text == "DSC"){
		char c = outbuf[0];
		outbuf[0] = outbuf[1];
		outbuf[1] = c;
	}

	i = osbdm_write_block(mtype, 16, address, outbuf, 2);	// write 1, 16-bit value
	oprintf(" - wrote %04x to %x", *(unsigned int *)outbuf, address);	// display result
}

//---------------------------------------------------------------------------
// 16-bit Flash memory read

void cmd_fread16(int argc, char **argv){
	unsigned long int address = StrToInt("0x"+AnsiString(argv[1]));
	unsigned char mtype;

	mtype = MEM_P_FLASH;	// default memory type is Program Flash
	if( argc > 2 ){	// if memory type argument
		if(argv[2][0] == 'x' || argv[2][0] == 'X'){
			mtype = MEM_X_FLASH;	// memory type is X Flash
		}
	}

	if(osbdm_read_block(mtype, 16, address, inbuf, 2)) 	// read 1, 16-bit value
		return oprint("ERROR receiving data");
//	testdump("Received", 2, inbuf);
	oprintf("Read: %04X from address %x", *(unsigned short *)inbuf, address);
}




//---------------------------------------------------------------------------
void cmd_test(int argc, char **argv){

	unsigned char buffer[2048];
	unsigned char adata[2]={0x5a, 0x5a};
	int r;



// TEMP remove this after testing
otfrm->TargetBox->Text = "RS08";

	unsigned char rstat;

	// if Coldfire test
	if(otfrm->TargetBox->Text == "CF"){

		// enable PST[0:3] on 5213
		buffer[0] = 0xFF;
		osbdm_write_block(MEM_RAM, 8, 0x40100051, buffer, 1);	// write 1, 8-bit value


		oprintf("Setting ram to 0x2000:0000");

		osbdm_write32(MEM_CREG, 0x801, 0x20000000);	// write control register
		osbdm_write32(MEM_CREG, 0xC05, 0x20000021);	// write control register
		osbdm_write32(MEM_CREG, 0xC04, 0x00000061);	// write control register

		for(int i=0; i<1024; i++)
			*(buffer+i) = '1'+(i&0xFF);	// fill buffer

		r = osbdm_write_block(MEM_RAM, 8, 0x20000000, buffer, 1024);	// write big block of data
		oprintf("T2 returned %d", r);
		memory_dump(MEM_RAM, 8, 0x20000000, 0x10, 0x100); 	// dump

		for(int i=0; i<1024; i+=2)
			*(unsigned short *)(buffer+i) = i;	// fill buffer

		r = osbdm_write_block(MEM_RAM, 16, 0x20000000, buffer, 1024);	// write big block of data
		oprintf("T2 returned %d", r);
		memory_dump(MEM_RAM, 16, 0x20000000, 0x14, 0x100);

		for(int i=0; i<1024; i+=4)
			*(unsigned long *)(buffer+i) = i;	// fill buffer

		r = osbdm_write_block(MEM_RAM, 32, 0x20000000, buffer, 1024);	// write big block of data
		oprintf("T2 returned %d", r);
		memory_dump(MEM_RAM, 32, 0x20000000, 0x14, 0x100);
	}

	// if RS08 test
	if(otfrm->TargetBox->Text == "RS08"){

		// test flash programming

		osbdm_target_halt();	// make sure target is halted


		osbdm_write8(MEM_RAM, 0x201, 0x02);	// disable watchdog timer

		osbdm_flash_dlstart();

		buffer[0] = 0xAC;  // NOP;
		buffer[1] = 0xAC;  // NOP;
		buffer[2] = 0xAC;  // NOP;
		buffer[3] = 0x30;  //
		buffer[4] = 0xFD;  // BRA *-1; abs=0x32

		osbdm_write_block(MEM_RAM, 8, 0x30, buffer, 5);	// write 5 8-bit values


		osbdm_flash_prog(0x30, 2, adata);

		if(osbdm_status(inbuf)) return oprint("ERROR: cannot get status");

		rstat = inbuf[9];

		osbdm_flash_dlend();

		oprintf("done.  rstat=%x", rstat);
	}

}


