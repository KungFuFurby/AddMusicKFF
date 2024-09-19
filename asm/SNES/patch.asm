
;Don't stop program due to changing the mapper multiple times.
warnings disable Wmapper_already_set

!FreeROM		= $0E8000		; DO NOT TOUCH THESE, otherwise the program won't be able to determine where the data in your ROM is stored!
;!Data			= $0E8000		; Data+$0000 = Table of music data pointers 300 bytes long. 
						; 	The first few entries are bogus because they're global songs uploaded with the SPC engine.
						; Data+$0300 = Table of sample data pointers 300 bytes long.
						; Data+$0600 = Table of sample loop pointers (relative to $0000; modified in-game).
						; Note that the first few bytes of !Data are meta bytes, so it's actually something like !Data+$0308
						
if read1($00FFD5) == $23			; \
	!UsingSA1 = 1				; | Check if this ROM uses the SA-1 patch.	
else						; |
	!UsingSA1 = 0				; / 
endif


if !UsingSA1
	if read1($00FFD7) == $0D
		fullsa1rom
	else
		sa1rom
	endif
	!SA1Addr1 = $3000
	!SA1Addr2 = $6000
	!Bank     = $000000
else
	lorom
	!SA1Addr1 = $0000
	!SA1Addr2 = $0000
	!Bank     = $800000
endif






!Version = $00

!SampleGroupPtrsLoc	= $008000

!FreeRAM		= $7FB000
!CurrentSong		= !FreeRAM+$00
!NoUploadSamples	= !FreeRAM+$01
!CurrentSongGroup	= !FreeRAM+$02
!SongPositionLow	= !FreeRAM+$04
!SongPositionHigh	= !FreeRAM+$05
!SPCOutput1		= !SongPositionLow
!SPCOutput2		= !SongPositionHigh
!SPCOutput3		= !FreeRAM+$06
!SPCOutput4		= !FreeRAM+$07
;!MusicBackup		= !FreeRAM+$08
!SampleCount		= !FreeRAM+$09
!SRCNTableBuffer	= !FreeRAM+$0A

!Trick   = !FreeRAM+$08
!Tricker = !BonusEnd

; FREERAM requires anywhere between 2 to potentially 1032 bytes of unused RAM (though somewhere in the range of, say, 100 is much more likely).
; Normally you shouldn't need to change this.
;
; Format:
; FREERAM+$0000: Current song being played.
; FREERAM+$0001: Flag.  If set, sample data will not be transferred when switching songs.
; FREERAM+$0002: ARAM/DSP Address lo byte (see below).
; FREERAM+$0003: ARAM/DSP Address hi byte
; FREERAM+$0004: Value to write to ARAM/DSP
; FREERAM+$0005: Current song position in ticks.
; FREERAM+$0006: Hi byte of above.
; FREERAM+$0007: Echo buffer location.  Recommended that you don't touch this unless necessary.  It's modified every time sample upload occurs, and referenced every time music upload occurs.
; FREERAM+$0008: Number of samples in current song (between 0 and 255)
; FREERAM+$0009: Used as a buffer for the sample pointer/loop table.  Could be up to 1024 bytes long, but this is unlikely (4 bytes per sample; do the math).

; 1DFB: Use this to request a song to play (more or less default behavior).
; 0DDA: Song to play once star/P-switch runs out.  If $FF, don't restore.

; Special status byte details:


; To write to ARAM: Set FREERAM+$02 and FREERAM+$03 to the address, and FREERAM+$04 to the value to write.  Note that the address cannot be $FFxx.  
; To write to the S-DSP: Set FREERAM+$02 to the address, FREERAM+$03 to #$FF, and FREERAM+$04 to the value to write.



; BE VERY CAREFUL WHEN CHANGING THIS FILE.  To be perfectly blunt, the parsing the program
; uses to find certain things is as dead simple as it gets, so if you change any labels or 
; defines and the program can no longer find them, Bad Things (TM) may happen.  Best to 
; leave this file alone unless absolutely necessary.

!GlobalMusicCount = #$00	; Changed automatically

