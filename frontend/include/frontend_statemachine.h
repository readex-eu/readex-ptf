#ifndef FRONTEND_STATEMACHINE
#define FRONTEND_STATEMACHINE

// maximum number of entries for transition tables
//#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
//#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
//#define BOOST_MPL_LIMIT_MAP_SIZE 50

// maximum number of states
#define FUSION_MAX_VECTOR_SIZE 15


#include "frontend.h"
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

namespace msm = boost::msm;




namespace frontend_statemachine {
// forward declarations within the namespace
StrategyRequest* createStrategyRequest( PeriscopeFrontend* fe );

void pushStrategyRequest( StrategyRequest* strategy_request );

// top level events and commands
struct basic_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Basic event" );
    basic_event( PeriscopeFrontend*  fe_in,
                 ACE_Reactor*        reactor_in,
                 ApplicationStarter* starter_in ) :
        fe( fe_in ),
        reactor( reactor_in ),
        starter( starter_in ) {
    }

    PeriscopeFrontend* const  fe;
    ACE_Reactor* const        reactor;
    ApplicationStarter* const starter;
};

struct initialize_plugin : public basic_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Initialize Plugin" );
    initialize_plugin( PeriscopeFrontend*  fe_in,
                       ACE_Reactor*        reactor_in,
                       ApplicationStarter* starter_in ) :
        basic_event( fe_in, reactor_in, starter_in ) {
    }
};

struct start_tuning_step {
    PSC_SM_TRACE_EVENT_DECLARATION( "Prepare Search" );
};

struct perform_analysis : public basic_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Perform Analysis" );
    perform_analysis( StrategyRequest*    strategy_in,
                      PeriscopeFrontend*  fe_in,
                      ACE_Reactor*        reactor_in,
                      ApplicationStarter* starter_in,
                      int                 tuning_step_in ) :
        basic_event( fe_in, reactor_in, starter_in ),
        strategy( strategy_in ),
        tuning_step( tuning_step_in ) {
    }

    StrategyRequest* strategy;
    int              tuning_step;
};

struct create_scenarios {
    PSC_SM_TRACE_EVENT_DECLARATION( "Create Scenarios" );
    create_scenarios( PeriscopeFrontend* fe_in ) : fe( fe_in ) {
    }
    PeriscopeFrontend* fe;
};

struct execute_scenarios {
    PSC_SM_TRACE_EVENT_DECLARATION( "Execute Scenarios" );
};

// sub-diagram as discussed
struct prepare_scenarios {
    PSC_SM_TRACE_EVENT_DECLARATION( "Prepare Scenarios" );
};

struct define_experiment {
    PSC_SM_TRACE_EVENT_DECLARATION( "Define Experiment" );
};

struct consider_restart : public basic_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Check Restart" );
    consider_restart( PeriscopeFrontend*  fe_in,
                      ACE_Reactor*        reactor_in,
                      ApplicationStarter* starter_in,
                      int                 search_step_in ) :
        basic_event( fe_in, reactor_in, starter_in ),
        search_step( search_step_in ) {
    }

    int search_step;
};

struct run_phase_experiments : public basic_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Run Phase Experiments" );
    run_phase_experiments( PeriscopeFrontend*  fe_in,
                           ACE_Reactor*        reactor_in,
                           ApplicationStarter* starter_in,
                           int                 search_step_in,
                           bool*               executed_in,
                           int                 experiment_count_in ) :
        basic_event( fe_in, reactor_in, starter_in ),
        search_step( search_step_in ),
        executed( executed_in ),
        experiment_count( experiment_count_in ) {
    }

    int   search_step;
    bool* executed;
    int   experiment_count;
};

// final block
struct finish_tuning_step {
    PSC_SM_TRACE_EVENT_DECLARATION( "Process Results" );
};

struct create_tuning_advice {
    PSC_SM_TRACE_EVENT_DECLARATION( "Create Tuning Advice" );
};

// tear down
struct finalize_autotuning {
    PSC_SM_TRACE_EVENT_DECLARATION( "Finalize Autotuning" );
    finalize_autotuning( PeriscopeFrontend* fe_in ) :
        fe( fe_in ) {
    }
    PeriscopeFrontend* fe;
};

