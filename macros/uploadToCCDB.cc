#include <gsl/span>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <TSystem.h>
#include <TFile.h>
#include <TH1F.h>
#include <TFormula.h>
#include "Framework/Logger.h"
#include "CCDB/BasicCCDBManager.h"

Double_t GetBoundaryForPercentile(TH1* histo, Double_t lPercentileRequested)
{
  // This function returns the boundary for a specific percentile.
  Double_t lReturnValue = 0.0;
  Double_t lPercentile = 100.0 - lPercentileRequested;

  const Long_t lNBins = histo->GetNbinsX();
  Double_t lCountDesired = lPercentile * histo->GetEntries() / 100;
  Long_t lCount = 0;
  for (Long_t ibin = 1; ibin < lNBins; ibin++) {
    lCount += histo->GetBinContent(ibin);
    if (lCount >= lCountDesired) {
      // Found bin I am looking for!
      Double_t lWidth = histo->GetBinWidth(ibin);
      Double_t lLeftPercentile = 100. * (lCount - histo->GetBinContent(ibin)) / histo->GetEntries();
      Double_t lRightPercentile = 100. * lCount / histo->GetEntries();

      Double_t lProportion = (lPercentile - lLeftPercentile) / (lRightPercentile - lLeftPercentile);

      lReturnValue = histo->GetBinLowEdge(ibin) + lProportion * lWidth;
      break;
    }
  }
  return lReturnValue;
}