!SFX1DF9Reg = 	$2140
!SpecialReg = 	$2141
!MusicReg = 	$2142
!SFX1DFCReg = 	$2143	


!SFX1DF9Mir = 	$1DF9|!SA1Addr2
!SpecialMir = 	$1DFA|!SA1Addr2
!MusicMir = 	$1DFB|!SA1Addr2
!SFX1DFCMir = 	$1DFC|!SA1Addr2

!MusicBackup = $0DDA|!SA1Addr2


!DefARAMRet = $042F	; This is the address that the SPC will jump to after uploading a block of data normally.
!ExpARAMRet = $0400	; This is the address that the SPC will jump to after uploading a block of data that precedes another block of data (used when uploading multiple blocks).
			; All of these are changed automatically.
!SongCount = $00	; How many songs exist.  Used by the fading routine; changed automatically.

incsrc "tweaks.asm"			
			
	
padbyte $55		
org $0E8000		; Clear out what parts of bank E we can (Lunar Magic install some hacks there).
pad $0EF100
org $0F8000		; Clear out what parts of bank F we can (Lunar Magic install some more hacks there).
pad $0FF051
	
org $8075
	JML MainLabel

org !FreeROM
db "@AMK"			; Program name
db !Version
dl SampleGroupPtrs		; Position of the sample group pointers.
dl MusicPtrs			; Position of music and sample pointers.
db $00, $00, $00, $00, $00	; Reserve some space for meta data.
db $00, $00, $00, $00		;
db $00, $00, $00, $00		;
db $00, $00, $00, $00		;
db $00, $00, $00, $00		;

MainLabel:
	STZ $10
	PHP
	PHB
	PHK
	PLB
if !PSwitchStarRestart == !true
	lda !Trick
	beq +
	lda !CurrentSong
	sta !MusicReg
	lda #$00
	sta !Trick
+
endif
	JSR HandleSpecialSongs
	REP #$20
	LDA !SFX1DF9Reg
	STA !SPCOutput1
	LDA !MusicReg
	STA !SPCOutput3
	SEP #$20

	LDA !SpecialMir
	STA !SpecialReg
	LDA !SFX1DF9Mir
	STA !SFX1DF9Reg
	LDA !SFX1DFCMir
	STA !SFX1DFCReg
	STZ !SFX1DF9Mir
	STZ !SpecialMir
	STZ !SFX1DFCMir
	LDA !MusicMir
	BEQ NoMusic
	CMP !CurrentSong
	BNE ChangeMusic
if !PSwitchStarRestart == !true
	cmp !GlobalMusicCount+1
	bcc ChangeMusic
endif

End:	
if !PSwitchStarRestart == !true	
	stz !MusicMir
endif
	CLI
	PLB
	PLP


	JML $00806B|!Bank	; Return.  TODO: Detect if the ROM is using the Wait Replace patch.

NoMusic:
	LDA #$00
	STA !NoUploadSamples
	BRA End
	
Fade:
	STA !MusicReg
	;STA !CurrentSong
	BRA End

PlayDirect:
;	PHA
;	STA !MusicReg
;	LDA !CurrentSong
;	STA !MusicBackup
;	PLA
;	STA !CurrentSong
;	BRA End

;	STA !MusicReg
;	PHA
;	LDA !CurrentSong
;	CMP !GlobalMusicCount+1
;	BCC +
;	STA !MusicBackup
;+
;	PLA
;	STA !CurrentSong
;	BRA End

if !PSwitchStarRestart == !true
	cmp !CurrentSong
	beq +
endif

	STA !MusicReg
	STA !CurrentSong
	BRA End

if !PSwitchStarRestart == !true
+	lda #$01
	sta !Trick
	lda #!Tricker
	sta !MusicReg
	bra End
endif

	
ChangeMusic:
	LDA $187A|!SA1Addr2		; \ 
	BEQ +
	LDA #$FF
+
	SEC
	SBC #$FD
	STA !SpecialMir
	
	;STA $7FFFFF
	
