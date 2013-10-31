/*
 * \file L1TdeRCT.cc
 *
 * version 0.0 A.Savin 2008/04/26
 * version 1.0 A.Savin 2008/05/05
 * this version contains single channel histos and 1D efficiencies
 */





#include "DQM/L1TMonitor/interface/L1TdeRCT.h"

#include "DQMServices/Core/interface/DQMStore.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Provenance/interface/EventAuxiliary.h"

#include "TF2.h"

#include <iostream>
#include <bitset>

#include "CondFormats/RunInfo/interface/RunInfo.h"
#include "CondFormats/DataRecord/interface/RunSummaryRcd.h"


using namespace edm;
using namespace std;

const unsigned int PHIBINS = 18;
const float PHIMIN = -0.5;
const float PHIMAX = 17.5;

const unsigned int ETABINS = 22;
const float ETAMIN = -0.5;
const float ETAMAX = 21.5;

const unsigned int BITETABINS = 44;
const float BITETAMIN = 0;
const float BITETAMAX = 22;

const unsigned int BITPHIBINS = 72;
const float BITPHIMIN = 0;
const float BITPHIMAX = 18;

const unsigned int BITRPHIBINS = 90;
const float BITRPHIMIN = 0;
const float BITRPHIMAX = 18;

const unsigned int TPGPHIBINS = 72;
const float TPGPHIMIN = -0.5;
const float TPGPHIMAX = 71.5;

const unsigned int TPGETABINS = 64;
const float TPGETAMIN = -32.;
const float TPGETAMAX = 32.;

const unsigned int TPGRANK = 256;
const float TPGRANKMIN = -.5;
const float TPGRANKMAX = 255.5;

const unsigned int DEBINS = 127;
const float DEMIN = -63.5;
const float DEMAX = 63.5;

const unsigned int ELBINS = 64;
const float ELMIN = -.5;
const float ELMAX = 63.5;

const unsigned int HCALBINS = 1024;
const float HCALMIN = -.5;
const float HCALMAX = 1023.5;

const unsigned int PhiEtaMax = 396;
const unsigned int CHNLBINS = 396;
const float CHNLMIN = -0.5;
const float CHNLMAX = 395.5;

bool first = true ;


const int L1TdeRCT::crateFED[90]=
    {613, 614, 603, 702, 718,
     611, 612, 602, 700, 718,
     627, 610, 601,716,  722,
     625, 626, 609, 714, 722,
     623, 624, 608, 712, 722,
     621, 622, 607, 710, 720,
     619, 620, 606, 708, 720,
     617, 618, 605, 706, 720,
     615, 616, 604, 704, 718,
     631, 632, 648, 703, 719,
     629, 630, 647, 701, 719,
     645, 628, 646, 717, 723,
     643, 644, 654, 715, 723,
     641, 642, 653, 713, 723,
     639, 640, 652, 711, 721,
     637, 638, 651, 709, 721,
     635, 636, 650, 707, 721,
     633, 634, 649, 705, 719
};



L1TdeRCT::L1TdeRCT(const ParameterSet & ps) :
   rctSourceEmul_( consumes<L1CaloRegionCollection>(ps.getParameter< InputTag >("rctSourceEmul") )),
   rctSourceData_( consumes<L1CaloRegionCollection>(ps.getParameter< InputTag >("rctSourceData") )),
   ecalTPGData_( consumes<EcalTrigPrimDigiCollection>(ps.getParameter< InputTag >("ecalTPGData") )),
   hcalTPGData_( consumes<HcalTrigPrimDigiCollection>(ps.getParameter< InputTag >("hcalTPGData") )),
   gtDigisLabel_( consumes<L1GlobalTriggerReadoutRecord>(ps.getParameter< InputTag >("gtDigisLabel") )),
   gtEGAlgoName_ ( ps.getParameter< std::string >("gtEGAlgoName") ),
   doubleThreshold_ ( ps.getParameter< int >("doubleThreshold") ),
   filterTriggerType_ (ps.getParameter< int >("filterTriggerType") )
{



  singlechannelhistos_ = ps.getUntrackedParameter < bool > ("singlechannelhistos", false);

  if (singlechannelhistos_)
    if(verbose_) std::cout << "L1TdeRCT: single channels histos ON" << std::endl;

  // verbosity switch
  verbose_ = ps.getUntrackedParameter < bool > ("verbose", false);

  if (verbose_)
    std::cout << "L1TdeRCT: constructor...." << std::endl;


  dbe = NULL;
  if (ps.getUntrackedParameter < bool > ("DQMStore", false)) {
    dbe = Service < DQMStore > ().operator->();
    dbe->setVerbose(0);
  }

  outputFile_ =
      ps.getUntrackedParameter < std::string > ("outputFile", "");
  if (outputFile_.size() != 0) {
    if(verbose_) std::
  cout << "L1T Monitoring histograms will be saved to " <<
  outputFile_.c_str() << std::endl;
  }

  bool disable =
      ps.getUntrackedParameter < bool > ("disableROOToutput", false);
  if (disable) {
    outputFile_ = "";
  }

  histFolder_
    = ps.getUntrackedParameter<std::string>("HistFolder", "L1TEMU/L1TdeRCT");

  if (dbe != NULL) {
    dbe->setCurrentFolder(histFolder_);
  }



}

L1TdeRCT::~L1TdeRCT()
{
}

void L1TdeRCT::beginJob(void)
{
  nev_ = 0;
}


void L1TdeRCT::endJob(void)
{
  if (verbose_)
    std::cout << "L1TdeRCT: end job...." << std::endl;
  LogInfo("EndJob") << "analyzed " << nev_ << " events";

  if (outputFile_.size() != 0 && dbe)
    dbe->save(outputFile_);

  //std::cout << "trig count is " << trigCount << std::endl;
  //std::cout << "no trig count is " << notrigCount << std::endl;


  return;
}

