/**
 * Author: Anamika Chowdhury
 * \file DetectRegion.h
 * \brief Contains a detect regions specific calls
 *
 ************************************************/
#ifndef COMPUTE_TUN_POTENTIAL_H_
#define COMPUTE_TUN_POTENTIAL_H_

#include <iostream>

#include <cubelib/Cube.h>
#include <cubelib/CubeServices.h>

#include "../../datamodel/include/SignificantRegion.h"


using namespace cube;

/*
 * Compute Intra-phase tuning potential
 */
std::list< SignificantRegion* >
ComputeIntraPhaseTP( Cnode*                      phase_node,
                     std::vector< Cnode* >&      cnodes,
                     std::vector< std::string >& region_names,
                     Cube*                       in_cube );

/*
 * Compute Inter-phase tuning potential
 */
PhaseRegion*
ComputeInterPhaseTP( Cnode* phase_node,
                     Cube*  in_cube,
                     double threshold );

/*  Print the tuning potential metric results */
void
printPotentialResults( std::list< SignificantRegion* >& significantRegions );

/* returns the average value of a phase node of a metric */
double
getPhaseNodeAvgValue( Cnode*   pnode,
                      Cube*    input,
                      Metric*  met_any,
                      Process* proc ) ;

/* returns the absolute value of a phase node of time metric */
double
getPhaseNodeTime( Cnode*   pnode,
                  Cube*    input,
                  Metric*  met_any,
                  Process* proc ) ;

#endif /*end of detect region */
