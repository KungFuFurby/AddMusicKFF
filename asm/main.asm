
; Super Mario World's modified SPC Program - Kipernal
; Heavily based on loveemu labo's assembly, which in turn was heavily based on C.Bongo's assembly.
; Bugs have been fixed and all addresses above $04FF have been turned into labels to make things portable.
;
; Easy way to tell whose comments are whose: My comments are indented with tabs, while loveemu labo's
; comments are indented with spaces.
;
; Notable code changes have been marked.  Or rather, they were.  Eventually the changes just got so
; massive that this stopped becoming feasible (though some marks still exist).
;
; Note: The L_XXXX labels are generic and almost none of them accurately reflect their placement in ARAM
; anymore.  Do not use them to locate a code's position.
;

; Major, major thanks to loveemu labo.  I could never have done this without this disassembly or without
; their documentation on the N-SPC engine.
!false = 0
!true = 1

incsrc "UserDefines.asm"


; Some documented RAM addresses: (note that addresses with "+x" are indexed by the current channel * 2)
; $00: Current input from APU0 (only available for 1 "loop").
; $01: Current input from APU1.
; $02: Current input from APU2.
; $03: Current input from APU3.
; $04: Value currently being processed from APU0 (available until whatever it is is done processing).
; $05: Value currently being processed from APU1.
; $06: Value currently being processed from APU2.
; $07: Value currently being processed from APU3.
; $08: Previous value from APU0?
; $09: Previous value from APU1?
; $0a: Previous value from APU2?
; $0b: Previous value from APU3?