void L1TdeRCT::analyze(const Event & e, const EventSetup & c)
{
  nev_++;
  if (verbose_) {
    std::cout << "L1TdeRCT: analyze...." << std::endl;
  }

    // filter according trigger type
    //  enum ExperimentType {
    //        Undefined          =  0,
    //        PhysicsTrigger     =  1,
    //        CalibrationTrigger =  2,
    //        RandomTrigger      =  3,
    //        Reserved           =  4,
    //        TracedEvent        =  5,
    //        TestTrigger        =  6,
    //        ErrorTrigger       = 15

  // fill a histogram with the trigger type, for normalization fill also last bin
    // ErrorTrigger + 1
    double triggerType = static_cast<double> (e.experimentType()) + 0.001;
    double triggerTypeLast = static_cast<double> (edm::EventAuxiliary::ExperimentType::ErrorTrigger)
                            + 0.001;
    triggerType_->Fill(triggerType);
    triggerType_->Fill(triggerTypeLast + 1);

    // filter only if trigger type is greater than 0, negative values disable filtering
    if (filterTriggerType_ >= 0) {

        // now filter, for real data only
        if (e.isRealData()) {
            if (!(e.experimentType() == filterTriggerType_)) {

                edm::LogInfo("L1TdeRCT") << "\n Event of TriggerType "
                        << e.experimentType() << " rejected" << std::endl;
                return;

            }
        }

    }

  // for GT decision word
  edm::Handle< L1GlobalTriggerReadoutRecord > gtRecord;

  // get GT decision word
  e.getByToken( gtDigisLabel_ , gtRecord );
  const DecisionWord dWord = gtRecord->decisionWord();  // this will get the decision word *before* masking disabled bits
 int effEGThresholdBitNumber = 999;
  if (gtEGAlgoName_ == "L1_SingleEG1")
    {
      effEGThresholdBitNumber = 46;
    }
  if (gtEGAlgoName_ == "L1_SingleEG5_0001")
    {
      effEGThresholdBitNumber = 47;
    }
  if (gtEGAlgoName_ == "L1_SingleEG8_0001")
    {
      effEGThresholdBitNumber = 48;
    }
  if (gtEGAlgoName_ == "L1_SingleEG10_0001")
    {
      effEGThresholdBitNumber = 49;
  }
  if (gtEGAlgoName_ == "L1_SingleEG12_0001")
    {
      effEGThresholdBitNumber = 50;
    }
  if (gtEGAlgoName_ == "L1_SingleEG15_0001")
    {
      effEGThresholdBitNumber = 51;
    }
  if (gtEGAlgoName_ == "L1_SingleEG20_0001")
    {
      effEGThresholdBitNumber = 52;
    }

  int algoBitNumber = 0;
  bool triggered = false;
  bool independent_triggered = false;
  DecisionWord::const_iterator algoItr;
  for (algoItr = dWord.begin(); algoItr != dWord.end(); algoItr++)
    {
      if (*algoItr)
        {
          triggerAlgoNumbers_->Fill(algoBitNumber);
          if (algoBitNumber == effEGThresholdBitNumber)
            {
              triggered = true;// Fill triggered events (numerator) here!
            }
          if (algoBitNumber <= 45 || algoBitNumber >= 53)
            {
              independent_triggered = true;// use the muon path only !
            }
        }
      algoBitNumber++;
    }


  if(triggered)
    trigCount++;
  else
    notrigCount++;

  // get TPGs
  edm::Handle<EcalTrigPrimDigiCollection> ecalTpData;
  edm::Handle<HcalTrigPrimDigiCollection> hcalTpData;

  // Get the RCT digis
  edm::Handle < L1CaloEmCollection > emData;
  edm::Handle < L1CaloRegionCollection > rgnData;

  // Get the RCT digis
  edm::Handle < L1CaloEmCollection > emEmul;
  edm::Handle < L1CaloRegionCollection > rgnEmul;

  // need to change to getByLabel
  // bool doEm = true; FIXME gcc461: variable 'doEm' set but not used
  // bool doHd = true; FIXME gcc461: variable 'doHd' set but not used
  bool doEcal = true;
  bool doHcal = true;

  // TPG, first try:
  e.getByToken(ecalTPGData_,ecalTpData);
  e.getByToken(hcalTPGData_,hcalTpData);

  if (!ecalTpData.isValid()) {
    edm::LogInfo("TPG DataNotFound") << "can't find EcalTrigPrimDigiCollection";
    if (verbose_)std::cout << "Can not find ecalTpData!" << std::endl ;

    doEcal = false ;
  }

  if(doEcal)
  {
  for(EcalTrigPrimDigiCollection::const_iterator iEcalTp = ecalTpData->begin(); iEcalTp != ecalTpData->end(); iEcalTp++)
    if(iEcalTp->compressedEt() > 0)
    {

  rctInputTPGEcalRank_ -> Fill(1.*(iEcalTp->compressedEt())) ;

  if(iEcalTp->id().ieta() > 0)
  {
  rctInputTPGEcalOccNoCut_ -> Fill(1.*(iEcalTp->id().ieta())-0.5,iEcalTp->id().iphi()) ;
  if(iEcalTp->compressedEt() > 3) rctInputTPGEcalOcc_ -> Fill(1.*(iEcalTp->id().ieta())-0.5,iEcalTp->id().iphi()) ;
  }
  else
  {
  rctInputTPGEcalOccNoCut_ -> Fill(1.*(iEcalTp->id().ieta())+0.5,iEcalTp->id().iphi()) ;
  if(iEcalTp->compressedEt() > 3) rctInputTPGEcalOcc_ -> Fill(1.*(iEcalTp->id().ieta())+0.5,iEcalTp->id().iphi()) ;
  }

if(verbose_) std::cout << " ECAL data: Energy: " << iEcalTp->compressedEt() << " eta " << iEcalTp->id().ieta() << " phi " << iEcalTp->id().iphi() << std::endl ;
    }
   }

  if (!hcalTpData.isValid()) {
    edm::LogInfo("TPG DataNotFound") << "can't find HcalTrigPrimDigiCollection";
    if (verbose_)std::cout << "Can not find hcalTpData!" << std::endl ;

    doHcal = false ;
  }


  if(doHcal)
  {

  for(HcalTrigPrimDigiCollection::const_iterator iHcalTp = hcalTpData->begin(); iHcalTp != hcalTpData->end(); iHcalTp++)
  {
    int highSample=0;
    int highEt=0;

    for (int nSample = 0; nSample < 10; nSample++)
      {
  if (iHcalTp->sample(nSample).compressedEt() != 0)
    {
      if(verbose_) std::cout << "HCAL data: Et "
          << iHcalTp->sample(nSample).compressedEt()
          << "  fg "
          << iHcalTp->sample(nSample).fineGrain()
          << "  ieta " << iHcalTp->id().ieta()
          << "  iphi " << iHcalTp->id().iphi()
          << "  sample " << nSample
                      << std::endl ;
      if (iHcalTp->sample(nSample).compressedEt() > highEt)
        {
    highSample = nSample;
                highEt =  iHcalTp->sample(nSample).compressedEt() ;
        }
    }

       }

     if(highEt != 0)
      {
                  if(iHcalTp->id().ieta() > 0)
                  rctInputTPGHcalOcc_ -> Fill(1.*(iHcalTp->id().ieta())-0.5,iHcalTp->id().iphi()) ;
                  else
                  rctInputTPGHcalOcc_ -> Fill(1.*(iHcalTp->id().ieta())+0.5,iHcalTp->id().iphi()) ;
                  rctInputTPGHcalSample_ -> Fill(highSample,highEt) ;
                  rctInputTPGHcalRank_ -> Fill(highEt) ;
       }

    }
  }


  e.getByToken(rctSourceData_,rgnData);
  e.getByToken(rctSourceEmul_,rgnEmul);

  if (!rgnData.isValid()) {
    edm::LogInfo("DataNotFound") << "can't find L1CaloRegionCollection";
    if (verbose_)std::cout << "Can not find rgnData!" << std::endl ;
    // doHd = false;
  }

//  if ( doHd ) {
  if (!rgnEmul.isValid()) {
    edm::LogInfo("DataNotFound") << "can't find L1CaloRegionCollection";
    // doHd = false;
    if (verbose_)std::cout << "Can not find rgnEmul!" << std::endl ;
  }
//  }


  e.getByToken(rctSourceData_,emData);
  e.getByToken(rctSourceEmul_,emEmul);

  if (!emData.isValid()) {
    edm::LogInfo("DataNotFound") << "can't find L1CaloEmCollection";
    if (verbose_)std::cout << "Can not find emData!" << std::endl ;
    // doEm = false;
  }

//  if ( doEm ) {

  if (!emEmul.isValid()) {
    edm::LogInfo("DataNotFound") << "can't find L1CaloEmCollection";
    if (verbose_)std::cout << "Can not find emEmul!" << std::endl ;
    // doEm = false;
    return ;
  }

//  }


  // Isolated and non-isolated EM

  // StepI: Reset

  int nelectrIsoData = 0;
  int nelectrNisoData = 0;
  int nelectrIsoEmul = 0;
  int nelectrNisoEmul = 0;

  int electronDataRank[2][PhiEtaMax]={{0}};
  int electronDataEta[2][PhiEtaMax]={{0}};
  int electronDataPhi[2][PhiEtaMax]={{0}};
  int electronEmulRank[2][PhiEtaMax]={{0}};
  int electronEmulEta[2][PhiEtaMax]={{0}};
  int electronEmulPhi[2][PhiEtaMax]={{0}};

    // region/bit arrays
  int nRegionData = 0;
  int nRegionEmul = 0;

  int regionDataRank[PhiEtaMax] = {0};
  int regionDataEta [PhiEtaMax] = {0};
  int regionDataPhi [PhiEtaMax] = {0};

  bool regionDataOverFlow [PhiEtaMax] = {false};
  bool regionDataTauVeto  [PhiEtaMax] = {false};
  bool regionDataMip      [PhiEtaMax] = {false};
  bool regionDataQuiet    [PhiEtaMax] = {false};
  bool regionDataHfPlusTau[PhiEtaMax] = {false};

  int regionEmulRank[PhiEtaMax] = {0};
  int regionEmulEta [PhiEtaMax] = {0};
  int regionEmulPhi [PhiEtaMax] = {0};

  bool regionEmulOverFlow [PhiEtaMax] = {false};
  bool regionEmulTauVeto  [PhiEtaMax] = {false};
  bool regionEmulMip      [PhiEtaMax] = {false};
  bool regionEmulQuiet    [PhiEtaMax] = {false};
  bool regionEmulHfPlusTau[PhiEtaMax] = {false};

if(first)
{
  first = false ;
}


  // StepII: fill variables

  for (L1CaloEmCollection::const_iterator iem = emEmul->begin();
       iem != emEmul->end();
       iem++)
  {
    if(iem->rank() >= 1)
    {
      if(iem->isolated())
      {
        rctIsoEmEmulOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi());

        // to  show bad channles in the 2D efficiency plots
        rctIsoEmIneffOcc_->Fill (iem->regionId().ieta(), iem->regionId().iphi(), 0.01);
        rctIsoEmEff1Occ_->Fill  (iem->regionId().ieta(), iem->regionId().iphi(), 0.01);

        int channel;

        channel=PHIBINS*iem->regionId().ieta()+iem->regionId().iphi();
        rctIsoEmEmulOcc1D_->Fill(channel);
        electronEmulRank[0][nelectrIsoEmul]=iem->rank() ;
        electronEmulEta[0][nelectrIsoEmul]=iem->regionId().ieta();
        electronEmulPhi[0][nelectrIsoEmul]=iem->regionId().iphi();
        nelectrIsoEmul++ ;
      }

      else
      {
        rctNisoEmEmulOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi());

        // to  show bad channles in the 2D efficiency plots
        rctNisoEmIneffOcc_->Fill (iem->regionId().ieta(), iem->regionId().iphi(), 0.01);
        rctNisoEmEff1Occ_->Fill  (iem->regionId().ieta(), iem->regionId().iphi(), 0.01);

        int channel;
//

        channel=PHIBINS*iem->regionId().ieta()+iem->regionId().iphi();
        rctNisoEmEmulOcc1D_->Fill(channel);
        electronEmulRank[1][nelectrNisoEmul]=iem->rank() ;
        electronEmulEta[1][nelectrNisoEmul]=iem->regionId().ieta();
        electronEmulPhi[1][nelectrNisoEmul]=iem->regionId().iphi();
        nelectrNisoEmul++ ;
      }
    }
  }

  for (L1CaloEmCollection::const_iterator iem = emData->begin();
       iem != emData->end();
       iem++)
  {
    if(iem->rank() >= 1)
    {
      if (iem->isolated())
      {
        rctIsoEmDataOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi());

        // new stuff to avoid 0's in emulator 2D //
        // rctIsoEmEmulOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi(),0.01);
        rctIsoEmOvereffOcc_->Fill (iem->regionId().ieta(),
                                   iem->regionId().iphi(), 0.01);

        int channel;

        channel=PHIBINS*iem->regionId().ieta()+iem->regionId().iphi();
        rctIsoEmDataOcc1D_->Fill(channel);

        // new stuff to avoid 0's
        // rctIsoEmEmulOcc1D_->Fill(channel);

        electronDataRank[0][nelectrIsoData]=iem->rank() ;
        electronDataEta[0][nelectrIsoData]=iem->regionId().ieta();
        electronDataPhi[0][nelectrIsoData]=iem->regionId().iphi();
        nelectrIsoData++ ;
      }

      else
      {
        rctNisoEmDataOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi());

        // new stuff to avoid 0's in emulator 2D //
        // rctNisoEmEmulOcc_->Fill(iem->regionId().ieta(), iem->regionId().iphi(),0.01);
        rctNisoEmOvereffOcc_->Fill (iem->regionId().ieta(),
                                    iem->regionId().iphi(), 0.01);

        int channel;

        channel=PHIBINS*iem->regionId().ieta()+iem->regionId().iphi();
        rctNisoEmDataOcc1D_->Fill(channel);

        // new stuff to avoid 0's
        // rctNisoEmEmulOcc1D_->Fill(channel);

        electronDataRank[1][nelectrNisoData]=iem->rank() ;
        electronDataEta[1][nelectrNisoData]=iem->regionId().ieta();
        electronDataPhi[1][nelectrNisoData]=iem->regionId().iphi();
        nelectrNisoData++ ;
      }
    }
  }

    // fill region/bit arrays for emulator
  for(L1CaloRegionCollection::const_iterator ireg = rgnEmul->begin();
      ireg != rgnEmul->end();
      ireg++)
  {
//     std::cout << "Emul: " << nRegionEmul << " " << ireg->gctEta() << " " << ireg->gctPhi() << std::endl;
    if(ireg->overFlow())  rctBitEmulOverFlow2D_ ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->tauVeto())   rctBitEmulTauVeto2D_  ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->mip())       rctBitEmulMip2D_      ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->quiet())     rctBitEmulQuiet2D_    ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->fineGrain()) rctBitEmulHfPlusTau2D_->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->et() > 0)
    {
      rctRegEmulOcc1D_->Fill(PHIBINS*ireg->gctEta() + ireg->gctPhi());
      rctRegEmulOcc2D_->Fill(ireg->gctEta(), ireg->gctPhi());
    }

    // to show bad channels in 2D efficiency plots:
    if(ireg->overFlow()) {
      rctBitUnmatchedEmulOverFlow2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctBitMatchedOverFlow2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
    }

    if(ireg->tauVeto()) {
      rctBitUnmatchedEmulTauVeto2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctBitMatchedTauVeto2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
    }

    if(ireg->mip()) {
      rctBitUnmatchedEmulMip2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctBitMatchedMip2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
    }

    if(ireg->quiet()) {
      rctBitUnmatchedEmulQuiet2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctBitMatchedQuiet2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
    }

    if(ireg->fineGrain()) {
      rctBitUnmatchedEmulHfPlusTau2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctBitMatchedHfPlusTau2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
    }

    if(ireg->et() > 0) {
      rctRegUnmatchedEmulOcc2D_->Fill (ireg->gctEta(), ireg->gctPhi(), 0.01);
      rctRegMatchedOcc2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01);