;	LDA !MusicMir
if !PSwitchIsSFX == !false
;	CMP #!PSwitch
;	BEQ .doExtraChecks
endif
;	CMP #!Starman
;	BEQ .doExtraChecks
;	BRA .okay
;	
;.doExtraChecks			; We can't allow the p-switch or starman songs to play during the level clear themes.
	LDA !CurrentSong
	CMP #!StageClear
	BEQ LevelEndMusicChange
	CMP #!IrisOut
	BEQ LevelEndMusicChange
	CMP #!Keyhole
	BEQ LevelEndMusicChange
	CMP #!BossClear		;;; this one too
	BNE Okay
	
LevelEndMusicChange:
	LDA !MusicMir
	CMP #!IrisOut
	BEQ Okay
	CMP #!SwitchPalace	;;; bonus game fix
	BEQ Okay
	CMP #!Miss		;;; sure why not
	BEQ Okay
	CMP #!RescueEgg
	BEQ Okay		; Yep
	CMP #!StaffRoll	; Added credits check
	BEQ Okay
	LDA $0100|!SA1Addr2		
	CMP #$10			
	BCC Okay
	;;; LDA !CurrentSong	;;; this is why we got here in first place, seems redundant
	;;; CMP #!StageClear
	;;; BEQ EndWithCancel
	;;; CMP #!IrisOut
	;;; BEQ EndWithCancel
EndWithCancel:
if !PSwitchStarRestart == !false
	STZ !MusicMir
endif
	BRA End
	
Okay:
	LDA !MusicMir			; \ Global songs require no uploads.
	CMP !GlobalMusicCount+1		; |
	BCC PlayDirect			; /
	

	CMP #$FF			; \ #$FF is fade.
	BEQ Fade			; /

if or(and(equal(!PSwitchIsSFX,!false),notequal(!PSwitch,$00)),notequal(!Starman,$00))
	LDA !CurrentSong		; \ 
if !PSwitchIsSFX == !false && !PSwitch != $00
	CMP #!PSwitch			; |
	BEQ +				; |
endif
if !Starman != $00
	CMP #!Starman			; |
	BNE ++				; | Don't upload samples if we're coming back from the pswitch or starman musics.
endif
endif
	;;;BRA ++			; |
+					; |
	LDA $0100|!SA1Addr2		; | \
	CMP #$12+1			; | | But if we're coming back from the p-switch or starman musics AND we're loading a new level, then we might need to reload the song as well.
	BCC ++				; | / ;;; can't be bad to allow everything below
	LDA !MusicMir			; |
	CMP !MusicBackup		; |
	BNE ++				; |
	CMP !CurrentSongGroup		; |
	BNE ++				; |
	STA !CurrentSong		; |
	STA !MusicBackup		; |
	STA !CurrentSongGroup		; |
	JMP SPCNormal			; |
++					; /
	LDA !MusicMir
	STA !CurrentSong
	STA !MusicBackup
	
;	LDA $0100|!SA1Addr2
;	CMP #$0F
;	BCC .forceMusicToPlay
;	LDA !CurrentSong
;	CMP #!StageClear
;	BEQ EndWithCancel
;	CMP #!IrisOut
;	BEQ EndWithCancel
;.forceMusicToPlay



;	LDA !MusicMir
;	CMP !MusicBackup
;	BNE +
;	STA !CurrentSong
;	JMP SkipSPCNormal
;+
;	LDA #$00
;	STA !MusicBackup
;+	LDA !MusicMir		;
;	STA !CurrentSong
;
;+++


	LDA !MusicMir
	CMP #!SongCount
	BCC +
	ORA #$80
	JMP Fade
+
;	REP #$30
;	LDA !MusicMir		; If the selected music is invalid,
;	AND #$00FF		; then fade out.  This is to mimic #$80 being the value to fade out
;	STA $00			; since that's used a lot (while any negative value would work).
;	ASL			; Of course, this won't work correctly if the user has more than #$80 songs,
;	CLC			; But they'd have issues anyway.  Might as well try to avoid complications
;	ADC $00			; when it's possible.
;	INC
;	INC
;	TAX
;	SEP #$20
;	LDA MusicPtrs,x
;	SEP #$30
;	INC			; \ If the bank byte is 0 or FF, then this music is invalid.
;	CMP #$02		; /
;	BCS +
;	LDA #$FF
;	JMP Fade
;+

	STA !CurrentSongGroup
	

	LDA #$FF		; Send this as early as possible
	STA $2141		;

	LDA $0100|!SA1Addr2	;\     Check Gamemode.
    	CMP #$0E		; | If on the Overworld,
	BNE +			; |    
	WAI			;/     then wait for an interrupt to prevent garbage during Submap transitions.
    
