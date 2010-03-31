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
 @internalTechnology
*/

#ifndef MTPDEVICEDPCONST_H
#define MTPDEVICEDPCONST_H

#include <mtp/mtpprotocolconstants.h>

/**
define all the operations that are supported by the device data provider
*/
static const TUint16 KMTPDeviceDpSupportedOperations[] = 
	{
	EMTPOpCodeGetDeviceInfo,
	EMTPOpCodeOpenSession,
	EMTPOpCodeCloseSession,
	EMTPOpCodeGetStorageIDs,
	EMTPOpCodeGetStorageInfo,
	EMTPOpCodeGetNumObjects,
	EMTPOpCodeGetObjectHandles,
	EMTPOpCodeResetDevice,
	EMTPOpCodeGetDevicePropDesc,
	EMTPOpCodeGetDevicePropValue,
	EMTPOpCodeSetDevicePropValue,
	EMTPOpCodeCopyObject,
	EMTPOpCodeMoveObject,
	EMTPOpCodeDeleteObject,
	EMTPOpCodeGetObject,
	EMTPOpCodeGetObjectInfo,
	EMTPOpCodeGetObjectPropDesc,
	EMTPOpCodeGetObjectPropList,
	EMTPOpCodeGetObjectPropsSupported,
	EMTPOpCodeGetObjectPropValue,
	EMTPOpCodeGetObjectReferences,
	EMTPOpCodeGetInterdependentPropDesc,
	EMTPOpCodeSendObject,
	EMTPOpCodeSendObjectInfo,
	EMTPOpCodeSendObjectPropList,
	EMTPOpCodeSetObjectPropList,
	EMTPOpCodeSetObjectPropValue,
	EMTPOpCodeSetObjectReferences,
	EMTPOpCodeGetServiceIDs,
	EMTPOpCodeGetFormatCapabilities
	};

/**
define all the device properties supported by the device data provider
*/
static const TUint16 KMTPDeviceDpSupportedProperties[] = 
	{
	//PTP
	EMTPDevicePropCodeBatteryLevel,
	
	//MTP specific
	EMTPDevicePropCodeSynchronizationPartner,
	EMTPDevicePropCodeDeviceFriendlyName,
	EMTPDevicePropCodeSupportedFormatsOrdered,
	EMTPDevicePropCodeSessionInitiatorVersionInfo,
	EMTPDevicePropCodePerceivedDeviceType,	
	EMTPDevicePropCodeDateTime,
	EMTPDevicePropCodeDeviceIcon,
	
	//MTP Device Service Extension
	EMTPDevicePropCodeFunctionalID,
	EMTPDevicePropCodeModelID,
	EMTPDevicePropCodeUseDeviceStage
	
	};

static const TUint16 KMTPDeviceDpSupportedServiceExtnProperties[] = 
	{
	//MTP Device Service Extension
	EMTPDevicePropCodeFunctionalID,
	EMTPDevicePropCodeModelID,
	EMTPDevicePropCodeUseDeviceStage
	};

/**
define all the object properties supported by the device data provider
*/

static const TUint16 KMTPDeviceDpSupportedObjectProperties[] = 
	{
	//Folder properties
	EMTPObjectPropCodeStorageID,
	EMTPObjectPropCodeObjectFormat,
	EMTPObjectPropCodeProtectionStatus,
	EMTPObjectPropCodeAssociationType,
	EMTPObjectPropCodeAssociationDesc,
	EMTPObjectPropCodeObjectSize,
	EMTPObjectPropCodeObjectFileName,
	EMTPObjectPropCodeDateModified,
	EMTPObjectPropCodeParentObject,
	EMTPObjectPropCodePersistentUniqueObjectIdentifier,
	EMTPObjectPropCodeName,
	EMTPObjectPropCodeNonConsumable	
	};	

static const TUint16 KMTPDeviceDpSupportedEvents[] = 
	{
	//PTP
	EMTPEventCodeStoreAdded,
	EMTPEventCodeStoreRemoved,
	EMTPEventCodeDevicePropChanged,
	EMTPEventCodeDeviceInfoChanged,
	EMTPEventCodeStoreFull,
	EMTPEventCodeDeviceReset,
	EMTPEventCodeStorageInfoChanged,
	};

const TInt32    KMTPDevicePropertyStoreVersion(0x00000100);
const TUint     KMTPDriveLetterPrefixLength(3);
const TUint     KMTPMaxDevicePropertyLength(255);
const TUint     KMTPSerialNumberLength(32);

/**
 Device property value defaults.
*/
const TInt KMTPDefaultBatteryLevel(0);

/** 
DeviceInfo dataset field defaults.
*/
_LIT(KMTPDefaultSerialNumber, "350149101482769");
_LIT(KMTPDefaultManufacturer, "Symbian");
_LIT(KMTPDefaultModel, "1234567890");

/**
 *This enumerator to identify resource file prameters.
 * EDevDpFormats is to identify FORMATS
 * EDevDpExtnUids is to identify UIDs for extn plugin.
 */
enum TDevDPConfigRSSParams
{
	EDevDpFormats  = 0,
	EDevDpExtnUids = 1,
		
};
	
#endif // MTPDEVICEDPCONST_H

