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

#ifndef MMTPFRAMEWORKCONFIG_H
#define MMTPFRAMEWORKCONFIG_H

#include <e32base.h>

/**
Defines the framework configurability parameter interface. 

The MTP framework implements a number of configurability parameters using a 
central repository. The initial set of configurability parameters are loaded 
from a compiled (binary) initialisation file. Read only access to the 
framework configurability data is available for reference purposes using this 
interface. Write access to these parameters is only available to the MTP 
framework. 
@publishedPartner
@released
*/
class MMTPFrameworkConfig
    {
public:

    /**
    Defines the framework configurability parameters.
    */
    enum TParameter
        {
	    /**
	    This parameter defines the Device Friendly Name MTP device property 
	    value. It is implemented as a string and has a maximum length of 254 
	    characters. The parameter value specified in the initialisation file 
	    may be overwritten in the course of MTP operational processing, if an 
	    MTP SetDevicePropValue specifying the Device Friendly Name device 
	    property is received.
	    
	    This parameter value can be retrieved as either @see TDes or @see HBufC
	    values.
	    */        
        EDeviceFriendlyName                 = 0x00000000,
        
	    /**
	    This parameter defines the Synchronization Partner MTP device property 
	    value. It is implemented as a string and has a maximum length of 254 
	    characters. The parameter value specified in the initialisation file 
	    may be overwritten in the course of MTP operational processing, if an 
	    MTP SetDevicePropValue specifying the Synchronization Partner device 
	    property is received.
	    
	    This parameter value can be retrieved as either @see TDes or @see HBufC
	    values.
	    */        
        ESynchronizationPartnerName         = 0x00000001,

	    /**
	    This parameter is reserved for future use.
	    */        
        ERamDriveMetadataStorageEnable      = 0x00000002,

	    /**
	    This parameter indicates if the framework will allocate MTP logical 
	    storage IDs. If set to ETrue, then the framework will allocates a
	    single logical storage ID on each of the available Symbian OS file 
	    system drives as they become available. If set to EFalse then the 
	    framework will not allocate any MTP logical storage IDs, in which
	    case the active data providers are responsible for doing so using 
	    the @see MMTPStorageMgr interface.
	    
	    This parameter value can be retrieved as an @see TBool value.
	    */        
        ELogicalStorageIdsAllocationEnable  = 0x00000003,

	    /**
	    This parameter specifies the Symbian OS file system drive which acts as
	    the default MTP storage ID (0x00000000). It is implemented as an 
	    integer value which represents the default drive using a zero based 
	    numbering scheme (i.e. 0 represents the A drive, 25 represents the Z 
	    drive etc.) or a MTP storage ID(assigned by the MTP framework).
	    
	    This parameter value can be retrieved as an @see TUint value.
	    */        
        EDefaultStorageDrive                = 0x00000004,

	    /**
	    This parameter specifies the device's default MTP Object Format Code.
	    This format is used when processing MTP operation which specify a 
	    default (0x00000000) ObjectFormatCode parameter (e.g. InitiateCapture, 
	    InitiateOpenCapture).
	    
	    This parameter value can be retrieved as an @see TUint value.
	    */        
        EDefaultObjectFormatCode            = 0x00000005,
        
        /**
        UID of the Stub SIS File
        This is used by the framework while loading the dataprovider.
        This parameter value can be retrieved as an @see TUint value.
        */         
        EPackageStubUID                     = 0x00000006,
		/**
		Implimentation UID of transport PlugIn with high priority.
		This is used when ETransportSwitchEnabled is enabled.when 
		MTP client try to start a transport protocol while another 
		one already exist, if the new one is equal to this parameter 
		value, the previous protocol plugin will be stopped and the 
		new transport protocol plugin will be started.
		 
		This value can be retrieved as an @TDes value.
	    */                
        ETransportHighPriorityUID           = 0x00000007,

        /**
        Transport protocol implementation switch enable flag.
        If set to ETrue, the switch is allowed;if set to EFalse,
        the switch is not allowed. 
		Refer to ETransportHighPriorityUID for more details.
		
		This value can be retrieved as an @see TBool value.
        */
        ETransportSwitchEnabled             = 0x00000008,
        
        /*
         * Reserved by MTP framework.
         * This parameter specifies the device default FunctionalID(128 bits, GUID),
         */
        EDeviceDefaultFuncationalID                 = 0x00000010,
        
        /*
         * Reserved by MTP framework.
         * This parameter specifies the device current FunctionalID(128 bits, GUID),
         */
        EDeviceCurrentFuncationalID                 = 0x00000011,
        
        /*
         * Reserved by MTP framework.
         * This parameter specifies the device default ModelID(128 bits, GUID),
         */
        EDeviceDefaultModelID                       = 0x00000012,
        
        /*
         * Reserved by MTP framework.
         * This parameter specifies the device current ModelID(128 bits, GUID),
         */
        EDeviceCurrentModelID                       = 0x00000013,
   
        /*
         * Reserved by MTP framework. 
         * Indicate whether the MTP server has crashed or been abnormally down or not.
         * This parameter value can be retrieved as an @see TBool value.
         */
        EAbnormalDown								= 0x00000014,
        
	    /**
	    This parameter specifies the Symbian OS file system drives which are 
	    excluded from the set of MTP storages which are managed by the MTP 
	    framework. Any drive which is identified in this set will not be 
	    assigned a corresponding MTP storage ID value by the MTP framework. 
	    This parameter is implemented as an array of integer values each of 
	    which represent an excluded drive using a zero based numbering scheme 
	    (i.e. 0 represents the A drive, 25 represents the Z drive etc.)
	    
	    This parameter value can be retrieved as an @see RArray<TUint> value.
	    */        
        EExcludedStorageDrives             = 0x00010000
        };
		
public:

    /**
    Provides the value of a descriptor configurability parameter. This should
    be used when the length of the parameter value is known not to exceed the 
    capacity of the parameter value buffer. If the maximum parameter length is 
    not known then the @see ValueL method should be used.
    @param aParam The identifier of the parameter value to be retrieved
    @param aValue The parameter value buffer to be filled. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetValueL(TParameter aParam, TDes& aValue) const = 0;
    
    /**
    Provides the value of a descriptor configurability parameter. This should 
    be used when the maximum length of the parameter value is not known.
    @param aParam The identifier of the parameter value to be retrieved.
    @return A pointer to a heap descriptor containing the parameter value. 
    Ownership IS transferred.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual HBufC* ValueL(TParameter aParam) const = 0;

    /**
    Provides the value of an unsigned integer configurability parameter.
    @param aParam The identifier of the parameter value to be retrieved
    @param aValue The parameter value buffer to be filled. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetValueL(TParameter aParam, TUint& aValue) const = 0;

    /**
    Provides the value of a boolean configurability parameter.
    @param aParam The identifier of the parameter value to be retrieved
    @param aValue The parameter value buffer to be filled. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetValueL(TParameter aParam, TBool& aValue) const = 0;

    /**
    Provides the value of an unsigned integer array configurabilitt parameter.
    @param aParam The identifier of the parameter value to be retrieved
    @param aValue The parameter value buffer to be filled. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetValueL(TParameter aParam, RArray<TUint>& aArray) const = 0;
    };
    
#endif // MMTPFRAMEWORKCONFIG_H