+
	
	SEI
	
	STZ $4200		; Disable NMI.  While NMI no longer messes with the audio ports,
				; interrupts at the wrong time during this delicate routine are bad.
	
	REP #$30		; $108055
	;LDA #$0000
	;SEP #$20
	LDA !MusicMir
	;REP #$30
	AND #$00FF
	STA $00
	ASL			;\
	CLC			;| Multiply by 3.
	ADC $00			;/
	TAX			; To index
	SEP #$20		; the music
	LDA MusicPtrs,x		; table data.
	STA $00
	INX
	LDA MusicPtrs,x
	STA $01
	INX
	LDA MusicPtrs,x
	STA $02
	
	REP #$10
	LDX #!DefARAMRet
	LDA !NoUploadSamples
	BNE +
	LDX #!ExpARAMRet
+	STX $03
	SEP #$10
	JSL UploadSPCData
	
	
	;STZ $2140		; $1080a1
	;STZ $2142
	;STZ $2143
	
	LDA !NoUploadSamples
	BEQ +
	;$2141 is still outputting a $BB, courtesy of acknowledging that it
	;is ready for a data transfer. This is zeroed out upon the
	;completion of the transfer, so wait for that to happen first.
-:
	LDA $2141
	BNE -
	JMP SPCNormal
+
	
	REP #$20
	
	LDY #$02		; \
	LDA [$00]		; | 
	STA $09			; |
	LDA [$00],y		; | This puts the location of the ARAM sample table into $09.
	CLC			; |
	ADC $09			; |
	XBA : XBA		; | Test the low byte of the accumulator
	BEQ NoAdd		; |
	CLC			; | The low byte of the sample table must be 0.
	ADC #$0100		; |
	AND #$FF00		; |
NoAdd:	STA $09			; /
	

	SEP #$20
	
					
	LDA.b #SampleGroupPtrs		; [$0D] is the pointer to this song's sample group.
	STA $0D				; Sample groups contain the number of samples in their first byte
	LDA.b #SampleGroupPtrs>>8	; Then the sample numbers themselves after that.
	STA $0E
	LDA.b #SampleGroupPtrs>>16
	STA $0F
	
	LDA !MusicMir			; \
	REP #$30			; |
	AND #$00FF			; |
	ASL				; | Index the table by the music number
	TAY				; |
	LDA [$0D],y			; /
	STA $0D				; The sample groups are stored in the same bank as the table.
	SEP #$30
	
	LDA [$0D]			; Get the number of samples
	STA !SampleCount
	
	REP #$20
	AND #$00FF
	ASL
	STA $0B				; $0B is used as a copy because CPX $XXXXXX doesn't exist.
	ASL
	CLC
	ADC $09				; $09 contains the location of the ARAM SRCN table (positions and loop positions).
	STA $07				; $07 contains where each sample should go in ARAM.
	
	REP #$30
	LDX #$0000			; Clear out x and y.
	LDY #$0000
	INC $0D				; Point $0D to the sample ID collection.
	
.loop
	CPY $0B				; 108100
	BEQ NoMoreSamples
	PHY				; We use y for indexing as well; push it.

	TYA				; \
	ASL				; | Get index for the SRCN buffer table
	TAX				; / and save it in X.

	LDA [$0D], y			; Get the next sample
	STA $00
					; Check for duplicate samples.
