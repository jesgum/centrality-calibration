#include <gsl/span>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

#include <TSystem.h>
#include <TFile.h>
#include <TH1F.h>
#include "Framework/Logger.h"
#include "CCDB/BasicCCDBManager.h"

void downloadCCDB(){

  int runs[] = {
    564356, 564359, 564373, 564374, 564387, 564400, 564414, 564430, 564445
  };

  int nRuns = sizeof(runs)/sizeof(int);
  cout<<"Processing "<<nRuns<<" runs..."<<endl;
  
  o2::ccdb::CcdbApi ccdb_api;
  ccdb_api.init("https://alice-ccdb.cern.ch");
  std::map<string, string> metadataRCT, headers;
  
  int nCentralities = 0;
  int nVertexZs = 0;
  
  std::vector<int64_t> middleTimestamps;
  cout<<"Extracting objects from the CCDB..."<<endl; 
  
  for(int ii=0; ii<nRuns; ii++){
    // Get the desired timestamps from the CCDB, no need to suffer with independent code
    headers = ccdb_api.retrieveHeaders(Form("RCT/Info/RunInformation/%i", runs[ii]), metadataRCT, -1);
    int64_t tsSOR = atol(headers["SOR"].c_str());
    int64_t tsEOR = atol(headers["EOR"].c_str());
    // produce mean timestamp for a given run now
    int64_t tsMiddle = static_cast<int64_t>(0.5f*static_cast<double>(tsSOR) + 0.5f*static_cast<double>(tsEOR));
    
    cout<<"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+"<<endl;
    cout<<"Run #"<<runs[ii]<<", "<<ii<<" out of "<<nRuns<<" (so far successful: "<<nCentralities<<" vertex-Z "<<nVertexZs<<") SOR "<<tsSOR<<" EOR "<<tsEOR<<" middle of run "<<tsMiddle<<endl;
    
    middleTimestamps.push_back(tsMiddle);
    
  }
  
  cout<<"Inspect middle timestamps again: "<<endl;
  
  for(int ii=0; ii<nRuns; ii++){
    cout<<"-~> Run #"<<runs[ii]<<", "<<ii<<" out of "<<nRuns<<", middle of run is timestamped as "<<middleTimestamps[ii]<<endl;
  }
    
  
  for(int ii=0; ii<nRuns; ii++){
    if(!gSystem->AccessPathName(Form("centrality-run-%i.root", runs[ii]))){
      cout<<"Run #"<<runs[ii]<<" has already been successfully extracted! Continuing"<<endl;
      nCentralities++;
      continue;
    }
    
    o2::ccdb::CcdbApi ccdb_api_centrality;
    ccdb_api_centrality.init("https://alice-ccdb.cern.ch");
    cout<<"-~> Run #"<<runs[ii]<<", "<<ii<<" out of "<<nRuns<<" (so far successful: "<<nCentralities<<" vertex-Z "<<nVertexZs<<") middle of run "<<middleTimestamps[ii]<<endl;
    
    cout<<"Centrality CCDB object for run #"<<runs[ii]<<" (timestamped "<<middleTimestamps[ii]<<" ): "<<endl;
    map<string, string> metadataCentralityCalib; // can be empty
    TList* list = ccdb_api_centrality.retrieveFromTFileAny<TList>("Centrality/Estimators", metadataCentralityCalib, middleTimestamps[ii]);
    if(!list){
      cout<<"No centrality calibration for run "<<runs[ii]<<" detected! Skipping!"<<endl;
      continue;
    }
    list->Print();
    TFile *extractedFile = new TFile(Form("centrality-run-%i.root", runs[ii]), "RECREATE");
    list->Write();
    extractedFile->Write();
    nCentralities++;
  }
  for(int ii=0; ii<nRuns; ii++){
    if(!gSystem->AccessPathName(Form("vertexz-run-%i.root", runs[ii]))){
      cout<<"Run #"<<runs[ii]<<" has already been successfully extracted! Continuing"<<endl;
      nVertexZs++;
      continue;
    }
    
    o2::ccdb::CcdbApi ccdb_api_vertexZ;
    ccdb_api_vertexZ.init("https://alice-ccdb.cern.ch");
    cout<<"-~> Run #"<<runs[ii]<<", "<<ii<<" out of "<<nRuns<<" (so far successful: "<<nCentralities<<" vertex-Z "<<nVertexZs<<") middle of run "<<middleTimestamps[ii]<<endl;
    map<string, string> metadataVertexZCalib; // can be empty
    TList* listVertexZ = ccdb_api_vertexZ.retrieveFromTFileAny<TList>("Centrality/Calibration", metadataVertexZCalib, middleTimestamps[ii]);
    if(!listVertexZ){
      cout<<"No vertex Z calibration for run "<<runs[ii]<<" detected! Skipping!"<<endl;
      continue;
    }
    listVertexZ->Print();
    
    TFile *vertexZFile = new TFile(Form("vertexz-run-%i.root", runs[ii]), "RECREATE");
    listVertexZ->Write();
    vertexZFile->Write();
    nVertexZs++;
  }
    
  cout<<"Runs: "<<nRuns<<" total centralities "<<nCentralities<<" total vertex-Z calibs "<<nVertexZs<<endl;
    
    
    
    
//    // Interface with local objects to be uploaded
//    cout<<"Now performing dedicated object upload... please wait..."<<endl;
//    cout<<"Opening file: "<<Form("AR_%i_glauberNBD_bc.root",runs[ii])<<endl;
//    TFile *file = new TFile(Form("../AR_%i_glauberNBD_bc.root",runs[ii]), "READ");
//    
//    cout<<"Opening calibration histogram..."<<endl;
//    TH1F *hCalibZeqFT0C = (TH1F*) file->Get("hCalibV0M_Unanchored");
//    hCalibZeqFT0C->SetName("hCalibZeqFT0C");
//    
//    // Create stuff to send to CCDB
//    cout<<"Defining metadata for this run..."<<endl;
//    map<string, string> metadata; // can be empty
//    metadata.insert(std::pair{"Description", Form("pass4-based calib for run %s", Form("%i", runs[ii]))});
//    metadata.insert(std::pair{"Author", "David Dobrigkeit Chinellato"});
//    
//    TList *listHistograms = new TList();
//    listHistograms->Add(hCalibZeqFT0C);
//    
//    // Send off to CCDB
//    cout<<"Attempting CCDB upload..."<<endl;
//    try {
//      ccdb_api.storeAsTFileAny(listHistograms, "Centrality/Estimators", metadata, tsSOR, tsEOR);
//    } catch (std::exception const& e) {
//      LOG(fatal) << "Failed at CCDB submission!";
//    }
//    cout<<"Finished with upload of run "<<runs[ii]<<" update! "<<endl;
//  }
//  cout<<"Done!"<<endl;
}
