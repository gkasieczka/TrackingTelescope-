#ifndef GUARD_PSIBinaryFileReader_h
#define GUARD_PSIBinaryFileReader_h

#include <fstream>
#include <set>

#include "PSIFileReader.h"

class PSIBinaryFileReader : public PSIFileReader
{
  public:
    PSIBinaryFileReader(std::string const InFileName);
    ~PSIBinaryFileReader ();

    bool OpenFile ();
    void ResetFile ();
    unsigned GetEntries() override { return 0; }

    bool ReadAddressesFromFile (std::string const);

    unsigned short readBinaryWordFromFile ();
    int nextBinaryHeader ();
    int decodeBinaryData ();
    int GetNextEvent ();
	void CloseFile();
    int CalculateLevels (TString const OutDir = "plots/");
    int LevelInfo (int const Value, int const iroc);
    std::pair<int, int> fill_pixel_info(int* evt , int ctr, int iroc);
    void DecodeHits ();

    void DrawWaveform(TString const);

  private:
    int fHeader;
    int fNextHeader;

    static int const MAXNDATA = 2000;
    int fBuffer[MAXNDATA];
    int fBufferSize;
    bool fEOF;
    std::ifstream fInputBinaryFile;
    unsigned int fUpperTime;
    unsigned int fLowerTime;

    int fData[MAXNDATA];

    static int const UBLevel = -680;
    std::vector< std::vector< float > > fLevelsROC;

    std::string fBinaryFileName;
    
};

#endif
