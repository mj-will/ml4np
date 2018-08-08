#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include<utility>

#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH2.h"

using std::cout;
using std::endl;
using std::to_string;

void Hists()
{
    gROOT->SetBatch(kTRUE);
    // input file
    Int_t Topology = 2;
    TString fname = "Topo" + std::to_string(Topology)  + ".root";
    cout<<"Loading "<<fname<<"..."<<endl;

    TFile *input = TFile::Open(fname);
    if (!input) {
         cout << "ERROR: could not open data file: " << fname <<  endl;
         exit(1);
    }
    
    // Get tree
    TString treeName = "datasetTopo" + std::to_string(Topology) + "/TestTree";
    TTree *t = (TTree*)input->Get(treeName);

    // pairs to plot
    std::vector<std::vector<TString>> variableNames = {{"P", "Time"}};
    std::vector<TString> particleNames;
    if (Topology == 0) {particleNames = {"El", "P", "Pip", "Pim"};}
    if (Topology == 1) {particleNames = {"El", "P", "Pip"};}
    if (Topology == 2) {particleNames = {"El", "P", "Pim"};}
    if (Topology == 3) {particleNames = {"El", "Pip", "Pim"};}

    // map to set limits
    std::map<TString, std::pair<Float_t, Float_t>> limits;
    limits["P"] =  make_pair(0., 10.);
    limits["Time"] = make_pair(-10., 10.);

    // file to save too
    TString targetName = "HistogramsTopo" + std::to_string(Topology) + ".root";
    TFile *target = new TFile( targetName,"RECREATE" );

    Int_t nbins = 100;
    // cuts
    TCut cs = "BDT>0";
    TCut cb = "BDT<=0";

    for (auto const& pn : particleNames ) {
        for (auto const& vn : variableNames) {
            cout<<"Making histograms for "<< pn + vn[0] + "Vs" + pn + vn[1] << "..." << endl;
            // bins and limits for x and y axis
            TString px = to_string(nbins) + "," + to_string(limits[vn[0]].first) + "," + to_string(limits[vn[0]].second);
            TString py = to_string(nbins) + "," + to_string(limits[vn[1]].first) + "," + to_string(limits[vn[1]].second);
            // draw histograms
            t->Draw( pn+vn[0]+":"+ pn+vn[1]+">>hs(" +  px + "," + py + ")", cs, "COL:GOFF" );
            t->Draw( pn+vn[0]+":"+ pn+vn[1]+">>hb(" +  px + "," + py + ")", cb, "COL:GOFF" );

            // get histograms
            TH2F *hs = (TH2F*)gDirectory->Get("hs");
            TH2F *hb = (TH2F*)gDirectory->Get("hb");

            // set axis labels and titles
            hs->SetName(pn + vn[0] + "Vs" + pn + vn[1] + "-s");
            hb->SetName(pn + vn[0] + "Vs" + pn + vn[1] + "-b");
            hs->GetXaxis()->SetTitle(pn + vn[0]); hs->GetYaxis()->SetTitle(pn + vn[1]);
            hb->GetXaxis()->SetTitle(pn + vn[0]); hb->GetYaxis()->SetTitle(pn + vn[1]);
            // set options
            hs->SetOption("COL");
            hb->SetOption("COL");
            // redraw to apply options before writing
            TCanvas *tmpC = new TCanvas;
            hs->Draw("A:goff");
            hb->Draw("A:goff");
            hs->Write();
            hb->Write();
        }
    }
    target->Close();
}
