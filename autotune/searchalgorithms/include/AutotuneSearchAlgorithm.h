#ifndef AUTOTUNE_SEARCHALGORITHM_H_
#define AUTOTUNE_SEARCHALGORITHM_H_

#include "ISearchAlgorithm.h"
#include "SearchAlgorithmManagement.h"
#include "ScenarioPoolSet.h"
#include "DriverContext.h"

typedef std::pair<int, double> MyPair;

struct CompareByValue {
    bool operator()( const MyPair& a, const MyPair& b ) const {
        //printf("%d %f, %d %f\n", a.first, a.second, b.first, b.second);
        return a.second < b.second;
    };
};

#endif
