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

void TMVAClassificationCategoryApplication()
{
   // Load library
   TMVA::Tools::Instance();
   TMVA::PyMethodBase::PyInitialize();

   // ---------------------------------------------------------------
   // default MVA methods to be trained + tested
   std::map<std::string,int> Use;
   //
   Use["LikelihoodCat"] = 0;
   Use["FisherCat"]     = 0;
   Use["BDT2"]          = 1;
   Use["BDT1"]          = 0;
   Use["PyKeras2"]      = 1;
   // ---------------------------------------------------------------

   std::cout << std::endl
             << "==> Start TMVAClassificationCategoryApplication" << std::endl;

   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.
   //
   TString fname = "../data/tmva3/Data2Pi.root";
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
   //
   TTree* theTree0 = (TTree*)input->Get("HSParticles");

   gROOT->cd();
   // Choose sample type
   // also remove NaNs
    TTree* theTree = theTree0->CopyTree("Detector==0&&PipTime!=0&&PimTime!=0&&PTime!=0&&!TMath::IsNaN(PipDeltaE)&&!TMath::IsNaN(PimDeltaE)&&!TMath::IsNaN(ElDeltaE)&&!TMath::IsNaN(PDeltaE)");

   //TTree* theTree = theTree0->CopyTree("Correct==0&&Detector==0&&PipTime!=0&&PimTime!=0&&PTime!=0");
   //
   // Try manually setting branches
  
   // declare varibles
   Float_t NPerm, NDet, ElTime, ElP, ElTh, ElPhi, PTime, PP, PTh;
   Float_t PPhi, PimTime, PimP, PimTh, PimPhi, PipTime, PipP;
   Float_t PipTh, PipPhi, PDet, PipDet, PimDet;
   // ints for tree
   Int_t iNPerm, iNDet, iPDet, iPimDet, iPipDet, Correct;

   // General variables
   theTree->SetBranchAddress( "Correct", &Correct);
   theTree->SetBranchAddress( "NPerm",   &iNPerm);
   theTree->SetBranchAddress( "NDet",    &iNDet);
   // Electron
   theTree->SetBranchAddress( "ElTime",  &ElTime);
   theTree->SetBranchAddress( "ElP",     &ElP);
   theTree->SetBranchAddress( "ElTh",    &ElTh);
   theTree->SetBranchAddress( "ElPhi",   &ElPhi);
   // Proton
   theTree->SetBranchAddress( "PTime",   &PTime);
   theTree->SetBranchAddress( "PP",      &PP);
   theTree->SetBranchAddress( "PTh",     &PTh);
   theTree->SetBranchAddress( "PPhi",    &PPhi);
   theTree->SetBranchAddress( "PDet",    &iPDet);
   // Pi +
   theTree->SetBranchAddress( "PipTime", &PipTime);
   theTree->SetBranchAddress( "PipP",    &PipP);
   theTree->SetBranchAddress( "PipTh",   &PipTh);
   theTree->SetBranchAddress( "PipPhi",  &PipPhi);
   theTree->SetBranchAddress( "PipDet",  &iPipDet);
   // Pi - 
   theTree->SetBranchAddress( "PimTime", &PimTime);
   theTree->SetBranchAddress( "PimP",    &PimP);
   theTree->SetBranchAddress( "PimTh",   &PimTh);
   theTree->SetBranchAddress( "PimPhi",  &PimPhi);
   theTree->SetBranchAddress( "PimDet",  &iPimDet);

   // loop!
   //  Create the Reader object

   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

   // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
   // General variables
   reader->AddVariable( "NPerm",   &NPerm);
   reader->AddVariable( "NDet",    &NDet);
   // Electron
   reader->AddVariable( "ElTime",  &ElTime);
   reader->AddVariable( "ElP",     &ElP);
   reader->AddVariable( "ElTh",    &ElTh);
   reader->AddVariable( "ElPhi",   &ElPhi);
   // Proton
   reader->AddVariable( "PTime",   &PTime);
   reader->AddVariable( "PP",      &PP);
   reader->AddVariable( "PTh",     &PTh);
   reader->AddVariable( "PPhi",    &PPhi);
   reader->AddVariable( "PDet",    &PDet);
   // Pi +
   reader->AddVariable( "PipTime", &PipTime);
   reader->AddVariable( "PipP",    &PipP);
   reader->AddVariable( "PipTh",   &PipTh);
   reader->AddVariable( "PipPhi",  &PipPhi);
   reader->AddVariable( "PipDet",  &PipDet);
   // Pi - 
   reader->AddVariable( "PimTime", &PimTime);
   reader->AddVariable( "PimP",    &PimP);
   reader->AddVariable( "PimTh",   &PimTh);
   reader->AddVariable( "PimPhi",  &PimPhi);
   reader->AddVariable( "PimDet",  &PimDet);

   // Book the MVA methods

   // Book methods

   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      if (it->second) {
         TString methodName = it->first + " method";
         TString weightfile = "datasetTest/weights/TMVAClassification_" + TString(it->first) + ".weights.xml";
         reader->BookMVA( methodName, weightfile );
      }
   }

   // Book output histograms
   UInt_t nbin = 100;
   std::map<std::string,TH1*> hist;
   hist["LikelihoodCat"] = new TH1F( "MVA_LikelihoodCat",   "MVA_LikelihoodCat", nbin, -1, 0.9999 );
   hist["FisherCat"]     = new TH1F( "MVA_FisherCat",       "MVA_FisherCat",     nbin, -4, 4 );
   hist["BDT2"]           = new TH1F( "MVA_BDT",             "MVA_BDT"     ,      nbin, -0.8 ,0.8);
   hist["PyKeras2"]       = new TH1F( "MVA_PyKeras",         "MVA_Pykeras",       nbin, -0.1, 1.1);


   // Event loop

   std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
   TStopwatch sw;
   sw.Start();

   double pBDT;
   double pPyKeras;
   int    eventclass;

   // File for results tree
   TFile *resultsFile = TFile::Open("results.root", "RECREATE");

   // Write TTree with results
   TTree *resultsTree = new TTree("resultsTree", "Tree with results");

   resultsTree->Branch("pBDT", &pBDT);
   resultsTree->Branch("pPyKeras", &pPyKeras);
   resultsTree->Branch("class", &eventclass);

   // for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) {

   std::map<std::string,double> p;

   for (Long64_t ievt=0; ievt <theTree->GetEntries(); ievt++) {
      if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;

      theTree->GetEntry(ievt);

      std::cout<<theTree->GetEntry(ievt)<<std::endl;
      std::cout<<Correct<<std::endl;
      std::cout<<NDet<<std::endl;

      if (ievt == 1000) {
          break;
      }

      // save event class
      eventclass = Correct;

      // Return the MVA outputs and fill into histograms
      
      for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
         if (!it->second) continue;
         TString methodName = it->first + " method";
         hist[it->first]->Fill( reader->EvaluateMVA( methodName ) );
         double r;
         p[it->first] = reader->EvaluateMVA( (methodName) );
      }
      pBDT = p["BDT2"];
      pPyKeras = p["PyKeras2"];
      std::cout<<pBDT;
      resultsTree->Fill();
   }
   sw.Stop();
   std::cout << "--- End of event loop: "; sw.Print();

   // Fill tree
   resultsFile->Write();

   // Write histograms

   TFile *target  = new TFile( "TMVApp.root","RECREATE" );
   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++)
      if (it->second) hist[it->first]->Write();

   target->Close();
   std::cout << "--- Created root file: \"TMVApp.root\" containing the MVA output histograms" << std::endl;

   delete reader;
   std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
   TMVAClassificationCategoryApplication();
   return 0;
}