class autotune_msm : public msm::front::state_machine_def<autotune_msm> {
public:
    // used in tracing macros
    PSC_SM_TRACE_MACHINE_DECLARATION( "Autotune" );
    // top level states
    struct InitialState : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Initial State" );
    };

    struct InitializingPlugin : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Initializing Plugin" );
    };

    struct StartingTuningStep : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Starting Tuning Step" );
    };

    struct PerformingAnalysis : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Performing Analysis" );
    };

    struct CreatingScenarios : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Creating Scenarios" );
    };

    struct PreparingScenarios : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Preparing Scenarios" );
    };

    struct DefiningExperiments : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Defining Experiments" );
    };

    struct ConsideringApplicationRestart : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Checking if application restart needed" );
    };

    struct RunningPhaseExperiments : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Running Phase Experiments" );
    };

    struct FinishingTuningStep : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Finishing Tuning Step" );
    };

    struct CreatingTuningAdvice : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Creating Tuning Advice" );
    };

    struct FinalState : public msm::front::state<> {
        PSC_SM_TRACE_SET_STATE_TRACING( "Final State" );
    };

    void initialize_plugin_action( initialize_plugin const& evt );

    void start_tuning_step_action( start_tuning_step const& evt );

    void perform_analysis_action( perform_analysis const& evt );

    void create_scenarios_action( create_scenarios const& evt );

    void prepare_scenarios_action( prepare_scenarios const& evt );

    void define_experiment_action( define_experiment const& evt );

    void consider_restart_action( consider_restart const& evt );

    void run_phase_experiments_action( run_phase_experiments const& evt );

    void finish_tuning_step_action( finish_tuning_step const& evt );

    void create_tuning_advice_action( create_tuning_advice const& evt );

    void finalize_autotuning_action( finalize_autotuning const& evt );

    template <class Fsm, class Event>
    void no_transition( Event const& e,
                        Fsm&,
                        int          state ) {
        psc_errmsg( "---- AUTOTUNE STATEMACHINE ---- ; "
                    "ERROR: Invalid event \"%s\" received in state: %s\n",
                    e.toString(), PSC_SM_TRACE_SINGLETON->get_current_state() );
        PSC_SM_TRACE_SINGLETON->print();
        exit( EXIT_FAILURE );
    }

    // this definition is needed by the state machine library and will be used as the initial state
    typedef InitialState initial_state;

    // transition table for autotune_statemachine
    typedef autotune_msm at_msm;
    struct transition_table : boost::mpl::vector<
            //      Start State                    Command                Next State                     Action                             Guard
            a_row < InitialState,                  initialize_plugin,     InitializingPlugin,            & at_msm::initialize_plugin_action     >,
            a_row < InitializingPlugin,            start_tuning_step,     StartingTuningStep,            & at_msm::start_tuning_step_action     >,
            // if there is no analysis to be made, create scenarios
            a_row < StartingTuningStep,            create_scenarios,      CreatingScenarios,             & at_msm::create_scenarios_action      >,
            // if there is an analysis to be made, perform it
            a_row < StartingTuningStep,            perform_analysis,      PerformingAnalysis,            & at_msm::perform_analysis_action      >,
            // after an analysis, follow with create scenarios
            a_row < PerformingAnalysis,            create_scenarios,      CreatingScenarios,             & at_msm::create_scenarios_action      >,
            a_row < CreatingScenarios,             prepare_scenarios,     PreparingScenarios,            & at_msm::prepare_scenarios_action     >,
            a_row < CreatingScenarios,             finish_tuning_step,    FinishingTuningStep,           & at_msm::finish_tuning_step_action    >,
            a_row < PreparingScenarios,            define_experiment,     DefiningExperiments,           & at_msm::define_experiment_action     >,
            a_row < DefiningExperiments,           consider_restart,      ConsideringApplicationRestart, & at_msm::consider_restart_action      >,
            a_row < ConsideringApplicationRestart, run_phase_experiments, RunningPhaseExperiments,       & at_msm::run_phase_experiments_action >,
            // if there was a restart in the run_phase_experiments_action, then we need to re-run this step
            a_row < RunningPhaseExperiments,       run_phase_experiments, RunningPhaseExperiments,       & at_msm::run_phase_experiments_action >,
            // jump to DefiningExperiments if prepared scenario pool not empty
            a_row < RunningPhaseExperiments,       define_experiment,     DefiningExperiments,           & at_msm::define_experiment_action     >,
            // jump to PreparingScenarios if scenario pool not empty
            a_row < RunningPhaseExperiments,       prepare_scenarios,     PreparingScenarios,            & at_msm::prepare_scenarios_action     >,
            // jump to CreatingScenarios if search not done
            a_row < RunningPhaseExperiments,       create_scenarios,      CreatingScenarios,             & at_msm::create_scenarios_action      >,
            // continue to FinishingTuningStep if search is done
            a_row < RunningPhaseExperiments,       finish_tuning_step,    FinishingTuningStep,           & at_msm::finish_tuning_step_action    >,
            // jump to StartingTuningStep if plugin not done
            a_row < FinishingTuningStep,           start_tuning_step,     StartingTuningStep,            & at_msm::start_tuning_step_action     >,
            // continue to CreatingTuningAdvice if plugin is done
            a_row < FinishingTuningStep,           create_tuning_advice,  CreatingTuningAdvice,          & at_msm::create_tuning_advice_action  >,
            a_row < CreatingTuningAdvice,          finalize_autotuning,   FinalState,                    & at_msm::finalize_autotuning_action   >
            > { };

private:
    /** Starts the agent network, the application processes and gathers the region information. */
    void start_application_and_agent_network( ACE_Reactor& reactor );
};
typedef msm::back::state_machine<autotune_msm> autotune_statemachine;
}

#endif
