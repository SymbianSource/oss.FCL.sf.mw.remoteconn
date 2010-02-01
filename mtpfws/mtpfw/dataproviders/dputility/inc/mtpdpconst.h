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
// mmtpdpconst.h
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef MTPDPCONST_H
#define MTPDPCONST_H

#include <mtp/mtpprotocolconstants.h>

/**
define all the object properties that are supported by the data provider
@internalTechnology
*/
static const TUint16 KMTPDpSupportedProperties[] =
	{
	EMTPObjectPropCodeStorageID,
	EMTPObjectPropCodeObjectFormat,
	EMTPObjectPropCodeProtectionStatus,
	EMTPObjectPropCodeObjectSize,
	EMTPObjectPropCodeObjectFileName,
	EMTPObjectPropCodeDateModified,
	EMTPObjectPropCodeParentObject,
	EMTPObjectPropCodePersistentUniqueObjectIdentifier,
	EMTPObjectPropCodeName,
	EMTPObjectPropCodeNonConsumable	
	};

/**
Define the supported property number .
@internalTechnology
*/
static const TUint16 KMTPDpPropertyNumber = sizeof(KMTPDpSupportedProperties)/sizeof(KMTPDpSupportedProperties[0]); 


/**
Define the supported property group number .
@internalTechnology
*/
static const TUint16 KMTPDpPropertyGroupNumber = 1; 

/**
Define the properties groups supported.
@internalTechnology
*/

static const TUint16 KMTPDpPropertyGroups[KMTPDpPropertyGroupNumber+1][KMTPDpPropertyNumber] =
    {
      	{}, // group 0 is not used.
      	//               group 1 contains 10 proeprties
      	{	EMTPObjectPropCodeStorageID,
			EMTPObjectPropCodeObjectFormat,
			EMTPObjectPropCodeProtectionStatus,
			EMTPObjectPropCodeObjectSize,
			EMTPObjectPropCodeObjectFileName,
			EMTPObjectPropCodeDateModified,
			EMTPObjectPropCodeParentObject,
			EMTPObjectPropCodePersistentUniqueObjectIdentifier,
			EMTPObjectPropCodeName,
			EMTPObjectPropCodeNonConsumable	}  
    };



/**
define the granularity from drive list
@internalTechnology
*/
static const TInt KMTPDriveGranularity = 5;

/**
define the granularity for the exclusion list
@internalTechnology
*/
static const TInt KMTPExclusionListGranularity = 5;

/**
define the drive root path length
@internalTechnology
*/
static const TInt KMTPDriveLetterPrefixLength = 3; //c:\
	
#endif // MTPDPCONST_H
