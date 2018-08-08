
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

void CombCut()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    TString outputFname = "CombBackgroundCut.root";
    TFile* outputFile = TFile::Open( outputFname, "RECREATE");

    TMVA::Factory *factory = new TMVA::Factory( "TMVACombinedBackgroundCut", outputFile,
                                                "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=classification");

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("datasetCombCut");

    vector<TString> variableNames = { "NDet", "NPerm", "Detector"};
    vector<TString> particleNames = {"El", "P","Pip","Pim"};
    vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };

    std::cout<<"--- Adding variables..."<<std::endl;

    for(auto const& value: variableNames) {
        dataloader->AddVariable( value, value, "units", 'F' );
        std::cout<<"            Added variable : " << value << std::endl;
    }

    std::cout<<"--- Loading data..."<<std::endl;

    // load data
    TFile *input(0);
    TString fname = "./Data_cut.root";
    input = TFile::Open( fname );
    if (!input) {
       std::cout << "ERROR: could not open data file" << std::endl;
       exit(1);
    }

    std::cout<<"--- ..."<<std::endl;

    std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;

    std::cout<<"--- Get ROOT Trees..."<<std::endl;

    // get input tree
    TString treeName = "HSParticles";
    TTree *inputTree = (TTree*)input->Get(treeName);
    gROOT->cd();
    TTree *signalTree = inputTree->CopyTree("classID==0");
    gROOT->cd();
    TTree *backgroundTree = inputTree->CopyTree("classID==1");


    std::cout<<"--- Preparing dataloader..."<<std::endl;

    dataloader->AddSignalTree(signalTree);
    dataloader->AddBackgroundTree(backgroundTree);

    dataloader->PrepareTrainingAndTestTree("","nTrain_Signal=10000:nTest_Background=1000:nTrain_Background=10000:nTest_Signal=1000:SplitMode=Random:NormMode=NumEvents:!V" );

    std::cout<<"--- Booking Methods..."<<std::endl;

    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT","!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20");

    factory->BookMethod(dataloader, TMVA::Types::kPyKeras, "PyKeras",
              "!H:!V:VarTransform=D,G:FilenameModel=modelCut.h5:FilenameTrainedModel=trainedModelCut.h5:NumEpochs=10:BatchSize=32:SaveBestOnly=true:Verbose=1");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT", "!H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.50:nCuts=20:MaxDepth=2");

    //factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "!H:!V:NeuronType=tanh:NCycles=1000:HiddenLayers=5,5:TestRate=5:EstimatorType=MSE");

    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    outputFile->Close();
    
}
