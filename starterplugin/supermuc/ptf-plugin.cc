#include <frontend.h>

static const char* enopt_metric_plugin         = "EnoptMP";
static const char* enopt_metric_plugin_env_var = "ENOPTMP";
static const char* tuning_plugins_sep          = ",";
static const char* enopt_tuning_plugin         = "EnoptTP,";
static const char* openmp_tuning_plugin        = "OpenMPTP";
static const char* job_startup_cmd             = "mpiexec -n";

extern "C" {

std::string StarterPlugin_appStartCmd(char* user_specified_environment)
{
    int processes = fe->get_mpinumprocs();
    RegistryService* regsrv = fe->get_registry();

    std::stringstream appstart_env_vars;
    std::stringstream appstart_cmd_stream;

    // add the necessary Score-P environment variables
    appstart_env_vars <<  "export SCOREP_ONLINEACCESS_ENABLE=true; export SCOREP_ENABLE_TRACING=false; export SCOREP_ONLINEACCESS_REG_PORT="
                      <<  regsrv->get_regport()
                      <<  "; export SCOREP_ONLINEACCESS_REG_HOST="  <<  regsrv->get_reghost()
                      <<  "; export SCOREP_ONLINEACCESS_APPL_NAME="  <<  fe->get_appname()
                      <<  "; export SCOREP_METRIC_PLUGINS="  <<  enopt_metric_plugin
                      <<  "; export SCOREP_METRIC_"  <<  enopt_metric_plugin_env_var << "=\"\""
                      <<  "; export SCOREP_TUNING_PLUGINS_SEP="  <<  tuning_plugins_sep
                      <<  "; export SCOREP_TUNING_PLUGINS="  <<  enopt_tuning_plugin  <<  openmp_tuning_plugin;

    // this forces Score-P to use clock_gettime() for time measurements on the SuperMUC; it is a necessary
    // workaround right now, because the online access interfaces handles other timers incorrectly, not
    // converting the ticks to nanoseconds
    appstart_env_vars  <<  "; export SCOREP_TIMER=\"clock_gettime\"";

    // set up the environment as requested by the user
    appstart_env_vars  <<  "; "  <<  user_specified_environment;

    // set up the environment as requested by the command line options
    appstart_env_vars  <<  "export OMP_NUM_THREADS="  <<  fe->get_ompnumthreads()  <<  "; ";

    // build and return the final command
    appstart_cmd_stream  <<  appstart_env_vars.rdbuf()  <<  " "  <<  job_startup_cmd
                         <<  " "  <<  processes  <<  " "  <<  opts.app_run_string;

    return appstart_cmd_stream.str();
}

// Optional. The behaviour is equivalent.
std::string StarterPlugin_agentStartPrefix(char* agentHost)
{
    std::stringstream pre_command;
    const char*       cwd = getenv( "PWD" );

    if( opts.has_force_localhost ) {
        pre_command <<  "cd "  <<  cwd  <<  ";";
    } else {
        pre_command <<  "ssh "  <<  agentHost  <<  " \"cd "  <<  cwd  <<  "\";";
    }

    // Obtain a backtrace in case one of the agents crashes.
    pre_command << " gdb --nx --batch -ex run -ex bt -ex continue -ex quit --args ";

    return pre_command.str();
}

}
