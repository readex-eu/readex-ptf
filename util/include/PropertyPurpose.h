/**
   @file	PropertyPurpose.h
   @ingroup Util
   @brief   MetaProperty header
   @author	Robert Mijakovic
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef PROPERTY_PURPOSE_
#define PROPERTY_PURPOSE_

typedef enum PropertyPurposeEnum {
    PSC_PROPERTY_PURPOSE_ANALYSIS = 99,
    PSC_PROPERTY_PURPOSE_TUNING   = 100
}PropertyPurpose;

#endif /* PROPERTY_PURPOSE_ */
