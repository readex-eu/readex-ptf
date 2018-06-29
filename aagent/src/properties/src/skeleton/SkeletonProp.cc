/**
   @file    SkeletonProp.cc
   @ingroup SkeletonProperties
   @brief   Execution time of OpenCL kernel property header
   @author  Author's name
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include <sstream>
#include "../../include/skeleton/SkeletonProp.h"

//Description :  Tells you the total amount of time spent on kernel execution
//CUDA Formula:  PSC_OPENCL_KERNEL_EXECUTION_TIME
//Recommendation :
//

/**
 * @brief Constructor
 * @ingroup SkeletonProperties
 *
 * Constructs a performance property.
 * A performance property (e.g. load imbalance, communication, cache misses,
 * redundant computations, etc.) is a class that characterizes a specific
 * performance behavior of a program and can be checked by a set of conditions.
 *
 * @param ct Context
 * @param phaseCt Phase Context
 **/
SkeletonProp::SkeletonProp( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

/**
 * @brief Destructor
 * @ingroup SkeletonProperties
 *
 **/
SkeletonProp::~SkeletonProp() {
}

/**
 * @brief Returns an unique id for a property
 * @ingroup SkeletonProperties
 *
 * This method returns an unique id for a property.
 * Property Ids are defined in PropertyID file.
 *
 * @return Property id
 */
PropertyID SkeletonProp::id() {
    return OPENCL_KERNELEXECTIME;
}

/**
 * @brief Prints information for a property
 * @ingroup SkeletonProperties
 *
 * This method prints information for a property.
 *
 */
void SkeletonProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

/**
 * @brief Returns is the performance property going to be reported.
 * @ingroup SkeletonProperties
 *
 * @return true if performance property is going to be reported
 */
bool SkeletonProp::condition() const {
    return Execution_Time > 0 && severity() > 0;
}

/**
 * @brief Returns the degree of confidence about the existence of a performance property.
 * @ingroup SkeletonProperties
 *
 * @return the degree of confidence about the existence of a performance property.
 */
double SkeletonProp::confidence() const {
    return 1.0;
}

/**
 * @brief Returns a severity figure that specifies the importance of the property.
 * @ingroup SkeletonProperties
 *
 * Returns a severity figure that specifies the importance of the property.
 * The higher this figure the more important or severe a performance property is.
 * The severity can be used to concentrate first on the most severe performance
 * property during the performance tuning process.
 *
 * @return Severity of a property
 */
double SkeletonProp::severity() const {
    return evaluationResult;
}

/**
 * @brief Returns the phase context of a property.
 * @ingroup SkeletonProperties
 *
 * @return phase context
 */
Context* SkeletonProp::get_phaseContext() {
    return phaseContext;
}

/**
 * @brief Requests the information required to evaluate the property.
 * @ingroup SkeletonProperties
 *
 * Requests the information required to evaluate the property from the
 * performance database.
 *
 * @return information about the gathered info type
 */
Gather_Required_Info_Type SkeletonProp::request_metrics() {
    pdb->request( context, PSC_OPENCL_KERNEL_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

/**
 * @brief Returns the name of a performance property.
 * @ingroup SkeletonProperties
 *
 * @return Name of a property
 */
std::string SkeletonProp::name() {
    return "Active cycles";
}

/**
 * @brief Evaluates a property.
 * @ingroup SkeletonProperties
 *
 * Takes the information from the performance database the information about the
 * execution for a property. Evaluates the property from this information.
 */
void SkeletonProp::evaluate() {
    Execution_Time = pdb->get( context, PSC_OPENCL_KERNEL_EXECUTION_TIME );

    evaluationResult = ( double )( Execution_Time );
}

/**
 * @brief Returns a clone of a property.
 * @ingroup SkeletonProperties
 *
 * @return Clone of a property
 */
Property* SkeletonProp::clone() {
    SkeletonProp* prop = new SkeletonProp( context, phaseContext );
    return prop;
}

/**
 * @brief Returns information about the property as a stream.
 * @ingroup SkeletonProperties
 *
 * Returns the information about the property as a stream.
 *
 * @return information about the property
 */
std::string SkeletonProp::info() {
    std::stringstream stream;

    stream << '\t' << " Execution_Time: " << Execution_Time;

    return stream.str();
}

/**
 * @brief Returns extra information about the property as a XML stream.
 * @ingroup SkeletonProperties
 *
 * Returns extra information about the property as a XML stream. This information
 * is used to transfer non standard information about the property.
 *
 * @return Extra XML information about the property
 */
std::string SkeletonProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Execution_Time>" << Execution_Time << "</Execution_Time>" << std::endl;

    return stream.str();
}
