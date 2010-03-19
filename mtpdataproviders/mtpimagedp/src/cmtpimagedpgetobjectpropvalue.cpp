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

#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeuint8.h>
#include <mtp/tmtptypeuint32.h>
#include <mtp/tmtptypeuint64.h>
#include <mtp/tmtptypeuint128.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpimagedpgetobjectpropvalue.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedp.h"
#include "mtpimagedputilits.h"


__FLOG_STMT(_LIT8(KComponent,"GetObjectPropValue");)

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPImageDpGetObjectPropValue::NewL(
                                            MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetObjectPropValue* self = new (ELeave) CMTPImageDpGetObjectPropValue(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/		
CMTPImageDpGetObjectPropValue::~CMTPImageDpGetObjectPropValue()
    {
    delete iMTPTypeString;	
    delete iMTPTypeArray;
    delete iObjectMeta;
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/		
CMTPImageDpGetObjectPropValue::CMTPImageDpGetObjectPropValue(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    :CMTPRequestProcessor(aFramework, aConnection,0, NULL),
    iObjectPropertyMgr(aDataProvider.PropertyMgr())
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);	
    }

/**
Second-phase construction
*/                  
void CMTPImageDpGetObjectPropValue::ConstructL()
    {
    iMTPTypeString = CMTPTypeString::NewL();
    iMTPTypeArray = CMTPTypeArray::NewL(EMTPTypeAUINT8);
    iObjectMeta = CMTPObjectMetaData::NewL();
    }

/**
* Check format code, prop code and objectg Handle
*/
TMTPResponseCode CMTPImageDpGetObjectPropValue::CheckRequestL()
    {
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);	

    if(responseCode == EMTPRespCodeOK)
        {
        const TInt count = sizeof(KMTPImageDpSupportedProperties) / sizeof(TUint16);
        TInt i = 0;
        for(i = 0; i < count; i++)
        
            if(KMTPImageDpSupportedProperties[i] == propCode)
                {
                break;
                }
            if(i == count)
                {
                responseCode = EMTPRespCodeInvalidObjectPropCode;
                }		
        }
    
    if(responseCode == EMTPRespCodeOK)
        {
        responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
        iObjectPropertyMgr.SetCurrentObjectL(*iObjectMeta, EFalse);
        }	
    
    return responseCode;	
    }
    
/**
GetObjectPropValue request handler
*/		
void CMTPImageDpGetObjectPropValue::ServiceL()
    {
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    switch(propCode)
        {
        case EMTPObjectPropCodeStorageID:
            ServiceStorageIdL();
            break;
        case EMTPObjectPropCodeObjectFormat:
            ServiceObjectFormatL();
            break;
        case EMTPObjectPropCodeProtectionStatus:
            ServiceProtectionStatusL();
            break;
        case EMTPObjectPropCodeObjectSize:
            ServiceObjectSizeL();
            break;
        case EMTPObjectPropCodeObjectFileName:
            ServiceFileNameL();
            break;
        case EMTPObjectPropCodeDateCreated:
            ServiceDateCreatedL();
            break;
        case EMTPObjectPropCodeDateModified:
            ServiceDateModifiedL();
            break;
        case EMTPObjectPropCodeParentObject:
            ServiceParentObjectL();
            break;
        case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
            ServicePuidL();
            break;
        case EMTPObjectPropCodeName:
            ServiceNameL();
            break;
        case EMTPObjectPropCodeWidth:
            ServiceWidthL();
            break;
        case EMTPObjectPropCodeHeight:
            ServiceHeightL();
            break;
        case EMTPObjectPropCodeImageBitDepth:
            ServiceImageBitDepthL();
            break;
        case EMTPObjectPropCodeRepresentativeSampleFormat:
            ServiceRepresentativeSampleFormatL();
            break;
        case EMTPObjectPropCodeRepresentativeSampleSize:
            ServiceRepresentativeSampleSizeL();
            break;
        case EMTPObjectPropCodeRepresentativeSampleHeight:
            ServiceRepresentativeSampleHeightL();
            break;
        case EMTPObjectPropCodeRepresentativeSampleWidth:
            ServiceRepresentativeSampleWidthL();
            break; 
        case EMTPObjectPropCodeRepresentativeSampleData:
            ServiceRepresentativeSampleDataL();
            break;
        case EMTPObjectPropCodeNonConsumable:
            ServiceNonConsumableL();
            break;            
        default:
            User::Leave(KErrGeneral);
        }	
    }	

void CMTPImageDpGetObjectPropValue::ServiceStorageIdL()
    {
    TUint32 storageId;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeStorageID, storageId);
    iMTPTypeUint32.Set(storageId);
    
    SendDataL(iMTPTypeUint32);
    }
    
