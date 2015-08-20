#include "PSIRootFileReader.h"

#include <iostream>
#include <string>
#include <stdint.h>
#include <stdlib.h>



PSIRootFileReader::PSIRootFileReader (std::string const InFileName,
				      std::string const CalibrationList,
				      std::string const AlignmentFileName,
				      int const nrocs,
				      bool const useGainInterpolator,
				      bool const useExternalCalibrationFunction
				      ) : PSIFileReader(CalibrationList,
							AlignmentFileName,
							nrocs,
							useGainInterpolator,
							useExternalCalibrationFunction)
{

  fFileName = InFileName;
  if (!OpenFile()) {
    std::cerr << "ERROR: cannot open input file: " << fFileName << std::endl;
    throw;
  }
}


PSIRootFileReader::~PSIRootFileReader ()
{
  Clear();
  delete fTree;

  // Crashed when uncommented. Live with the memleak for now
  //delete fRootFile;
}


bool PSIRootFileReader::OpenFile ()
{

  fRootFile = new TFile(fFileName.c_str(), "READ");

  if (!fRootFile->IsOpen())
    return false;

  fTree = (TTree*)fRootFile->Get("tree");

  fAtEntry = 0;
  fNEntries = fTree->GetEntries();

  if (!(fNEntries>0))
    return false;

  // Set Branch Addresses
  fTree->SetBranchAddress("event_number",&f_event_number);
  fTree->SetBranchAddress("time",&f_time);

  fTree->SetBranchAddress("plane", &f_plane);
  fTree->SetBranchAddress("col", &f_col);
  fTree->SetBranchAddress("row", &f_row);
  fTree->SetBranchAddress("adc", &f_adc);
  fTree->SetBranchAddress("charge", &f_charge);
  fTree->SetBranchAddress("sig", &f_signal);

  return true;
}

void PSIRootFileReader::ResetFile ()
{
  delete fTree;
  delete fRootFile;
  OpenFile();
}

int PSIRootFileReader::GetNextEvent (){

    Clear();
    ClearSignal();
    AddSignal(*f_signal);

    for (int i = 0; i != NMAXROCS; ++i)
        fPlaneMap[i].SetROC(i);

    if (fAtEntry == fNEntries)
        return -1;

    fTree->GetEntry(fAtEntry);
    fAtEntry++;



  for (uint8_t iHit = 0; iHit != f_plane->size(); iHit++){

    int roc = (*f_plane)[iHit];
    int col = (*f_col)[iHit];
    int row = (*f_row)[iHit];
    int adc = (*f_adc)[iHit];

    if (!IsPixelMasked( 1*100000 + roc*10000 + col*100 + row)){
      PLTHit* Hit = new PLTHit(1, roc, col, row, adc);

      // TODO: Either use calibration or read charge from root file
      //if (fUseGainInterpolator)
      //  fGainInterpolator.SetCharge(*Hit);
      //else
      //  fGainCal.SetCharge(*Hit);

      // Dummy calibration for now
      //Hit->SetCharge((*f_adc)[iHit] * 65);

      //std::cout << roc << " " << col << " " << row <<
      fGainCal.SetCharge(*Hit);
      //std::cout << "  " << Hit->Charge() << std::endl;
      /** FIXME: add correct number here*/
//      if (roc < 4) { }
      fAlignment.AlignHit(*Hit);
      fHits.push_back(Hit);
      fPlaneMap[Hit->ROC()].AddHit(Hit);
    }
  }

  // Loop over all planes and clusterize each one, then add each plane to the correct telescope (by channel number
  for (std::map< int, PLTPlane>::iterator it = fPlaneMap.begin(); it != fPlaneMap.end(); ++it) {
    it->second.Clusterize(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
    AddPlane( &(it->second) );
  }


  // If we are doing single plane-efficiencies:
  // Just send all events to the tracking and sort it out there
  if (DoingSinglePlaneEfficiency()){
    RunTracking( *((PLTTelescope*) this));
  }
  // Otherwise require exactly one hit per plane
  else {
    // 6 AND 3F FOR 6PLANES: TODO -> make dynamic!!!
    if (NClusters() == 4 && HitPlaneBits() == 0xf) {
      RunTracking( *((PLTTelescope*) this));
    }
  }

  return 0;

}
