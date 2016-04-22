#ifndef DoAlignment_h
#define DoAlignment_h

#include "PSIBinaryFileReader.h"
#include "PSIRootFileReader.h"
#include "GetNames.h"
#include "TestPlaneEfficiencySilicon.h"
#include "PLTPlane.h"
#include "PLTAlignment.h"

int DoAlignment (std::string, TFile*, TString, int);
void print_progress(uint32_t, uint32_t);

#endif // DoAlignment_h