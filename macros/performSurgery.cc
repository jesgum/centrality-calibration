#include <cmath>
#include <gsl/span>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "CCDB/BasicCCDBManager.h"
#include "Framework/Logger.h"
#include <TFile.h>
#include <TH1F.h>
#include <TSystem.h>

static int uniqueCounter = 0;
TH1F *GetCalibrationHistogramFromFile(TFile *infile) {
  if (!infile || infile->IsZombie()) {
    cout << "Cannot open file" << endl;
    return nullptr;
  }

  TH1F *hCalib = (TH1F *)infile->Get("hCalib");
  if (!hCalib) {
    cout << "Histogram 'hCalib' not found in file: " << infile->GetName() << endl;
    infile->Close();
    delete infile;
    return nullptr;
  }

  TH1F *hCalibClone = (TH1F *)hCalib->Clone(Form("hCalibClone_%d", uniqueCounter++));
  hCalibClone->SetDirectory(0);
  delete hCalib;
  infile->Close();
  delete infile;
  return hCalibClone;
}

void performSurgery() {
  int runs[] = {564356, 564359, 564373, 564374, 564387, 564400, 564414, 564430, 564445};
  int nRuns = sizeof(runs) / sizeof(int);
  cout << "Processing " << nRuns << " runs..." << endl;

  for (int ii = 0; ii < nRuns; ++ii) {
    TH1F *hCalibBCs = GetCalibrationHistogramFromFile(new TFile(Form("AR_%d_calibration_hFT0M_BCs.root", runs[ii]), "read"));
    if (!hCalibBCs) {
      cout << "Skipping run " << runs[ii] << ": BCs calibration missing"<< endl;
      continue;
    }
    hCalibBCs->SetName("hCalibZeqFT0LightIonAncBc");

    TH1F *hCalibCol = GetCalibrationHistogramFromFile(new TFile(Form("AR_%d_calibration_hFT0M_Collisions.root", runs[ii]), "read"));
    if (!hCalibCol) {
      cout << "Skipping run " << runs[ii] << ": Collisions calibration missing"<< endl;
      delete hCalibBCs;
      continue;
    }
    hCalibCol->SetName("hCalibZeqFT0LightIonAncCol");

    // Opening up the patient
    TFile *ccdbfile = new TFile(Form("centrality-run-%d.root", runs[ii]), "update");
    if (!ccdbfile || ccdbfile->IsZombie()) {
      cout << "File not found: centrality-run-" << runs[ii] << ".root" << endl;
      delete hCalibBCs;
      delete hCalibCol;
      continue;
    }

    // Check if patient had surgery before
    TH1F *hTryCalibBCs = (TH1F *)ccdbfile->Get("hCalibZeqFT0LightIonAncBc");
    TH1F *hTryCalibCol = (TH1F *)ccdbfile->Get("hCalibZeqFT0LightIonAncCol");
    if (hTryCalibBCs || hTryCalibCol) {
      cout << "Run " << runs[ii] << " already underwent surgery, skipping"<< endl;
      delete hTryCalibBCs;
      delete hTryCalibCol;
      delete hCalibBCs;
      delete hCalibCol;
      ccdbfile->Close();
      delete ccdbfile;
      continue;
    }

    ccdbfile->cd();
    hCalibBCs->Write();
    hCalibCol->Write();
    ccdbfile->Close();

    delete hCalibBCs;
    delete hCalibCol;
    delete ccdbfile;
  }
}