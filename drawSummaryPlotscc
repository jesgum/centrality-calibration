TH1D *GetCentralityHisto(TH1F *hMult, TH1 *hCalib, Double_t lCent1, Double_t lCent2, TString lName = "h"){
  TH1D *h = (TH1D*) hMult->Clone(lName.Data());
  
  //cout<<"lLo = "<<lLo<<", lHi = "<<lHi<<endl;
  for(Int_t ii=1; ii<hMult->GetNbinsX()+1; ii++) {
    float centrality = hCalib->GetBinContent( hCalib->FindBin(hMult->GetBinCenter(ii)));
    if( centrality < lCent1 ) h->SetBinContent(ii, 0);
    if( centrality > lCent2 ) h->SetBinContent(ii, 0);
  }
  return h;
}

Double_t GetMidRange(TH1* histo, Double_t loweringEdge = 56000){
  // this calculates the average of rising edge vs lowering edge.
  Double_t risingEdge = -1;
  for(int ii=1; ii<histo->GetNbinsX(); ii++){
    if(histo->GetBinContent(ii)>0){
      risingEdge = histo->GetBinCenter(ii);
      break;
    }
  }
  for(int ii=histo->FindBin(loweringEdge); ii>0; ii--){
    if(histo->GetBinContent(ii)>0){
      loweringEdge = histo->GetBinCenter(ii);
      break;
    }
  }
  return 0.5*(risingEdge+loweringEdge);
}

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

