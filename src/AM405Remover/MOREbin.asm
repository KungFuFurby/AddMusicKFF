;This is the SPC code for my more.bin. Assembles with SPCAS.
;Important addresses:
;$125C/125E will get the next byte of song data.
;$0697 will write to the DSP

dw end2-start2
dw $669F
base $669F
start2:
SetInstrumentStandard:
;Set the high byte for the pointer to sample data
mov x,$46		;F8  46
mov a,$C1+x	;F4  C1
bmi Percussion	;30  08
;If it's a normal instrument, set for $60D7
mov $E3,#$60	;8F 5F E3
mov y,#$05		;8D  05
dec a			;9C  
mov $E2,#$D7	;8F 46 E2
bra Instrument	;2F  08
Percussion:
;If it's percussion, set for $60E6
mov $E3,#$61	;8F 5F E3
mov $E2,#$95	;8F A5 E2
mov y,#$06		;8D  06
setc 			;80  
sbc a,#$D0		;A8  D0
Instrument:
;If it's a normal instrument, multiply the instument number-1 *5 to get the pointer to instrument data
;If it's a percussion, multiply instrument number by 6 to get pointer to percussion data
mul ya			;CF  
;And add that to the pointer we found earlier
db $7A,$E2 		;adc ya,$E2
db $DA,$E2 		;mov $E2,ya
ret 			;6F  

SetInstrument:
;Loop through and set all the settings for the instrument number
mov y,#$05		;8D  05
SampleSettingsLoop:
db $F7,$E2		;mov a,[$E2]+y
db $D7,$14		;mov [$14]+y,a
dec y			;DC  
bpl SampleSettingsLoop			;10  F9

SetSample:
;Set up the pointer to custom samples
mov $14, #$30		;8F 30 14
mov $15, #$01		;8F 01 15
mov y, #$06			;8D  06
mov a,x				;7D  
lsr a				;5C  
mul ya			;CF  
db $7A,$14		;adc ya,$14		;7A  14
db $DA,$14		;mov $14, ya		;DA  14
ret 			;6F 

ReturnSample:
;Fixes the source number after a sound effect is done playing
call SetSample
jmp $0D61

SetupSample:
;Starts the sample setting
;These are times when it will do tremelo instead of setting the sample, since they are the same command
cmp $06,#$20		;68  20
bcc Tremelo			;B0  03
call $125C
bpl Tremelo
and a,#$7F		;48  80
mov $E0,a		;C4  E0
mov a,#$04
push a
mov a,#$00		;E8  00
push a			;2D  
call $125C
mov $E1,a
call SetSample	;3F 30 04
;Set source number
pop y			;EE  
mov a,$E0		;E4  E0
db $D7,$14      ;mov [$14]+y,a
;Set Gain
pop y
mov a,$E1
db $D7,$14      ;mov [$14]+y,a
;This does all the necessary "Boiler plate" to change the source number
jmp $0D5B		;5F 5B 0D

Tremelo:
;Return to the Tremelo command
jmp $0E3E

InstrumentHack:
;This is stolen right from carol's more.bin. I only have a rudimentary understanding of what it does
call $125C		;3F 5C 12
bmi Notinc			;30  01
inc a			;BC  
Notinc:
mov $C1+x,a		;D4  C1
SampleHack:
mov x,$46		;F8  46
;I used to have to spread all this stuff out due to space constraints. Not a problem now, but it's already here.
jmp DoInstrumentStuff

CheckPercussion:
;This is stolen right from carol's more.bin. I only have a rudimentary understanding of what it does
push a			;2D  
cmp a,#$D0		;68  D0
bcs ItWasPercussion			;B0  05
mov a,$48		;E4  48
jmp $0CAB		;5F AB 0C
ItWasPercussion:
pop a			;AE  
jmp $0CB0		;5F B0 0C

;Little table that holds the detune values for the 7 different channels.
Offsets:
db $00,$00,$00,$00,$00,$00,$00,$00

PitchBendStuff:
;This is stolen right from carol's more.bin. I only have a rudimentary understanding of what it does
mov a,$90+x		;F4  90
beq NoPitch		;F0  04
mov a,$02B0+x	;F5 B0 02
ret 			;6F  
NoPitch:
mov a,$02D1+x	;F5 D1 02
mov $02B0+x,a	;D5 B0 02
ret 			;6F  

