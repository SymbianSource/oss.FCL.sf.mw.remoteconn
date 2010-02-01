// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef MMTPDATAPROVIDERCONFIG_H
#define MMTPDATAPROVIDERCONFIG_H

#include <e32std.h>

/**
Defines the data provider configurability parameter interface.
@publishedPartner
@released
*/
class MMTPDataProviderConfig
	{
public:

    /**
    Defines the data provider configurability parameters.
    */
    enum TParameter
        {
	    /**
	    This parameter specifies the type of the data provider which in turn 
	    determines how it will be loaded. The set of valid parameter values is 
	    defined by the @see TMTPDataProviderType enumeration.
	    
	    This parameter value can be accessed using the @see UintValue API.
	    */
        EDataProviderType,

	    /**
	    This parameter specifies the highest major MTP protocol revision 
	    supported by the data provider. MTP protocol revision identifiers take 
	    the form N.N, of which the major protocol revision comprises the most 
	    significant digit(s) to the left of the period (.), e.g. a major 
	    protocol version value of 1 would represent all MTP protocol revisions 
	    from 1.0 up to (but not including) 2.0. 
	    
	    This parameter value can be accessed using the @see UintValue API.
	    */
        EMajorProtocolRevision,						
        
	    /**
	    This parameter indicates whether the object enumerations which the data 
	    provider registers with the MTP object manager should persist when the 
	    data provider is unloaded from the MTP framework. If the persistent 
	    option is selected, then on each subsequent activation after its 
	    initial object store enumeration is complete the data provider will be 
	    required to re-enumerate only that portion of its object store which 
	    has changed (objects added, removed, or modified) since it was last 
	    loaded. If the persistent option is not selected, then the data 
	    provider will be required to re-enumerate its entire object store each 
	    time it is loaded. 
	    
	    This parameter value can be accessed using the @see BoolValue API
	    */
        EObjectEnumerationPersistent,
        	
	    /**
	    This parameter specifies the modes of operation which the data provider
	    supports. It is specified as an integer which encodes a set of bit 
	    flags. The set of valid bit flag values is defined by the @see TMTPMode
	    enumeration. 
	    
	    The set of parameter values can be accessed using the 
	    @see GetArrayValue API.
	    */
        ESupportedModes,				

	    /**
	    This parameter is reserved for future use.
	    */
        EServerName,			
        
	    /**
	    This parameter is reserved for future use.
	    */
        EServerImageName,			
        
        /**
        This parameter specifies an optional configuration data resource whose
        form and function is specific to the data provider. 
        
        The parameter value is specified as an LLINK resource and can be 
        accessed using the @see DesC8Value API.
        */
        EOpaqueResource,
        
        /**
        This parameter specifies the resource filename in which the set of data 
        provider configurability parameter values are specified.
	    
	    The set of parameter values can be accessed using the 
	    @see DesCValue API.
        */
        EResourceFileName,

	    /**
	    This parameter specifies the enumeration phase of the data provider	    
	    
	    This parameter value can be accessed using the @see UintValue API.
	    */	    
        EEnumerationPhase
        };

public:

    /**
    Provides the specified @see TBool parameter value.
    @param aParam The parameter value to retrieve.
    @return Returns The parameter value
    @panic USER 0, if the parameter data type is not @see TBool.
    */
    virtual TBool BoolValue(TParameter aParam) const = 0;

    /**
    Provides the specified @see TDesC parameter value.
    @param aParam The parameter value to retrieve.
    @return Returns The parameter value
    @panic USER 0, if the parameter data type is not @see TDesC.
    */
	virtual const TDesC& DesCValue(TParameter aParam) const = 0;

    /**
    Provides the specified @see TDesC8 parameter value.
    @param aParam The parameter value to retrieve.
    @return Returns The parameter value
    @panic USER 0, if the parameter data type is not @see TDesC8.
    */
	virtual const TDesC8& DesC8Value(TParameter aParam) const = 0;

    /**
    Provides the specified @see TUint parameter value.
    @param aParam The parameter value to retrieve.
    @return Returns The parameter value
    @panic USER 0, if the parameter data type is not @see TUint.
    */
	virtual TUint UintValue(TParameter aParam) const = 0;
	
    /**
    Retrieves the specified @see RArray<TUint> parameter value.
    @param aParam The parameter value to retrieve.
    @return Returns The parameter value
    @panic USER 0, if the parameter data type is not @see RArray<TUint>.
    */
    virtual void GetArrayValue(TParameter aParam, RArray<TUint>& aArray) const = 0;
	};

#endif // MMTPDATAPROVIDERCONFIG_H
