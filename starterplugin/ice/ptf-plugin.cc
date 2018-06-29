#include <frontend.h>

static const char* openmp_env_vars = "OpenMPTP";
static const char* job_startup_cmd = "srun_ps -n";

extern "C" {

std::string StarterPlugin_appStartCmd(char* user_specified_environment)
{
    int processes = fe->get_mpinumprocs();
    RegistryService* regsrv = fe->get_registry();

    std::stringstream appstart_env_vars;
    std::stringstream appstart_cmd_stream;

    appstart_env_vars <<  "export SCOREP_ONLINEACCESS_ENABLE=true; export SCOREP_ENABLE_TRACING=false; export SCOREP_ONLINEACCESS_REG_PORT="
                      <<  regsrv->get_regport()
                      <<  "; export SCOREP_ONLINEACCESS_REG_HOST="  << regsrv->get_reghost()
                      <<  "; export SCOREP_ONLINEACCESS_APPL_NAME="  << fe->get_appname()
                      <<  "; export SCOREP_TUNING_PLUGINS="  <<  openmp_env_vars
                      <<  "; " << user_specified_environment;

    // this forces Score-P to use clock_gettime() for time measurements on the SuperMUC; it is a necessary
    // workaround right now, because the online access interfaces handles other timers incorrectly, not
    // converting the ticks to nanoseconds
    appstart_env_vars  <<  "export SCOREP_TIMER=\"clock_gettime\";";

    // set the environment as requested by the plugins
    appstart_env_vars <<  "export OMP_NUM_THREADS="  <<  fe->get_ompnumthreads()  <<  "; ";

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