GlobalTuningStuff:
;This is stolen right from carol's more.bin. I only have a rudimentary understanding of what it does
call $125C		;3F 5C 12
cmp a,#$AA		;68  AA
beq AAVol		;F0  03
mov $43,a		;C4  43
ret 			;6F 
AAVol:
mov a,#$01		;E8  01
mov $10A8,a		;C5 A8 10
ret 			;6F  

OtherPitchBendStuff:
;This is stolen right from carol's more.bin. I have almost no understanding of what it does
push a			;2D  
mov a,#$02		;E8  02
mov $10A8,a		;C5 A8 10
mov $0C,a		;C4  0C
pop a			;AE  
ret 			;6F  

Hijack1DF9:
;This lets you use the code insertion command within 1DF9 sound effects
cmp a,#$ED			;\Check if the command is $ED. If not, it's not a code insertion command
bne notcode1DF9		;/
mov a,#$18			;\Since the code insertion routine for 1DF9 and 1DFC are the same
mov SEInsert1,a		;|except 1DF9 uses $18 as a pointer and 1DFC uses $1A as a pointer,
mov SEInsert2,a		;|and all code is in RAM, I figured this would be a good way to save space
mov SEInsert3,a		;|
mov SEInsert4,a		;|
mov SEInsert5,a		;|
mov SEInsert6,a		;|
mov SEInsert7,a		;|
mov SEInsert8,a		;|
mov SEInsert9,a		;|
mov SEInsertA,a		;/
call InsertCodeSE	;The shared subroutine that writes the code and jumps to it
jmp $0752			;Return basically
notcode1DF9:
mov x, #$08			;\restore code
call $05C5			;/
jmp $079B			;Return

Hijack1DFC:
;This lets you use the code insertion command within 1DF9 sound effects
cmp a,#$ED
bne notcode1DFC		;\Check if the command is $ED. If not, it's not a code insertion command
mov a,#$1A			;/
mov SEInsert1,a		;|except 1DF9 uses $18 as a pointer and 1DFC uses $1A as a pointer,
mov SEInsert2,a		;|and all code is in RAM, I figured this would be a good way to save space
mov SEInsert3,a		;|
mov SEInsert4,a		;|
mov SEInsert5,a		;|
mov SEInsert6,a		;|
mov SEInsert7,a		;|
mov SEInsert8,a		;|
mov SEInsert9,a		;|
mov SEInsertA,a		;/
call InsertCodeSE	;The shared subroutine that writes the code and jumps to it
jmp $087B			;Return
notcode1DFC:
cmp a,#$FF			;\Restore code
bne NotFF1DFC		;|
jmp $08BF			;/Return
NotFF1DFC:			
jmp $08C3			;Return



SharedCodeStartSE:
						;Grab a byte
                        db $3A
						SEInsert1:
						db $18				;incw $18/1A, will change depending on where you come from
						db $E7
						SEInsert2:
						db $18				;mov a,[$18/1A+x], will change depending on where you come from
						;Put it in $17
                        mov $17,a
						;Grab a byte
                        db $3A
						SEInsert3:
						db $18				;incw $18/1A, will change depending on where you come from
						db $E7
						SEInsert4:
						db $18				;mov a,[$18/1A+x], will change depending on where you come from
						;Put it in $16. This sets the number of bytes to upload.
                        mov $16,a
DataInsertLoopSE:
						;Get a byte
                        db $3A
						SEInsert5:
						db $18				;incw $18/1A, will change depending on where you come from
						db $E7
						SEInsert6:
						db $18				;mov a,[$18/1A+x], will change depending on where you come from
						;Put it at the next ARAM position
                        db $C7                  ;mov [$14+x],a
                        db $14                  ;SPCAS won't assemble this...
						
						;Increase the pointer for where to write data
                        inc $14
                        bne NotOVDataPosSE
						;If it overflows, increase the high byte
                        inc $15
NotOVDataPosSE:
						;And decrease the number of bytes left
                        dec $16
                        cmp $16,#$FF
                        bne NotOVDataBytesSE
						;If it overflows, decrease the high byte as well
                        dec $17
                        cmp $17,#$FF
						;If that overflows, the loop is done
                        beq EndDataLoopSE
NotOVDataBytesSE:
						;Back to the beginning
                        bra DataInsertLoopSE
EndDataLoopSE:
						;We're done uploading stuff, so return
                        ret

