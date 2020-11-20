#include <stdio.h>
#include "class_string.h"
#include "class_int.h"

#define MAX_CH (8)

#define RATS_TAG_SIZE (8)

#define OW_INSERT_AT	(0x1392)
#define OW_LIMIT		(0x41DE)
#define MISC_INSERT_AT	(0x6800)
#define MISC_LIMIT		(0x1800)
#define LEVEL_INSERT_AT (0x13A8)
#define LEVEL_LIMIT		(0x41C8)
class AddMusic
{
public:
	AddMusic(FILE *fp, unsigned char *ROMBuf, char *ROMFileName, unsigned int ROMFileSize, const char *IniFilePath) : 
		ROMfp(fp),
		ROM(ROMBuf),
		ROMname(ROMFileName),
		ROMsize(ROMFileSize),
		IniPath(IniFilePath),
		buf(NULL)
	{
		MarkRATS();
	}
	~AddMusic()
	{
		if(buf) free(buf);
	}


	int AMmain(void);
	int initialize(void);
	int Remove(void);
	int CreateNewINI(int version, bool bNew=false);

	int SoundEffect(void);
private:
	int convert(const char *FileName, const int MusicNum, int Num);
	void mml_to_spc(void);
	void adjust_address(int num);
	void save_to_rom(unsigned int limit, unsigned int TablePtr);

	bool mml_to_se(FILE *txt, cstring &se, int num, int port, bool b_smkdsmws);

	int GetInt(bool msg);
	int GetHex(bool msg);
	int GetPitch(int i, bool SoundEffect=false);
	int GetNoteValue(int i, bool SoundEffect=false);
	void put(unsigned char c);
	void error(const char *msg, bool show_line);

	void write_with_RATS(unsigned int address, unsigned int size, const char *b);
	unsigned int search_free_space(unsigned int size);
	unsigned int search_free_space_latter_no_bank(unsigned int size);
	unsigned int RemoveRATS(unsigned int address);
	void MarkRATS(void);

	int RemoveLevelMusic(unsigned int p, int musicnum, int offset);
	int RemoveMiscMusic(unsigned int low, unsigned int middle, unsigned int high);


private:
	FILE *ROMfp;
	unsigned char *ROM;
	char *ROMname;
	unsigned int ROMsize;
	const char *IniPath;


	unsigned int TableAddress;
	unsigned int OWMusicAddress;
	bool b_LevelMusic;
	bool b_common;

	unsigned int FileSize;	//txt file size
	char *buf;		//txt buffer
	unsigned int ptr;
	unsigned int line;
	bool b_inserted;
	bool b_error;

	int TRNSMAP[256];
	unsigned int ch;
	int LastNoteValue;
	int DefNoteValue;
	bool update_q[MAX_CH+1];
	bool triplet;
	int octave;
	int Instrument[MAX_CH+1];
	unsigned int ptrARAM[MAX_CH+1];
	unsigned int ptrIntro[MAX_CH];

	cint ptrLoop;
	cint ptrLabelLoop;
	cint ptrLabelNum;

	unsigned int ptrMusic[0x10];

	cstring OneSPC[MAX_CH+1];
	cstring SPC;

	bool HasIntro;
	bool NotHaveMusic[MAX_CH][2];
	bool WontLoop;

	bool SE_VolUpdate;
};