.duplicateCheckLoop
	DEY
	DEY
	BMI .notDuplicateSample
	CMP [$0D], y
	BNE .duplicateCheckLoop
					; Duplicate sample detected!
					; Copy the corresponding SRCN buffer table entry over, and do no further loading.				
	STX $00				; X contains the current SRCN buffer table ID. Store it.
	TYA				; Y contains the duplicate SRCN buffer table ID divided by 2.
	ASL
	TAX
	TAY				;Save duplicate SRCN buffer table ID in Y.
	LDA !SRCNTableBuffer,x
	LDX $00				;Reload current SRCN buffer table ID in X.
	STA !SRCNTableBuffer,x
	TYX				;Reload duplicate SRCN buffer table ID from Y to X.
	LDA !SRCNTableBuffer+2,x
	LDX $00				;Reload current SRCN buffer table ID in X.
	STA !SRCNTableBuffer+2,x
	BRA .nextSample			;We're done.

.notDuplicateSample
	TXY
					; A contains the sample number

	ASL				; \ A contains the sample loop position table index
	TAX				; | X contains the sample loop position table index
	LDA SampleLoopPtrs,x		; | A contains this sample's loop position
	CLC				; |
	ADC $07				; | A contains this sample's loop position relative to its position in ARAM. 
	TYX				; | Copy y (the SRCN buffer table index) to x.
	STA !SRCNTableBuffer+2,x	; | Store the loop position to the SRCN table buffer.
	LDA $07				; | Copy the start position of the sample.
	STA !SRCNTableBuffer,x		; / Store the start position to the SRCN table buffer.
	
	LDA $00				; \
	ASL				; |
	CLC				; | Multiply by 3 to index the sample table data.
	ADC $00				; |
	TAX				; /
	
	;SEP #$10			
	LDA SamplePtrs,x
	STA $00
	INX
	INX

	SEP #$20
	LDA SamplePtrs,x
	STA $02
	
	REP #$20
	LDA #!ExpARAMRet
	STA $03
	LDA [$00]
	STA $05
	BEQ .empty			; If the sample's position in the ROM is 0 (which is invalid), skip it; it's empty.
	INC $00
	INC $00
	SEP #$30
	JSL UploadSPCDataDynamic
.empty
	REP #$30			; 10814f
	LDA $07
	CLC
	ADC $05
	STA $07
.nextSample
	PLY
	INY
	INY
	BRA .loop
NoMoreSamples:
					; $108166
	
	LDA $09				; \ $09 is the address of the ARAM SRCN table.
	STA $07				; |
	LDA $0B				; |
	ASL				; |
	STA $05				; |
	LDA.w #!SRCNTableBuffer		; |
	STA $00				; | Upload the ARAM SRCN table.
	SEP #$20			; |
	LDA.b #!SRCNTableBuffer>>16	; |
	STA $02				; |
	JSL UploadSPCDataDynamic	; /

	LDA $08				; \
	STA $0C				; | Set the DIR DSP register.
	LDA #$5D ;DIR DSP register	; | We will save the writes to make to direct pages $0B-$0C.
	STA $0B				; | 
	STZ $02				; | 
	REP #$20			; |
	TDC				; | In case of non-zero direct page register...
	CLC				; | 
	ADC #$000B			; | 
	STA $00				; | 
	LDA #$0002			; | 
	STA $05				; | 
	LDA #$00F2 ;DSPADDR		; | 
	STA $07				; | 
	LDA #!DefARAMRet		; |
	STA $03				; |
	JSL UploadSPCDataDynamic	; /
	SEP #$20
	;$2141 is still outputting a $BB, courtesy of acknowledging that it
	;is ready for a data transfer. This is zeroed out upon the
	;completion of the transfer, so wait for that to happen first.
-:
	LDA $2141
	BNE -
	JMP SkipSPCNormal
	
					; Time to get the SPC out of its loop.
					
	REP #$20			; \ 108173
	STZ $09				; | Size = 0 (jump to location instead of upload data).
	LDA #!DefARAMRet		; | 
	STA $0B				; | Location = #!DefARAMRet
	LDA #$0009|!SA1Addr1		; | 
	STA $00				; | 
	SEP #$30			; | 
if !UsingSA1				; |
	LDA #$7E			; | 
else					; |
	LDA #$00			; |
