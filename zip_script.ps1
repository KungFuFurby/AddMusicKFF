move src\Release\AddmusicK.exe	AddmusicKVS2019.exe
move src\Release\AM4Batch.exe	AM4Batch.exe
move src\Release\AMKGUI.exe		AMKGUI.exe
move src\Release\AMMBatch.exe	AMMBatch.exe
dir * -Include *.gitkeep -Recurse | del -Force
del src\AddmusicK\Release -Recurse
del src\AM4Batch\Release -Recurse
del src\AMMBatch\Release -Recurse
del src\AM405Remover\Release -Recurse
del src\AMKGUI\obj -Recurse
del src\Release -Recurse
7z a src.zip src
mkdir AddmusicKFF
move 1DF9, 1DFC, asm, music, readme_files, samples, SPCs, stats, Visualizations, Addmusic_list.txt, "Addmusic_sample groups.txt", "Addmusic_sound effects.txt", addmusicMRemover.pl, readme.html, src.zip, *.exe, *.dll, Makefile AddmusicKFF
7z a AMKFF_latest.zip AddmusicKFF