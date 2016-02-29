#ifndef GUARD_PSIRootFileReader_h
#define GUARD_PSIRootFileReader_h

#include "PSIFileReader.h"

#include "TFile.h"
#include "TTree.h"
#include "GetNames.h"
#include <iomanip>

class PSIRootFileReader : public PSIFileReader
{
  public:
    PSIRootFileReader(  std::string const,
                        std::string const,
                        std::string const,
                        int const,
                        bool const,
                        bool const,
                        bool const onlyAlign = false,
                        uint8_t const TelescopeID=0);
    ~PSIRootFileReader ();

    bool OpenFile ();
    void ResetFile ();
    int GetNextEvent ();

    // Make tree accessible
    TTree * fTree;
    TFile * fRootFile;

  private:
    std::string fFileName;

    const bool fOnlyAlign;
    uint8_t telescopeID;

    //  Current entry and total number of entries
    int fAtEntry;
    int fNEntries;

    // Scalar Branches
    int f_event_number;
    float f_time;

    // Vector Branches
    std::vector<int> * f_plane;
    std::vector<int> * f_col;
    std::vector<int> * f_row;
    std::vector<int> * f_adc;
    std::vector<int> * f_charge;
    std::vector<float> * f_signal;
};

#endif
