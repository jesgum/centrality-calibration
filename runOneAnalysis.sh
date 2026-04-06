#! /bin/bash
touch jobLock_Analysis_${1}_${2}_${3}

##int runGlauFit004(TString lInputFileName = "AnalysisResultsLHC24ar.root", Double_t lFitRange = /*350*/500., TString histogramName = "hFT0C_BCs", int ancestorMode = 2, Bool_t lFreek = kFALSE, Bool_t lFreef = kFALSE, Float_t lfvalue = 0.800)

root.exe -q -b runGlauFit004.C\(\"ARs/AR_${1}.root\",500,\"${2}\",${3}\)
rm jobLock_Analysis_${1}_${2}_${3}
