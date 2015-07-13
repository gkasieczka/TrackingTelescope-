#include "FileWriterTracking.h"
#include "GetNames.h"

using namespace std;

/** ============================
 CONSTRUCTOR
 =================================*/
FileWriterTracking::FileWriterTracking(string InFileName, uint8_t telescopeID, PSIFileReader * FR):
    nRoc(GetNumberOfROCS(telescopeID)) {

    NewFileName = getFileName(InFileName);
    intree = ((PSIRootFileReader*) FR)->fTree;
    newfile = new TFile(NewFileName.c_str(), "RECREATE");
    newtree = intree->CloneTree(0);
    br_charge_all.resize(nRoc);
    addBranches();

}
FileWriterTracking::~FileWriterTracking(){ }

/** ============================
 AUXILIARY FUNCTIONS
 =================================*/
string FileWriterTracking::getFileName(string InFileName){
    stringstream ss(InFileName);
    while (getline(ss, NewFileName, '/')){}
    NewFileName.insert(int(NewFileName.length()-5), "_withTracks");
    return NewFileName;
}
void FileWriterTracking::addBranches(){
    newtree->Branch("hit_plane_bits", &br_hit_plane_bits);
    newtree->Branch("diam1_track_x", &br_diam1_track_x);
    newtree->Branch("diam1_track_y", &br_diam1_track_y);
    newtree->Branch("diam2_track_x", &br_diam2_track_x);
    newtree->Branch("diam2_track_y", &br_diam2_track_y);
    newtree->Branch("dist_to_dia1", &br_dist_to_dia1);
    newtree->Branch("dist_to_dia2", &br_dist_to_dia2);
    newtree->Branch("chi2", &br_chi2);
    newtree->Branch("chi2_x", &br_chi2_x);
    newtree->Branch("chi2_y", &br_chi2_y);
    newtree->Branch("slope_x", &br_slope_x);
    newtree->Branch("slope_y", &br_slope_y);
    newtree->Branch("n_tracks", &br_n_tracks);
    newtree->Branch("n_clusters", &br_n_clusters);
    for (uint8_t iRoc = 0; iRoc != nRoc; iRoc++){
        TString branch_name = TString::Format("charge_all_ROC%d", iRoc);
        newtree->Branch(branch_name, &(br_charge_all[iRoc]));
    }
}
void FileWriterTracking::fillTree(){
    newtree->Fill();
}
void FileWriterTracking::saveTree(){
    newfile->cd();
    newtree->Write();
    newfile->Write();
    newfile->Close();
    delete newfile;

}
void FileWriterTracking::clearVectors(){
    for (uint8_t iRoc = 0; iRoc !=nRoc; iRoc++)
        br_charge_all[iRoc]->clear();
}

