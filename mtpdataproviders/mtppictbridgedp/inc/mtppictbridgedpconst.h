// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
 */

#ifndef MTPPICTBRIDGEDPCONST_H
#define MTPPICTBRIDGEDPCONST_H

#include <mtp/mtpprotocolconstants.h>
#include <comms-infras/commsdebugutility.h>
#include "mtpdpconst.h"


_LIT(KDeviceDiscovery, "DDISCVRY.DPS"); 
_LIT(KHostDiscovery, "HDISCVRY.DPS");
_LIT(KHostRequest, "HREQUEST.DPS");
_LIT(KHostResponse, "HRSPONSE.DPS");


static const TUint KDiscoveryTime = 15 * 1000 * 1000; //15s 
static const TUint KSendTimeout = 15* 1000 * 1000; // 15s


/**
define all the operations that are supported by the pictbridge data provider
*/
static const TUint16 KMTPPictBridgeDpSupportedOperations[] = 
    {
    EMTPOpCodeGetObjectInfo,
//    EMTPOpCodeGetObjectPropList,
    EMTPOpCodeGetObject,
//    EMTPOpCodeSetObjectPropList,
    EMTPOpCodeSendObjectInfo,
//    EMTPOpCodeSendObjectPropList,
    EMTPOpCodeSendObject
//    EMTPOpCodeGetObjectPropsSupported,
//    EMTPOpCodeGetObjectPropDesc,
//    EMTPOpCodeGetFormatCapabilities,
//    EMTPOpCodeDeleteObject
    };  

/**
define all the object properties that are supported by the pictbridge data provider
*/
static const TUint16 KMTPPictBridgeDpSupportedProperties[] = 
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

static const TUint32 KPtpNoValue = 0x00;
static const TInt KMTPProcessLimit = 1000; // how many entries processed before completing the ao
static const TInt KExtensionBufferSize=5; //related to the size of strings in SMTPValidCodeExtensionMapping

// extensions of image files that are shared via MTP PictBridge Data Provider

struct SMTPValidCodeExtensionMapping
    {
    public:
    TMTPFormatCode iFormatCode;
    const TText*   iExtension;
    };

// Note when adding extensions: 
// same value for iExtension can be in the file only once, 
// values for iFormatCode code can be multiple times

#if defined(_UNICODE)
#define CASTING(a) ((const TText*)L ## a)
#else
#define CASTING(a) ((const TText*)(a))
#endif

static const SMTPValidCodeExtensionMapping KMTPValidCodeExtensionMappings[] =
    {
        {EMTPFormatCodeScript, CASTING("dps")} // dps script
    };

#endif // MTPPICTBRIDGEDPCONST_H

