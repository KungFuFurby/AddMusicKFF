#define __STDC_WANT_LIB_EXT1__ 1
#include "AddMusic.h"
#include <cctype>
#include <cstring>		// // //
#include <cstdlib>		// // //

#ifdef _WIN32		// // //
#define _CRT_SECURE_NO_WARNINGS
#endif

inline int macro_SNEStoPC(int a) {
	return ((((a & 0x7F0000) / 2) | (a & 0x7FFF)) + 0x200);
}

//these really change nothing...
//I added them to shut up the compiler warnings
//do to the original code being poorly validated.
//I could force a termination here and probably should,
//but that would possibly change the functioning of addmusic.

void fread_safe(void *dest, size_t size, FILE *file) {		// // //
	if (::fread(dest, 1, size, file)) {
		return;
	}
}

void fwrite_safe(const void *src, size_t size, FILE *file) {		// // //
	if (::fwrite(src, 1, size, file)) {
		return;
	}
}

bool fopen_safe(FILE **file, const char *str, const char *mode) {
#ifdef _WIN32
	if (!::fopen_s(file, str, mode)) {		// // //
		return true;
	}
	return false;
#else
	if (FILE *f = ::fopen(str, mode)) {		// // //
		*file = f;
		return true;
	}
	return false;
#endif
}

