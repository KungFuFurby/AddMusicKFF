move src\Release\AddmusicK.exe	AddmusicK.exe
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
mkdir "AddmusicK_1.0.11"
move 1DF9, 1DFC, asm, music, readme_files, samples, SPCs, stats, Visualizations, Addmusic_list.txt, "Addmusic_sample groups.txt", "Addmusic_sound effects.txt", addmusicMRemover.pl, readme.html, src.zip, *.exe, *.dll, Makefile "AddmusicK_1.0.11"
7z a "AddmusicK1.0.11_latestRC.zip" "AddmusicK_1.0.11"