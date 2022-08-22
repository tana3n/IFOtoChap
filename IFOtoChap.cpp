// IFOtoChap.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <fstream>
#include <sstream>

#include <bitset>
#include <string>
#include <filesystem>
#include <windows.h>

#include "IFOtoChap.hpp"

using namespace std::filesystem;

std::string appname = "IfoDumpCell";
std::string Version = "1.0.0";

void usage() {
    std::cout << "Usage: " + appname + "[options] <input> \n";

    std::cout << "--chapter \ttOutput Nero's Chapter Sheet.\n";
    std::cout << "--cue \tOutput cue Sheet.\n";

    return;
}
void cli_parser(char* argopts[], int optsum) {

    std::ifstream ifs(argopts[optsum - 1]);
    if (!ifs.is_open()) {
        std::cout << "[Error]Failed read Input File\n\n";
        return;
    }
    struct _opts option;
    option.cue = true;
    option.chapter = false;

    for (int i = 0; i < optsum; i++) {
        if (!_stricmp(argopts[i], "--cue")) {
            option.cue = true;
            continue;
        }
        if (!_stricmp(argopts[i], "--chapter")) {
            option.chapter = true;
            continue;
        }
    }
    parse_ifo(argopts[optsum - 1], &option);

    return;
}
uint32_t hex2dec(uint8_t input) {
    std::stringstream ss;
    ss << std::hex << int(input);
    return uint32_t(std::stoi(ss.str().c_str(), nullptr, 10));
}

void parse_ifo(const char* input_src, struct _opts *option) {
    struct _info info;

    std::ifstream input(input_src, std::ios::in | std::ios::binary);
    if (!input) {
        std::cout << "Can not open source: " << input_src;
        return;
    }
    char* iBuf = new char[1048576];
    input.read(iBuf, 1048576);
    //check header for DVDVIDEO-VTS
    if ((iBuf[9] != 'V') | (iBuf[10] != 'T') | (iBuf[11] != 'S')) {
        std::cout << "NotFound DVDVIDEO-VTS header" << input_src;
        return;
    }
    std::cout << "PGCN's count:" << uint16_t(iBuf[0x1000]) << uint16_t(iBuf[0x1001]) << std::endl;
    uint16_t PGCNs = iBuf[0x1001];
    uint32_t endAddress = (iBuf[0x1004] << 24) & 0xFFFFFFFF | (iBuf[0x1005] << 16) & 0xFFFFFF | (iBuf[0x1006] << 8) & 0xFFFF | (iBuf[0x1007] & 0xFF);

    std::cout << "PGCN EndAddress:" 
        << std::hex << endAddress
        << std::endl;
    uint32_t current = 0x1008;
    uint16_t i = 0;

    while (i < PGCNs){
        uint32_t Offsets = ( (iBuf[current + 4] & 0xFF) << 24) | ( (iBuf[current + 5] & 0xFF) << 16) | ( (iBuf[current + 6] & 0xFF) << 8) | ( (iBuf[current + 7] & 0xFF) );
        std::cout << "PGCN[" << i + 1 << "]StartByte: 0x"
            << std::hex << Offsets 
            << std::endl;
        uint32_t PGCN_Offset = Offsets + 0x1000;
        uint16_t PGCN_Cells = iBuf[PGCN_Offset + 0x03];
        std::cout << "PGCN_Cells count:" << std::dec << PGCN_Cells << std::endl;

        uint32_t pcgPlayback_offset =( ( (iBuf[PGCN_Offset + 0xE8] & 0xFF) << 8) | (iBuf[PGCN_Offset + 0xE9]&0xFF)) + PGCN_Offset;

        //ここでPGC単位のParseをして書き出すと良い
        std::cout << "cell playback information table[" << i + 1 << "] StartByte: 0x"
            << std::hex << pcgPlayback_offset
            << std::endl;
        uint16_t j = 0;
        double playback_ms = 0;
        uint32_t fN = 0;
        if (PGCN_Cells > 1) writeInitial(option);
        else  std::cout << "[Parser]This PGCN cannot find chapter infomation" << std::endl;
        while (j < PGCN_Cells - 1) {
            uint32_t tmp_hh = hex2dec(iBuf[pcgPlayback_offset + 4]);
            uint32_t tmp_mm = hex2dec(iBuf[pcgPlayback_offset + 5]);
            uint32_t tmp_ss = hex2dec(iBuf[pcgPlayback_offset + 6]);
            uint32_t tmp_ms = iBuf[pcgPlayback_offset + 7];
            uint32_t frameNum = tmp_ms & 0x3F;
            uint32_t playback_ff = tmp_ms >> 6 & 0x3;
            //playback_time += (tmp_hh * 60 * 60 + tmp_mm * 60 + tmp_ss) * 1000 + tmp_ms;

            uint32_t tM = 60 * tmp_hh + tmp_mm;
            fN =fN + (60 * 60 * 30 * tmp_hh) + (60 * 30 * tmp_mm) + (30 * tmp_ss) + hex2dec(frameNum) -(2 * (tM - tM / 10));
            
            uint32_t D = fN / 17982;

            uint32_t M = fN % 17982;
            uint32_t L = 0;
            
            if (M >= 2) L = (M - 2) / 1798;
            uint32_t fN2 = fN -(D * 18) - (2 * L);
            //std::cout << std::dec << "fN: " << fN<< "\n";
//            std::cout << std::setfill('0') << std::right << std::setw(2) << std::dec << "CHAPTER" << j <<"=" << double(fN) * 1001.0 /30000.0<< std::endl;
            /*
            playback_hh += playback_time / 1000 / 60 / 60 ;// hh
            playback_mm += (fN) * 1001.0 / 30000.0 % 60;
            playback_ss += fN * 1001.0 / 30000.0 % 60;
            playback_ms += uint32_t(playback_time / 1000) % 60;
            */
            //std::cout << "fN2: " << fN2 << "\n";
            info.playback_ss = uint32_t(double(fN2) * 1001.0 / 30000.0) % 60;
            info.playback_hh = uint32_t(double(fN2) * 1001.0 / 30000.0) / 60 / 60;
            info.playback_mm = uint32_t(double(fN2) * 1001.0 / 30000.0) / 60 % 60;
            info.playback_ms = double(fN2) * 1001.0 / 30000.0 - uint32_t(double(fN2) * 1001.0 / 30000.0);
            info.playback_no = j + 2;
            writeChapter(fN2, option, &info);

            j = j + 1;
            pcgPlayback_offset = pcgPlayback_offset + 24;

        }
        current = current + 8;
        i = i + 1;
        std::cout <<std::endl;
    }

}