/***************************************************************
Remove
***************************************************************/
int AddMusic::Remove() {
	bool b_deleted = false;

	if (OWMusicAddress == 0xF0208) {	//it is possible that the ROM has the data by AddMusic 4.0, by Carol
		unsigned int tmp;
		fseek(ROMfp, 0xF0208, SEEK_SET);
		fread_safe(&tmp, 4, ROMfp);
		if (tmp == 0x136041FD && !memcmp(&ROM[0xF0200], "STAR", 4) && *(unsigned int*)&ROM[0xF0204] == 0xBDFB4204) {
			memset(&ROM[0xF0200], 0, 0x4205);		// // //
			fseek(ROMfp, 0xF0200, SEEK_SET);
			fwrite_safe(&ROM[0xF0200], 0x4205, ROMfp);

			const unsigned int OldAMptr[4] = {0xA0200, 0x120200, 0x1A0200, 0x220200};
			char Tag[0xC];
			for (int i = 0; i < 4; i++) {
				unsigned int address = OldAMptr[i];
				for (int j = 0; j < 8; j++, address += 0x2000) {
					fseek(ROMfp, address, SEEK_SET);
					fread_safe(Tag, 0xC, ROMfp);
					if (strncmp(Tag, "STAR", 4) || *(unsigned int*)&Tag[4] != 0xE7FF1800 || *(unsigned int*)&Tag[8] != 0x680017F0)
						continue;
					memset(ROM + address, 0, 0x17F2);		// // //
					fseek(ROMfp, address, SEEK_SET);
					fwrite_safe(ROM + address, 0x17F2, ROMfp);
				}
			}
			printf("Removed data by old AddMusic in java\n");
			b_deleted = true;
		}
	}

	if (!b_deleted) {
		puts("[OVERWORLD]");	//tries to erase OW music
		unsigned int ErasedSize;
		unsigned int code_version = 0xFFFFFFFF;

		if (ROM[0x1940] == 0x22) {
			TableAddress = *(unsigned int*)&ROM[0x1941] & 0xFFFFFF;
			TableAddress = macro_SNEStoPC(TableAddress);
			fseek(ROMfp, TableAddress - 0x185, SEEK_SET);
			if (memcmp(&ROM[TableAddress - 0x185], "@HFD", 4)) {
				fseek(ROMfp, TableAddress - 0x181, SEEK_SET);
				code_version = fgetc(ROMfp);
				if (code_version >= 0x04) {
					fseek(ROMfp, TableAddress + 0x158, SEEK_SET);
					OWMusicAddress = fgetc(ROMfp);
					fseek(ROMfp, TableAddress + 0x15C, SEEK_SET);
					OWMusicAddress |= fgetc(ROMfp) << 8;
					fseek(ROMfp, TableAddress + 0x160, SEEK_SET);
					OWMusicAddress |= fgetc(ROMfp) << 16;
					OWMusicAddress = macro_SNEStoPC(OWMusicAddress);
					ErasedSize = RemoveRATS(OWMusicAddress);
				}
				else ErasedSize = RemoveRATS(OWMusicAddress);
			}
			else {
				if (!memcmp(&ROM[TableAddress - 0x69], "STAR", 4)) {
					fseek(ROMfp, TableAddress - 0x61, SEEK_SET);
					code_version = fgetc(ROMfp);
					if (code_version >= 0x04) {
						fseek(ROMfp, TableAddress + 0x158, SEEK_SET);
						OWMusicAddress = fgetc(ROMfp);
						fseek(ROMfp, TableAddress + 0x15C, SEEK_SET);
						OWMusicAddress |= fgetc(ROMfp) << 8;
						fseek(ROMfp, TableAddress + 0x160, SEEK_SET);
						OWMusicAddress |= fgetc(ROMfp) << 16;
						OWMusicAddress = macro_SNEStoPC(OWMusicAddress);
						ErasedSize = RemoveRATS(OWMusicAddress);
					}
					else ErasedSize = RemoveRATS(OWMusicAddress);
				}
				else ErasedSize = RemoveRATS(OWMusicAddress);
			}
		}
		else ErasedSize = RemoveRATS(OWMusicAddress);
		if (ErasedSize) {
			printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, OWMusicAddress - 8);
		}
		if (ROM[0x1940] == 0x22) {	//tries to erase level music

			puts("[LEVEL]");
			TableAddress = *(unsigned int*)&ROM[0x1941] & 0xFFFFFF;
			TableAddress = macro_SNEStoPC(TableAddress);
			fseek(ROMfp, TableAddress - 0x185, SEEK_SET);
			int tempread;
			fread_safe(&tempread, 4, ROMfp);
			if (!memcmp(&tempread, "@HFD", 4)) {
				RemoveLevelMusic(TableAddress - 0x180, 0x80, 0x180);
				fseek(ROMfp, TableAddress - 0x181, SEEK_SET);
				switch (fgetc(ROMfp)) {
				case 0x05:
					RemoveMiscMusic(TableAddress + 0x47, TableAddress + 0x48, TableAddress + 0x4E);
					printf("Removed old data by unofficial AddMusic 4.05 (current version)\n");
					break;
				default:
					printf("Version not recognized");
				}
				unsigned int ErasedSize = RemoveRATS(TableAddress - 0x185);
				memset(&ROM[TableAddress - 0x18D], 0, ErasedSize);		// // //
			}
			else {
				RemoveLevelMusic(TableAddress - 0x60, 0x20, 0x60);
				puts("[MISC]");
				fseek(ROMfp, TableAddress - 0x68, SEEK_SET);
				fread_safe(&tempread, 4, ROMfp);
				if (memcmp(&tempread, "STAR", 4)) {	//whether or not the main code was by my first unofficial AM (4.0+1)
					fseek(ROMfp, TableAddress - 0x61, SEEK_SET);
					switch (fgetc(ROMfp)) {
					case 0x01:
						RemoveMiscMusic(TableAddress + 0x61, TableAddress + 0x62, TableAddress + 0x47);
						printf("Removed old data by unofficial AddMusic 4.02\n");
						// CreateNewINI(0x01);
						break;
					case 0x02:
						RemoveMiscMusic(TableAddress + 0x38, TableAddress + 0x39, TableAddress + 0x3F);
						printf("Removed old data by unofficial AddMusic 4.03\n");
						// CreateNewINI(0x02);
						break;
					case 0x03:
						RemoveMiscMusic(TableAddress + 0x38, TableAddress + 0x39, TableAddress + 0x3F);
						printf("Removed old data by unofficial AddMusic 4.03+\n");
						//to original SMW code (activating Bonus Game)
						fseek(ROMfp, 0x1165, SEEK_SET); fwrite_safe("\xA9\xFF\x8D\x25\x14", 5, ROMfp);
						//to original SMW code (init code?)
						fseek(ROMfp, 0x23C7, SEEK_SET); fwrite_safe("\x22\x41\x82\x04", 4, ROMfp);
						unsigned char music[7];
						fseek(ROMfp, 0x20F8A, SEEK_SET); fread_safe(music, 7, ROMfp);
						for (int i = 0; i < 7; i++) if (music[i] >= 7) music[i] += 3;
						fseek(ROMfp, 0x20F8A, SEEK_SET); fwrite_safe(music, 7, ROMfp);
						fseek(ROMfp, 0x25DC8, SEEK_SET); fwrite_safe(music, 7, ROMfp);
						// CreateNewINI(0x03);
						break;
					case 0x04:
						RemoveMiscMusic(TableAddress + 0x47, TableAddress + 0x48, TableAddress + 0x4E);
						printf("Removed old data by unofficial AddMusic 4.04\n");
						break;
					}
					unsigned int ErasedSize = RemoveRATS(TableAddress - 0x61);
					memset(&ROM[TableAddress - 0x69], 0, ErasedSize);		// // //
				}
				else {
					//remove the main code which didn't have version number
					unsigned int ErasedSize = RemoveRATS(TableAddress - 0x60);
					memset(&ROM[TableAddress - 0x68], 0, ErasedSize);		// // //
					printf("Removed old data by unofficial AddMusic 4.01\n");
					// CreateNewINI(0x00);
				}
			}

		}
	}

	//to original OW music pointer
	//fseek(ROMfp, 0x30F, SEEK_SET); fputc(0xB1, ROMfp);
	//fseek(ROMfp, 0x314, SEEK_SET); fputc(0x98, ROMfp);
	//fseek(ROMfp, 0x319, SEEK_SET); fputc(0x0E, ROMfp);

