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
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypestring.h>

#include "mtpdpconst.h"
#include "cmtpimagedpgetobjectpropdesc.h"
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"GetObjectPropDesc");)

_LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPImageDpGetObjectPropDesc::NewL(
                                            MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection,CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpGetObjectPropDesc* self = new (ELeave) CMTPImageDpGetObjectPropDesc(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;

    }

/**
Destructor
*/		
CMTPImageDpGetObjectPropDesc::~CMTPImageDpGetObjectPropDesc()
    {	
    __FLOG(_L8(">> ~CMTPImageDpGetObjectPropDesc"));
    __FLOG(_L8(">> CMTPImageDpCopyObject::~CMTPImageDpCopyObject"));
    delete iObjectProperty;
    __FLOG(_L8("<< ~CMTPImageDpGetObjectPropDesc"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/	
CMTPImageDpGetObjectPropDesc::CMTPImageDpGetObjectPropDesc(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

/**
check property code
*/
TMTPResponseCode CMTPImageDpGetObjectPropDesc::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObjectPropDesc::CheckRequestL"));
    TMTPResponseCode response = CMTPRequestProcessor::CheckRequestL(); 
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);

    const TInt count = sizeof(KMTPImageDpSupportedProperties) / sizeof(TUint16);
    TInt i = 0;
    for(i = 0; i < count; i++)
        {
        if(KMTPImageDpSupportedProperties[i] == propCode)
            {
            break;
            }
        }
    if(i == count)
        {
        response = EMTPRespCodeInvalidObjectPropCode;
        }

    __FLOG(_L8("<< CMTPImageDpGetObjectPropDesc::CheckRequestL"));
    return response;	
    }

    
/**
GetObjectPropDesc request handler
*/	
void CMTPImageDpGetObjectPropDesc::ServiceL()
    {
    delete iObjectProperty;
    iObjectProperty = NULL;	
    
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    __FLOG_VA((_L8(">> CMTPImageDpGetObjectPropDesc::ServiceL propcode %d"), propCode));	    
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
        case EMTPObjectPropCodeNonConsumable:
            ServiceNonConsumableL();
            break;            
        default:
            {
            //Leave 
            User::Leave(KErrGeneral);
            }
            break;
        }

    __ASSERT_DEBUG(iObjectProperty, Panic(EMTPImageDpObjectPropertyNull));
    
    iObjectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(propCode));
    SendDataL(*iObjectProperty);

    __FLOG(_L8("<< CMTPImageDpGetObjectPropDesc::ServiceL"));
    }


/**
Second-phase construction
*/			
void CMTPImageDpGetObjectPropDesc::ConstructL()
    {	    
    }
        

void CMTPImageDpGetObjectPropDesc::ServiceStorageIdL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID);	
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceObjectFormatL()
    {	
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat);	
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceProtectionStatusL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    //Currently, we only support EMTPProtectionNoProtection and EMTPProtectionReadOnly
//  TUint16 values[] = {EMTPProtectionNoProtection, EMTPProtectionReadOnly, EMTPProtectionReadOnlyData, EMTPProtectionNonTransferable};
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
    
void CMTPImageDpGetObjectPropDesc::ServiceObjectSizeL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize);	
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceFileNameL()
    {
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName );
    // Althrough iObjectProperty is released in ServiceL(), release it here maybe a more safer way.
    if ( iObjectProperty != NULL )
	    {
	    delete iObjectProperty;
	    iObjectProperty = NULL;
	    }

    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFileName, CMTPTypeObjectPropDesc::ERegularExpressionForm, form);

    CleanupStack::PopAndDestroy( form );
    }

void CMTPImageDpGetObjectPropDesc::ServiceDateCreatedL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeString;
    info.iFormFlag     = CMTPTypeObjectPropDesc::EDateTimeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateCreated, info, NULL);
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceDateModifiedL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified);
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceParentObjectL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject);
    }
    
void CMTPImageDpGetObjectPropDesc::ServicePuidL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceNameL()
    {
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName);	
    }
    
void CMTPImageDpGetObjectPropDesc::ServiceWidthL()
    {    
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
   
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);

    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeWidth, info, expectedForm);
    CleanupStack::PopAndDestroy(expectedForm);
    }

void CMTPImageDpGetObjectPropDesc::ServiceHeightL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);

    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeHeight, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm);    
    }

void CMTPImageDpGetObjectPropDesc::ServiceImageBitDepthL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);

    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeImageBitDepth, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm); 
    
    //to do: deine the Group code
    //iObjectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, 1);
    }

void CMTPImageDpGetObjectPropDesc::ServiceRepresentativeSampleFormatL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT16;
    info.iFormFlag     = CMTPTypeObjectPropDesc::EEnumerationForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    TUint16 values[] = {EMTPFormatCodeEXIFJPEG};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }  
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeRepresentativeSampleFormat, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm);
    }

void CMTPImageDpGetObjectPropDesc::ServiceRepresentativeSampleSizeL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);

    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeRepresentativeSampleSize, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm); 
    }

void CMTPImageDpGetObjectPropDesc::ServiceRepresentativeSampleHeightL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeRepresentativeSampleHeight, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm);   
     
    }

void CMTPImageDpGetObjectPropDesc::ServiceRepresentativeSampleWidthL()
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);

    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeRepresentativeSampleWidth, info, expectedForm);
    
    CleanupStack::PopAndDestroy(expectedForm);
    }

void CMTPImageDpGetObjectPropDesc::ServiceNonConsumableL()
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

TUint16 CMTPImageDpGetObjectPropDesc::GetPropertyGroupNumber(const TUint16 aPropCode) const
    {
    for( TInt propCodeIndex = 0 ; propCodeIndex < KMTPImageDpGroupOneSize ; propCodeIndex++)
        {
            if(KMTPImageDpGroupOneProperties[propCodeIndex] == aPropCode)
                {
                return KMTPImageDpPropertyGroupOneNumber;
                }
        }
    
    // if not foud, the group number should be 0.
    return 0;
    }