/*      rctRegDeltaEtOcc2D_->Fill       (ireg->gctEta(), ireg->gctPhi(), 0.01); */
    }

    nRegionEmul = PHIBINS * ireg->gctEta() + ireg->gctPhi();

    regionEmulRank     [nRegionEmul] = ireg->et() ;
    regionEmulEta      [nRegionEmul] = ireg->gctEta();
    regionEmulPhi      [nRegionEmul] = ireg->gctPhi();
    regionEmulOverFlow [nRegionEmul] = ireg->overFlow();
    regionEmulTauVeto  [nRegionEmul] = ireg->tauVeto();
    regionEmulMip      [nRegionEmul] = ireg->mip();
    regionEmulQuiet    [nRegionEmul] = ireg->quiet();
    regionEmulHfPlusTau[nRegionEmul] = ireg->fineGrain();
  }
      // fill region/bit arrays for hardware
  for(L1CaloRegionCollection::const_iterator ireg = rgnData->begin();
      ireg != rgnData->end();
      ireg++)
  {
//     std::cout << "Data: " << nRegionData << " " << ireg->gctEta() << " " << ireg->gctPhi() << std::endl;
    if(ireg->overFlow())  rctBitDataOverFlow2D_ ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->tauVeto())   rctBitDataTauVeto2D_  ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->mip())       rctBitDataMip2D_      ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->quiet())     rctBitDataQuiet2D_    ->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->fineGrain()) rctBitDataHfPlusTau2D_->Fill(ireg->gctEta(), ireg->gctPhi());
    if(ireg->et() > 0)
      {
      rctRegDataOcc1D_      ->Fill(PHIBINS*ireg->gctEta() + ireg->gctPhi());
      rctRegDataOcc2D_      ->Fill(ireg->gctEta(), ireg->gctPhi());
      }
    // to show bad channels in 2D inefficiency:
    // if(ireg->overFlow())  rctBitEmulOverFlow2D_ ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    // if(ireg->tauVeto())   rctBitEmulTauVeto2D_  ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    // if(ireg->mip())       rctBitEmulMip2D_      ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    // if(ireg->quiet())     rctBitEmulQuiet2D_    ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    // if(ireg->fineGrain()) rctBitEmulHfPlusTau2D_->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    // if(ireg->et() > 0)    rctRegEmulOcc2D_      ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->overFlow())  rctBitUnmatchedDataOverFlow2D_ ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->tauVeto())   rctBitUnmatchedDataTauVeto2D_  ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->mip())       rctBitUnmatchedDataMip2D_      ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->quiet())     rctBitUnmatchedDataQuiet2D_    ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->fineGrain()) rctBitUnmatchedDataHfPlusTau2D_->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);
    if(ireg->et() > 0)    rctRegUnmatchedDataOcc2D_      ->Fill(ireg->gctEta(), ireg->gctPhi(), 0.01);

    nRegionData = PHIBINS * ireg->gctEta() + ireg->gctPhi();

    regionDataRank     [nRegionData] = ireg->et() ;
    regionDataEta      [nRegionData] = ireg->gctEta();
    regionDataPhi      [nRegionData] = ireg->gctPhi();
    regionDataOverFlow [nRegionData] = ireg->overFlow();
    regionDataTauVeto  [nRegionData] = ireg->tauVeto();
    regionDataMip      [nRegionData] = ireg->mip();
    regionDataQuiet    [nRegionData] = ireg->quiet();
    regionDataHfPlusTau[nRegionData] = ireg->fineGrain();
  }

 if(verbose_)
{
  std::cout << "I found Data! Iso: " << nelectrIsoData << " Niso: " << nelectrNisoData <<  std::endl ;
  for(int i=0; i<nelectrIsoData; i++)
  std::cout << " Iso Energy " << electronDataRank[0][i] << " eta " << electronDataEta[0][i] << " phi " << electronDataPhi[0][i] << std::endl ;
  for(int i=0; i<nelectrNisoData; i++)
  std::cout << " Niso Energy " << electronDataRank[1][i] << " eta " << electronDataEta[1][i] << " phi " << electronDataPhi[1][i] << std::endl ;

  std::cout << "I found Emul! Iso: " << nelectrIsoEmul << " Niso: " << nelectrNisoEmul <<  std::endl ;
  for(int i=0; i<nelectrIsoEmul; i++)
  std::cout << " Iso Energy " << electronEmulRank[0][i] << " eta " << electronEmulEta[0][i] << " phi " << electronEmulPhi[0][i] << std::endl ;
  for(int i=0; i<nelectrNisoEmul; i++)
  std::cout << " Niso Energy " << electronEmulRank[1][i] << " eta " << electronEmulEta[1][i] << " phi " << electronEmulPhi[1][i] << std::endl ;

  std::cout << "I found Data! Regions: " << PhiEtaMax <<  std::endl ;
  for(int i=0; i<(int)PhiEtaMax; i++)
 if(regionDataRank[i] !=0 )  std::cout << " Energy " << regionDataRank[i] << " eta " << regionDataEta[i] << " phi " << regionDataPhi[i] << std::endl ;

  std::cout << "I found Emul! Regions: " << PhiEtaMax <<  std::endl ;
  for(int i=0; i<(int)PhiEtaMax; i++)
 if(regionEmulRank[i] !=0 )  std::cout << " Energy " << regionEmulRank[i] << " eta " << regionEmulEta[i] << " phi " << regionEmulPhi[i] << std::endl ;
}

  // StepIII: calculate and fill

  for(int k=0; k<2; k++)
  {
    int nelectrE, nelectrD;

    if(k==0)
    {
      nelectrE=nelectrIsoEmul;
      nelectrD=nelectrIsoData;
    }

    else
    {
      nelectrE=nelectrNisoEmul;
      nelectrD=nelectrNisoData;
    }

    for(int i = 0; i < nelectrE; i++)
      {
	//bool triggered = l1SingleEG2; //false; //HACK until true trigger implimented
	double trigThresh = doubleThreshold_;  //ditto
	  if(singlechannelhistos_) {
	    int chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
	    if(k==1 && independent_triggered) { //non-iso
	      //std::cout << "eta " << electronEmulEta[k][i] << " phi " << electronEmulPhi[k][i] << " with rank " <<  electronEmulRank[k][i] << std::endl;
	      trigEffOcc_[chnl]->Fill(electronEmulRank[k][i]);
//	    }
	    if(triggered)
	      trigEffTriggOcc_[chnl]->Fill(electronEmulRank[k][i]); }
	  }
	  //find number of objects with rank above 2x trigger threshold
	  //and number after requiring a trigger too
	  if(electronEmulRank[k][i]>=trigThresh){
	    if(k==1 && independent_triggered) { //non-iso
	      trigEffThreshOcc_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i]);
	      trigEffTriggThreshOcc_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.01);
//	    }
	    if(triggered)
	      trigEffTriggThreshOcc_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98001); }
	  }



	Bool_t found = kFALSE;

	for(int j = 0; j < nelectrD; j++)
      {
        if(electronEmulEta[k][i]==electronDataEta[k][j] &&
           electronEmulPhi[k][i]==electronDataPhi[k][j])
        {
          if(k==0)
          {
            rctIsoEmEff1Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98001);
            // Weight is for ROOT; when added to initial weight of 0.01, should just exceed 0.99

            int chnl;

            chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
            rctIsoEmEff1Occ1D_->Fill(chnl);
            if(singlechannelhistos_)
            {
              int energy_difference;

              energy_difference=(electronEmulRank[k][i] - electronDataRank[k][j]);
              rctIsoEffChannel_[chnl]->Fill(energy_difference);
            }
	    
            if(electronEmulRank[k][i]==electronDataRank[k][j])
            {
              rctIsoEmEff2Occ1D_->Fill(chnl);
              rctIsoEmEff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98012);
              // Weight is for ROOT; should just exceed 0.99
              // NOTE: Weight is different for eff 2 because this isn't filled initially
              // for current definition of Eff2 and Ineff2 we need to add additional
              // factor 0.99 since we divide over eff1 which is 0.99001 e.g. we use 0.99001**2 !
              rctIsoEmIneff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.0099);
            }
            else
            {
              rctIsoEmIneff2Occ1D_->Fill(chnl);
              rctIsoEmIneff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.9801);
	      //Check for the bit that is different and store it
	      bitset<8> bitDifference( electronEmulRank[k][i]^electronDataRank[k][j] );
	      for( size_t n=0; n < bitDifference.size(); n++){
		  if( n < 4 ){ rctIsoEmBitDiff_->Fill( electronEmulEta[k][i], electronEmulPhi[k][i]+n*0.25, bitDifference[n] ); }
		  if( n >= 4 ){ rctIsoEmBitDiff_->Fill( electronEmulEta[k][i]+0.5, electronEmulPhi[k][i]+(n-4)*0.25, bitDifference[n] ); }
	      }
            }
          }

          else
          {
            rctNisoEmEff1Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98001);
            // Weight is for ROOT; when added to initial weight of 0.01, should just exceed 0.99

            int chnl;

            chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
            rctNisoEmEff1Occ1D_->Fill(chnl);
            if(singlechannelhistos_)
            {
              int energy_difference;

              energy_difference=(electronEmulRank[k][i] - electronDataRank[k][j]) ;
              rctNisoEffChannel_[chnl]->Fill(energy_difference) ;
            }


            if(electronEmulRank[k][i]==electronDataRank[k][j])
            {
              rctNisoEmEff2Occ1D_->Fill(chnl);
              rctNisoEmEff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98012);
              // Weight is for ROOT; should just exceed 0.99
              // NOTE: Weight is different for eff 2 because this isn't filled initially
              // see comments fo Iso
              rctNisoEmIneff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.0099);
            }
            else
            {
              rctNisoEmIneff2Occ1D_->Fill(chnl);
              rctNisoEmIneff2Occ_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.9801);
	      //Check for the bit that is different and store it
	      bitset<8> bitDifference( electronEmulRank[k][i]^electronDataRank[k][j] );
	      for( size_t n=0; n < bitDifference.size(); n++){
		  if( n < 4 ){ rctNIsoEmBitDiff_->Fill( electronEmulEta[k][i], electronEmulPhi[k][i]+n*0.25, bitDifference[n] ); }
		  if( n >= 4 ){ rctNIsoEmBitDiff_->Fill( electronEmulEta[k][i]+0.5, electronEmulPhi[k][i]+(n-4)*0.25, bitDifference[n] ); }
	      }
            }
          }

          found = kTRUE;
        }
      }

      if(found == kFALSE)
      {
        if(k==0)
        {
          rctIsoEmIneffOcc_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98);
          // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

          int chnl;

	  //Store the bit map for the emulator
	  bitset<8> bit( electronEmulRank[k][i] );
	  for( size_t n=0; n < bit.size(); n++){
	     if( n < 4 ){ rctIsoEmBitOff_->Fill( electronEmulEta[k][i], electronEmulPhi[k][i]+n*0.25, bit[n] ); }
	     if( n >= 4 ){ rctIsoEmBitOff_->Fill( electronEmulEta[k][i]+0.5, electronEmulPhi[k][i]+(n-4)*0.25, bit[n] ); }
	  }

          chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
          rctIsoEmIneffOcc1D_->Fill(chnl);
          if(singlechannelhistos_)
          {
            rctIsoIneffChannel_[chnl]->Fill(electronEmulRank[k][i]);
          }
        }

        else
        {
          rctNisoEmIneffOcc_->Fill(electronEmulEta[k][i], electronEmulPhi[k][i], 0.98);
          // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

          int chnl;

          chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
          rctNisoEmIneffOcc1D_->Fill(chnl);

	  //Store the bit map for the emulator
	  bitset<8> bit( electronEmulRank[k][i] );
	  for( size_t n=0; n < bit.size(); n++){
	     if( n < 4 ){ rctNIsoEmBitOff_->Fill( electronEmulEta[k][i], electronEmulPhi[k][i]+n*0.25, bit[n] ); }
	     if( n >= 4 ){ rctNIsoEmBitOff_->Fill( electronEmulEta[k][i]+0.5, electronEmulPhi[k][i]+(n-4)*0.25, bit[n] ); }
	  }

          if(singlechannelhistos_)
          {
            rctNisoIneffChannel_[chnl]->Fill(electronEmulRank[k][i]);
          }
        }
      }

    }

    DivideME1D(rctIsoEmEff1Occ1D_, rctIsoEmEmulOcc1D_, rctIsoEmEff1oneD_);
    DivideME2D(rctIsoEmEff1Occ_, rctIsoEmEmulOcc_, rctIsoEmEff1_) ;