endif					; |
	STA $02				; | 
	JSL UploadSPCData		; /

	;$2141 is still outputting a $BB, courtesy of acknowledging that it
	;is ready for a data transfer. This is zeroed out upon the
	;completion of the transfer, so wait for that to happen first.
-:
	LDA $2141
	BNE -

SPCNormal:				
	SEP #$30
	
	LDA #$00
	STA !NoUploadSamples

SkipSPCNormal:
	LDA !GlobalMusicCount+1
	
;NoUpload:
	STA !MusicReg
	
-:
	LDA.w $4212
	BPL -
-:
	LDA.w $4212
	BMI -
	LDA #$81
	STA $4200
	JMP End
	
HandleSpecialSongs:
	LDA $0100|!SA1Addr2
	CMP #$0F
	BEQ +
	LDA !MusicMir
	CMP #!Miss
	BEQ +
	CMP #!GameOver
	BEQ +
if !PSwitch != $00 || !Starman != $00
	CMP #!StageClear	;;; more checks here should help
	BEQ ++
	CMP #!IrisOut
	BEQ ++
	CMP #!BossClear
	BEQ ++
	CMP #!Keyhole
	BEQ ++
endif
if !PSwitch != $00
	LDA $14AD|!SA1Addr2
	ORA $14AE|!SA1Addr2
	ORA $190C|!SA1Addr2
	BNE .powMusic
endif
if !Starman != $00
	LDA $1490|!SA1Addr2
	CMP #$1E
	BEQ .restoreFromStarMusic
	BCS .starMusic
endif
++
	RTS
	
+
	STZ $14AD|!SA1Addr2
	STZ $14AE|!SA1Addr2
	STZ $190C|!SA1Addr2
	STZ $1490|!SA1Addr2
	RTS
	
if !PSwitch != $00
.powMusic
	lda $1493|!SA1Addr2		;\ KevinM's edit: don't set the song at level end (goal/sphere/boss)
	ora $1434|!SA1Addr2		;| (keyhole)
	ora $14AB|!SA1Addr2		;| (bonus game)
	bne ++					;/ This prevents an issue with non-standard goal songs.

if !PSwitchStarRestart == !true
	jsr SkipPowStar
	bcs ++
if !Starman != $00
	lda $1490|!SA1Addr2		; If both P-switch and starman music should be playing
	cmp #$1E
	bcs .starMusic			;;; just play the star music
endif
if !PSwitchIsSFX == !false
	lda !MusicMir
	cmp #!PSwitch
	beq ++
	lda !CurrentSong
	cmp #!PSwitch
	bne +
endif

	stz !MusicMir
	rts

if !PSwitchIsSFX == !false
if !PSwitch != $00
+	LDA #!PSwitch
	STA !MusicMir
endif
++	RTS
endif

else
if !Starman != $00
	LDA $1490|!SA1Addr2		; If both P-switch and starman music should be playing
	CMP #$1E
	BCS .starMusic			;;; just play the star music
endif
endif

if !PSwitchIsSFX == !false && !PSwitchStarRestart == !false && !PSwitch != $00
	LDA #!PSwitch
	STA !MusicMir
endif
++
	RTS
endif
	
.starMusic
if !Starman != $00
if !PSwitchStarRestart == !true
	jsr SkipPowStar
	bcs ++
	lda !MusicMir
	cmp #!Starman
	beq ++
	lda !CurrentSong
	cmp #!Starman
	bne +
	stz !MusicMir
	rts
endif

+	LDA #!Starman
	STA !MusicMir
++	RTS

	
.restoreFromStarMusic
	LDA !MusicBackup
	STA !MusicMir
+
	RTS
endif

if !PSwitchStarRestart == !true
SkipPowStar:
	lda !CurrentSong
	cmp #!StageClear
	beq +
	cmp #!IrisOut
	beq +
	cmp #!BossClear
	beq +
	cmp #!Keyhole
	beq +
	clc
+	rts
endif
	
pushpc

incsrc "SongSampleList.asm"

MusicPtrs:
SamplePtrs:
SampleLoopPtrs:
UploadSPCEngine:
UploadSPCData:
UploadSPCDataDynamic:

pullpc