#include <iostream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

void Classification ()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    vector<Int_t> Topos = {0, 1, 2, 3};

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
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

        TMVA::DataLoader *dataloader=new TMVA::DataLoader("datasetTopo"+std::to_string(topo));


        vector<TString> variableNames = { "NDet", "NPerm", "Detector"};
        vector<TString> particleNames = {"El", "P","Pip","Pim"};
        vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2"};

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
            for(auto const& value: variableNames) {
                dataloader->AddVariable( value, value, "units", 'F' );
            }
        }  
        dataloader->AddSignalTree(signalTree, 1.0);
        dataloader->AddBackgroundTree(backgroundTree, 1.0);

        dataloader->PrepareTrainingAndTestTree((TCut("")),"nTrain_Signal=5001:nTrain_Background=5000:SplitMode=Random:NormMode=NumEvents:!V" );

        factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT","!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20");

        factory->TrainAllMethods();
        factory->TestAllMethods();
        factory->EvaluateAllMethods();

        outputFile->Close();
        delete factory;
        delete dataloader;
    }


}
