/*
 * Stores the parameter information extracted from the configuration file
 * interval: start-step-end
 * THIS DEFINITIONS SHOULD BE INCLUDED IN autotune_services.h file for consistency WITH
 * THE MANAGEMENT OF MPI-PARAMETERS
 */

enum param_t { unknown, PART_F, NUM_W, NSPEED, NLATENCY, MRECV, WRECV, UNTHR, TSIZE, WFUNCT };

struct MW_TP {
    param_t ptype;
    int     start;
    int     step;
    int     end;
    float   startfloat;
    MW_TP() : start( 0 ), step( 0 ), end( 0 ) {
    }
};