; $45: Increases each "tick"
; $0281+x: Pan
; $0241+x: Volume
; $02d1+x: Fine tuning
; $02b1+x: Current note value (not pitch).
; $02b0+x: Current fractional note value, final calculation of portamento
; $81+x: Pan fade tick counter
; $80+x: Volume fade tick counter
; $a1+x: Vibrato maximum offset
; $b1+x: Tremolo maximum offset
; $c0+x: Repeat counter
; $c1+x: Instrument
; $0161: Strong portamento/legato
; $58: Global volume
; $60: Echo volume fade
; $52: Tempo fade
; $43: Global transpose
; $57: Global volume (default #$C0)
; $51: Tempo (default #$36)
; $0130: Used as an instrument "backup" for the $E5 / $F3 commands.  It is $30 bytes long, not the normal 8 (6 bytes for each channel).
; $0140: Don't use (part of the instrument "backup")
; $0141: Don't use (part of the instrument "backup")
; $0150: Don't use (part of the instrument "backup")
; $0151: Don't use (part of the instrument "backup")
; $0160: #$01 to enable Yoshi Drums (now handled elsewhere). Has various purposes; originally used by AddmusicM.
; $0211+x: The volume part of the qXX command.
; $0387: Amount the tempo should be increased by (used by the "time is running out!" sound effect to speed up the music).

; $48: Bitwise indicator of the current channel being processed.
; $5C: Used to indicate that a volume needs to be updated (long routine, so it's only done when necessary).
; $0166: 4 bytes; used as the output byte to send to the 5A22.  Originally AMM only used 2 of these, we can use all 4 for whatever we like.
; $13: Only the highest bit was ever modified or read. It forced the pitch or volume to be updated when a pitch slide, tremolo or vibrato was either being delayed or was not active.

; $5E: Used to mute a channel (via Yoshi Drums, etc.).  One bit per channel, setting it stops a channel from playing.
; $6E: Which channels are affected by Yoshi drums.

!SurroundSound = $02a1
!PanFadeDuration = $81
!PanFadeDestination = $02a0
!VolumeMult = $02e1

!MasterVolume = $57

!Pan = $0281			; 
!Volume = $0241			; 
!BackupSRCN = $0220		; 
!HTuneValues = $0120		; 
!CustomInstrumentPos = $6c	; Position of the custom instrument table.

!MusicPModChannels = $4a	; \ The music channels that have pitch modulation enabled on them.
!MusicNoiseChannels = $4b	; | The same as the above, but for noise.
!MusicEchoChannels = $4c	; / The same as the above, but for echo.

!SFXPModChannels = $4d		; \ The SFX channels that have pitch modulation enabled on them.
!SFXNoiseChannels = $4e		; | (etc.)
!SFXEchoChannels = $4f		; / (etc.)
				; All of these addresses must be sequential, in this order, and in the direct page.
				; ...so basically, don't change them.

!EchoDelay = $5d		; The value for the echo delay.
!MaxEchoDelay = $038d		; The highest echo delay reached for the current song (cleared on SPC upload).
!NCKValue = $5f			; The value for the noise clock.

!PauseMusic = $0388		; Pauses the music if not zero.  Used in the pause SFX.

!ChSFXPtrs = $20		; Two bytes per channel, so $20 - $2f.
!ChSFXNoteTimer = $01d0
!ChSFXPriority = $01d1
;!ChSFXTimeToStart = $01d1		; Time until the SFX on this channel starts.
!ChSFXNoteTimerBackup = $03d1	; Used to save space when two consecutive notes use the same length.
!ChSFXPtrBackup = $03c0		; A copy of $20w, only updated when a sound effect starts.  Used by the #$FE command to restart a sound effect.

!ArpLength = $0310		; The length between each change in pitch, measured in ticks, for the arpeggio command.
!ArpNotePtrs = $03b0		; The pointer for the sequence of pitch changes for the current arpeggio.
!ArpTimeLeft = $0311		; How much time until we get the next note.
!ArpNoteIndex = $0230		; What note number we're on.
!ArpNoteCount = $0261		; How many notes there are.
!ArpCurrentDelta = $0270	; The current pitch change for the current channel, in semitones.
!ArpType = $0231		; The arpeggio type.  0 = arpeggio, 1 = trill, 2 = glissando.
!ArpSpecial = $02e0		; The index of the loop point for this arpeggio, or the pitch difference for trill and glissando.

!PreviousNote = $0271		; The most recently played note for this channel.

!WaitTime = $6b			

!SecondVTable = $6f		; Set to 1 if we're using the N-SPC velocity table.

!PlayingVoices = $0382		; Each bit is set to 1 if there's currently a voice playing on that channel (either by music or SFX).
!SpeedUpBackUp = $0384		; Used to restore the speed after pausing the music.

!reserveBufferZeroEDLGateDistance = SubC_table2_reserveBuffer_zeroEDL-SubC_table2_reserveBuffer_zeroEDLGate-2

!ProtectSFX6 = $038a		; If set, sound effects cannot start on channel #6 (but they can keep playing if they've already started)
!ProtectSFX7 = $038b		; If set, sound effects cannot start on channel #7 (but they can keep playing if they've already started)

!remoteCodeTargetAddr = $0390	; The address to jump to for remote code.  16-bit and this IS a table.
!remoteCodeType = $03a0		; The remote code type.
!remoteCodeTimeLeft = $03a1	; The amount of time left until we run remote code if the type is 1 or 2.
!remoteCodeTimeValue = $01a0	; The value to set the timer to when necessary.
!remoteCodeTargetAddr2 = $0190	; The address to jump to for "start of note" code.  16-bit.
!InRest = $01a1

arch spc700-raw
org $000000
base $0400			; Do not change this.

if !noSFX = !true
print "NoSFX is enabled"
endif
{		; Program setup
	clrp
	mov   x, #$cf
	mov   sp, x              ; set SP to 01cf
	mov   a, #$00
	mov	y, a
	
-	mov	$0100+y, a
	mov	$0200+y, a
	mov	$0300+y, a
	dbnz	y, -
	
	movw	$00, ya
	

	
	
	mov   y, #$0c
L_0529:
	mov   a, DefDSPRegs-1+y
	mov   $f2, a
	mov   a, DefDSPValues-1+y
	mov   $f3, a               ; write A to DSP reg Y
	dbnz  y, L_0529            ; set initial DSP reg values
	
	mov   $f1, #$f0		; Reset ports, disable timers
	mov   $fa, #$10		; Set Timer 0's frequency to 2 ms
	mov   $51, #$36		; Set the tempo to #$36
	mov   $f1, #$01		; Reset and start timer 0
}			
; main loop

{ ; The main program loop.
MainLoop:
	print "MainLoopPos: $",pc
	mov   y, $fd
	beq   MainLoop             ; wait for counter 0 increment
	push  y
	mov   a, #$38
	mul   ya
	clrc
	adc   a, $44
	mov   $44, a
	bcc   L_0573
	inc   $45
if !noSFX = !false
	call	ProcessSFX
endif
	call  ProcessAPU1Input			; APU1 has to come first since it receives the "pause" sound effects that it pseudo-sends to APU0.
	call  ProcessAPU0Input
if !noSFX = !false
	call  ProcessAPU3Input
endif
	mov   x, #$00
	call  ReadInputRegister             ; read/send APU0
	mov   x, #$01
	call  ReadInputRegister             ; read/send APU1
	mov   x, #$03
	call  ReadInputRegister             ; read/send APU3
if !noSFX = !false	
	mov	a, !ProtectSFX6
	beq	+
	mov	$00, #$00
+
	mov	a, !ProtectSFX7
	beq	+
	mov	$03, #$00
+
endif
L_0573:
	mov   a, $51
	pop   y
	mul   ya
	clrc
	adc   a, $49
	mov   $49, a
;Slowdown partial bugfix by KungFuFurby 6/24/20
;This fix doubles the tolerance the sound driver is able to handle before
;ticks are lost.
;Additional possible fixes:
;- Send Y to $0D (a free byte) OR...
;-- Swap $49 and $44's purpose, and repurpose $45 for utilization with the
;   addw opcode instead of being a just a tick counter
;- Read $FD more often (only needed if we overflow from 15 to 0) and
;  reserve a memory location for this purpose.
	bcs   SoundTickOn
	cmp   y, #$00
	beq   L_058D
	
SoundTickOn:
	mov   a, !PauseMusic
	bne   L_0586
	call  ProcessAPU2Input		; Also handles playing the current music.
L_0586:
	mov   x, #$02
	call  ReadInputRegister             ; read/send APU2
	
	setp				; MODIFIED CODE
	movw  ya, $0166&$FF			;
	clrp				; Send the output values two at a time.
	movw  $f4, ya			;
	setp				;
	movw  ya, $0168&$FF		;	
	clrp				;
	movw  $f6, ya			;
	
	bra   MainLoop             ; restart main loop
L_058D:
	mov   a, $06             ; if writing 0 to APU2 then
	beq   MainLoop             ;   restart main loop
	
{ ; Execute code for each channel.

	
	
	mov   x, #$0e            ; foreach voice
	mov   $48, #$80
L_0596:
	mov   a, $31+x
	beq   L_059D             ; skip call if vptr == 0
	call  HandleVoice             ; do per-voice fades/dsps?
L_059D:
	lsr   $48
	dec   x
	dec   x
	bpl   L_0596             ; loop for each voice
	bra   MainLoop             ; restart main loop

}
	
}	
; send 04+X to APUX; get APUX to 00+X with "debounce"?
;L_05A5:
ReadInputRegister:
{
L_05AC:
	mov   a, $f4+x		; \ Get the input byte
	cbne  $f4+x, L_05AC	; / Keep getting it until it's "stable"
	mov   y, a		; \ 
	mov   a, $08+x		; |
	mov   $08+x, y		; |
	cbne  $08+x, L_05C1	; |
	mov   y, #$00		; |
L_05C1:				; |
	mov   $00+x, y		; |
	ret			; / 
}	

macro OpenRunningRemoteCodeGate()
	mov	a, #$f4				;mov a, d+x opcode
	mov	runningRemoteCodeGate, a
endmacro

RunRemoteCode:
{
	mov	a, $30+x
	push	a
	mov	a, $31+x
	push	a
	mov	a, !remoteCodeTargetAddr+x
	mov	$30+x, a
	mov	a, !remoteCodeTargetAddr+1+x
	mov	$31+x, a
RunRemoteCode_Exec:
	mov	a, #$6f			;RET opcode
	mov	runningRemoteCodeGate, a
	call	L_0C57			; This feels evil.  Oh well.  At any rate, this'll run the code we give it.
	%OpenRunningRemoteCodeGate()
	pop	a
	mov	$31+x, a
	pop	a
	mov	$30+x, a
	ret
}

RunRemoteCode2:
{
	mov	a, $30+x
	push	a
	mov	a, $31+x
	push	a
	mov	a, !remoteCodeTargetAddr2+x
	mov	$30+x, a
	mov	a, !remoteCodeTargetAddr2+1+x
	mov	$31+x, a
	bra	RunRemoteCode_Exec
}
	
; handle a note vcmd
NoteVCMD:			
{	
				; X should contain the current channel * 2.
				; A should contain the note (can be percussion or a normal pitch note).
	cmp	a, #$d0
	bcs	PercNote             ; percussion note
	cmp	a,#$C6			;;;;;;;;;;;;Code change
	bcc	NormalNote
	beq	L_05CD

if_rest:
	mov	a, #$01
	mov	!InRest+x, a
if !noSFX = !false
	call	TerminateIfSFXPlaying
endif
	mov	a, !remoteCodeType+x
	cmp	a, #$03
	beq	L_05CD
	mov	a, $48
	call	KeyOffVoices
	tclr	$0162, a
L_05CD:
	ret
;UseGainInstead:
;	push	a			; 
;	mov	a, x			; \
;	lsr	a			; | GAIN Register into y
;	xcn	a			; |
;	or	a, #$07			; |
;	mov	y, a			; /
;	pop	a			; 
;	call	DSPWrite		; Write
;	dec	y			; \
;	dec	y			; | Clear ADSR bit to force GAIN.
;	mov	a, #$7f			; |
;	jmp	DSPWrite		; /
	
PercNote:
	
	mov	$c1+x, a
	setc
	sbc	a, #$d0
	mov	y, #$07
	mov	$10, #PercussionTable
	mov	$11, #PercussionTable>>8
	call	ApplyInstrument             ; set sample A-$D0 in bank $5FA5 width 6
NormalNote:						;;;;;;;;;;/ Code change
	
	and	a, #$7f		; Right now the note is somewhere between #$80 and #$C6 or so.  Get rid of the MSB to bring it down to #$00 - #$46
	push	a			; MODIFIED CODE
	
	mov	a, #$00
	mov	!InRest+x, a
if !noSFX = !false	
	mov	a, $48		; If $48 is 0, then this is SFX code.
	beq	NoPitchAdjust	; Don't adjust the pitch.
endif
				; That says no pitch adjust, but we do more stuff here related to the "no sound effects allowed" club.

	mov	a, !remoteCodeType+x
	dec	a
	bne	.notType1RemoteCode
	
	mov	a, !remoteCodeTimeValue+x
	mov	!remoteCodeTimeLeft+x, a
	
.notType1RemoteCode
	
	mov	a, !remoteCodeTargetAddr2+1+x
	beq	.noRemoteCode			

	call	RunRemoteCode2
	
.noRemoteCode
	
	
	mov	a, $02d1+x	; \ 
	mov	$02b0+x, a	; / Tuning into $02b0+x	
	
	
	pop	a
	clrc				; \ Add the global transpose
	adc	a, $43 			; /
	clrc				; \
	adc	a, !HTuneValues+x		; / Add the h tune...
	clrc				; \
	adc	a, !ArpCurrentDelta+x	; / Add the arpeggio delta...
	
	bra +
NoPitchAdjust:
	mov	a, #$00
	mov	$02b0+x, a
	pop	a
+
	mov	$02b1+x, a	; $02b1 gets the note to play.
	mov	a, #$00		; \ 
	mov	$0330+x, a	; | 
	mov	$0360+x, a	; | Reset vibrato and tremolo counters.
	mov	$a0+x, a	; |
	mov	$0110+x, a	; |
	mov	$b0+x, a	; /
	or	($5c), ($48)       ; set volume changed flg
	or	($47), ($48)       ; set key on shadow vbit
if !noSFX = !false
	mov	a, $48		; If $48 is 0, then this is SFX code.
	beq	L_062B		; Don't adjust the pitch.	
endif
	mov	a, $0300+x	; Get pitch envelope length.
	mov	$90+x, a	; If zero, then we are not processing a pitch envelope.
	beq	L_062B
	mov	a, $0301+x	; Get pitch envelope delay.
	mov	$91+x, a	; 
	mov	a, $0320+x	; Check the pitch envelope sign.
	bne	L_0621		;
	mov	a, $02b1+x	;
	setc			;
	sbc	a, $0321+x	;
	mov	$02b1+x, a	;
L_0621:				;
	mov	a, $0321+x	;
	clrc			;
	adc	a, $02b1+x	;
	call	CalcPortamentoDelta
L_062B:
	call	DDEEFix	
; set DSP pitch from $10/11
SetPitch:			;
if !noSFX = !false
	call	TerminateIfSFXPlaying
endif
	push	x
	mov	a, $11
	asl	a
	mov	y, #$00
	mov	x, #$18
	div	ya, x
	mov	x, a
	mov	a, PitchTable+1+y
	mov	$15, a
	mov	a, PitchTable+0+y
	mov	$14, a             ; set $14/5 from pitch table
	;mov	a, PitchTable+3+y
	;push	a
	mov	a, PitchTable+2+y
	setc
	sbc	a, $14
	;pop	y
	;subw	ya, $14
	mov	y, $10
	mul	ya
	mov	a, y
	mov	y, #$00
	addw	ya, $14
	mov	$15, y
	asl	a
	rol	$15
	mov	$14, a
	bra	+
-	lsr	$15
	ror	a
	inc	x
+	cmp	x, #$06
	bne	-
	mov	$14, a
	pop	x
	mov	a, $02f0+x
	push	a
	mov	y, $15
	mul	ya
	movw	$16, ya
	pop	a
	mov	y, $14
	mul	ya
	push	y
	mov	a, $0210+x
	push	a
	mov	y, $14
	mul	ya
	addw	ya, $16
	movw	$16, ya
	pop	a
	mov	y, $15
	mul	ya
	mov	y, a
	pop	a
	addw	ya, $16
	movw	$16, ya
	mov	a, x               ; set voice X pitch DSP reg from $16/7
	xcn	a                 ;  (if vbit clear in $1d)
	lsr	a
	or	a, #$02            ; A = voice X pitch DSP reg
	mov	y, $16
	movw	$f2, ya
	inc	a
	mov	y, $17
	movw	$f2, ya
	ret
	
}

L_09CDWPreCheck:
	mov	a, $91+x		; If pitch slide delta is being delayed...
	beq	L_1119
	dec	$91+x
	bra	L_112A
L_1119:
if !noSFX = !false
	mov	a, $1d			; \ Check to see if this channel is muted (by a sound effect or whatever)
	and	a, $48			; |
	bne	L_112A			; /
endif
	set1	$13.7			;

; add pitch slide delta and set DSP pitch
L_09CD:
	mov	a, #$02b0&$FF
	mov	y, #$02b0>>8       ; pitch (notenum fixed-point)
	dec	$90+x
	;Modifies $02b0-$02b1, $02c0-$02c1, $02d0
	call	L_1075             ; add pitch slide delta to value                                ;ERROR
L_112A:
DDEEFix:
{
	mov	a, $02b1+x
	mov	y, a
	mov	a, $90+x
	beq	+
-
	mov	a, $02b0+x
	bra	++
+
if !noSFX = !false
	mov	a, $48		; If $48 is 0, then this is SFX code.
	beq	-		; Don't adjust the pitch.
	and	a, $1d
	bne	-
endif
	mov	a, $02d1+x	; Make sure that the correct fine tune value is used for the music.
	mov	$02b0+x, a
++
	movw	$10, ya            ; notenum to $10/11
	ret
}


EffectModifier:				; Call this whenever either $1d or the various echo, noise, or pitch modulation addresses are modified.
{	
	push	x
	push	y
	mov	x, #$00
	mov	y, #$d1			; The DSP register for pitch modulation reversed.
					; $10 = the current music whatever
					; $12 = the current SFX whatever
-						
					; Formula: The output for the DSP register is
					; S'M + SE
					; Where 
					; M is !WhateverMusicChannels,
					; E is !WhateverSFXChannels.
					; and S is $1d (the current channels for which SFX are enabled)
					; Yay logic!

	inc	y			; \
	mov	a, y			; | Get the next DSP register into a.
	xcn	a			; /
	mov	$f2, a			;
						
if !noSFX = !false
	mov	a, $1d			; \ a = S
	eor	a, #$ff			; | a = S'
	and	a, !MusicPModChannels+x	; / a = S'M
	
	mov	$15, a

	mov	a, !SFXPModChannels+x	; \ a = S
	and	a, $1d			; | a = SE
	or	a, $15			; / a = S'M + SE
else
	mov	a, !MusicPModChannels+x
endif
	
					; \ Write to the relevant DSP register.
	mov	$f3, a			; / (coincidentally, the order is the opposite of DSPWrite)
	
	inc	x			; \
	cmp	x, #$03			; | Do this three times.
	bne	-			; /

	pop	y
	pop	x
	ret
}



}
if !noSFX = !false
macro RestoreVolLevelsPostNoise()
	mov	a, $d0+x
	mov	$f3, a
	inc	$f2
	mov	a, $d1+x
	mov	$f3, a
	mov	a, $13
	tclr	$1a, a
endmacro

macro NoiseBackupThenZeroVolLevels()
	mov	a, $f3
	mov	$d0+x, a
	mov	$f3, #$00
	inc	$f2
	mov	a, $f3
	mov	$d1+x, a
	mov	$f3, #$00
	or	($1a), ($13)
endmacro

ProcessSFX:				; Major code changes ahead.
{					; Originally, the SMW SFX were handled within their port handling routines.
					; This meant that there was a near duplicate copy of the 1DF9 code for 1DFC SFX.
					; It also meant that SFX were limited to just #4 and #6 (#7 was a special case).
					; This fixes that.
	
	mov	$48, #$00		; Let NoteVCMD know that this is SFX code.
	
	mov	x, #$0e			; For each voice (x = current channel * 2)
	mov	$18, #$80		; $18 = bitwise indicator of current channel
.loop					;
	mov	a, !ChSFXPtrs+1+x	; If the high byte is zero, then there's no SFX here.  Skip it.
	beq	.nothing		;
	mov	$46, x			; $46 gets the channel currently being processed.
	call	HandleSFXVoice		;
.nothing				;
	lsr	$18			;
	dec	x			;
	dec	x			;
	bpl	.loop			;
	jmp	EffectModifier		;
}

GetNextSFXByte:
{
	mov	x, $46			; Ensure x contains the channel number * 2.
	mov	a, (!ChSFXPtrs+x)	; Get the byte.
	push	p			; Remember the flags (negative and zero).
	inc	!ChSFXPtrs+x		; \ 
	bne	+			; | Increase the correct SFX pointer.
	inc	!ChSFXPtrs+1+x		; /
+					;
	pop	p			; Get the flags back.  inc modifies them.
	ret				;
}


	
	

EndSFX:
{
	; Original code here would have cleared out $04, etc.
	; We can't do that, though, since it's impossible to know what input port was responsible for this sound effect.
	; Not that it's much of an issue...just useful for sanity checks and such, really.
	
	mov	a, #$00			; \ Zero out the high byte of this SFX
	mov	!ChSFXPtrs+1+x, a	; / This will ensure that it's no longer processed.
					; Old code simply set $04+inputport to 0 to do this.
					; Of course, that doesn't work so well when some channels don't map to input ports...
				
	
	mov	a, $18
if !useSFXSequenceFor1DFASFX = !false
	bbc!1DFASFXChannel	$18, +
	cmp	$1c, #$00
	bne	++
endif
+					; \
	tclr	$1d, a			; | Clear the bit of $1d that this SFX corresponds to.
++
	tclr	!SFXNoiseChannels, a	; / Turn noise off for this channel's SFX.

	call	EffectModifier
if !noiseFrequencySFXInstanceResolution = !true
	cmp	!SFXNoiseChannels, #$00
	beq	.restoreMusicNoise
	call	SetSFXNoise
	call	ModifyNoise
	bra	RestoreInstrumentInformation
endif
.restoreMusicNoise:
	mov	a, #$00
	mov	$1f, a
if !useSFXSequenceFor1DFASFX = !false
	bbc!1DFASFXChannel	$18, +
	mov	a, $1c
	bne	++
endif
+
	mov	!ChSFXPriority+x, a
++
	mov	a, $0389
	call	ModifyNoise
	mov	a, !MusicNoiseChannels
	and	a, $1a
	beq	RestoreInstrumentInformation
	mov	$12, a
	mov	a, !SFXNoiseChannels
	tclr	$12, a
	mov	$13, #$01
	;Restore VxVOL DSP registers of all music noise channels.
	mov	x, #$00
	mov	$f2, x
-
	lsr	$12
	push	p
	bcc	++
if !noiseFrequencySFXInstanceResolution = !true
	call	RestoreVolLevelsPostNoise
else
	%RestoreVolLevelsPostNoise()
endif
++
	call	NoiseSetVolLevelsNextCh
	pop	p
	bne	-

	mov	x, $46			; \ 
endif
RestoreInstrumentInformation:		; Call this with x = currentchannel*2 to restore all instrument properties for that channel.
	
	mov	a, !BackupSRCN+x	; |
	bne	.restoreSample		; |
	mov	a, $c1+x		; | Fix instrument.
	beq	+			; |
	dec	a			; |
	jmp	SetInstrument		; |
+					; /
	ret				;
					;
.restoreSample				; \ 
	jmp   RestoreMusicSample	; / Fix sample.
}
if !noSFX = !false
HandleSFXVoice:
{
	setp
	dec	!ChSFXNoteTimer&$FF+x
	clrp
	beq	+	
	jmp	.processSFXPitch
+
.getMoreSFXData
	call	GetNextSFXByte
	bne	+			; If the current byte is zero, then end it.
	jmp	EndSFX
+
	bmi	.noteOrCommand		; If it's negative, then it's a command or note.
	mov	!ChSFXNoteTimerBackup+x, a			
					; The current byte is the duration.
	
	call	GetNextSFXByte		; Get the next byte.  It's either a volume or a command/note.
	bmi	.noteOrCommand		; If it's negative, then it's a command or a note.
	push	a			; \ This is a volume command.  Remember it for a moment.
	mov	a, x			; | 
	lsr	a			; |
	xcn	a			; | Put the left volume DSP register for this channel into y.
if !noiseFrequencySFXInstanceResolution = !true
	mov	y, a			; |
	pop	a			; |
	call	.setVolFromNoiseSetting ; |
	call	DSPWrite		; / Set the volume for the left speaker.
	inc	y			; \
	call	DSPWrite		; / Set the volume for the right speaker.  We might change it later, but this saves space.
else
	pop	y
	movw	$f2, ya			; Set the volume for the left speaker.
	inc	a
	movw	$f2, ya			; Set the volume for the right speaker.  We might change it later, but this saves space.
endif
	call	GetNextSFXByte		;
	bmi	.noteOrCommand		; If the byte is positive, then set the right volume to the byte we just got.
if !noiseFrequencySFXInstanceResolution = !true
	call	.setVolFromNoiseSetting ;
endif
	mov	$f3, a			; > Set the volume for the right speaker.
	call	GetNextSFXByte		;
	bra	.noteOrCommand		; At this point, we must have gotten a command/note.  Assume that it is, even if it's not.
	
.executeCode				; 
	call	GetNextSFXByte		; \ 
	mov	$14, a			; | Get the address of the code to execute and put it into $14w
	call	GetNextSFXByte		; |
	mov 	$15, a			; / 
	push	x			; \ 
	mov	x, #$00			; | Jump to that address
	call	+			; | (no "call (d+x)")
	pop	x			; / 
	bra	.getMoreSFXData		;
+					;
	jmp	($14+x)			;

.noteOrCommand				; SFX commands!
	cmp	a, #$dd			; \ 
	beq	.pitchBendCommand	; / $DD is the pitch bend command.

	cmp	a, #$eb			; \
	beq	.pitchBendCommand2	; / $EB is...another pitch bend command.

	cmp	a, #$fd			; \ 
	beq	.executeCode		; / $FD is the code execution command.
	cmp	a, #$da			; \ 
	beq	.instrumentCommand	; / $DA is the instrument command.
	cmp	a, #$fe			; \
	beq	.loopSFX		; / $FE is the restart SFX command.
	bcc	.playNote		; / Play a note.
	mov	a, !ChSFXPtrs+x		; \ Move back one byte.
	bne	+			; |
	dec	!ChSFXPtrs+1+x		; |
+					; |
	dec	!ChSFXPtrs+x		; |
	mov	a, $18			; | #$FF is the loop the last note command.
	bra	.keyOnNote		; /
; other $80+
.loopSFX
	mov	a, !ChSFXPtrBackup+1+x	; \
	mov	!ChSFXPtrs+1+x, a	; | Set the current pointer to the backup pointer,
	mov	a, !ChSFXPtrBackup+x	; | Thus restarting this sound effect.
	mov	!ChSFXPtrs+x, a		; /
	bra	.getMoreSFXData
	
.playNote
	call	NoteVCMD		; Loooooooong routine that starts playing the note in A on channel (X/2).
	mov	a, $18
	push	a
	mov	a, !InRest+x
	pop	a
	beq	.keyOnNote
	call	KeyOffVoices
	bra	.setNoteLength
.keyOnNote
	call	KeyOnVoices		; Key on the voice.
.setNoteLength
	mov	a, !ChSFXNoteTimerBackup+x	
					; \ Get the length of the note back
	mov     !ChSFXNoteTimer+x, a	; / And since it was actually a length, store it.
.processSFXPitch
	mov	a, $91+x		; If pitch slide is not being delayed...
	beq	+
	dec	$91+x
	ret
+
	mov	a, $90+x		; pitch slide counter
	beq	+
	call	L_09CD			; add pitch slide delta and set DSP pitch
	jmp	SetPitch                ; force voice DSP pitch from 02B0/1
+
	mov	a, #$02			; \
	cmp	a, !ChSFXNoteTimer+x	; |
	bne	.return1		; | If the time between notes is 2 ticks
	mov	a, $18			; | Then key off this channel in preparation for the next note.
	;mov	y, #$5c			; | This doesn't happen during pitch bends.
	;call	DSPWrite		; /
	call	KeyOffVoices
.return1
	ret
; DD
.pitchBendCommand			; This command is all sorts of weird.
	call	GetNextSFXByte		; The pitch of the note is this byte.
	call	NoteVCMD		; 
	mov	a, $18			; \
	call	KeyOnVoices		; /
; EB
.pitchBendCommand2
	call	GetNextSFXByte		;
	mov	$91+x, a		; Set pitch bend length. (This wasn't functional in vanilla SMW.)
	call	GetNextSFXByte		;
	mov	$90+x, a		; Set number of ticks to bend pitch for.
	push	a			;
	call	GetNextSFXByte		;
	pop	y			;
	call	CalcPortamentoDelta	; \ Calculate the pitch difference.
	bra	.setNoteLength		; /

; DA
.instrumentCommand	
	mov	a, $18			; \ Disable noise for this channel.
	tclr	!SFXNoiseChannels, a	; / (EffectModifier is called a bit later)
if !noiseFrequencySFXInstanceResolution = !true
	tclr	$1a, a
endif

.getInstrumentByte
	call	GetNextSFXByte		; Get the parameter for the instrument command.
	bmi	.noise			; If it's negative, then it's a noise command.
	call	SetSFXInstrument	; No noise here!
	jmp	.getMoreSFXData		; We're done here; get the next SFX command.
.noise	
	and	a, #$1f			; \ Noise can only be from #$00 - #$1F	
	or	(!SFXNoiseChannels), ($18)
if !noiseFrequencySFXInstanceResolution = !true
	mov	$01f1+x, a
	cmp	!SFXNoiseChannels, $18
	beq	.noiseNoPrevSFXFrequency
	call	SetSFXNoise
	bra	.noiseCheckMusic

.noiseNoPrevSFXFrequency
	mov	$1f, #$00
if !noiseFrequencyMatchChecks = !true
	mov	$1e, a
else
	;Store the channel number instead.
	mov	$1e, x
endif
endif
	;All music channels with noise need to have their VxVOL values
	;zeroed out here if the frequency is not a match.
.noiseCheckMusic
if !noiseFrequencyMatchChecks = !true
	cmp	a, $0389
	beq	.noiseSetFreq
endif
	cmp	!MusicNoiseChannels, #$00
	beq	.noiseSetFreq
	push	x
	push	a
	mov	$12, !MusicNoiseChannels
	mov	a, $1a
	or	a, !SFXNoiseChannels
	or	a, $1d
	tclr	$12, a
	mov	$13, #$01
	;Zero out VxVOL DSP registers of all music noise channels.
	mov	x, #$00
	mov	$f2, x
-
	lsr	$12
	push	p
	bcc	+
if !noiseFrequencySFXInstanceResolution = !true
	call	NoiseBackupThenZeroVolLevels
else
	%NoiseBackupThenZeroVolLevels()
endif
+
	call	NoiseSetVolLevelsNextCh
	pop	p
	bne	-

	pop	a
	pop	x

.noiseSetFreq
	call	ModifyNoise
	bra	.getInstrumentByte	; Now we...go back until we find an actual instrument?  Odd way of doing it, but I guess that works.
if !noiseFrequencySFXInstanceResolution = !true
.setVolFromNoiseSetting
	mov	$11, a
	mov	a, $18
	and	a, !SFXNoiseChannels
	beq	.setNormalVol
if !noiseFrequencyMatchChecks = !true
	mov	a, $01f1+x
	cmp	a, $1e
	beq	.setNormalVol
else
	cmp	x, $1e
	beq	.setNormalVol
endif
	mov	a, !ChSFXPriority+x
	cmp	a, $1f
	bcc	.storeVolFromNoiseSetting
	bne	.setNormalVol
	cmp	x, #$0e
	beq	.setNormalVol
;Now we scan the priorities of all channels higher than this one.
	mov	$12, x
-
	inc	$12
	inc	$12
	push	x
	mov	a, !ChSFXPriority+x
	mov	x, $12
	cmp	a, !ChSFXPriority+x
	pop	x
	bcc	.storeVolFromNoiseSetting
	bne	.setNormalVol
	cmp	$12, #$0e
	bcc	-
.storeVolFromNoiseSetting
	mov	a, y
	lsr	a
	mov	a, $11
	bcc	+
	mov	$d1+x, a
	bra	.setZeroVolFromNoiseSetting
+
	mov	$d0+x, a
.setZeroVolFromNoiseSetting	
	mov	a, #$00
	ret
.setNormalVol
	mov	a, $11
	ret

SetSFXNoise:
	push	x
	;Decide which SFX channel's noise frequency will be used as the
	;comparison point, using priority as the factor. Ties favor highest
	;channel ID.
	mov	$12, !SFXNoiseChannels
	mov	x, #$00
	mov	$13, x
	setc
-
	asl	$13
	lsr	$12
	push	p
	bcc	+
	mov	a, !ChSFXPriority+x
	cmp	a, $1f
	bcc	+
	mov	$1f, a
	mov	a, $01f1+x
if !noiseFrequencyMatchChecks = !true
	mov	$1e, a
else
	mov	$11, a
	;We'll just store the winning channel number instead, since we only
	;have one channel to allocate for noise.
	;The frequency to use is stored in scratch RAM.
	mov	$1e, x
endif
+
	inc	x
	inc	x
	pop	p
	bne	-
	;OK, we have our winning frequency, now compare and set VxVOL values
	;based off of noise frequency.
	mov	$12, !SFXNoiseChannels
	mov	a, $13
	tclr	$12, a
if !noiseFrequencyMatchChecks = !true
	mov	a, $1e
else
	mov	a, $11
endif
	mov	x, #$00
	mov	$f2, x
	mov	$13, #$01
-
	lsr	$12
	push	p
	push	a
	bcc	++
if !noiseFrequencyMatchChecks = !true
	cmp	a, $01f1+x
else
	cmp	x, $1e
endif
	beq	+
	setc
	call	NoiseBackupThenZeroVolLevels
	bra	++
+
	clrc
	mov	a, $13
	and	a, $1a
	beq	++
	setc
	call	RestoreVolLevelsPostNoise
++
	call	NoiseSetVolLevelsNextCh
	pop	a
	pop	p
	bne	-

	pop	x
	ret

NoiseBackupThenZeroVolLevels:
	%NoiseBackupThenZeroVolLevels()
	ret

RestoreVolLevelsPostNoise:
	%RestoreVolLevelsPostNoise()
	ret

endif

NoiseSetVolLevelsNextCh:
	notc
	adc	$f2, #$0f
	inc	x
	inc	x
	asl	$13
	ret

SetSFXInstrument:
	mov	y, #$09			; \ 
	mul	ya			; | Set up the instrument table for SFX
	mov	x, a			; |
	mov	a, $46			; | \
	xcn	a			; | | Get the correct DSP register "base" into y.
	lsr	a			; | |
	mov	y, a			; | /
	mov	$12, #$08		; / 9 bytes of instrument data.
-					; \
	mov	a, SFXInstrumentTable+x	; |
	call	DSPWrite		; | Loop that sets various DSP registers.
	inc	x			; |
	inc	y			; |
	dbnz	$12, -    		; / 
	mov	a, SFXInstrumentTable+x ; \
	mov	x, $46			; | Set pitch base multiplier.
	mov	$0210+x, a		; /
	mov	a, #$00			; \ Disable sub-tuning
	mov	$02f0+x, a		; /
	ret
}

if !PSwitchIsSFX = !true

PSwitchPtrs:
	dw PSwitchCh0
	dw PSwitchCh1
	dw PSwitchCh2

PSwitchCh2:
	db $DA, $02			; @2
	db $FD				; #jsr PSwitchInitCh1And2
	dw PSwitchInitCh1And2
PSwitchCh2NoteLen3X1:
	db $30, $00,      $C6 		; r=24
PSwitchCh2NoteLen2X1:
	db $20, $00, $26, $A4		; y0o4c=16
PSwitchCh2NoteLen1:
	db $10, $0A, $1D, $9F		; y5o3g=8
PSwitchCh2NoteLen2:
	db $10, $00,      $C6		; r=8
PSwitchCh2NoteLen3:
	db $10, $13, $13, $AB		; y10o4g=8
PSwitchCh2NoteLen4:
	db $10, $17, $0F, $9F		; y12o3c=8
PSwitchCh2NoteLen2X2:
	db $20, $1D, $0A, $A4		; y15o4c=4
PSwitchCh2NoteLen5:
	db $10, $26, $00, $9F		; y20o3g=8
PSwitchCh2NoteLen3X2:
	db $30, $00,      $C6		; r=24
PSwitchCh2NoteLen2X3:
	db $20, $26, $00, $A5		; y20o4c+=16
PSwitchCh2NoteLen6:
	db $10, $1D, $0A, $A0		; y15o3g+=8
PSwitchCh2NoteLen7:
	db $10, $00,      $C6		; r=8
PSwitchCh2NoteLen8:
	db $10, $13, $13, $AC		; y10o4g+=8
PSwitchCh2NoteLen9:
	db $10, $0C, $18, $A0		; y7o3c+=8
PSwitchCh2NoteLen2X4:
	db $20, $1D, $0A, $A5		; y5o4c+=4
PSwitchCh2NoteLen10:
	db $10, $00, $26, $A0		; y0o3g+=8
	db $FD				; #jsr PSwitchNextLoopCh2
	dw PSwitchNextLoopCh2
	db $FE				; loop
	
PSwitchCh1:
	db $DA, $02	; @2
	db $FD				; #jsr PSwitchInitCh1And2
	dw PSwitchInitCh1And2
PSwitchCh1NoteLen2X1:
	db $20, $26, $00, $8C		; y0o2c=16
PSwitchCh1NoteLen4X1:	
	db $40,           $93		; y0o2g=8^24
PSwitchCh1NoteLen3X1:
	db $30,           $98		; y0o3c=24
	db                $93		; y0o2g=24
PSwitchCh1NoteLen2X2:
	db $20, $04, $22, $8D		; y2o2c+=16
PSwitchCh1NoteLen4X2:
	db $40, $0A, $1D, $94		; y5o2g+=8^24
PSwitchCh1NoteLen3X2:
	db $30, $13, $13, $99		; y10o3c+=24
PSwitchCh1NoteLen3X3:
	db $30, $1E, $08, $94		; y16o2g+=24
	db $FD				; #jsr PSwitchNextLoopCh1
	dw PSwitchNextLoopCh1
	db $FE
	
PSwitchCh0:
	db $DA, $09			; @9
	db $FD				; #jsr PSwitchInitCh0
	dw PSwitchInitCh0
PSwitchCh0NoteLen:
	db $10, $0D, $B0		; o4g=8
	db 	$B0			; o4g=8
	db	$B9			; o5e=8
	db	$B9			; o5e=8
	db $FD				; #jsr PSwitchNextLoopCh0
	dw PSwitchNextLoopCh0
	db $FE			

PSwitchInitCh1And2:
	;Set the ADSR of the current channel to match the real one, since
	;there is no SFX instrument that exactly replicates this parameter.
	mov	a, $46
	lsr	a
	xcn	a
	or	a, #$06
	mov	$f2, a
	mov	$f3, #$6a
	bra	PSwitchInit

PSwitchInitCh0:
	mov	a, #$00
	mov	PSwitchCh0LoopCounter+1, a
PSwitchInit:
	;Don't call this again for subsequent loops.
	mov	x, $46
	mov	a, !ChSFXPtrBackup+x
	clrc
	adc	a, #$05
	mov	!ChSFXPtrBackup+x, a
	mov	a, !ChSFXPtrBackup+1+x
	adc	a, #$00
	mov	!ChSFXPtrBackup+1+x, a
	;Init loop counter
	mov	a, #$00
	mov	PSwitchLoopCounter+1, a
	call	PSwitchSetNoteLengthCh0
	call	PSwitchSetNoteLengthCh1
	call	PSwitchSetNoteLengthCh2
	ret

PSwitchNextLoopCh2:
if !PSwitchSFXCh2ID < 7
	clrc
	mov	a, $1b
	and	a, #($ff<<(!PSwitchSFXCh2ID+1))&$ff
-
	asl	a
	bcs	PSwitchSetNoteLengthCh2
	bne	-
endif
	call	PSwitchIncLoopCounter

PSwitchSetNoteLengthCh2:
	mov	y, PSwitchLoopCounter+1
	mov	a, PSwitchNoteLengths+y
	push	y
	mov	y, #$0a
-
	push	a
	mov	a, PSwitchCh2NoteLenOffsets-1+y
	mov	x, a
	pop	a
	mov	PSwitchCh2NoteLen1+x, a
	dbnz	y, -
	pop	y

	asl	a
	mov	PSwitchCh2NoteLen2X1, a
	mov	PSwitchCh2NoteLen2X2, a
	mov	PSwitchCh2NoteLen2X3, a
	mov	PSwitchCh2NoteLen2X4, a
	clrc
	adc	a, PSwitchNoteLengths+y
	mov	PSwitchCh2NoteLen3X1, a
	mov	PSwitchCh2NoteLen3X2, a
	ret

PSwitchCh2NoteLenOffsets:
	db	PSwitchCh2NoteLen1-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen2-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen3-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen4-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen5-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen6-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen7-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen8-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen9-PSwitchCh2NoteLen1
	db	PSwitchCh2NoteLen10-PSwitchCh2NoteLen1

PSwitchNextLoopCh1:
if !PSwitchSFXCh1ID < 7
	clrc
	mov	a, $1b
	and	a, #($ff<<(!PSwitchSFXCh1ID+1))&$ff
-
	asl	a
	bcs	PSwitchSetNoteLengthCh1
	bne	-
endif
	call	PSwitchIncLoopCounter

PSwitchSetNoteLengthCh1:
	mov	y, PSwitchLoopCounter+1
	mov	a, PSwitchNoteLengths+y
	asl	a
	mov	PSwitchCh1NoteLen2X1, a
	mov	PSwitchCh1NoteLen2X2, a
	clrc
	adc	a, PSwitchNoteLengths+y
	mov	PSwitchCh1NoteLen3X1, a
	mov	PSwitchCh1NoteLen3X2, a
	mov	PSwitchCh1NoteLen3X3, a
	clrc
	adc	a, PSwitchNoteLengths+y
	mov	PSwitchCh1NoteLen4X1, a
	mov	PSwitchCh1NoteLen4X2, a
	ret

PSwitchNextLoopCh0:
PSwitchCh0LoopCounter:
	mov	a, #$00
	inc	a
	mov	PSwitchCh0LoopCounter+1, a
	cmp 	a, #$06
	bcc	PSwitchSetNoteLengthCh0
	mov	a, #$00
	mov	PSwitchCh0LoopCounter+1, a

PSwitchCh0IncMainLoopCounter:
if !PSwitchSFXCh0ID < 7
	clrc
	mov	a, $1b
	and	a, #($ff<<(!PSwitchSFXCh0ID+1))&$ff
-
	asl	a
	bcs	PSwitchSetNoteLengthCh0
	bne	-
endif
	call	PSwitchIncLoopCounter

PSwitchSetNoteLengthCh0:
	mov	y, PSwitchLoopCounter+1
	mov	a, PSwitchNoteLengths+y
	mov	PSwitchCh0NoteLen, a
	ret

PSwitchLoopCounter:
PSwitchIncLoopCounter:
	mov	a, #$00
	cmp	a, #$04
	bcs	+
	inc	PSwitchLoopCounter+1
+
	ret

PSwitchNoteLengths:
	db $0D, $0D, $0B, $09, $07	
endif

SFXTerminateVCMD:
	db $00

SFXTerminateCh:
	mov	a, !ChSFXPtrs+1+x
	beq	+
	mov	a, #SFXTerminateVCMD&$ff
	mov	!ChSFXPtrs+x, a
	mov	a, #SFXTerminateVCMD>>8
	mov	!ChSFXPtrs+1+x, a
	mov	a, #$03
	mov	!ChSFXNoteTimer+x, a
+
	ret
endif
SpeedUpMusic:
	mov	a, #$0a
	mov	$0387, a
	mov	a, $51
	call	L_0E14             ; add #$0A to tempo; zero tempo low      ;ERROR * 2
if !noSFX = !false
	mov	a, #$1d
	mov	$03, a
	mov	$00, a
	mov	a, #$00
	mov	$04, a
	mov	$07, a
else
	mov	a, #$00
	mov	$00, a
	mov	$04, a
endif
	;ret
;
ProcessAPU0Input:
	mov	a, $00				; \ If the value from $1DF9 was $80+, then play the "time is running out!" jingle.
	bmi	SpeedUpMusic			; /
if !noSFX = !false
	mov	x, #(!1DF9SFXChannel*2)		; \ 
	mov	y, #$00				; | 
	mov	$10, #(1<<!1DF9SFXChannel)	; | 
--						; | 
	bra 	ProcessSFXInput			; / Actually a subroutine.
	
if !PSwitchIsSFX = !true
PSwitchSFX:
	cmp	a, #$80
	bne	PlayPSwitchSFX
StopPSwitchSFX:
	mov	x, #$0e
StopPSwitchSFXLoop:
	asl	$1b
	push	p
	bcc	StopPSwitchSFXSkipCh
	call	SFXTerminateCh
StopPSwitchSFXSkipCh:
	dec	x
	dec	x
	pop	p
	bne	StopPSwitchSFXLoop
	ret

PlayPSwitchSFX:
	push	a
	mov	$03, #$81
	mov	y, #$03
	push	y
	mov	x, #(!PSwitchSFXCh0ID*2)
	mov	$10, #(1<<!PSwitchSFXCh0ID)
	call	ProcessSFXInput

	pop	y
	mov	$03, #$82
	push	y
	mov	x, #(!PSwitchSFXCh1ID*2)
	mov	$10, #(1<<!PSwitchSFXCh1ID)
	call	ProcessSFXInput

	pop	y
	mov	$03, #$83
	mov	x, #(!PSwitchSFXCh2ID*2)
	mov	$10, #(1<<!PSwitchSFXCh2ID)
	call	ProcessSFXInput

	pop	a
	and	a, #$40
	bne	PlayPSwitchActivateSFX
	ret

PlayPSwitchActivateSFX:
	mov	x, #(!PSwitchSFXTriggerChID*2)
	mov	$10, #(1<<!PSwitchSFXTriggerChID)
	mov	a, #$0b
	mov	y, #$00
	bra	ProcessSFXInput_prepareForSFX

endif
	
ProcessAPU3Input:
if !PSwitchIsSFX = !true
	mov	a, $03				;
	bmi	PSwitchSFX			;
endif
	mov	x, #(!1DFCSFXChannel*2)		; \ 
	mov	y, #$03				; | 
	mov	$10, #(1<<!1DFCSFXChannel)	; |
	;bra	ProcessSFXInput			; /


ProcessSFXInput:				; X = channel number * 2 to play a potential SFX on, y = input port to process, $10 = bitwise indicator of the current channel.
{
	mov	a, $0000+y			; \ If we've just received data from the SNES, prepare to process it.
	bne	.prepareForSFX			; / This involves keying off the correct channel.
						;
	;mov	a, !ChSFXTimeToStart+x		; Check to see if we've waited long enough to start the SFX.
	;bne	.waitingToStart			;
						;
	ret					;
		
.prepareForSFX					;
	push	a				;
	push	y				; \ 
	asl	a				; |
	mov	y, a				; | Y = SFX * 2, index to a table.
	pop	a				; | If a is 0, then the table we load from table 1.
	push	a				;
	cmp	a, #$01				; | Otherwise, we load from table 2.
	bcc	.loadFromSFXTable0		; /
if !useSFXSequenceFor1DFASFX = !true
	bne	.loadFromSFXTable1
	cmp	y, #$04*2
	beq	.useAPU1GirderSFX
	;cmp	y, #$01*2
	;beq	.useAPU1JumpSFX
.useAPU1JumpSFX
	mov	a, #APU1JumpSFXSequence&$FF
	mov	y, #APU1JumpSFXSequence>>8&$FF
	bra	.gottenPointerNoPop
.useAPU1GirderSFX
	mov	a, #APU1GirderClickSFXSequence&$FF
	mov	y, #APU1GirderClickSFXSequence>>8&$FF
	bra	.gottenPointerNoPop
endif
						;
.loadFromSFXTable1				;
if !PSwitchIsSFX = !true
	cmp	$03, #$81			;
	bcs	.PSwitchSFX
endif
	mov	a, SFXTable1-1+y		; \
	push	a				; | Move the pointer to the current SFX to the correct pointer.
	mov	a, SFXTable1-2+y		; |
	bra	.gottenPointer			;
						;
.loadFromSFXTable0				;
	mov	a, SFXTable0-1+y		; \
	push	a				; |
	mov	a, SFXTable0-2+y		; /
if !PSwitchIsSFX = !true
	bra	.gottenPointer

.PSwitchSFX	
	;Because the high bit gets shifted out, we don't need to do any more
	;modifications to Y.
	mov	a, PSwitchPtrs-1+y
	push	a
	mov	a, PSwitchPtrs-2+y

endif	
.gottenPointer
	pop	y
.gottenPointerNoPop
	movw	$14, ya

;Check SFX priority.
	mov	y, #$00
	mov	a, ($14)+y
	cmp	a, #$E0
	beq	.getSFXPriority
	mov	a, #$00
	bra	.sfxPriorityCheck
.getSFXPriority
	incw	$14
	mov	a, ($14)+y
	incw	$14

.sfxPriorityCheck
	pop	y
	push	a
	mov	a, !ChSFXPtrs+1+x
	pop	a
	beq	.checkAPU1SFX

	cmp	a, !ChSFXPriority+x
	bcs	.checkAPU1SFX

.noSFXOverwrite
	pop	a
	ret

.checkAPU1SFX
if !useSFXSequenceFor1DFASFX = !false
	;Check and see if APU1 SFX is playing there via detecting $1D.
	;APU1 SFX is playing if APU0/APU3 SFX sequence data is not playing,
	;but $1D has a voice bit set.
	push	a
	mov	a, $1d
	and	a, $10
	pop	a
	beq	.sfxAllocAllowed

	;Stop all APU1 SFX on the same channel.
	mov	$05, #$00
	mov	$1c, #$00
endif

.sfxAllocAllowed
	mov	!ChSFXPriority+x, a
	pop	a	
	mov	$0004+y, a			; > Tell the SPC to process this SFX.
	;mov	a, #$01				; \ We need to wait 2 ticks before processing SFX.
	;mov	!ChSFXTimeToStart+x, a		; /
	mov	a, $10				; \
	push	y
	call	KeyOffVoices
	pop	y
	or	($1d), ($10)			;
if !PSwitchIsSFX = !true
	cmp	$03, #$81			;
	or	($1b), ($10)
	bcs	.PSwitchSFXChSet
	eor	($1b), ($10)
.PSwitchSFXChSet
endif
	
	mov	a, $15
	mov	!ChSFXPtrs+1+x, a		; Store to current pointer
	mov	!ChSFXPtrBackup+1+x, a		; And backup pointer.
	mov	a, $14				;
	mov	!ChSFXPtrs+x, a			; Store to current pointer.
	mov	!ChSFXPtrBackup+x, a		; And backup pointer.
						;
	call	EffectModifier
	mov	a, #$00				; \
	mov	$90+x, a			; |
	mov	$91+x, a			; /
.return						;
	;ret					;
						;
.waitingToStart					;
	;setp					;
	;dec	!ChSFXTimeToStart&$FF+x		;
	;clrp					;
	;bne	.return				;
						;
	
	mov	a, #$02
	mov	!ChSFXNoteTimer+x, a		; Prevent an edge case.
endif
	ret

}

;

HandleYoshiDrums:				; Subroutine.  Call it any time anything Yoshi-drum related happens.

	mov	$5e, #$00
	mov	a, $6e
.drumSet
	tset	$5e, a

	;If you are using any extra code that also sets the mute flag, send
	;all of them over to HandleYoshiDrums_externalChMuteFlags+1 (and
	;uncomment the two lines below) so that they get preserved
	;accordingly. The +1 is important, since it's embedded in code (and
	;labels can't go in the middle of opcodes unless I were to input raw
	;hex opcodes instead of letting the assembler do its job).
HandleYoshiDrums_externalChMuteFlags:
	;mov	a, #$00
	;tset	$5e, a

	mov	a, $5e
	jmp	KeyOffVoices

UnpauseMusic:
	mov a, #$00
	mov !PauseMusic, a
.unsetMute:
	clr1 !NCKValue.6		; Unset the mute flag.
	call SetFLGFromNCKValue

	mov a, !SpeedUpBackUp	;\
	mov $0387, a			;/ Restore the tempo.
	ret

.silent:	
	mov a, #$01			;\ Set pause flag to solve issue when doing start+select quickly
	mov !PauseMusic, a	;/
	
	mov $f2, #$5c		; \ Key off voices
	mov $f3, #$ff		; / (so the music doesn't restart playing when using start+select)

	dec a
	mov $f2, #$2c		;\
	mov $f3, a		;| Mute echo.
	mov $f2, #$3c		;|
	mov $f3, a		;/
	bra .unsetMute

EnableYoshiDrums:				; Enable Yoshi drums.
	mov	a, #$4E			;TCLR opcode
	bra	+


DisableYoshiDrums:				; And disable them.
	mov	a, #$0E			;TSET opcode
+
	mov	HandleYoshiDrums_drumSet, a
if !useSFXSequenceFor1DFASFX = !false && !noSFX = !false
	call	HandleYoshiDrums
	bra	ProcessAPU1SFX
else
	bra	HandleYoshiDrums
endif

L_099C:
	mov	a, #$6c		; Mute, disable echo.  We don't want any rogue sounds during upload
	mov	y, #$60		; and we ESPECIALLY don't want the echo buffer to overwrite anything.
	movw	$f2, ya
	mov	!NCKValue, y
	
	mov	a, #$ff
	call	KeyOffVoices

	mov	a, #$00
	call	SetEDLDSP		; Also set the delay to 0.
	mov	$02, a			; 
	mov	$06, a			; Reset the song number
	mov	$0A, a			; 
if !noSFX = !false
	mov	$1d, a
endif
	mov	!MaxEchoDelay, a	;
	mov	a, #!reserveBufferZeroEDLGateDistance
	mov	SubC_table2_reserveBuffer_zeroEDLGate+1, a
	call	EffectModifier

	jmp	L_12F2             ; do standardish SPC transfer                                ;ERROR
				; Note that after this, the program is "reset"; it jumps to wherever the 5A22 tells it to.
				; The stack is also cleared.
	;ret

if !noSFX = !false
ForceSFXEchoOff:
	mov	a, #$00
	bra	+
ForceSFXEchoOn:
	mov	a, #$ff
+	mov	!SFXEchoChannels, a
if !useSFXSequenceFor1DFASFX = !false
	call	EffectModifier
	bra	ProcessAPU1SFX
else
	jmp	EffectModifier
endif
endif
ProcessAPU1Input:				; Input from SMW $1DFA
	mov	a, $01
	cmp	a, #$ff
	beq	L_099C
	cmp	a, #$02			; 02 = turn on Yoshi drums
	beq	EnableYoshiDrums	;
	cmp	a, #$03			; 03 = turn off Yoshi drums
	beq	DisableYoshiDrums	;
if !noSFX = !false
	cmp	a, #$05			;
	beq	ForceSFXEchoOff		;
	cmp	a, #$06			;
	beq	ForceSFXEchoOn		;
endif
	cmp	a, #$07			; 07 pauses music
if !noSFX = !false
	beq	PlayPauseSFX		;
else
	beq	PauseMusic
endif
	cmp	a, #$08			; 08 unpauses music
if !noSFX = !false
	beq	PlayUnpauseSFX
else
	beq	UnpauseMusic
endif
	cmp 	a, #$09			; KevinM's edit:
if !noSFX = !false
	beq	PlayUnpauseSilentSFX	; 09 unpauses music, but with the silent sfx
else
	beq	UnpauseMusic_silent
endif
if !noSFX = !false
	cmp	a, #$01			; 01 = jump SFX
	beq	CheckAPU1SFXPriority	;
	cmp	a, #$04
	beq	CheckAPU1SFXPriority
;
ProcessAPU1SFX:
if !useSFXSequenceFor1DFASFX = !false
	mov	a, $05		; 
	cmp	a, #$01		; \ If the currently playing SFX is the jump SFX
	beq	L_0A51		; / Then process that.
	cmp	a, #$04		; Else, if it's the girder SFX, then do that stuff.
	beq	L_0A11             ; (jmp $0b08)
L_0A0D:
	ret
L_0A11:
	jmp	L_0B08
else
	ret
endif

PlayPauseSFX:
	mov	a, #$11
	mov	$00, a
	mov	!ProtectSFX6, a
if !useSFXSequenceFor1DFASFX = !false
	bra	ProcessAPU1SFX
else
	ret
endif

PlayUnpauseSilentSFX:
	mov	a, #$2C
	bra	+
PlayUnpauseSFX:
	mov	a, #$12
+
	mov	$00, a
	mov	a, #$00
	mov	!ProtectSFX6, a
	;mov	$08, #$00
if !useSFXSequenceFor1DFASFX = !false
	bra	ProcessAPU1SFX
else
	ret
endif
else
	ret
endif

PauseMusic:
	mov a, $0387		;\
	mov !SpeedUpBackUp, a	;| Set the tempo increase to 0 and save it.
	mov a, #$00		;|
	mov $0387, a		;/

	inc a
	mov !PauseMusic, a
	
	set1  !NCKValue.6	; Set the mute flag.
	;ModifyNoise, called when restoring the noise frequency, will handle
	;setting the FLG DSP register.
	ret

if !noSFX = !false	
CheckAPU1SFXPriority:
if !useSFXSequenceFor1DFASFX = !true
	mov	x, #(!1DFASFXChannel*2)
	mov	y, #$01
	mov	$10, #(1<<!1DFASFXChannel)
	jmp	ProcessSFXInput
else
	mov	y, a
	;mov	y, #$00		;Default priority
	cmp	a, #$01
	bne	+
	mov	y, !JumpSFX1DFAPriority		;Priority for jump SFX
	bra	.gotPriority
+
	;cmp	a, #$04
	;bne	+
	mov	y, !GirderSFX1DFAPriority	;Priority for girder SFX
	;bra	.gotPriority
+

.gotPriority
	cmp	y, !ChSFXPriority+(!1DFASFXChannel*2)
	bcc	ProcessAPU1SFX

	mov	!ChSFXPriority+(!1DFASFXChannel*2), y
L_0A14:
	mov	$05, a		;
	;cmp	$05, #$01
	;bne	+
	mov	a, #$34
+
	cmp	$05, #$04
	bne	+
	mov	a, #$1c
+
	mov	$1c, a
	mov	a, #(1<<!1DFASFXChannel)	; \ Key off channel 7.
	
	call	KeyOffVoices
	set1	$1d.!1DFASFXChannel		; Turn off channel 7's music
if !PSwitchIsSFX = !true
	clr1	$1b.!1DFASFXChannel		; Turn off channel 7's P-Switch allocation
endif
	mov	x, #(!1DFASFXChannel*2)
	jmp	SFXTerminateCh
; $01 = 01
L_0A51:						;;;;;;;;/ Code change
	mov	$48, #$00		; Let NoteVCMD know that this is SFX code.

L_0A56:
	dbnz	$1c, L_0A38
RestoreInstrumentFromAPU1SFX:
	clr1	$1d.!1DFASFXChannel
	mov	a, #$00
	mov	$05, a
	mov	!ChSFXPriority+(!1DFASFXChannel*2), a
	mov	x, #(!1DFASFXChannel*2)
	jmp	RestoreInstrumentInformation

L_0A38:
	cmp	$1c, #$30			; Process the jump SFX.
	beq	L_0A68
	bcs	L_0AB0
; Beyond this point is essentially the hard-coded way to key on a note and
; manually set the pitch bend parameters, with the timer deciding what
; executes when.
	cmp	$1c, #$2a
	bne	L_0A99
	mov	x, #(!1DFASFXChannel*2)
	mov	$46, x
	mov	y, #$00
	mov	$91+x, y
	mov	y, #$12
	mov	$90+x, y
	mov	a, #$b9
	call	CalcPortamentoDelta
	bra	L_0A99

L_0A68:
	mov	$46, #(!1DFASFXChannel*2)
	mov	a, #$08
	call	SetSFXInstrument
	mov	a, #$b2
	call	NoteVCMD
	mov	y, #$00
	mov	$91+x, y
	mov	y, #$05
	mov	$90+x, y
	mov	a, #$b5
	call	CalcPortamentoDelta
	mov	y, #$38
	call	Quick1DFAMonoVolDSPWritesWKON
L_0A99:
	call	SFX1DFAKOFFCheck
L_0AA5:
	mov	x, #(!1DFASFXChannel*2)
	mov	a, $90+x
	beq	L_0AB0
	call	L_09CD
	jmp	SetPitch             ; force voice DSP pitch from 02B0/1
L_0AB0:
	ret

; $01 = 04 && $05 != 01
L_0B08:
	mov	$48, #$00		; Let NoteVCMD know that this is SFX code.
	dbnz	$1c, L_0AF2
	bra	RestoreInstrumentFromAPU1SFX

L_0AF2:
	cmp	$1c, #$18		; Process the girder SFX.
	beq	L_0AF7
	bcs	L_0AB0
; Beyond this point is essentially the hard-coded way to key on a couple of
; notes, with the timer deciding what executes when.
	cmp	$1c, #$0c
	bne	L_0B33
L_0AF7:
	mov	$46, #(!1DFASFXChannel*2)
	mov	a, #$07
	call	SetSFXInstrument
	mov	a, #$a4
	call	NoteVCMD

L_0B1C:
	mov	y, #$28
	call	Quick1DFAMonoVolDSPWritesWKON
L_0B33:
SFX1DFAKOFFCheck:
	mov	a, #$02
	cbne	$1c, L_0AB0
	mov	a, #(1<<!1DFASFXChannel)
	;mov	y, #$5c
	jmp	KeyOffVoices

Quick1DFAMonoVolDSPWritesWKON:
	mov	a, #(!1DFASFXChannel*$10)
	movw	$f2, ya
	inc	a
	movw	$f2, ya
	mov	a, #(1<<!1DFASFXChannel)
	jmp	KeyOnVoices
endif
endif
				; Call this routine to play the song currently in A.
PlaySong:

	;mov	y, #$00		
	;mov	$0387, y		; Zero out the tempo modifier.

if !noSFX = !false
	mov	!SFXEchoChannels, #$00
endif

L_0B5A:
	mov	$06, a		; Song number goes into $06.
	mov	$0c,#$02		;
	asl	a			; Turn A from a song number into a pointer
	mov	y, a		
	mov	a, SongPointers-$02+y	; Get the pointer for the current song
	push	a				; MODIFIED
	mov	$40, a
	mov	a, SongPointers-$01+y
	push	a				; MODIFIED
	mov	$41, a		; $40.w now points to the current song.
	; MODIFIED CODE START
	mov	a,#$00			; Clear various new addresses.
	mov	x,#$07			; These weren't used before, so they weren't cleared before.
-					;
	mov	$0160+x,a		;
	;mov	$245A+x,a		; These were used in AMM for the special pulse wave BRR file.
	;mov	$2463+x,a		; They can't be used now, since chances are music or something else is there now.
	dec	x			; Plus, the BRR file would be in an unstable location.
	bpl	-			;
					;
	mov	!WaitTime, #$02		;
	;mov	WaitTimeByte-1,a	;
					;
-	call	L_0BF0		; Get the first measure address.
	movw	$16, ya		; This is guaranteed to be valid, so save it and get the next one.
	mov	a, y		;
	bne	-			; Loop until the high byte of the measure address is 0
	
	mov	a, $16		; If the low byte of the current measure is #$FF, then we have one more to skip.
	beq	+			;
	call	L_0BF0		;
	+
	
	movw	ya, $40
	movw	!CustomInstrumentPos, ya
	
	pop	a
	mov	$41, a
	pop	a
	mov	$40, a
	; MODIFIED CODE END
	
	mov	x, #$0e            ; Loop through every channel
if !noSFX = !false
	mov	$48, #$80
endif
L_0B6D:
	mov	a, #$0a
	mov	!Pan+x, a         ; Pan[ch] = #$0A
	mov	a, #$ff
	mov	!Volume+x, a         ; Volume[ch] = #$FF
	inc	a
	mov	$02d1+x, a         ; Tuning[ch] = 0
	mov	!PanFadeDuration+x, a           ; PanFade[ch] = 0
	mov	$80+x, a           ; VolVade[ch] = 0
	mov	$a1+x, a		; Vibrato[ch] = 0
	mov	$b1+x, a		; ?
	mov	$0161+x, a	; Strong portamento/legato
	mov	!HTuneValues+x, a	
	
	mov	!ArpNoteIndex+x, a
	mov	!ArpNoteCount+x, a
	mov	!ArpCurrentDelta+x, a
	call	ClearRemoteCodeAddresses
if !noSFX = !false
	push	a
	;Don't clear pitch base if it is occupied by SFX.
	mov	a, $1d
	and	a, $48
	pop	a
	bne	+
endif
	mov	$02f0+x, a
	mov	$0210+x, a
+
if !noSFX = !false
	lsr	$48
endif
	dec	x
	dec	x
	bpl	L_0B6D	
	; MODIFIED CODE START
	mov	!MusicPModChannels, a
	mov	!MusicEchoChannels, a
	mov	!MusicNoiseChannels, a
	mov	!SecondVTable, a
	; MODIFIED CODE END	
	mov	$58, a             ; MasterVolumeFade = 0
	mov	$60, a             ; EchoVolumeFade = 0
	mov	$52, a             ; TempoFade = 0
	mov	$43, a             ; GlobalTranspose = 0
	mov	!PauseMusic, a		; Unpause the music, if it's been paused.
if !noSFX = !false
	mov	!ProtectSFX6, a		; Protection against START + SELECT
	mov	!ProtectSFX7, a		; Protection against START + SELECT
endif
	mov	$6e,a						; MODIFIED, channels affected by Yoshi drums.
	mov	$5e,a						; Also MODIFIED
	mov	!MasterVolume, #$c0          ; MasterVolume = #$C0
	mov	$51, #$36          ; Tempo = #$36


	; MODIFIED CODE START
	mov	y, #$10
-	
	; repeat ctr + Instrument[ch] = 0
	mov	$c0-1+y, a
	;(!ArpSpecial + !VolumeMult get zeroed out here...)
	mov	!ArpSpecial-1+y, a
	mov	!ArpNotePtrs-1+y, a
	;(!ArpLength + !ArpTimeLeft get zeroed out here...)
	mov	!ArpLength-1+y, a
	mov	$0300-1+y, a
	dbnz	y, -
	; MODIFIED CODE END
	
	call	EffectModifier
	bra	L_0BA5
;
L_0BA3:
	mov	$06, a		; ???
L_0BA5:
	mov	a, #$00
if !noSFX = !false
	mov	$0389, a
	mov	a, !NCKValue		; \ 
	and	!NCKValue, #$20		; | Disable mute and reset, keep echo off.
	cmp	!SFXNoiseChannels, #$00
	bne	+
	mov	a, #$00			; | Only reset the noise clock if SFX is not using it.
+
	call	ModifyNoise		; /
	mov	a, $1d		
	eor	a, #$ff		
	;mov	y, #$5c
	jmp	KeyOffVoices		; Set the key off for each voice to ~$1D.  Note that there is a ret in DSPWrite, so execution ends here. (goto L_0586?)
else
	and	!NCKValue, #$20		; \ Disable mute and reset, reset the noise clock, keep echo off.
	call	ModifyNoise		; /
	mov	a, #$ff
	jmp	KeyOffVoices
endif
; fade volume out over 240 counts
FadeOut:
	mov	x, #$f0
	mov	$58, x
	mov	a, #$00
	mov	$59, a
	setc
	sbc	a, !MasterVolume
	call	Divide16
	movw	$5a, ya            ; set volume fade out after 240 counts
	bra	L_0BE7
;
ProcessAPU2Input:

	; MODIFIED CODE START
	
	setp			    ; Get the special AMM byte.
	bbc1	$0160&$FF, .nothing ; If the second bit is set, then we've enabled sync. Otherwise, do nothing.
	incw	$0166&$FF	; Increase $166.
				; Note that this is different from AMM's code.
				; The old code never let the low byte go above #$C0.
				; A good idea in theory, but it both assumes that all
				; songs use 4/4 time, and it makes, for example,
				; using the song's time as an index to a table more difficult.
				; If the SNES needs 0 <= value < #$C0, it can limit the value itself.
.nothing			; 
	clrp			;
	mov	a, $02
	bmi	FadeOut		
	beq	L_0BE7
	jmp	PlaySong             ; play song in A
L_0BE7:
	mov	a, $0c
	bne	L_0BFE
	mov	a, $06
	bne	L_0C46
L_0BEF:
	ret
; read next word at $40 into YA
L_0BF0:
	mov	y, #$00
	mov	a, ($40)+y
	incw	$40
	push	a
	mov	a, ($40)+y
	incw	$40
	mov	y, a
	pop	a
	ret
;
L_0BFE:
	dbnz	$0c, L_0BEF
L_0C01:
	call	L_0BF0             ; read next word at $40
	movw	$16, ya            ; save in $16/17
	mov	a, y               ; high byte zero?
	bne	L_0C22
	mov	a, $16             ; refetch lo byte
	beq	L_0BA3             ; key off, return if also zero
	dec	$42
	beq	L_0C1C
	bpl	L_0C15
	mov	$42, a
L_0C15:
	call	L_0BF0             ; read next word at $40
	movw	$40, ya            ; "goto" that address
	bra	L_0C01             ; and continue
L_0C1C:
	incw	$40
	incw	$40               ; skip goto address
	bra	L_0C01             ; continue
L_0C22:
	mov	y, #$0f            ; high byte not zero:
	
L_0C24:				; This short loop sets $30 to contain the pointers to each track's starting "measure."
	mov	a, ($16)+y
	mov	$30+y, a
	dec	y
	bpl	L_0C24             ; set vptrs from [$16]


	mov	x, #$0e
	mov	$48, #$80          ; foreach voice
L_0C31:
	mov	a, $31+x
	beq	L_0C40             ;  next if vptr hi = 0
	mov	a, #$01
	mov	$70+x, a           ;  set duration counter to 1
	mov	a, $c1+x
	bne	L_0C40
	call	SetInstrument             ;  set instr to 0 if no instr set
L_0C40:
	lsr	$48
	dec	x
	dec	x
	bpl	L_0C31             ; loop
L_0C46:
	mov	x, #$00
	mov	$47, x
	mov	$48, #$01          ; foreach voice
L_0C4D:
	mov	$46, x
	mov	a, $31+x
	bne	+			; (fix an out-of-range error)
	jmp	L_0CC9             ; next if vptr hi zero
+
	dec	$70+x             ; dec duration counter
	beq	L_0C57		; (fix another out-of-range error)
	jmp	L_0CC6             ; if not zero, skip to voice readahead
L_0C57:
	call	GetCommandData             ; get next vbyte
	bne	L_0C7A

runningRemoteCodeGate:
	mov	a, $c0+x           ; vcmd 00: end repeat/return
	beq	L_0C01             ;  goto next $40 section if rpt count 0
	dec	$c0+x             ;  dec repeat count
	bne	L_0C6E             ;  if zero then
	mov	a, $03e0+x
	mov	$30+x, a
	mov	a, $03e1+x
	bra	L_0C76             ;   goto 03E0/1
L_0C6E:
	mov	a, $03f0+x         ;  else
	mov	$30+x, a
	mov	a, $03f1+x         ;   goto 03F0/1
L_0C76:
	mov	$31+x, a
	bra	L_0C57             ;  continue to next vbyte
L_0C7A:
	bmi	L_0C9F             ; vcmds 01-7f
	
	mov	$0200+x, a         ;  set cmd as duration
	call	GetCommandDataFast             ;  get next vcmd
	bmi	L_0C9F             ;  if not note then
	push	a
	xcn	a
	and	a, #$07
	mov	y, a
	mov	a, NoteDurations+y
	mov	$0201+x, a         ; set dur% from high nybble
	pop	a
	and	a, #$0f			
	cmp	!SecondVTable, #$00	; \ 
	beq	+			; | Get the correct velocity table index
	or	a, #$10			; |
+					; |
	mov	y, a			; /
	mov	a, VelocityValues+y
	mov	$0211+x, a         ; set per-note vol from low nybble
	or	($5c), ($48)       ; mark vol changed?
	call	GetCommandDataFast             ; get next vbyte
L_0C9F:
	cmp	a, #$da
	bcc	L_0CA8             ; vcmd da-ff:
	
	call	L_0D40             ; dispatch vcmd
	bra	L_0C57             ; do next vcmd
L_0CA8:
	                           ; vcmd 80-d9 (note)
if !noSFX = !false
	mov	$10, $1d	; Check if there's a sound effect on the current channel.
	or	($10),($5e)	; If it's muted or there's a sound effect playing...
else
	mov	$10, $5e	; Check if this channel's music is muted.
endif
	and	($10), ($48)	; Only check the current channel.
	
					; Warning: The code ahead gets messy thanks to arpeggio modifications.

	mov	y, a			; / Put the current note into y for now.

	
	cmp	y, #$c6			; \ If the note is a rest or tie, then don't save the current note pitch.
	bcs	+				; /
.anythingGoes
	mov	!PreviousNote+x, a	; Save the current note pitch.  The arpeggio command needs it.
+
	mov	a, $10
	bne	L_0CB3
	cmp	y, #$c6			; \ Ties and rests shouldn't affect anything arpeggio related.
	bcs	+			; /
	mov	a, !ArpNoteCount+x	; \ If there's currently an arpeggio playing (which handles its own notes)...
	beq	+			; | Then don't play a note.  The arpeggio handler up ahead will do it automatically.
	mov	a, #$01			; | But we do have to restart the timer so that it will work correctly (and start right now).
	mov	!ArpTimeLeft+x, a	; /
	dec	a
	dec	a
	mov	!ArpNoteIndex+x, a	; Set the note index to -1 (which will be increased to 0, where it should be for the first note).
	
	mov	a, y			; \
	cmp	a, #$c6			; | Ties/rests don't mess up glissando.
	bcs	.notGlissando		; /
	mov	a, !ArpType+x		; \
	cmp	a, #$02			; |
	bne	.notGlissando		; | If the arpeggio type is glissando,
	mov	a, !ArpNoteCount+x	; | We can only play one note with this on.
	dec	a			; |
	mov	!ArpNoteCount+x, a	; |
	bne	.glissandoIsStillOn	; |
	mov	!ArpCurrentDelta+x, a	; | If we're turning it off, then reset the delta.
+
.glissandoOver
	mov	a, y			; / And actually play the next note.
	call	NoteVCMD             ; handle note cmd if vbit 1D clear
.glissandoIsStillOn
.notGlissando
L_0CB3:
	mov	a, $0200+x
	mov	$70+x, a           ; set duration counter from duration
	mov	y, a
	mov	a, $0201+x
	mul	ya
	mov	a, y
	bne	L_0CC1
	inc	a
L_0CC1:
	mov	$0100+x, a         ; set note dur counter from dur * dur%
	bra	L_0CC9
L_0CC6:
	call	L_10A1             ; do voice readahead
L_0CC9:	
	call	HandleArpeggio	; Handle all things related to arpeggio.
	inc	x
	inc	x
	asl	$48
	bcs	L_0CD2
	jmp	L_0C4D             ; loop
L_0CD2:		
	mov	a, $52             ; do global fades
	beq	L_0CE3
	dbnz	$52, L_0CDC
	movw	ya, $52
	bra	L_0CE1
L_0CDC:
	movw	ya, $54
	addw	ya, $50
L_0CE1:
	movw	$50, ya
L_0CE3:
	mov	a, $60
	beq	L_0D03
	dbnz	$60, L_0CF4
	mov	a, #$00
	mov	y, $62
	movw	$61, ya
	mov	y, $64
	bra	L_0CFE
L_0CF4:
	movw	ya, $65
	addw	ya, $61
	movw	$61, ya
	movw	ya, $67
	addw	ya, $63
L_0CFE:
	movw	$63, ya
	call	L_0EEB
L_0D03:
	mov	a, $58
	beq	L_0D17
	dbnz	$58, L_0D0E
	movw	ya, $58
	bra	L_0D12
L_0D0E:
	movw	ya, $5a
	addw	ya, $56
L_0D12:
	movw	$56, ya
	mov	$5c, #$ff          ; set all vol chg flags
L_0D17:
	mov	x, #$0e
	mov	$48, #$80
L_0D1C:
	mov	a, $31+x
	beq	L_0D23
	call	L_0FDB             ; per-voice fades?
L_0D23:
	lsr	$48
	dec	x
	dec	x
	bpl	L_0D1C
	mov	$5c, #$00          ; clear volchg flags
if !noSFX = !false
	mov	a, $1d
	eor	a, #$FF		;;;;;;;;;;;;;;;Code change
	and	a, $47		; Set legato to off for voice.
	mov	$12, a
else
	mov	$12, $47
endif
	mov	a, $0162
	push	a
	or	a, $12
	mov	$0162, a
	pop	a
	and	a, $0161
	eor	a, #$FF
	and	a, $12		
	;and     a, $47
; key on voices in A
KeyOnVoices:
	mov	$F2, #$5C
	mov	$F3, #$00
	mov	$F2, #$4C
	mov	$F3, a
	tset	!PlayingVoices, a
	ret

KeyOffVoiceWithCheck:
if !noSFX = !false
	call	TerminateIfSFXPlaying
endif
	mov	a, $48
KeyOffVoices:
	tclr	!PlayingVoices, a
	mov	y, #$5c
DSPWrite:
	mov	$f2, y	; write A to DSP reg Y
	mov	$f3, a	
	ret
	
; dispatch vcmd in A
L_0D40:
	mov	x, a
	asl	a
	mov	y, a
	mov	a, CommandDispatchTable-($DA*2&$FF)+1+y        ; $DA minimum? (F90)
	push	a
	mov	a, CommandDispatchTable-($DA*2&$FF)+y
	push	a
	mov	a, CommandLengthTable-$DA+x
	mov	x, $46
	dec	a
	beq	L_1266

; get next vcmd stream byte for voice $46
GetCommandData:
	mov	x, $46
; get next vcmd stream byte into A/Y
GetCommandDataFast:
	mov	a, ($30+x)
L_1260:
	inc	$30+x
	bne	L_1266
	inc	$31+x
L_1266:
	mov	y, a
	ret

	incsrc "Commands.asm"
	
; vcmd DA: set instrument
; vcmd DB: set pan
; vcmd DC: pan fade
; vcmd DE: vibrato on
; vcmd DF: vibrato off
; vcmd EA: vibrato fade
; vcmd E0: set master volume
; vcmd E1: master vol fade
; vcmd E2: tempo
; vcmd E3: tempo fade
; vcmd E4: transpose (global)
; vcmd E5: tremolo on
; vcmd E6: tremolo off
; vcmd EB: pitch envelope (release)
; vcmd EC: pitch envelope (attack)
; vcmd E7: set voice volume base
; vcmd E8: voice volume base fade
; vcmd EE: tuning
; vcmd E9: call subroutine
; vcmd EF: set echo vbits/volume
; vcmd F2: echo volume fade
; vcmd F0: disable echo
; vcmd F1: set echo delay, feedback, filter

; calculate portamento delta
CalcPortamentoDelta:
	and	a, #$7f
	mov	$16, #$02b0&$FF
	;References $02B1, $02C0-$02C1, $02D0
	mov	y, $90+x           ; portamento steps
	push	y
	bra	SlideToParamIndexedPostFetch

cmdE8:					; Fade the volume
	mov   $80+x, a
	mov   $16, #$0240&$FF
	;References $0241, $0250-$0251, $0260
	bra   SlideToParamIndexed

cmdDC:					; Fade the pan
	mov   !PanFadeDuration+x, a
	mov   $16, #$0280&$FF
	;References $0281, $0290-$0291, $02A0

SlideToParamIndexed:
	;Calculate the delta value for an indexed value.
	push	a
	call	GetCommandDataFast
SlideToParamIndexedPostFetch:
	or	($16), ($46)
	mov	$17, #$02
	mov	y, #$20
	mov	($16)+y, a
	mov	y, #$01
	setc
	sbc	a, ($16)+y
	pop	x
	call	Divide16
	push	y
	mov	y, #$10
	mov	($16)+y, a
	pop	a
	inc	y
	mov	($16)+y, a
	ret

; signed 16 bit division
Divide16:
	bcs	L_0F85
	eor	a, #$ff
	inc	a
	call	L_0F85
	movw	$14, ya
	movw	ya, $0e
	subw	ya, $14
	ret
L_0F85:
	mov	y, #$00
	div	ya, x
	push	a
	mov	a, #$00
	div	ya, x
	pop	y
	mov	x, $46
	ret
; dispatch table for 0d44 (vcmds)
		;106B
incsrc "CommandTable.asm"

;
L_0FDB:	
	
	mov	a, $80+x
	beq	L_0FEB
	or	($5c), ($48)
	mov	a, #$0240&$FF
	mov	y, #$0240>>8
	dec	$80+x
	;Modifies $0240-$0241, $0250-$0251, $0260
	call	L_1075
L_0FEB:
	mov	y, $b1+x
	beq	L_1013
	mov	a, $0370+x
	cbne	$b0+x, L_1011
	or	($5c), ($48)
	mov	a, $0360+x
	bpl	L_1005
	inc	y
	bne	L_1005
	mov	a, #$80
	bra	L_1009
L_1005:
	clrc
	adc	a, $0361+x
L_1009:
	mov	$0360+x, a
	call	L_123A
	bra	L_1019
L_1011:
	inc	$b0+x
L_1013:
	mov	a, $0211+x         ; volume from note
	call	L_124D             ; set voice vol from master/base/note
L_1019:
	mov	a, !PanFadeDuration+x
	bne	L_1024
	mov	a, $48
	and	a, $5c
	bne	L_102D
	ret
; do: pan fade and set volume
L_1024:
	mov	a, #$0280&$FF
	mov	y, #$0280>>8
	dec	!PanFadeDuration+x
	;Modifies $0280-$0281, $0290-$0291, $02a0
	call	L_1075
L_102D:
	mov	a, !Pan+x		; Get the pan for this channel.
	mov	y, a		;
	mov	a, $0280+x	;
	movw	$10, ya            ; set $10/1 from voice pan
; set voice volume DSP regs with pan value from $10/1
L_1036:
if !noSFX = !false
	call	TerminateIfSFXPlaying
endif
	mov	a, x		;
	xcn	a			;
	lsr	a			;
	mov	$12, a             ; $12 = voice X volume DSP reg
L_103B:
	mov	y, $11
	mov	a, PanValues+$01+y         ; next pan val from table
	setc
	sbc	a, PanValues+y         ; pan val
	mov	y, $10
	mul	ya
	mov	a, y
	mov	y, $11
	clrc
	adc	a, PanValues+y         ; add integer part to pan val
	mov	y, a
	mov	a, $0371+x         ; volume
	mul	ya
	
	mov	$14, y			; \ 
	mov	a, !VolumeMult+x	; | Add the computed volume to (computedvolume * volumemultipier / $100)
	mul	ya			; |
	mov	a, y			; |
	clrc				; |
	adc	a, $14			; |
	mov	y, a			; /
	
	mov	a, !SurroundSound+x         ; bits 7/6 will negate volume L/R
	bbc1	$12.0, L_105A
	asl	a
L_105A:
	bpl	L_1061
	mov	a, y
	eor	a, #$ff
	inc	a
	mov	y, a
L_1061:
if !noSFX = !false
	mov	a, $48
	and	a, !MusicNoiseChannels
	beq	++
	and	a, $1d
	bne	++
if !noiseFrequencyMatchChecks = !true
	;Hardware limitations prevent more than one noise frequency from
	;playing at once. Thus, we zero out the voice volume of the music
	;if SFX is using the noise and the frequencies don't match.
	mov	a, !NCKValue
	and	a, #$1f
	cmp	a, $0389
	beq	++
else
	mov	a, !SFXNoiseChannels
	beq	++
endif
	push	x
	bbc1	$12.0, +
	inc	x
+
	mov	$d0+x, y
	mov	y, #$00
	pop	x
++
endif
	mov	a, $12
	movw	$f2, ya             ; set DSP vol if vbit 1D clear
	mov	a, #$00
	mov	y, #$14
	subw	ya, $10
	movw	$10, ya            ; $10/11 = #$1400 - $10/11
	inc	$12               ; go back and do R chan vol
	bbc1	$12.1, L_103B
	ret
; add fade delta to value (set final value at end)
L_1075:
	movw	$14, ya		;
	clrc			; Fade delta counter decides which way this branch goes
	bne	L_1088		; (ideally through the dec d+x opcode prior to calling L_1075)
	adc	a, #$20		; \ $16 gets the passed pointer plus #$20
	movw	$16, ya		; /
	mov	a, x		; \ mov y, x 
	mov	y, a		; / mov y, $48
	mov	a, #$00		;
	push	a			;
	mov	a, ($16)+y	; a gets this channel's value.
	inc	y			; 
	bra	L_109A		;
L_1088:				;
	adc	a, #$10		;
	movw	$16, ya		;
	mov	a, x		;
	mov	y, a		;
	mov	a, ($14)+y	;
	clrc			;
	adc	a, ($16)+y	;
	push	a			;
	inc	y			;
	mov	a, ($14)+y	;
	adc	a, ($16)+y	;
L_109A:				;
	mov	($14)+y, a	;
	dec	y			;
	pop	a			;
	mov	($14)+y, a	;
	ret			;

ShouldSkipKeyOff:		; Returns with carry set if the key off should be skipped.  Otherwise, the key off should be performed.	
{
;L_10AC:							;
	mov	a, $30+x				; \ 
	mov	y, $31+x				; |
	movw	$14, ya					; |
L_10B2:							; |
	mov	y, #$00					; |
	mov	a, ($14)+y				; | Loop until the next byte is a note/command.
	beq	L_10D1					; |
	bmi	L_10BF					; |
L_10BA:							; |
;Bugfix by KungFuFurby 11/19/20:
;All additions are being directly sent to the pointer instead of using y.
;This is to avoid a softlock via a 256-byte wraparound due to the Y
;register overflowing back to zero.
	incw	$14					; |
	mov	a, ($14)+y				; |
	bpl	L_10BA					; /
L_10BF:
	cmp	a, #$c6					; \ C6 is a tie.
	beq	skip_keyoff				; / So we shouldn't key off the voice.
	cmp	a, #$da					; \ Anything less than $DA is a note (or percussion, which counts as a note)
	bcc	L_10D1					; / So we have to key off in preparation
	cmp	a, #$fb					; \ FB is a variable-length command.
	bne	.normalCommand				; / So it has special handling.
	incw	$14					; \
	mov	a, ($14)+y				; / Get the next byte
	bpl	.normal					; \ 
	mov	a, #$03
	bra	+
.normal
	inc	a					; \ Add the number of bytes in the command.
	inc	a					; / Plus the number of bytes the command itself takes up .	
	bra	+					;
	
.normalCommand
	mov	y, a					; \ 
	mov	a, CommandLengthTable-$DA+y		; | Add the length of the current command (so we get the next note/command/whatever).
	mov	y, #$00					; |
+							; |
	addw	ya, $14					; |
	movw	$14, ya					; |
	bra	L_10B2					; /
L_10D1:							;
	mov	$10, a
	clrc
	call	TerminateOnLegatoEnable			; Key off the current voice (with conditions).

	mov	a, !InRest+x
	bne	+
	mov	a, !remoteCodeType+x
	cmp	a, #$03
	bne	keyoff
	mov	a, $10
	cmp	a, #$c7
	beq	skipKeyOffAndRunCode
	mov	a, $70+x
	cmp	a, !WaitTime
	beq	keyoff
skipKeyOffAndRunCode:
	call	RunRemoteCode
	bra	skip_keyoff
+
keyoff:
	setc
	ret
skip_keyoff:
	clrc
	ret
}

TerminateOnLegatoEnable:
	mov	a, $48
	and	a,$0161
	and	a,$0162
	beq	+
	;WARNING: Won't work if anything else is in the stack!
	pop	a	;Jump forward one pointer in the stack in order to
	pop	a	;terminate the entire preceding routine.
+
	ret


L_10A1:

	mov	a, !remoteCodeType+x
	dec	a
	bne	.noRemoteCode2
	mov	a, !remoteCodeTimeLeft+x
	dec	a
	mov	!remoteCodeTimeLeft+x, a
	bne	.noRemoteCode2
	call	RunRemoteCode

.noRemoteCode2

	setp						;
	dec	$0100&$FF+x					;
	clrp						;
	beq	.doKeyOffCheck 			;L_10AC					;
	
	mov	a, !remoteCodeType+x			; \ Branch away if we have no code to run before a note ends.
	cmp	a, #$02					; |
	bne 	.noRemoteCode				; /

	mov	a, !remoteCodeTimeValue+x		; \
	cmp	a, $0100+x				; | Also branch if we're not ready to run said code yet.
	bne	.noRemoteCode				; /
	
	call	ShouldSkipKeyOff			; \ If we're going to skip the keyoff, then also don't run the code.
	bcc	.noRemoteCode				; /
	
	call	RunRemoteCode				;
	
.noRemoteCode
	mov	a, !WaitTime				;
	cbne	$70+x, +				;
.doKeyOffCheck
	call	ShouldSkipKeyOff
	
	bcc	+
	call	KeyOffVoiceWithCheck 
+
	
	clr1	$13.7					;
	mov	a, $90+x				;
if !noSFX = !false
	beq	L_10E4					;
	mov	a, $48					;
	and	a, $1d					;
	beq	L_1111					;
else
	bne	L_1111
endif
L_10E4:
	mov	a, ($30+x)				; Code for handling the $DD command.
	cmp	a, #$dd					; I don't know why this is here instead of in its dispatch table.
	beq	+					; Maybe so that it can properly do the "read-ahead" effect?
	call	L_112A
	bra	L_1133
+
if !noSFX = !false
	mov	a, $48					; \ 
	and	a, $1d					; | Check to see if the current channel is disabled with a sound effect.
	beq	L_10FB					; /
	mov	$10, #$04
L_10F3:
	call	L_1260
	dbnz	$10, L_10F3
	bra	L_1111
endif
L_10FB:
	call	L_1260					; \ 
	call	GetCommandDataFast			; |
	mov	$91+x, a				; | Get the $DD parameters.
	call	GetCommandDataFast			; |
	mov	$90+x, a				; |
	call	GetCommandDataFast			; /
	clrc
	adc	a, $43
	call	CalcPortamentoDelta
L_1111:
	call	L_09CDWPreCheck
L_1133:
	mov	a, $a1+x
	beq	L_1140
	mov	a, $0340+x
	cmp	a, $a0+x
	beq	L_1144
	inc	$a0+x
L_1140:
	bbs1	$13.7, L_1195		; If $13.7 is set, recalibrate the pitch.
-
	ret
L_1144:					; Process vibrato.

	mov	a, !PlayingVoices	; \ 
	and	a, $48			; | If there's no voice playing on this channel,
	beq	-			; / then don't do all these time-consuming calculations.
	
	mov	a, $0341+x
	beq	L_1166
	cmp	a, $0110+x
	bne	L_1155
	mov	a, $0351+x
	mov	$a1+x, a
	bra	L_1166
L_1155:
	mov	a, $0110+x
	beq	L_115C
	mov	a, $a1+x
L_115C:
	clrc
	adc	a, $0350+x
	mov	$a1+x, a
	setp
	inc	$0110&$FF+x
	clrp
L_1166:
	mov	a, $0330+x
	clrc
	adc	a, $0331+x
	mov	$0330+x, a
;
L_1170:
	mov	$12, a
	asl	a
	asl	a
	bcc	L_1178
	eor	a, #$ff
L_1178:
	mov	y, a
	mov	a, $a1+x
	cmp	a, #$f1
	bcs	L_1185
	mul	ya
	mov	a, y
	mov	y, #$00
	bra	L_1188
L_1185:
	and	a, #$0f
	mul	ya
L_1188:
	bbc1	$12.7, L_1191
	movw	$12, ya
	movw	ya, $0e
	subw	ya, $12
L_1191:
	addw	ya, $10		;Add vibrato offset to resulting pitch.
	movw	$10, ya
L_1195:
	jmp	SetPitch
; per-voice fades/dsps?


HandleVoice:
	clr1	$13.7
	
	mov	a, $b1+x
	beq	L_11A7
	mov	a, $0370+x
	cbne	$b0+x, L_11A7
	call	L_122D             ; voice vol calculations
L_11A7:
	mov	a, !Pan+x
	mov	y, a
	mov	a, $0280+x
	movw	$10, ya            ; $10/11 = voice pan value
	mov	a, !PanFadeDuration+x           ; voice pan fade counter
	bne	L_11B9
	bbs1	$13.7, L_11C3	; If $13.7 is set, recalibrate the volume.
	bra	L_11C6
L_11B9:
	mov	a, $0291+x
	mov	y, a
	mov	a, $0290+x         ; pan fade delta
	call	L_1201             ; add delta (with mutations)?
L_11C3:
	call	L_1036             ; set voice DSP regs, pan from $10/11
L_11C6:
	clr1	$13.7
	call	DDEEFix	
	mov	a, $90+x           ; pitch slide counter
	beq	L_11E3
	mov	a, $91+x
	bne	L_11E3
	mov	a, $02c1+x
	mov	y, a
	mov	a, $02c0+x
	call	L_11FF             ; add pitch slide delta
L_11E3:
	mov	a, $a1+x
	bne	L_11EB
L_11E7:
	bbs1	$13.7, L_1195      ; If $13.7 is set, recalibrate the pitch.
	ret
L_11EB:
	mov	a, $0340+x	; Process vibrato.
	cbne	$a0+x, L_11E7
	mov	y, $49
	mov	a, $0331+x
	mul	ya
	mov	a, y
	clrc
	adc	a, $0330+x
	jmp	L_1170
;
L_11FF:
	set1	$13.7
;
L_1201:
	movw	$16, ya
	mov	$12, y
	bbc1	$12.7, L_120E
	movw	ya, $0e
	subw	ya, $16
	movw	$16, ya
L_120E:
	mov	y, $49	;Account for fractions of a music tick in slides.
	mov	a, $16
	mul	ya
	mov	$14, y
	mov	$15, #$00
	mov	y, $49
	mov	a, $17
	mul	ya
	addw	ya, $14
	bbc1	$12.7, L_1228
	movw	$14, ya
	movw	ya, $0e
	subw	ya, $14
L_1228:
	addw	ya, $10
	movw	$10, ya
	ret
;
L_122D:
	set1	$13.7
	mov	y, $49
	mov	a, $0361+x
	mul	ya
	mov	a, y
	clrc
	adc	a, $0360+x
L_123A:
	asl	a
	bcc	L_123F
	eor	a, #$ff
L_123F:
	mov	y, $b1+x
	mul	ya
	mov	a, $0211+x
	mul	ya
	mov	a, y
	eor	a, #$ff
	setc
	adc	a, $0211+x
L_124D:
	mov	y, a
; set voice volume from master/base/A
	mov	a, !Volume+x	; Get volume
	mul	ya		; Multiply by qX
	mov	a, !MasterVolume             ; master volume
	mul	ya		; Multiply by master volume.
	mov	a, y		;
	mul	ya		; \ Vol = [(Vol^2) / 2] ?
	mov	a, y		; /
	mov	$0371+x, a         ; voice volume
	ret
	
SetFLGFromNCKValue:
	mov	a, !NCKValue
ModifyNoise:				; A should contain the noise value.
	and	a, #$1f
	and	!NCKValue, #$e0		; Clear the current noise bits.
	or	a, !NCKValue		; \ Set and save the current noise bits.
	mov	!NCKValue, a		; / 
	mov	y, #$6c			; \ Write
	jmp	DSPWrite		; /

; for 0C89 - note dur%'s
NoteDurations:
	db $33, $66, $80, $99, $B3, $CC, $E6, $FF

; per-note velocity values
VelocityValues:
	db $08, $12, $1B, $24, $2C, $35, $3E, $47, $51, $5A, $62, $6B, $7D, $8F, $A1, $B3	; Normal, SMW velocities.
	db $19, $33, $4C, $66, $72, $7F, $8C, $99, $A5, $B2, $Bf, $CC, $D8, $E5, $F2, $FC	; Standard N-SPC velocities.

; pan table (max pan full L = $14.00)
PanValues:
	db $00, $01, $03, $07, $0D, $15, $1E, $29, $34, $42, $51, $5E, $67, $6E, $73, $77
	db $7A, $7C, $7D, $7E, $7F






; default values (1295) for DSP regs (12A1)
;  mvol L/R max, echo vol L/R zero, FLG = echo off/noise 400HZ
;  echo feedback = $60, echo/pitchmod/noise vbits off
;  source dir = $8000, echo ram = $6000, echo delay = 32ms

DefDSPValues:
		db $7F, $7F, $00, $00, $2F, $00, $00, $00, $00, $2F, $88, $00 

DefDSPRegs:
		db $0C, $1C, $2C, $3C, $6C, $0D, $2D, $3D, $4D, $5D, $6D, $7D

; echo filters 0 and 1
EchoFilter0:
	db $FF, $08, $17, $24, $24, $17, $08, $FF
EchoFilter1:
	db $7F, $00, $00, $00, $00, $00, $00, $00


; pitch table
PitchTable:
	dw $085f
	dw $08de
	dw $0965
	dw $09f4
	dw $0a8c
	dw $0b2c
	dw $0bd6
	dw $0c8b
	dw $0d4a
	dw $0e14
	dw $0eea
	dw $0fcd
	dw $10be

; do: standardish SPU transfer
L_12F2:
	print "ReuploadPos: $",pc		
				; This is where the engine should jump to when downloading consecutive blocks of data.
	
				; The following is a near identical copy of the SPC IPL ROM, just modified a touch to emulate
				; SMW sending data to ($14)+y instead of ($00)+y.  See "http:;www.romhacking.net/documents/197/".
			
			

		
	
	mov	$F4, #$AA		; Signal "ready" to 5A22: $2140-1 will return #$BBAA
	mov	$F5, #$BB
Wait:
	cmp	$F4, #$CC		; wait for 5A22 to write #$CC to $2140
	bne	Wait
	bra	Start
Trans:
	mov	y, $F4		; *** TRANSFER ROUTINE ***
	bne	Trans		; First, wait for 5A22 to indicate Byte 0 ready on $2140
StartLoop:
	cmp	y, $F4		; start loop: wait for "next byte/end" signal on $2140
	bne	Unexpected
	mov	a, $F5		; Got "next byte" ($2140 matches expected byte index)
	mov	$F4, y		; Read byte-to-write from $2141, echo $2140 to signal
	mov	($14)+y, a		; ready, and write the byte and update the counter.
	inc	y
	bne	StartLoop
	inc	$15			; (handle $xxFF->$xx00 overflow case on increment)
Unexpected:
	bpl	StartLoop
	cmp	y, $F4		; If "next byte/end" is not equal to expected next byte
	bpl	StartLoop		; index, it's "end": drop back into the main loop.
Start:
	movw	ya, $F6		; *** MAIN LOOP ***
	movw	$14, ya		; Get address from 5A22's $2142-3, 
	movw	ya, $F4		; mode from $2141, and echo $2140 back
	mov	$F4, a
	mov	a, y
	mov	x, a
	bne	Trans		; Mode non-0: begin transfer	
	
				; reset ports, keep timer running
	mov	$f1, #$31		; Has to be done quickly or else subsequent writes
				; to the SPC will be ignored (playing music right
				; after loading it, for example).  Important to note:
				; even despite doing this as quickly as possible, the
				; 5A22 will still have to wait a bit (6 NOPs or so)
				; before sending any data.
	
	mov	a, #$00
	mov	y, a
	movw	$f4, ya
	movw	$f6, ya
	movw	$04, ya
	movw	$06, ya
	setp		; Clear the output ports
	movw	$0166&$FF, ya	;
	movw	$0168&$FF, ya	;
	clrp		;
	
	;mov	$0387, a
	mov	!PauseMusic, a
if !noSFX = !false
	mov	$0389, a
endif
	mov	!MaxEchoDelay, a
	
	mov	x, #$cf		; Reset the stack pointer.
	mov	sp, x
	
	mov	x, #$00
	mov	$01, x
	
	mov	!NCKValue, #$20
	call	SetFLGFromNCKValue
	
	mov	y, #$10
	mov	a, #$00
-
	mov	!ChSFXPtrs-1+y, a	; \ Turn off sound effects
	dbnz	y, -			; /
if !PSwitchIsSFX = !true
	mov	$1b, #$00
endif	
	jmp	($0014+x)		; Jump to address
	
GetSampleTableLocation:

	print "SRCNTableCodePos: $",pc		
				; This is where the engine should jump to after uploading samples.

-	cmp	$f4, #$CC	; Wait for the 5A22 to send #$CC to $2140.
	bne -			; By then it should have also written DIR to $2141
				; as well as the jump address to $2142-$2143.
				
	mov	a, #$5d
	mov	y, $f5		; Set DIR to the 5A22's $2141
	movw	$f2, ya
	push	y
	
	movw	ya, $f6
	movw	$14, ya
	mov	$f1, #$31		; Reset input ports
	pop	a
	mov	$f5, a		; Echo back DIR
	mov	y, #$00
	jmp	($0014+x)		; Jump to the upload location.
	

	incsrc "InstrumentData.asm"
	

if !noSFX = !false
if !useSFXSequenceFor1DFASFX = !true
APU1JumpSFXSequence:
	db $E0
	db !JumpSFX1DFAPriority
	db $DA,$08,$05,$38,$DD,$B2,$00,$05,$B5,$2A,$EB,$01,$12,$B9,$00
APU1GirderClickSFXSequence:
	db $E0
	db !GirderSFX1DFAPriority
	db $DA,$07,$0C,$28,$A4,$A4,$00
endif

	SFXTable0:
	SFXTable1:
endif
	SongPointers:

	
	