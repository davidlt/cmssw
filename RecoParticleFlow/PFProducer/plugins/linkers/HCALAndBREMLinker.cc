#include "RecoParticleFlow/PFProducer/interface/BlockElementLinkerBase.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElementCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElementBrem.h"
#include "RecoParticleFlow/PFClusterTools/interface/LinkByRecHit.h"

class HCALAndBREMLinker : public BlockElementLinkerBase {
public:
  HCALAndBREMLinker(const edm::ParameterSet& conf) :
    BlockElementLinkerBase(conf),
    _useKDTree(conf.getParameter<bool>("useKDTree")),
    _debug(conf.getUntrackedParameter<bool>("debug",false)) {}
  
  double testLink 
  ( const reco::PFBlockElement*,
    const reco::PFBlockElement* ) const override;

private:
  bool _useKDTree,_debug;
};

DEFINE_EDM_PLUGIN(BlockElementLinkerFactory, 
		  HCALAndBREMLinker, 
		  "HCALAndBREMLinker");

double HCALAndBREMLinker::testLink
  ( const reco::PFBlockElement* elem1,
    const reco::PFBlockElement* elem2) const { 
  constexpr reco::PFTrajectoryPoint::LayerType HCALEnt =
    reco::PFTrajectoryPoint::HCALEntrance;
  const reco::PFBlockElementCluster *hcalelem(NULL);
  const reco::PFBlockElementBrem    *bremelem(NULL);
  double dist(-1.0);
  if( elem1->type() < elem2->type() ) {
    hcalelem = static_cast<const reco::PFBlockElementCluster*>(elem1);
    bremelem = static_cast<const reco::PFBlockElementBrem*>(elem2);
  } else {
    hcalelem = static_cast<const reco::PFBlockElementCluster*>(elem2);
    bremelem = static_cast<const reco::PFBlockElementBrem*>(elem1);
  }
  const reco::PFClusterRef& clusterref = hcalelem->clusterRef();
  const reco::PFRecTrack& track = bremelem->trackPF();
  const reco::PFTrajectoryPoint& tkAtHCAL =
    track.extrapolatedPoint( HCALEnt );
  if( tkAtHCAL.isValid() ) {
    dist = LinkByRecHit::testTrackAndClusterByRecHit( track, *clusterref, 
						      true, _debug );
  }  
  return dist;
}
