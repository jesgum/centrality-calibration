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


void uploadToCCDB()
{
  // Settings
  const char* ccdbPath = "Users/j/jekarlss/Test/Centrality";
  // const char* ccdbPath = "Centrality/Estimators";

  const char* author = "Jesper Karlsson Gumprecht";
  const char* description = "LHC26ak pass1-based calib with extra estimators.";
  // Data
  const bool uploadFT0C = true;
  const bool uploadFT0Cvar1 = true;
  const bool uploadFT0Cvar2 = true;
  const bool uploadFT0M = true;
  const bool uploadFV0A = true;
  const bool uploadNGlo = true;
  const bool uploadNMFT = true;

  // Monte Carlo
  const bool uploadMonteCarlo = false;

  // Upload switches
  const bool doUploadToCCDB = false;
  const bool doSaveForInspection = true;

  // Read run numbers from file
  std::vector<int> runs;
  std::ifstream file("../runNumbers.txt");
  std::string content;
  std::getline(file, content, '\0'); // read entire file

  std::stringstream ss(content);
  std::string token;
  while (std::getline(ss, token, ',')) {
    // strip whitespace
    token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
    if (!token.empty()) {
      runs.push_back(std::stoi(token));
    }
  }

  std::cout << "Processing " << runs.size() << " runs..." << std::endl;

  o2::ccdb::CcdbApi ccdb_api;
  ccdb_api.init("https://alice-ccdb.cern.ch");
  std::map<string, string> metadataRCT, headers;
  for (size_t ii = 0; ii < runs.size(); ii++) {
    //  Get the desired timestamps from the CCDB, no need to suffer with independent code
    headers = ccdb_api.retrieveHeaders(Form("RCT/Info/RunInformation/%i", runs[ii]), metadataRCT, -1);
    int64_t tsSOR = atol(headers["SOR"].c_str());
    int64_t tsEOR = atol(headers["EOR"].c_str());

    // safety margins
    tsSOR = tsSOR - 300;
    tsEOR = tsEOR + 300;

    std::cout << "Run " << runs[ii] << " SOR " << tsSOR << " EOR " << tsEOR << std::endl;

    // Interface with local objects to be uploaded
    std::cout << "Now performing dedicated object upload... please wait..." << std::endl;

    // Constructing entirety of list to be uploaded for this run
    TList* listHistograms = new TList();

    //__________________________________________________________________
    // Upload calibration, please

    // Step 1: upload existing FT0C calibration for real data
    std::cout << "Adding FT0C for run " << runs[ii] << std::endl;
    TFile* file1 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hFT0C_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFT0C = (TH1F*)file1->Get("hCalib");
    hCalibFT0C->SetName("hCalibFT0C_input");
    listHistograms->Add(hCalibFT0C->Clone("hCalibZeqFT0C"));

    // Step 2: variant FT0C with no scale-to-fit near AP (matches other estimators for now)
    std::cout << "Adding FT0Cvariant1 for run " << runs[ii] << std::endl;
    TFile* file2 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hFT0C_BCs_var1.root", runs[ii]), "READ");
    TH1F* hCalibFT0Cvar1 = (TH1F*)file2->Get("hCalib");
    hCalibFT0Cvar1->SetName("hCalibFT0Cvar1_input");
    listHistograms->Add(hCalibFT0Cvar1->Clone("hCalibZeqFT0Cvar1"));

    // Step 2bis: variant FT0C with no scale-to-fit near AP, ancestor mode zero
    std::cout << "Adding FT0Cvariant2 for run " << runs[ii] << std::endl;
    // AR_545367_calibration_ancestorMode2_hFT0C_BCs.root
    TFile* file2bis = new TFile(Form("../results/AR_%i_calibration_ancestorMode0_fixedK_fixedMu_hFT0C_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFT0Cvar2 = (TH1F*)file2bis->Get("hCalib");
    hCalibFT0Cvar2->SetName("hCalibFT0Cvar2_input");
    listHistograms->Add(hCalibFT0Cvar2->Clone("hCalibZeqFT0Cvar2"));

    // Step 3: FT0M
    std::cout << "Adding FT0 for run " << runs[ii] << std::endl;
    TFile* file3 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hFT0M_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFT0 = (TH1F*)file3->Get("hCalib");
    hCalibFT0->SetName("hCalibFT0_input");
    listHistograms->Add(hCalibFT0->Clone("hCalibZeqFT0"));

    // Step 4: FV0A
    std::cout << "Adding FV0A for run " << runs[ii] << std::endl;
    TFile* file4 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hFV0A_BCs.root", runs[ii]), "READ");
    TH1F* hCalibFV0A = (TH1F*)file4->Get("hCalib");
    hCalibFV0A->SetName("hCalibFV0A_input");
    listHistograms->Add(hCalibFV0A->Clone("hCalibZeqFV0"));

    // Step 5: NGlobal
    std::cout << "Adding NGlobal for run " << runs[ii] << std::endl;
    TFile* file5 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hNGlobalTracks.root", runs[ii]), "READ");
    TH1F* hCalibNGlobal = (TH1F*)file5->Get("hCalib");
    hCalibNGlobal->SetName("hCalibNGlobal_input");
    listHistograms->Add(hCalibNGlobal->Clone("hCalibZeqNGlobal"));

    // Step 6: MFT
    std::cout << "Adding MFT for run " << runs[ii] << std::endl;
    TFile* file6 = new TFile(Form("../results/AR_%i_calibration_ancestorMode2_fixedK_fixedMu_hNMFTTracks.root", runs[ii]), "READ");
    TH1F* hCalibMFT = (TH1F*)file6->Get("hCalib");
    if (hCalibMFT) {
      hCalibMFT->SetName("hCalibMFT_input");
      listHistograms->Add(hCalibMFT->Clone("hCalibZeqMFT"));
    } else {
      std::cout << "MFT does not exist for run " << runs[ii] << ", skipping..." << std::endl;
    }

    // Step 1b: add mc calibration as well
    std::cout << "Adding Monte Carlo calibration for run " << runs[ii] << std::endl;

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
        std::cout << "FT0C scaling:  " << std::endl;
        f1scale->Print();
        for (int i = 0; i < 5; i++) {
          std::cout << "par " << i << " = " << f1scale->GetParameter(i) << std::endl;
        }
      } else {
        std::cout << "No specific FT0C calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault1 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        if (!fileScaleDefault1)
          std::cout << "no fileScaleDefault1" << std::endl;
        f1scale = (TFormula*)fileScaleDefault1->Get("f1scaleDefault");
        if (!f1scale)
          std::cout << "no f1scaleDefault" << std::endl;
        f1scale->SetName("PYTHIA-FT0C_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0C"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0Cvar1");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0Cvar1_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar1"));
      } else {
        std::cout << "No specific FT0Cvar1 calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault2 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault2->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FT0Cvar1_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar1"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0C");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0Cvar2_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar2"));
        std::cout << "FT0Cvar2 scaling:  " << std::endl;
        f1scale->Print();
        for (int i = 0; i < 5; i++) {
          std::cout << "par " << i << " = " << f1scale->GetParameter(i) << std::endl;
        }
      } else {
        std::cout << "No specific FT0Cvar2 calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault2 = new TFile("../montecarlo/FT0C_mcCalibDefault.root", "READ");
        if (!fileScaleDefault2)
          std::cout << "no fileScaleDefault2" << std::endl;
        f1scale = (TFormula*)fileScaleDefault2->Get("f1scaleDefault");
        if (!f1scale)
          std::cout << "no f1scaleDefault" << std::endl;
        f1scale->SetName("PYTHIA-FT0Cvar2_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0Cvar2"));
      }

      f1scale = (TFormula*)file1->Get("PYTHIA-FT0");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FT0_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0"));
      } else {
        std::cout << "No specific calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault3 = new TFile("../montecarlo/FT0M_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault3->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FT0_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FT0"));
      }
      std::cout << "Did FT0" << std::endl;

      f1scale = (TFormula*)file1->Get("PYTHIA-FV0");
      if (f1scale) {
        f1scale->SetName("PYTHIA-FV0A_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FV0"));
      } else {
        std::cout << "No specific calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault4 = new TFile("../montecarlo/FV0A_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault4->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-FV0A_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-FV0"));
      }
      std::cout << "Did FV0" << std::endl;

      f1scale = (TFormula*)file1->Get("PYTHIA-MFT");
      if (f1scale) {
        f1scale->SetName("PYTHIA-MFT_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-MFT"));
      } else {
        std::cout << "No specific calib found, adding default calibration for run " << runs[ii] << std::endl;
        fileScaleDefault5 = new TFile("../montecarlo/NMFTTracks_mcCalibDefault.root", "READ");
        f1scale = (TFormula*)fileScaleDefault5->Get("f1scaleDefault");
        f1scale->SetName("PYTHIA-MFT_input");
        listHistograms->Add(f1scale->Clone("PYTHIA-MFT"));
      }
      std::cout << "Did MFT" << std::endl;
    }

    listHistograms->ls();

    // Create stuff to send to CCDB
    std::cout << "Defining metadata for this run..." << std::endl;
    map<string, string> metadata; // can be empty
    metadata.insert(std::pair{ "Description", Form("%s Run: %i", description, runs[ii]) });
    metadata.insert(std::pair{ "Author", author });

    // Send off to CCDB
    if (doUploadToCCDB) {
      std::cout << "Attempting CCDB upload..." << std::endl;
      try {
        ccdb_api.storeAsTFileAny(listHistograms, Form("%s", ccdbPath), metadata, tsSOR, tsEOR);
      } catch (std::exception const& e) {
        LOG(fatal) << "Failed at CCDB submission!";
      }
      std::cout << "Finished with upload of run " << runs[ii] << " update! " << std::endl;
    }


    // save for posterior inspection if required
    if (doSaveForInspection) {
    std::cout << "Will now save for posterity" << std::endl;
      TFile* fileListOutput = new TFile(Form("CCDB-content-%i.root", runs[ii]), "RECREATE");
      listHistograms->Write();
      fileListOutput->Write();
    }
  }

  std::cout << std::endl;
  std::cout << "Settings:" << std::endl;
  std::cout << "  - Upload path: " << ccdbPath << std::endl;
  std::cout << "  - Author.....: " << author << std::endl;
  std::cout << "  - Description: " << description << std::endl;
  std::cout << std::endl;
  std::cout << "Estimators:" << std::endl;
  std::cout << "  - FT0C....: " << uploadFT0C << std::endl;
  std::cout << "  - FT0Cvar1: " << uploadFT0Cvar1 << std::endl;
  std::cout << "  - FT0Cvar2: " << uploadFT0Cvar2 << std::endl;
  std::cout << "  - FT0M....: " << uploadFT0M << std::endl;
  std::cout << "  - FV0A....: " << uploadFV0A << std::endl;
  std::cout << "  - NGlo....: " << uploadNGlo << std::endl;
  std::cout << "  - NMFT....: " << uploadNMFT << std::endl;
  std::cout << std::endl;
  std::cout << "Done!" << std::endl;
}