/*
	//to original SMW code (SPC uploading)
	fseek(ROMfp, 0x27C, SEEK_SET); fwrite_safe("\xA0\x00\x00\xA9\xAA\xBB", 6, ROMfp);
	fseek(ROMfp, 0x28D, SEEK_SET); fwrite_safe("\xB7\x00\xC8\xEB", 4, ROMfp);
	fseek(ROMfp, 0x296, SEEK_SET); fwrite_safe("\xB7\x00\xC8\xEB", 4, ROMfp);
	fseek(ROMfp, 0x2B4, SEEK_SET); fwrite_safe("\xC2\x20\xB7\x00\xC8\xC8\xAA\xB7\x00\xC8\xC8\x8D\x42\x21\xE2\x20", 0x10, ROMfp);
*/
/*
	//to original SMW code (activating Bonus Game)
	fseek(ROMfp, 0x1165, SEEK_SET); fwrite_safe("\xA9\xFF\x8D\x25\x14", 5, ROMfp);
*/
	//to original SMW code (uploading level music)
//	fseek(ROMfp, 0x1902, SEEK_SET); fwrite_safe("\x20\x34\x81", 3, ROMfp);
//
//	//to original SMW code (brightness, and its direction)
//	fseek(ROMfp, 0x193B, SEEK_SET); fwrite_safe("\x29\xBF\x8D\xDA\x0D\x9C\xAE\x0D\x9C\xAF\x0D", 0xB, ROMfp);
///*
//	//to original SMW code (init code?)
//	fseek(ROMfp, 0x23C7, SEEK_SET); fwrite_safe("\x22\x41\x82\x04", 4, ROMfp);
//*/
//	//to original SMW code (related to P-switch and/or Star Music)
//	fseek(ROMfp, 0x284A, SEEK_SET); fwrite_safe("\xAD\xDA\x0D\x29\x7F\x09\x40\x8D\xDA\x0D", 0xA, ROMfp);
//
//	//to original SMW code (related to music when loading level)
//	fseek(ROMfp, 0x2875F, SEEK_SET); fwrite_safe("\xD0", 1, ROMfp);

	puts("");
	return 0;
}

int AddMusic::RemoveLevelMusic(unsigned int p, int musicnum, int offset) {
	for (int i = 0; i < musicnum; i++) {	// erases old data
		unsigned int address;
		fseek(ROMfp, p + i * 3, SEEK_SET); fread_safe((char *)&address, 3, ROMfp);		// // //
		address &= 0xFFFFFF;
		address = macro_SNEStoPC(address);
		if (0x80200 <= address && address < ROMsize) {
			unsigned int ErasedSize = RemoveRATS(address);
			printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, address - 8);
			address = 0;
			fseek(ROMfp, TableAddress - offset + i * 3, SEEK_SET);
			fwrite_safe(&address, 3, ROMfp);
		}
	}

	return 0;
}

