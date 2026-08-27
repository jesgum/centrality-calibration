#include <map>
#include <vector>
#include <string>
#include <iostream>

void drawAnchorPointStability()
{
  std::map<int, int> runToFill = {
    { 572106, 11787 },
    { 572150, 11791 },
    { 572183, 11793 },
    { 572184, 11793 },
    { 572196, 11794 },
    { 572216, 11795 },
    { 572227, 11796 },
    { 572244, 11798 },
    { 572245, 11798 },
    { 572246, 11798 },
    { 572266, 11803 },
    { 572268, 11803 },
    { 572279, 11806 },
    { 572280, 11806 },
    { 572292, 11807 },
    { 572303, 11808 },
    { 572304, 11808 },
    { 572306, 11808 },
    { 572307, 11808 },
    { 572311, 11808 },
    { 572323, 11809 },
    { 572324, 11809 },
    { 572337, 11810 },
    { 572338, 11810 },
    { 572339, 11810 },
    { 572340, 11810 },
    { 572353, 11811 },
    { 572354, 11811 },
    { 572355, 11811 },
    { 572369, 11813 },
    { 572384, 11817 },
    { 572397, 11819 },
    { 572408, 11820 },
    { 572422, 11822 },
    { 572438, 11823 },
    { 572449, 11824 },
    { 572467, 11828 },
    { 572469, 11828 },
    { 572485, 11829 },
    { 572486, 11829 },
    { 572487, 11829 },
    { 572488, 11829 },
    { 572489, 11829 },
    { 572501, 11830 },
    { 572502, 11830 },
    { 572504, 11830 },
    { 572515, 11832 },
    { 572527, 11833 },
    { 572541, 11836 },
    { 572542, 11836 },
    { 572543, 11836 },
    { 572556, 11837 },
    { 572557, 11837 },
    { 572559, 11837 },
    { 572570, 11839 },
    { 572581, 11840 },
    { 572593, 11841 },
    { 572609, 11844 },
    { 572622, 11846 },
    { 572635, 11849 },
    { 572646, 11851 },
    { 572647, 11851 },
    { 572658, 11852 },
    { 572669, 11853 },
    { 572693, 11860 },
    { 572704, 11861 },
    { 572715, 11863 }
  };

  std::vector<int> fillOrder;
  {
    std::set<int> seen;
    for (auto& kv : runToFill) {
      if (seen.insert(kv.second).second) {
        fillOrder.push_back(kv.second);
      }
    }
  }

  std::vector<int> palette = {
    kRed + 1, kBlue + 1, kGreen + 2, kOrange + 1, kMagenta + 1,
    kCyan + 2, kYellow + 2, kViolet + 1, kTeal + 2, kPink + 1,
    kSpring + 5, kAzure + 2, kOrange - 3, kRed - 7, kBlue - 7
  };

  std::map<int, int> fillColor;
  for (size_t i = 0; i < fillOrder.size(); ++i) {
    fillColor[fillOrder[i]] = palette[i % palette.size()];
  }

  const std::string plotDir = "../results/";
  const std::string suffix = "_calibration_ancestorMode2_fixedK_fixedMu_hFT0C_BCs.root";

  std::map<int, std::vector<std::pair<double, double>>> fillPoints;
  std::vector<int> runList, fillList;
  std::vector<double> anchorValues;

  int idx = 0;
  for (auto& kv : runToFill) {
    int run = kv.first;
    int fill = kv.second;

    std::string fname = plotDir + "AR_" + std::to_string(run) + suffix;
    TFile* f = TFile::Open(fname.c_str(), "READ");
    if (!f || f->IsZombie()) {
      std::cout << "WARNING: file not found for run " << run << ", skipping." << std::endl;
      if (f)
        delete f;
      continue;
    }

    TH1* h = (TH1*)f->Get("hAnchorPoint");
    if (!h) {
      std::cout << "WARNING: hAnchorPoint not found in run " << run << ", skipping." << std::endl;
      f->Close();
      delete f;
      continue;
    }

    double val = h->GetBinContent(1);
    double err = h->GetBinError(1);

    runList.push_back(run);
    anchorValues.push_back(val);
    fillList.push_back(fill);
    fillPoints[fill].push_back({ (double)idx, val });

    f->Close();
    delete f;
    idx++;
  }

  int nPoints = runList.size();
  if (nPoints == 0) {
    std::cerr << "ERROR: no data points found. Check the plots/ directory." << std::endl;
    return;
  }

  TCanvas* c = new TCanvas("c_anchor", "Anchor Point vs Run", 1600, 800);
  c->SetTicks(1, 1);
  c->SetLeftMargin(0.07);
  c->SetRightMargin(0.015);
  c->SetBottomMargin(0.15);
  c->SetTopMargin(0.18);
  c->SetGridy();

  double ymin = 220;
  double ymax = 270;
  // double ymin = *std::min_element(anchorValues.begin(), anchorValues.end());
  // double ymax = *std::max_element(anchorValues.begin(), anchorValues.end());
  double ypad = (ymax - ymin) * 0.15;
  if (ypad == 0)
    ypad = 1.0;

  TH1F* hFrame = new TH1F("hFrame", "", nPoints, -0.5, nPoints - 0.5);
  hFrame->SetStats(0);
  hFrame->GetYaxis()->SetTitle("Anchor Point");
  hFrame->GetXaxis()->SetTitle("Run Number");
  hFrame->GetXaxis()->SetTitleOffset(2.05);
  hFrame->GetXaxis()->SetLabelSize(0.035);
  hFrame->GetXaxis()->SetLabelOffset(0.01);
  hFrame->GetYaxis()->SetTitleOffset(0.9);
  hFrame->SetMinimum(ymin - ypad);
  hFrame->SetMaximum(ymax + ypad);

  for (int i = 0; i < nPoints; ++i) {
    hFrame->GetXaxis()->SetBinLabel(i + 1, std::to_string(runList[i]).c_str());
  }
  hFrame->GetXaxis()->LabelsOption("v");

  hFrame->Draw("AXIS");

  TLegend* leg = new TLegend(0.07, 0.83, 0.985, 0.97);
  leg->SetNColumns(fillOrder.size() > 10 ? 10 : fillOrder.size());
  leg->SetBorderSize(1);
  leg->SetTextSize(0.030);

  // Draw fill background bands, each in its own legend color
  {
    int start = 0;
    for (int i = 1; i <= nPoints; ++i) {
      bool newFill = (i == nPoints) || (fillList[i] != fillList[i - 1]);
      if (newFill) {
        int fill = fillList[i - 1];
        TBox* box = new TBox(start - 0.5, ymin - ypad, i - 0.5, ymax + ypad);
        box->SetFillColorAlpha(fillColor[fill], 0.2);
        box->SetLineWidth(0);
        box->Draw("same");
        start = i;
        leg->AddEntry(box, Form("Fill %d", fill), "f");
      }
    }
  }

  std::map<int, TGraph*> fillGraphs;
  for (auto& [fill, pts] : fillPoints) {
    int n = pts.size();
    TGraph* g = new TGraph(n);
    for (int i = 0; i < n; ++i) {
      g->SetPoint(i, pts[i].first, pts[i].second);
    }
    int col = fillColor[fill];
    g->SetMarkerColor(kBlack);
    // g->SetMarkerColor(col);
    g->SetLineColor(kBlack);
    // g->SetLineColor(col);
    g->SetMarkerStyle(20);
    g->SetMarkerSize(1.0);
    g->Draw("P same");
    fillGraphs[fill] = g;
    // leg->AddEntry(g, Form("Fill %d", fill), "p");
  }

  hFrame->Draw("AXIS same");

  leg->Draw();
  c->Update();
  c->SaveAs("anchor_point_vs_run.pdf");
  c->SaveAs("anchor_point_vs_run.png");

  std::cout << "\nDone! Plots saved to anchor_point_vs_run.pdf/.png" << std::endl;
}