/// \file
/// 
//  script to sort TTree based on topology and dectector
///
/// \author Michael Williams

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

Float_t check (Float_t input)
{
    Float_t output;
    if (TMath::IsNaN(input)) {
        return 0;
    }
    else {
        return input;
    }
}
void SortTree()
{
    // load library
    TMVA::Tools::Instance();
    TMVA::PyMethodBase::PyInitialize();

    // load the initial tree
    TString fname = "../data/tmva3/Data2Pi.root";
    std::cout << "--- TMVAClassificationApp    : Accessing " << fname << "!" << endl;
    TFile *input = TFile::Open(fname, "READ");
    if (!input) {
        std::cout << "ERROR: could not open data file: " << fname << endl;
        exit(1);
    }

    // get the tree
    TTree *inputTree = (TTree*)input->Get("HSParticles");

    inputTree->Print();


    // All variables
    // General
    Int_t Topo;
    Int_t NPerm;
    Int_t NDet;
    Int_t Detector;
    Int_t Correct;
    // Electron
    Float_t ElTime;
    Float_t ElEdep;
    Float_t ElDeltaE;
    Float_t ElPreE;
    Float_t ElP;
    Float_t ElTh;
    Float_t ElPhi;
    Float_t ElVz;
    Float_t ElTrChi2;
    Int_t   ElDet;
    // Proton
    Float_t PTime;
    Float_t PEdep;
    Float_t PDeltaE;
    Float_t PPreE;
    Float_t PP;
    Float_t PTh;
    Float_t PPhi;
    Float_t PVz;
    Float_t PTrChi2;
    Int_t   PDet;
    // Pi +
    Float_t PipTime;
    Float_t PipEdep;
    Float_t PipDeltaE;
    Float_t PipPreE;
    Float_t PipP;
    Float_t PipTh;
    Float_t PipPhi;
    Float_t PipVz;
    Float_t PipTrChi2;
    Int_t   PipDet;
    // Pi -
    Float_t PimTime;
    Float_t PimEdep;
    Float_t PimDeltaE;
    Float_t PimPreE;
    Float_t PimP;
    Float_t PimTh;
    Float_t PimPhi;
    Float_t PimVz;
    Float_t PimTrChi2;
    Int_t   PimDet;

    // Detector to split for
    Int_t Det = 1;

    // create ouput file
    TString outputFname = "../data/tmva3/Data2Pi_" + std::to_string(Det) + ".root";
    TFile *output = TFile::Open(outputFname, "RECREATE");

    // trees to fill
    TTree *t0 = new TTree("D1Tree", "Tree for detector type 1");

    // set branches
    // General
    t0->Branch("Topo",    &Topo,     "Topo/I");
    t0->Branch("NPerm",   &NPerm,    "NPerm/I");
    t0->Branch("NDet",    &NDet,     "NDet/I");
    t0->Branch("Detector",&Detector, "Detector/I");
    t0->Branch("Correct", &Correct,  "Correct/I");
    // Electron
    t0->Branch("ElTime",  &ElTime,   "ElTime/F");
    t0->Branch("ElEdep",  &ElEdep,   "ElEdep/F");
    t0->Branch("ElDeltaE",&ElDeltaE, "ELDeltaE/F");
    t0->Branch("ElPreE",  &ElPreE,   "ElPreE/F");
    t0->Branch("ElP",     &ElP,      "ElP/F");
    t0->Branch("ElTh",    &ElTh,     "ElTh/F");
    t0->Branch("ElPhi",   &ElPhi,    "ElPhi/F");
    t0->Branch("ElVz",    &ElVz,     "ElVz/F");
    t0->Branch("ElTrChi2",&ElTrChi2, "ElTrChi2/F");
    t0->Branch("ElDet",   &ElDet,    "ElDet/I");
    // Proton 
    t0->Branch("PTime",  &PTime,   "PTime/F");
    t0->Branch("PEdep",  &PEdep,   "PEdep/F");
    t0->Branch("PDeltaE",&PDeltaE, "ELDeltaE/F");
    t0->Branch("PPreE",  &PPreE,   "PPreE/F");
    t0->Branch("PP",     &PP,      "PP/F");
    t0->Branch("PTh",    &PTh,     "PTh/F");
    t0->Branch("PPhi",   &PPhi,    "PPhi/F");
    t0->Branch("PVz",    &PVz,     "PVz/F");
    t0->Branch("PTrChi2",&PTrChi2, "PTrChi2/F");
    t0->Branch("PDet",   &PDet,    "PDet/I");
    // Pi +
    t0->Branch("PipTime",  &PipTime,   "PipTime/F");
    t0->Branch("PipEdep",  &PipEdep,   "PipEdep/F");
    t0->Branch("PipDeltaE",&PipDeltaE, "ELDeltaE/F");
    t0->Branch("PipPreE",  &PipPreE,   "PipPreE/F");
    t0->Branch("PipP",     &PipP,      "PipP/F");
    t0->Branch("PipTh",    &PipTh,     "PipTh/F");
    t0->Branch("PipPhi",   &PipPhi,    "PipPhi/F");
    t0->Branch("PipVz",    &PipVz,     "PipVz/F");
    t0->Branch("PipTrChi2",&PipTrChi2, "PipTrChi2/F");
    t0->Branch("PipDet",   &PipDet,    "PipDet/I");
    // Pi -
    t0->Branch("PimTime",  &PimTime,   "PimTime/F");
    t0->Branch("PimEdep",  &PimEdep,   "PimEdep/F");
    t0->Branch("PimDeltaE",&PimDeltaE, "ELDeltaE/F");
    t0->Branch("PimPreE",  &PimPreE,   "PimPreE/F");
    t0->Branch("PimP",     &PimP,      "PimP/F");
    t0->Branch("PimTh",    &PimTh,     "PimTh/F");
    t0->Branch("PimPhi",   &PimPhi,    "PimPhi/F");
    t0->Branch("PimVz",    &PimVz,     "PimVz/F");
    t0->Branch("PimTrChi2",&PimTrChi2, "PimTrChi2/F");
    t0->Branch("PimDet",   &PimDet,    "PimDet/I");

    // set up TTreeReader
    TTreeReader reader("HSParticles", input);

    // Define reader
    TTreeReaderValue<Int_t>   tmpTopo(reader, "Topo");
    TTreeReaderValue<Int_t>   tmpNPerm(reader, "NPerm");
    TTreeReaderValue<Int_t>   tmpNDet(reader, "NDet");
    TTreeReaderValue<Int_t>   tmpDetector(reader, "Detector");
    TTreeReaderValue<Int_t>   tmpCorrect(reader, "Correct");
    // Electron
    TTreeReaderValue<Float_t> tmpElTime(reader, "ElTime");
    TTreeReaderValue<Float_t> tmpElEdep(reader, "ElEdep");
    TTreeReaderValue<Float_t> tmpElDeltaE(reader, "ElDeltaE");
    TTreeReaderValue<Float_t> tmpElPreE(reader, "ElPreE");
    TTreeReaderValue<Float_t> tmpElP(reader, "ElP");
    TTreeReaderValue<Float_t> tmpElTh(reader, "ElTh");
    TTreeReaderValue<Float_t> tmpElPhi(reader, "ElPhi");
    TTreeReaderValue<Float_t> tmpElVz(reader, "ElVz");
    TTreeReaderValue<Float_t> tmpElTrChi2(reader, "ElTrChi2");
    TTreeReaderValue<Int_t>   tmpElDet(reader, "ElDet");
    // Proton 
    TTreeReaderValue<Float_t> tmpPTime(reader, "PTime");
    TTreeReaderValue<Float_t> tmpPEdep(reader, "PEdep");
    TTreeReaderValue<Float_t> tmpPDeltaE(reader, "PDeltaE");
    TTreeReaderValue<Float_t> tmpPPreE(reader, "PPreE");
    TTreeReaderValue<Float_t> tmpPP(reader, "PP");
    TTreeReaderValue<Float_t> tmpPTh(reader, "PTh");
    TTreeReaderValue<Float_t> tmpPPhi(reader, "PPhi");
    TTreeReaderValue<Float_t> tmpPVz(reader, "PVz");
    TTreeReaderValue<Float_t> tmpPTrChi2(reader, "PTrChi2");
    TTreeReaderValue<Int_t>   tmpPDet(reader, "ElDet");
    // Pi +
    TTreeReaderValue<Float_t> tmpPipTime(reader, "PipTime");
    TTreeReaderValue<Float_t> tmpPipEdep(reader, "PipEdep");
    TTreeReaderValue<Float_t> tmpPipDeltaE(reader, "PipDeltaE");
    TTreeReaderValue<Float_t> tmpPipPreE(reader, "PipPreE");
    TTreeReaderValue<Float_t> tmpPipP(reader, "PipP");
    TTreeReaderValue<Float_t> tmpPipTh(reader, "PipTh");
    TTreeReaderValue<Float_t> tmpPipPhi(reader, "PipPhi");
    TTreeReaderValue<Float_t> tmpPipVz(reader, "PipVz");
    TTreeReaderValue<Float_t> tmpPipTrChi2(reader, "PipTrChi2");
    TTreeReaderValue<Int_t>   tmpPipDet(reader, "ElDet");
    // Pi -
    TTreeReaderValue<Float_t> tmpPimTime(reader, "PimTime");
    TTreeReaderValue<Float_t> tmpPimEdep(reader, "PimEdep");
    TTreeReaderValue<Float_t> tmpPimDeltaE(reader, "PimDeltaE");
    TTreeReaderValue<Float_t> tmpPimPreE(reader, "PimPreE");
    TTreeReaderValue<Float_t> tmpPimP(reader, "PimP");
    TTreeReaderValue<Float_t> tmpPimTh(reader, "PimTh");
    TTreeReaderValue<Float_t> tmpPimPhi(reader, "PimPhi");
    TTreeReaderValue<Float_t> tmpPimVz(reader, "PimVz");
    TTreeReaderValue<Float_t> tmpPimTrChi2(reader, "PimTrChi2");
    TTreeReaderValue<Int_t>   tmpPimDet(reader, "ElDet");


    while ((reader.Next())){
        if (*tmpDetector == Det) {
            // General
            Topo = *tmpTopo;
            NPerm = *tmpNPerm;
            NDet = *tmpNDet;
            Correct = *tmpCorrect;
            // Electron
            ElTime = check(*tmpElTime);
            ElEdep = check(*tmpElEdep);
            ElDeltaE = check(*tmpElDeltaE);
            ElPreE = check(*tmpElPreE);
            ElP = check(*tmpElP);
            ElTh = check(*tmpElTh);
            ElPhi = check(*tmpElPhi);
            ElVz = check(*tmpElVz);
            ElTrChi2 = check(*tmpElTrChi2);
            // Proton 
            PTime = check(*tmpPTime);
            PEdep = check(*tmpPEdep);
            PDeltaE = check(*tmpPDeltaE);
            PPreE = check(*tmpPPreE);
            PP = check(*tmpPP);
            PTh = check(*tmpPTh);
            PPhi = check(*tmpPPhi);
            PVz = check(*tmpPVz);
            PTrChi2 = check(*tmpPTrChi2);
            // Pi + 
            PipTime = check(*tmpPipTime);
            PipEdep = check(*tmpPipEdep);
            PipDeltaE = check(*tmpPipDeltaE);
            PipPreE = check(*tmpPipPreE);
            PipP = check(*tmpPipP);
            PipTh = check(*tmpPipTh);
            PipPhi = check(*tmpPipPhi);
            PipVz = check(*tmpPipVz);
            PipTrChi2 = check(*tmpPipTrChi2);
            // Pi - 
            PimTime = check(*tmpPimTime);
            PimEdep = check(*tmpPimEdep);
            PimDeltaE = check(*tmpPimDeltaE);
            PimPreE = check(*tmpPimPreE);
            PimP = check(*tmpPimP);
            PimTh = check(*tmpPimTh);
            PimPhi = check(*tmpPimPhi);
            PimVz = check(*tmpPimVz);
            PimTrChi2 = check(*tmpPimTrChi2);

            t0->Fill();
        }

    }
    t0->Print();
    output->Write();
    output->Close();

}