void CMTPImageDpGetObjectPropValue::ServiceObjectFormatL()
    {
    TUint16 format(EMTPFormatCodeUndefined);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectFormat, format);
    iMTPTypeUint16.Set(format);
    
    SendDataL(iMTPTypeUint16);	
    }
    
void CMTPImageDpGetObjectPropValue::ServiceProtectionStatusL()
    {
    TUint16 protection;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeProtectionStatus, protection);
    iMTPTypeUint16.Set(protection);

    SendDataL(iMTPTypeUint16);	
    }
    
void CMTPImageDpGetObjectPropValue::ServiceObjectSizeL()
    {
    TUint64 size;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectSize, size);
    iMTPTypeUint64.Set(size);

    SendDataL(iMTPTypeUint64);
    }
    
    
void CMTPImageDpGetObjectPropValue::ServiceFileNameL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectFileName, *iMTPTypeString);
    SendDataL(*iMTPTypeString);
    }

void CMTPImageDpGetObjectPropValue::ServiceDateCreatedL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeDateCreated, *iMTPTypeString);
    SendDataL(*iMTPTypeString);
    }
    
void CMTPImageDpGetObjectPropValue::ServiceDateModifiedL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeDateModified, *iMTPTypeString);
    SendDataL(*iMTPTypeString);
    }
    
void CMTPImageDpGetObjectPropValue::ServiceParentObjectL()
    {
    TUint32 parent;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeParentObject, parent);
    if (parent == KMTPHandleNoParent)
        {
        parent = 0;
        }
    iMTPTypeUint32.Set(parent);
    
    SendDataL(iMTPTypeUint32);
    }
    
void CMTPImageDpGetObjectPropValue::ServicePuidL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodePersistentUniqueObjectIdentifier, iMTPTypeUint128);
    SendDataL(iMTPTypeUint128);
    }
    
void CMTPImageDpGetObjectPropValue::ServiceNameL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeName, *iMTPTypeString);
    SendDataL(*iMTPTypeString);
    }
    
void CMTPImageDpGetObjectPropValue::ServiceWidthL()
    {
    TUint32 width;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeWidth, width);
    iMTPTypeUint32.Set(width);
    
    SendDataL(iMTPTypeUint32);
    }

void CMTPImageDpGetObjectPropValue::ServiceHeightL()
    {
    TUint32 height;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeHeight, height);
    iMTPTypeUint32.Set(height);
    
    SendDataL(iMTPTypeUint32);
    }

void CMTPImageDpGetObjectPropValue::ServiceImageBitDepthL()
    {
    TUint32 imageBitDepth;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeImageBitDepth, imageBitDepth);
    iMTPTypeUint32.Set(imageBitDepth);
    
    SendDataL(iMTPTypeUint32);
    }


void CMTPImageDpGetObjectPropValue::ServiceRepresentativeSampleFormatL()
    {
    TUint16 representativeSampleFormat;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleFormat, representativeSampleFormat);
    iMTPTypeUint16.Set(representativeSampleFormat);
    
    SendDataL(iMTPTypeUint16);
    }

void CMTPImageDpGetObjectPropValue::ServiceRepresentativeSampleSizeL()
    {
    TUint32 representativeSampleSize;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleSize, representativeSampleSize);
    iMTPTypeUint32.Set(representativeSampleSize);
    
    SendDataL(iMTPTypeUint32);
    }

void CMTPImageDpGetObjectPropValue::ServiceRepresentativeSampleHeightL()
    {
    TUint32 representativeSampleHeight;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleHeight, representativeSampleHeight);
    iMTPTypeUint32.Set(representativeSampleHeight);
    
    SendDataL(iMTPTypeUint32);
    }

void CMTPImageDpGetObjectPropValue::ServiceRepresentativeSampleWidthL()
    {
    TUint32 representativeSampleWidth;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleWidth, representativeSampleWidth);
    iMTPTypeUint32.Set(representativeSampleWidth);
    
    SendDataL(iMTPTypeUint32);
    }

void CMTPImageDpGetObjectPropValue::ServiceRepresentativeSampleDataL()
    {
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleData, *iMTPTypeArray);
    
    SendDataL(*iMTPTypeArray);
    }

void CMTPImageDpGetObjectPropValue::ServiceNonConsumableL()
    {
    TUint8 nonConsumable;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeNonConsumable, nonConsumable);
    iMTPTypeUint8.Set(nonConsumable);
    SendDataL(iMTPTypeUint8);
    }
