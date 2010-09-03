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

#include <barsread.h>
#include "cmtpdataproviderconfig.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdataproviderconfigTraces.h"
#endif


/**
CMTPDataProviderConfig factory method.
@param aReader The data provider configuration resource file reader.
@param aFileName The data provider configuration resource file name.
@return A pointer to a new CMTPDataProviderConfig instance. Ownership IS 
transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPDataProviderConfig* CMTPDataProviderConfig::NewL(TResourceReader& aReader, const TDesC& aFileName)
    {
    CMTPDataProviderConfig* self = new (ELeave) CMTPDataProviderConfig();
    CleanupStack::PushL(self);
    self->ConstructL(aReader, aFileName);
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
CMTPDataProviderConfig::~CMTPDataProviderConfig()
    {
    delete iServerName;
    delete iServerImageName;
    delete iResourceFileName;
    }

/**
Indicates if the data provider supporte the specified operational mode.
@param aMode The operational mode.
@return ETrue if the mode is supported, otherwise EFalse.
*/
TBool CMTPDataProviderConfig::ModeSupported(TMTPOperationalMode aMode) const
    {
    return (iSupportedModes & aMode);
    }
    
TUint CMTPDataProviderConfig::UintValue(MMTPDataProviderConfig::TParameter aParam) const
    {
    TUint value(0);
    switch (aParam)
        {
    case EDataProviderType:
        value = iType;
        break;

    case EMajorProtocolRevision:
        value = iVersion;
        break;

    case EOpaqueResource:
        value = iOpaqueResource;
        break;

    case EEnumerationPhase:
        value = iEnumerationPhase;
        break;

    default:
        User::Invariant();
        break;
        }
    return value;
    }
    
TBool CMTPDataProviderConfig::BoolValue(MMTPDataProviderConfig::TParameter aParam) const
    {
    TBool value(EFalse);
    switch (aParam)
        {
    case EObjectEnumerationPersistent:
        value = iObjectEnumerationPersistent;
        break;

    default:
        User::Invariant();
        }
    return value;
    }
    
const TDesC& CMTPDataProviderConfig::DesCValue(MMTPDataProviderConfig::TParameter aParam) const
    {
    const TDesC* value(&KNullDesC);
    switch (aParam)
        {
    case EServerName:
        value = iServerName;
        break;

    case EServerImageName:
        value = iServerImageName;
        break;

    case EResourceFileName:
        value = iResourceFileName;
        break;

    default:
        User::Invariant();
        break;
        }
    return *value;
    }
    
const TDesC8& CMTPDataProviderConfig::DesC8Value(MMTPDataProviderConfig::TParameter /*aParam*/) const
    {
    User::Invariant();
    return KNullDesC8;
    }
    
void CMTPDataProviderConfig::GetArrayValue(MMTPDataProviderConfig::TParameter aParam, RArray<TUint>& aArray) const
    {
    switch (aParam)
        {
    case ESupportedModes:
        {
        if (iSupportedModes & EModePTP)
            {
            aArray.Append(EModePTP);
            }
        if (iSupportedModes & EModeMTP)
            {
            aArray.Append(EModeMTP);
            }
        if (iSupportedModes & EModePictBridge)
            {
            aArray.Append(EModePictBridge);
            }

        break;
        }

    default:
        User::Invariant();
        break;
        }
    }

/**
Constructor
*/
CMTPDataProviderConfig::CMTPDataProviderConfig()
    {

    }

/**
Second phase construction. Reads the data from the reader and configures the instance.
See mtpdpinfo.rh for the definition of the MTP_DATA_PROVIDER resource structure.
@param aReader    Resource reader opened on the corresponding RSC file.
*/
void CMTPDataProviderConfig::ConstructL(TResourceReader& aReader, const TDesC& aFileName)
    {
    iResourceFileName = aFileName.AllocL();
    
    // WORD type
    iType = static_cast<TMTPDataProviderTypes>(aReader.ReadUint16());
    if(KMTPDataProviderTypeECOM != iType)
        {
        OstTrace1( TRACE_ERROR, CMTPDATAPROVIDERCONFIG_CONSTRUCTL, "type %d not MTP Dp ecom plugin", iType);
        User::Leave(KErrCorrupt);
        }

    // BYTE major_version
    iVersion = aReader.ReadInt8();

    // BYTE object_enumeration_persistent
    iObjectEnumerationPersistent = (aReader.ReadInt8() != 0);

    // WORD supported_modes
    iSupportedModes = aReader.ReadUint16();

    // LTEXT server_name
    iServerName = aReader.ReadHBufCL();

    // LTEXT server_image_name
    iServerImageName = aReader.ReadHBufCL();

    // LLINK opaque_resource
    iOpaqueResource = aReader.ReadUint32();

    // BYTE enumeration_phase
    iEnumerationPhase = aReader.ReadUint8();
    
    
    }

