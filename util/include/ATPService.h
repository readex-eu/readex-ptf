#ifndef READEX_ATPSERVER_H
#define READEX_ATPSERVER_H
/**
 * @file ATPService.h
 * @ingroup ATP Service
 * @brief ATP service management and message protocol
 * @author Anamika Chowdhury
 *  Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$
 * This file is part of the Periscope performance measurement tool.
  See http://www.lrr.in.tum.de/periscope for details.

  Copyright (c) 2017, Technische Universitaet Muenchen, Germany
  See the COPYING file in the base directory of the package for details.
 */

#include <unordered_set>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>


#include "regxx.h"
#include "timing.h"
#include "TuningParameter.h"
#include "Variant.h"

typedef enum atp_receive_type {
    ATP_SPECS = 0,
    VALID_CONFIGURATIONS,
    NEARBY_VALID_CONFIGURATIONS
} atp_receive_type; // atp_receive_type

/*typedef struct atp {
    std::string name;
    int* values;
} atp;*/

/*auto custom_deleter = [](void* buf) { free(buf); };*/

class atpService final{

public:
    inline atpService (double g_timedout){
        set_atp_server_timeout (g_timedout);
    };

    /* start ATP server*/
    int startATPServer (RegistryService*);

    /* get ATP specifications */
    //std::unordered_set< TuningParameter* > getATPSpecs ();
    //vector <TuningParameter*> getATPSpecs ();
    std::unordered_map <std::string, std::vector< TuningParameter* > > getATPSpecs ();

    /* get valid configurations */
    //std::unordered_set< std::unique_ptr <Variant>> getValidConfigurations (std::string&);
    std::unordered_map <int, std::vector<int32_t>>  getValidConfigurations (const char*);

    /* get nearbyvalid configurations */
    std::unordered_set< std::unique_ptr <Variant>> getNearbyValidConfigurations (std::string&);

    /*ask for valid configuration*/
    bool isValidConfiguration ();

    std::string getDomainByParamName(std::string);

    std::vector<std::string> getParamNamesbyDomain(std::string);


    inline const int port ( ) const noexcept{
        return port_;
    };

    inline const std::string& app( ) const noexcept{
        return app_;
    };

    inline const std::string host( ) const noexcept{
        return host_;
    };

    inline const std::string& tag( ) const noexcept{
        return tag_;
    };

    inline const int sock( ) const noexcept{
        return sock_;
    };

    inline const int ID ( ) const noexcept{
        return atpServerID;
    };

    inline const int32_t elements_per_combination ( ) const noexcept{
        return elements_per_combination_;
    };

    inline const int hasConstraint ( ) const noexcept{
        return constraint_;
    };

    inline const  std::unordered_set <std::string>& domains( ) const noexcept{
        return domains_;
    };

private:
    int         port_;
    std::string app_;
    std::string host_;
    std::string tag_;
    int         atpServerID;
    int         sock_;
    int         constraint_= 0;
    int32_t     elements_per_combination_;
    std::unordered_set <std::string> domains_;
    std::unordered_map <int,std::string> tp_index_name_mapping_;
    std::unordered_map<std::string, std::vector<std::string>> domain_paramNames_mapping_;
    //std::unordered_map <std::string, std::vector< TuningParameter* > > domain_tp_mapping_;
    //std::unordered_map <int, std::vector<int32_t>> valid_combination_; //<0, <1(tp1),2(tp2)>>, <1,<2,4>>,<2,<1,4>> >
    //std::unordered_map<std::string,std::vector<std::int32_t >> valid_atp_mapping_; //<p1, <1,2,4>>, <p2,<2,4>>....



};

#endif //READEX_ATPSERVER_H
