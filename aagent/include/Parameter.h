#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <string>
#include <stdint.h>

enum ParameterNodeType {
    NODE_PARAMETER_STRING,
    NODE_PARAMETER_INTEGER,
    NODE_PARAMETER_UNSIGNEDINT
};

/**
 * Identifies a user parameter in the application.
 */

struct Parameter_t {
    std::string       param_name;
    std::string       param_value;
    ParameterNodeType param_type;
    //uint64_t    aa_parameter_id;
};


#endif /* PARAMETER_H_ */
