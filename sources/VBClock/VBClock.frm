VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "VBClock"
   ClientHeight    =   375
   ClientLeft      =   3570
   ClientTop       =   2460
   ClientWidth     =   1695
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   375
   ScaleWidth      =   1695
   ShowInTaskbar   =   0   'False
   Begin VB.Timer Timer 
      Interval        =   1000
      Left            =   1245
      Top             =   -15
   End
   Begin VB.Label lab 
      Alignment       =   2  'Center
      Caption         =   "Date/Time with Visual Basic"
      BeginProperty Font 
         Name            =   "Courier"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   0
      TabIndex        =   0
      Top             =   -15
      Width           =   1680
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Timer_Timer()
    lab.Caption = Date & vbCrLf & Time
End Sub
