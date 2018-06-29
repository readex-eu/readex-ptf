/**
 * Author: Anamika Chowdhury
 * \file DetectRegion.h
 * \brief Contains a detect regions specific calls
 *
 ************************************************/
#ifndef PROCESSING_INTENSITY_H_
#define PROCESSING_INTENSITY_H_

#include <iostream>

#include <cubelib/Cube.h>

#include "../../datamodel/include/SignificantRegion.h"


using namespace cube;

/*
 * Compute Transfer Intensity for each significant region
 */
double
getProcessingIntensity( Cnode* sig_node,
                        Cube*  in_cube );

double
getPhaseNodeAvgProcIntnsty( Cnode*   pnode,
                            Cube *   input,
                            Process* proc );

#endif //end of detect region
