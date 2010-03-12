// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef MTPDATAPROVIDERAPITYPES_H
#define MTPDATAPROVIDERAPITYPES_H

#include <mtp/mtpdataproviderconfig.hrh>

class MMTPConnection;

/** 
Defines the various data provider type categories.
@publishedPartner
@released
*/
enum TMTPDataProviderTypes
    {
    /**
    ECOM plug-in.
    */
    EEcom   = KMTPDataProviderTypeECOM
    };

/** 
Defines the MTP framework operational modes.
@publishedPartner
@released
*/
enum TMTPOperationalMode
    {
    /**
    Undefined mode.
    */
    EModeUnknown    = KMTPModeUnknown,
    
    /**
    MTP Mode.
    */
    EModeMTP        = KMTPModeMTP,
    
    /**
    PTP Mode.
    */
    EModePTP        = KMTPModePTP,
    
    /**
    PictBridge Mode.
    */
    EModePictBridge = KMTPModePictBridge,
    
	};

/** 
Defines the MTP framework event notification types.
@publishedPartner
@released
*/
enum TMTPNotification
    {
    /**
    This notification indicates that the operational mode of the MTP data 
    provider framework has changed. Details of the mode change are provided in
    the accompanying @see TMTPNotificationParamsSessionChange notification 
    parameter block.
    */
    EMTPModeChanged,
    
    /**
    This notification indicates that an active MTP session has become inactive. 
    Details of the session are provided in the accompanying 
    @see TMTPNotificationParamsSessionChange notification parameter block.
    */
    EMTPSessionClosed,

    /**
    This notification indicates that a new MTP session has become active. 
    Details of the session are provided in the accompanying 
    @see TMTPNotificationParamsSessionChange notification parameter block.
    */
    EMTPSessionOpened,

    /**
    This notification indicates that a new MTP storage has become available as
    a result of the insertion of removable storage media. 
    Details of the storage are provided in the accompanying 
    @see TMTPNotificationParamsStorageChange notification parameter block.
    */
    EMTPStorageAdded,

    /**
    This notification indicates that an MTP storage has become unavailable as
    a result of the removal of removable storage media. 
    Details of the storage are provided in the accompanying 
    @see TMTPNotificationParamsStorageChange notification parameter block.
    */
    EMTPStorageRemoved,
	
	/**
    This notification indicates that an MTP storage has become unavailable as
    a result of the usb cable removed. 
    Details of the storage are provided in the accompanying     
    */
	EMTPDisconnected,
	
	/**
	This notification indicates that an MTP Object has been Renamed.
	*/
	EMTPRenameObject,
	
	EMTPObjectAdded
    };

/** 
Defines the MTP framework event notification types.
This notification is from dp to framework
@publishedPartner
@released
*/
enum TMTPNotificationToFramework
    {
    EMTPAddFolder = 0x10000000,
    };

/**
Defines the notification parameter block which accompanies @see EMTPModeChanged 
notifications.
*/
class TMTPNotificationParamsModeChange
    {
public:

    /**
    The new MTP operational mode TO which the MTP framework is 
    transitioning.
    */
    TMTPOperationalMode    iNewMode;
    
    /**
    The previous MTP operational mode FROM which the MTP framework is 
    transitioning.
    */
    TMTPOperationalMode    iPreviousMode;
    };

/**
Defines the notification parameter block which accompanies @see ESessionOpened 
and @see ESessionClosed notifications.
*/
class TMTPNotificationParamsSessionChange
    {
public:

    /**
    The MTP connection assigned identifier of the session.
    */
    TUint           iMTPId;

    /**
    The MTP connection on which the exists.
    */
    MMTPConnection& iConnection;
    };

/**
Defines the notification parameter block which accompanies @see EStorageAdded 
and @see EStorageRemoved notifications.
*/
class TMTPNotificationParamsStorageChange
    {
public:

    /**
    The MTP StorageID assigned to the storage.
    */
    TUint32 iStorageId;
    };

/**
Defines the notification parameter block which accompanies @see EMTPAddFolder 
notification
*/
class TMTPNotificationParamsFolderChange
    {
public:

    /**
    The Folder on which need to do action.
    */
    const TDesC& iFolderChanged;
    };

/** 
Defines the various data provider MTP feature support categories.
@publishedPartner
@released
*/
enum TMTPSupportCategory
    {
    /**
    MTP association type datacodes.
    */
    EAssociationTypes       = 0,
    
    /**
    MTP device property identifier datacodes.
    */
    EDeviceProperties       = 1,
    
    /**
    MTP event identifier datacodes.
    */
    EEvents                 = 2,

    /**
    Produced MTP object format identifier datacodes.
    */
    EObjectCaptureFormats   = 3,

    /**
    Consumed MTP object format identifier datacodes.
    */
    EObjectPlaybackFormats  = 4,
    
    /**
    MTP object property identifier datacodes.
    */
    EObjectProperties       = 5,

    /**
    MTP operation identifier datacodes.
    */
    EOperations             = 6,  
    
    /**
    Supported storage systems (@see CMTPStorageMetaData::TStorageSystemType).
    */
    EStorageSystemTypes     = 7,
    
    /**
    MTP extension set identifier strings.
    */
    EVendorExtensionSets    = 8,
    
    /**
    Folder exclusion list strings
     */
    EFolderExclusionSets    = 9,
    
    /**
    Format code and file extension mapping strings
     */
    EFormatExtensionSets    = 10,
    
    /**
    MTP Device Service Extension service identifier strings.
    */
    EServiceIDs    = 11,

    /**
    The number of supported categories.
    */
    ENumCategories
    };

/** 
Defines the MTP operation transaction processing phases.
@publishedPartner
@released
*/
enum TMTPTransactionPhase
{
    /**
    Transaction phase unspecified.
    */
    EUndefined          = 0x00000000,
	
    /**
    No MTP transaction is in-progress.
    */
	EIdlePhase          = 0x00000001,
	
    /**
    MTP request phase.
    */
    ERequestPhase       = 0x00000002,
	
    /**
    MTP data phase (data flow direction is MTP initiator to MTP responder).
    */
    EDataIToRPhase      = 0x00000004,
	
    /**
    MTP data phase (data flow direction is MTP responder to MTP initiator).
    */
    EDataRToIPhase      = 0x00000008,
	
    /**
    MTP response phase.
    */
    EResponsePhase      = 0x00000010,
	
    /**
    Transaction conmpleting phase.
    */
    ECompletingPhase    = 0x00000020
};

class TMTPNotificationParamsHandle
{
public:

/**
The MTP framework assigned Object Handle in the session.
*/
TUint32           iHandleId;   
/**
The MTP framework assigned Object FileName in the session.
*/
TFileName         iFileName;

};

#endif // MTPDATAPROVIDERAPITYPES_H
