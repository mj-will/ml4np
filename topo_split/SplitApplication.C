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
    Use["PyKeras"]      = 0;
    Use["BDT"]          = 1;
    Use["BDT0"]          = 0;
    // ---------------------------------------------------------------

    std::cout << std::endl
              << "==> Start TMVAClassificationCategoryApplication" << std::endl;


    // Prepare input tree
    TString fname = "/home/mikew/git_repos/HASPECT6/Projects/tmva/SmartGenSplits.root";
    //TString fname = "../data/tmva3/Data2Pi_clean.root";
    std::cout << "--- TMVAClassificationApp    : Accessing " << fname << "!" << std::endl;
    TFile *input = TFile::Open(fname);
    if (!input) {
       std::cout << "ERROR: could not open data file: " << fname << std::endl;
       exit(1);
    }
    // Prepare the tree;
    TTree* theTree = (TTree*)input->Get("THSMVATree");

    Int_t NPerm, NDet, Correct, Detector, Topo;
    Float_t fNPerm, fNDet, fDetector;
    Double_t MissMass, MissMass2;
    
    theTree->SetBranchAddress("MissMass2",&MissMass2);
    theTree->SetBranchAddress("MissMass",&MissMass);
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

    // make readers
    
    std::vector<TMVA::Reader *> readers = {};
    std::vector<Int_t> topos = {0, 1, 2, 3};

    // loop over topologies and make a reader for each
    for (auto const& topo: topos) {
        TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );
        readers.push_back(reader);

        if (topo == 0) {

            // Set branches in loop
            vector<TString> variableNamesTopo0 = {};
            vector<TString> particleNamesTopo0 = {"El", "P","Pip","Pim"};
            vector<TString> particlePropertiesTopo0 = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNamesTopo0) {
                for(auto const& pp: particlePropertiesTopo0) {
                    variableNamesTopo0.push_back(pn+pp);
                };
            };
            // add remaining variables
            readers[topo]->AddVariable("NDet",    &fNDet);
            readers[topo]->AddVariable("NPerm",   &fNPerm);
            readers[topo]->AddVariable("Detector",&fDetector);

        
            // add most of the variables
            for (UInt_t ivar = 0; ivar<nVars; ivar++) {
                readers[topo]->AddVariable( variableNames[ivar], &(treevars[ivar]));
            }
        } 

        if (topo == 1) {

            // Set variables names
            vector<TString> variableNamesTopo1 = {};
            vector<TString> particleNamesTopo1 = {"El", "P","Pip"};
            vector<TString> particlePropertiesTopo1 = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNamesTopo1) {
                for(auto const& pp: particlePropertiesTopo1) {
                    variableNamesTopo1.push_back(pn+pp);
                };
            };
            // add remaining variables
            readers[topo]->AddVariable("NPerm",   &fNPerm);
            readers[topo]->AddVariable("NDet",    &fNDet);
            readers[topo]->AddVariable("Detector",&fDetector);

            int nVarsTopo1 = variableNamesTopo1.size();
            
            // add variables to reader
            // need position of variables in original treevars vector
            for (auto const& vn: variableNamesTopo1) {
                // get index of current variable name in complete list
                // this corresponds to index in treevars
                auto idx = distance(variableNames.begin(), find(variableNames.begin(), variableNames.end(), vn));
                readers[topo]->AddVariable( vn, &(treevars[idx]));
            }
        }
    

        if (topo == 2) {

            // Set variables names
            vector<TString> variableNamesTopo2 = {};
            vector<TString> particleNamesTopo2 = {"El", "P","Pim"};
            vector<TString> particlePropertiesTopo2 = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNamesTopo2) {
                for(auto const& pp: particlePropertiesTopo2) {
                    if (pn+pp == "PTime") {
                        variableNamesTopo2.push_back("ElDet");
                        variableNamesTopo2.push_back(pn+pp);
                    }
                    
                    else if (pn+pp == "PimTime") {
                        variableNamesTopo2.push_back("PDet");
                        variableNamesTopo2.push_back(pn+pp);
                    }
                    else {
                        variableNamesTopo2.push_back(pn+pp);
                    }
                };
            };
            readers[topo]->AddVariable("NPerm",   &fNPerm);
            readers[topo]->AddVariable("NDet",    &fNDet);
            readers[topo]->AddVariable("Detector",&fDetector);
            
            int nVarsTopo2 = variableNamesTopo2.size();

            // add variables to reader
            // need position of variables in original treevars vector
            for (auto const& vn: variableNamesTopo2) {
                // get index of current variable name in complete list
                // this corresponds to index in treevars
                auto idx = distance(variableNames.begin(), find(variableNames.begin(), variableNames.end(), vn));
                readers[topo]->AddVariable( vn, &(treevars[idx]));
            }
        }

        if (topo == 3) {


            // Set variables names
            vector<TString> variableNamesTopo3 = {};
            vector<TString> particleNamesTopo3 = {"El", "Pip","Pim"};
            vector<TString> particlePropertiesTopo3 = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNamesTopo3 in all possible combinations
            for(auto const& pn: particleNamesTopo3) {
                for(auto const& pp: particlePropertiesTopo3) {
                    variableNamesTopo3.push_back(pn+pp);
                };
            };

            // add remaining variables
            readers[topo]->AddVariable("NPerm",   &fNPerm);
            readers[topo]->AddVariable("NDet",    &fNDet);
            readers[topo]->AddVariable("Detector",&fDetector);

            int nVarsTopo3 = variableNamesTopo3.size();

            // add variables to reader
            // need position of variables in original treevars vector
            for (auto const& vn : variableNamesTopo3) {
                // get index of current variable name in complete list
                // this corresponds to index in treevars
                auto idx = distance(variableNames.begin(), find(variableNames.begin(), variableNames.end(), vn));
                readers[topo]->AddVariable( vn, &(treevars[idx]));
            }
        }
    }
    // Book the MVA methods
    // Book all methods on each reader
    for (auto const& topo: topos) {
        for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
            if (it->second) {
                TString methodName = it->first + " method";
                TString weightfile = "datasetTopo"+std::to_string(topo)+"/weights/TMVAClassification_" + TString(it->first) + ".weights.xml";
                readers[topo]->BookMVA( methodName, weightfile );
            }
        }
    }


    // Event loop
    //

    // File for results tree
    TFile *resultsFile =  new TFile("results_THSMVAData.root", "RECREATE");
 
    // Write TTree with results
    TTree *resultsTree = new TTree("resultsTree", "Tree with results");
    resultsTree->SetName("resultsTree");
 
    resultsTree->Branch("MissMass2",&MissMass2);
    resultsTree->Branch("MissMass",&MissMass);
    resultsTree->Branch("Topo",    &Topo);
    resultsTree->Branch("NPerm",   &NPerm);
    resultsTree->Branch("NDet",    &NDet);
    resultsTree->Branch("Correct", &Correct);
    resultsTree->Branch("Detector",&Detector);
 
    for(UInt_t ivar = 0; ivar<nVars; ivar++) {
        resultsTree->Branch( variableNames[ivar], &(treevars[ivar]));
    }  
    // integer used to check class
    Int_t                classID;
    resultsTree->Branch("classID", &classID);
    TString         className;
    std::vector<TString> classNames =  {"Background", "Signal", }; // {0, 1}
    std::vector<TString> branchNames = {};
    // vector to correct for ordering of data
    std::vector<Int_t> classCor = {0, 1};
 
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
 
       if (ievt == 1000) {
           //break;
       }
 
       // save event class
       classID = classCor.at(Correct);
       className = classNames.at(classID);
 
       // Return the MVA outputs 
       // loop over methods 
       for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) {
          if (!it->second) continue;
 
          TString methodName = it->first + " method";
          p[it->first] = readers[Topo]->EvaluateMVA( (methodName) );
 
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
    SplitApplication();
    return 0;
}
