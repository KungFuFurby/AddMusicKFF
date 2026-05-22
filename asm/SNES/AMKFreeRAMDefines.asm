;AddmusicK FreeRAM Defines
;For AddmusicK 1.0.12 Version X Fork (as of 05/21/2026)

!FreeRAM		= $7FB000
!CurrentSong		= !FreeRAM+$00
!NoUploadSamples	= !FreeRAM+$02
!CurrentSongGroup	= !FreeRAM+$03
!SongPositionLow	= !FreeRAM+$05
!SongPositionHigh	= !FreeRAM+$06
!SPCOutput1		= !SongPositionLow
!SPCOutput2		= !SongPositionHigh
!SPCOutput3		= !FreeRAM+$07
!SPCOutput4		= !FreeRAM+$08
!Trick			= !FreeRAM+$09
!SampleCount		= !FreeRAM+$0A
!SRCNTableBuffer	= !FreeRAM+$0B