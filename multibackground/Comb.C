
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

void Comb()
{
    // Load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    TString outputFname = "CombBackground.root";
    TFile* outputFile = TFile::Open( outputFname, "RECREATE");

    TMVA::Factory *factory = new TMVA::Factory( "TMVACombinedBackground", outputFile,
                                                "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=classification");

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("datasetComb");

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

    dataloader->AddSpectator("spec1:=__index__", "Index", "units", 'F');
     

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

    TList *list = new TList;
    list->Add(backgroundTree);
    list->Add(backgroundTreeK);
    TTree *newtree = TTree::MergeTrees(list); newtree->SetName("HSParticles"); newtree->Write();

    std::cout<<"--- Preparing dataloader..."<<std::endl;

    dataloader->AddSignalTree(signalTree);
    dataloader->AddBackgroundTree(newtree);

    dataloader->PrepareTrainingAndTestTree("","nTrain_Signal=50000:nTest_Background=50000:nTrain_Background=50000:nTest_Signal=50000:SplitMode=Random:NormMode=NumEvents:!V" );

    std::cout<<"--- Booking Methods..."<<std::endl;

    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT","!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20");

    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT", "!H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.50:nCuts=20:MaxDepth=2");

    //factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "!H:!V:NeuronType=tanh:NCycles=1000:HiddenLayers=5,5:TestRate=5:EstimatorType=MSE");

    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    outputFile->Close();
    
}
