
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

void Multi()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    TString outputFname = "MultiBackground.root";
    TFile* outputFile = TFile::Open( outputFname, "RECREATE");

    TMVA::Factory *factory = new TMVA::Factory( "TMVAMulticlass", outputFile,
                                                "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=multiclass");

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("datasetMulti");

    vector<TString> variableNames = { "NDet", "NPerm", "Detector"};
    vector<TString> particleNames = {"El", "P","Pip","Pim"};
    vector<TString> particleProperties = {"Time","Edep","DeltaE", "PreE", "P", "Th", "Phi", "Vz", "TrChi2", "Det"};

    // Add (particleName + particleProperty) string to variableNames in all possible combinations
    for(auto const& pn: particleNames) {
        for(auto const& pp: particleProperties) {
            variableNames.push_back(pn+pp);
        };
    };

    // Fill the tree

    for(auto const& value: variableNames) {
        dataloader->AddVariable( value, value, "units", 'F' );
    }
     
    dataloader->AddSpectator("UID", "UID");

    std::cout<<"--- Loading data..."<<std::endl;

    // load data
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

    std::cout<<"--- Preparing dataloader..."<<std::endl;

    dataloader->AddTree(signalTree, "Signals");
    dataloader->AddTree(backgroundTree, "CBackground");
    dataloader->AddTree(backgroundTreeK, "KBackground");

    dataloader->PrepareTrainingAndTestTree("","nTrain_Signals=50000:nTrain_CBackground=50000:nTrain_KBackground=50000:SplitMode=Random:NormMode=NumEvents:!V" );

    std::cout<<"--- Booking Methods..."<<std::endl;

    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT","!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=Grad:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20");

    factory->BookMethod(dataloader, TMVA::Types::kPyKeras, "PyKeras",
              "!H:!V:VarTransform=D,G:FilenameModel=model.h5:FilenameTrainedModel=trainedModel.h5:NumEpochs=100:BatchSize=32:SaveBestOnly=true:Verbose=1");

    //factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator:EstimatorType=sigmoid" ); // BFGS training with bayesian regulators

    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    outputFile->Close();
    
}
