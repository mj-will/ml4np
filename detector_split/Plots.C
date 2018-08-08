
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TCanvas.h"
#include "TGraph.h"

#include "TMVA/ROCCurve.h"

struct rocPair {
    TMVA::ROCCurve rocBDT;
    TMVA::ROCCurve rocPyKeras;
};


rocPair getROC (TString det)
{
    // Load tree
    
    TFile *f = new TFile("resultsDet" + det + ".root", "READ");
    TTree *t = (TTree*)f->Get("resultsTree");

    // number of entries
    int Nentries = t->GetEntries();

    // create reader for each method
    TTreeReader reader("resultsTree", f);
    TTreeReaderValue<double> pBDT(reader, "pBDT");
    TTreeReaderValue<double> pPyKeras(reader, "pPyKeras");
    TTreeReaderValue<int> flag(reader, "eventclass");

    t->Print();

    // ROC curves
    vector<Float_t> outputs0;
    vector<Float_t> outputs1;
    vector<Bool_t> classes;
    vector<Float_t> weights;

    while (reader.Next()) {
        outputs0.push_back(*pBDT);
        outputs1.push_back(*pPyKeras);
        classes.push_back(*flag);
        weights.push_back(1.0);
    }
    TMVA::ROCCurve rocBDT {outputs0, classes, weights};
    TMVA::ROCCurve rocPyKeras {outputs1, classes, weights};
    
    rocPair roc {
        rocBDT = rocBDT,
        rocPyKeras = rocPyKeras
    };
    return roc;
}




void Plots()
{
    // script to read results.root
    //
    // Author: Michael Williams
    
    // Load tree
    //
    // set grahp styles
    //gStyle->SetOptTitle(kFALSE);
    //gStyle->SetPalette(kBlueRedYellow);
    
    TString det0 = "0";
    TString det1 = "1";

    // det 0
    rocPair roc0 = getROC(det0);
    TMVA::ROCCurve rocBDT0 = roc0.rocBDT;
    TMVA::ROCCurve rocPyKeras0 = roc0.rocPyKeras;
    
    // det 1
    rocPair roc1 = getROC(det1);
    TMVA::ROCCurve rocBDT1 = roc1.rocBDT;
    TMVA::ROCCurve rocPyKeras1 = roc1.rocPyKeras;

    // makes plots
    TCanvas *c1 = new TCanvas;
    TGraph *gr1 = rocBDT0.GetROCCurve();      gr1->SetTitle("BDT0");
    TGraph *gr2 = rocPyKeras0.GetROCCurve();  gr2->SetTitle("PyKeras0");
    TGraph *gr3 = rocBDT1.GetROCCurve();      gr3->SetTitle("BDT1");
    TGraph *gr4 = rocPyKeras1.GetROCCurve();  gr4->SetTitle("PyKeras1");

    gr1->SetLineWidth(2);
    gr1->SetLineColor(kRed);
    gr1->SetMarkerColor(kRed);

    gr2->SetLineWidth(2);
    gr2->SetLineColor(kBlue);
    gr2->SetMarkerColor(kBlue);

    gr3->SetLineWidth(2);
    gr3->SetLineColor(kGreen);
    gr3->SetMarkerColor(kGreen);

    gr4->SetLineWidth(2);
    gr4->SetLineColor(kYellow);
    gr4->SetMarkerColor(kYellow);

    gr1->Draw("");
    gr2->Draw("SAME");
    gr3->Draw("SAME");
    gr4->Draw("SAME");

    gPad->BuildLegend();

    TImage *imgROC = TImage::Create();
    imgROC->FromPad(c1);
    imgROC->WriteImage("ROC.png");

    // Histograms
    TFile *f = new TFile("resultsDet" + det0 + ".root", "READ");
    TTree *t = (TTree*)f->Get("resultsTree");

    // cuts
    TCut cut1 = "eventclass==1";
    TCut cut2 = "eventclass==0";

    // hist of background
    TCanvas *cBKG = new TCanvas;
    t->Draw("pBDT:pPyKeras",cut2,"surf2");

    TImage *imgBKG = TImage::Create();
    imgBKG->FromPad(cBKG);
    imgBKG->WriteImage("background2D.png");

    // hist of signal
    TCanvas *cSig = new TCanvas;
    t->Draw("pBDT:pPyKeras",cut1,"surf2");

    TImage *imgSig = TImage::Create();
    imgSig->FromPad(cSig);
    imgSig->WriteImage("signal2D.png");

    // hist of both
    TCanvas *c = new TCanvas;
    t->Draw("pBDT:pPyKeras","","surf2");

    TImage *img = TImage::Create();
    img->FromPad(c);
    img->WriteImage("data2D.png");
}

