   
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

    // Create reader object
    TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

    // Load data
     TFile *inputSignal(0);
     TFile *inputBackground(0);
     TString fnameSignal = "../data/rho10_tm1_sm1_stat/Signal.root";
     TString fnameBackground = "../data/rho10_tm1_sm1_stat/Background.root";
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

     // Create a ROOT output file
     TString outfileName("TMVAKeras.root");
     TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

// TODO: create loop with TTreeReader
// TODO: fix order of variables

   Float_t ElTime, ElP, ElTh, ElPhi, PTime, PP, PTh, PPhi, PimTime, PimP, PimTh, PimPhi, PipTime, PipP, PipTh, PipPhi;
   reader->AddVariable( "ElTime", &ElTime);
   reader->AddVariable( "ElP", &ElP );
   reader->AddVariable( "ElTh", &ElTh );
   reader->AddVariable( "ElPhi", &ElPhi);
   reader->AddVariable( "PTime", &PTime);
   reader->AddVariable( "PP", &PP );
   reader->AddVariable( "PTh", &PTh );
   reader->AddVariable( "PPhi", &PPhi);
   reader->AddVariable( "PipTime", &PipTime);
   reader->AddVariable( "PipP", &PipP );
   reader->AddVariable( "PipTh", &PipTh );
   reader->AddVariable( "PipPhi", &PipPhi);
   reader->AddVariable( "PimTime", &PimTime);
   reader->AddVariable( "PimP", &PimP );
   reader->AddVariable( "PimTh", &PimTh );
   reader->AddVariable( "PimPhi", &PimPhi);
<<<<<<< HEAD


   //reader->BookMVA("BDT", TString("dataset/weights/TMVAClassification_BDTB.weights.xml"));
=======

>>>>>>> 7a54c55fe8b986b8fcc5b738395815f7e060a660

   reader->BookMVA("PyKeras-0", TString("dataset/weights/TMVAClassification_PyKeras-0.weights.xml"));

  outputFile->Close();

}
