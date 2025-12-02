;New SPC command added: Store #$FD to $1DFB to immediately stop the track playing on the SPC. Used for SPC->MSU transitions.
	;;SA-1 define
!addr = !SA1Addr2
;;Free RAM
!MIR_TRACK	 = $0F3A|!addr
!MIR_VOLUME	 = $0F3C|!addr
;!MIR_STATUS	 = $0F3D|!addr
;;MSU-1 defines
;Read
!MSU_STATUS 	= $2000
!MSU_READ 		= $2001
!MSU_ID			= $2002

;Write
!MSU_TRACK		= $2004
!MSU_VOLUME		= $2006
!MSU_CONTROL	= $2007

HandleMSU:
	
	LDA !MSU_ID
	CMP #$53
	BEQ +
	RTS
	+		;If no MSU, return.
    
	LDA !SpecialMir
	CMP #$07
	BEQ .Pause
	CMP #$08
	BEQ .Unpause	
	BRA .Continue
	
	.Pause
	STZ !MSU_CONTROL
	BRA .Continue
	
	.Unpause
	LDA #$03
	STA !MSU_CONTROL
	
.Continue
	LDA !MusicMir
	BEQ .Return ;If track = 0, we don't need to do anything.
	CMP #$FD
	BEQ .Return
	

	
	LDA !MusicMir	;If track > # tracks, fade.
	CMP #!SongCount
	BCC .NoFadeMSU
	LDA #$FF
	STA !MusicMir
	LDA !MIR_VOLUME
	BEQ .Return
	SEC : SBC #!MSU1FADE_SPEED
	BCS +
	LDA #$00
	+
	STA !MIR_VOLUME
	STA !MSU_VOLUME
	RTS
	.NoFadeMSU
	
	
	LDA !MusicMir ;If track is already playing, we don't need to do anything
	CMP !MIR_TRACK
	BEQ .Return
	STA !MIR_TRACK
	
	STZ !MSU_CONTROL ;Set the volume
	LDA #!MSU1DEF_VOLUME
	STA !MSU_VOLUME
	STA !MIR_VOLUME
	
	LDA !MIR_TRACK ;Set the track
	STA !MSU_TRACK
	STZ !MSU_TRACK+1
	
	-
	BIT $2000 : BVS - ;Wait for MSU-1 to seek.
	
	LDX #$03 ;Determine loop status.
	CMP #$0A
	BCS +
	CMP #$05
	BEQ +
    CMP #$06
    BEQ +
	LDX #$01
	+
	STX !MSU_CONTROL
	
	LDA !MSU_STATUS ;If the track is not missing...
	AND #$08
	BEQ .PlaySuccess
	
	STZ !MSU_VOLUME ;If the track is missing, kill MSU and allow SPC to play.
	STZ !MSU_CONTROL
	STZ !MIR_TRACK
	
	.Return
	RTS
	
	.PlaySuccess ;Kill SPC.
	LDA !MusicMir
	CMP !PSwitch
	BEQ +
	CMP !Starman
	BEQ +
	STA !MusicBackup
	+
	STA !CurrentSong
	LDA #$FD
	STA !MusicMir
	RTS


