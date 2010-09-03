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
// mmtpfiledpconst.h
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __MTPFILEDPCONST_H__
#define __MTPFILEDPCONST_H__

#include <mtp/mtpprotocolconstants.h>


/**
define all the operations that are supported by the file data provider
@internalTechnology

*/
static const TUint16 KMTPFileDpSupportedOperations[] = 
	{
	EMTPOpCodeGetObjectInfo,
	EMTPOpCodeGetObject,
	EMTPOpCodeDeleteObject,
	EMTPOpCodeSendObjectInfo,
	EMTPOpCodeSendObject,
	EMTPOpCodeMoveObject,
	EMTPOpCodeCopyObject,
	EMTPOpCodeGetPartialObject,
		
	//MTP op codes
	EMTPOpCodeGetObjectPropsSupported,
	EMTPOpCodeGetObjectPropDesc,
	EMTPOpCodeGetObjectPropValue,
	EMTPOpCodeSetObjectPropValue,
	EMTPOpCodeGetObjectReferences,
	EMTPOpCodeSetObjectReferences,
	//EMTPOpCodeSkip,
	
	//MTP enhanced operation codes
	EMTPOpCodeGetObjectPropList,
	EMTPOpCodeSetObjectPropList,
	EMTPOpCodeSendObjectPropList,
	EMTPOpCodeGetFormatCapabilities,
	EMTPOpCodeSetObjectProtection
	};	

/**
define all the object properties that are supported by the file data provider
@internalTechnology

*/
static const TUint16 KMTPFileDpSupportedProperties[] =
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
	EMTPObjectPropCodeNonConsumable,
	EMTPObjectPropCodeHidden
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
	
#endif // __MTPFILEDPCONST_H__