//    DivideME1D(rctIsoEmEff2Occ1D_, rctIsoEmEmulOcc1D_, rctIsoEmEff2oneD_);
//    DivideME2D(rctIsoEmEff2Occ_, rctIsoEmEmulOcc_, rctIsoEmEff2_) ;
    DivideME1D(rctIsoEmEff2Occ1D_, rctIsoEmEff1Occ1D_, rctIsoEmEff2oneD_);
    DivideME2D(rctIsoEmEff2Occ_, rctIsoEmEff1Occ_, rctIsoEmEff2_) ;
//    DivideME1D(rctIsoEmIneff2Occ1D_, rctIsoEmEmulOcc1D_, rctIsoEmIneff2oneD_);
//    DivideME2D(rctIsoEmIneff2Occ_, rctIsoEmEmulOcc_, rctIsoEmIneff2_) ;
    DivideME1D(rctIsoEmIneff2Occ1D_, rctIsoEmEff1Occ1D_, rctIsoEmIneff2oneD_);
    DivideME2D(rctIsoEmIneff2Occ_, rctIsoEmEff1Occ_, rctIsoEmIneff2_) ;

    DivideME1D(rctNisoEmEff1Occ1D_, rctNisoEmEmulOcc1D_, rctNisoEmEff1oneD_);
    DivideME2D(rctNisoEmEff1Occ_, rctNisoEmEmulOcc_, rctNisoEmEff1_);
//    DivideME1D(rctNisoEmEff2Occ1D_, rctNisoEmEmulOcc1D_, rctNisoEmEff2oneD_);
//    DivideME2D(rctNisoEmEff2Occ_, rctNisoEmEmulOcc_, rctNisoEmEff2_);
    DivideME1D(rctNisoEmEff2Occ1D_, rctNisoEmEff1Occ1D_, rctNisoEmEff2oneD_);
    DivideME2D(rctNisoEmEff2Occ_, rctNisoEmEff1Occ_, rctNisoEmEff2_);
//    DivideME1D(rctNisoEmIneff2Occ1D_, rctNisoEmEmulOcc1D_, rctNisoEmIneff2oneD_);
//    DivideME2D(rctNisoEmIneff2Occ_, rctNisoEmEmulOcc_, rctNisoEmIneff2_);
    DivideME1D(rctNisoEmIneff2Occ1D_, rctNisoEmEff1Occ1D_, rctNisoEmIneff2oneD_);
    DivideME2D(rctNisoEmIneff2Occ_, rctNisoEmEff1Occ_, rctNisoEmIneff2_);

    DivideME1D(rctIsoEmIneffOcc1D_, rctIsoEmEmulOcc1D_, rctIsoEmIneff1D_);
    DivideME2D(rctIsoEmIneffOcc_, rctIsoEmEmulOcc_, rctIsoEmIneff_);
    DivideME1D(rctNisoEmIneffOcc1D_, rctNisoEmEmulOcc1D_, rctNisoEmIneff1D_);
    DivideME2D(rctNisoEmIneffOcc_, rctNisoEmEmulOcc_, rctNisoEmIneff_);

    DivideME2D(trigEffTriggThreshOcc_, trigEffThreshOcc_, trigEffThresh_);
    if(singlechannelhistos_) {
      for(int i = 0; i < nelectrE; i++)
	{
	  int chnl=PHIBINS*electronEmulEta[k][i]+electronEmulPhi[k][i];
	  DivideME1D(trigEffTriggOcc_[chnl], trigEffOcc_[chnl], trigEff_[chnl]);
	}
    }

    for(int i = 0; i < nelectrD; i++)
    {
      Bool_t found = kFALSE;

      for(int j = 0; j < nelectrE; j++)
      {
        if(electronEmulEta[k][j]==electronDataEta[k][i] &&
           electronEmulPhi[k][j]==electronDataPhi[k][i])
        {
          found = kTRUE;
        }
      }

      if(found == kFALSE)
      {
        if(k==0)
        {
          rctIsoEmOvereffOcc_->Fill(electronDataEta[k][i], electronDataPhi[k][i], 0.98);
          // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

          int chnl;

	  //Store the bit map for the emulator
	  bitset<8> bit( electronDataRank[k][i] );
	  for( size_t n=0; n < bit.size(); n++){
	     if( n < 4 ){ rctIsoEmBitOn_->Fill( electronDataEta[k][i], electronDataPhi[k][i]+n*0.25, bit[n] ); }
	     if( n >= 4 ){ rctIsoEmBitOn_->Fill( electronDataEta[k][i]+0.5, electronDataPhi[k][i]+(n-4)*0.25, bit[n] ); }
	  }

          chnl=PHIBINS*electronDataEta[k][i]+electronDataPhi[k][i];
          rctIsoEmOvereffOcc1D_->Fill(chnl);

          if(singlechannelhistos_)
          {
            rctIsoOvereffChannel_[chnl]->Fill(electronDataRank[k][i]);
          }
        }

        else
        {
          rctNisoEmOvereffOcc_->Fill(electronDataEta[k][i], electronDataPhi[k][i], 0.98);
          // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

          int chnl;

	  //Store the bit map for the emulator
	  bitset<8> bit( electronDataRank[k][i] );
	  for( size_t n=0; n < bit.size(); n++){
	     if( n < 4 ){ rctNIsoEmBitOn_->Fill( electronDataEta[k][i], electronDataPhi[k][i]+n*0.25, bit[n] ); }
	     if( n >= 4 ){ rctNIsoEmBitOn_->Fill( electronDataEta[k][i]+0.5, electronDataPhi[k][i]+(n-4)*0.25, bit[n] ); }
	  }

          chnl=PHIBINS*electronDataEta[k][i]+electronDataPhi[k][i];
          rctNisoEmOvereffOcc1D_->Fill(chnl) ;

          if(singlechannelhistos_)
          {
            rctNisoOvereffChannel_[chnl]->Fill(electronDataRank[k][i]);
          }
        }
      }
    }

  }

    // we try new definition of overefficiency:
    DivideME1D(rctIsoEmOvereffOcc1D_, rctIsoEmDataOcc1D_, rctIsoEmOvereff1D_);
    DivideME2D(rctIsoEmOvereffOcc_, rctIsoEmDataOcc_, rctIsoEmOvereff_);
    DivideME1D(rctNisoEmOvereffOcc1D_, rctNisoEmDataOcc1D_, rctNisoEmOvereff1D_);
    DivideME2D(rctNisoEmOvereffOcc_, rctNisoEmDataOcc_, rctNisoEmOvereff_);


    // calculate region/bit information
  for(unsigned int i = 0; i < (int)PhiEtaMax; i++)
{
      Bool_t regFound       = kFALSE;
      Bool_t overFlowFound  = kFALSE;
      Bool_t tauVetoFound   = kFALSE;
      Bool_t mipFound       = kFALSE;
      Bool_t quietFound     = kFALSE;
      Bool_t hfPlusTauFound = kFALSE;

//       for(int j = 0; j < nRegionData; j++)
//    {
//         if(regionEmulEta[i] == regionDataEta[j] &&
//            regionEmulPhi[i] == regionDataPhi[j])
//         {
          if(regionDataRank[i] >= 1 && regionEmulRank[i] >= 1)
          {
            int chnl;

            chnl = PHIBINS*regionEmulEta[i] + regionEmulPhi[i];
            rctRegMatchedOcc1D_->Fill(chnl);
            rctRegMatchedOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98001);
            // Weight is for ROOT; when added to initial weight of 0.01, should just exceed 0.99

            if(singlechannelhistos_) rctRegEffChannel_[chnl]->Fill(regionEmulRank[i] - regionDataRank[i]);

           // see comments for Iso Eff2

            if(regionEmulRank[i] == regionDataRank[i])
             {
             rctRegSpEffOcc1D_->Fill(chnl);
//             rctRegSpEffOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.99001);
             rctRegSpEffOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98012);
             rctRegSpIneffOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.0099);
             }
            else
             {
             rctRegSpIneffOcc1D_->Fill(chnl);
             rctRegSpIneffOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.9801);

	      bitset<10> bitDifference( regionEmulRank[i]^regionDataRank[i] );
	      for( size_t n=0; n < bitDifference.size(); n++){
		  if( n < 5 ){ rctRegBitDiff_->Fill( regionEmulEta[i], regionEmulPhi[i]+n*0.2, bitDifference[n] ); }
		  if( n >= 5 ){ rctRegBitDiff_->Fill( regionEmulEta[i]+0.5, regionEmulPhi[i]+(n-5)*0.2, bitDifference[n] ); }
	      }

             }
            // Weight is for ROOT; should just exceed 0.99
            // NOTE: Weight is different for eff 2 because this isn't filled initially

            regFound = kTRUE;
          }

          if(regionEmulOverFlow[i] == true &&
             regionDataOverFlow[i] == true)
          {
            rctBitMatchedOverFlow2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98001);
            overFlowFound = kTRUE;
          }

          if(regionEmulTauVeto[i] == true &&
             regionDataTauVeto[i] == true)
          {
            rctBitMatchedTauVeto2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98001);
            tauVetoFound = kTRUE;
          }

          if (regionEmulMip[i] == true && regionDataMip[i] == true) {
            rctBitMatchedMip2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98001);
            mipFound = kTRUE;
          }

          if (regionEmulQuiet[i] == true && regionDataQuiet[i] == true) {
            rctBitMatchedQuiet2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98001);
            quietFound = kTRUE;
          }

          if (regionEmulHfPlusTau[i] == true && regionDataHfPlusTau[i] == true) {
            rctBitMatchedHfPlusTau2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98001);
            hfPlusTauFound = kTRUE;
          }


