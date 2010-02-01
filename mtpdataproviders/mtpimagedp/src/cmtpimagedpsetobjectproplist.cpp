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
 @internalTechnology
*/

#include <mtp/tmtptyperequest.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypeobjectproplist.h>

#include "cmtpimagedpsetobjectproplist.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "cmtpimagedp.h"
#include "mtpimagedputilits.h"

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDpSetObjectPropList");)

MMTPRequestProcessor* CMTPImageDpSetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpSetObjectPropList* self = new (ELeave) CMTPImageDpSetObjectPropList(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
CMTPImageDpSetObjectPropList::~CMTPImageDpSetObjectPropList()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::~CMTPImageDpSetObjectPropList"));
    delete iPropertyList;
    delete iObjectMeta;
    __FLOG(_L8("<< CMTPImageDpSetObjectPropList::~CMTPImageDpSetObjectPropList"));
    __FLOG_CLOSE;
    }
    
CMTPImageDpSetObjectPropList::CMTPImageDpSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iDataProvider(aDataProvider),
    iPropertyMgr(aDataProvider.PropertyMgr())		
    {
    
    }
    
void CMTPImageDpSetObjectPropList::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::ConstructL"));
    iPropertyList = CMTPTypeObjectPropList::NewL();
    iObjectMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpSetObjectPropList::ConstructL"));
    }

void CMTPImageDpSetObjectPropList::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::ConstructL"));
    ReceiveDataL(*iPropertyList);
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::ConstructL"));
    }

TBool CMTPImageDpSetObjectPropList::DoHandleResponsePhaseL()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::DoHandleResponsePhaseL"));
    MMTPObjectMgr& objects(iFramework.ObjectMgr());
    TUint32 parameter(0);
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    const TUint count(iPropertyList->NumberOfElements());
    iPropertyList->ResetCursor();
    __FLOG_VA((_L8("setting %d properties"), count));
    TUint32 preHandle = KMTPHandleNone;
    for (TUint i(0); ((i < count) && (responseCode == EMTPRespCodeOK)); i++)
        {
        CMTPTypeObjectPropListElement& element=iPropertyList->GetNextElementL(); 
        TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);
        TUint16 propertyCode = element.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode);
        TUint16 dataType = element.Uint16L(CMTPTypeObjectPropListElement::EDatatype);
        __FLOG_VA((_L8("set property, propertycode %d, datatype %d, handle %d"), propertyCode, dataType, handle));
        
        responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, handle, *iObjectMeta);
        if ((EMTPRespCodeOK == responseCode) && (iObjectMeta->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId()))
            {
            // Object is owned by the FileDp
            responseCode = CheckPropCode(propertyCode, dataType);
            if (responseCode == EMTPRespCodeOK)
                {
                if(preHandle != handle)
                    {
                    iPropertyMgr.SetCurrentObjectL(*iObjectMeta, ETrue);
                    }
                
                switch(propertyCode)
                    {
                    case EMTPObjectPropCodeObjectFileName:
                    case EMTPObjectPropCodeName:
                    case EMTPObjectPropCodeDateModified:
                        iPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propertyCode), element.StringL(CMTPTypeObjectPropListElement::EValue));
                        objects.ModifyObjectL(*iObjectMeta);
                        break;
                    case EMTPObjectPropCodeNonConsumable:
                        iPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propertyCode), element.Uint8L(CMTPTypeObjectPropListElement::EValue));
                        objects.ModifyObjectL(*iObjectMeta);
                        break;                        
                    default:
                        responseCode = EMTPRespCodeInvalidObjectPropCode;
                        break;
                    }
                
                }
            if (responseCode != EMTPRespCodeOK)
                {
                // Return the index of the failed property in the response.
                parameter = i;
                }
            }
        preHandle = handle;
        }

    SendResponseL(responseCode, 1, &parameter);
    __FLOG(_L8("<< CMTPImageDpSetObjectPropList::DoHandleResponsePhaseL"));
    return EFalse;
    }

TBool CMTPImageDpSetObjectPropList::HasDataphase() const
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::HasDataphase"));
    return ETrue;
    }

TMTPResponseCode CMTPImageDpSetObjectPropList::CheckPropCode(TUint16 aPropertyCode, TUint16 aDataType) const
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectPropList::CheckPropCode"));
    TMTPResponseCode responseCode = EMTPRespCodeOK;
    switch(aPropertyCode)
        {
        case EMTPObjectPropCodeStorageID:
        case EMTPObjectPropCodeObjectFormat:
        case EMTPObjectPropCodeObjectSize:		
        case EMTPObjectPropCodeParentObject:
        case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
        case EMTPObjectPropCodeProtectionStatus:
        case EMTPObjectPropCodeWidth:
        case EMTPObjectPropCodeHeight:
        case EMTPObjectPropCodeImageBitDepth:
        case EMTPObjectPropCodeRepresentativeSampleFormat:
        case EMTPObjectPropCodeRepresentativeSampleSize:
        case EMTPObjectPropCodeRepresentativeSampleHeight:
        case EMTPObjectPropCodeRepresentativeSampleWidth:
        case EMTPObjectPropCodeDateCreated:
            responseCode = 	EMTPRespCodeAccessDenied;
            break;
                            
        case EMTPObjectPropCodeObjectFileName:	
        case EMTPObjectPropCodeName:
        case EMTPObjectPropCodeDateModified:
            if (aDataType != EMTPTypeString)
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            break;
        case EMTPObjectPropCodeNonConsumable:
            if (aDataType != EMTPTypeUINT8)
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            break;            
        default:
            responseCode = EMTPRespCodeInvalidObjectPropCode;
        }
    __FLOG(_L8("<< CMTPImageDpSetObjectPropList::CheckPropCode"));
    return responseCode;
    }
    

