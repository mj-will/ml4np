
#include "TFile.h"

void TestHists()
{
    // Save hists from test results
    // Author: Michael Williams
    
    // load .root file

    TString fname = "TMVApp.root";
    TFile *f = new TFile(fname, "READ");
    f->ls();

    // get hists
    TH1F* hBDT = (TH1F*)f->Get("MVA_BDT") ;
    TH1F* hPyKeras= (TH1F*)f->Get("MVA_PyKeras") ;

    // BDT
    //
    // canvas to fill
    TCanvas *c = new TCanvas;
    // draw hists
    hBDT->Draw();

    // make plot
    TImage *img = TImage::Create();
    img->FromPad(c);
    img->WriteImage("histBDT.png");

    delete hBDT;
    delete c;
    delete img;

    // PyKeras
    //
    // c1anvas to fill
    TCanvas *c1 = new TCanvas;
    // draw hists
    hPyKeras->Draw();

    // make plot
    TImage *img1 = TImage::Create();
    img1->FromPad(c1);
    img1->WriteImage("histPyKeras.png");

    delete hPyKeras;
    delete c1;
    delete img1;
}
