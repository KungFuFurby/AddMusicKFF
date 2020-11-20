lorom 
header 
!MainCodeLocation	= $1E8000	;Point it to some free space
!FreeRAM		= $7007FF
!MAIN_CODE_SIZE		= END_OF_FILE-AddressTable
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
org $00807C
			JSL SPCUpload1
			NOP #$2
org $00808D
			JSL SPCUpload2
org $008096
			JSL SPCUpload2
org $0080B4
			JSL SPCUpload2
			JSL SPCUpload2
			TAX
			JSL SPCUpload3
			NOP #$3
org $008F65
			JSL Bonus
			NOP
org $009702
			NOP #$3

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

org $00973B
			NOP #$5
			JSL CustomMusicMain
			NOP #$2
org $00A1C7
			JSL ClearRAM 
org $00A64A
			LDA $0DDA
			AND #$7F
			JSL HackingRoutine 
			NOP
org $05855F
			BRA $02
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
org !MainCodeLocation
			db "STAR"
			dw !MAIN_CODE_SIZE-$01
			dw !MAIN_CODE_SIZE-$01^$FFFF
			db "@HFD" ;Used to identify the difference between mine and Romi's version
			db $05	;code version, for removing or modifying this main code in the future.

AddressTable:		;One pointer for every song
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	

			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	

			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	

			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	

			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	

			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	
			dl $000000,$000000,$000000,$000000	


CustomMusicMain: 	STZ $0DAE
			STZ $0DAF

			LDA $0100
			CMP #$11
			BNE Return

			LDA $0DDA
			CMP !FreeRAM
			CLC
			BEQ ActualNumber
			STA !FreeRAM
			CMP #$20
			BCC UploadLevel
CustomMusic:		REP #$30
			AND #$00FF
			JSR FixMusicNum
			NOP
			NOP
			NOP
			NOP
			TAX
			LDA AddressTable-$18,x
			PHA
			LDA AddressTable-$18+$01,x
			STA $04

			LDA #$8000
			STA $00
			SEP #$30
			LDA #$00

			PHK
			PER $0006
			PEA $804C
			JML $80811A

			REP #$20
			PLA
			STA $00
			SEP #$20
			LDX #$1D
.wait			DEX
			BNE .wait
			LDA $05

			PHK
			PER $0006
			PEA $804C
			JML $80811A


			SEC
ActualNumber:		LDA $0DDA
			BIT #$E0
			BEQ Clear1DFB
			AND #$03
			INC A
			ORA #$20
			STA $0DDA
			BCS StoreTo1DFB
Clear1DFB:		LDA #$00
StoreTo1DFB:		STA $1DFB
Return:			RTL

UploadLevel:		PHK
			PER $0006
			PEA $804C
			JML $008148
			LDA $0DDA
			BRA StoreTo1DFB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HackingRoutine:		AND #$3F
			STA $0DDA
			LDA #$FF
			STA !FreeRAM
			RTL

ClearRAM: 		LDA #$FF
			STA !FreeRAM
			JML $848241

Bonus:			LDA #$FF
			STA !FreeRAM
			STA $1425
			RTL

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SPCUpload1:		LDA $00
			AND #$7FFF
			TAY
			LDA #$8000
			STA $00
			LDA #$BBAA
			RTL


SPCUpload2:		LDA [$00],y
			INY
			BPL .R
			LDY #$0000
			INC $02
.R			XBA
			RTL

SPCUpload3:		JSL SPCUpload2
			JSL SPCUpload2
			STA $2142
			XBA
			STA $2143
			RTL
FixMusicNum:
			SEC 
			SBC #$0020 
			STA $00 
			ASL A 
			CLC 
			ADC $00 
			CLC 
			ADC #$0018 
			RTS


END_OF_FILE: