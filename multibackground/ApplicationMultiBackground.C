/// \author Michael Williams


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

void ApplicationMultiBackground()
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

    // Prepare input tree
    TString fname = "../data/tmva3/Data_clean.root";
    std::cout << "--- TMVAClassificationApp    : Accessing " << fname << "!" << std::endl;
    TFile *input = TFile::Open(fname);
    if (!input) {
       std::cout << "ERROR: could not open data file: " << fname << std::endl;
       exit(1);
    }

    // make the reader

    TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent");

    Int_t NPerm, NDet, Detector, Topo;
    Float_t fNPerm, fNDet, fDetector;
    Long64_t Correct;

    vector<TString> variableNames = {};
    vector<TString> particleNames = {"El", "P","Pip","Pim"};
    vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };
    // add remaining variables
    reader->AddVariable("NDet",    &fNDet);
    reader->AddVariable("NPerm",   &fNPerm);
    reader->AddVariable("Detector",&fDetector);


    int nVars = variableNames.size();
    std::vector<Double_t> vars( nVars);
    Float_t treevars[nVars];
// add most of the variables
    for (UInt_t ivar = 0; ivar<nVars; ivar++) {
        reader->AddVariable( variableNames[ivar], &(treevars[ivar]));
            }
    Float_t UID;
    reader->AddSpectator("UID", &UID);

    // Prepare the tree;
    TTree* theTree = (TTree*)input->Get("HSParticles");

    
    theTree->SetBranchAddress("Topo",    &Topo);
    theTree->SetBranchAddress("NPerm",   &NPerm);
    theTree->SetBranchAddress("NDet",    &NDet);
    theTree->SetBranchAddress("Correct", &Correct);
    theTree->SetBranchAddress("Detector",&Detector);
    // Set branches in loop

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };


    for(UInt_t ivar = 0; ivar<nVars; ivar++) {
        theTree->SetBranchAddress( variableNames[ivar], &(treevars[ivar]));
    }  

    theTree->Print();
   // Book the MVA methods

   // Book methods

   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      if (it->second) {
         TString methodName = it->first + " method";
         TString weightfile = "dataset/weights/TMVAMulticlass_" + TString(it->first) + ".weights.xml";
         reader->BookMVA( methodName, weightfile );
      }
   }

   // Event loop

   std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
   TStopwatch sw;
   sw.Start();

   // File for results tree
   TFile *resultsFile = TFile::Open("resultsMB.root", "RECREATE");

   // Write TTree with results
   TTree *resultsTree = new TTree("resultsTree", "Tree with results");

   // integer used to check class
   Int_t                classID;
   resultsTree->Branch("classID", &classID);

   TString              className;
   //resultsTree->Branch("className", &className);
   std::vector<TString> classNames =  {"Signals", "CBackground", "KBackground"}; // {0, 1, 2}
   std::vector<TString> branchNames = {};
   // vector to correct for ordering of data
   std::vector<Int_t> classCor = {1, 0, 2};

   // get vector of branches to add
   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
       if (!it->second) continue;
       for (auto const& cn : classNames){
           TString branchName = it->first + "_" + cn;
           branchNames.push_back(branchName);
       }
   }

   // add branches
   int nRes = branchNames.size();
   std::vector<Float_t> v( nRes);
   Float_t resultsTreeVars[nRes];

   for(UInt_t ivar = 0; ivar<nRes; ivar++) {
       resultsTree->Branch( branchNames[ivar], &(resultsTreeVars[ivar]));
    }  

   // vector map to contain results for each method
   std::map<std::string,std::vector<Float_t>> p;

   for (Long64_t ievt=0; ievt <theTree->GetEntries(); ievt++) {
      if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;

      theTree->GetEntry(ievt);

      if (ievt == 1000) {
          break;
      }

      // save event class
      classID = classCor.at(Correct);
      className = classNames.at(classID);

      // Return the MVA outputs 
      // loop over methods 
      for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
         if (!it->second) continue;

         TString methodName = it->first + " method";
         p[it->first] = reader->EvaluateMulticlass( (methodName) );

         // iterate over classes  and add the results for each class to the tree
         // each method will have as many entries as classes
         // so loop over method number and add number of class
         for (UInt_t inc=0; inc<classNames.size(); inc++) {
             resultsTreeVars[((std::distance(Use.begin(), it)) * classNames.size() + inc)] = p[it->first][inc];
         }
      }
      
      resultsTree->Fill();
   }
   sw.Stop();
   std::cout << "--- End of event loop: "; sw.Print();

   // Fill tree
   resultsFile->Write();
   resultsFile->Close();


   std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
   ApplicationMultiBackground();
   return 0;
}
