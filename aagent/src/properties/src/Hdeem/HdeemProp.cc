/**
   @file    HdeemProp.cc
   @ingroup HdeemProperties
   @brief   Energy Measurements using hdeem
   @author  Andreas Gocht
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
    Copyright (c) 2016, Technische Universitaet Dresden, Germany
    See the COPYING file in the base directory of the package for details.

   @endverbatim
 */


#include "HdeemProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include <sstream>

//Description :  Tells you the total energy consumption
//CUDA Formula:  PSC_HDEEM_BLADE
//Recommendation :
//

/**
 * @brief Constructor
 * @ingroup HdeemProperties
 *
 * Constructs a performance property.
 * A performance property (e.g. load imbalance, communication, cache misses,
 * redundant computations, etc.) is a class that characterizes a specific
 * performance behavior of a program and can be checked by a set of conditions.
 *
 * @param ct Context
 * @param phaseCt Phase Context
 **/
HdeemProp::HdeemProp( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

/**
 * @brief Destructor
 * @ingroup HdeemProperties
 *
 **/
HdeemProp::~HdeemProp() {
}

/**
 * @brief Returns an unique id for a property
 * @ingroup HdeemProperties
 *
 * This method returns an unique id for a property.
 * Property Ids are defined in PropertyID file.
 *
 * @return Property id
 */
PropertyID HdeemProp::id() {
    return HDEEM_ENERGY_CONSUMTION_BLADE;
}

/**
 * @brief Prints information for a property
 * @ingroup HdeemProperties
 *
 * This method prints information for a property.
 *
 */
void HdeemProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

/**
 * @brief Returns is the performance property going to be reported.
 * @ingroup HdeemProperties
 *
 * @return true if performance property is going to be reported
 */
bool HdeemProp::condition() const {
    return energyConsumption > 0;
}

/**
 * @brief Returns the degree of confidence about the existence of a performance property.
 * @ingroup HdeemProperties
 *
 * @return the degree of confidence about the existence of a performance property.
 */
double HdeemProp::confidence() const {
    return 1.0;
}

/**
 * @brief Returns a severity figure that specifies the importance of the property.
 * @ingroup HdeemProperties
 *
 * Returns a severity figure that specifies the importance of the property.
 * The higher this figure the more important or severe a performance property is.
 * The severity can be used to concentrate first on the most severe performance
 * property during the performance tuning process.
 *
 * @return Severity of a property
 */
double HdeemProp::severity() const {
    return evaluationResult;
}

/**
 * @brief Returns the phase context of a property.
 * @ingroup HdeemProperties
 *
 * @return phase context
 */
Context* HdeemProp::get_phaseContext() {
    return phaseContext;
}

/**
 * @brief Requests the information required to evaluate the property.
 * @ingroup HdeemProperties
 *
 * Requests the information required to evaluate the property from the
 * performance database.
 *
 * @return information about the gathered info type
 */
Gather_Required_Info_Type HdeemProp::request_metrics() {
    pdb->request( context, PSC_SYNC_HDEEM_BLADE );

    return ALL_INFO_GATHERED;
}

/**
 * @brief Returns the name of a performance property.
 * @ingroup HdeemProperties
 *
 * @return Name of a property
 */
std::string HdeemProp::name() {
    return "Hdeem Energy Consumption";
}

/**
 * @brief Evaluates a property.
 * @ingroup HdeemProperties
 *
 * Takes the information from the performance database the information about the
 * execution for a property. Evaluates the property from this information.
 */
void HdeemProp::evaluate() {
    INT64 value = pdb->get( context, PSC_SYNC_HDEEM_BLADE );

    union {
        INT64 in;
        INT64 out;
    } translate;

    translate.in = value;

    energyConsumption = translate.out;


    evaluationResult = energyConsumption;
}

/**
 * @brief Returns a clone of a property.
 * @ingroup HdeemProperties
 *
 * @return Clone of a property
 */
Property* HdeemProp::clone() {
    HdeemProp* prop = new HdeemProp( context, phaseContext );
    return prop;
}

/**
 * @brief Returns information about the property as a stream.
 * @ingroup HdeemProperties
 *
 * Returns the information about the property as a stream.
 *
 * @return information about the property
 */
std::string HdeemProp::info() {
    std::stringstream stream;

    stream << '\t' << " Energy Consumption: " << energyConsumption;

    return stream.str();
}

/**
 * @brief Returns extra information about the property as a XML stream.
 * @ingroup HdeemProperties
 *
 * Returns extra information about the property as a XML stream. This information
 * is used to transfer non standard information about the property.
 *
 * @return Extra XML information about the property
 */
std::string HdeemProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<energyConsumption>" << energyConsumption << "</energyConsumption>" << std::endl;

    return stream.str();
}
