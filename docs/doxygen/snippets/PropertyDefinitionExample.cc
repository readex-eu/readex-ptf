/** A Property class.
 *  This is just an example how a property should look like.
 *
 */

#include "AdvanceSpeculativeLoadProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

AdvanceSpeculativeLoadProp::AdvanceSpeculativeLoadProp( Context* ct, Context* phaseCt ) : Property( ct ) {
    storesALAT              = 0;
    instrInBody             = 0;
    totalInstructionsIssued = 0;
    totalStoresALAT         = 0;
    phaseCycles             = 0;
    phaseContext            = phaseCt;
}

AdvanceSpeculativeLoadProp::~AdvanceSpeculativeLoadProp() {
}

PropertyID AdvanceSpeculativeLoadProp::id() {
    return ADVANCESPECULATIVELOAD;
}

void AdvanceSpeculativeLoadProp::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl <<
    "                  " <<
    context->getRegion()->str_print( appl->get_file_name_maping() ) <<
    std::endl;
}

bool AdvanceSpeculativeLoadProp::condition() const {
    return storesALAT > 0;
}

double AdvanceSpeculativeLoadProp::confidence() const {
    return 1.0;
}

double AdvanceSpeculativeLoadProp::severity() const {
    double percentStoresALAT = ( double )storesALAT / ( double )instrInBody;
    totalStoresALAT = ( INT64 )totalInstructionsIssued * percentStoresALAT;
    return ( double )( totalStoresALAT * 2 ) / ( double )phaseCycles * 100;
    return 100.0;
}

Context* AdvanceSpeculativeLoadProp::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type AdvanceSpeculativeLoadProp::request_metrics() {
    pdb->request( context, ADVANCE_LOAD );
    pdb->request( context, INSTRUCTIONS );
    pdb->request( context, INSTRUCTIONS_IN_LOOP_BODY );
    pdb->request( phaseContext, CYCLES );
    return ALL_INFO_GATHERED;
}

std::string AdvanceSpeculativeLoadProp::name() {
    return "Advance speculative load instructions";
}

void AdvanceSpeculativeLoadProp::evaluate() {
    storesALAT              = pdb->get( context, ADVANCE_LOAD );
    totalInstructionsIssued = pdb->get( context, INSTRUCTIONS );
    instrInBody             = pdb->get( context, INSTRUCTIONS_IN_LOOP_BODY );
    phaseCycles             = pdb->get( phaseContext, CYCLES );
}

Property* AdvanceSpeculativeLoadProp::clone() {
    AdvanceSpeculativeLoadProp* prop = new AdvanceSpeculativeLoadProp( context, phaseContext );
    return prop;
}


Prop_List AdvanceSpeculativeLoadProp::next() {
}

std::string AdvanceSpeculativeLoadProp::info() {
    std::stringstream stream;

    stream << '\t' << " Stores going to ALAT: " << storesALAT << "  Total: " << totalStoresALAT;

    return stream.str();
};

std::string AdvanceSpeculativeLoadProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t<addInfo>" << std::endl;
    stream << "\t\t<storesALAT>" << storesALAT << "</storesALAT>" << std::endl;
    stream << "\t\t<totalStoresALAT>" << totalStoresALAT << "</totalStoresALAT>" << std::endl;
    stream << "\t</addInfo>" << std::endl;

    return stream.str();
}
