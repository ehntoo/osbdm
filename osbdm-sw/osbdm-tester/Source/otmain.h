//----------------------------------------------------------------------------
#ifndef otmainH
#define otmainH
//----------------------------------------------------------------------------
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <StdActns.hpp>
#include <ToolWin.hpp>
//----------------------------------------------------------------------------
class Totfrm : public TForm
{
__published:
    TOpenDialog *OpenDialog;
    TSaveDialog *SaveDialog;
    TToolBar *ToolBar1;
    TToolButton *ToolButton4;
    TToolButton *ToolButton5;
    TToolButton *ToolButton6;
    TActionList *ActionList1;
    TAction *FileOpen1;
    TAction *FileSave1;
    TAction *FileSaveAs1;
    TAction *FileExit1;
    TEditCut *EditCut1;
    TEditCopy *EditCopy1;
    TEditPaste *EditPaste1;
    TAction *HelpAbout1;
    TStatusBar *StatusBar;
    TImageList *ImageList1;
	TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *FileOpenItem;
    TMenuItem *FileSaveItem;
    TMenuItem *FileSaveAsItem;
    TMenuItem *N1;
    TMenuItem *FileExitItem;
    TMenuItem *Edit1;
    TMenuItem *CutItem;
    TMenuItem *CopyItem;
    TMenuItem *PasteItem;
    TMenuItem *Help1;
    TMenuItem *HelpAboutItem;
	TPanel *Panel1;
	TSplitter *Splitter1;
	TPanel *Panel2;
	TMemo *Commands;
	TMemo *Output;
	TTimer *TesterTimer;
	TToolButton *ToolButton7;
	TBitBtn *BitBtn1;
	TBitBtn *AgainButton;
	TToolButton *ToolButton1;
	TMenuItem *Help;
	TMenuItem *N2;
	TBitBtn *TermButton;
	TComboBox *TargetBox;
	TLabel *Label1;
	TToolButton *ToolButton2;
		void __fastcall FileOpen1Execute(TObject *Sender);
		void __fastcall FileExit1Execute(TObject *Sender);
		void __fastcall FileSave1Execute(TObject *Sender);
		void __fastcall HelpAbout1Execute(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall CommandsKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall TesterTimerTimer(TObject *Sender);
	void __fastcall AgainButtonClick(TObject *Sender);
	void __fastcall HelpClick(TObject *Sender);
	void __fastcall TermButtonClick(TObject *Sender);
private:
public:
	virtual __fastcall Totfrm(TComponent *AOwner);
	char ProcessTargetConfig(AnsiString Cfname);
	int RunScript(char *fpath);

};
//----------------------------------------------------------------------------
extern Totfrm *otfrm;
//----------------------------------------------------------------------------

// Other Functions

void werror (const char *message, ...);
int OpenBDM(char force);
void CloseBDM(char force);

int ProcessCommand(char *cbuf, bool ScriptMode);
char LoadFile(char *fpath, char ProcessType);
void cmd_help();

// Macros

#define VA_START(VA_LIST, VAR)  va_start(VA_LIST, VAR)
/* variadic function helper macros
 "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's  use without inhibiting further decls
 and without declaring an actual variable.  */
#define VA_OPEN(AP, VAR)    { va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)        } va_end(AP); }
#define VA_FIXEDARG(AP, T, N)   struct Qdmy


// Variables

typedef const struct
{
	char *  cmd;                    // command name
	int     min_args;               // min num of args command accepts
	void    (*func)(int, char **);  // function to call
	char *  desc;		            // brief description of command
	char *  syntax;                 // syntax of command

} CFG_CMD;

extern CFG_CMD CFG_CMDTAB[];
extern const int CFG_NUM_CMD;
#define CFG_CMDTAB_SIZE  (sizeof(CFG_CMDTAB)/sizeof(CFG_CMD))

#define CFG_MAX_ARGS	10		// maximum number of arguments per command
#define CFG_MAX_BUF     256		// maximum number of charcters per line


#endif
