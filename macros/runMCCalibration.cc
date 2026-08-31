Double_t GetBoundaryForPercentile( TH1 *histo, Double_t lPercentileRequested ) {
  //This function returns the boundary for a specific percentile.
  Double_t lReturnValue = 0.0;
  Double_t lPercentile = 100.0 - lPercentileRequested;
  
  const Long_t lNBins = histo->GetNbinsX();
  Double_t lCountDesired = lPercentile * histo->GetEntries()/100;
  Long_t lCount = 0;
  for(Long_t ibin=1;ibin<lNBins;ibin++){
    lCount += histo->GetBinContent(ibin);
    if(lCount >= lCountDesired){
      //Found bin I am looking for!
      Double_t lWidth = histo->GetBinWidth(ibin);
      Double_t lLeftPercentile = 100.*(lCount - histo->GetBinContent(ibin))/histo->GetEntries();
      Double_t lRightPercentile = 100.*lCount / histo->GetEntries();
      
      Double_t lProportion = (lPercentile - lLeftPercentile)/(lRightPercentile-lLeftPercentile);
      
      lReturnValue = histo->GetBinLowEdge(ibin) + lProportion*lWidth;
      break;
    }
  }
  return lReturnValue;
}

void runMCCalibration(TString estimator = "FT0M" , TString histogramName = "hFT0C_Collisions"){
  const char* name2DHist = "FT0M";
  int runs[] = {564373,564359,564356};
  const int nRuns = sizeof(runs)/sizeof(int);
  cout<<"Processing "<<nRuns<<" runs..."<<endl;
  
  TH1F *hA = new TH1F("hA", "", nRuns, 0, nRuns);
  TH1F *hB = new TH1F("hB", "", nRuns, 0, nRuns);
  TH1F *hC = new TH1F("hC", "", nRuns, 0, nRuns);
  TH1F *hAmc = new TH1F("hAmc", "", nRuns, 0, nRuns);
  TH1F *hBmc = new TH1F("hBmc", "", nRuns, 0, nRuns);
  TH1F *hCmc = new TH1F("hCmc", "", nRuns, 0, nRuns);

  TCanvas *c1 = new TCanvas("c1","",800,600);
  c1->SetTicks(1,1);
  c1->SetTopMargin(0.02);
  c1->SetBottomMargin(0.15);
  c1->SetLeftMargin(0.16);
  c1->SetRightMargin(0.04);
  c1->SetFrameFillStyle(0);
  c1->SetFillStyle(0);
  
  int dataFiles = 0;
  int mcFiles = 0;
  
  for(int ii=0; ii<nRuns; ii++){
    TString filenameData = Form("../ARs_data/AR_%d.root", runs[ii]);
    TString filenameMC = Form("../ARs_mc/AR_%d.root", runs[ii]);
    TFile *fileData = new TFile(filenameData.Data(), "READ");
    TFile *fileMC = new TFile(filenameMC.Data(), "READ");
    
    if (!fileData || fileData->IsZombie() || fileData == nullptr) {
      cout<<"PROBLEM with fileData"<<endl;
    }
    if (!fileMC || fileMC->IsZombie() || fileMC == nullptr) {
      cout<<"PROBLEM with fileMC"<<endl;
    }

    int runNumber = runs[ii];
    cout<<"Run: "<<runNumber<<endl;
    bool goodToGo = true;
    
    if(gSystem->AccessPathName(filenameData.Data())) {
      cout<<"No data file! "<<endl;
      goodToGo = false;
    }else{
      dataFiles++;
    }
    
    if(gSystem->AccessPathName(filenameMC.Data())) {
      cout<<"No MC file! "<<endl;
      goodToGo = false;
    }else{
      mcFiles++;
    }
    if(!goodToGo){
      cout<<"-~> SKIPPED <~-"<<endl; 
      continue;
    }

    
    TH2F *h2dData = (TH2F*) fileData->Get(Form("centrality-study/hNGlobalTracksVs%s", name2DHist));
    if(!h2dData || h2dData == nullptr){
      cout<<"PROBLEM with h2dData"<<endl;
    }
    h2dData->SetName(Form("h2dData_%i", runNumber));
    TH2F *h2dMC = (TH2F*) fileMC->Get(Form("centrality-study/hNGlobalTracksVs%s", name2DHist));
    if(!h2dMC || h2dMC == nullptr){
      cout<<"PROBLEM with h2dMC"<<endl;
    }
    h2dMC->SetName(Form("h2dMC_%i", runNumber));
    
    TProfile *hProfData = h2dData->ProfileX(Form("hProfData_%i", runNumber));
    TProfile *hProfMC = h2dMC->ProfileX(Form("hProfMC_%i", runNumber));
    
    hProfData->SetLineColor(kBlack);
    hProfMC->SetLineColor(kRed);
    
    hProfData->GetYaxis()->SetTitle("Mean Nch");
    hProfData->GetXaxis()->SetTitle(Form("%s amplitude",estimator.Data()));
    hProfData->GetYaxis()->SetTitleSize(0.055);
    hProfData->GetXaxis()->SetTitleSize(0.055);
    hProfData->GetYaxis()->SetLabelSize(0.036);
    hProfData->GetXaxis()->SetLabelSize(0.036);
    hProfData->GetYaxis()->SetRangeUser(0.0, 360);

    hProfData->SetTitle("");
    hProfData->SetLineColor(kBlack);
    hProfData->SetMarkerColor(kBlack);
    hProfData->Draw();
    
    hProfMC->Draw("same");
    hProfData->Draw("same");
    
    // get the range in which the supercalibration is desirable
    TH1F *hV0Mfine = 0x0;
    hV0Mfine = (TH1F *) fileData -> Get(Form("centrality-study/%s", histogramName.Data()));
    if(!hV0Mfine){
      cout<<"PROBLEM with hV0Mfine"<<endl;
    }
    
    Double_t lFitRangeMax = GetBoundaryForPercentile(hV0Mfine, 0.1);
    cout<<"Fit range max estimated from histogram: "<<lFitRangeMax<<endl;
    
    Double_t lFitRangeMin = 0.015*GetBoundaryForPercentile(hV0Mfine, 0.01);
    if(lFitRangeMax<10000) lFitRangeMin = 0.02*GetBoundaryForPercentile(hV0Mfine, 0.01);
    
    cout<<"Fit range min estimated from histogram: "<<lFitRangeMin<<endl;
    
    hProfData->GetXaxis()->SetRangeUser(0.0, 30000);
    
    // perform supercalibration
    TF1 *f1 = new TF1("f1", "[0] + [1]*TMath::Power(x,[2])", lFitRangeMin,lFitRangeMax);
    TF1 *f1mc = new TF1("f1mc", "[0] + [1]*TMath::Power(x,[2])", lFitRangeMin,lFitRangeMax);
    
    f1->SetParameter(0, 0);
    f1->SetParameter(1, 3000./lFitRangeMax);
    f1->SetParameter(2, 1.0);
    f1mc->SetParameter(0, 0);
    f1mc->SetParameter(1, 3000./lFitRangeMax);
    f1mc->SetParameter(2, 1.0);
    
    hProfData->Fit("f1", "IREM0");
    hProfMC->Fit("f1mc", "IREM0");
    
    f1->SetLineStyle(2);
    f1->SetLineColor(kGray+1);
    f1mc->SetLineStyle(2);
    f1mc->SetLineColor(kRed);
    
    f1->Draw("same");
    f1mc->Draw("same");
    
    //data boundary for 1%
    double xBoundFor1 = GetBoundaryForPercentile(hV0Mfine, 1);
    
    TLegend *leg3 = new TLegend(0.20, 0.76, 0.656, 0.939);
    leg3->SetBorderSize(0);
    leg3->SetFillStyle(0);
    leg3->SetNColumns(1);
    leg3->SetColumnSeparation(0.001);
    leg3->AddEntry(hProfData, Form("Real Data, %i", runNumber), "l");
    leg3->AddEntry(hProfMC, Form("Monte Carlo, %i", runNumber), "l");
    leg3->AddEntry(f1, Form("data fit: %.2f + %.3f#times(FT0C)^{%.3f}", f1->GetParameter(0), f1->GetParameter(1), f1->GetParameter(2)), "l");
    leg3->AddEntry(f1mc, Form("mc fit: %.2f + %.3f#times(FT0C)^{%.3f}", f1mc->GetParameter(0), f1mc->GetParameter(1), f1mc->GetParameter(2)), "l");
    leg3->AddEntry((TObject*)0x0, Form("1%% boundary in data: %.1f", xBoundFor1), "");
    leg3->Draw();
    c1->SaveAs(Form("mc/%s_superCalib_%i.pdf", estimator.Data(), runNumber));
    
    hA -> SetBinContent(ii+1, f1->GetParameter(0));
    hB -> SetBinContent(ii+1, f1->GetParameter(1));
    hC -> SetBinContent(ii+1, f1->GetParameter(2));
    hAmc -> SetBinContent(ii+1, f1mc->GetParameter(0));
    hBmc -> SetBinContent(ii+1, f1mc->GetParameter(1));
    hCmc -> SetBinContent(ii+1, f1mc->GetParameter(2));
    
    TFile *mcCalibFile = new TFile(Form("mc/%s_mcCalib_%i.root",estimator.Data(), runNumber), "RECREATE");
    //return pow(((pars[0] + pars[1] * pow(x, pars[2])) - pars[3]) / pars[4], 1.0f / pars[5]);
    TFormula *f1scale = new TFormula("f1scale", "TMath::Power((([0]+[1]*TMath::Power(x,[2]))-[3])/[4],1.0/[5])");
    // correct order: 0, 1, 2: are from monte carlo!
    // correct order: 3, 4, 5: are from real data!
    
    f1scale->SetParameter(0,f1mc->GetParameter(0));
    f1scale->SetParameter(1,f1mc->GetParameter(1));
    f1scale->SetParameter(2,f1mc->GetParameter(2));
    f1scale->SetParameter(3,f1->GetParameter(0));
    f1scale->SetParameter(4,f1->GetParameter(1));
    f1scale->SetParameter(5,f1->GetParameter(2));
    
    f1scale->Write();
    mcCalibFile->Write();
    mcCalibFile->Close();

  }
  
  fileData->Close();
  fileMC->Close();
  
  cout<<"n runs "<<nRuns<<" data files "<<dataFiles<<" MC files "<<mcFiles<<endl;
  
  // trending of supercalibration
  
  TCanvas *c5 = new TCanvas("c5","c5",1200,1000);
  c5->SetFrameFillStyle(0);
  c5->SetFillStyle(0);
  c5->Divide(1,3);
  c5->cd(1)->SetPad(0, 0.0, 1, 0.4);
  c5->cd(2)->SetPad(0, 0.4, 1, 0.7);
  c5->cd(3)->SetPad(0, 0.7, 1, 1.0);
  
  c5->cd(1)->SetTicks(1,1);
  c5->cd(1)->SetTopMargin(0.002);
  c5->cd(1)->SetBottomMargin(0.24);
  c5->cd(1)->SetLeftMargin(0.13);
  c5->cd(1)->SetRightMargin(0.04);
  c5->cd(1)->SetFrameFillStyle(0);
  c5->cd(1)->SetFillStyle(0);
  
  c5->cd(2)->SetTicks(1,1);
  c5->cd(2)->SetTopMargin(0.002);
  c5->cd(2)->SetBottomMargin(0.002);
  c5->cd(2)->SetLeftMargin(0.13);
  c5->cd(2)->SetRightMargin(0.04);
  c5->cd(2)->SetFrameFillStyle(0);
  c5->cd(2)->SetFillStyle(0);
  
  c5->cd(3)->SetTicks(1,1);
  c5->cd(3)->SetTopMargin(0.022);
  c5->cd(3)->SetBottomMargin(0.002);
  c5->cd(3)->SetLeftMargin(0.13);
  c5->cd(3)->SetRightMargin(0.04);
  c5->cd(3)->SetFrameFillStyle(0);
  c5->cd(3)->SetFillStyle(0);
  
  float factorScale = 1.333;
  
  hA->GetYaxis()->SetTitleSize(0.055);
  hA->GetXaxis()->SetTitleSize(0.055);
  hA->GetYaxis()->SetLabelSize(0.036);
  hA->GetXaxis()->SetLabelSize(0.05);
  hB->GetYaxis()->SetTitleSize(0.055*factorScale);
  hB->GetXaxis()->SetTitleSize(0.055*factorScale);
  hB->GetYaxis()->SetLabelSize(0.036*factorScale);
  hB->GetXaxis()->SetLabelSize(0.036*factorScale);
  hC->GetYaxis()->SetTitleSize(0.055*factorScale);
  hC->GetXaxis()->SetTitleSize(0.055*factorScale);
  hC->GetYaxis()->SetLabelSize(0.036*factorScale);
  hC->GetXaxis()->SetLabelSize(0.036*factorScale);
  hA->LabelsOption("v");
  
  for(int ii=0; ii<nRuns; ii++){
    hA->GetXaxis()->SetBinLabel(ii+1, Form("%i", runs[ii]));
    hB->GetXaxis()->SetBinLabel(ii+1, Form("%i", runs[ii]));
    hC->GetXaxis()->SetBinLabel(ii+1, Form("%i", runs[ii]));
  }
  
  hA->SetLineColor(kBlack);
  hB->SetLineColor(kBlack);
  hC->SetLineColor(kBlack);
  hAmc->SetLineColor(kRed+1);
  hBmc->SetLineColor(kRed+1);
  hCmc->SetLineColor(kRed+1);
  hA->GetYaxis()->SetRangeUser(0,650);
  hB->GetYaxis()->SetRangeUser(0,0.688);
  hC->GetYaxis()->SetRangeUser(0.91,1.92);
  
  hA->GetYaxis()->SetTitle("A");
  hB->GetYaxis()->SetTitle("B");
  hC->GetYaxis()->SetTitle("C");
  
  TLegend *leg2a = new TLegend(0.609, 0.78, 0.81, 0.918);
  leg2a->SetBorderSize(0);
  leg2a->SetFillStyle(0);
  leg2a->AddEntry(hA, "Data", "l");
  leg2a->AddEntry(hAmc, "Monte Carlo", "l");
  
  c5->cd(1);
  hA->Draw();
  hAmc->Draw("same");
  leg2a->Draw();
  c5->cd(2);
  hB->Draw();
  hBmc->Draw("same");
  leg2a->Draw();
  c5->cd(3);
  hC->Draw();
  hCmc->Draw("same");
  leg2a->Draw();
  
  cout<<"calculating averages ...." <<endl;
  float avA = 0.0f, avB = 0.0f, avC = 0.0f;
  float avAmc = 0.0f, avBmc = 0.0f, avCmc = 0.0f;
  
  for(Int_t ib=1; ib<31; ib++){
    avA += hA->GetBinContent(ib);
    avB += hB->GetBinContent(ib);
    avC += hC->GetBinContent(ib);
    avAmc += hAmc->GetBinContent(ib);
    avBmc += hBmc->GetBinContent(ib);
    avCmc += hCmc->GetBinContent(ib);
  }
  
  avA /= static_cast<double>(nRuns);
  avB /= static_cast<double>(nRuns);
  avC /= static_cast<double>(nRuns);
  avAmc /= static_cast<double>(nRuns);
  avBmc /= static_cast<double>(nRuns);
  avCmc /= static_cast<double>(nRuns);
  
  TFile *mcCalibFileDefault = new TFile(Form("mc/%s_mcCalibDefault.root",estimator.Data()), "RECREATE");
  //return pow(((pars[0] + pars[1] * pow(x, pars[2])) - pars[3]) / pars[4], 1.0f / pars[5]);
  TFormula *f1scaleDefault = new TFormula("f1scaleDefault", "TMath::Power((([0]+[1]*TMath::Power(x,[2]))-[3])/[4],1.0/[5])");
  // correct order: 0, 1, 2: are from monte carlo!
  // correct order: 3, 4, 5: are from real data!
  
  f1scaleDefault->SetParameter(0,avAmc);
  f1scaleDefault->SetParameter(1,avBmc);
  f1scaleDefault->SetParameter(2,avCmc);
  f1scaleDefault->SetParameter(3,avA);
  f1scaleDefault->SetParameter(4,avB);
  f1scaleDefault->SetParameter(5,avC);
  f1scaleDefault->Write();
  mcCalibFileDefault->Write();
  mcCalibFileDefault->Close();
  
  c5->SaveAs(Form("%s_monteCarloFitTrend.pdf", estimator.Data()));
  c5->SaveAs(Form("%s_monteCarloFitTrend.root", estimator.Data()));
}

