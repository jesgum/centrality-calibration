void drawCentralityBands(TPad* pad, TH1D* hRef, const std::vector<double>& centEdges,
                         double yFloor, int color, int lineColor, float alpha,
                         float xmin, float xmax, int iLabelStart = 0, int iLabelStop = -1,
                         float labelSize = 0.025, float labelAngle = 90)
{
  pad->cd();
  double xMin = xmin;
  double xMax = xmax;
  double yMax = hRef->GetMaximum();
  int nBands = (int)centEdges.size() - 1;
  if (iLabelStop < 0) {
    iLabelStop = nBands - 1;
  }

  for (int i = 0; i + 1 < (int)centEdges.size(); i++) {
    double bandCenterX = 0.5 * (centEdges[i] + centEdges[i + 1]);

    if (i % 2 != 0) {
      double bandX1 = TMath::Max(centEdges[i], xMin);
      double bandX2 = TMath::Min(centEdges[i + 1], xMax);

      if (i == nBands - 1) {
        bandX2 = xMax;
      }
      if (bandX1 < bandX2) {
        int binL = hRef->FindBin(bandX1);
        int binR = hRef->FindBin(bandX2);
        for (int b = binL; b <= binR; b++) {
          double x1   = TMath::Max(hRef->GetXaxis()->GetBinLowEdge(b), bandX1);
          double x2   = TMath::Min(hRef->GetXaxis()->GetBinUpEdge(b),  bandX2);
          if (b == binR) {
            x2 = bandX2;  // force last bin to reach exactly to band edge
          }
          double yTop = TMath::Min(hRef->GetBinContent(b), yMax);
          if (yTop <= yFloor || x1 >= x2) {
            continue;
          }
          TBox* box = new TBox(x1, yFloor, x2, yTop);
          box->SetFillColorAlpha(color, alpha);
          box->SetLineWidth(0);
          box->Draw();
        }

        auto drawEdgeLine = [&](double x) {
          if (x <= xMin || x >= xMax) {
            return;
          }
          double yTop = TMath::Min(hRef->GetBinContent(hRef->FindBin(x)), yMax);
          TLine* line = new TLine(x, yFloor, x, yTop);
          line->SetLineColor(lineColor);
          line->SetLineWidth(1);
          line->SetLineStyle(kSolid);
          line->Draw();
        };
        drawEdgeLine(bandX1);
        drawEdgeLine(bandX2);
      }
    }

    if (i >= iLabelStart && i <= iLabelStop) {
      int centLow  = (nBands - 1 - i) * 5;
      int centHigh = (nBands     - i) * 5;
      double textHeight = labelSize * (1 - pad->GetLeftMargin() - pad->GetRightMargin());
      double ndcX = (bandCenterX - xMin) / (xMax - xMin) * (1 - pad->GetLeftMargin() - pad->GetRightMargin()) + pad->GetLeftMargin();
      double ndcY = pad->GetBottomMargin() + 0.08;
      if (labelAngle == 60) {
        ndcY += 0.04;
      }
      if (labelAngle > 0) {
        ndcX += textHeight * 0.25 * TMath::Sin(labelAngle * TMath::DegToRad());
        if (i == nBands - 1) {
          ndcX -= 0.1;
        }
      }

      TLatex* lat = new TLatex(ndcX, ndcY, TString::Format("%d-%d%%", centLow, centHigh));
      lat->SetNDC();
      lat->SetTextAlign(labelAngle > 0 ? 21 : 22);
      lat->SetTextAngle(labelAngle);
      lat->SetTextSize(labelSize);
      lat->SetTextColor(lineColor);
      lat->Draw();
    }
  }
}

