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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpsetobjectproplistTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_CMTPIMAGEDPSETOBJECTPROPLIST_DES_ENTRY );
    delete iPropertyList;
    delete iObjectMeta;
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_CMTPIMAGEDPSETOBJECTPROPLIST_DES_EXIT );
    }
    
CMTPImageDpSetObjectPropList::CMTPImageDpSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iDataProvider(aDataProvider),
    iPropertyMgr(aDataProvider.PropertyMgr())		
    {
    
    }
    
void CMTPImageDpSetObjectPropList::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
    iPropertyList = CMTPTypeObjectPropList::NewL();
    iObjectMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_CONSTRUCTL_EXIT );
    }

void CMTPImageDpSetObjectPropList::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_SERVICEL_ENTRY );
    ReceiveDataL(*iPropertyList);
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_SERVICEL_EXIT );
    }

TBool CMTPImageDpSetObjectPropList::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
    MMTPObjectMgr& objects(iFramework.ObjectMgr());
    TUint32 parameter(0);
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    const TUint count(iPropertyList->NumberOfElements());
    iPropertyList->ResetCursor();
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL, 
            "setting %d properties", count );
    TUint32 preHandle = KMTPHandleNone;
    for (TUint i(0); ((i < count) && (responseCode == EMTPRespCodeOK)); i++)
        {
        CMTPTypeObjectPropListElement& element=iPropertyList->GetNextElementL(); 
        TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);
        TUint16 propertyCode = element.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode);
        TUint16 dataType = element.Uint16L(CMTPTypeObjectPropListElement::EDatatype);
        OstTraceExt3( TRACE_NORMAL, DUP1_CMTPIMAGEDPSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL, 
                "set property, propertycode %d, datatype %d, handle %d", propertyCode, dataType, handle );
        
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
                    case EMTPObjectPropCodeHidden:
                        iPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propertyCode), element.Uint16L(CMTPTypeObjectPropListElement::EValue));
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
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;
    }

TBool CMTPImageDpSetObjectPropList::HasDataphase() const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_HASDATAPHASE_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_HASDATAPHASE_EXIT );
    return ETrue;
    }

TMTPResponseCode CMTPImageDpSetObjectPropList::CheckPropCode(TUint16 aPropertyCode, TUint16 aDataType) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPLIST_CHECKPROPCODE_ENTRY );
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
        case EMTPObjectPropCodeRepresentativeSampleData:
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
        case EMTPObjectPropCodeHidden:
            if (aDataType != EMTPTypeUINT16)
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            break;
        default:
            responseCode = EMTPRespCodeInvalidObjectPropCode;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPLIST_CHECKPROPCODE_EXIT );
    return responseCode;
    }
    

