//---------------------------------------------------------------------------

#ifndef SimpleTermH
#define SimpleTermH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TTermFrm : public TForm
{
__published:	// IDE-managed Components
	TMemo *Memo;
	TTimer *STimer;
	TToolBar *ToolBar1;
	TOpenDialog *OpenDialog1;
	TBitBtn *BitBtn1;
	void __fastcall MemoKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall STimerTimer(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall MemoKeyPress(TObject *Sender, char &Key);
	void __fastcall BitBtn1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TTermFrm(TComponent* Owner);
	void SendFile(String filepath);
	char PollForSerial();


};
//---------------------------------------------------------------------------
extern PACKAGE TTermFrm *TermFrm;
//---------------------------------------------------------------------------
#endif
