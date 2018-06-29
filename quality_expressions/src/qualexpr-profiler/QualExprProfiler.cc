/**
   @file    QualExprProfiler.cc
   @ingroup QualityExpressionProfilerInternal
   @brief   Quality expression profilers - implementations
   @author  Laurent Morin
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.

   @endverbatim
 */

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "qualexpr-profiler/QualExprProfiler.h"

namespace quality_expressions_core {
clockid_t QualExprTimerStdUnix::m_start_clock_id = 1;

void QualExprEvent::display( const std::string& indent,
                             std::stringstream& s ) const {
    s << '[' << std::setprecision( 24 ) << m_timestamp << "] Event: state=" << m_state << " instance=" << m_eid << " semantic=" << std::hex << m_semanticId << std::dec << " value=" << m_value;
}

const QualExprEvent& QualExprEventBuilder::pushEvent( profiling_event_t* event ) {
    QualExprEvent* qeEvent = new QualExprEvent( event->m_state, event->m_semanticId );
    qeEvent->m_timestamp = m_timer.timestamp();
    qeEvent->m_value     = event->m_value;
    qeEvent->m_eid       = event->m_eid;
    return *qeEvent;
}

void QualExprEventBuilder::popEventSequence( const QualExprEvent& event ) {
    delete &event;
}

const QualExprEvent& QualExprEventBuilder::pushEvent_singleThread( profiling_event_t* event ) {
    QualExprEvent* qeEvent = &m_event;
    qeEvent->m_state      = event->m_state;
    qeEvent->m_semanticId = event->m_semanticId;
    qeEvent->m_timestamp  = m_timer.timestamp();
    qeEvent->m_value      = event->m_value;
    qeEvent->m_eid        = event->m_eid;
    return *qeEvent;
}

void QualExprEventBuilder::popEventSequence_singleThread( const QualExprEvent& event ) {
}
}
