#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

int BDT()
{
   // Here the preparation phase begins

   // Read training and test data
   // (it is also possible to use ASCII format as input -> see TMVA Users Guide)
   TFile *inputSignal(0);
   TFile *inputBackground(0);
   TString fnameSignal = "./data/rho10_tm1_sm1_stat/Signal.root";
   TString fnameBackground = "./data/rho10_tm1_sm1_stat/Background.root";
   if (!gSystem->AccessPathName( fnameSignal )) {
      inputSignal = TFile::Open( fnameSignal ); // check if file in local directory exists
   }
   else {
       printf("Signal file not found");
   }

   if (!gSystem->AccessPathName( fnameBackground)) {
      inputBackground= TFile::Open( fnameBackground); // check if file in local directory exists
   }
   else {
       printf("Background file not found");
   }
   if ((!inputSignal) || (!inputBackground)) {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }
   std::cout << "--- TMVAClassification       : Using input files: " << inputSignal->GetName() << inputBackground->GetName() << std::endl;

   // Register the training and test trees

   TTree *signalTree     = (TTree*)inputSignal->Get("HSParticles");
   TTree *background     = (TTree*)inputBackground->Get("HSParticles");

   // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory is
   // the only TMVA object you have to interact with
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in
   // front of the "Silent" argument in the option string
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

   TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataset");
   // If you wish to modify default settings
   // (please check "src/Config.h" to see all available global options)
   //
   //    (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
   //    (TMVA::gConfig().GetIONames()).fWeightFileDir = "myWeightDirectory";

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
   dataloader->AddVariable( "PTime", 'F' );
   dataloader->AddVariable( "ElTime", 'F' );
   dataloader->AddVariable( "PimTime", 'F' );
   dataloader->AddVariable( "PipTime", 'F');
   dataloader->AddVariable( "ElP", 'F');
   dataloader->AddVariable( "ElTh", 'F');
   dataloader->AddVariable( "ElPhi", 'F');
   dataloader->AddVariable( "PP", 'F');
   dataloader->AddVariable( "PTh", 'F');
   dataloader->AddVariable( "PPhi", 'F');
   dataloader->AddVariable( "PimP", 'F'); 
   dataloader->AddVariable( "PimTh", 'F'); 
   dataloader->AddVariable( "PimPhi", 'F'); 
   dataloader->AddVariable( "PipP", 'F'); 
   dataloader->AddVariable( "PipTh", 'F'); 
   dataloader->AddVariable( "PipPhi", 'F'); 
   // dataloader->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
   // dataloader->AddVariable( "var3",                "Variable 3", "units", 'F' );
   // dataloader->AddVariable( "var4",                "Variable 4", "units", 'F' );

   // You can add so-called "Spectator variables", which are not used in the MVA training,
   // but will appear in the final "TestTree" produced by TMVA. This TestTree will contain the
   // input variables, the response values of all trained MVAs, and the spectator variables

   //dataloader->AddSpectator( "spec1 := var1*2",  "Spectator 1", "units", 'F' );
   //dataloader->AddSpectator( "spec2 := var1*3",  "Spectator 2", "units", 'F' );


   // global event weights per tree (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;

   // You can add an arbitrary number of signal or background trees
   dataloader->AddSignalTree    ( signalTree,     signalWeight );
   dataloader->AddBackgroundTree( background, backgroundWeight );

   // To give different trees for training and testing, do as follows:
   //
   //     dataloader->AddSignalTree( signalTrainingTree, signalTrainWeight, "Training" );
   //     dataloader->AddSignalTree( signalTestTree,     signalTestWeight,  "Test" );

   // Use the following code instead of the above two or four lines to add signal and background
   // training and test events "by hand"
   // NOTE that in this case one should not give expressions (such as "var1+var2") in the input
   //      variable definition, but simply compute the expression before adding the event
   // ```cpp
   // // --- begin ----------------------------------------------------------
   // std::vector<Double_t> vars( 4 ); // vector has size of number of input variables
   // Float_t  treevars[4], weight;
   //
   // // Signal
   // for (UInt_t ivar=0; ivar<4; ivar++) signalTree->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   // for (UInt_t i=0; i<signalTree->GetEntries(); i++) {
   //    signalTree->GetEntry(i);
   //    for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //    // add training and test events; here: first half is training, second is testing
   //    // note that the weight can also be event-wise
   //    if (i < signalTree->GetEntries()/2.0) dataloader->AddSignalTrainingEvent( vars, signalWeight );
   //    else                              dataloader->AddSignalTestEvent    ( vars, signalWeight );
   // }
   //
   // // Background (has event weights)
   // background->SetBranchAddress( "weight", &weight );
   // for (UInt_t ivar=0; ivar<4; ivar++) background->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   // for (UInt_t i=0; i<background->GetEntries(); i++) {
   //    background->GetEntry(i);
   //    for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //    // add training and test events; here: first half is training, second is testing
   //    // note that the weight can also be event-wise
   //    if (i < background->GetEntries()/2) dataloader->AddBackgroundTrainingEvent( vars, backgroundWeight*weight );
   //    else                                dataloader->AddBackgroundTestEvent    ( vars, backgroundWeight*weight );
   // }
   // // --- end ------------------------------------------------------------
   // ```
   // End of tree registration

   // Set individual event weights (the variables must exist in the original TTree)
   // -  for signal    : `dataloader->SetSignalWeightExpression    ("weight1*weight2");`
   // -  for background: `dataloader->SetBackgroundWeightExpression("weight1*weight2");`
   // dataloader->SetBackgroundWeightExpression( "weight" );

   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

   // Tell the dataloader how to use the training and testing events
   //
   // If no numbers of events are given, half of the events in the tree are used
   // for training, and the other half for testing:
   //
   //    dataloader->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
   //
   // To also specify the number of testing events, use:
   //
   //    dataloader->PrepareTrainingAndTestTree( mycut,
   //         "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );
   dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );

   // ### Book MVA methods
   //
   // Please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

   factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
                           "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );

   factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT",
                           "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

   factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT+",
                           "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=5:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

   factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTB",
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );

   factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD",
                           "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );


   // For an example of the category classifier usage, see: TMVAClassificationCategory
   //
   // --------------------------------------------------------------------------------------------------
   //  Now you can optimize the setting (configuration) of the MVAs using the set of training events
   // STILL EXPERIMENTAL and only implemented for BDT's !
   //
   //     factory->OptimizeAllMethods("SigEffAt001","Scan");
   //     factory->OptimizeAllMethods("ROCIntegral","FitGA");
   //
   // --------------------------------------------------------------------------------------------------

   // Now you can tell the factory to train, test, and evaluate the MVAs
   //
   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------

   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;

   delete factory;
   delete dataloader;
   // Launch the GUI for the root macros
   if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );

   return 0;
}

int main( int argc, char** argv )
{
   // Select methods (don't look at this code - not of interest)
   TString methodList;
   for (int i=1; i<argc; i++) {
      TString regMethod(argv[i]);
      if(regMethod=="-b" || regMethod=="--batch") continue;
      if (!methodList.IsNull()) methodList += TString(",");
      methodList += regMethod;
   }
   return BDT();
}