//         }
//       }



      if(regFound == kFALSE && regionEmulRank[i] >= 1 )
      {
        int chnl;

	bitset<10> bit( regionEmulRank[i] );
	for( size_t n=0; n < bit.size(); n++){
	   if( n < 5 ){ rctRegBitOff_->Fill( regionEmulEta[i], regionEmulPhi[i]+n*0.2, bit[n] ); }
	   if( n >= 5 ){ rctRegBitOff_->Fill( regionEmulEta[i]+0.5, regionEmulPhi[i]+(n-5)*0.2, bit[n] ); }
	}

        chnl = PHIBINS*regionEmulEta[i] + regionEmulPhi[i];
        rctRegUnmatchedEmulOcc1D_->Fill(chnl);
        rctRegUnmatchedEmulOcc2D_->Fill(regionEmulEta[i], regionEmulPhi[i],0.98);
        // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

        if(singlechannelhistos_) rctRegIneffChannel_[chnl]->Fill(regionEmulRank[i]);
      }

      if(overFlowFound == kFALSE && regionEmulOverFlow[i] == true)
      {
        rctBitUnmatchedEmulOverFlow2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98);
      }

      if(tauVetoFound == kFALSE && regionEmulTauVeto[i] == true)
      {
        rctBitUnmatchedEmulTauVeto2D_->Fill(regionEmulEta[i], regionEmulPhi[i], 0.98);
      }

      if (mipFound == kFALSE && regionEmulMip[i] == true) {
        rctBitUnmatchedEmulMip2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98);
      }

      if (quietFound == kFALSE && regionEmulQuiet[i] == true) {
        rctBitUnmatchedEmulQuiet2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98);
      }

      if (hfPlusTauFound == kFALSE && regionEmulHfPlusTau[i] == true ) {
        rctBitUnmatchedEmulHfPlusTau2D_->Fill (regionEmulEta[i], regionEmulPhi[i], 0.98);
      }


}


      DivideME1D(rctRegMatchedOcc1D_, rctRegEmulOcc1D_, rctRegEff1D_);
      DivideME2D(rctRegMatchedOcc2D_, rctRegEmulOcc2D_, rctRegEff2D_);
//      DivideME1D(rctRegSpEffOcc1D_, rctRegEmulOcc1D_, rctRegSpEff1D_);
//      DivideME2D(rctRegSpEffOcc2D_, rctRegEmulOcc2D_, rctRegSpEff2D_);
      DivideME1D(rctRegSpEffOcc1D_, rctRegMatchedOcc1D_, rctRegSpEff1D_);
      DivideME2D(rctRegSpEffOcc2D_, rctRegMatchedOcc2D_, rctRegSpEff2D_);
//      DivideME1D(rctRegSpIneffOcc1D_, rctRegEmulOcc1D_, rctRegSpIneff1D_);
//      DivideME2D(rctRegSpIneffOcc2D_, rctRegEmulOcc2D_, rctRegSpIneff2D_);
      DivideME1D(rctRegSpIneffOcc1D_, rctRegMatchedOcc1D_, rctRegSpIneff1D_);
      DivideME2D(rctRegSpIneffOcc2D_, rctRegMatchedOcc2D_, rctRegSpIneff2D_);
      DivideME2D(rctBitMatchedOverFlow2D_, rctBitEmulOverFlow2D_, rctBitOverFlowEff2D_);
      DivideME2D(rctBitMatchedTauVeto2D_, rctBitEmulTauVeto2D_, rctBitTauVetoEff2D_);
      DivideME2D (rctBitMatchedMip2D_, rctBitEmulMip2D_, rctBitMipEff2D_);
      // QUIETBIT: To add quiet bit information, uncomment following line:
      // DivideME2D (rctBitMatchedQuiet2D_, rctBitEmulQuiet2D_, rctBitQuietEff2D_);
      DivideME2D (rctBitMatchedHfPlusTau2D_, rctBitEmulHfPlusTau2D_, rctBitHfPlusTauEff2D_);

      DivideME1D (rctRegUnmatchedEmulOcc1D_, rctRegEmulOcc1D_, rctRegIneff1D_);
      DivideME2D (rctRegUnmatchedEmulOcc2D_, rctRegEmulOcc2D_, rctRegIneff2D_);
      DivideME2D (rctBitUnmatchedEmulOverFlow2D_, rctBitEmulOverFlow2D_, rctBitOverFlowIneff2D_);
      DivideME2D (rctBitUnmatchedEmulTauVeto2D_, rctBitEmulTauVeto2D_, rctBitTauVetoIneff2D_);
      DivideME2D (rctBitUnmatchedEmulMip2D_, rctBitEmulMip2D_, rctBitMipIneff2D_);
      // QUIETBIT: To add quiet bit information, uncomment the following line:
      // DivideME2D (rctBitUnmatchedEmulQuiet2D_, rctBitEmulQuiet2D_, rctBitQuietIneff2D_);
      DivideME2D (rctBitUnmatchedEmulHfPlusTau2D_, rctBitEmulHfPlusTau2D_, rctBitHfPlusTauIneff2D_);


  // for(int i = 0; i < nRegionData; i++)
  for (int i = 0; i < (int)PhiEtaMax; i++)
{
      Bool_t regFound       = kFALSE;
      Bool_t overFlowFound  = kFALSE;
      Bool_t tauVetoFound   = kFALSE;
      Bool_t mipFound       = kFALSE;
      Bool_t quietFound     = kFALSE;
      Bool_t hfPlusTauFound = kFALSE;

//       for(int j = 0; j < nRegionEmul; j++)
//      {
//         if(regionEmulEta[j] == regionDataEta[i] &&
//            regionEmulPhi[j] == regionDataPhi[i])
//         {

          if(regionEmulRank[i] >= 1 && regionDataRank[i] >= 1)
            regFound = kTRUE;

          if(regionDataOverFlow[i] == true &&
             regionEmulOverFlow[i] == true)
            overFlowFound = kTRUE;

          if(regionDataTauVeto[i] == true &&
             regionEmulTauVeto[i] == true)
            tauVetoFound = kTRUE;

          if (regionDataMip[i] == true && regionEmulMip[i] == true)
            mipFound = kTRUE;

          if (regionDataQuiet[i] == true && regionEmulQuiet[i] == true)
            quietFound = kTRUE;

          if (regionDataHfPlusTau[i] == true && regionEmulHfPlusTau[i] == true)
            hfPlusTauFound = kTRUE;
//         }
//       }

      if(regFound == kFALSE && regionDataRank[i] >= 1)
      {
        int chnl;

	bitset<10> bit( regionDataRank[i] );
	for( size_t n=0; n < bit.size(); n++){
	    if( n < 5 ){ rctRegBitOn_->Fill( regionDataEta[i], regionDataPhi[i]+n*0.2, bit[n] ); }
	    if( n >= 5 ){ rctRegBitOn_->Fill( regionDataEta[i]+0.5, regionDataPhi[i]+(n-5)*0.2, bit[n] ); }
	}


        chnl = PHIBINS*regionDataEta[i] + regionDataPhi[i];
        rctRegUnmatchedDataOcc1D_->Fill(chnl);
        rctRegUnmatchedDataOcc2D_->Fill(regionDataEta[i], regionDataPhi[i], 0.98);
        // Weight is for ROOT; when added to initial weight of 0.01, should equal 0.99

        // we try a new definition of overefficiency:
        // DivideME1D(rctRegUnmatchedDataOcc1D_, rctRegDataOcc1D_, rctRegOvereff1D_);
        // DivideME2D(rctRegUnmatchedDataOcc2D_, rctRegDataOcc2D_, rctRegOvereff2D_);

        if(singlechannelhistos_) rctRegOvereffChannel_[chnl]->Fill(regionDataRank[i]);
      }

      if(overFlowFound == kFALSE && regionDataOverFlow[i] == true )
      {
        rctBitUnmatchedDataOverFlow2D_->Fill(regionDataEta[i], regionDataPhi[i], 0.98);
      }

      if(tauVetoFound == kFALSE && regionDataTauVeto[i] == true )
      {
        rctBitUnmatchedDataTauVeto2D_->Fill(regionDataEta[i], regionDataPhi[i], 0.98);
      }

      if (mipFound == kFALSE && regionDataMip[i] == true ) {
        rctBitUnmatchedDataMip2D_->Fill (regionDataEta[i], regionDataPhi[i], 0.98);
      }

      if (quietFound == kFALSE && regionDataQuiet[i] == true ) {
        rctBitUnmatchedDataQuiet2D_->Fill (regionDataEta[i], regionDataPhi[i], 0.98);
      }

      if (hfPlusTauFound == kFALSE && regionDataHfPlusTau[i] == true ) {
        rctBitUnmatchedDataHfPlusTau2D_->Fill (regionDataEta[i], regionDataPhi[i], 0.98);
      }

}

    // we try a new definition of overefficiency:
    DivideME1D(rctRegUnmatchedDataOcc1D_, rctRegDataOcc1D_, rctRegOvereff1D_);
    DivideME2D(rctRegUnmatchedDataOcc2D_, rctRegDataOcc2D_, rctRegOvereff2D_);
    DivideME2D(rctBitUnmatchedDataOverFlow2D_, rctBitDataOverFlow2D_, rctBitOverFlowOvereff2D_);
    DivideME2D(rctBitUnmatchedDataTauVeto2D_, rctBitDataTauVeto2D_, rctBitTauVetoOvereff2D_);
    DivideME2D (rctBitUnmatchedDataMip2D_, rctBitDataMip2D_,
          rctBitMipOvereff2D_);
    // QUIETBIT: To add quiet bit information, uncomment following 2 lines:
    // DivideME2D (rctBitUnmatchedDataQuiet2D_, rctBitDataQuiet2D_,
          // rctBitQuietOvereff2D_);
    DivideME2D (rctBitUnmatchedDataHfPlusTau2D_, rctBitDataHfPlusTau2D_,
          rctBitHfPlusTauOvereff2D_);

}

