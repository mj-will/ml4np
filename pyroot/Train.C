/// \author Michael Williams



#include "TFile.h"
#include "TTree.h"

void Train()
{
   // Load library
   TMVA::Tools::Instance();
   TMVA::PyMethodBase::PyInitialize();

   std::cout << std::endl
             << "==> Start TMVAClassificationCategoryApplication" << std::endl;

   // Prepare input trees for each detector
   std::vector<TString> fnames;
   fnames.push_back( "../data/tmva3/Data2Pi_0.root" );
   fnames.push_back( "../data/tmva3/Data2Pi_1.root" );
}
