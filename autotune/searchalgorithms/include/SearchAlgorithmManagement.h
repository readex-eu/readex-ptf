#ifndef __SEARCHALGORITHM_MANAGEMENT_H_
#define __SEARCHALGORITHM_MANAGEMENT_H_

extern "C" {
ISearchAlgorithm* getSearchAlgorithmInstance( void );

int getVersionMajor( void );

int getVersionMinor( void );

string getName( void );

string getShortSummary( void );
}

#endif
