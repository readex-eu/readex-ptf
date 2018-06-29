#include "accl_statemachine.h"

namespace aagent_accl_msm_namespace {
void aagent_accl_msm::handle_request( request_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "ACCL request;\n" );
}

void aagent_accl_msm::handle_reply( reply_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "ACCL reply;\n" );
}

void aagent_accl_msm::handle_send_request( send_request_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "ACCL send_request;\n" );
}

void aagent_accl_msm::handle_send_reply( send_reply_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "ACCL send_reply;\n" );
}
}