void drawGlauberPbPb(const char* infile = "glauber_hFT0C_BCs.root", const char* outfile = "hGlauber.pdf", const int range = 60000)
{
  gStyle->SetOptStat(0);
  TFile* file = new TFile(infile, "read");

  if (file->IsZombie()) {
    cerr << "Spooky scary zombiefile!" << endl;
    return;
  }

  TH1D* hFT0C = (TH1D*)file->Get("hData");
  TH1D* hGlauber = (TH1D*)file->Get("hGlauber");

  if (!hFT0C) {
    cerr << "hV0MUltraFine is nullptr" << endl;;
  }

  if (!hGlauber) {
    cerr << "hGlauber is nullptr" << endl;;
  }


  std::vector<double> centEdges;
  double total = hGlauber->Integral();
  double cumulative = 0;
  centEdges.push_back(hGlauber->GetXaxis()->GetXmax());
  for (int i = hGlauber->GetNbinsX(); i >= 1; i--) {
    cumulative += hGlauber->GetBinContent(i);
    if (cumulative / total >= 0.05 * (int)centEdges.size()) {
      centEdges.push_back(hGlauber->GetBinCenter(i));
    }
    if ((int)centEdges.size() > 20) {
      break;
    }
  }
  std::reverse(centEdges.begin(), centEdges.end());

  int xRange = range;
  int xRangeInset1 = 630;
  int xRangeInset2 = 1099;
  hFT0C->GetXaxis()->SetRangeUser(0, xRange);
  hFT0C->GetXaxis()->SetTitle("FT0C Amplitude");
  hFT0C->GetYaxis()->SetTitleOffset(0.7);
  hGlauber->GetXaxis()->SetRangeUser(0, xRange);
  hGlauber->SetLineColor(kRed);
  hGlauber->SetLineWidth(2);

  TH1D* hInsetFT0C = (TH1D*)hFT0C->Clone("hInsetFT0C");
  TH1D* hInsetGlauber = (TH1D*)hGlauber->Clone("hGlauber");

  hInsetFT0C->GetXaxis()->SetRangeUser(0, xRangeInset1);
  hInsetFT0C->GetYaxis()->SetRangeUser(1e3, 3e7);
  hInsetGlauber->GetXaxis()->SetRangeUser(0, xRangeInset1);

  TCanvas* canv = new TCanvas("canv", "", 1200, 800);
  canv->SetMargin(0.1, 0.03, 0.13, 0.03);
  canv->SetLogy();
  hFT0C->Draw();
  hGlauber->Draw("same");
  // canv->SaveAs("hGlauber.pdf");

  TCanvas* canvInset = new TCanvas("canvInset", "", 1200, 800);
  canvInset->SetMargin(0.1, 0.03, 0.13, 0.03);
  canvInset->SetLogy();
  hInsetFT0C->Draw();
  hInsetGlauber->Draw("same");
  // canvInset->SaveAs("hInsetGlauber.pdf");

  int rebinFactor = 80;
  TH1D* hRatio = (TH1D*)hGlauber->Clone("hRatio");
  hRatio->Divide(hFT0C, hGlauber, 1, 1, "B");
  hRatio->Rebin(rebinFactor);
  hRatio->Scale(1. / rebinFactor);
  hRatio->SetTitle("");
  hRatio->GetXaxis()->SetTitle("FT0C Amplitude");
  hRatio->GetYaxis()->SetTitle("Data/Fit");
  hRatio->GetXaxis()->SetRangeUser(0, xRange);
  
  TLine* unity = new TLine(0, 1, xRange, 1);
  unity->SetLineWidth(2);
  unity->SetLineColor(kBlack);
  unity->SetLineStyle(7);

  TCanvas* canvRatio = new TCanvas("canvRatio", "", 1200, 800);
  canvRatio->SetMargin(0.1, 0.03, 0.13, 0.03);
  hRatio->GetYaxis()->SetRangeUser(0.61, 2.19);
  hRatio->Draw("hist");
  unity->Draw();
  // canvRatio->SaveAs("hRatio.pdf");

  TLine* insetUnity = new TLine(0, 1, xRangeInset2, 1);
  insetUnity->SetLineWidth(2);
  insetUnity->SetLineColor(kBlack);
  insetUnity->SetLineStyle(7);

  int rebinFactorInset = 40;
  TH1D* hInsetRatio = (TH1D*)hInsetGlauber->Clone("hInsetGlauber");
  hInsetRatio->Divide(hInsetFT0C, hInsetGlauber, 1, 1, "B");
  hInsetRatio->Rebin(rebinFactorInset);
  hInsetRatio->Scale(1. / rebinFactorInset);
  hInsetRatio->SetTitle("");
  hInsetRatio->GetXaxis()->SetTitle("FT0C Amplitude");
  hInsetRatio->GetYaxis()->SetTitle("Data/Fit");
  hInsetRatio->GetXaxis()->SetRangeUser(0, xRangeInset2);

  TCanvas* canvInsetRatio = new TCanvas("canvInsetRatio", "", 1200, 800);
  canvInsetRatio->SetMargin(0.1, 0.03, 0.13, 0.03);
  hInsetRatio->GetYaxis()->SetRangeUser(0.65, 1.55);
  hInsetRatio->Draw("hist");
  insetUnity->Draw();
  // canvInsetRatio->SaveAs("hInsetRatio.pdf");

  TCanvas* canvGlauberFull = new TCanvas("canvGlauberFull", "", 1200, 1200);

  TPad* padUp = new TPad("padUp", "", 0, 0.5, 1, 1);
  padUp->SetMargin(0.1, 0.03, 0, 0.03);
  padUp->SetLogy();
  padUp->Draw();
  padUp->cd();
  padUp->SetTicks(1, 1);
  hFT0C->GetYaxis()->SetLabelSize(0.06);
  hFT0C->Draw("");
  drawCentralityBands(padUp, hGlauber, centEdges, 1e-1, kGray, kBlack, 1, 0, xRange, 8, -1, 0.03, 90);
  hFT0C->Draw("same");
  hGlauber->Draw("same");
  padUp->RedrawAxis();
  canvGlauberFull->cd();

  TPad* padDown = new TPad("padDown", "", 0, 0, 1, 0.5);
  padDown->SetMargin(0.1, 0.03, 0.16, 0);
  padDown->Draw();
  padDown->cd();
  padDown->SetTicks(1, 1);
  hRatio->GetYaxis()->SetLabelSize(0.06);
  hRatio->GetXaxis()->SetLabelSize(0.06);
  hRatio->GetXaxis()->SetTitleSize(0.06);
  hRatio->GetYaxis()->SetTitleSize(0.06);
  hRatio->GetYaxis()->SetTitleOffset(0.8);
  hRatio->Draw("hist");
  drawCentralityBands(padDown, hRatio, centEdges, 0.61, kRed, kRed, 0.15, 0, xRange, 8, -1, 0.03, 90);
  hRatio->Draw("hist same");
  unity->Draw("same");
  padDown->RedrawAxis();
  canvGlauberFull->cd();

  TPad* padUpInset = new TPad("padUpInset", "", 0.55, 0.71, 0.95, 0.96);
  padUpInset->SetMargin(0.15, 0.03, 0.19, 0.03);
  padUpInset->SetLogy();
  padUpInset->SetFillColorAlpha(0, 0);
  padUpInset->Draw();
  padUpInset->cd();
  padUpInset->SetTicks(1, 1);
  hInsetFT0C->GetXaxis()->SetLabelSize(0.08);
  hInsetFT0C->GetXaxis()->SetTitleSize(0.1);
  hInsetFT0C->GetXaxis()->SetTitleOffset(0.9);
  hInsetFT0C->GetYaxis()->SetLabelSize(0.08);
  hInsetFT0C->GetYaxis()->SetTitleSize(0.1);
  hInsetFT0C->GetYaxis()->SetTitleOffset(0.75);
  hInsetFT0C->Draw();
  drawCentralityBands(padUpInset, hInsetGlauber, centEdges, 1e+3, kGray, kBlack, 1, 0, xRangeInset1, 0, 3, 0.05, 0);
  hInsetFT0C->Draw("same");
  hInsetGlauber->Draw("same");
  padUpInset->RedrawAxis();
  canvGlauberFull->cd();

  TPad* padDownInset = new TPad("padDownInset", "", 0.15, 0.225, 0.6, 0.475);
  padDownInset->SetMargin(0.15, 0.03, 0.19, 0.03);
  padDownInset->Draw();
  padDownInset->cd();
  padDownInset->SetTicks(1, 1);
  hInsetRatio->GetXaxis()->SetLabelSize(0.08);
  hInsetRatio->GetXaxis()->SetTitleSize(0.1);
  hInsetRatio->GetXaxis()->SetTitleOffset(0.9);
  hInsetRatio->GetYaxis()->SetLabelSize(0.08);
  hInsetRatio->GetYaxis()->SetTitleSize(0.1);
  hInsetRatio->GetYaxis()->SetTitleOffset(0.75);
  hInsetRatio->Draw("hist");
  drawCentralityBands(padDownInset, hInsetRatio, centEdges, 0.65, kRed, kRed, 0.15, 0, xRangeInset2+20, 0, 4, 0.05, 60);
  hInsetRatio->Draw("same hist");
  insetUnity->Draw("same");
  padDownInset->RedrawAxis();
  canvGlauberFull->cd();

  TLatex* lat = new TLatex();
  lat->SetNDC();
  lat->SetTextFont(42);
  lat->SetTextSize(0.03);
  lat->DrawLatexNDC(0.14, 0.94, "ALICE Pb-Pb #sqrt{#it{s}_{NN}} = 5.36 TeV");
  lat->SetTextSize(0.02);
  lat->DrawLatexNDC(0.14, 0.91, "5%-wide intervals shown alternatingly");
  lat->DrawLatexNDC(0.14, 0.88, "LHC24ar_pass3, Run 560123");
  canvGlauberFull->SaveAs(outfile);
}