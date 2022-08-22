#pragma once
#include <filesystem>
using namespace std::filesystem;

void parse_ifo(const char* input_src, struct _opts* option);
void writeChapter(double playback, struct _opts* option, struct _info* info);
void writeInitial(struct _opts* option);

struct _T_adtsheader {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
	uint8_t b4;
	uint8_t b5;
	uint8_t b6;
};

struct _latmheader {
	int chanelConfiguration;
	int samplingFrequency;
	int audioObjectType;
};
struct _info {
	uint32_t playback_hh;
	uint32_t playback_mm;
	uint32_t playback_ss;
	double playback_ms;
	uint32_t playback_no;
};

struct _opts {
	bool cue;
	path output;
	bool chapter;

};
