#ifndef GetNames_h
#define GetNames_h

#include <iostream>
#include <string>
#include <stdlib.h>

std::string GetCalibrationFilename(int telescopeID);
std::string GetAlignmentFilename(int telescopeID, bool useInitial=0);
std::string GetMaskingFilename(int telescopeID);
const char * GetSignalBranchName();
int GetNumberOfROCS(int telescopeID);
int GetNumberOfSignals(int);
int GetUseGainInterpolator(int telescopeID);
int GetUseExternalCalibrationFunction(int telescopeID);
int GetUseRootInput(int telescopeID);

#endif // GetNames_h