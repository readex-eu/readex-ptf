#include "UniformDistributionModel.h"

#include <boost/random/uniform_smallint.hpp>

UniformDistributionModel::UniformDistributionModel( SearchSpace* ss )
    : searchSpace( ss ) {
    rng.seed( std::time( 0 ) );
}

TuningSpecification* UniformDistributionModel::sample() const {
    std::vector<TuningParameter*>   tuningParams;
    std::map<TuningParameter*, int> sample_;

    tuningParams = searchSpace->getVariantSpace()->getTuningParameters();
    for( int i = 0; i < tuningParams.size(); i++ ) {
        TuningParameter* tp = tuningParams[ i ];

        int k = ( tp->getRangeTo() - tp->getRangeFrom() ) / tp->getRangeStep();

        boost::uniform_smallint<> choice( 0, k );
        sample_[ tp ] = tp->getRangeFrom() + choice( rng ) * tp->getRangeStep();
    }

    if(withRtsSupport())
    {
       vector<Rts*> rtsVec  = searchSpace->getRts();
       list<Rts*>*  rtsList = new std::list<Rts*>( rtsVec.begin(), rtsVec.end() );
       return new TuningSpecification( new Variant( sample_ ), rtsList );
    }
    else
    {
        vector<Region*> regionVec  = searchSpace->getRegions();
        list<Region*>*  regionList = new std::list<Region*>( regionVec.begin(), regionVec.end() );
        return new TuningSpecification( new Variant( sample_ ), regionList );
    }

}