void writeChapter(double playback, struct _opts* option, struct _info* info) {
    if (option->chapter) {
        std::cout << std::setw(2) << std::setfill('0') << std::dec << "CHAPTER";
        std::cout << std::setw(2) << std::setfill('0') << info->playback_no;
        std::cout << "=";
        std::cout << std::setw(2) << std::setfill('0') << info->playback_hh;
        std::cout << ":";
        std::cout << std::setw(2) << std::setfill('0') << info->playback_mm;
        std::cout << ":";
        std::cout << std::setw(2) << std::setfill('0') << info->playback_ss;
        std::cout << ":" << round(info->playback_ms * 1000) << std::endl;

        std::cout << "CHAPTERNAME";
        std::cout << std::setfill('0') << std::right << std::setw(2) << info->playback_no;
        std::cout << "=" << "Chapter " << info->playback_no << std::endl;
    }
    if (option->cue) {
        std::cout << "  TRACK " << std::setfill('0') << std::dec << info->playback_no << " AUDIO" << std::endl;
        std::cout << "    TITLE \"Track " << std::setfill('0') << std::dec << info->playback_no << "\"" << std::endl ;
        std::cout << "    INDEX 01 ";
        if (info->playback_hh >= 1) info->playback_mm = info->playback_mm * (info->playback_hh % 60 ) * 60;
        std::cout << std::setw(2) << std::setfill('0') << info->playback_mm;
        std::cout << ":";
        std::cout << std::setw(2) << std::setfill('0') << info->playback_ss;
        std::cout << ":" << std::setw(2) << std::setfill('0') << floor(info->playback_ms * 100) << std::endl;

    }
}

void writeInitial(struct _opts* option) {
    if (option->chapter) {
        std::cout << "CHAPTER01=00:00:00:000" << std::endl;
        std::cout << "CHAPTERNAME01=Chapter 1" << std::endl;
    }
    if (option->cue) {

        std::cout << "PERFORMER \"" << "VTS_01_1.IFO" << "\"" << std::endl;
        std::cout << "TITLE \"" << "VTS_01_1.IFO" << "\"" << std::endl;
        std::cout << "REM COMPOSER \"\""<< std::endl;
        std::cout << "FILE \"" << "VTS_01_1.WAV" << "\" WAVE" << std::endl;
        std::cout << "  TRACK 01 AUDIO" << std::endl;
        std::cout << "    TITLE \"Track 01\"" << std::endl;
        std::cout << "    INDEX 01 00:00:00" << std::endl;
    }

}
int main(int argc, char* argv[]) {
    std::cout << appname <<" Version " << Version << "\n\n";

    if (!argv[1]) {
        usage();

        return 0;
    }
    cli_parser(argv, argc);

}

