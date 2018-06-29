/**
   @file    experiment.h
   @ingroup AnalysisAgent
   @brief   Performance experiment abstraction header
   @author  Edmond Kereku
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

#include "Property.h"
#include "strategy.h"

/**
 * @class Experiment
 * @ingroup AnalysisAgent
 *
 * @brief Represents an experiment
 */
class Experiment {
private:
    Application* application;
    bool         started;

public:
    Experiment( Application* app );

    ~Experiment();

    void set_application( Application* app );

    void begin();

    bool end();

    void run();

    double get_duration();

    int get_threads();
};


#endif // EXPERIMENT_H_
