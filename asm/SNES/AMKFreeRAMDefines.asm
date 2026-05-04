;AddmusicK FreeRAM Defines
;For AddmusicKFF Fork (as of 9/19/2024)

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