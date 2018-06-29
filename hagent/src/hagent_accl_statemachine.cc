#include "hagent_accl_statemachine.h"

namespace hagent_accl_msm_namespace {
void hagent_accl_msm::handle_request( request_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "HL ACCL request;\n" );
}

void hagent_accl_msm::handle_reply( reply_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "HL ACCL reply;\n" );
}

void hagent_accl_msm::handle_send_request( send_request_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "HL ACCL send_request;\n" );
}

void hagent_accl_msm::handle_send_reply( send_reply_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "HL ACCL send_reply;\n" );
}

void hagent_accl_msm::handle_start( start_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "HL ACCL start;\n" );
}
}
