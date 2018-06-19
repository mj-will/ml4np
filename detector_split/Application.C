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

void Application()
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

   TString  detector = "1";

   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.
   //
   TString fname = "../data/tmva3/Data2Pi_" + TString(detector) + ".root";
   std::cout << "--- TMVAClassificationApp    : Accessing " << fname << "!" << std::endl;
   TFile *input = TFile::Open(fname);
   if (!input) {
      std::cout << "ERROR: could not open data file: " << fname << std::endl;
      exit(1);
   }
   // Prepare the tree
   // - here the variable names have to corresponds to your tree
   // - you can use the same variables as above which is slightly faster,
   //   but of course you can use different ones and copy the values inside the event loop
   TString treeName = "D" + TString(detector) + "Tree";

   TTree* theTree = (TTree*)input->Get(treeName);


   // Try manually setting branches
   vector<TString> variableNames = {"Topo", "NPerm", "NDet"};
   vector<TString> particleNames = {"El","P","Pip","Pim"};
   vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2"};

   //// Add (particleName + particleProperty) string to variableNames in all possible combinations
   //for(auto const& pn: particleNames) {
   //    for(auto const& pp: particleProperties) {
   //        variableNames.push_back(pn+pp);
   //    };
   //};
   //for(auto const& value: variableNames) {
   //    theTree->SetBranchAddress( value, value);
   //    reader->AddVariable( value, value);
   //    
   //};

   // declare varibles
   // ints for tree
   //
   // All variables
   // General
   Int_t iTopo;
   Int_t iNPerm;
   Int_t iNDet;
   Int_t Detector;
   Int_t Correct;
   Float_t Topo;
   Float_t NPerm;
   Float_t NDet;
   // Electron
   Float_t ElTime;
   Float_t ElEdep;
   Float_t ElDeltaE;
   Float_t ElPreE;
   Float_t ElP;
   Float_t ElTh;
   Float_t ElPhi;
   Float_t ElVz;
   Float_t ElTrChi2;
   Int_t   iElDet;
   // Proton
   Float_t PTime;
   Float_t PEdep;
   Float_t PDeltaE;
   Float_t PPreE;
   Float_t PP;
   Float_t PTh;
   Float_t PPhi;
   Float_t PVz;
   Float_t PTrChi2;
   Int_t   iPDet;
   // Pi +
   Float_t PipTime;
   Float_t PipEdep;
   Float_t PipDeltaE;
   Float_t PipPreE;
   Float_t PipP;
   Float_t PipTh;
   Float_t PipPhi;
   Float_t PipVz;
   Float_t PipTrChi2;
   Int_t   iPipDet;
   // Pi -
   Float_t PimTime;
   Float_t PimEdep;
   Float_t PimDeltaE;
   Float_t PimPreE;
   Float_t PimP;
   Float_t PimTh;
   Float_t PimPhi;
   Float_t PimVz;
   Float_t PimTrChi2;
   Int_t   iPimDet;

   std::cout<<detector<<std::endl;
   //  Create the Reader object
   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

   if (detector == "0") {

       // General
       theTree->SetBranchAddress("Topo",    &iTopo);
       theTree->SetBranchAddress("NPerm",   &iNPerm);
       theTree->SetBranchAddress("NDet",    &iNDet);
       theTree->SetBranchAddress("Correct", &Correct);
       // Electron
       theTree->SetBranchAddress("ElTime",  &ElTime);
       //theTree->SetBranchAddress("ElEdep",  &ElEdep);
       theTree->SetBranchAddress("ElDeltaE",&ElDeltaE);
       //theTree->SetBranchAddress("ElPreE",  &ElPreE);
       theTree->SetBranchAddress("ElP",     &ElP);
       theTree->SetBranchAddress("ElTh",    &ElTh);
       theTree->SetBranchAddress("ElPhi",   &ElPhi);
       //theTree->SetBranchAddress("ElVz",    &ElVz);
       //theTree->SetBranchAddress("ElTrChi2",&ElTrChi2);
       //theTree->SetBranchAddress("ElDet",   &iElDet);
       // Proton 
       theTree->SetBranchAddress("PTime",  &PTime);
       theTree->SetBranchAddress("PEdep",  &PEdep);
       theTree->SetBranchAddress("PDeltaE",&PDeltaE);
       theTree->SetBranchAddress("PPreE",  &PPreE);
       theTree->SetBranchAddress("PP",     &PP);
       theTree->SetBranchAddress("PTh",    &PTh);
       theTree->SetBranchAddress("PPhi",   &PPhi);
       theTree->SetBranchAddress("PVz",    &PVz);
       theTree->SetBranchAddress("PTrChi2",&PTrChi2);
       //theTree->SetBranchAddress("PDet",   &iPDet);
       // Pi +
       theTree->SetBranchAddress("PipTime",  &PipTime);
       theTree->SetBranchAddress("PipEdep",  &PipEdep);
       theTree->SetBranchAddress("PipDeltaE",&PipDeltaE);
       theTree->SetBranchAddress("PipPreE",  &PipPreE);
       theTree->SetBranchAddress("PipP",     &PipP);
       theTree->SetBranchAddress("PipTh",    &PipTh);
       theTree->SetBranchAddress("PipPhi",   &PipPhi);
       theTree->SetBranchAddress("PipVz",    &PipVz);
       theTree->SetBranchAddress("PipTrChi2",&PipTrChi2);
       //theTree->SetBranchAddress("PipDet",   &iPipDet);
       // Pi -
       theTree->SetBranchAddress("PimTime",  &PimTime);
       theTree->SetBranchAddress("PimEdep",  &PimEdep);
       theTree->SetBranchAddress("PimDeltaE",&PimDeltaE);
       theTree->SetBranchAddress("PimPreE",  &PimPreE);
       theTree->SetBranchAddress("PimP",     &PimP);
       theTree->SetBranchAddress("PimTh",    &PimTh);
       theTree->SetBranchAddress("PimPhi",   &PipPhi);
       theTree->SetBranchAddress("PimVz",    &PipVz);
       theTree->SetBranchAddress("PimTrChi2",&PipTrChi2);
       //theTree->SetBranchAddress("PimDet",   &iPimDet);

       // loop!

       // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
       // General
       reader->AddVariable("Topo",    &Topo);
       reader->AddVariable("NPerm",   &NPerm);
       reader->AddVariable("NDet",    &NDet);
       // Electron
       reader->AddVariable("ElTime",  &ElTime);
       //reader->AddVariable("ElEdep",  &ElEdep);
       reader->AddVariable("ElDeltaE",&ElDeltaE);
       //reader->AddVariable("ElPreE",  &ElPreE);
       reader->AddVariable("ElP",     &ElP);
       reader->AddVariable("ElTh",    &ElTh);
       reader->AddVariable("ElPhi",   &ElPhi);
       //reader->AddVariable("ElVz",    &ElVz);
       //reader->AddVariable("ElTrChi2",&ElTrChi2);
       //reader->AddVariable("ElDet",   &ElDet);
       // Proton 
       reader->AddVariable("PTime",  &PTime);
       reader->AddVariable("PEdep",  &PEdep);
       reader->AddVariable("PDeltaE",&PDeltaE);
       reader->AddVariable("PPreE",  &PPreE);
       reader->AddVariable("PP",     &PP);
       reader->AddVariable("PTh",    &PTh);
       reader->AddVariable("PPhi",   &PPhi);
       reader->AddVariable("PVz",    &PVz);
       reader->AddVariable("PTrChi2",&PTrChi2);
       //reader->AddVariable("PDet",   &PDet);
       // Pi +
       reader->AddVariable("PipTime",  &PipTime);
       reader->AddVariable("PipEdep",  &PipEdep);
       reader->AddVariable("PipDeltaE",&PipDeltaE);
       reader->AddVariable("PipPreE",  &PipPreE);
       reader->AddVariable("PipP",     &PipP);
       reader->AddVariable("PipTh",    &PipTh);
       reader->AddVariable("PipPhi",   &PipPhi);
       reader->AddVariable("PipVz",    &PipVz);
       reader->AddVariable("PipTrChi2",&PipTrChi2);
       //reader->AddVariable("PipDet",   &PipDet);
       // Pi -
       reader->AddVariable("PimTime",  &PimTime);
       reader->AddVariable("PimEdep",  &PimEdep);
       reader->AddVariable("PimDeltaE",&PimDeltaE);
       reader->AddVariable("PimPreE",  &PimPreE);
       reader->AddVariable("PimP",     &PimP);
       reader->AddVariable("PimTh",    &PimTh);
       reader->AddVariable("PimPhi",   &PipPhi);
       reader->AddVariable("PimVz",    &PipVz);
       reader->AddVariable("PimTrChi2",&PipTrChi2);
       //reader->AddVariable("PimDet",   &PimDet);
   }
   if (detector == "1") {
       // General
       theTree->SetBranchAddress("Topo",    &iTopo);
       theTree->SetBranchAddress("NPerm",   &iNPerm);
       theTree->SetBranchAddress("NDet",    &iNDet);
       theTree->SetBranchAddress("Correct", &Correct);
       // Electron
       theTree->SetBranchAddress("ElTime",  &ElTime);
       theTree->SetBranchAddress("ElEdep",  &ElEdep);
       theTree->SetBranchAddress("ElDeltaE",&ElDeltaE);
       theTree->SetBranchAddress("ElPreE",  &ElPreE);
       theTree->SetBranchAddress("ElP",     &ElP);
       theTree->SetBranchAddress("ElTh",    &ElTh);
       theTree->SetBranchAddress("ElPhi",   &ElPhi);
       theTree->SetBranchAddress("ElVz",    &ElVz);
       theTree->SetBranchAddress("ElTrChi2",&ElTrChi2);
       //theTree->SetBranchAddress("ElDet",   &iElDet);
       // Proton 
       theTree->SetBranchAddress("PTime",  &PTime);
       theTree->SetBranchAddress("PEdep",  &PEdep);
       theTree->SetBranchAddress("PDeltaE",&PDeltaE);
       theTree->SetBranchAddress("PPreE",  &PPreE);
       theTree->SetBranchAddress("PP",     &PP);
       theTree->SetBranchAddress("PTh",    &PTh);
       theTree->SetBranchAddress("PPhi",   &PPhi);
       theTree->SetBranchAddress("PVz",    &PVz);
       theTree->SetBranchAddress("PTrChi2",&PTrChi2);
       //theTree->SetBranchAddress("PDet",   &iPDet);
       // Pi +
       theTree->SetBranchAddress("PipTime",  &PipTime);
       theTree->SetBranchAddress("PipEdep",  &PipEdep);
       theTree->SetBranchAddress("PipDeltaE",&PipDeltaE);
       theTree->SetBranchAddress("PipPreE",  &PipPreE);
       theTree->SetBranchAddress("PipP",     &PipP);
       theTree->SetBranchAddress("PipTh",    &PipTh);
       theTree->SetBranchAddress("PipPhi",   &PipPhi);
       theTree->SetBranchAddress("PipVz",    &PipVz);
       theTree->SetBranchAddress("PipTrChi2",&PipTrChi2);
       //theTree->SetBranchAddress("PipDet",   &iPipDet);
       // Pi -
       theTree->SetBranchAddress("PimTime",  &PimTime);
       theTree->SetBranchAddress("PimEdep",  &PimEdep);
       theTree->SetBranchAddress("PimDeltaE",&PimDeltaE);
       theTree->SetBranchAddress("PimPreE",  &PimPreE);
       theTree->SetBranchAddress("PimP",     &PimP);
       theTree->SetBranchAddress("PimTh",    &PimTh);
       theTree->SetBranchAddress("PimPhi",   &PipPhi);
       theTree->SetBranchAddress("PimVz",    &PipVz);
       theTree->SetBranchAddress("PimTrChi2",&PipTrChi2);
       //theTree->SetBranchAddress("PimDet",   &iPimDet);

       // loop!

       // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
       // General
       reader->AddVariable("Topo",    &Topo);
       reader->AddVariable("NPerm",   &NPerm);
       reader->AddVariable("NDet",    &NDet);
       // Electron
       reader->AddVariable("ElTime",  &ElTime);
       reader->AddVariable("ElEdep",  &ElEdep);
       reader->AddVariable("ElDeltaE",&ElDeltaE);
       reader->AddVariable("ElPreE",  &ElPreE);
       reader->AddVariable("ElP",     &ElP);
       reader->AddVariable("ElTh",    &ElTh);
       reader->AddVariable("ElPhi",   &ElPhi);
       reader->AddVariable("ElVz",    &ElVz);
       reader->AddVariable("ElTrChi2",&ElTrChi2);
       //reader->AddVariable("ElDet",   &ElDet);
       // Proton 
       reader->AddVariable("PTime",  &PTime);
       reader->AddVariable("PEdep",  &PEdep);
       reader->AddVariable("PDeltaE",&PDeltaE);
       reader->AddVariable("PPreE",  &PPreE);
       reader->AddVariable("PP",     &PP);
       reader->AddVariable("PTh",    &PTh);
       reader->AddVariable("PPhi",   &PPhi);
       reader->AddVariable("PVz",    &PVz);
       reader->AddVariable("PTrChi2",&PTrChi2);
       //reader->AddVariable("PDet",   &PDet);
       // Pi +
       reader->AddVariable("PipTime",  &PipTime);
       reader->AddVariable("PipEdep",  &PipEdep);
       reader->AddVariable("PipDeltaE",&PipDeltaE);
       reader->AddVariable("PipPreE",  &PipPreE);
       reader->AddVariable("PipP",     &PipP);
       reader->AddVariable("PipTh",    &PipTh);
       reader->AddVariable("PipPhi",   &PipPhi);
       reader->AddVariable("PipVz",    &PipVz);
       reader->AddVariable("PipTrChi2",&PipTrChi2);
       //reader->AddVariable("PipDet",   &PipDet);
       // Pi -
       reader->AddVariable("PimTime",  &PimTime);
       reader->AddVariable("PimEdep",  &PimEdep);
       reader->AddVariable("PimDeltaE",&PimDeltaE);
       reader->AddVariable("PimPreE",  &PimPreE);
       reader->AddVariable("PimP",     &PimP);
       reader->AddVariable("PimTh",    &PimTh);
       reader->AddVariable("PimPhi",   &PipPhi);
       reader->AddVariable("PimVz",    &PipVz);
       reader->AddVariable("PimTrChi2",&PipTrChi2);
       //reader->AddVariable("PimDet",   &PimDet);
   }
   // Book the MVA methods

   // Book methods

   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      if (it->second) {
         TString methodName = it->first + " method";
         TString weightfile = "datasetDet"+TString(detector)+"/weights/TMVAClassification_" + TString(it->first) + ".weights.xml";
         reader->BookMVA( methodName, weightfile );
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
   TFile *resultsFile = TFile::Open("resultsDet" + TString(detector)+ ".root", "UPDATE");

   // Write TTree with results
   TTree *resultsTree = new TTree("resultsTree", "Tree with results");

   resultsTree->Branch("pBDT", &pBDT);
   resultsTree->Branch("pPyKeras", &pPyKeras);
   resultsTree->Branch("class", &eventclass);

   // for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) {

   std::map<std::string,double> p;

   for (Long64_t ievt=0; ievt <= 10000; ievt++) {
      if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;

      theTree->GetEntry(ievt);

      // save event class
      eventclass = Correct;

      // Return the MVA outputs and fill into histograms
      
      for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
         if (!it->second) continue;
         TString methodName = it->first + " method";
         p[it->first] = reader->EvaluateMVA( (methodName) );
         //std::cout<<it->first<<std::endl;
      }
      if (Use["PyKeras"]) {
          pPyKeras = p["PyKeras"];
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


   delete reader;
   std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
   Application();
   return 0;
}
