#ifndef PLUGIN_MANAGEMENT_H_
#define PLUGIN_MANAGEMENT_H_

#include <string>

//! [pluginmanagement_h]
extern "C" {
IPlugin* getPluginInstance( void );
int getVersionMajor( void );
int getVersionMinor( void );
string getName( void );
string getShortSummary( void );
}
//! [pluginmanagement_h]

#endif
