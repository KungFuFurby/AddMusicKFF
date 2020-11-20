#include "AddMusic.h"
#include <cctype>

bool header = 1;
char ini_file_path[300];

inline int macro_SNEStoPC(int a)
{
	return ((((a&0x7F0000)/2)|(a&0x7FFF))+0x200);
}
inline int macro_PCtoSNES(int a)
{
	return (((a&0xFF8000)*2|((a&0x7FFF)|0x8000))|0x800000);
}


//this is where most of the hacking happens for functionality that does
//not exist under linux.  Leave it to windows to add a million non-standard
//functions and call them standard.  These ini functions should behave EXACTLY
//as the do in windows.  They are inefficient and crappy as hell, but I don't
//care.  They work, and should work on every platform.
#if !defined(WIN32) || defined(__MINGW32__)
	#include <unistd.h>
	#define GetCurrentDirectory(a, b) getcwd((b), (a))
	#define ZeroMemory(p, sz) memset((p), 0, (sz))
	#define MAX_PATH 256

	char *buffer;
	unsigned int buffer_pos;

	bool open_ini(const char *name)
	{
		FILE *ini_file = fopen(name,"r");
		if(ini_file == NULL){
			return false;
		}
		fseek(ini_file, 0L, SEEK_END);
		unsigned int bytes = ftell(ini_file);
		fseek(ini_file, 0L, SEEK_SET);	
		buffer = (char*)calloc(bytes+1, sizeof(char));	
		 
		if(buffer == NULL){
			return false;
		}
		 
		if(!fread(buffer, sizeof(char), bytes, ini_file)){
			return false;
		}
		buffer[bytes] = '\0';
		fclose(ini_file);
		buffer_pos = 0;
		return true;
	}

	bool get_sec(const char *sec)
	{
		int len = strlen(sec) - 1;
		while(buffer[buffer_pos] != '\0'){
			if(buffer[buffer_pos] == '['){
				buffer_pos++;
				for(int i = 0; i <= len; i++){
					if(buffer[buffer_pos] == sec[i]){
						buffer_pos++;
						if(i == len && buffer[buffer_pos] == ']'){
							buffer_pos++;
							while(isspace(buffer[buffer_pos])){
								buffer_pos++;
							}
							return true;
						}
						continue;
					}
					if(buffer[buffer_pos] == ']'){
						break;
					}
					if(buffer[buffer_pos] != sec[i]){
						while(buffer[buffer_pos] != ']'){
							buffer_pos++;
						}
						break;
					}
					buffer_pos++;
				}
			}
			if(buffer[buffer_pos] != ']'){
				while(isspace(buffer[buffer_pos])){
					buffer_pos++;
				}
				return true;
			}else{
				while(buffer[buffer_pos] != '['
					&& buffer[buffer_pos] != '\0'){
					buffer_pos++;
				}
				buffer_pos--;
			}
			buffer_pos++;
		}
		return false;
	}

	bool get_key(const char *key)
	{
		int len = strlen(key);
		while(buffer[buffer_pos] != '\0'){
			for(int i = 0; i <= len && buffer[buffer_pos] != '\0'; i++){
				if(i == len && buffer[buffer_pos] == '='){
					buffer_pos++;
					return true;
				}
				if(buffer[buffer_pos] != key[i]){
					break;
				}
				buffer_pos++;
			}
			while(buffer[buffer_pos] != '\0' 
				&& buffer[buffer_pos] != '\n'
				&& buffer[buffer_pos] != '\r'){
				buffer_pos++;
			}
			while(isspace(buffer[buffer_pos])){
				buffer_pos++;
			}	
			if(buffer[buffer_pos] == '['){
				return false;
			}
		}
		return false;
	}

	unsigned short GetPrivateProfileString( const char *sec, const char *key,
						const char *def, char *ret,
	  					unsigned short size, const char *name)
	{
		if(!open_ini(name)){
			memcpy(ret, def, strlen(def)+1);
			free(buffer);
			return size;
		}
		if(!get_sec(sec)){
			memcpy(ret, def, strlen(def)+1);
			free(buffer);
			return size;
		}
		if(!get_key(key)){
			memcpy(ret, def, strlen(def)+1);
			free(buffer);
			return size;
		}
		if(buffer[buffer_pos] == '\n' || buffer[buffer_pos] == '\r'){
			memcpy(ret, def, strlen(def)+1);
			free(buffer);
			return size;
		}
		unsigned int s = 0;
		while(buffer[buffer_pos] != '\n' && buffer[buffer_pos] != '\r' 
			&& buffer[buffer_pos] != '\0' && s != size){
			buffer_pos++;
			s++;
		}
		memset(ret, 0, size);
		strncpy(ret,&buffer[buffer_pos-s],s);
		free(buffer);
		return s;
	}
	unsigned int GetPrivateProfileInt(const char *sec, const char *key,
  					  unsigned int def, const char *name)
	{
		unsigned int ret = 0;
		if(!open_ini(name)){
			free(buffer);
			return def;
		}
		if(!get_sec(sec)){
			free(buffer);
			return def;
		}
		if(!get_key(key)){
			free(buffer);
			return def;
		}
		while(isdigit(buffer[buffer_pos])){
			ret = ret * 10 + (buffer[buffer_pos]-48);
			buffer_pos++;
		}
		free(buffer);
		return ret;
	}


#else
#include <Windows.h>
#endif

const int common_num[0x0B]={0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x10, 0x11, 0x13, 0x14, 0x15};

//these really change nothing...
//I added them to shut up the compiler warnings
//do to the original code being poorly validated.
//I could force a termination here and probably should,
//but that would possibly change the functioning of addmusic.

void  fread_safe(void *dest, unsigned int size1, unsigned int size2, FILE *file)
{
	if(fread(dest, size1, size2, file)){
		return;
	}
}

void  fread_safe(const void *dest, unsigned int size1, unsigned int size2, FILE *file)
{
	if(fread(const_cast<void *>(dest), size1, size2, file)){
		return;
	}
}

void fwrite_safe(const void *src, unsigned int size1, unsigned int size2, FILE *file)
{
	if(fwrite(src, size1, size2, file)){
		return;
	}
}

bool fopen_safe(FILE **file, const char *str, const char *mode)
{
	*file = fopen(str, mode);
	if(!*file){
		return false;
	}
	return true;
}