InsertCodeSE:
						;Get a byte
                        db $3A
						SEInsert7:
						db $18				;incw $18/1A, will change depending on where you come from
						db $E7
						SEInsert8:
						db $18				;mov a,[$18/1A+x], will change depending on where you come from
						;Put it into the pointer for data write that get's changed and the pointer for the jump
                        mov $15,a
                        mov $11,a
						;Get a byte
                        db $3A
						SEInsert9:
						db $18				;incw $18/1A, will change depending on where you come from
						db $E7
						SEInsertA:
						db $18				;mov a,[$18/1A+x], will change depending on where you come from
						;Put it into the pointer for data write that get's changed and the pointer for the jump
                        mov $14,a
                        mov $10,a
						;Write all the data
                        call SharedCodeStartSE
						;Then jump to the code (There is no jmp [$xxxx], so I need to 0 out x first)
                        mov x,#$00
                        db $1F,$10,$00          ;jmp [$0010+x], SPCAS won't assemble this

end2:

;Hijack to return source number after a sound effect
dw end5-start5
dw $0A64
base $0A64
start5:
jmp ReturnSample
end5:

;Hijack to return source number after a sound effect
dw end6-start6
dw $0749
base $0749
start6:
jmp ReturnSample
end6:

;Hijack to return source number after a sound effect
dw end7-start7
dw $0872
base $0872
start7:
jmp ReturnSample
end7:

;Hijack to return source number after a sound effect
dw end8-start8
dw $0B19
base $0B19
start8:
jmp ReturnSample
end8:


;Hijack for setting the source number. $E5 that is.
dw end9-start9
dw $0E3B
base $0E3B
start9:
jmp SetupSample
end9:


;Fix the normal instrument loading to work with $E5
dw endA-startA
dw $0D47
base $0D47
startA:
jmp InstrumentHack
endA:





;Fix some percussion stuff
dw endD-startD
dw $0CA8
base $0CA8
startD:
jmp CheckPercussion
endD:

;X is no longer the channel number here, so we gotta call the one that gets the channel number as well as the next byte
dw endE-startE
dw $0C57
base $0C57
startE:
call $125C
endE:

;I dunno, something with pitch bends
dw endF-startF
dw $062F
base $062F
startF:
call PitchBendStuff
endF:


;I dunno, something with pitch bends
dw end10-start10
dw $09DA
base $09DA
start10:
call PitchBendStuff
end10:

;I dunno, something with pitch bends
dw end15-start15
dw $112E
base $112E
start15:
call PitchBendStuff
end15:


;I dunno, something with pitch bends
dw end16-start16
dw $11CC
base $11CC
start16:
call PitchBendStuff
end16:


;I dunno, something else with pitch bends
dw end11-start11
dw $0B5C
base $0B5C
start11:
call OtherPitchBendStuff
end11:


;I dunno, something with global tuning. Not sure why this is necessary.
dw end12-start12
dw $0E35
base $0E35
start12:
call GlobalTuningStuff
end12:


;Changing the tremelo command because carol's does
dw end13-start13
dw $0E44
base $0E44
start13:
mov $01F1+x,a
end13:


;Changing the tremelo command because carol's does
dw end17-start17
dw $1231
base $1231
start17:
mov $01F1+x,a
end17:


;Changing the tremelo command because carol's does
dw end14-start14
dw $1006
base $1006
start14:
adc a,$01F1+x
end14:


;Allow for a different command in 1DF9 sound effects
dw end19-start19
dw $0796
base $0796
start19:
jmp Hijack1DF9
end19:


;Allow for a different command in 1DFC sound effects
dw end18-start18
dw $08BB
base $08BB
start18:
jmp Hijack1DFC
end18:


dw end1-start1
dw $0400
base $0400
start1:
FunctionPointers:
dw DirectDSPWrite,SetReTune,InsertData,InsertCode

CommandsInit:
                        call $125C              ;3F 5C 12       (GetNextData)
						bmi HFDFuncs			;If it's negative, it's a subcommand, otherwise, it's ADSR
ADSR:	
						mov $10,a				;Back up a, it's about to get overwritten
						mov a,$46				;Channel number << 3
						db $9F                  ;xcn    SPCAS won't essemble this
						lsr a
						or a,#$06				;Setting DSP register $Channelnum6
						push a					;\Set up the registers to write to
						dec a					;|
						push a					;/
						mov a,$10				;Get back the first ADSR byte
						or a,#$80				;Set bit 7 so it will enable ADSR
						pop y					;\Pop out the register to write to
						call $0697				;/and write to DSP
						call $125C				;Get the next ADSR byte
						pop y					;Get the register to write to
						call $0697				;And write to DSP
						ret						;Then return
		
