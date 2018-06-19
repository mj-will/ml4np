
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

void Hists()
{
    // script to read results.root
    //
    // Author: Michael Williams
    
    // Load tree
    
    TFile *f = new TFile("results.root", "READ");
    TTree *t = (TTree*)f->Get("resultsTree");

    TTreeReader reader("resultsTree", f);
    TTreeReaderValue<double> pBDT(reader, "pBDT");
    TTreeReaderValue<double> pPyKeras(reader, "pPyKeras");
    TTreeReaderValue<int> flag(reader, "class");

    t->Print();
    std::cout.precision(16);
    while (reader.Next())
    {
        //std::cout<<*p<<std::endl;
    }



    // cuts
    TCut c1 = "class==1";
    TCut c2 = "class==0";

    // hist of background
    TCanvas *cBKG = new TCanvas;
    t->Draw("pBDT:pPyKeras",c2,"surf2");

    TImage *imgBKG = TImage::Create();
    imgBKG->FromPad(cBKG);
    imgBKG->WriteImage("background2D.png");

    // hist of signal
    TCanvas *cSig = new TCanvas;
    t->Draw("pBDT:pPyKeras",c1,"surf2");

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
