/// \file
/// \ingroup tutorial_tmva
/// \notebook -nodraw
/// This macro provides a simple example on how to use the trained classifiers
/// (with categories) within an analysis module
/// - Project   : TMVA - a Root-integrated toolkit for multivariate data analysis
/// - Package   : TMVA
/// - Exectuable: TMVAClassificationCategoryApplication
///
/// \macro_output
/// \macro_code
/// \author Andreas Hoecker


#include <cstdlib>
#include <vector>
#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TStopwatch.h"
#include "TMath.h"

#if not defined(__CINT__) || defined(__MAKECINT__)
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#endif

// two types of category methods are implemented
Bool_t UseOffsetMethod = kTRUE;

void SplitApplication()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    // ---------------------------------------------------------------
    // default MVA methods to be trained + tested
    std::map<std::string,int> Use;
    //
    Use["PyKeras"]      = 1;
    Use["BDT"]          = 1;
    // ---------------------------------------------------------------

    std::cout << std::endl
              << "==> Start TMVAClassificationCategoryApplication" << std::endl;

    TString  detector = "ALL";

    // Prepare input tree
    TString fname = "../data/tmva3/Data2Pi_" + TString(detector) + ".root";
    std::cout << "--- TMVAClassificationApp    : Accessing " << fname << "!" << std::endl;
    TFile *input = TFile::Open(fname);
    if (!input) {
       std::cout << "ERROR: could not open data file: " << fname << std::endl;
       exit(1);
    }
    // Prepare the tree
    TString treeName = "D" + TString(detector) + "Tree";
    TTree* theTree = (TTree*)input->Get(treeName);

    // Set branches in loop
    vector<TString> variableNames = {};
    vector<TString> particleNames = {"P","Pip","Pim"};
    vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2"};

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };

    // Fill the tree

    int nVars = variableNames.size();
    std::vector<Double_t> vars( nVars);
    Float_t treevars[nVars];

    for(UInt_t ivar = 0; ivar<nVars; ivar++) {
        theTree->SetBranchAddress( variableNames[ivar], &(treevars[ivar]));
    }  

    Int_t iTopo, iNPerm, iNDet, Correct;
    Float_t Topo, NPerm, NDet;
    Float_t ElTime, ElDeltaE, ElP, ElTh, ElPhi;
    
    // set electron manually
    theTree->SetBranchAddress("Topo",    &iTopo);
    theTree->SetBranchAddress("NPerm",   &iNPerm);
    theTree->SetBranchAddress("NDet",    &iNDet);
    theTree->SetBranchAddress("Correct", &Correct);
    //
    theTree->SetBranchAddress("ElTime",  &ElTime);
    theTree->SetBranchAddress("ElDeltaE",&ElDeltaE);
    theTree->SetBranchAddress("ElP",     &ElP);
    theTree->SetBranchAddress("ElTh",    &ElTh);
    theTree->SetBranchAddress("ElPhi",   &ElPhi);

    // make readers
    
    std::map<TString,TMVA::Reader *> readers;
    std::vector<TString> detectors = {"0", "1", "ALL"};

    // loop over detectors and make a reader for each
    for (auto const& det: detectors) {
        TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );
        readers[det] = reader;
        // add most of the variables
        for (UInt_t ivar = 0; ivar<nVars; ivar++) {
            readers[det]->AddVariable( variableNames[ivar], &(treevars[ivar]));
        }
        // add remaining variables
        readers[det]->AddVariable("Topo",    &Topo);
        readers[det]->AddVariable("NPerm",   &NPerm);
        readers[det]->AddVariable("NDet",    &NDet);
        // Electron
        readers[det]->AddVariable("ElTime",  &ElTime);
        readers[det]->AddVariable("ElDeltaE",&ElDeltaE);
        readers[det]->AddVariable("ElP",     &ElP);
        readers[det]->AddVariable("ElTh",    &ElTh);
        readers[det]->AddVariable("ElPhi",   &ElPhi);
    }

    // Book the MVA methods
    // Book all methods on each reader
    for (auto const& det: detectors) {
        for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
            if (it->second) {
                TString methodName = it->first + " method";
                TString weightfile = "datasetDet"+TString(det)+"/weights/TMVAClassification_" + TString(it->first) + ".weights.xml";
                readers[det]->BookMVA( methodName, weightfile );
            }
        }
    }


    // Event loop

    std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
    TStopwatch sw;
    sw.Start();

    double pBDT;
    double pPyKeras;
    int    eventclass;

    // File for results tree
    TFile *resultsFile = TFile::Open("resultsDet" + TString(detector)+ ".root", "RECREATE");

    // Write TTree with results
    TTree *resultsTree = new TTree("resultsTree", "Tree with results");

    resultsTree->Branch("pBDT", &pBDT);
    resultsTree->Branch("pPyKeras", &pPyKeras);
    resultsTree->Branch("eventclass", &eventclass);

    // for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) {

    std::map<std::string,double> p;

    for (Long64_t ievt=0; ievt <= 10000; ievt++) {
       if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;

       theTree->GetEntry(ievt);

       // save event class
       eventclass = Correct;

       // Return the MVA outputs and fill into histograms
       if (Detector == 0) {
           TString thisDet "0";
       }
       if (Detector == 1) {
           TString thisDet "1";
       }
    
       for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
          if (!it->second) continue;
          TString methodName = it->first + " method";
          p[it->first] = readers[thisDet]->EvaluateMVA( (methodName) );
          //std::cout<<it->first<<std::endl;
       }
       if (Use["PyKeras"]) {
           pPyKeras = p["PyKeras"];
           std::cout<<pPyKeras<<std::endl;
       }
       if (Use["BDT"]) {
           pBDT = p["BDT"];
       }
       resultsTree->Fill();
    }
    sw.Stop();
    std::cout << "--- End of event loop: "; sw.Print();

    // Fill tree
    resultsFile->Write();
    resultsFile->Close();


    delete readers;
    std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
    SplitApplication();
    return 0;
}