void L1TdeRCT::DivideME2D(MonitorElement* numerator, MonitorElement* denominator, MonitorElement* result){

   TH2F* num = numerator->getTH2F();
   TH2F* den = denominator->getTH2F();
   TH2F* res = result->getTH2F();

   res->Divide(num,den,1,1,"");

}

void L1TdeRCT::DivideME1D(MonitorElement* numerator, MonitorElement* denominator, MonitorElement* result){

   TH1F* num = numerator->getTH1F();
   TH1F* den = denominator->getTH1F();
   TH1F* res = result->getTH1F();

   res->Divide(num,den,1,1,"");

}


void L1TdeRCT::beginRun(const edm::Run& run , const edm::EventSetup& es) 
{
  readFEDVector(fedVectorMonitorRUN_,es);

  // get hold of back-end interface
  DQMStore *dbe = 0;
  dbe = Service < DQMStore > ().operator->();

  if (dbe) {
    dbe->setCurrentFolder(histFolder_);
    dbe->rmdir(histFolder_);
  }


  if (dbe) {

    dbe->setCurrentFolder(histFolder_);

    triggerType_ =
      dbe->book1D("TriggerType", "TriggerType", 17, -0.5, 16.5);

    triggerAlgoNumbers_ =
      dbe->book1D("gtTriggerAlgoNumbers", "gtTriggerAlgoNumbers", 128, -0.5, 127.5);

    rctInputTPGEcalOcc_ =
  dbe->book2D("rctInputTPGEcalOcc", "rctInputTPGEcalOcc", TPGETABINS, TPGETAMIN,
        TPGETAMAX, TPGPHIBINS, TPGPHIMIN, TPGPHIMAX);

    rctInputTPGEcalOccNoCut_ =
  dbe->book2D("rctInputTPGEcalOccNoCut", "rctInputTPGEcalOccNoCut", TPGETABINS, TPGETAMIN,
        TPGETAMAX, TPGPHIBINS, TPGPHIMIN, TPGPHIMAX);

    rctInputTPGEcalRank_ =
  dbe->book1D("rctInputTPGEcalRank", "rctInputTPGEcalRank", TPGRANK, TPGRANKMIN, TPGRANKMAX) ;

    rctInputTPGHcalOcc_ =
  dbe->book2D("rctInputTPGHcalOcc", "rctInputTPGHcalOcc", TPGETABINS, TPGETAMIN,
        TPGETAMAX, TPGPHIBINS, TPGPHIMIN, TPGPHIMAX);

    rctInputTPGHcalSample_ =
  dbe->book1D("rctInputTPGHcalSample", "rctInputTPGHcalSample", 10, -0.5, 9.5) ;

    rctInputTPGHcalRank_ =
  dbe->book1D("rctInputTPGHcalRank", "rctInputTPGHcalRank", TPGRANK, TPGRANKMIN, TPGRANKMAX) ;

    dbe->setCurrentFolder(histFolder_+"/EffCurves/NisoEm/");

    trigEffThresh_ =
      dbe->book2D("trigEffThresh", "Rank occupancy >= 2x trig thresh",
		  ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    dbe->setCurrentFolder(histFolder_+"/EffCurves/NisoEm/ServiceData");

    trigEffThreshOcc_ =
      dbe->book2D("trigEffThreshOcc", "Rank occupancy >= 2x trig thresh",
		  ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);
    trigEffTriggThreshOcc_ =
      dbe->book2D("trigEffTriggThreshOcc", "Rank occupancy >= 2x trig thresh, triggered",
		  ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    dbe->setCurrentFolder(histFolder_+"/IsoEm");

    rctIsoEmEff1_ =
  dbe->book2D("rctIsoEmEff1", "rctIsoEmEff1", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmEff1oneD_ =
  dbe->book1D("rctIsoEmEff1oneD", "rctIsoEmEff1oneD",
        CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmEff2_ =
  dbe->book2D("rctIsoEmEff2", "rctIsoEmEff2, energy matching required", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmEff2oneD_ =
  dbe->book1D("rctIsoEmEff2oneD", "rctIsoEmEff2oneD, energy matching required",
        CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmIneff2_ =
  dbe->book2D("rctIsoEmIneff2", "rctIsoEmIneff2, energy matching required", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmIneff2oneD_ =
  dbe->book1D("rctIsoEmIneff2oneD", "rctIsoEmIneff2oneD, energy matching required",
        CHNLBINS, CHNLMIN, CHNLMAX);


    rctIsoEmIneff_ =
  dbe->book2D("rctIsoEmIneff", "rctIsoEmIneff", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmIneff1D_ =
  dbe->book1D("rctIsoEmIneff1D", "rctIsoEmIneff1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmOvereff_ =
  dbe->book2D("rctIsoEmOvereff", "rctIsoEmOvereff", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmOvereff1D_ =
  dbe->book1D("rctIsoEmOvereff1D", "rctIsoEmOvereff1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    dbe->setCurrentFolder(histFolder_+"/IsoEm/ServiceData");

    rctIsoEmDataOcc_ =
  dbe->book2D("rctIsoEmDataOcc", "rctIsoEmDataOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmDataOcc1D_ =
  dbe->book1D("rctIsoEmDataOcc1D", "rctIsoEmDataOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmEmulOcc_ =
  dbe->book2D("rctIsoEmEmulOcc", "rctIsoEmEmulOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmEmulOcc1D_ =
  dbe->book1D("rctIsoEmEmulOcc1D", "rctIsoEmEmulOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmEff1Occ_ =
  dbe->book2D("rctIsoEmEff1Occ", "rctIsoEmEff1Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmEff1Occ1D_ =
  dbe->book1D("rctIsoEmEff1Occ1D", "rctIsoEmEff1Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmEff2Occ_ =
  dbe->book2D("rctIsoEmEff2Occ", "rctIsoEmEff2Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmEff2Occ1D_ =
  dbe->book1D("rctIsoEmEff2Occ1D", "rctIsoEmEff2Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmIneff2Occ_ =
  dbe->book2D("rctIsoEmIneff2Occ", "rctIsoEmIneff2Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmIneff2Occ1D_ =
  dbe->book1D("rctIsoEmIneff2Occ1D", "rctIsoEmIneff2Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmIneffOcc_ =
  dbe->book2D("rctIsoEmIneffOcc", "rctIsoEmIneffOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmIneffOcc1D_ =
  dbe->book1D("rctIsoEmIneffOcc1D", "rctIsoEmIneffOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctIsoEmOvereffOcc_ =
  dbe->book2D("rctIsoEmOvereffOcc", "rctIsoEmOvereffOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctIsoEmOvereffOcc1D_ =
  dbe->book1D("rctIsoEmOvereffOcc1D", "rctIsoEmOvereffOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);


    dbe->setCurrentFolder(histFolder_+"/NisoEm");
    rctNisoEmEff1_ =
  dbe->book2D("rctNisoEmEff1", "rctNisoEmEff1", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmEff1oneD_ =
  dbe->book1D("rctNisoEmEff1oneD", "rctNisoEmEff1oneD",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmEff2_ =
  dbe->book2D("rctNisoEmEff2", "rctNisoEmEff2, energy matching required", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmEff2oneD_ =
  dbe->book1D("rctNisoEmEff2oneD", "rctNisoEmEff2oneD, energy matching required",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmIneff2_ =
  dbe->book2D("rctNisoEmIneff2", "rctNisoEmIneff2, energy matching required", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmIneff2oneD_ =
  dbe->book1D("rctNisoEmIneff2oneD", "rctNisoEmIneff2oneD, energy matching required",
                    CHNLBINS, CHNLMIN, CHNLMAX);


    rctNisoEmIneff_ =
  dbe->book2D("rctNisoEmIneff", "rctNisoEmIneff", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmIneff1D_ =
  dbe->book1D("rctNisoEmIneff1D", "rctNisoEmIneff1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmOvereff_ =
  dbe->book2D("rctNisoEmOvereff", "rctNisoEmOvereff", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmOvereff1D_ =
  dbe->book1D("rctNisoEmOvereff1D", "rctNisoEmOvereff1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    dbe->setCurrentFolder(histFolder_+"/NisoEm/ServiceData");

    rctNisoEmDataOcc_ =
  dbe->book2D("rctNisoEmDataOcc", "rctNisoEmDataOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmDataOcc1D_ =
  dbe->book1D("rctNisoEmDataOcc1D", "rctNisoEmDataOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmEmulOcc_ =
  dbe->book2D("rctNisoEmEmulOcc", "rctNisoEmEmulOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmEmulOcc1D_ =
  dbe->book1D("rctNisoEmEmulOcc1D", "rctNisoEmEmulOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmEff1Occ_ =
  dbe->book2D("rctNisoEmEff1Occ", "rctNisoEmEff1Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmEff1Occ1D_ =
  dbe->book1D("rctNisoEmEff1Occ1D", "rctNisoEmEff1Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmEff2Occ_ =
  dbe->book2D("rctNisoEmEff2Occ", "rctNisoEmEff2Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmEff2Occ1D_ =
  dbe->book1D("rctNisoEmEff2Occ1D", "rctNisoEmEff2Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmIneff2Occ_ =
  dbe->book2D("rctNisoEmIneff2Occ", "rctNisoEmIneff2Occ", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmIneff2Occ1D_ =
  dbe->book1D("rctNisoEmIneff2Occ1D", "rctNisoEmIneff2Occ1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmIneffOcc_ =
  dbe->book2D("rctNisoEmIneffOcc", "rctNisoEmIneffOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmIneffOcc1D_ =
  dbe->book1D("rctNisoEmIneffOcc1D", "rctNisoEmIneffOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    rctNisoEmOvereffOcc_ =
  dbe->book2D("rctNisoEmOvereffOcc", "rctNisoEmOvereffOcc", ETABINS, ETAMIN,
        ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctNisoEmOvereffOcc1D_ =
  dbe->book1D("rctNisoEmOvereffOcc1D", "rctNisoEmOvereffOcc1D",
                    CHNLBINS, CHNLMIN, CHNLMAX);

    // region information
    dbe->setCurrentFolder(histFolder_+"/RegionData");

    rctRegEff1D_ =
      dbe->book1D("rctRegEff1D", "1D region efficiency",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegIneff1D_ =
      dbe->book1D("rctRegIneff1D", "1D region inefficiency",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegOvereff1D_ =
      dbe->book1D("rctRegOvereff1D", "1D region overefficiency",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegSpEff1D_ =
      dbe->book1D("rctRegSpEff1D", "1D region efficiency, energy matching required",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegSpIneff1D_ =
      dbe->book1D("rctRegSpIneff1D", "1D region inefficiency, energy matching required",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegEff2D_ =
      dbe->book2D("rctRegEff2D", "2D region efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegIneff2D_ =
      dbe->book2D("rctRegIneff2D", "2D region inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegOvereff2D_ =
      dbe->book2D("rctRegOvereff2D", "2D region overefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegSpEff2D_ =
      dbe->book2D("rctRegSpEff2D", "2D region efficiency, energy matching required",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegSpIneff2D_ =
      dbe->book2D("rctRegSpIneff2D", "2D region inefficiency, energy matching required",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    dbe->setCurrentFolder(histFolder_+"/RegionData/ServiceData");

    rctRegDataOcc1D_ =
      dbe->book1D("rctRegDataOcc1D", "1D region occupancy from data",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegEmulOcc1D_ =
      dbe->book1D("rctRegEmulOcc1D", "1D region occupancy from emulator",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegMatchedOcc1D_ =
      dbe->book1D("rctRegMatchedOcc1D", "1D region occupancy for matched hits",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegUnmatchedDataOcc1D_ =
      dbe->book1D("rctRegUnmatchedDataOcc1D", "1D region occupancy for unmatched hardware hits",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegUnmatchedEmulOcc1D_ =
      dbe->book1D("rctRegUnmatchedEmulOcc1D", "1D region occupancy for unmatched emulator hits",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegSpEffOcc1D_ =
      dbe->book1D("rctRegSpEffOcc1D", "1D region occupancy for \\Delta E_{T} efficiency",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegSpIneffOcc1D_ =
      dbe->book1D("rctRegSpIneffOcc1D", "1D region occupancy for \\Delta E_{T} efficiency ",
      CHNLBINS, CHNLMIN, CHNLMAX);

    rctRegDataOcc2D_ =
      dbe->book2D("rctRegDataOcc2D", "2D region occupancy from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegEmulOcc2D_ =
      dbe->book2D("rctRegEmulOcc2D", "2D region occupancy from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegMatchedOcc2D_ =
      dbe->book2D("rctRegMatchedOcc2D", "2D region occupancy for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegUnmatchedDataOcc2D_ =
      dbe->book2D("rctRegUnmatchedDataOcc2D", "2D region occupancy for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegUnmatchedEmulOcc2D_ =
      dbe->book2D("rctRegUnmatchedEmulOcc2D", "2D region occupancy for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

//    rctRegDeltaEt2D_ =
//      dbe->book2D("rctRegDeltaEt2D", " \\Delta E_{T}  for each channel",
//      CHNLBINS, CHNLMIN, CHNLMAX, 100, -50., 50.);

    rctRegSpEffOcc2D_ =
      dbe->book2D("rctRegSpEffOcc2D", "2D region occupancy for \\Delta E_{T} efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctRegSpIneffOcc2D_ =
      dbe->book2D("rctRegSpIneffOcc2D", "2D region occupancy for \\Delta E_{T} inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    // bit information
    dbe->setCurrentFolder(histFolder_+"/BitData");

    rctBitOverFlowEff2D_ =
      dbe->book2D("rctBitOverFlowEff2D", "2D overflow bit efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitOverFlowIneff2D_ =
      dbe->book2D("rctBitOverFlowIneff2D", "2D overflow bit inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitOverFlowOvereff2D_ =
      dbe->book2D("rctBitOverFlowOvereff2D", "2D overflow bit overefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitTauVetoEff2D_ =
      dbe->book2D("rctBitTauVetoEff2D", "2D tau veto bit efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitTauVetoIneff2D_ =
      dbe->book2D("rctBitTauVetoIneff2D", "2D tau veto bit inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitTauVetoOvereff2D_ =
      dbe->book2D("rctBitTauVetoOvereff2D", "2D tau veto bit overefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMipEff2D_ =
      dbe->book2D("rctBitMipEff2D", "2D mip bit efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMipIneff2D_ =
      dbe->book2D("rctBitMipIneff2D", "2D mip bit inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMipOvereff2D_ =
      dbe->book2D("rctBitMipOvereff2D", "2D mip bit overefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    // QUIETBIT: To add quiet bit information, uncomment following 11 lines:
    // rctBitQuietEff2D_ =
      // dbe->book2D("rctBitQuietEff2D", "2D quiet bit efficiency",
      // ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    // rctBitQuietIneff2D_ =
      // dbe->book2D("rctBitQuietIneff2D", "2D quiet bit inefficiency",
      // ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    // rctBitQuietOvereff2D_ =
      // dbe->book2D("rctBitQuietOvereff2D", "2D quiet bit overefficiency",
      // ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitHfPlusTauEff2D_ =
      dbe->book2D("rctBitHfPlusTauEff2D", "2D HfPlusTau bit efficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitHfPlusTauIneff2D_ =
      dbe->book2D("rctBitHfPlusTauIneff2D", "2D HfPlusTau bit inefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitHfPlusTauOvereff2D_ =
      dbe->book2D("rctBitHfPlusTauOvereff2D", "2D HfPlusTau bit overefficiency",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    dbe->setCurrentFolder(histFolder_+"/BitData/ServiceData");

    rctBitEmulOverFlow2D_ =
      dbe->book2D("rctBitEmulOverFlow2D", "2D overflow bit from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitDataOverFlow2D_ =
      dbe->book2D("rctBitDataOverFlow2D", "2D overflow bit from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMatchedOverFlow2D_ =
      dbe->book2D("rctBitMatchedOverFlow2D", "2D overflow bit for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedEmulOverFlow2D_ =
      dbe->book2D("rctBitUnmatchedEmulOverFlow2D", "2D overflow bit for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedDataOverFlow2D_ =
      dbe->book2D("rctBitUnmatchedDataOverFlow2D", "2D overflow bit for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitEmulTauVeto2D_ =
      dbe->book2D("rctBitEmulTauVeto2D", "2D tau veto bit from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitDataTauVeto2D_ =
      dbe->book2D("rctBitDataTauVeto2D", "2D tau veto bit from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMatchedTauVeto2D_ =
      dbe->book2D("rctBitMatchedTauVeto2D", "2D tau veto bit for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedEmulTauVeto2D_ =
      dbe->book2D("rctBitUnmatchedEmulTauVeto2D", "2D tau veto bit for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedDataTauVeto2D_ =
      dbe->book2D("rctBitUnmatchedDataTauVeto2D", "2D tau veto bit for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitEmulMip2D_ =
      dbe->book2D("rctBitEmulMip2D", "2D mip bit from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitDataMip2D_ =
      dbe->book2D("rctBitDataMip2D", "2D mip bit from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMatchedMip2D_ =
      dbe->book2D("rctBitMatchedMip2D", "2D mip bit for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedEmulMip2D_ =
      dbe->book2D("rctBitUnmatchedEmulMip2D", "2D mip bit for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedDataMip2D_ =
      dbe->book2D("rctBitUnmatchedDataMip2D", "2D mip bit for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitEmulQuiet2D_ =
      dbe->book2D("rctBitEmulQuiet2D", "2D quiet bit from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitDataQuiet2D_ =
      dbe->book2D("rctBitDataQuiet2D", "2D quiet bit from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMatchedQuiet2D_ =
      dbe->book2D("rctBitMatchedQuiet2D", "2D quiet bit for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedEmulQuiet2D_ =
      dbe->book2D("rctBitUnmatchedEmulQuiet2D", "2D quiet bit for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedDataQuiet2D_ =
      dbe->book2D("rctBitUnmatchedDataQuiet2D", "2D quiet bit for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitEmulHfPlusTau2D_ =
      dbe->book2D("rctBitEmulHfPlusTau2D", "2D HfPlusTau bit from emulator",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitDataHfPlusTau2D_ =
      dbe->book2D("rctBitDataHfPlusTau2D", "2D HfPlusTau bit from hardware",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitMatchedHfPlusTau2D_ =
      dbe->book2D("rctBitMatchedHfPlusTau2D", "2D HfPlusTau bit for matched hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedEmulHfPlusTau2D_ =
      dbe->book2D("rctBitUnmatchedEmulHfPlusTau2D", "2D HfPlusTau bit for unmatched emulator hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);

    rctBitUnmatchedDataHfPlusTau2D_ =
      dbe->book2D("rctBitUnmatchedDataHfPlusTau2D", "2D HfPlusTau bit for unmatched hardware hits",
      ETABINS, ETAMIN, ETAMAX, PHIBINS, PHIMIN, PHIMAX);
 
    dbe->setCurrentFolder(histFolder_+"/BitMon");
    rctRegBitOn_ = 
      dbe->book2D("rctRegBitOn", "Monitoring for Bits Stuck On",
      BITETABINS, BITETAMIN, BITETAMAX, BITRPHIBINS, BITRPHIMIN, BITRPHIMAX);

    rctRegBitOff_ = 
      dbe->book2D("rctRegBitOff", "Monitoring for Bits Stuck Off",
      BITETABINS, BITETAMIN, BITETAMAX, BITRPHIBINS, BITRPHIMIN, BITRPHIMAX);

    rctRegBitDiff_ = 
      dbe->book2D("rctRegBitDiff", "Monitoring for Bits Difference",
      BITETABINS, BITETAMIN, BITETAMAX, BITRPHIBINS, BITRPHIMIN, BITRPHIMAX);

    rctIsoEmBitOn_ = 
      dbe->book2D("rctIsoEmBitOn", "Monitoring for Bits Stuck On",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);

    rctIsoEmBitOff_ = 
      dbe->book2D("rctIsoEmBitOff", "Monitoring for Bits Stuck Off",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);

    rctIsoEmBitDiff_ = 
      dbe->book2D("rctIsoEmBitDiff", "Monitoring for Bits Difference",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);

    rctNIsoEmBitOn_ = 
      dbe->book2D("rctNIsoEmBitOn", "Monitoring for Bits Stuck On",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);

    rctNIsoEmBitOff_ = 
      dbe->book2D("rctNIsoEmBitOff", "Monitoring for Bits Stuck Off",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);

    rctNIsoEmBitDiff_ = 
      dbe->book2D("rctNIsoEmBitDiff", "Monitoring for Bits Difference",
      BITETABINS, BITETAMIN, BITETAMAX, BITPHIBINS, BITPHIMIN, BITPHIMAX);


    dbe->setCurrentFolder(histFolder_+"/DBData");
    fedVectorMonitorRUN_ = dbe->book2D("rctFedVectorMonitorRUN", "FED Vector Monitor Per Run",90,0,90,2,0,2);
    fedVectorMonitorLS_ = dbe->book2D("rctFedVectorMonitorLS", "FED Vector Monitor Per LS",90,0,90,2,0,2);

    for(unsigned int i=0;i<90;++i) {
      char fed[10];
      sprintf(fed,"%d",crateFED[i]);
      fedVectorMonitorRUN_->getTH2F()->GetXaxis()->SetBinLabel(i+1,fed);
      fedVectorMonitorLS_->getTH2F()->GetXaxis()->SetBinLabel(i+1,fed);
    }
      fedVectorMonitorRUN_->getTH2F()->GetYaxis()->SetBinLabel(1,"OUT");
      fedVectorMonitorRUN_->getTH2F()->GetYaxis()->SetBinLabel(2,"IN");
      fedVectorMonitorLS_->getTH2F()->GetYaxis()->SetBinLabel(1,"OUT");
      fedVectorMonitorLS_->getTH2F()->GetYaxis()->SetBinLabel(2,"IN");

    

// for single channels

    if(singlechannelhistos_)
   {
    for(int m=0; m<12; m++)
    {
    if(m==0) dbe->setCurrentFolder(histFolder_+"/IsoEm/ServiceData/Eff1SnglChnls");
    if(m==1) dbe->setCurrentFolder(histFolder_+"/NisoEm/ServiceData/Eff1SnglChnls");
    if(m==2) dbe->setCurrentFolder(histFolder_+"/RegionData/ServiceData/EffSnglChnls");
    if(m==3) dbe->setCurrentFolder(histFolder_+"/IsoEm/ServiceData/IneffSnglChnls");
    if(m==4) dbe->setCurrentFolder(histFolder_+"/NisoEm/ServiceData/IneffSnglChnls");
    if(m==5) dbe->setCurrentFolder(histFolder_+"/RegionData/ServiceData/IneffSnglChnls");
    if(m==6) dbe->setCurrentFolder(histFolder_+"/IsoEm/ServiceData/OvereffSnglChnls");
    if(m==7) dbe->setCurrentFolder(histFolder_+"/NisoEm/ServiceData/OvereffSnglChnls");
    if(m==8) dbe->setCurrentFolder(histFolder_+"/RegionData/ServiceData/OvereffSnglChnls");
    if(m==9) dbe->setCurrentFolder(histFolder_+"/EffCurves/NisoEm/ServiceData/SingleChannels");
    if(m==10) dbe->setCurrentFolder(histFolder_+"/EffCurves/NisoEm/ServiceData/SingleChannels");
    if(m==11) dbe->setCurrentFolder(histFolder_+"/EffCurves/NisoEm/ServiceData/SingleChannels");

    for(int i=0; i<ETAMAX; i++)
    {
     for(int j=0; j<PHIMAX; j++)
     {
     char name[80], channel[80]={""} ;

     if(m==0) strcpy(name,"(Eemul-Edata)Chnl") ;
     if(m==1) strcpy(name,"(Eemul-Edata)Chnl") ;
     if(m==2) strcpy(name,"(Eemul-Edata)Chnl") ;
     if(m==3) strcpy(name,"EemulChnl") ;
     if(m==4) strcpy(name,"EemulChnl") ;
     if(m==5) strcpy(name,"EemulChnl") ;
     if(m==6) strcpy(name,"EdataChnl") ;
     if(m==7) strcpy(name,"EdataChnl") ;
     if(m==8) strcpy(name,"EdataChnl") ;
     if(m==9) strcpy(name,"EemulChnlEff") ;
     if(m==10) strcpy(name,"EemulChnlTrig") ;
     if(m==11) strcpy(name,"EemulChnl") ;

     if(i<10 && j<10) sprintf(channel,"_0%d0%d",i,j);
     else if(i<10) sprintf(channel,"_0%d%d",i,j);
      else if(j<10) sprintf(channel,"_%d0%d",i,j);
       else sprintf(channel,"_%d%d",i,j);
     strcat(name,channel);

     int chnl=PHIBINS*i+j;

     if(m==0) rctIsoEffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==1) rctNisoEffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==2) rctRegEffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==3) rctIsoIneffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==4) rctNisoIneffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==5) rctRegIneffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==6) rctIsoOvereffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==7) rctNisoOvereffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==8) rctRegOvereffChannel_[chnl] =
  dbe->book1D(name, name, DEBINS, DEMIN, DEMAX);
     if(m==9) trigEff_[chnl] =
	 dbe->book1D(name, name, ELBINS, ELMIN, ELMAX);
     if(m==10) trigEffOcc_[chnl] =
	 dbe->book1D(name, name, ELBINS, ELMIN, ELMAX);
     if(m==11) trigEffTriggOcc_[chnl] =
	 dbe->book1D(name, name, ELBINS, ELMIN, ELMAX);
     }
    }
    }
   }

//end of single channels


  }
  notrigCount=0;
  trigCount=0;

}

void L1TdeRCT::beginLuminosityBlock(const edm::LuminosityBlock& ls,const edm::EventSetup& es)
{
  readFEDVector(fedVectorMonitorLS_,es);
}

void L1TdeRCT::readFEDVector(MonitorElement* histogram,const edm::EventSetup& es)
{
  // adding fed mask into channel mask
  edm::ESHandle<RunInfo> sum;
  es.get<RunInfoRcd>().get(sum);
  const RunInfo* summary=sum.product();

  std::vector<int> caloFeds;  // pare down the feds to the intresting ones

  const std::vector<int> Feds = summary->m_fed_in;
  for(std::vector<int>::const_iterator cf = Feds.begin(); cf != Feds.end(); ++cf){
    int fedNum = *cf;
    if(fedNum > 600 && fedNum <724) 
      caloFeds.push_back(fedNum);
  }
  
  for(unsigned int i=0;i<90;++i) {
    std::vector<int>::iterator fv = std::find(caloFeds.begin(),caloFeds.end(),crateFED[i]);
    if(fv!=caloFeds.end()) {
      histogram->setBinContent(i+1,2,1);
      histogram->setBinContent(i+1,1,0);
    }
    else
      {
	histogram->setBinContent(i+1,2,0);
	histogram->setBinContent(i+1,1,1);

      }
    
  }
  
} 