void uploadToCCDB()
{
  // Read run numbers from file
  std::vector<int> runs;
  std::ifstream file("runNumbers.txt");
  std::string content;
  std::getline(file, content, '\0'); // read entire file

  std::stringstream ss(content);
  std::string token;
  while (std::getline(ss, token, ',')) {
    // strip whitespace
    token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
    if (!token.empty()) runs.push_back(std::stoi(token));
  }

  bool uploadMonteCarlo = false;
  const char* ccdbPath = "Test/Centrality";
  int nRuns = sizeof(runs) / sizeof(int);
  cout << "Processing " << nRuns << " runs..." << endl;

  o2::ccdb::CcdbApi ccdb_api;
  ccdb_api.init("https://alice-ccdb.cern.ch");
  std::map<string, string> metadataRCT, headers;
  for (int ii = 0; ii < nRuns; ii++) {
    // for(int ii=0; ii<5; ii++){
    //  Get the desired timestamps from the CCDB, no need to suffer with independent code
    headers = ccdb_api.retrieveHeaders(Form("RCT/Info/RunInformation/%i", runs[ii]), metadataRCT, -1);
    int64_t tsSOR = atol(headers["SOR"].c_str());
    int64_t tsEOR = atol(headers["EOR"].c_str());

    // safety margins
    tsSOR = tsSOR - 300;
    tsEOR = tsEOR + 300;

    cout << "Run " << runs[ii] << " SOR " << tsSOR << " EOR " << tsEOR << endl;

    // Interface with local objects to be uploaded
    cout << "Now performing dedicated object upload... please wait..." << endl;

    // Constructing entirety of list to be uploaded for this run
    TList* listHistograms = new TList();

    //__________________________________________________________________
    // Upload calibration, please

    // Step 1: upload existing FT0C calibration for real data
    cout << "Adding FT0C for run " << runs[ii] << endl;
    TFile* file1 = new TFile(Form("AR_%i_calibration_hFT0C_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFT0C = (TH1F*)file1->Get("hCalib");
    hCalibFT0C->SetName("hCalibFT0C_input");
    listHistograms->Add(hCalibFT0C->Clone("hCalibZeqFT0C"));

    // Step 2: variant FT0C with no scale-to-fit near AP (matches other estimators for now)
    cout << "Adding FT0Cvariant1 for run " << runs[ii] << endl;
    TFile* file2 = new TFile(Form("AR_%i_calibration_hFT0C_BCs_variants1.root", runs[ii]), "READ");
    TH1F* hCalibFT0Cvar1 = (TH1F*)file2->Get("hCalib");
    hCalibFT0Cvar1->SetName("hCalibFT0Cvar1_input");
    listHistograms->Add(hCalibFT0Cvar1->Clone("hCalibZeqFT0Cvar1"));

    // Step 2bis: variant FT0C with no scale-to-fit near AP, ancestor mode zero
    cout << "Adding FT0Cvariant2 for run " << runs[ii] << endl;
    // AR_545367_calibration_ancestorMode2_hFT0C_BCs.root
    TFile* file2bis = new TFile(Form("AR_%i_calibration_hFT0C_BCs_variants2.root", runs[ii]), "READ");
    TH1F* hCalibFT0Cvar2 = (TH1F*)file2bis->Get("hCalib");
    hCalibFT0Cvar2->SetName("hCalibFT0Cvar2_input");
    listHistograms->Add(hCalibFT0Cvar2->Clone("hCalibZeqFT0Cvar2"));

    // Step 3: FT0M
    cout << "Adding FT0 for run " << runs[ii] << endl;
    TFile* file3 = new TFile(Form("AR_%i_calibration_hFT0M_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFT0 = (TH1F*)file3->Get("hCalib");
    hCalibFT0->SetName("hCalibFT0_input");
    listHistograms->Add(hCalibFT0->Clone("hCalibZeqFT0"));

    // Step 4: FV0A
    cout << "Adding FV0A for run " << runs[ii] << endl;
    TFile* file4 = new TFile(Form("AR_%i_calibration_hFV0A_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFV0A = (TH1F*)file4->Get("hCalib");
    hCalibFV0A->SetName("hCalibFV0A_input");
    listHistograms->Add(hCalibFV0A->Clone("hCalibZeqFV0"));

    // Step 5: NGlobal
    cout << "Adding NGlobal for run " << runs[ii] << endl;
    TFile* file5 = new TFile(Form("AR_%i_calibration_hNGlobalTracks.root", runs[ii]), "READ");
    TH1F* hCalibNGlobal = (TH1F*)file5->Get("hCalib");
    hCalibNGlobal->SetName("hCalibNGlobal_input");
    listHistograms->Add(hCalibNGlobal->Clone("hCalibZeqNGlobal"));

    // Step 6: MFT
    cout << "Adding MFT for run " << runs[ii] << endl;
    TFile* file6 = new TFile(Form("AR_%i_calibration_hNMFTTracks.root", runs[ii]), "READ");
    TH1F* hCalibMFT = (TH1F*)file6->Get("hCalib");
    if (hCalibMFT) {
      hCalibMFT->SetName("hCalibMFT_input");
      listHistograms->Add(hCalibMFT->Clone("hCalibZeqMFT"));
    } else {
      cout << "MFT does not exist for run " << runs[ii] << ", skipping..." << endl;
    }

    // Step 1b: add mc calibration as well
    cout << "Adding Monte Carlo calibration for run " << runs[ii] << endl;

    TFile* fileScale1 = 0x0;
    TFile* fileScale2 = 0x0;
    TFile* fileScale3 = 0x0;
    TFile* fileScale4 = 0x0;
    TFile* fileScale5 = 0x0;
    TFile* fileScaleDefault1 = 0x0;
    TFile* fileScaleDefault2 = 0x0;
    TFile* fileScaleDefault3 = 0x0;
    TFile* fileScaleDefault4 = 0x0;
    TFile* fileScaleDefault5 = 0x0;

    if (uploadMonteCarlo) {
      TFormula* f1scale = 0x0;
      f1scale = (TFormula*)file1->Get("PYTHIA-FT0C");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0C_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0C"));
        cout << "FT0C scaling:  " << endl;
        f1scale->Print();
        for (int i = 0; i < 5; i++) {
          cout << "par " << i << " = " << f1scale->GetParameter(i) << endl;
        }
      } else {
        cout << "No specific FT0C calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault1 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        if (!fileScaleDefault1)
          cout << "no fileScaleDefault1" << endl;
        f1scale = (TFormula*)fileScaleDefault1->Get("f1scaleDefault");
        if (!f1scale)
          cout << "no f1scaleDefault" << endl;
        f1scale->SetName("PYTHIA-FT0C_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0C"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0Cvar1");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0Cvar1_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar1"));
      } else {
        cout << "No specific FT0Cvar1 calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault2 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault2->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FT0Cvar1_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar1"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0C");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0Cvar2_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar2"));
        cout << "FT0Cvar2 scaling:  " << endl;
        f1scale->Print();
        for (int i = 0; i < 5; i++) {
          cout << "par " << i << " = " << f1scale->GetParameter(i) << endl;
        }
      } else {
        cout << "No specific FT0Cvar2 calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault2 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        if (!fileScaleDefault2)
          cout << "no fileScaleDefault2" << endl;
        f1scale = (TFormula*)fileScaleDefault2->Get("f1scaleDefault");
        if (!f1scale)
          cout << "no f1scaleDefault" << endl;
        f1scale->SetName("PYTHIA-FT0Cvar2_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar2"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0"));
      } else {
        cout << "No specific calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault3 = new TFile("../montecarlo/FT0M_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault3->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FT0_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0"));
      }
      cout << "Did FT0" << endl;

      f1scale = (TFormula*)file1->Get("PYTHIA-FV0");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FV0A_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FV0"));
      } else {
        cout << "No specific calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault4 = new TFile("../montecarlo/FV0A_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault4->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FV0A_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FV0"));
      }
      cout << "Did FV0" << endl;

      bool healthyMFT = true;

      //    // check if for this run the MFT calibration actually made any sense
      //    TFile *mftSanityFile = new TFile(Form("../ARs/AR_%i.root", runs[ii]), "READ");
      //    // get the range in which the supercalibration is desirable
      //    TH1F *hV0Mfine = 0x0;
      //    hV0Mfine = (TH1F *) mftSanityFile -> Get(Form("centrality-study/%s", "hNMFTTracks"));
      //
      //    Double_t lFitRangeMax = GetBoundaryForPercentile(hV0Mfine, 0.1);
      //    Double_t lFitRangeMin = 0.02*GetBoundaryForPercentile(hV0Mfine, 0.01);
      //    cout<<"Run #"<<runs[ii]<<" ("<<ii<<"/"<<nRuns<<") min "<<lFitRangeMin<<" max "<<lFitRangeMax<<endl;
      //    if(TMath::Abs(lFitRangeMax - lFitRangeMin) > 500){
      //      cout<<"Judged sane for the MFT!"<<endl;
      //    }else{
      //      cout<<"Unhealthy for the MFT!"<<endl;
      //      healthyMFT = false;
      //    }
      //    mftSanityFile->Close();

      f1scale = (TFormula*)file1->Get("PYTHIA-MFT");
      if (f1scale) {
        f1scale->SetName("PYTHIA-MFT_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-MFT"));
      } else {
        cout << "No specific calib found, adding default calibration for run " << runs[ii] << endl;
        fileScaleDefault5 = new TFile("../montecarlo/NMFTTracks_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault5->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-MFT_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-MFT"));
      }
      cout << "Did MFT" << endl;
    }

    listHistograms->ls();

    // Create stuff to send to CCDB
    cout << "Defining metadata for this run..." << endl;
    map<string, string> metadata; // can be empty
    metadata.insert(std::pair{ "Description", Form("pass3-based calib for run %s: update with extra estimators", Form("%i", runs[ii])) });
    metadata.insert(std::pair{ "Author", "Jesper Karlsson Gumprecht" });
    //
    //    TList *listHistograms = new TList();
    //    listHistograms->Add(hCalibZeqFT0C);
    //
    // Send off to CCDB
    cout << "Attempting CCDB upload..." << endl;
    try {
      ccdb_api.storeAsTFileAny(listHistograms, Form("Users/j/jekarlss/%s", ccdbPath), metadata, tsSOR, tsEOR);
      //      ccdb_api.storeAsTFileAny(listHistograms, "Centrality/Estimators", metadata, tsSOR, tsEOR);
    } catch (std::exception const& e) {
      LOG(fatal) << "Failed at CCDB submission!";
    }
    cout << "Finished with upload of run " << runs[ii] << " update! " << endl;

    cout << "Will now save for posterity" << endl;

    // save for posterior inspection if required
    TFile* fileListOutput = new TFile(Form("CCDB-content-%i.root", runs[ii]), "RECREATE");
    listHistograms->Write();
    fileListOutput->Write();
    //    fileListOutput->Close();

    file1->Close();
    //    file2->Close();
    //    file3->Close();
    //    file4->Close();
    //    file5->Close();
    if (file6)
      file6->Close();

    if (fileScale1)
      fileScale1->Close();
    if (fileScale2)
      fileScale2->Close();
    if (fileScale3)
      fileScale3->Close();
    if (fileScale4)
      fileScale4->Close();
    if (fileScale5)
      fileScale5->Close();

    if (fileScaleDefault1)
      fileScaleDefault1->Close();
    if (fileScaleDefault2)
      fileScaleDefault2->Close();
    if (fileScaleDefault3)
      fileScaleDefault3->Close();
    if (fileScaleDefault4)
      fileScaleDefault4->Close();
    if (fileScaleDefault5)
      fileScaleDefault5->Close();

    //    cout<<"Opening file: "<<Form("AR_%i_glauberNBD_bc.root",runs[ii])<<endl;
    //    TFile *file = new TFile(Form("../AR_%i_glauberNBD_bc.root",runs[ii]), "READ");
    //
    //    cout<<"Opening calibration histogram..."<<endl;
    //    TH1F *hCalibZeqFT0C = (TH1F*) file->Get("hCalibV0M_Unanchored");
    //    hCalibZeqFT0C->SetName("hCalibZeqFT0C");
    //
  }
  cout << "Done!" << endl;
}
