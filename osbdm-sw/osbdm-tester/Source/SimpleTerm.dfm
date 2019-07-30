object TermFrm: TTermFrm
  Left = 322
  Top = 971
  Width = 561
  Height = 477
  Caption = 'TermFrm'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Memo: TMemo
    Left = 0
    Top = 29
    Width = 553
    Height = 419
    Align = alClient
    Color = clBlack
    Font.Charset = ANSI_CHARSET
    Font.Color = clSilver
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 0
    OnKeyDown = MemoKeyDown
    OnKeyPress = MemoKeyPress
  end
  object ToolBar1: TToolBar
    Left = 0
    Top = 0
    Width = 553
    Height = 29
    Caption = 'ToolBar1'
    TabOrder = 1
    object BitBtn1: TBitBtn
      Left = 0
      Top = 2
      Width = 75
      Height = 22
      Caption = 'Send File'
      TabOrder = 0
      OnClick = BitBtn1Click
    end
  end
  object STimer: TTimer
    Enabled = False
    Interval = 200
    OnTimer = STimerTimer
    Left = 32
    Top = 368
  end
  object OpenDialog1: TOpenDialog
    Left = 64
    Top = 368
  end
end
