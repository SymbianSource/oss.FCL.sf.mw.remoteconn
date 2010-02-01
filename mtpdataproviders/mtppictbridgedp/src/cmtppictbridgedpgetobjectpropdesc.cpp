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

#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectpropdesc.h>

#include <mtp/cmtptypestring.h>
#include "cmtppictbridgedpgetobjectpropdesc.h"
#include "mtppictbridgedpconst.h"
#include "mtppictbridgedppanic.h"

 
_LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");

/**
Two-phase construction method
@param aPlugin  The data provider plugin
@param aFramework   The data provider framework
@param aConnection  The connection from which the request comes
@param aDataProvider  The pictBridge dataprovider
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPPictBridgeGetObjectPropDesc::NewL(
    MMTPDataProviderFramework& aFramework, 
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider )
    {
    CMTPPictBridgeGetObjectPropDesc* self = new (ELeave) CMTPPictBridgeGetObjectPropDesc(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/      
CMTPPictBridgeGetObjectPropDesc::~CMTPPictBridgeGetObjectPropDesc()
    {   
    delete iObjectProperty;
    }

/**
Standard c++ constructor
*/  
CMTPPictBridgeGetObjectPropDesc::CMTPPictBridgeGetObjectPropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iPictBridgeDP(aDataProvider)
    {
    }
    
/**
GetObjectPropDesc request handler
*/  
void CMTPPictBridgeGetObjectPropDesc::ServiceL()
    {
    delete iObjectProperty;
    iObjectProperty = NULL; 
    
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
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
        case EMTPObjectPropCodeNonConsumable:
            ServiceNonConsumableL();
            break;
        default:
            Panic(EMTPPictBridgeDpUnsupportedProperty);
        }
    __ASSERT_DEBUG(iObjectProperty, Panic(EMTPPictBridgeDpObjectPropertyNull));
    iObjectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(propCode));
    SendDataL(*iObjectProperty);    
    }


/**
Second-phase construction
*/          
void CMTPPictBridgeGetObjectPropDesc::ConstructL()
    {
    }
        

void CMTPPictBridgeGetObjectPropDesc::ServiceStorageIdL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID);    
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceObjectFormatL()
    {   
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat); 
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceProtectionStatusL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    //Currently, we only support EMTPProtectionNoProtection and EMTPProtectionReadOnly
    TUint16 values[] = {EMTPProtectionNoProtection, EMTPProtectionReadOnly};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }  
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeProtectionStatus, *expectedForm);          
    CleanupStack::PopAndDestroy(expectedForm);
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceObjectSizeL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize);   
    }
    
    
void CMTPPictBridgeGetObjectPropDesc::ServiceFileNameL()
    {
        CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName ); // + form
        // Althrough iObjectProperty is released in ServiceL(),
        // release it here maybe a more safer way 
        if ( iObjectProperty != NULL )
        {
        delete iObjectProperty;
        iObjectProperty = NULL;
        }

        iObjectProperty = CMTPTypeObjectPropDesc::NewL( EMTPObjectPropCodeObjectFileName,
            CMTPTypeObjectPropDesc::ERegularExpressionForm,
            form );

        CleanupStack::PopAndDestroy( form ); // - form
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceDateModifiedL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified);
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceParentObjectL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject);
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServicePuidL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceNameL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName); 
    }
    
void CMTPPictBridgeGetObjectPropDesc::ServiceNonConsumableL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT8);
    CleanupStack::PushL(expectedForm);
    TUint8 values[] = {0,1};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint8 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeNonConsumable, *expectedForm);     
    CleanupStack::PopAndDestroy(expectedForm);
    }


    
TUint16  CMTPPictBridgeGetObjectPropDesc::GetPropertyGroupNumber(const TUint16 aPropCode) const
    {
    // begin from group 1, since group 0 is not used
    for(int group = 1; group <= KMTPDpPropertyGroupNumber; group++)
        {
        for( int propCodeIndex = 0 ; propCodeIndex < KMTPDpPropertyNumber ; propCodeIndex++)
            {
                if(KMTPDpPropertyGroups[group][propCodeIndex] == aPropCode)
                    {
                    return group;
                    }
            }
        }
    
    // if not foud, the group number should be 0.
    return 0;
    }