HFDFuncs:
						and a,#$7F				;\Get rid of th ehigh bit
						asl a					;|It's a word table, so we need to multiply by 2
                        mov x,a					;/and that is our index
												;Jump to the function
                        db $1F                  ;jmp [FunctionPointers+x]
                        dw FunctionPointers     ;SPCAS won't assemble this

DirectDSPWrite:
                        mov a,$46               ;E4 46
                        db $9F                  ;xcn    SPCAS won't essemble this
                        lsr a                   ;5C
                        push a                  ;2D
                        call $125C              ;3F 5C 12
                        push a                  ;2D
                        and a,#$0F              ;28 0F
                        cmp a,#$0A              ;68 0A
												;If the register to write to is a channel specific one
												;write it to the currecnt channel being processed
                        bcc Channelspecific     ;90 (0A/0E)
                        pop a                   ;AE
                        mov y,a                 ;FD
                        pop a                   ;AE
						push y
                        call $125C              ;3F 5E 12
						pop y
                        call $0697              ;3F 97 06
                        ret                     ;6F

Channelspecific:
                        pop a                   ;AE
                        and a,#$0F              ;28 0F
                        mov $10,a               ;C4 10
                        pop a                   ;AE
                        or a,$10                ;04 10
						push a
                        call $125C              ;3F 5C 12
						pop y
                        call $0697              ;3F 97 06
                        ret                     ;6F

SetReTune:
						;Get's the value to return to
                        call $125C              ;3F 5C 12
SetReTuneKnownVal:
                        push a                  ;2D
                        mov a,$46               ;E4 46
                        lsr a                   ;5C
                        mov x,a                 ;5D
                        pop a                   ;AE
                        mov Offsets+x,a         ;D5 xx xx
                        ret                     ;6F

						
											
InsertData:				;I basically comment this all above, it's jsut in a slightly different
						;Form. Read the comments there...						
                        call $125C
                        mov $15,a
                        call $125C
                        mov $14,a
SharedCodeStart:
                        call $125C
                        mov $17,a
                        call $125C
                        mov $16,a
DataInsertLoop:
                        call $125C
                        mov x,#$00
                        db $C7                  ;mov [$14+x],a
                        db $14                  ;SPCAS won't assemble this...

                        inc $14
                        bne NotOVDataPos
                        inc $15
NotOVDataPos:
                        dec $16
                        cmp $16,#$FF
                        bne NotOVDataBytes
                        dec $17
                        cmp $17,#$FF
                        beq EndDataLoop
NotOVDataBytes:
                        bra DataInsertLoop
EndDataLoop:
                        ret

InsertCode:
                        call $125C
                        mov $15,a
                        mov $11,a
                        call $125C
                        mov $14,a
                        mov $10,a
                        call SharedCodeStart
                        mov x,#$00
                        db $1F,$10,$00          ;jmp [$0010+x], SPCAS won't assemble this

EchoFix:
						mov a,#$68				;\Move the echo buffer to $6800
						mov y,#$6D				;|This fixes an OW bug
						call $0697				;/
						mov a,#$00				;\Disable echo
						mov y,#$4D				;|
						call $0697				;/
						mov a,#$00				;\Make the delay size 0
						mov y,#$7D				;|
						call $0697				;/
						mov a,#$00
						mov x,#$07
						
EmptyDetune:
						mov Offsets+x,a			;\Clear out the detune values
						dec x					;|
						bpl EmptyDetune			;/
						mov a,#$BB				;\Restore code. Tells the snes the SPC is ready for data
						mov $00F5,a				;/
						ret

DoReTune:
						;Adds the return value for the currect channel to the current note
                        push x                  ;4D
                        push a                  ;2D
                        mov a,$46               ;E4 46
                        lsr a                   ;5C
                        mov x,a                 ;5D
                        pop a                   ;AE
                        clrc                    ;60
                        adc a,$43               ;84
                        adc a,Offsets+x         ;95 xx xx
                        pop x                   ;CE
                        ret                     ;6F
	
DoInstrumentStuff:	
call SetInstrumentStandard		;3F 00 04
call SetSample		;3F 30 04
call SetInstrument		;3F 23 04
jmp $0D5B		;5F 5B 0D

end1:

;Hijack for h tune command
dw endC-startC
dw $05E5
base $05E5
startC:
call DoReTune
endC:

;Hijack $ED command, usually empty
dw end3-start3
dw $0FB6
base $0FB6
start3:
dw CommandsInit
end3:


;Fix the echo whenever something is uploaded to the SPC
dw end4-start4
dw $12F7
base $12F7
start4:
call EchoFix
nop
nop
end4:
