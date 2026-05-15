;AddmusicK FreeRAM Defines
;For AddmusicK 1.0.9

!FreeRAM		= $7FB000
!CurrentSong		= !FreeRAM+$00
!NoUploadSamples	= !FreeRAM+$01
!SongPositionLow	= !FreeRAM+$04
!SongPositionHigh	= !FreeRAM+$05
!SPCOutput1		= !SongPositionLow
!SPCOutput2		= !SongPositionHigh
!SPCOutput3		= !FreeRAM+$06
!SPCOutput4		= !FreeRAM+$07
!Trick			= !FreeRAM+$08
!SampleCount		= !FreeRAM+$09
!SRCNTableBuffer	= !FreeRAM+$0A