int AddMusic::RemoveMiscMusic(unsigned int low, unsigned int middle, unsigned int high) {
	unsigned int address;
	fseek(ROMfp, low, SEEK_SET);
	address = fgetc(ROMfp);
	fseek(ROMfp, middle, SEEK_SET);
	address |= (fgetc(ROMfp) | 0x80) << 8;
	fseek(ROMfp, high, SEEK_SET);
	address |= (fgetc(ROMfp)) << 16;
	address = macro_SNEStoPC(address);
	unsigned int ErasedSize = RemoveRATS(address);
	printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, address - 8);
	return 0;
}

/***************************************************************
initialize
***************************************************************/
int AddMusic::initialize(void) {
	try {
		//For Overworld
		OWMusicAddress = (unsigned char)ROM[0x30F] | (unsigned char)ROM[0x314] << 8 | (unsigned char)ROM[0x319] << 16;

		OWMusicAddress = macro_SNEStoPC(OWMusicAddress);	//to PC address
		TableAddress = *(unsigned int*)&ROM[0x1941] & 0xFFFFFF;	//TableAddress, as main code loc
		TableAddress = macro_SNEStoPC(TableAddress);

		if (OWMusicAddress != 0x071AB1 || ROM[0x1940] == 0x22) Remove();
	}
	catch (const char *str) {
		puts(str);
		return 1;
	}

	return 0;
}

/***************************************************************
helper
***************************************************************/

unsigned int AddMusic::RemoveRATS(unsigned int address) {
	if (address < 0x80200 || ROMsize <= address) return 0;
	if (!memcmp(&ROM[address - 8], "STAR", 4) && (*(unsigned short*)&ROM[address - 4] + *(unsigned short*)&ROM[address - 2]) == 0xFFFF) {
		unsigned int EraseSize = *(unsigned short*)&ROM[address - 4] + 9;
		memset(&ROM[address - 8], 0, EraseSize);		// // //
		fseek(ROMfp, address - 8, SEEK_SET);
		fwrite_safe(&ROM[address - 8], EraseSize, ROMfp);
		return EraseSize;
	}

	return 0;
}

/***************************************************************
main
***************************************************************/

// Like normal AM405, but it only removes data, and then saves the ROM.
// This is...to ensure maximal compatibility.... Yeah.

int removeAM405Data(int argc, char *argv[]) {
	bool header = 1;		// // //

	FILE *ROMFile = NULL;
	if (!fopen_safe(&ROMFile, argv[1], "rb+")) {
		printf("Couldn't open the ROM file\n");
		return 1;
	}
	fseek(ROMFile, 0, SEEK_END);
	unsigned int ROMSize = ftell(ROMFile);
	fseek(ROMFile, 0, SEEK_SET);
	if ((ROMSize % 0x8000) != 0x200) {
		header = 0;
		ROMSize += 0x200;
	}
	unsigned char *ROMBuf = (unsigned char *)malloc(ROMSize);
	if (!ROMBuf) {
		printf("Memory allocation failed\n");
		fclose(ROMFile);
		return 1;
	}

	if (!header) {
		fread_safe(ROMBuf + 0x200, ROMSize - 0x200, ROMFile);
		memset(ROMBuf, 0, 0x200);		// // //
		fseek(ROMFile, 0, SEEK_SET);
		fwrite_safe(ROMBuf, ROMSize, ROMFile);
	}
	else {
		fread_safe(ROMBuf, ROMSize, ROMFile);
	}

	if (ROMSize <= 0x80200) {
		printf("The ROM must be expanded\n");
		fclose(ROMFile);
		free(ROMBuf);
		return 1;
	}

	AddMusic AM(ROMFile, ROMBuf, ROMSize);

	AM.initialize();		// // //

	if (!header) {
		fseek(ROMFile, 0x200, SEEK_SET);
		fread_safe(ROMBuf, ROMSize - 0x200, ROMFile);
		fseek(ROMFile, 0, SEEK_SET);
		fwrite_safe(ROMBuf, ROMSize - 0x200, ROMFile);
	}
	fclose(ROMFile);
	free(ROMBuf);
	return 0;
}