void drawSummaryPlots( TString lInputFileName = "AnalysisResultsLHC24ar_glauberNBD_ancestorMode2_isCollision0.root",
                         TString lRunNumber = "000000",
                         TString lXaxisTitle = "FT0C Amplitude"
                         ){

  TFile *file = new TFile(lInputFileName.Data(), "READ");
  file->ls();

  TString calibFileName = lInputFileName.Data(); 
  calibFileName.ReplaceAll("glauberNBD", "calibration");
  
  TFile *file2 = new TFile(calibFileName.Data(), "READ");
  file2->ls();
  
//  return ;
  
  TH1F *hV0M = (TH1F*) file2->Get("hData");
  if(!hV0M) hV0M = (TH1F*) file2->Get("hV0MUltraFine");
  //TH1F *hRatio = (TH1F*) file->Get("hRatio");
//  hRatio->SetName("hRatio");
//  TH1F *hRawStitched = (TH1F*) file2->Get("hCalib");
  TH1F *hGlauberFunctionHyperFine = (TH1F*) file2->Get("hGlauber");
  
  int rebinRatio = 20;
  TH1F *hRatio = (TH1F*) hV0M->Clone("hRatio");
  TH1F *hGlauberFunction = (TH1F*) hGlauberFunctionHyperFine->Clone("hGlauberFunction");
  hRatio->Rebin(rebinRatio);
  hGlauberFunction->Rebin(rebinRatio);
  hRatio->Divide(hGlauberFunction);
  
  hGlauberFunctionHyperFine->SetName("hGlauber_bc");
  TH1F *hCalib = (TH1F*) file2->Get("hCalib");
  
  // Double_t lFitRangeMax = 56000;
  // Double_t lFitRange = /*350*/300.;

  Double_t lFitRange = 0.012*GetBoundaryForPercentile(hV0M, 0.01);
  cout<<"Fit range min estimated fromm histogram: "<<lFitRange<<endl; 

  Double_t lFitRangeMax = GetBoundaryForPercentile(hV0M, 0.002);
  cout<<"Fit range max estimated from histogram: "<<hV0M<<endl;
  
  TCanvas *c1 = new TCanvas("c1", "", 900,900);
  c1->SetFrameFillStyle(0);
  c1->SetFillStyle(0);
  c1->Divide(1,2);
  c1->SetFrameFillStyle(0);
  c1->SetFillStyle(0);
  
  
  c1->cd(1);
  c1->cd(1)->SetLogy();
  c1->cd(1)->SetTicks(1,1);
  c1->cd(1)->SetPad(0,0.5,1,1);
  c1->cd(2)->SetPad(0,0.0,1,.5);
  
  c1->cd(1)->SetBottomMargin(0.001);
  c1->cd(1)->SetRightMargin(0.05);
  c1->cd(1)->SetTopMargin(0.02);
  c1->cd(1)->SetLeftMargin(0.08);
  
  c1->cd(2)->SetBottomMargin(0.14);
  c1->cd(2)->SetRightMargin(0.05);
  c1->cd(2)->SetTopMargin(0.001);
  c1->cd(2)->SetLeftMargin(0.08);
  c1->cd(2)->SetTicks(1,1);
  c1->cd(1);
  
  
  hV0M->GetXaxis()->SetRangeUser(0,lFitRangeMax);
  hV0M->GetYaxis()->SetRangeUser(0.25,hV0M->GetMaximum()*3);
  hV0M->SetLineColor(kBlack);
  hV0M->SetMarkerStyle(20);
  hV0M->SetMarkerColor(kBlack);
  hV0M->SetMarkerSize(0.5);
  hV0M->GetYaxis()->SetTitleSize(0.07);
  hV0M->GetYaxis()->SetLabelSize(0.05);
  hV0M->GetYaxis()->SetTitle("Count");
  hV0M->GetYaxis()->SetTitleOffset(0.9);
  hV0M->GetXaxis()->SetLabelSize(0.05);
  hV0M->GetXaxis()->SetTitleSize(0.06);
  hV0M->GetXaxis()->SetTitle(lXaxisTitle.Data());
  hV0M->GetYaxis()->SetTickLength(0.015);
  hV0M->SetStats(0);
  hV0M->SetTitle("");
  hV0M->Draw("hist");
  
  
  TH1D* hSlice[25];
  TLatex *latPer = new TLatex();
  latPer->SetNDC();
  latPer->SetTextFont(42);
  latPer->SetTextSize(0.04);
  
  TLatex *latPerAngle = new TLatex();
  latPerAngle->SetTextFont(42);
  latPerAngle->SetTextAlign(22);
  latPerAngle->SetTextSize(0.030);
  latPerAngle->SetTextAngle(90);
  
  for(Int_t ii=0; ii<20; ii=ii+1){
    hSlice[ii] = GetCentralityHisto( hV0M, hCalib, ii*5, ii*5+5, Form("h%i", ii) );
    hSlice[ii] ->SetFillColorAlpha(kBlack,0.1);
    if(ii%2==0)
      hSlice[ii]->Draw("same hist");
  }
  
  for(Int_t ii=0; ii<20; ii=ii+1){
    if( ii*5<60 ){
      latPerAngle->DrawLatex( GetMidRange(hSlice[ii], lFitRangeMax), 1, Form("%i-%i%%", ii*5, ii*5+5));
      cout<<"Mean: "<<hSlice[ii]->GetMean()<<" midrange: "<<GetMidRange(hSlice[ii], lFitRangeMax)<<endl;
    }
  }
  hGlauberFunctionHyperFine->Draw("same hist");
  latPer->SetTextSize(0.06);
  latPer-> DrawLatex(0.12, 0.88, "ALICE Pb-Pb 5.36 TeV");
  latPer->SetTextSize(0.033);
  latPer -> DrawLatex(0.12,0.835, "5%-wide intervals shown alternatingly");
  latPer->SetTextSize(0.062);
  latPer -> DrawLatex(0.12,0.7, Form("Run %s", lRunNumber.Data()));
  
  TLegend *legO = new TLegend(0.12, 0.76, 0.42, 0.821);
  hGlauberFunctionHyperFine->SetMarkerColor(kRed);
  legO->SetBorderSize(0);
  legO->SetFillColorAlpha(0, 0);
  legO->AddEntry(hGlauberFunctionHyperFine, "Glauber + NBD fit");
  legO->Draw();
  
  // create extra inset pad
  
  TPad *inset1 = new TPad("inset1", "",0.575724, 0.441648, 0.924, 0.961098);
  inset1->SetPad(0.575724, 0.441648, 0.924, 0.961098);
  //inset1->SetFillColor(kGray);
  inset1->cd();
  
  TH1F *hV0Mzoomed = (TH1F*) hV0M->Clone("hV0Mzoomed");
  hV0Mzoomed -> GetXaxis()->SetRangeUser(0,590*lFitRangeMax/59000.);
  hV0Mzoomed -> GetXaxis()->SetNdivisions(404);
  hV0Mzoomed -> GetXaxis()->SetTitleSize(0.1);
  hV0Mzoomed -> GetYaxis()->UnZoom();
  hV0Mzoomed -> GetYaxis()->SetRangeUser(hGlauberFunctionHyperFine->GetMaximum() * 0.05, hV0Mzoomed->GetMaximum() * 5.5);
  hV0Mzoomed -> GetYaxis()->SetTitleSize(0.1);
  hV0Mzoomed -> GetYaxis()->SetTitleOffset(1.1); 
  hV0Mzoomed -> GetXaxis()->SetLabelSize(0.06);
  hV0Mzoomed -> GetYaxis()->SetLabelSize(0.06);
  hV0Mzoomed->Draw();
  
  for(Int_t ii=0; ii<20; ii=ii+2){
    hSlice[ii]->Draw("same hist");
  }
  latPerAngle->SetTextSize(0.037);
  latPerAngle->SetTextAngle(0);
  for(Int_t ii=0; ii<20; ii=ii+1){
    if( ii*5>77 )
      latPerAngle->DrawLatex( GetMidRange(hSlice[ii],lFitRangeMax), hGlauberFunctionHyperFine->GetMaximum() * 0.12, Form("%i-%i%%", ii*5, ii*5+5));
  }
  latPerAngle->SetTextSize(0.033);
  latPerAngle->SetTextAngle(90);
  
  hGlauberFunctionHyperFine->SetLineColor(kRed);
  hGlauberFunctionHyperFine->Draw("same hist");
  
  latPer->SetTextSize(0.06);
  latPer->SetTextAlign(32);
//  latPer->DrawLatex(0.94,0.825, "Shown: glauber normalized after fit ");
//  latPer->DrawLatex(0.94,0.75, "to integral in range: 275-325");
  
  
  c1->cd(1);
  
  inset1->SetLogy();
  inset1->SetTicks(1,1);
  inset1->SetBottomMargin(0.194);
  inset1->SetRightMargin(0.03);
  inset1->SetTopMargin(0.085);
  inset1->SetLeftMargin(0.11);
  
  
  
  inset1->Draw("same");
  
  
  c1->cd(2);
  Float_t lLoRangeRatio = 0.62;
  Float_t lHiRangeRatio = 2.495;
  
  hRatio->GetYaxis()->SetTitle("Data/Fit");
  hRatio->GetXaxis()->SetTitle(lXaxisTitle.Data());
  hRatio->GetYaxis()->SetTitleSize(0.055);
  hRatio->GetYaxis()->SetTitleOffset(0.7);
  hRatio->GetXaxis()->SetTitleSize(0.055);
  hRatio->GetYaxis()->SetLabelSize(0.045);
  hRatio->GetXaxis()->SetLabelSize(0.045);
  hRatio->GetYaxis()->SetRangeUser(lLoRangeRatio,lHiRangeRatio);
  hRatio->GetXaxis()->SetRangeUser(0,lFitRangeMax);
  hRatio->SetMarkerStyle(20);
  hRatio->SetMarkerColor(kRed);
  hRatio->SetLineColor(kRed);
  //hRatio->SetMarkerSize(1.0);
  hRatio->SetMarkerSize(.5);
  hRatio->SetStats(0);
  hRatio->SetTitle("");
  
  hRatio->Draw("hist");
//  hRatioWide->SetLineColor(kRed);
//  hRatioWide->SetMarkerColor(kRed);
//  hRatioWide->SetMarkerStyle(20);
//  hRatioWide->SetMarkerSize(1.2);
//  hRatioWide->SetLineWidth(2);
  
  TLine *line = new TLine(0,1,lFitRangeMax,1);
  line->SetLineStyle(7);
  line->SetLineColor(kGray+1);
  line->Draw();
  
  TLine *lFitRangeLine = new TLine( lFitRange, lLoRangeRatio, lFitRange, 0.9) ;
  lFitRangeLine->SetLineColor(kBlue);
  lFitRangeLine->SetLineWidth(1);
  lFitRangeLine->SetLineStyle(2);
  //lFitRangeLine->Draw();
  
  TH1D *hRatioZoomed = (TH1D*) hRatio->Clone("hRatioZoomed");
  hRatioZoomed->GetXaxis()->SetRangeUser(0,1150*lFitRangeMax/59000.);

  
  hRatio->SetLineWidth(1);
  hRatio->Draw("same hist");
  
  latPerAngle->SetTextColor(kRed);
  for(Int_t ii=0; ii<20; ii=ii+1){
    if( ii*5<60 )
    latPerAngle->DrawLatex( GetMidRange(hSlice[ii], lFitRangeMax), 0.74, Form("%i-%i%%", ii*5, ii*5+5));
  }
  //hRatioWide->Draw("same");
  
  TH1D* hSliceRatio[25];
  
  for(Int_t ii=0; ii<20; ii=ii+2){
    hSliceRatio[ii] = GetCentralityHisto( hRatio, hCalib, ii*5, ii*5+5, Form("h%i", ii) );
    hSliceRatio[ii] ->SetFillColorAlpha(kRed,0.1);
    hSliceRatio[ii]->Draw("same hist");
    //if( ii*5<60 )
    //latPerAngle->DrawLatex( hSlice[ii]->GetMean(), 1, Form("%i-%i%%", ii*5, ii*5+5));
  }

  
  TPad *inset2 = new TPad("inset1", "",0.104677, 0.47032, 0.558775, 0.965753);
  inset2->SetPad(0.104677, 0.47032, 0.558775, 0.965753);
  //inset2->SetFillColor(kGray);
  inset2->SetTicks(1,1);
  inset2->SetBottomMargin(0.194);
  inset2->SetRightMargin(0.03);
  inset2->SetTopMargin(0.045);
  inset2->SetLeftMargin(0.11);
  inset2->cd();
  hRatioZoomed -> GetYaxis()->SetRangeUser(lLoRangeRatio, 1.55);
  hRatioZoomed -> GetYaxis()->SetTitleSize(0.1);
  hRatioZoomed -> GetXaxis()->SetTitleSize(0.1);
  hRatioZoomed -> GetYaxis()->SetTitleOffset(1.1);
  hRatioZoomed -> GetXaxis()->SetLabelSize(0.06);
  hRatioZoomed -> GetYaxis()->SetLabelSize(0.06);
  hRatioZoomed->Draw("hist");
  
  TLine *line2 = new TLine(0,1,1150,1);
  line2->SetLineStyle(7);
  line2->SetLineColor(kGray+1);
  line2->Draw();
  hRatioZoomed->Draw("hist same");
  
  for(Int_t ii=0; ii<20; ii=ii+2){
    hSliceRatio[ii]->Draw("same hist");
    //if( ii*5<60 )
    //latPerAngle->DrawLatex( hSlice[ii]->GetMean(), 1, Form("%i-%i%%", ii*5, ii*5+5));
  }
  latPerAngle->SetTextSize(0.04);
  latPerAngle->SetTextAngle(45);
  for(Int_t ii=0; ii<20; ii=ii+1){
    if( ii*5>70 )
      latPerAngle->DrawLatex( GetMidRange(hSlice[ii], lFitRangeMax)+10, 0.79, Form("%i-%i%%", ii*5, ii*5+5));
  }
  latPerAngle->SetTextSize(0.033);
  latPerAngle->SetTextAngle(90);
  
  c1->cd(2);
  inset2->Draw();
  
  TString exportName = calibFileName; 
  exportName.ReplaceAll("calibration", "summaryPlot"); 
  exportName.ReplaceAll(".root", ".pdf"); 

  c1->SaveAs(exportName.Data());
}
