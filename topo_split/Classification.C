#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include<TMVA/MethodRXGB.h>



void Classification ()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();
    ROOT::R::TRInterface &r = ROOT::R::TRInterface::Instance();

    vector<Int_t> Topos = {0};

    for (auto const& topo: Topos) {

        std::cout<<"Training and testing BDT for topology " << topo << std::endl;
        // load input file        

        TFile *input(0);
        TString fname = "../data/tmva3/Data2Pi_T"+std::to_string(topo)+".root";
        input = TFile::Open( fname );
        if (!input) {
           std::cout << "ERROR: could not open data file" << std::endl;
           exit(1);
        }
        std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;

        // get input tree
        TString treeName = "Topo"+std::to_string(topo)+"Tree";
        TTree *inputTree = (TTree*)input->Get(treeName);
        gROOT->cd();
        TTree *signalTree = inputTree->CopyTree("Correct==1");
        gROOT->cd();
        TTree *backgroundTree = inputTree->CopyTree("Correct==0");


        TString outputName = "Topo" + std::to_string(topo) + ".root";

        TFile *outputFile = TFile::Open(outputName, "RECREATE");

        TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification" );

        TMVA::DataLoader *dataloader=new TMVA::DataLoader("datasetTopo"+std::to_string(topo));

        vector<TString> variableNames;

        if (topo == 0) {

            // Set branches in loop
            variableNames = {"NDet", "NPerm", "Detector"};
            vector<TString> particleNames = {"El", "P","Pip","Pim"};
            vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNames) {
                for(auto const& pp: particleProperties) {
                    variableNames.push_back(pn+pp);
                };
            };
        } 

        if (topo == 1) {

            // Set variables names
            variableNames = {"NDet", "NPerm", "Detector"};
            vector<TString> particleNames = {"El", "P","Pip"};
            vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNames) {
                for(auto const& pp: particleProperties) {
                    variableNames.push_back(pn+pp);
                };
            };

        }
    
        if (topo == 2) {

            // Set variables names
            variableNames = {"NDet", "NPerm", "Detector"};
            vector<TString> particleNames = {"El", "P","Pim"};
            vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNames) {
                for(auto const& pp: particleProperties) {
                    if (pn+pp == "PTime") {
                        variableNames.push_back("ElDet");
                        variableNames.push_back(pn+pp);
                    }
                    
                    else if (pn+pp == "PimTime") {
                        variableNames.push_back("PDet");
                        variableNames.push_back(pn+pp);
                    }
                    else {
                        variableNames.push_back(pn+pp);
                    }
                };
            };
        }

        if (topo == 3) {
            // Set variables names
            variableNames = {"NDet", "NPerm", "Detector"};
            vector<TString> particleNames = {"El", "Pip","Pim"};
            vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

            // Add (particleName + particleProperty) string to variableNames in all possible combinations
            for(auto const& pn: particleNames) {
                for(auto const& pp: particleProperties) {
                    variableNames.push_back(pn+pp);
                };
            };
        }
        
        // add variables to reader
        // need position of variables in original treevars vector
        for (auto const& vn: variableNames) {
            dataloader->AddVariable( vn, vn, "units", 'F');
        } 

        dataloader->AddSignalTree(signalTree, 1.0);
        dataloader->AddBackgroundTree(backgroundTree, 1.0);

        dataloader->PrepareTrainingAndTestTree((TCut("")),"nTrain_Signal=10000:nTrain_Background=10000:SplitMode=Random:NormMode=NumEvents:!V" );

        factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT","!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20");

        factory->BookMethod(dataloader,TMVA::Types::kRXGB, "RXGB", "!V:NRounds=160:MaxDepth=3:Eta=1");

        factory->TrainAllMethods();
        factory->TestAllMethods();
        factory->EvaluateAllMethods();

        outputFile->Close();
        delete factory;
        delete dataloader;
    }


}
