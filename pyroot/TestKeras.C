   
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
#include "TStopwatch.h"
 
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

using namespace TMVA;

void TestKeras()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    // methods to use
    std::map<std::string,int> Methods;
    Methods["BDT"]                = 1;
    Methods["PyKeras"]            = 1;

    // Create reader object
    TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

    // Load data
     TFile *inputSignal(0);
     TFile *inputBackground(0);
     TString fnameSignal = "../data/tmva3/Signal2Pi.root";
     TString fnameBackground = "../data/tmva3/BG2Pi.root";
     if (!gSystem->AccessPathName( fnameSignal )) {
         inputSignal = TFile::Open( fnameSignal ); // check if file in local directory exists
     }
     else {
          printf("Signal file not found");
                                                       }
                                      
     if (!gSystem->AccessPathName( fnameBackground)) {
          inputBackground= TFile::Open( fnameBackground); // check if file in local directory exists
     }
     else {
          printf("Background file not found");
     }
     if ((!inputSignal) || (!inputBackground)) {
        std::cout << "ERROR: could not open data file" << std::endl;
        exit(1);
     }
    
     // Register the training and test trees
     
     TTree *signalTree     = (TTree*)inputSignal->Get("HSParticles");
     TTree *background     = (TTree*)inputBackground->Get("HSParticles");


// TODO: create loop with TTreeReader
// TODO: fix order of variables
   

   Float_t NPerm, NDet, ElTime, ElP, ElTh, ElPhi, PTime, PP, PTh, PPhi, PimTime, PimP, PimTh, PimPhi, PipTime, PipP, PipTh, PipPhi, PDet, PipDet, PimDet;

   reader->AddVariable( "NPerm", &NPerm);
   reader->AddVariable( "NDet", &NDet);
   reader->AddVariable( "ElTime", &ElTime);
   reader->AddVariable( "ElP", &ElP );
   reader->AddVariable( "ElTh", &ElTh );
   reader->AddVariable( "ElPhi", &ElPhi);
   reader->AddVariable( "PTime", &PTime);
   reader->AddVariable( "PP", &PP );
   reader->AddVariable( "PTh", &PTh );
   reader->AddVariable( "PPhi", &PPhi);
   reader->AddVariable( "PDet", &PDet);
   reader->AddVariable( "PipTime", &PipTime);
   reader->AddVariable( "PipP", &PipP );
   reader->AddVariable( "PipTh", &PipTh );
   reader->AddVariable( "PipPhi", &PipPhi);
   reader->AddVariable( "PipDet", &PipDet);
   reader->AddVariable( "PimTime", &PimTime);
   reader->AddVariable( "PimP", &PimP );
   reader->AddVariable( "PimTh", &PimTh );
   reader->AddVariable( "PimPhi", &PimPhi);
   reader->AddVariable( "PimDet", &PimDet);

   // Book methods

   reader->BookMVA("BDT", TString("datasetBkg0/weights/TMVAMultiBkg0_BDT-Bkg0.weights.xml"));

   reader->BookMVA("PyKeras-Bkg0", TString("datasetBkg0/weights/TMVAMultiBkg0_PyKeras-Bkg0.weights.xml "));

    // Book output hists
    UInt_t nbin = 100;
    std::map<std::string,TH1*> hist;

    if (Methods["BDT"])      hist["BDT"]     = new TH1F( "MVA_BDT",   "MVA_BDT"     , nbin, -0.8 ,0.8);

    // Return MVA outputs and fill hsits
    if (Methods["BDT"])      histBd     ->Fill( reader->EvaluateMVA("BDT"));

    // Write histograms
    TString outfileName("TMVAKeras.root");
    TFile *outputFile  = new TFile( outfileName, "RECREATE" );

    if (Methods["BDT"])      histBdt      ->Write();

    outputFile->Close();



}
