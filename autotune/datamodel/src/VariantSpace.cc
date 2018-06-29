#include "VariantSpace.h"

void VariantSpace::addTuningParameter( TuningParameter* tp ) {
    tuningParameters.push_back( tp );
}

vector<TuningParameter*> VariantSpace::getTuningParameters() const {
    return tuningParameters;
}
