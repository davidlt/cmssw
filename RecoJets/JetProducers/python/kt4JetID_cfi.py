import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.JetIDParams_cfi import *

kt4JetID = cms.EDProducer('JetIDProducer', JetIDParams,
        src = cms.InputTag('kt4CaloJets')
        
)
