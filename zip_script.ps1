# move src\Release\AddmusicK.exe	AddmusicKVB2019.exe
move src\Release\AM4Batch.exe	AM4Batch.exe
move src\Release\AMKGUI.exe		AMKGUI.exe
move src\Release\AMMBatch.exe	AMMBatch.exe
7z a src.zip src
7z a "AddmusicK1.0.9_latestRC.zip" 1DF9 1DFC asm music readme_files samples SPCs stats Addmusic_list.txt "Addmusic_sample groups.txt" "Addmusic_sound effects.txt" addmusicMRemover.pl readme.html src.zip *.exe