/***************************************************************
Remove
***************************************************************/
int AddMusic::Remove(void)
{
	bool b_deleted=false;

	if(OWMusicAddress == 0xF0208){	//it is possible that the ROM has the data by AddMusic 4.0, by Carol
		unsigned int tmp;
		fseek(ROMfp, 0xF0208, SEEK_SET);
		fread_safe(&tmp, 1, 4, ROMfp);
		if( tmp == 0x136041FD && !memcmp( &ROM[0xF0200], "STAR", 4) && *(unsigned int*)&ROM[0xF0204]==0xBDFB4204){
			ZeroMemory( &ROM[0xF0200], 0x4205);
			fseek(ROMfp, 0xF0200, SEEK_SET);
			fwrite_safe( &ROM[0xF0200], 1, 0x4205, ROMfp);

			const unsigned int OldAMptr[4]={0xA0200, 0x120200, 0x1A0200, 0x220200};
			char Tag[0xC];
			for(int i=0;i<4;i++){
				unsigned int address = OldAMptr[i];
				for(int j=0;j<8;j++, address+=0x2000){
					fseek(ROMfp, address, SEEK_SET);
					fread_safe(Tag, 1, 0xC, ROMfp);
					if( strncmp(Tag, "STAR", 4) || *(unsigned int*)&Tag[4] != 0xE7FF1800 || *(unsigned int*)&Tag[8] != 0x680017F0) 							continue;
					ZeroMemory(ROM+address, 0x17F2);
					fseek(ROMfp, address, SEEK_SET);
					fwrite_safe(ROM+address, 1, 0x17F2, ROMfp);
				}
			}
			printf("Removed data by old AddMusic in java\n");
			b_deleted=true;
		}
	}

	if( !b_deleted ){
		puts("[OVERWORLD]");	//tries to erase OW music
		unsigned int ErasedSize;
		unsigned int code_version=0xFFFFFFFF;

		if(ROM[0x1940]==0x22){
			TableAddress = *(unsigned int*)&ROM[0x1941]&0xFFFFFF;
			TableAddress = macro_SNEStoPC(TableAddress);
			fseek(ROMfp, TableAddress-0x185, SEEK_SET);
			if( memcmp( &ROM[TableAddress - 0x185], "@HFD", 4)){
				fseek(ROMfp, TableAddress-0x181, SEEK_SET);
				code_version = fgetc(ROMfp);
				if(code_version >= 0x04){
					fseek(ROMfp, TableAddress+0x158, SEEK_SET);
					OWMusicAddress = fgetc(ROMfp);
					fseek(ROMfp, TableAddress+0x15C, SEEK_SET);
					OWMusicAddress |= fgetc(ROMfp)<<8;
					fseek(ROMfp, TableAddress+0x160, SEEK_SET);
					OWMusicAddress |= fgetc(ROMfp)<<16;
					OWMusicAddress = macro_SNEStoPC(OWMusicAddress);
					ErasedSize = RemoveRATS(OWMusicAddress);
				}else ErasedSize = RemoveRATS(OWMusicAddress);
			}
			else{
				if( !memcmp( &ROM[TableAddress - 0x69], "STAR", 4)){
					fseek(ROMfp, TableAddress-0x61, SEEK_SET);
					code_version = fgetc(ROMfp);
					if(code_version >= 0x04){
						fseek(ROMfp, TableAddress+0x158, SEEK_SET);
						OWMusicAddress = fgetc(ROMfp);
						fseek(ROMfp, TableAddress+0x15C, SEEK_SET);
						OWMusicAddress |= fgetc(ROMfp)<<8;
						fseek(ROMfp, TableAddress+0x160, SEEK_SET);
						OWMusicAddress |= fgetc(ROMfp)<<16;
						OWMusicAddress = macro_SNEStoPC(OWMusicAddress);
						ErasedSize = RemoveRATS(OWMusicAddress);
					}
					else ErasedSize = RemoveRATS(OWMusicAddress);
				}
				else ErasedSize = RemoveRATS(OWMusicAddress);
			}
		}
		else ErasedSize = RemoveRATS(OWMusicAddress);
		if(ErasedSize){
			printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, OWMusicAddress-8);
		}
		if(ROM[0x1940] == 0x22){	//tries to erase level music
			
			puts("[LEVEL]");
			TableAddress = *(unsigned int*)&ROM[0x1941]&0xFFFFFF;
			TableAddress = macro_SNEStoPC(TableAddress);
			fseek(ROMfp, TableAddress-0x185, SEEK_SET);
			int tempread;
			fread_safe(&tempread, 1, 4, ROMfp);
			if( !memcmp( &tempread, "@HFD", 4)){			
				RemoveLevelMusic(TableAddress-0x180,0x80,0x180);
				fseek(ROMfp, TableAddress-0x181, SEEK_SET);
				switch( fgetc(ROMfp)){
					case 0x05:{
						RemoveMiscMusic( TableAddress+0x47, TableAddress+0x48, TableAddress+0x4E);
						printf("Removed old data by unofficial AddMusic 4.05 (current version)\n");
						break;}
					default:{
						printf("Version not recognized");
					}
					}
					unsigned int ErasedSize = RemoveRATS(TableAddress-0x185);
					ZeroMemory(&ROM[TableAddress-0x18D], ErasedSize);
				}
			else{
				RemoveLevelMusic(TableAddress-0x60,0x20,0x60);
				puts("[MISC]");
				fseek(ROMfp, TableAddress - 0x68, SEEK_SET);
				fread_safe(&tempread, 1, 4, ROMfp);
				if( memcmp( &tempread, "STAR", 4) ){	//whether or not the main code was by my first unofficial AM (4.0+1)
						fseek(ROMfp, TableAddress-0x61, SEEK_SET);
					switch( fgetc(ROMfp) ){
						case 0x01:{
							RemoveMiscMusic( TableAddress+0x61, TableAddress+0x62, TableAddress+0x47);
							printf("Removed old data by unofficial AddMusic 4.02\n");
							CreateNewINI(0x01);
							break;}
						case 0x02:{
							RemoveMiscMusic( TableAddress+0x38, TableAddress+0x39, TableAddress+0x3F);
							printf("Removed old data by unofficial AddMusic 4.03\n");
							CreateNewINI(0x02);
							break;}
						case 0x03:{
							RemoveMiscMusic( TableAddress+0x38, TableAddress+0x39, TableAddress+0x3F);
							printf("Removed old data by unofficial AddMusic 4.03+\n");
							//to original SMW code (activating Bonus Game)
							fseek(ROMfp, 0x1165, SEEK_SET); fwrite_safe("\xA9\xFF\x8D\x25\x14", 1, 5, ROMfp);
							//to original SMW code (init code?)
							fseek(ROMfp, 0x23C7, SEEK_SET); fwrite_safe("\x22\x41\x82\x04", 1, 4, ROMfp);
							unsigned char music[7];
							fseek(ROMfp, 0x20F8A, SEEK_SET); fread_safe(music, 1, 7, ROMfp);
							for(int i=0; i<7; i++) if(music[i]>=7) music[i]+=3;
							fseek(ROMfp, 0x20F8A, SEEK_SET); fwrite_safe(music, 1, 7, ROMfp);
							fseek(ROMfp, 0x25DC8, SEEK_SET); fwrite_safe(music, 1, 7, ROMfp);
							CreateNewINI(0x03);
							break;}
						case 0x04:{
							RemoveMiscMusic( TableAddress+0x47, TableAddress+0x48, TableAddress+0x4E);
							printf("Removed old data by unofficial AddMusic 4.04\n");
							break;}
					}
					unsigned int ErasedSize = RemoveRATS(TableAddress-0x61);
					ZeroMemory(&ROM[TableAddress-0x69], ErasedSize);
				}else{
					//remove the main code which didn't have version number
					unsigned int ErasedSize = RemoveRATS(TableAddress-0x60);
					ZeroMemory(&ROM[TableAddress-0x68], ErasedSize);
					printf("Removed old data by unofficial AddMusic 4.01\n");
					CreateNewINI(0x00);
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
	fseek(ROMfp, 0x27C, SEEK_SET); fwrite_safe("\xA0\x00\x00\xA9\xAA\xBB", 1, 6, ROMfp);
	fseek(ROMfp, 0x28D, SEEK_SET); fwrite_safe("\xB7\x00\xC8\xEB", 1, 4, ROMfp);
	fseek(ROMfp, 0x296, SEEK_SET); fwrite_safe("\xB7\x00\xC8\xEB", 1, 4, ROMfp);
	fseek(ROMfp, 0x2B4, SEEK_SET); fwrite_safe("\xC2\x20\xB7\x00\xC8\xC8\xAA\xB7\x00\xC8\xC8\x8D\x42\x21\xE2\x20", 1, 0x10, ROMfp);
*/
/*
	//to original SMW code (activating Bonus Game)
	fseek(ROMfp, 0x1165, SEEK_SET); fwrite_safe("\xA9\xFF\x8D\x25\x14", 1, 5, ROMfp);
*/
	//to original SMW code (uploading level music)
//	fseek(ROMfp, 0x1902, SEEK_SET); fwrite_safe("\x20\x34\x81", 1, 3, ROMfp);
//
//	//to original SMW code (brightness, and its direction)
//	fseek(ROMfp, 0x193B, SEEK_SET); fwrite_safe("\x29\xBF\x8D\xDA\x0D\x9C\xAE\x0D\x9C\xAF\x0D", 1, 0xB, ROMfp);
///*
//	//to original SMW code (init code?)
//	fseek(ROMfp, 0x23C7, SEEK_SET); fwrite_safe("\x22\x41\x82\x04", 1, 4, ROMfp);
//*/
//	//to original SMW code (related to P-switch and/or Star Music)
//	fseek(ROMfp, 0x284A, SEEK_SET); fwrite_safe("\xAD\xDA\x0D\x29\x7F\x09\x40\x8D\xDA\x0D", 1, 0xA, ROMfp);
//
//	//to original SMW code (related to music when loading level)
//	fseek(ROMfp, 0x2875F, SEEK_SET); fwrite_safe("\xD0", 1, 1, ROMfp);

	puts("");
	return 0;
}

int AddMusic::RemoveLevelMusic(unsigned int p, int musicnum, int offset)
{
	for(int i=0; i<musicnum ; i++){	// erases old data
		unsigned int address;
		fseek(ROMfp, p + i*3, SEEK_SET); fread_safe( (const char *)&address, 1, 3, ROMfp);
		address &= 0xFFFFFF;
		address = macro_SNEStoPC(address);
		if( 0x80200 <= address && address < ROMsize){
			unsigned int ErasedSize = RemoveRATS(address);
			printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, address-8);
			address = 0;
			fseek(ROMfp, TableAddress - offset + i*3, SEEK_SET);
			fwrite_safe(&address, 1, 3, ROMfp);
		}
	}

	return 0;
}

int AddMusic::RemoveMiscMusic(unsigned int low, unsigned int middle, unsigned int high)
{
	unsigned int address;
	fseek(ROMfp, low, SEEK_SET);
	address = fgetc(ROMfp);
	fseek(ROMfp, middle, SEEK_SET);
	address |= (fgetc(ROMfp) | 0x80)<<8;
	fseek(ROMfp, high, SEEK_SET);
	address |= (fgetc(ROMfp))<<16;
	address = macro_SNEStoPC(address);
	unsigned int ErasedSize = RemoveRATS(address);
	printf("Erased 0x%04X bytes, at 0x%06X\n", ErasedSize, address-8);
	return 0;
}


int AddMusic::CreateNewINI(int version, bool bNew)
{
	char file_path[0x80 + 0x10][MAX_PATH];
	char misc_path[0xB][MAX_PATH];
	for(int i=0; i<(0x80 + 0x10) ; i++) ZeroMemory(file_path[i], MAX_PATH);
	for(int i=0; i<=0xA; i++) ZeroMemory(misc_path[i], MAX_PATH);

	char freeram[7]={0, 0, 0, 0, 0, 0, 0};
	char TitleMusic[3]={0,0,0};

	GetPrivateProfileString("SETTINGS", "FreeRAM", "7EC100", freeram, 7, IniPath);
	GetPrivateProfileString("SETTINGS", "TitleMusic", "01", TitleMusic, 3, IniPath);
	if(version < 0x02){
		for( int i=0x11; i<=0x18 ; i++){
			char str[3]; sprintf(str, "%02X", i);
			GetPrivateProfileString("OVERWORLD", str, "", file_path[i - 0x11], MAX_PATH, IniPath);
		}
	}else if(version==0x02 || version==0x03){
		for(int i=0x07; i<=0x16 ; i++){
			char str[3]; sprintf(str, "%02X", i);
			GetPrivateProfileString("OVERWORLD", str, "", file_path[i - 0x7], MAX_PATH, IniPath);
		}
	}else if(version==0x04){
		for(int i=0x0A; i<=0x19; i++){
			char str[3]; sprintf(str, "%02X", i);
			GetPrivateProfileString("OVERWORLD", str, "", file_path[i - 0xA], MAX_PATH, IniPath);
		}
	}
	if(version >= 0x03){
		for(int i=0; i<=0xA; i++){
			char str[3]; sprintf(str, "%02X", common_num[i]);
			GetPrivateProfileString("MISC", str, "", misc_path[i], MAX_PATH, IniPath);
		}
	}
	for( int i=0x20; i<=0x9F ; i++){
		char str[32]; sprintf(str, "%02X", i);
		GetPrivateProfileString("LEVEL", str, "", file_path[(i - 0x20) + 0x10], MAX_PATH, IniPath);
	}

	remove(IniPath);

	FILE *ini = NULL;
	if(!fopen_safe(&ini, IniPath, "w")){
		printf("Couldn't make %s\n", IniPath);
		return 1;
	};

	fprintf(ini, "[SETTINGS]\nCreate.msc=1\nFreeRAM=%s\nTitleMusic=%s\n[OVERWORLD]\n", freeram, TitleMusic);
	for(int i=0xA ; i<=0x19 ; i++){
		fprintf(ini, "%02X=%s\n", i, file_path[i - 0xA]);
	}
	fprintf(ini, "[MISC]\n");
	const char *common_name[0x0B]={
		"Mario Died.txt", "Game Over.txt", "Passed Boss.txt", "Passed Level.txt", "Have Star.txt", "P-switch.txt",
		"Into Keyhole.txt", "Zoom In.txt", "Welcome.txt", "Done Bonus Game.txt", "Rescue Egg.txt"
	};
	for(int i=0; i<=0xA ; i++){
		fprintf(ini, "%02X=%s\n", common_num[i], (bNew ? common_name[i] : misc_path[i]));
	}
	fwrite_safe("[LEVEL]\n", 1, strlen("[LEVEL]\n"), ini);
	for(int i=0x20 ; i<=0x9F ; i++){
		char str[MAX_PATH+4]; sprintf(str, "%02X=%s\n", i, file_path[(i - 0x20) + 0x10]);
		fwrite_safe(str, 1, strlen(str), ini);
	}
	fclose(ini);
	return 0;
}

/***************************************************************
initialize
***************************************************************/
int AddMusic::initialize(void)
{
	unsigned char main_code[]={
		//version
		0x40,0x48,0x46,0x44,0x05,
		//table
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		
		//main code
		0x9C,0xAE,0x0D,0x9C,0xAF,0x0D,0xAD,0x00,0x01,0xC9,0x11,0xD0,0x7D,0xAE,0xDA,0x0D,
		0xAD,0x25,0x14,0xD0,0x10,0xAD,0x1A,0x14,0x0D,0x1D,0x14,0xF0,0x08,0x8A,0xCF,0x00,
		0xC1,0x7E,0x18,0xF0,0x4F,0x8A,0x8F,0x00,0xC1,0x7E,0xC9,0x20,0x90,0x5D,0xC2,0x30,
		0x29,0xFF,0x00,0x20,0x00,0x00,0xEA,0xEA,0xEA,0xEA, //0x4A,0x4A,0x85,0x00,0x0A,0x65,0x00,
		0xAA,0xBF,0xC9,0x7E,0x00,0x48,
		0xBF,0xCA,0x7E,0x00,0x85,0x04,0xA9,0x00,0x80,0x85,0x00,0xE2,0x30,0xA9,0x00,0x4B,
		0x62,0x06,0x00,0xF4,0x4C,0x80,0x5C,0x1A,0x81,0x80,0xC2,0x20,0x68,0x85,0x00,0xE2,
		0x20,0xA2,0x00,0xCA,0xD0,0xFD,0xA5,0x05,0x4B,0x62,0x06,0x00,0xF4,0x4C,0x80,0x5C,
		0x1A,0x81,0x80,0x38,0xAD,0xDA,0x0D,0x89,0xE0,0xF0,0x0A,0x29,0x00, //0x03,
		0x1A,0x09,0x20,
		0x8D,0xDA,0x0D,0xB0,0x02,0xA9,0x00,0x8D,0xFB,0x1D,0x6B,0xDA,0x4B,0x62,0x06,0x00,
		0xF4,0x4C,0x80,0x5C,0x48,0x81,0x00,0x68,0x80,0xED,0x29,0x3F,0x8D,0xDA,0x0D,0xA9,
		0xFF,0x8F,0x00,0xC1,0x7E,0x6B,0xA5,0x00,0x29,0xFF,0x7F,0xA8,0xA9,0x00,0x80,0x85,
		0x00,0xA9,0xAA,0xBB,0x6B,0xB7,0x00,0xC8,0x10,0x05,0xA0,0x00,0x00,0xE6,0x02,0xEB,
		0x6B,0x22,0x16,0x81,0x00,0x22,0x16,0x81,0x00,0x8D,0x42,0x21,0xEB,0x8D,0x43,0x21,
		0x6B,0x22,0x91,0x8D,0x84,0xAD,0xFB,0x1D,0x10,0x04,0x9C,0xFB,0x1D,0x6B,0xC9,0x0A,
		0x2A,0x8F,0x00,0xC1,0x7E,0x4A,0x90,0x09,0x48,0x22,0xAF,0x81,0x00,0x68,0x8D,0xFB,
		0x1D,0x6B,0x9D,0x11,0x1F,0x29,0xFF,0x00,0x48,0x08,0xE2,0x30,0xAA,0xAF,0x00,0xC1,
		0x7E,0x29,0x01,0x85,0x00,0xBF,0xC8,0xDB,0x04,0xC9,0x0A,0xA9,0x00,0x2A,0x45,0x00,
		0xF0,0x1C,0x78,0x9C,0x00,0x42,0xA5,0x00,0x49,0x01,0x8F,0x00,0xC1,0x7E,0x4A,0x22,
		0xAF,0x81,0x00,0xAD,0x12,0x42,0x10,0xFB,0x58,0xA9,0xA1,0x8D,0x00,0x42,0x28,0x68,
		0x6B,0xD0,0x1A,0xAA,0xBD,0xA1,0x1E,0x30,0x14,0xA9,0x46,0x8E,0x83,0x01,0xE2,0x30,
		0xAF,0xD2,0x83,0x04,0x22,0x3F,0x81,0x00,0xA9,0xFF,0x8D,0xFB,0x1D,0x6B,0xF4,0x4C,
		0x80,0xB0,0x04,0x5C,0x0E,0x81,0x80,0xA9,0x00,0x85,0x00,0xA9,0x00,0x85,0x01,0xA9,
		0x00,0x5C,0x1A,0x81,0x80,
		0x38,0xE9,0x20,0x00,0x85,0x00,0x0A,0x18,0x65,0x00,0x18,0x69,0x18,0x00,0x60
	};

	try{
		//For Overworld
		OWMusicAddress = (unsigned char)ROM[0x30F] | (unsigned char)ROM[0x314]<<8 | (unsigned char)ROM[0x319]<<16;

		OWMusicAddress = macro_SNEStoPC(OWMusicAddress);	//to PC address
		TableAddress= *(unsigned int*)&ROM[0x1941]&0xFFFFFF;	//TableAddress, as main code loc
		TableAddress=macro_SNEStoPC(TableAddress);

		if( OWMusicAddress != 0x071AB1 || ROM[0x1940] ==0x22 ) Remove();

/*		unsigned int FreeRAM, TitleMusic;
		char tmp_str[7]={0, 0, 0, 0, 0, 0, 0};
		GetPrivateProfileString("SETTINGS", "FreeRAM", "7EC100", tmp_str, 7, IniPath);
		FreeRAM = (strtoul(tmp_str, 0, 16)&0xFFFFFF);
		GetPrivateProfileString("SETTINGS", "TitleMusic", "01", tmp_str, 3, IniPath);
		TitleMusic = (unsigned char)strtoul(tmp_str, 0, 16);

		unsigned int p = search_free_space( sizeof(main_code)+RATS_TAG_SIZE);
		unsigned int p2 = p;
		p-=0x1F8;
		p+=0x124;
		p = macro_PCtoSNES(p);
		unsigned int tmp;

		//Reloc
		int Reloc[]={0x9D,0xA2,0x123,0x127,0x14B,0x181,0x1A6};
		for(unsigned int i=0; i<(sizeof(Reloc)/sizeof(int)); i++){
			short loc = *(unsigned short*)&main_code[Reloc[i]+0x124] - 0x8000;
			tmp=loc+p;
			memcpy(&main_code[Reloc[i]+0x124], &tmp, 3);
		}
		//!FreeRAM
		int FreeRAMloc[]={0x80,0x88,0x103,0x143,0x15F,0x17C};
		for(unsigned int i=0; i<(sizeof(FreeRAMloc)/sizeof(int)); i++){
			memcpy(&main_code[FreeRAMloc[i]+0x124], &FreeRAM, 3);
		}

		write_with_RATS( p2, sizeof(main_code), (const char *)main_code);
		printf("Main code inserted at 0x%06X\n\n", p2);

		//p-switch and star music related
		tmp = p + 0xFB;
		fseek(ROMfp, 0x284F, SEEK_SET); fputc(0x22, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp); fputc(0xEA, ROMfp);
		//SPC Uploading
		fseek(ROMfp, 0x27C, SEEK_SET); fwrite_safe("\x22\x00\x00\x00\xEA\xEA", 1, 6, ROMfp);
		tmp = p + 0x107;
		fseek(ROMfp, 0x27D, SEEK_SET); fwrite_safe(&tmp, 1, 3, ROMfp);
		tmp = p + 0x116;
		fseek(ROMfp, 0x28D, SEEK_SET); fputc(0x22, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp);
		fseek(ROMfp, 0x296, SEEK_SET); fputc(0x22, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp);
		fseek(ROMfp, 0x2B4, SEEK_SET);
		fputc(0x22, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp);
		fputc(0x22, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp);
		tmp = p + 0x122;
		fwrite_safe("\xAA\x22", 1, 2, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp); fwrite_safe("\xEA\xEA\xEA", 1, 3, ROMfp);

		//hack stuff
		tmp = macro_SNEStoPC(p+0x95);
		fseek(ROMfp, tmp, SEEK_SET);
		short tmp2 = (p+0x1C6)&0xFFFF;
		fwrite_safe(&tmp2, 1, 2, ROMfp);

		//level music loading
		fseek(ROMfp, 0x1902, SEEK_SET); fwrite_safe("\xEA\xEA\xEA", 1, 3, ROMfp);
		//music setting during level-loading
		fseek(ROMfp, 0x2875F, SEEK_SET); fputc(0x80, ROMfp);
		//ow hack 1
		tmp = p + 0x132;
		fseek(ROMfp, 0x2362, SEEK_SET); fwrite_safe(&tmp, 1, 3, ROMfp);
		//ow hack 2
		tmp = p + 0x153;
		fseek(ROMfp, 0x21CA5, SEEK_SET); fputc(0x22,ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp); fwrite_safe("\xEA\xEA", 1, 2, ROMfp);
		//ow hack 3
		tmp = p + 0x192;
		fseek(ROMfp, 0x20FDA, SEEK_SET); fputc(0x22,ROMfp); fwrite_safe(&tmp,1,3,ROMfp); fputc(0xEA,ROMfp);
		//title hack
		tmp = p + 0x149;
		fseek(ROMfp, 0x18C3, SEEK_SET); fputc(0xA9, ROMfp); fputc(TitleMusic, ROMfp); fwrite_safe("\xC9\x0A\x22",1,3,ROMfp); fwrite_safe(&tmp,1,3,ROMfp);
		//main hack
		tmp = p + 0x61;
		fseek(ROMfp, 0x193B, SEEK_SET); fwrite_safe("\xEA\xEA\xEA\xEA\xEA\x22", 1, 6, ROMfp); fwrite_safe(&tmp, 1, 3, ROMfp); fwrite_safe("\xEA\xEA", 1, 2, ROMfp);
		//modify earthquake code
		fseek(ROMfp, 0x26860, SEEK_SET); tmp = fgetc(ROMfp); fseek(ROMfp, 0x20FD8, SEEK_SET); fputc(tmp, ROMfp);*/
	}
	catch(const char * str){
		puts(str);
		return 1;
	}

	FILE *tmp = NULL;
	if(!fopen_safe(&tmp, IniPath, "r")) CreateNewINI(0x04, true);
	else fclose(tmp);

	return 0;
}

/***************************************************************
Add Music Main
***************************************************************/
int AddMusic::AMmain(void)
{
	fseek(ROMfp, 0, SEEK_SET);
	fread_safe(ROM, 1, ROMsize, ROMfp);
	MarkRATS();

	TableAddress= *(unsigned int*)&ROM[0x1941]&0xFFFFFF;
	TableAddress=macro_SNEStoPC(TableAddress);

	unsigned int Createmsc = GetPrivateProfileInt("SETTINGS", "Create.msc", 0, IniPath);
	TableAddress-=0x180;
	
	bool b_erased=false;
	b_inserted=false;
	b_error = false;


	try{

		//Overworld
		b_LevelMusic=false;
		b_common=false;
		printf("[Overworld Music]\n");

		SPC.erase();
		ZeroMemory(ptrMusic, sizeof(ptrMusic));

		for(int i=0xA; i<=0x19 ;i++){
			char str[7];
			sprintf(str, "%02X", i);
			char path[MAX_PATH];
			GetPrivateProfileString("OVERWORLD", str, "", path, MAX_PATH, IniPath);
			if(path[0]!=0){
				char path2[MAX_PATH];
				sprintf(path2, "OW/%s", path);
				convert(path2, i, i - 0xA);
			}
		}
		save_to_rom(OW_LIMIT, 0);
		if(b_error) return 0;


		//misc Music
		b_common=true;
		printf("[MISC]\n");
		SPC.erase();
		ZeroMemory(ptrMusic, sizeof(ptrMusic));
		for(int i=0; i<=0xA ; i++){
			char str[7];
			sprintf(str, "%02X", common_num[i]);
			char path[MAX_PATH];
			GetPrivateProfileString("MISC", str, "", path, MAX_PATH, IniPath);
			if(path[0]!=0){
				char path2[MAX_PATH];
				sprintf(path2, "MISC/%s", path);
				convert(path2, common_num[i], i);
			}
		}
		save_to_rom( MISC_LIMIT, 0);
		if(b_error) return 0;

		//Level
		b_common=false;
		b_LevelMusic=true;
		printf("[Level Music]\n");
		for(int i=0x20; i<0xA0 ; i+=1){
			SPC.erase();
			ZeroMemory(ptrMusic, sizeof(ptrMusic));
			for(int j=0 ; j<1 ; j++){
				char str[7];
				sprintf(str, "%X", i|j);
				char path[MAX_PATH];
				GetPrivateProfileString("LEVEL", str, "", path, MAX_PATH, IniPath);
				if(path[0]!=0){
					char path2[MAX_PATH];
					sprintf(path2, "LEVEL/%s", path);
					convert(path2, i|j, j);
				}
			}
			save_to_rom(LEVEL_LIMIT, TableAddress+(i - 0x20)*3);
		}

	}
	catch(const char * str){
		puts(str);
		return 0;
	}
	if(b_error) return 0;

	if(Createmsc && (strlen(ROMname))>4){
		unsigned int Length = (strlen(ROMname))-3;
		ROMname[Length++]='m';
		ROMname[Length++]='s';
		ROMname[Length++]='c';
		FILE *mscfp = NULL;
		if(fopen_safe(&mscfp, ROMname, "w")){
			char num[3];
			char path[MAX_PATH];
			char content[MAX_PATH];
			for(int i=0x20;i<0xA0;i++){
				sprintf(num, "%02X", i);
				GetPrivateProfileString("LEVEL", num, "", path, MAX_PATH, IniPath);
				if(path[0]==0) continue;
				if( 4 < (Length = strlen(path))) path[Length-4]=0;
				sprintf(content, "%02X\t0\t%s\n", i, path);
				fwrite_safe(content, 1, strlen(content), mscfp);
			}
			for(int i=0x07;i<=0x16;i++){
				sprintf(num, "%02X", i);
				GetPrivateProfileString("OVERWORLD", num, "", path, MAX_PATH, IniPath);
				if(path[0]==0) continue;
				if( 4 < (Length = strlen(path))) path[Length-4]=0;
				sprintf(content, "%02X\t1\t%s\n", i, path);
				fwrite_safe(content, 1, strlen(content), mscfp);
			}
			fclose(mscfp);
		}
	}

	if(b_inserted) return 1;
	else return 2;
}
/***************************************************************
save to rom
***************************************************************/
void AddMusic::save_to_rom(unsigned int limit, unsigned int TablePtr)
{
	if(!SPC.length()) return;

	printf("%4X / %4X\n", SPC.length(), limit);
	if(SPC.length() > limit) throw "The music data size is too large";

	unsigned int p;
	char int_str[5]={0, 0, 0, 0, 0};

	if(b_common){
		SPC.append( (int)0x1370001A);
		for(int i=0, j=9, k=0; i<0x1A ; i+=2, j++){
			if( common_num[k] == j){
				SPC.append( (short)ptrMusic[k]);
				k++;
			}
			else SPC.append( (short)0);
		}
		SPC.append( (int)0x05000000);
		*(unsigned int*)int_str = ( MISC_INSERT_AT <<16)|(SPC.length()-0x22);
		SPC.insert(0, int_str, 4);
	}else if(b_LevelMusic){
		SPC.append((int)0x13A00008);
		for(int i=0 ; i<=3 ; i++) SPC.append( (short)ptrMusic[i]);

		SPC.append((int)0x05000000);
		*(unsigned int*)int_str=( LEVEL_INSERT_AT<<16)|(SPC.length()-0x10);
		SPC.insert(0, int_str, 4);
	}else{	//OW music
		SPC.append( (int)0x13720020);
		for(int i=0; i<0x10 ; i++) SPC.append( (short)ptrMusic[i]);
		SPC.append((int)0x05000000);
		*(unsigned int*)int_str=( OW_INSERT_AT<<16)|(SPC.length()-0x28);
		SPC.insert(0, int_str, 4);
	}
	write_with_RATS( (p = search_free_space_latter_no_bank(SPC.length()+RATS_TAG_SIZE)), SPC.length(), SPC.c_str());

	printf("Music inserted at 0x%06X\n\n", p);
	p -= (0x200-RATS_TAG_SIZE);
	p = macro_PCtoSNES(p);
	if(b_common){
		fseek(ROMfp, TableAddress + 0xA7 + 0x120, SEEK_SET);
		fputc( (unsigned char)p, ROMfp); fputc( (unsigned char)(p>>8), ROMfp);
		fseek(ROMfp, TableAddress + 0xAE + 0x120, SEEK_SET);
		fputc((unsigned char)(p>>16), ROMfp);
	}else if(TablePtr){
		fseek(ROMfp, TablePtr, SEEK_SET);
		fwrite_safe(&p, 1, 3, ROMfp);
	}else{	//OW music
		fseek(ROMfp, TableAddress + 0x1B8 + 0x120, SEEK_SET); fputc( (unsigned char)p, ROMfp);
		fseek(ROMfp, TableAddress + 0x1BC + 0x120, SEEK_SET); fputc( (unsigned char)(p>>8), ROMfp);
		fseek(ROMfp, TableAddress + 0x1C0 + 0x120, SEEK_SET); fputc( (unsigned char)(p>>16), ROMfp);
	}
	return;
}
/***************************************************************
convert
***************************************************************/
int AddMusic::convert(const char *FileName, const int MusicNum, int Num)
{
	FILE *fp = NULL;
	if(!fopen_safe(&fp, FileName, "rb")){
		printf(" %02X : %s : Couldn't open the file\n", MusicNum, FileName);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	if( !(FileSize = ftell(fp)) ){	//Get file size
		printf("%s : The file contains nothing\n", FileName);
		fclose(fp);
		return -1;
	}
	b_inserted=true;

	fseek(fp, 0, SEEK_SET);
	buf = (char *)malloc(FileSize+1);
	if(!buf){
		printf("Memory allocation failed\n");
		fclose(fp);
		b_error=true;
		return -1;
	}
	fread_safe(buf, 1, FileSize, fp);
	fclose(fp);
	buf[FileSize]=0;

	printf("%3X : %s\n", MusicNum, FileName);
	mml_to_spc();
	free(buf); buf=0;
	adjust_address(Num);

	return 0;
}
/***************************************************************
mml_to_spc
***************************************************************/
void AddMusic::mml_to_spc(void)
{
	ptr=0;			//pointer of buffer
	line=1;			//the line number of txt

	ch=0;
	int lastCh=0;
	octave=4;
	LastNoteValue= -1;
	DefNoteValue= 8;
	memset(Instrument, 0xFF, sizeof(int)*(MAX_CH+1));

	unsigned int LastLoop=0xFFFFFFFF;

	int i,j;

	int q[MAX_CH+1];
	for(int z=0;z<(MAX_CH+1);z++){
		q[z]=0x7F;
		update_q[z]=true;
		OneSPC[z].erase();
		if( z!= 8 ) NotHaveMusic[z][0] = NotHaveMusic[z][1] = false;
	}
	HasIntro=false;
	WontLoop=false;
	triplet=false;
	bool PitchSlide=false;

	int LoopLabel=0;

//	LoopCount=0;
	ZeroMemory(ptrARAM, sizeof(ptrARAM));
	ZeroMemory(ptrIntro, sizeof(ptrIntro));
//	ZeroMemory(ptrLoop, sizeof(ptrLoop));
	ptrLoop.erase();
	ptrLabelLoop.erase();
	ptrLabelNum.erase();

	ZeroMemory(TRNSMAP, sizeof(TRNSMAP));
	int tmpTrns[19] = {0, 0, 5, 0, 0, 0, 0, 0, 0, -5, 6, 0, -5, 0, 0, 8, 0, 0, 0};
	memcpy(TRNSMAP, tmpTrns, sizeof(int)*19);



	while(ptr<FileSize){
		switch(buf[ptr]){
			case '?':
				ptr++;
				i=GetInt(false);
				if(i==-1) i=0;
				switch(i){
					case 0: WontLoop=true; break;
					case 1: NotHaveMusic[ch][0]=true; break;
					case 2: NotHaveMusic[ch][1]=true; break;
				}
				continue;
			case '!':	//‹­§I—¹
				ptr=0xFFFFFFFF;
				continue;
			case '#':	//ch number
				ptr++;
				i=GetInt(false);
				if( 0 <= i && i <=7){
					ch=i;
					q[8] = q[ch];
					update_q[8] = update_q[ch];
					LastNoteValue=-1;
				}else error("Invalid channel number", true);
				continue;
			case 'l':	//default note value
				ptr++;
				i=GetInt(false);
				if( 1<= i && i <= 192) DefNoteValue=i;
				else error("Invalid note value", true);
				continue;
			case 'w':	//global volume
				ptr++;
				i=GetInt(false);
				if( 0<=i && i<=255){
					put(0xE0);
					put(i);
				}else error("Invalid volume", true);
				continue;
			case 'v':	//volume / ch
				ptr++;
				i=GetInt(false);
				if( 0<=i && i<=255){
					put(0xE7);
					put(i);
				}else error("Invalid volume", true);
				continue;
			case 'q':	//quantization
				ptr++;
				i=GetHex(false);
				if( 0<=i && i<=0x7F){
					q[ch] = q[8] = i;
					update_q[ch] = update_q[8] = true;
				}else error("Invalid value in 'q' command", true);
				continue;
			case 'y':	//pan
				ptr++;
				i=GetInt(false);
				if( 0<=i && i<=20 ){
					put(0xDB);
					put(i);
				}else error("Invalid pan value", true);
				continue;
			case '/':
				HasIntro=true;
				ptr++;
				ptrIntro[ch] = OneSPC[ch].length();
				LastNoteValue=-1;
//				update_q[ch]=update_q[8]=true;
				continue;
			case 't':
				ptr++;
				if(!strncmp(&buf[ptr], "uning[", 6)){
					ptr+=6;
					i=GetInt(false);
					if( 0<=i&&i<=255){
						if(strncmp(&buf[ptr], "]=", 2)){error("Invalid tuning", true); continue;}
						ptr+=2;
						while(1){
							bool plus=true;
							if(buf[ptr]=='+') ptr++;
							else if(buf[ptr]=='-'){ptr++, plus=false;}
							j=GetInt(false);
							if(j==-1){error("Invalid tuning value", true); break;}
							if(!plus) j=-j;
							TRNSMAP[i]=j;
							if(buf[ptr]!=',') break;
							ptr++;
							if(++i==256){ error("[] exceeded the range", true); break;}
						}
					}else error("[] out of the range", true);
					continue;
				}
				i=GetInt(false);
				if( 0<=i && i<=255){
					put(0xE2);
					put(i);
				}else error("Invalid tempo", true);
				continue;
			case 'o':
				ptr++;
				i=GetInt(false);
				if( 1<=i && i<=6) octave=i;
				else error("Invalid octave", true);
				continue;
			case '@':{
				bool direct=false;
				ptr++;
				if(buf[ptr]=='@'){ ptr++; direct=true;}
				i=GetInt(false);
				if( (0<=i && i<= 18) || (direct&&0<=i)){
					put(0xDA);
					put(i);
					Instrument[ch]=i;
				}else if( 21<=i && i<=30 && !direct) Instrument[ch]=i;
				else error("Invalid intrument", true);
				continue;}
			case '(':
				ptr++;
				i=GetInt(false);
				if( i==-1){
					error("Specify label number", true);
					continue;					
				}else if(i==0){
					error("Label number can't be 0", true);
					continue;
				}
				if( buf[ptr]==')'){
					ptr++;
					if(buf[ptr]=='[') LoopLabel=i;
					else{
						LastNoteValue=-1;
						ptrLoop.append( OneSPC[ch].length()+1+(ch << 16) );

						int tmp;
						for(tmp = 0; tmp<ptrLabelNum.length() ; tmp++)
							if(ptrLabelNum[tmp] == i) break;
						if(tmp == ptrLabelNum.length() ){
							error("Specified label number can't be found", true);
							tmp = 0;
						}

						put(0xE9);
						put(ptrLabelLoop[tmp]);
						put(ptrLabelLoop[tmp]>>8);
						i=GetInt(false);
						if( i==-1) i=1;
						if( i<1 || 127<i){
							error("Invalid loop count", true);
							i=1;
						}
						put(i);
					}
				}
				else if(buf[ptr]==')') ptr++;
				continue;
			case '[':
				ptr++;

				LastLoop = OneSPC[8].length();
				lastCh=ch;
				ch=8;
				LastNoteValue=-1;
				Instrument[8] = Instrument[lastCh];
				if( LoopLabel ){
					bool duplicate=false;
					for(int tmp=0;tmp<ptrLabelNum.length();tmp++)
						if( LoopLabel == ptrLabelNum[tmp] ){
							error("Specified label number duplicated", true);
							duplicate=true;
							break;
						}
					

					if(!duplicate){
						ptrLabelLoop.append(LastLoop);
						ptrLabelNum.append(LoopLabel);
					}
				}
				LoopLabel=0;
				continue;
			case ']':
				ptr++;
				i=GetInt(false);
				if( i==-1) i=1;
				if( i<1 || 127<i){
					error("Invalid loop count", true);
					i=1;
				}
				put(0x00);

				ch=lastCh;
				ptrLoop.append( OneSPC[ch].length()+1+(ch << 16) );

				put(0xE9);
				put(LastLoop);
				put(LastLoop>>8);
				put(i);
				continue;
			case '*':
				ptr++;
				LastNoteValue=-1;
				i=GetInt(false);
				if( i==-1) i=1;
				if( i<1 || 127<i){
					error("Invalid loop count", true);
					i=1;
				}
				ptrLoop.append( OneSPC[ch].length()+1+(ch << 16) );
				put(0xE9);
				put(LastLoop);
				put(LastLoop>>8);
				put(i);
				continue;
			case 'p':{
				ptr++;
				i=GetInt(false);
				char temp=buf[ptr++];
				j=GetInt(false);
				if( (1<=i&&i<=255) && (temp==',') && (0<=j&&j<=255) ){
					put(0xDE);
					put(0x00);
					put(i);
					put(j);
				}else if(!i) put(0xDF);
				else error("Invalid value in 'p' command", true);
				continue;}
			case '{':
				ptr++;
				if(!triplet) triplet=true;
				else error("triplet-enable-command specified repeatedly", true);
				continue;
			case '}':
				ptr++;
				if(triplet) triplet=false;
				else error("triplet-disable-command specified repeatedly", true);
				continue;
			case '>':
				ptr++;
				if( ++octave > 7){
					octave=7;
					error("octave is too high", true);
				}
				continue;
			case '<':
				ptr++;
				if( --octave < 0){
					octave=0;
					error("octave is too low", true);
				}
				continue;
			case '&':
				ptr++;
				if(PitchSlide) error("'&' command specified repeatedly", true);
				else PitchSlide=true;
				continue;
			case '$':
				ptr++;
				i=GetHex(false);
				if( 0<=i && i<= 0xFF){
					put(i);
				}else error("Invalid hexadecimal value", true);
				continue;
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'a':
			case 'b':
			case 'r':
			case '^':
				j = buf[ptr++];
				i = (j=='r')?0xC7:
					(j=='^')?0xC6:GetPitch(j);
				if(i<0) i=0xC7;
				else if( 21<=Instrument[ch]&&Instrument[ch]<=30 && i<0xC6){
					i = 0xD0+(Instrument[ch]-21);
					Instrument[ch]=-1;
				}
				if(PitchSlide){
					PitchSlide=false;
					put(0xDD);
					put(0x00);
					put(LastNoteValue);
					put(i);
				}
				j=GetNoteValue( GetInt(false) );
				if(j>=0x80){			//Note value was too long!
					put(0x60);
					if(update_q[ch]){
						put(q[ch]);
						update_q[ch] = update_q[8] = false;
					}
					put(i);				//pitch
					j-=0x60;
					while(j>0x60){
						j-=0x60;
						put(0xC6);		//tie
					}
					if(j>0){
						if( j!=0x60) put(j);
						put(0xC6);
					}
					LastNoteValue=j;
					continue;
				}else if(j>0){			//note value needed to be updated
					put(j);
					LastNoteValue=j;
					if(update_q[ch]){
						put(q[ch]);
						update_q[ch]=update_q[8]=false;
					}
				}
				put(i);					//pitch
				continue;



			case '\n':
				ptr++, line++;
				continue;
			case ';':
				while( buf[ptr]!='\n' && buf[ptr]!=0) ptr++;
				continue;
			default:
				ptr++;
				continue;
		}//switch
	}//while

	for(i=0;i<MAX_CH;i++){
		if(!OneSPC[i].length()) continue;
		ch=i;
		put(0x00);
	}

	if(b_error) throw "There shouldn't be any error in the .txt file";

	return;
}

/***************************************************************
adjust address
***************************************************************/
void AddMusic::adjust_address(int num)
{
	//adjust starting address
	unsigned int p=  (b_LevelMusic	?	LEVEL_INSERT_AT :
			 b_common		?	MISC_INSERT_AT :
								OW_INSERT_AT) + SPC.length();
	for(unsigned int i=0 ; i<(MAX_CH+1) ; i++){
		ptrARAM[i]=(OneSPC[i].length()) ? p : 0;
		if(HasIntro && i<8 && (NotHaveMusic[i][0] ? OneSPC[i].length() : ptrIntro[i])) ptrIntro[i]+=ptrARAM[i];
		p += OneSPC[i].length();
	}

	//adjust loop address
	for(int i=0 ; i<ptrLoop.length() ; i++){
		if(!ptrLoop[i]) continue;
		ch =ptrLoop[i]>>16;
		p=(unsigned short)ptrLoop[i];
		*(unsigned short*)&OneSPC[ch][p] += ptrARAM[8];
	}

	//Copy OneSPC[][] data to SPC[]
	unsigned int total=0;
	for(unsigned int i=0 ; i<(MAX_CH+1) ; i++){
		if(OneSPC[i].length()){
			SPC += OneSPC[i];
			total+=OneSPC[i].length();
		}
		char str[32]; sprintf(str, "Ch%i : 0x%03X  ", i, OneSPC[i].length());
		printf("%s", str);
	}
	puts("");
	printf("total size : 0x%4X (%i)\n", total, total);

	for(int i=0;i<MAX_CH;i++){
		SPC.append( (short)( NotHaveMusic[i][0] ? 0 : (unsigned short)ptrARAM[i]) );
	}
	for(int i=0;i<MAX_CH && HasIntro;i++){
		SPC.append( (short)(NotHaveMusic[i][1] ? 0 : (unsigned short)ptrIntro[i]) );
	}

	unsigned int ARAM_INSERT_AT = ( b_LevelMusic	? LEVEL_INSERT_AT :
							b_common		? MISC_INSERT_AT : OW_INSERT_AT);
	ptrMusic[num]=SPC.length()+ARAM_INSERT_AT;
	p = ARAM_INSERT_AT + SPC.length() - (HasIntro ? 0x20 : 0x10);
	SPC.append( (short)p );
	if(HasIntro){
		p = ARAM_INSERT_AT + SPC.length() - 0x12;
		SPC.append( (short)p );
	}
	if(!WontLoop){
		SPC.append( (short)0x00FF );
		SPC.append( (short)((unsigned short)ptrMusic[num] + (HasIntro ? 2 : 0)) );
	}else{

		SPC.append( (short)0 );
	}

	return;
}

/***************************************************************
Sound Effect
***************************************************************/
int AddMusic::SoundEffect(void)
{
	unsigned int address = (*(unsigned int*)&ROM[0x256])&0xFFFFFF;
	address = macro_SNEStoPC(address);
	RemoveRATS(address);
	fseek(ROMfp, 0x255, SEEK_SET);
	fwrite_safe("\x9C\x00\x01\x9C\x09\x01", 1, 6, ROMfp);

	address = (unsigned char)ROM[0x2E9] | ((unsigned char)ROM[0x2EE]<<8) | ((unsigned char)ROM[0x2F3]<<16);
	address = (macro_SNEStoPC(address));
	bool b_smkdsmws=false;
	if(address>=0x80200){
		char str_smkdsmws[16];
		fseek(ROMfp, address - 0x10, SEEK_SET);
		fread_safe(str_smkdsmws, 1, 16, ROMfp);
		if(!memcmp(str_smkdsmws, "smkdsmwsSTAR", 12)) b_smkdsmws=true;
		else
			if(memcmp(&str_smkdsmws[4], "STAR", 4) || memcmp(&str_smkdsmws[12], "Romi", 4)){
				printf("Sound effect can't be edited.\n");
				return 0;
			}
	}else{
		cstring SPCengine('\0', 0xE3E + 0x04);
		cstring SndTone('\0', 0xFC + 0x04);
		cstring SndPtr('\0', 0x1FC);
		cstring SndEff('\0', 0x86F);
		cstring bgmTone('\0', 0xBE);
		cstring DlmTone('\0', 0x3C);
		fseek(ROMfp, 0x70200, SEEK_SET);
		fread_safe(SPCengine.c_str(), 1, 0xE3E + 0x04, ROMfp);
		fread_safe(SndTone.c_str(), 1, 0xAB + 0x04, ROMfp);
		fread_safe(SndPtr.c_str(), 1, 0x68, ROMfp);
		fread_safe(SndPtr.c_str(0xFE), 1, 0x54, ROMfp);
		fread_safe(SndEff.c_str(), 1, 0x86F, ROMfp);
		fread_safe(bgmTone.c_str(),1, 0x5F, ROMfp);
		fread_safe(DlmTone.c_str(),1, 0x36, ROMfp);

		for(int i=0; i<0x1FC ;i+=2){
			if(*(unsigned short*)&SndPtr[i]) *(unsigned short*)&SndPtr[i]+= ((0x5570 + 0xFC + 0x1FC) - 0x56D7);
		}

		*(unsigned short*)&SndTone[0x00]=0xC61;
		SPCengine+=SndTone; SPCengine+=SndPtr; SPCengine+=SndEff; SPCengine+=bgmTone; SPCengine+=DlmTone;
		//Sound effect ptr $1DFC
		*(unsigned short*)&SPCengine[0x853 - 0x500 + 0x04] = 0x5570 + 0xFC - 0x02;
		*(unsigned short*)&SPCengine[0x858 - 0x500 + 0x04] = 0x5570 + 0xFC - 0x01;
		//Sound effect ptr $1DF9
		*(unsigned short*)&SPCengine[0x724 - 0x500 + 0x04] = 0x5570 + 0xFC + 0xFE - 0x02;
		*(unsigned short*)&SPCengine[0x729 - 0x500 + 0x04] = 0x5570 + 0xFC + 0xFE - 0x01;
		//bgmTone
		SPCengine[0xD51 - 0x500 + 0x04] = (unsigned char)(0x5570 + 0xFC + 0x1FC + 0x86F);
		SPCengine[0xD54 - 0x500 + 0x04] = (unsigned char)((0x5570 + 0xFC + 0x1FC + 0x86F)>>8);
		//DlmTone
		SPCengine[0x5D6 - 0x500 + 0x04] = (unsigned char)(0x5570 + 0xFC + 0x1FC + 0x86F + 0xBE);
		SPCengine[0x5D9 - 0x500 + 0x04] = (unsigned char)((0x5570 + 0xFC + 0x1FC + 0x86F + 0xBE)>>8);

		SPCengine.insert(0, "Romi", 4);
		SPCengine.append((int)0x05000000);
		unsigned int p;
		try{
			p = search_free_space_latter_no_bank(SPCengine.length() + RATS_TAG_SIZE);
		}
		catch(const char *str){
			puts(str);
			return 2;
		}
		write_with_RATS(p, SPCengine.length(), SPCengine.c_str());
		p-=0x1F4; p = macro_PCtoSNES(p);
		fseek(ROMfp, 0x2E9, SEEK_SET); fputc( (unsigned char)p, ROMfp);
		fseek(ROMfp, 0x2EE, SEEK_SET); fputc( (unsigned char)(p>>8), ROMfp);
		fseek(ROMfp, 0x2F3, SEEK_SET); fputc( (unsigned char)(p>>16), ROMfp);
	}

	cstring SE(0, 0x7F*2*2);
	memmove(&SE[0x00], &ROM[0x71159], 0x54);	//1DF9
	memmove(&SE[0xFE], &ROM[0x710F1], 0x68);	//1DFC

	int first_num[2] = { 0x2B, 0x35};
	for(int port=0; port<2 ; port++){
		printf("[%s]\n", (port ? "1DFC" : "1DF9"));
		for( int i=first_num[port] ; i<0x80 ; i++){
			char num[3], str[MAX_PATH], path[MAX_PATH];
			sprintf(num, "%02X", i);
			GetPrivateProfileString( (port ? "1DFC" : "1DF9"), num, "", str, MAX_PATH, IniPath);
			sprintf(path, "%s/%s", (port ? "1DFC" : "1DF9"), str);
			FILE *se = NULL;
			if(strlen(str) > 0 && fopen_safe(&se, path, "rb")){
				if( !strncmp("txt", &str[(strlen(str)-3)], 3)){//txt
					printf("Number: %s\n", num);
					mml_to_se(se, SE, i, port, b_smkdsmws);
				}else{//bin
					fseek(se, 0, SEEK_END);
					unsigned int size = ftell(se);
					fseek(se, 0, SEEK_SET);
					if(size){
						printf("Number: %s (binary file)\n", num);
						*(unsigned short*)&SE[port*0xFE + (i-1)*2] = (unsigned short)(b_smkdsmws ? (0x61D0) : (0x61D1) );
						for(unsigned int i=0; i<size ; i++) SE+=(char)fgetc(se);
					}
				}
				fclose(se);
			}
		}
	}

	if(SE.length()==0x1FC){
		SE.erase();
	}else{
		unsigned int size = SE.length();
		if((size - 0x1FC)>=(unsigned int)(b_smkdsmws ? 0x4CA : 0x4C9)){
			printf("Size is too large\n");
			return 1;
		}
		size-=0x1FC;
		size |= (b_smkdsmws ? 0x61D00000 : 0x61D10000);
		SE.insert(0x1FC, (const char *)&size, 4);
		SE.erase(0xFE, 0x68);
		size = (b_smkdsmws ? 0x57D80096 : 0x56D40096);
		SE.insert(0xFE, (const char *)&size, 0x04);
		SE.erase(0x00, 0x54);
		size = (b_smkdsmws ? 0x58C400AA : 0x57BE00AA);
		SE.insert(0x00, (const char *)&size, 0x04);
	}

	FILE *MORE_bin= NULL;
	if(fopen_safe(&MORE_bin, "MORE.bin", "rb") ){
		fseek(MORE_bin, 0, SEEK_END);
		unsigned int size = ftell(MORE_bin);
		fseek(MORE_bin, 0, SEEK_SET);
		for(unsigned int i=0; i<size ; i++) SE += (unsigned char)fgetc(MORE_bin);
		fclose(MORE_bin);
	}

	if( !SE.length() ){
		printf("There was no data to insert\n");
		return 1;
	}

	unsigned char MORE_asm[]={
		0x9C,0x00,0x01,0x9C,0x09,0x01,0xC2,0x10,0xA2,0x00,0x00,0xCA,0xD0,0xFD,0xE2,0x10,
		0xA9,0x00,0x85,0x00,0xA9,0x00,0x85,0x01,0xA9,0x00,0x85,0x02,0xF4,0x5A,0x80,0x5C,
		0x1D,0x81,0x80
	};
	unsigned int p;
	try{
		p = search_free_space(SE.length()+4+sizeof(MORE_asm)+RATS_TAG_SIZE);
	}
	catch(const char *str){
		puts(str);
		return 2;
	}
	unsigned int p2=p;
	p -= 0x200; p = macro_PCtoSNES(p);
	MORE_asm[0x11] = (unsigned char)(p+8+sizeof(MORE_asm));
	MORE_asm[0x15] = (unsigned char)((p+8+sizeof(MORE_asm))>>8);
	MORE_asm[0x19] = (unsigned char)((p+8+sizeof(MORE_asm))>>16);

	SE.insert( 0, (const char *)MORE_asm, sizeof(MORE_asm));
	SE.append((int)0x05000000);
	write_with_RATS(p2, SE.length(), SE.c_str());
	fseek(ROMfp, 0x255, SEEK_SET);
	putc( 0x5C, ROMfp);
	p+=8;
	fwrite_safe(&p, 1, 3, ROMfp); fwrite_safe("\xEA\xEA", 1, 2, ROMfp);

	printf("Inserted sound effects successfully, at 0x%06X\n", p2);
	return 0;
}

bool AddMusic::mml_to_se(FILE *txt, cstring& se, int num, int port, bool b_smkdsmws)
{
	cstring SE;
	fseek(txt, 0, SEEK_END);
	unsigned int size = ftell(txt);
	fseek(txt, 0, SEEK_SET);

	buf = (char *)malloc(size+1);
	fread_safe(buf, 1, size, txt);
	buf[size]=0;

	ptr=0;			//pointer of buffer
	line=1;			//the line number of txt
	unsigned int instr;

	int LastNote=-1;
	bool FirstNote=true;
	bool PitchSlide=false;

	octave=4;
	LastNoteValue= -1;
	DefNoteValue= 8;
	int Volume[2]={0x7F, 0x7F};
	SE_VolUpdate=true;

	b_error=false;
	triplet=false;

	int i,j;
	while(ptr<size){
		switch(buf[ptr]){
			case '!':
				ptr=0xFFFFFFFF;
				continue;
			case 'v':
				ptr++;
				i=GetInt(false);
				if(i<0 || 128<=i) error("Invalid volume value", true);
				else{
					Volume[0] = Volume[1] = i;
					if(buf[ptr]==','){
						ptr++;
						i=GetInt(false);
						if(i<0 || 128<=i) error("Invalid volume value", true);
						else Volume[1]=i;
					}
					SE_VolUpdate=true;
				}
				continue;
			case 'l':	//default note value
				ptr++;
				i=GetInt(false);
				if( 1<= i && i <= 192) DefNoteValue=i;
				else error("Invalid note value", true);
				continue;
			case '@':
				ptr++;
				i=GetInt(false);
				if(i==-1 || 128<=i){
					i=0;
					error("Invalid instrument number", true);
				}
				j=-1;
				if(buf[ptr]==','){
					ptr++;
					j=GetHex(false);
					if(!(0<=j && j<=0x1F)){
						j=-1;
						error("NCK value invalidly specified", true);
					}
				}
				if( 0<=i && i<= 0xFF){
					SE += (char)0xDA;
					if(j!=-1) SE+=(char)(j|0x80);
					SE += (char)i;
					instr=i;
				}
				continue;
			case 'o':
				ptr++;
				i=GetInt(false);
				if( 1<=i && i<=6) octave=i;
				else error("Invalid octave", true);
				continue;
			case '$':
				ptr++;
				i=GetHex(false);
				if( 0<=i && i<= 0xFF){
					SE += (char)i;
				}else error("Invalid hexadecimal value", true);
				continue;
			case '>':
				ptr++;
				if( ++octave > 7){
					octave=7;
					error("octave is too high", true);
				}
				continue;
			case '<':
				ptr++;
				if( --octave < 0){
					octave=0;
					error("octave is too low", true);
				}
				continue;
/*			case '&':
				ptr++;
				if(PitchSlide) error("'&' command specified repeatedly", true);
				else PitchSlide=true;
				continue;*/
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'a':
			case 'b':
			case 'r':
			case '^':
				j = buf[ptr++];
				i = (j=='r')?0xC7:
					(j=='^')?0xC6:GetPitch(j, true);
				if(i<0) i=0xC7;
				j=GetNoteValue( GetInt(false), true );

				if(i!=0xC6 && i!=0xC7 && (buf[ptr]=='&' || PitchSlide)){
					PitchSlide=true;
					if(FirstNote){
						if(LastNote==-1) LastNote=i;
						else{
							if(j>0){
								SE+=(char)j;
								LastNoteValue=j;
							}
							if(SE_VolUpdate){
								SE += (char)Volume[0];
								if( Volume[0] != Volume[1]) SE += (char)Volume[1];
								SE_VolUpdate=false;
							}
							SE+=(char)0xDD;
							SE+=(char)LastNote;
							SE+=(char)0x00;
							SE+=(char)LastNoteValue;
							SE+=(char)i;
							FirstNote=false;
							LastNote=i;
						}
					}else{
						if(j>0){
							SE+=(char)j;
							LastNoteValue=j;
						}
						if(SE_VolUpdate){
							SE += (char)Volume[0];
							if( Volume[0] != Volume[1]) SE += (char)Volume[1];
							SE_VolUpdate=false;
						}
						SE += (char)0xEB;
						SE += (char)0x00;
						SE += (char)LastNoteValue;
						SE += (char)i;
					}
					if(j>0) LastNoteValue=j;
					continue;
				}else FirstNote=true, PitchSlide=false;

				if(j>=0x80){			//Note value was too long
					SE += (char)0x7F;
					if(SE_VolUpdate){
						SE += (char)Volume[0];
						if( Volume[0] != Volume[1]) SE += (char)Volume[1];
						SE_VolUpdate=false;
					}
					SE += (char)i;				//pitch
					j-=0x7F;
					while(j>0x7F){
						j-=0x7F;
						SE += (char)0xC6;		//tie
					}
					if(j>0){
						if( j!=0x7F) SE += (char)j;
						SE += (char)0xC6;
					}
					LastNoteValue=j;
					continue;
				}else if(j>0){			//note value needed to be updated
					SE += (char)j;
					LastNoteValue=j;
					if(SE_VolUpdate){
						SE += (char)Volume[0];
						if( Volume[0] != Volume[1]) SE += (char)Volume[1];
						SE_VolUpdate=false;
					}
				}
				SE += (char)i;					//pitch
				continue;


			case '\n':
				ptr++, line++;
				continue;
			case ';':
				while( buf[ptr]!='\n' && buf[ptr]!=0) ptr++;
				continue;
			default:
				ptr++;
				continue;
		}//switch
	}//while

	SE += '\0';

	free(buf);
	buf=0;

	if(!b_error){
		*(unsigned short*)&se[port*0xFE + (num-1)*2] = (unsigned short)(se.length()+(b_smkdsmws ? (0x61D0 - 0x1FC) : (0x61D1 - 0x1FC) ));
		se +=  SE;
	}
	return (!b_error);
}
/***************************************************************
helper
***************************************************************/
void AddMusic::put(unsigned char c)
{
	OneSPC[ch] += (char)c;
	return;
}

int AddMusic::GetInt(bool msg)
{
	int i=0;	//number
	int d=0;	//digit

	while('0'<= buf[ptr] && buf[ptr] <= '9') d++, i = (i*10)+(buf[ptr++] - 0x30);
	if(!d){
		if(msg) error("Invalid decimal number", true);
		return -1;
	}

	return i;
}

int AddMusic::GetHex(bool msg)
{
	int i=0;
	int d=0;
	int j;

	while(1){
		if('0'<=buf[ptr]&&buf[ptr]<='9') j=buf[ptr++] - 0x30;
		else if('A'<=buf[ptr]&&buf[ptr]<='F') j=buf[ptr++] - 0x37;
		else if('a'<=buf[ptr]&&buf[ptr]<='f') j=buf[ptr++] - 0x57;
		else break;
		d++, i=(i*16)+j;
	}
	if(!d){
		if(msg) error("Invalid hexadecimal number", true);
		return -1;
	}
	return i;
}
int AddMusic::GetPitch(int i, bool SoundEffect)
{
	static const int Pitch[]={ 9, 11, 0, 2, 4, 5, 7 };

	i= Pitch[i - 0x61] + (octave-1)*12 + 0x80;
	if(buf[ptr]=='+') i++, ptr++;
	else if(buf[ptr]=='-') i--, ptr++;
	if(!SoundEffect)
		if( 0<=Instrument[ch] && Instrument[ch]<=18) i-=TRNSMAP[Instrument[ch]];
	if( i<0x80){
		error("Musical interval is too low", true);
		return -1;
	}
	else if(i>=0xC6){
		error("Musical interval is too high", true);
		return -1;
	}

	return i;
}
int AddMusic::GetNoteValue(int i, bool SoundEffect)
{
	bool still = true;
	if(i==-1 && buf[ptr]=='='){
		ptr++;
		i=GetInt(false);
		if( 1<=i && i<=192 ) still=false;
	}
	if(still){
		if( !(1 <= i && i <= 192) ) i=DefNoteValue;
		i = 192/i;
		if(buf[ptr]=='.'){
			if(buf[++ptr]=='.') ptr++, i = i*7/4;
			else i = i*3/2;
		}
		if(triplet) i = i*2/3;
	}
	if(!SoundEffect){
		if(i==LastNoteValue && !update_q[ch]) return 0;
	}else{
		if(i==LastNoteValue && !SE_VolUpdate) return 0;
	}
	return i;
}
void AddMusic::error(const char *msg, bool show_line)
{
	b_error=true;
		printf("ERROR : ");
		if(show_line) printf("%4i: ", line);
		printf("%s\n", msg);
	return;
}


void AddMusic::write_with_RATS(unsigned int address, unsigned int size, const char *b)
{
	if(!size || !address) return;
	char RATStag[RATS_TAG_SIZE]={'S','T','A','R', 0, 0, 0, 0};
	*(unsigned int*)&RATStag[4]= (size-1) | (((size-1)^0xFFFF)<<16);

	fseek(ROMfp, address, SEEK_SET);
	fwrite_safe(RATStag, 1, RATS_TAG_SIZE, ROMfp);
	fwrite_safe(b, 1, size, ROMfp);
	memset(&ROM[address], 'X', size+RATS_TAG_SIZE);
	return;
}
unsigned int AddMusic::search_free_space(unsigned int size)
{
	unsigned int MaxBank = (ROMsize - 0x200)/0x8000;
	for(unsigned int bank = 0x10; bank<MaxBank ; bank++)
		for(unsigned int i=0, j=0; i<0x8000; i++)
			if(ROM[bank*0x8000 + i + 0x200]){
				j=0;
				continue;
			}else if(++j == size) return (bank*0x8000 + i-(j-1) + 0x200);

	throw "Unable to find free space in your ROM";
}
unsigned int AddMusic::search_free_space_latter_no_bank(unsigned int size)
{
	for(unsigned int i=(ROMsize-1), j=0; i>=0x80200 ; i--)
		if(ROM[i]){
			j=0; continue;
		}else if( ++j == size) return i;

	throw "Unable to find free space in your ROM";
}
unsigned int AddMusic::RemoveRATS(unsigned int address)
{
	if( address<0x80200 || ROMsize<=address) return 0;
	if( !memcmp(&ROM[address-8], "STAR", 4) && (*(unsigned short*)&ROM[address-4]+*(unsigned short*)&ROM[address-2])==0xFFFF){
		unsigned int EraseSize = *(unsigned short*)&ROM[address-4] + 9;
		ZeroMemory(&ROM[address-8], EraseSize);
		fseek(ROMfp, address-8, SEEK_SET);
		fwrite_safe(&ROM[address-8], 1, EraseSize, ROMfp);
		return EraseSize;
	}

	return 0;
}
void AddMusic::MarkRATS(void)
{
	//for(unsigned int i=0x80200;i<ROMsize;){	//•ÛŒì—Ìˆæ–„‚ßs‚­‚µ
	//	if(*(unsigned int*)&ROM[i++] != 0x52415453) continue;				//STAR
	//	i+=3;
	//	int RATSsize = *(unsigned short *)&ROM[i];
	//	i+=2;
	//	int RATSsize2= *(unsigned short *)&ROM[i];
	//	i+=2;
	//	if(RATSsize != (RATSsize2^0xFFFF)) continue;		//³‚µ‚¢RATSƒ^ƒO‚©
	//	for(;RATSsize>=0;RATSsize--) ROM[i++]='X';		//”ñ0‚ð‘ã“ü
	//}
}
/***************************************************************
main
***************************************************************/

// Like normal AM405, but it only removes data, and then saves the ROM.
// This is...to ensure maximal compatibility.... Yeah.

int removeAM405Data(int argc, char *argv[])
{
	bool b_SE = false;
	bool custom_path = false;

	if(!(argc >= 2 && argc <= 5)){
		printf(
			"Unofficial AddMusic v4.04\n"
			"usage : [AddMusic.exe] [ROM.smc] (option)\n"
			"option\n"
			"-se : insert sound effect\n"
			"-i [file.ini]\n"
			);
		return 0;
	}
	if( argc == 3){
		if(!strcmp("-se", argv[2])) b_SE = true;
		else printf("Invalid option\n");
	}else if(argc == 4){
		if(!strcmp("-i", argv[2])){
			strcpy(ini_file_path, argv[3]);
			custom_path = true;
		}else{
			printf("Invalid option\n");
			return 0;
		}
	}else if(argc == 5){
		if(!strcmp("-se", argv[2])) b_SE = true;
		if(!strcmp("-i", argv[3])){
			strcpy(ini_file_path, argv[4]);
			custom_path = true;
		}else{
			printf("Invalid option\n");
			return 0;
		}
	}

	FILE *ROMFile = NULL;
	if(!fopen_safe(&ROMFile, argv[1], "rb+")){
		printf("Couldn't open the ROM file\n");
		return 1;
	}
	fseek(ROMFile, 0, SEEK_END);
	unsigned int ROMSize = ftell(ROMFile);
	fseek(ROMFile, 0, SEEK_SET);
	if( (ROMSize%0x8000)!=0x200 ){
		header = 0;
		ROMSize += 0x200;
	}
	unsigned char *ROMBuf=(unsigned char *)malloc(ROMSize);
	if(!ROMBuf){
		printf("Memory allocation failed\n");
		fclose(ROMFile);
		return 1;
	}

	if(!header){
		fread_safe(ROMBuf+0x200, 1, ROMSize-0x200, ROMFile);
		ZeroMemory(ROMBuf, 0x200);
		fseek(ROMFile, 0, SEEK_SET);
		fwrite_safe(ROMBuf,1,ROMSize,ROMFile);
	}else{
		fread_safe(ROMBuf, 1, ROMSize, ROMFile);
	}


	bool quit=false;

	if(ROMSize <= 0x80200){
		printf("The ROM must be expanded\n");
		quit=true;
	}
	if(quit){
		fclose(ROMFile);
		free(ROMBuf);
		return 1;
	}

	char CurrentPath[MAX_PATH];
	char IniPath[MAX_PATH];
	if(GetCurrentDirectory(MAX_PATH, CurrentPath) == NULL){
		return 1;
	}
	
	if(!custom_path){
		sprintf(IniPath, "%s/%s", CurrentPath, (b_SE ? "SoundEffect.ini" : "AddMusic.ini") );
	}else{
		sprintf(IniPath, "%s/%s", CurrentPath, ini_file_path );
	}

	AddMusic AM(ROMFile, ROMBuf, argv[1], ROMSize, IniPath);

	if(b_SE){
		AM.SoundEffect();
	}else{
		AM.initialize();
		}
	if(!header){
		fseek(ROMFile, 0x200, SEEK_SET);
		fread_safe(ROMBuf, 1, ROMSize-0x200, ROMFile);
		fseek(ROMFile, 0, SEEK_SET);
		fwrite_safe(ROMBuf,1,ROMSize-0x200,ROMFile);
	}
	fclose(ROMFile);
	free(ROMBuf);
	return 0;
}

