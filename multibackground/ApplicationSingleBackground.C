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

void ApplicationSingleBackground()
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

    std::cout<<"--- Loading data..."<<std::endl;

    // load data
    //
    TFile *inputAll(0);
    TString fnameAll = "./tmpTree.root";
    inputAll = TFile::Open( fnameAll );

    TTree *theTree;

    if (!inputAll) {

        std::cout<<"    Did not find tree with combined backgrounds..."<<std::endl;
        std::cout<<"    Writing combined tree... "<<std::endl;

        TFile *input(0);
        TString fname = "../data/tmva3/Data2Pi_clean.root";
        input = TFile::Open( fname );
        if (!input) {
           std::cout << "ERROR: could not open data file" << std::endl;
           exit(1);
        }

        std::cout<<"--- ..."<<std::endl;

        TFile *inputBkg(0);
        TString fnameBkg = "../data/tmva3/Data2K_clean.root";
        inputBkg = TFile::Open( fnameBkg );
        if (!inputBkg) {
           std::cout << "ERROR: could not open data file" << std::endl;
           exit(1);
        }
        std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;
        std::cout << "--- TMVAClassification       : Using input background file: " << inputBkg->GetName() << std::endl;

        std::cout<<"--- Get ROOT Trees..."<<std::endl;

        // get input tree
        TString treeName = "HSParticles";
        TTree *inputTree = (TTree*)input->Get(treeName);
        gROOT->cd();
        TTree *signalTree = inputTree->CopyTree("Correct==1");
        gROOT->cd();
        TTree *backgroundTree = inputTree->CopyTree("Correct==0");

        // load second background
        TTree *backgroundTreeK = (TTree*)inputBkg->Get(treeName);

        TList *list = new TList;
        list->Add(inputTree);
        //list->Add(signalTree);
        //list->Add(backgroundTree);
        list->Add(backgroundTreeK);

        TFile *tmp = TFile::Open("tmpTree.root", "RECREATE");
        theTree = TTree::MergeTrees(list); theTree->SetName("HSParticles"); theTree->Write();

    }

    else {
        std::cout<<"    Found combined tree..."<<std::endl;
        TString treeName = "HSParticles";
        theTree = (TTree*)inputAll->Get(treeName);

    }

    Int_t NPerm, NDet, Correct, Detector, Topo;
    Float_t fNPerm, fNDet, fDetector;
    
    theTree->SetBranchAddress("Topo",    &Topo);
    theTree->SetBranchAddress("NPerm",   &NPerm);
    theTree->SetBranchAddress("NDet",    &NDet);
    theTree->SetBranchAddress("Correct", &Correct);
    theTree->SetBranchAddress("Detector",&Detector);
    // Set branches in loop
    vector<TString> variableNames = {};
    vector<TString> particleNames = {"El", "P","Pip","Pim"};
    vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };

    // Fill the tree

    int nVars = variableNames.size();
    std::vector<Float_t> vars( nVars);
    Float_t treevars[nVars];

    for(UInt_t ivar = 0; ivar<nVars; ivar++) {
        theTree->SetBranchAddress( variableNames[ivar], &(treevars[ivar]));
    }  


    theTree->Print();

    // make reader
    
    TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

    reader->AddVariable("NDet",    &fNDet);
    reader->AddVariable("NPerm",   &fNPerm);
    reader->AddVariable("Detector",&fDetector);

    // add most of the variables
    for (UInt_t ivar = 0; ivar<nVars; ivar++) {
        reader->AddVariable( variableNames[ivar], &(treevars[ivar]));
    }

    for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
        if (it->second) {
            TString methodName = it->first + " method";
            TString weightfile = "datasetComb/weights/TMVACombinedBackground_" + TString(it->first) + ".weights.xml";
            reader->BookMVA( methodName, weightfile );
        }
    }
    

    // Event loop
    //

    // File for results tree
    TFile *resultsFile = TFile::Open("resultsSingleBackground.root", "RECREATE");
 
    // Write TTree with results
    TTree *resultsTree = new TTree("resultsTree", "Tree with results");
 
    // integer used to check class
    Int_t                classID;
    resultsTree->Branch("classID", &classID);
    Int_t                originalID;
    resultsTree->Branch("originalID", &originalID);
 
    TString         className;
    //resultsTree->Branch("className", &className, "className/C");
    std::vector<TString> classNames =  {"Background", "Signal", }; // {0, 1}
    std::vector<TString> branchNames = {};
    // vector to correct for ordering of data
    std::vector<Int_t> classCor = {0, 1, 0};
 
    // get vector of branches to add
    for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
        if (!it->second) continue;
        branchNames.push_back(it->first);
    }
 
    // add branches
    int nRes = branchNames.size();
    std::vector<Float_t> v( nRes);
    Float_t resultsTreeVars[nRes];
 
    for(UInt_t ivar = 0; ivar<nRes; ivar++) {
        resultsTree->Branch( branchNames[ivar], &(resultsTreeVars[ivar]));
     }  
 
    // vector map to contain results for each method
    std::map<std::string,Float_t> p;
    // std::map<std::string, std::vector<Float_t>> p; \\ for multi-class
 
    std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
    TStopwatch sw;
    sw.Start();

    for (Long64_t ievt=0; ievt <theTree->GetEntries(); ievt++) {
       if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;
 
       theTree->GetEntry(ievt);
 
       if (ievt == 100000) {
           break;
       }
 
       // save event class
       classID = classCor.at(Correct);
       originalID = Correct;
       //if (classID == 1) className = "Signal";
       //if (classID == 0) className = "Background";
       // className = classNames.at(classID);
 
       // Return the MVA outputs 
       // loop over methods 
       for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
          if (!it->second) continue;
 
          TString methodName = it->first + " method";
          p[it->first] = reader->EvaluateMVA( (methodName) );
 
          // iterate over classes  and add the results for each class to the tree
          // each method will have as many entries as classes
          // so loop over method number and add number of class
          //for (UInt_t inc=0; inc<classNames.size(); inc++) {
          //    resultsTreeVars[(std::distance(Use.begin(), it) + inc)] = p[it->first][inc];
          //}
          resultsTreeVars[(std::distance(Use.begin(), it))] = p[it->first];
       }
       
       resultsTree->Fill();
    }
    sw.Stop();
    std::cout << "--- End of event loop: "; sw.Print();
 
    // Fill tree
    resultsFile->Write();
    resultsFile->Close();

    std::cout << "==> Topological split application is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
    ApplicationSingleBackground();
    return 0;
}
