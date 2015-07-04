#include "TestBinaryFileReader.h"
#include "RootItems.h"

using namespace std;

/** ============================
 RUN DEFAULT ANALYSIS
 =================================*/
int TestPSIBinaryFileReader (string const InFileName, TFile * out_f,  TString const RunNumber,
                             int telescopeID)
{

    /** ============================
     Constants
     =================================*/
    const uint16_t NROC = GetNumberOfROCS(telescopeID);
    const uint32_t ph_threshold(300000);
    bool do_slope;
    /** set output directory and gStyle */
    TString const PlotsDir = "plots/";
    TString const OutDir = PlotsDir + RunNumber + "/";
    cout << OutDir << endl;
    gStyle->SetOptStat(0);


    /** ============================
     Single Plane Studies
     =================================*/
    if ((telescopeID == 1) || (telescopeID == 2)){
        int n_events = TestPlaneEfficiencySilicon(InFileName, out_f, RunNumber, telescopeID);
        for (int iplane = 1; iplane != 5; iplane++){
            cout << "Going to call TestPlaneEfficiency " << iplane << endl;
            TestPlaneEfficiency(InFileName, out_f, RunNumber, iplane, n_events, telescopeID);
        }
    }


    /** ============================
     Initialize File Reader
     =================================*/
    PSIFileReader * FR;

    if (GetUseRootInput(telescopeID)){
        FR = new PSIRootFileReader(InFileName, GetCalibrationFilename(telescopeID), GetAlignmentFilename(telescopeID),
            GetNumberOfROCS(telescopeID), GetUseGainInterpolator(telescopeID), GetUseExternalCalibrationFunction(telescopeID));
    }
    else {
        FR = new PSIBinaryFileReader(InFileName, GetCalibrationFilename(telescopeID), GetAlignmentFilename(telescopeID),
            GetNumberOfROCS(telescopeID), GetUseGainInterpolator(telescopeID), GetUseExternalCalibrationFunction(telescopeID));
        ((PSIBinaryFileReader*) FR)->CalculateLevels(10000, OutDir);
    }
    FR->GetAlignment()->SetErrors(telescopeID);
    FILE * f = fopen("MyGainCal.dat", "w");
    FR->GetGainCal()->PrintGainCal(f);
    fclose(f);

    /** apply masking */
    FR->ReadPixelMask(GetMaskingFilename(telescopeID));


    /** ============================
     Prepare Root Items
     =================================*/
    RootItems RootItems(telescopeID, RunNumber);


//    for (uint16_t iroc = 0; iroc != NROC; ++iroc){
//        hNHitsPerCluster.push_back( TH1F( Form("NHitsPerCluster_ROC%i",iroc),
//                                          Form("NHitsPerCluster_ROC%i",iroc), 10, 0, 10));
//    }
//
//    for (uint16_t iroc = 0; iroc != NROC; ++iroc){
//        hNClusters.push_back( TH1F( Form("NClusters_ROC%i",iroc),
//                                    Form("NClusters_ROC%i",iroc), 10, 0, 10));
//    }

    /** ============================
     Prepare Coincidence Histogram
     =================================*/
    uint16_t n_bins = pow(2, NROC);
    TH1F hCoincidenceMap("CoincidenceMap", "CoincidenceMap", n_bins, 0, n_bins);
    vector<string> bin;
    for (uint16_t i(0); i < n_bins; i++){
        bin.push_back("");
        int z = i;
        for (int16_t j(NROC - 1); j>=0; j--){
            int x = pow(2,j);
            int y = z/x;
            if (y == 1) z -= x;
            bin[i].push_back(y+'0');
        }
    }
    hCoincidenceMap.LabelsDeflate();
    hCoincidenceMap.SetFillColor(40);
    hCoincidenceMap.GetYaxis()->SetTitle("Number of Hits");
    hCoincidenceMap.GetYaxis()->SetTitleOffset(0.5);
    hCoincidenceMap.GetYaxis()->SetTitleSize(0.04);
    hCoincidenceMap.GetXaxis()->SetTitle("Coincidences");
    hCoincidenceMap.GetXaxis()->SetTitleSize(0.04);
    hCoincidenceMap.GetXaxis()->SetLabelOffset(99);
    Float_t x, y;
    TText t;
    t.SetTextAngle(0);
    t.SetTextSize(0.04);
    t.SetTextAlign(22);
    cout << endl;

    /** ============================
     Prepare PulseHeight histograms
     ================================= */
    int const phMin = 0;
    int const phNBins = 50;
    /** standard */
    TH1F * hPulseHeight[NROC][4];
    int const phMax = 50000;
    for (uint16_t iroc = 0; iroc != NROC; ++iroc) {
        TString Name = TString::Format("PulseHeight_ROC%i_All", iroc);
        hPulseHeight[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeight_ROC%i_NPix1", iroc);
        hPulseHeight[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeight_ROC%i_NPix2", iroc);
        hPulseHeight[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeight_ROC%i_NPix3Plus", iroc);
        hPulseHeight[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
    }
    /** long histogram (see the protons) */
    TH1F * hPulseHeightLong[NROC][4];
    int const phLongMax = 300000;
    for (uint16_t iroc = 0; iroc != NROC; ++iroc) {
        TString Name = TString::Format("PulseHeightLong_ROC%i_All", iroc);
        hPulseHeightLong[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
        Name = TString::Format("PulseHeightLong_ROC%i_NPix1", iroc);
        hPulseHeightLong[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
        Name = TString::Format("PulseHeightLong_ROC%i_NPix2", iroc);
        hPulseHeightLong[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
        Name = TString::Format("PulseHeightLong_ROC%i_NPix3Plus", iroc);
        hPulseHeightLong[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phLongMax);
    }
    /** For Tracks, using additional selections*/
    TH1F* hPulseHeightOffline[NROC][4];
    for (uint16_t iroc = 0; iroc != NROC; ++iroc) {
        TString Name = TString::Format("PulseHeightOffline_ROC%i_All", iroc);
        hPulseHeightOffline[iroc][0] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeightOffline_ROC%i_NPix1", iroc);
        hPulseHeightOffline[iroc][1] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeightOffline_ROC%i_NPix2", iroc);
        hPulseHeightOffline[iroc][2] = new TH1F(Name, Name, phNBins, phMin, phMax);
        Name = TString::Format("PulseHeightOffline_ROC%i_NPix3Plus", iroc);
        hPulseHeightOffline[iroc][3] = new TH1F(Name, Name, phNBins, phMin, phMax);
    }
    /** formatting histos */
    int const HistColors[4] = { 1, 4, 28, 2 };
    for (uint16_t iroc = 0; iroc != NROC; ++iroc) {
        for (uint16_t inpix = 0; inpix != 4; ++inpix) {
            hPulseHeight[iroc][inpix]->SetXTitle("Charge (electrons)");
            hPulseHeight[iroc][inpix]->SetYTitle("Number of Clusters");
            hPulseHeight[iroc][inpix]->SetLineColor(HistColors[inpix]);
            hPulseHeightLong[iroc][inpix]->SetXTitle("Charge (electrons)");
            hPulseHeightLong[iroc][inpix]->SetYTitle("Number of Clusters");
            hPulseHeightLong[iroc][inpix]->SetLineColor(HistColors[inpix]);
            hPulseHeightOffline[iroc][inpix]->SetXTitle("Charge (electrons)");
            hPulseHeightOffline[iroc][inpix]->SetYTitle("Number of Clusters");
            hPulseHeightOffline[iroc][inpix]->SetLineColor(HistColors[inpix]);
        }
    }

    /** ============================
     2D Pulse Height maps for All and Track6
     =================================*/
    double AvgPH2D[NROC][PLTU::NCOL][PLTU::NROW];
    int NAvgPH2D[NROC][PLTU::NCOL][PLTU::NROW];
    for (uint16_t i = 0; i != NROC; ++i)
        for (uint16_t icol = 0; icol != PLTU::NCOL; ++icol)
            for (uint16_t irow = 0; irow != PLTU::NROW; ++irow) {
                AvgPH2D[i][icol][irow] = 0;
                NAvgPH2D[i][icol][irow] = 0;
            }


    /** ============================
     Pulse height average counts and averages, define TGraphErrors
     =================================*/
    int NAvgPH[NROC][4];
    double AvgPH[NROC][4];
    vector<vector<TGraphErrors> > gAvgPH;
    /** formatting graphs */
    for (int i = 0; i != NROC; ++i) {
        vector<TGraphErrors> tmp_gr_vector;
        for (uint16_t j = 0; j != 4; ++j) {
            NAvgPH[i][j] = 0;
            AvgPH[i][j] = 0;
            TGraphErrors gr;
            gr.SetName( Form("PulseHeightTime_ROC%i_NPix%i", i, j) );
            gr.SetTitle( Form("Average Pulse Height ROC %i NPix %i", i, j) );
            gr.GetXaxis()->SetTitle("Event Number");
            gr.GetYaxis()->SetTitle("Average Pulse Height (electrons)");
            gr.SetLineColor(HistColors[j]);
            gr.SetMarkerColor(HistColors[j]);
            gr.SetMinimum(0);
            gr.SetMaximum(60000);
            gr.GetXaxis()->SetTitle("Event Number");
            gr.GetYaxis()->SetTitle("Average Pulse Height (electrons)");
            tmp_gr_vector.push_back(gr);
        }
        gAvgPH.push_back(tmp_gr_vector);
    }

    /** Track Chi2 Distribution */
    TH1F hChi2("Chi2", "Chi2", 240, 0., 20.);

    /** Track Chi2 Distribution */
    TH1F hChi2X("Chi2X", "Chi2X", 240, 0., 20.);

    /** Track Chi2 Distribution */
    TH1F hChi2Y("Chi2Y", "Chi2Y", 240, 0., 20.);



    /** ============================
     Residual histograms
     =================================
     -> hResidual:    x=dX / y=dY
     -> hResidualXdY: x=X  / y=dY
     -> hResidualYdX: x=Y  / y=dX */
    vector<TH2F> hResidual;
    vector<TH2F> hResidualXdY;
    vector<TH2F> hResidualYdX;

    for (uint16_t iroc = 0; iroc != NROC; ++iroc){
        hResidual.push_back(TH2F(Form("Residual_ROC%i",iroc),
                                 Form("Residual_ROC%i",iroc), 100, -.15, .15, 100, -.15, .15));
        hResidualXdY.push_back(TH2F(Form("ResidualXdY_ROC%i",iroc),
                                    Form("ResidualXdY_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
        hResidualYdX.push_back(TH2F(Form("ResidualYdX_ROC%i",iroc),
                                    Form("ResidualYdX_ROC%i",iroc), 200, -1, 1, 100, -.5, .5));
    }

    float_t  onepc[NROC];
    float_t  twopc[NROC];
    float_t threepc[NROC];


    /** ============================
     Times for Counting
     ================================= */
    int const TimeWidth = 20000;
    int NGraphPoints = 0;
    int const StartTime = 0;
    int ThisTime;


    /** ============================
     Stuff for root input
     =================================
    Create a couple of pointers, input to add the tracks to the output */
    TTree * intree;
    TFile * newfile;
    TTree * newtree;

    int br_hit_plane_bits;
    float br_diam1_track_x, br_diam1_track_y;
    float br_diam2_track_x, br_diam2_track_y;
    float br_chi2_x,br_chi2_y;
    float br_slope_x, br_slope_y;
    uint8_t br_n_tracks, br_n_clusters;
    float br_charge_all;
    float br_charge_1pix;
    float br_charge_2pix;
    float br_charge_3pixplus;

    /** Pointer to the actual input root tree. Only works for the
        root-file producer. Ugly hack to avoid opening the same ROOT file twice */
    if (GetUseRootInput(telescopeID) && (telescopeID==7)){

    /** Extract filename if a full path is given */
    stringstream ss(InFileName);
    string newfile_name;
    while (getline(ss, newfile_name, '/')){}

    newfile_name.insert(int(newfile_name.length()-5), "_withTracks");

    intree = ((PSIRootFileReader*) FR)->fTree;
    newfile = new TFile(newfile_name.c_str(), "RECREATE");
    newtree = intree->CloneTree(0); // Do no copy the data yet

    newtree->Branch("hit_plane_bits", &br_hit_plane_bits);
    newtree->Branch("diam1_track_x", &br_diam1_track_x);
    newtree->Branch("diam1_track_y", &br_diam1_track_y);
    newtree->Branch("diam2_track_x", &br_diam2_track_x);
    newtree->Branch("diam2_track_y", &br_diam2_track_y);
    newtree->Branch("chi2_x", &br_chi2_x);
    newtree->Branch("chi2_y", &br_chi2_y);
    newtree->Branch("slope_x", &br_slope_x);
    newtree->Branch("slope_y", &br_slope_y);
    newtree->Branch("n_tracks", &br_n_tracks);
    newtree->Branch("n_clusters", &br_n_clusters);
    newtree->Branch("charge_all", &br_charge_all);
    newtree->Branch("charge_1pix", &br_charge_1pix);
    newtree->Branch("charge_2pix", &br_charge_2pix);
    newtree->Branch("charge_3pixplus", &br_charge_3pixplus);
    }

    /** ============================
     Event Loop
     ================================= */
    uint64_t indexi = -1;
    uint64_t start = clock(), now = clock();
    for (int ievent = 0; FR->GetNextEvent() >= 0; ++ievent) {

        indexi++;
        ThisTime = ievent;

        /** print progress */
        if (ievent % 10000 == 0){
            cout << "Processing event: " << ievent << endl;
            cout << "elapsed time: " << float((clock() - now)) / CLOCKS_PER_SEC << endl;
            now = clock();
        }

        /** Write out information for PAD studies */
        if (GetUseRootInput(telescopeID)){

            br_hit_plane_bits = FR->HitPlaneBits();

            br_n_tracks = FR->NTracks();
            br_n_clusters = FR->NClusters();

            if (FR->NTracks() > 0){
                PLTTrack * Track = FR->Track(0);

                /** fill branches */
                br_chi2_x = Track->Chi2X();
                br_chi2_y = Track->Chi2Y();
                br_slope_x = Track->fSlopeX;
                br_slope_y = Track->fSlopeY;
            }

            /** z-coordinate of the diamonds */
            float diam1_z = 3.5; // cm from front (front diamond)
            float diam2_z = 5.5;
            /** add track info */
            if (FR->NTracks()==1){
                PLTTrack * Track = FR->Track(0);
                br_diam1_track_x = Track->ExtrapolateX(diam1_z);
                br_diam1_track_y = Track->ExtrapolateY(diam1_z);
                br_diam2_track_x = Track->ExtrapolateX(diam2_z);
                br_diam2_track_y = Track->ExtrapolateY(diam2_z);
            }
            else {
                br_diam1_track_x = -999.;
                br_diam1_track_y = -999.;
                br_diam2_track_x = -999.;
                br_diam2_track_y = -999.;
                br_slope_x = -999.;
            }
            newtree->Fill();
        }

        /** fill coincidence map */
        hCoincidenceMap.Fill(FR->HitPlaneBits());

        /** make average pulseheight maps*/
        if (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) {
            for (uint16_t i = 0; i != NROC; ++i) {
                for (uint16_t j = 0; j != 4; ++j) {
                    gAvgPH[i][j].Set(NGraphPoints+1);
                    gAvgPH[i][j].SetPoint(NGraphPoints, ThisTime - TimeWidth/2, AvgPH[i][j]);
                    gAvgPH[i][j].SetPointError(NGraphPoints, TimeWidth/2, AvgPH[i][j]/sqrt((float) NAvgPH[i][j]));
                    printf("AvgCharge: %i %i N:%9i : %13.3E\n", i, j, NAvgPH[i][j], AvgPH[i][j]);
                    NAvgPH[i][j] = 0;
                    AvgPH[i][j] = 0;
                }
            }
          ++NGraphPoints;
        }

        /** draw tracks if there is more than one hit*/
        static int ieventdraw = 0;
        if (ieventdraw < 20) {
            uint16_t hp = FR->HitPlaneBits();
            if (hp != 0 && hp != 1 && hp != 2 && hp != 4 && hp != 8)
                FR->DrawTracksAndHits(TString::Format(OutDir + "/Tracks_Ev%i.gif", ++ieventdraw).Data() );
        }

        /** loop over the planes */
        for (size_t iplane = 0; iplane != FR->NPlanes(); ++iplane) {

            PLTPlane* Plane = FR->Plane(iplane);

            /** fill cluster histo */
            RootItems.nClusters()[Plane->ROC()]->Fill(Plane->NClusters());

            for (size_t icluster = 0; icluster != Plane->NClusters(); ++icluster) {
                PLTCluster* Cluster = Plane->Cluster(icluster);

                if (iplane < NROC) {
                    /** fill pulse height histo for all*/
                    hPulseHeight[iplane][0]->Fill(Cluster->Charge());
                    hPulseHeightLong[iplane][0]->Fill(Cluster->Charge());
                    br_charge_all = Cluster->Charge();

                    /** ignore charges above a certain threshold */
                    if (Cluster->Charge() > ph_threshold) continue;

                    /** average pulse heights */
                    PLTU::AddToRunningAverage(AvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], NAvgPH2D[iplane][Cluster->SeedHit()->Column()][ Cluster->SeedHit()->Row()], Cluster->Charge());
                    PLTU::AddToRunningAverage(AvgPH[iplane][0], NAvgPH[iplane][0], Cluster->Charge());

                    /** fill pulse height histo one pix */
                    if (Cluster->NHits() == 1) {
                        hPulseHeight[iplane][1]->Fill(Cluster->Charge());
                        onepc[iplane]++;
                        hPulseHeightLong[iplane][1]->Fill(Cluster->Charge());
                        PLTU::AddToRunningAverage(AvgPH[iplane][1], NAvgPH[iplane][1], Cluster->Charge());
                        br_charge_1pix = Cluster->Charge();
                    }

                    /** fill pulse height histo two pix */
                    else if (Cluster->NHits() == 2) {
                        hPulseHeight[iplane][2]->Fill(Cluster->Charge());
                        twopc[iplane]++;
                        hPulseHeightLong[iplane][2]->Fill(Cluster->Charge());
                        PLTU::AddToRunningAverage(AvgPH[iplane][2], NAvgPH[iplane][2], Cluster->Charge());
                        br_charge_2pix = Cluster->Charge();
                    }
                    /** fill pulse height histo >3 pix */
                    else if (Cluster->NHits() >= 3) {
                        hPulseHeight[iplane][3]->Fill(Cluster->Charge());
                        threepc[iplane]++;
                        hPulseHeightLong[iplane][3]->Fill(Cluster->Charge());
                        PLTU::AddToRunningAverage(AvgPH[iplane][3], NAvgPH[iplane][3], Cluster->Charge());
                        br_charge_3pixplus = Cluster->Charge();
                    }
                }

                /** fill hits per cluster histo */
                RootItems.nHitsPerCluster()[Cluster->ROC()]->Fill(Cluster->NHits());

                /** fill high occupancy high */
                if (Cluster->Charge() > 50000)
                    for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit)
//                        hOccupancyHighPH[Cluster->ROC()].Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );
                        RootItems.OccupancyHighPH()[Cluster->ROC()]->Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );

                /** fill low occupancy */
                else if (Cluster->Charge() > 10000 && Cluster->Charge() < 40000)
                    for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit)
//                        hOccupancyLowPH[Cluster->ROC()].Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );
                        RootItems.OccupancyLowPH()[Cluster->ROC()]->Fill( Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row() );
            }
            /** fill occupancy histo */
            for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
                PLTHit* Hit = Plane->Hit(ihit);

                if (Hit->ROC() < NROC) RootItems.Occupancy()[Hit->ROC()]->Fill(Hit->Column(), Hit->Row());
                else cerr << "Oops, ROC >= NROC?" << endl;
            }
        }

		if (telescopeID == 7 && FR->NTracks() == 1 && FR->Track(0)->NClusters() == NROC) {

            do_slope = true;
            for (uint8_t i_rocs(0); i_rocs != NROC; i_rocs++)
                if (FR->Track(0)->Cluster(i_rocs)->Charge() > ph_threshold){
                    do_slope = false;
                    break;
                }

            if (do_slope) {

				PLTTrack * Track = FR->Track(0);

                /** fill chi2 histos */
				hChi2X.Fill(Track->Chi2X() );
				hChi2Y.Fill(Track->Chi2Y() );
				hChi2.Fill(Track->Chi2() );

                /** fill slope histos */
                RootItems.TrackSlopeX()->Fill(Track->fSlopeX);
                RootItems.TrackSlopeY()->Fill(Track->fSlopeY);

				/** loop over the clusters */
				for (size_t icluster = 0; icluster < Track->NClusters(); icluster++){

					/** get the ROC in of the cluster and fill the corresponding residual */
					uint8_t ROC = Track->Cluster(icluster)->ROC();
					PLTCluster * Cluster = Track->Cluster(icluster);

                    /** fill residuals */
					hResidual[ROC].Fill(Track->LResidualX(ROC), Track->LResidualY(ROC)); // dX vs dY
					hResidualXdY[ROC].Fill(Cluster->LX(), Track->LResidualY(ROC));// X vs dY
					hResidualYdX[ROC].Fill(Cluster->LY(), Track->LResidualX(ROC)); // Y vs dX

                    /** ignore events above a certain threshold */
					if (Cluster->Charge() > ph_threshold) continue;
					//printf("High Charge: %13.3E\n", Cluster->Charge());

					/** fill the offline pulse heights (Track6+|Slope| < 0.01 in x and y ) */
					if ((fabs(Track->fSlopeX) < 0.01) && (fabs(Track->fSlopeY) < 0.01)){
						hPulseHeightOffline[Cluster->ROC()][0]->Fill(Cluster->Charge());

						if (Cluster->NHits() == 1)
							hPulseHeightOffline[Cluster->ROC()][1]->Fill(Cluster->Charge());
						else if (Cluster->NHits() == 2)
							hPulseHeightOffline[Cluster->ROC()][2]->Fill(Cluster->Charge());
						else if (Cluster->NHits() >= 3)
							hPulseHeightOffline[Cluster->ROC()][3]->Fill(Cluster->Charge());
					}
				}
            }
		}

	} /** END OF EVENT LOOP */

    delete FR;

  // Catch up on PH by time graph
  for (int i = 0; i != NROC; ++i) {
    for (int j = 0; j != 4; ++j) {
      gAvgPH[i][j].Set(NGraphPoints+1);
      gAvgPH[i][j].SetPoint(NGraphPoints, NGraphPoints*TimeWidth + TimeWidth/2, AvgPH[i][j]);
      gAvgPH[i][j].SetPointError(NGraphPoints, TimeWidth/2, AvgPH[i][j]/sqrt((float) NAvgPH[i][j]));
      printf("AvgCharge: %i %i N:%9i : %13.3E\n", i, j, NAvgPH[i][j], AvgPH[i][j]);
      NAvgPH[i][j] = 0;
      AvgPH[i][j] = 0;
    }
  }
  ++NGraphPoints;


  // Store root file with added tracking info
  if (GetUseRootInput(telescopeID)){
    newfile->cd();
    newtree->Write();
  }

  TCanvas Can;
  Can.cd();

  out_f->cd();

  for (int iroc = 0; iroc != NROC; ++iroc) {

    /** Draw Occupancy histograms */
    RootItems.Occupancy()[iroc]->SetMinimum(0);
    //RootItems.Occupancy()[iroc].SetAxisRange(12,38,"X");
    //RootItems.Occupancy()[iroc].SetAxisRange(39,80,"Y");
    RootItems.Occupancy()[iroc]->Draw("colz");
    Can.SaveAs( OutDir+TString(RootItems.Occupancy()[iroc]->GetName()) + ".gif");
    RootItems.Occupancy()[iroc]->Write();

    TH1F* hOccupancy1DZ = PLTU::HistFrom2D(RootItems.Occupancy()[iroc]);
    Can.cd();
    hOccupancy1DZ->Draw("hist");
    if (hOccupancy1DZ->GetEntries() > 0) {
      Can.SetLogy(1);
    }
    Can.SaveAs(OutDir+TString(hOccupancy1DZ->GetName()) + ".gif");
    hOccupancy1DZ->Write();
    Can.SetLogy(0);

    // Grab the quantile you're interested in here
    Double_t QProbability[1] = { 0.95 }; // Quantile positions in [0, 1]
    Double_t QValue[1];                  // Quantile values
    hOccupancy1DZ->GetQuantiles(1, QValue, QProbability);
    if(QValue[0] > 1 && RootItems.Occupancy()[iroc]->GetMaximum() > QValue[0]) {
      RootItems.Occupancy()[iroc]->SetMaximum(QValue[0]);
    }
    Can.cd();
    RootItems.Occupancy()[iroc]->Draw("colz");
    Can.SaveAs( OutDir+Form("Occupancy_ROC%i_Quantile.gif", iroc) );
    delete hOccupancy1DZ;

    Can.cd();
    hOccupancy1DZ = PLTU::HistFrom2D(RootItems.Occupancy()[iroc], 0, QValue[0], TString::Format("Occupancy1DZ_ROC%i_Quantile", iroc), 20);
    hOccupancy1DZ->Draw("hist");
    Can.SaveAs(OutDir+TString(hOccupancy1DZ->GetName()) + ".gif");
    delete hOccupancy1DZ;


    // Get 3x3 efficiency hists and draw
    TH2F* h3x3 = PLTU::Get3x3EfficiencyHist(*RootItems.Occupancy()[iroc], 0, 51, 0, 79);
    h3x3->SetTitle( TString::Format("Occupancy Efficiency 3x3 ROC%i", iroc) );
    Can.cd();
    h3x3->SetMinimum(0);
    h3x3->SetMaximum(3);
    h3x3->Draw("colz");
    Can.SaveAs(OutDir+TString(h3x3->GetName()) + ".gif");

    Can.cd();
    TH1F* h3x3_1DZ = PLTU::HistFrom2D(h3x3, "", 50);
    h3x3_1DZ->Draw("hist");
    Can.SaveAs(OutDir+TString(h3x3_1DZ->GetName()) + ".gif");
    delete h3x3;

    /** clusters per event */
    RootItems.DrawSaveTH1F(RootItems.nClusters(), iroc, Can, "Number of clusters per event", "Events");

    /** hits per cluster */
    RootItems.DrawSaveTH1F(RootItems.nHitsPerCluster(), iroc, Can, "Number of hits per cluster", "Number of Clusters");

    Can.cd();
    RootItems.OccupancyHighPH()[iroc]->SetMinimum(0);
    RootItems.OccupancyHighPH()[iroc]->Draw("colz");
    Can.SaveAs( OutDir+TString(RootItems.OccupancyHighPH()[iroc]->GetName()) + ".gif");

    RootItems.OccupancyLowPH()[iroc]->SetMinimum(0);
    RootItems.OccupancyLowPH()[iroc]->Draw("colz");
    Can.SaveAs( OutDir+TString(RootItems.OccupancyLowPH()[iroc]->GetName()) + ".gif");

//    float_t oneovertwo[iroc],oneoverthree[iroc],twooverthree[iroc];  //unused?
//
//    oneovertwo[iroc] = onepc[iroc]/twopc[iroc];
//    oneoverthree[iroc] = onepc[iroc]/threepc[iroc];
//    twooverthree[iroc] = twopc[iroc]/threepc[iroc]; //unused?

    /** ============================
     Draw the Pulse Heights
     ================================= */
    gStyle->SetOptStat(0);

    /** legends */
    TLegend leg_histo(0.77, 0.7, 0.9, 0.88, "");
    leg_histo.SetFillColor(4000);
    leg_histo.SetFillStyle(4000);
    leg_histo.SetBorderSize(0);
    leg_histo.SetTextAlign(11);
    leg_histo.AddEntry(hPulseHeight[iroc][0], "All", "l");
    leg_histo.AddEntry(hPulseHeight[iroc][1], "1 Pix", "l");
    leg_histo.AddEntry(hPulseHeight[iroc][2], "2 Pix", "l");
    leg_histo.AddEntry(hPulseHeight[iroc][3], "3+ Pix", "l");
    TLegend leg_mean(0.77, 0.45, 0.9, 0.66, "Mean values:");
    leg_mean.SetTextAlign(11);
    leg_mean.SetFillStyle(0);
    leg_mean.SetBorderSize(0);

    /** standard */
    Can.cd();
    hPulseHeight[iroc][0]->SetTitle( TString::Format("Pulse Height ROC%i", iroc) );
    hPulseHeight[iroc][0]->Draw("hist");
    hPulseHeight[iroc][1]->Draw("samehist");
    hPulseHeight[iroc][2]->Draw("samehist");
    hPulseHeight[iroc][3]->Draw("samehist");
    leg_mean.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeight[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    leg_mean.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeight[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    leg_mean.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeight[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    leg_mean.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeight[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
//    TLegend lRatio(0.75, 0.1, 0.90, 0.4, "Ratio:");
//    lRatio.SetTextAlign(11);
//    lRatio.SetFillStyle(0);
//    lRatio.SetBorderSize(0);
//	  leg_mean.AddEntry( "oneovertwo", TString::Format("%8.0f", oneovertwo[iroc])+" 1pix/2pix");
//    lRatio.AddEntry( "oneoverthree", TString::Format("%8.0f", oneoverthree, "")+" 1 over 3");
//    lRatio.AddEntry( "twooverthree", TString::Format("%8.0f", twooverthree, "")+" 2 over 3");
//    lRatio.Draw("same");
    leg_mean.Draw("same");
    leg_histo.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeight_ROC%i.gif", iroc));

    for (uint8_t i(0); i != 4; i++)
        hPulseHeight[iroc][i]->Write();

    /** offline */
    Can.cd();
    hPulseHeightOffline[iroc][0]->SetTitle( TString::Format("Pulse Height Offline ROC%i", iroc) );
    hPulseHeightOffline[iroc][0]->Draw("hist");
    hPulseHeightOffline[iroc][1]->Draw("samehist");
    hPulseHeightOffline[iroc][2]->Draw("samehist");
    hPulseHeightOffline[iroc][3]->Draw("samehist");
    leg_mean.Clear();
    leg_mean.SetHeader("Mean:");
    leg_mean.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    leg_mean.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    leg_mean.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    leg_mean.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeightOffline[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    leg_mean.Draw("same");
    leg_histo.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightOffline_ROC%i.gif", iroc));

    for (uint8_t i(0); i != 4; i++)
        hPulseHeightOffline[iroc][i]->Write();

    /** long */
    Can.cd();
    hPulseHeightLong[iroc][0]->SetTitle( TString::Format("Pulse Height Long ROC%i", iroc) );
    hPulseHeightLong[iroc][0]->Draw("hist");
    hPulseHeightLong[iroc][1]->Draw("samehist");
    hPulseHeightLong[iroc][2]->Draw("samehist");
    hPulseHeightLong[iroc][3]->Draw("samehist");
    leg_mean.Clear();
    leg_mean.SetHeader("Mean:");
    leg_mean.AddEntry( "PH0PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][0]->GetMean()), "")->SetTextColor(HistColors[0]);
    leg_mean.AddEntry( "PH1PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][1]->GetMean()), "")->SetTextColor(HistColors[1]);
    leg_mean.AddEntry( "PH2PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][2]->GetMean()), "")->SetTextColor(HistColors[2]);
    leg_mean.AddEntry( "PH3PMean", TString::Format("%8.0f", hPulseHeightLong[iroc][3]->GetMean()), "")->SetTextColor(HistColors[3]);
    leg_mean.Draw("same");
    leg_histo.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightLong_ROC%i.gif", iroc));

    for (uint8_t i(0); i != 4; i++)
        hPulseHeightLong[iroc][i]->Write();

    /** average pulse height */
    Can.cd();
    gAvgPH[iroc][0].SetTitle( TString::Format("Average Pulse Height ROC%i", iroc) );
    gAvgPH[iroc][0].Draw("Ape");
    gAvgPH[iroc][1].Draw("samepe");
    gAvgPH[iroc][2].Draw("samepe");
    gAvgPH[iroc][3].Draw("samepe");
    leg_histo.Draw("same");
    Can.SaveAs(OutDir+TString::Format("PulseHeightTime_ROC%i.gif", iroc));

    // Use AvgPH2D to draw PH 2D maps
    TString Name = TString::Format("PulseHeightAvg2D_ROC%i", iroc);
    TH2F hPulseHeightAvg2D(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
    for (int icol = 0; icol != PLTU::NCOL; ++icol) {
      for (int irow = 0; irow != PLTU::NROW; ++irow) {
        hPulseHeightAvg2D.SetBinContent(icol+1, irow+1, AvgPH2D[iroc][icol][irow]);
      }
    }
    Can.cd();
    hPulseHeightAvg2D.SetMinimum(0);
    hPulseHeightAvg2D.SetMaximum(100000);
    hPulseHeightAvg2D.Draw("colz");
    Can.SaveAs(OutDir+hPulseHeightAvg2D.GetName() + ".gif");

    // 2D Residuals
    Can.cd();
    hResidual[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + ".gif");

    // 2D Residuals X/dY
    gStyle->SetOptStat(1111);
    hResidualXdY[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualXdY[iroc].GetName()) + ".gif");

    // 2D Residuals Y/dX
    gStyle->SetOptStat(1111);
    hResidualYdX[iroc].Draw("colz");
    Can.SaveAs( OutDir+TString(hResidualYdX[iroc].GetName()) + ".gif");

    // Residual X-Projection
    Can.cd();
    hResidual[iroc].ProjectionX()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_X.gif");

    // Residual Y-Projection
    Can.cd();
    hResidual[iroc].ProjectionY()->Draw();
    Can.SaveAs( OutDir+TString(hResidual[iroc].GetName()) + "_Y.gif");

    gStyle->SetOptStat(0);


  } // end of loop over ROCs

  TCanvas Can2("CoincidenceMap", "CoincidenceMap", 1200, 400);
  Can2.cd();
  Can2.SetLogy(1);
  hCoincidenceMap.Draw("");
//    y = 0.1 * (gPad->GetUymin() - 0.5*hCoincidenceMap.GetYaxis()->GetBinWidth(1);
//    y = 0.2 * (gPad->GetUymin() - gPad->GetUymax());
    y = 0;
    for (int i=0;i<n_bins;i++) {
        x = hCoincidenceMap.GetXaxis()->GetBinCenter(i+1);
        t.DrawText(x,y,bin[i].c_str());}
    Can2.SaveAs(OutDir+"Occupancy_Coincidence.gif");
    Can2.SetLogy(0);

    /** draw tracking slopes and chi2 */
    gStyle->SetOptFit();
    Can.cd();
    RootItems.FitSlope(RootItems.TrackSlopeX() );
    RootItems.TrackSlopeX()->Draw();
    RootItems.LegendSlope(RootItems.TrackSlopeX() );
    Can.SaveAs(OutDir + "TrackSlopeX.gif");
    RootItems.TrackSlopeX()->Write();

    Can.cd();
    RootItems.FitSlope(RootItems.TrackSlopeY() );
    RootItems.TrackSlopeY()->Draw();
    RootItems.LegendSlope(RootItems.TrackSlopeY() );
    Can.SaveAs(OutDir+"TrackSlopeY.gif");
    RootItems.TrackSlopeY()->Write();

    Can.cd();
    gStyle->SetOptStat(000001111);
    hChi2.Draw("hist");
    Can.SaveAs(OutDir+"Chi2.gif");

    gStyle->SetOptStat(0);
//    hChi2X.Scale( 1/hChi2X.Integral());
    hChi2X.Draw("hist");
    Can.SaveAs(OutDir+"Chi2X.gif");

    gStyle->SetOptStat(0);
    hChi2Y.Draw("hist");
    Can.SaveAs(OutDir+"Chi2Y.gif");
    gStyle->SetOptStat(0);

    /** make index.html as overview */
	WriteHTML(PlotsDir + RunNumber, GetCalibrationFilename(telescopeID), telescopeID);

    /** print total events */
    cout << "=======================\n";
    cout << "Total events: " << indexi + 1 << endl;
    cout << "Elapsed time: " << float((clock() - start)) / CLOCKS_PER_SEC << endl;
    cout <<"=======================\n";

	return 0;
}
