#ifndef UNIFORMDISTRIBUTIONMODEL_H_
#define UNIFORMDISTRIBUTIONMODEL_H_

#include "RandomSearch.h"

#include <boost/random/mersenne_twister.hpp>

class UniformDistributionModel : public ProbabilityModel {
private:
    SearchSpace* searchSpace;

    mutable boost::mt19937 rng;

public:
    explicit UniformDistributionModel( SearchSpace* ss );

    TuningSpecification* sample() const;
};

#endif /* UNIFORMDISTRIBUTIONMODEL_H_ */
