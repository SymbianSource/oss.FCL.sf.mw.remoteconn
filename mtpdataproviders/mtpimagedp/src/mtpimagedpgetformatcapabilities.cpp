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

#include <mtp/cmtptypeinterdependentpropdesc.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtptypestring.h>

#include "cmtpimagedpgetformatcapabilities.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"ImageDpGetFormatCapabilities");)
MMTPRequestProcessor* CMTPImageDpGetFormatCapabilities::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpGetFormatCapabilities* self = new (ELeave) CMTPImageDpGetFormatCapabilities(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
CMTPImageDpGetFormatCapabilities::~CMTPImageDpGetFormatCapabilities()
    {
    __FLOG(_L8(">> ~CMTPPictureDpGetObject"));
    delete iCapabilityList;
    __FLOG(_L8("<< ~CMTPPictureDpGetObject"));
    __FLOG_CLOSE;
    }

void CMTPImageDpGetFormatCapabilities::ServiceL()
    {
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::ServiceL"));
    delete iCapabilityList;
    iCapabilityList = NULL;
    iCapabilityList = CMTPTypeFormatCapabilityList::NewL();
    BuildFormatExifJpegL();
    SendDataL(*iCapabilityList); 
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::ServiceL"));   
    }
    
void CMTPImageDpGetFormatCapabilities::BuildFormatExifJpegL()
    {
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::BuildFormatExifJpegL"));
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC( EMTPFormatCodeEXIFJPEG ,interDesc );
    
    //EMTPObjectPropCodeStorageID
    CMTPTypeObjectPropDesc* desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeStorageID);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeStorageID)); 
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodeObjectFormat
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectFormat);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeObjectFormat));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodeProtectionStatus
    frmCap->AppendL( ServiceProtectionStatusL() );
    
    //EMTPObjectPropCodeObjectSize
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectSize);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeObjectSize));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodeObjectFileName
    _LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName );
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectFileName, CMTPTypeObjectPropDesc::ERegularExpressionForm, form);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeObjectFileName));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc); 
    CleanupStack::PopAndDestroy(form ); 
    
    //EMTPObjectPropCodeDateModified
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeDateModified);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeDateModified));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodeParentObject
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeParentObject);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeParentObject));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodePersistentUniqueObjectIdentifier));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);

    //EMTPObjectPropCodeName
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeName);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeName));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    //EMTPObjectPropCodeNonConsumable
    frmCap->AppendL(ServiceNonConsumableL() );
    
    
    
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    info.iDataType     = EMTPTypeString;
    info.iFormFlag     = CMTPTypeObjectPropDesc::EDateTimeForm;
    info.iGetSet       = CMTPTypeObjectPropDesc::EReadOnly;
    //EMTPObjectPropCodeDateCreated
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeDateCreated, info, NULL);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeDateCreated));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    info.iDataType     = EMTPTypeUINT32;
    info.iFormFlag     = CMTPTypeObjectPropDesc::ERangeForm;
    
    CMTPTypeObjectPropDescRangeForm* expectedForm = CMTPTypeObjectPropDescRangeForm::NewLC(EMTPTypeUINT32);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMinimumValue, 0x00000001);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EMaximumValue, 0x20000000);
    expectedForm->SetUint32L(CMTPTypeObjectPropDescRangeForm::EStepSize, 0x00000001);
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeWidth, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeWidth));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeHeight, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeHeight));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeImageBitDepth, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeImageBitDepth));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeRepresentativeSampleSize, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeRepresentativeSampleSize));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeRepresentativeSampleHeight, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeRepresentativeSampleHeight));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeRepresentativeSampleWidth, info, expectedForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeRepresentativeSampleWidth));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    CleanupStack::PopAndDestroy(expectedForm);
    
    info.iDataType = EMTPTypeUINT16;
    info.iFormFlag = CMTPTypeObjectPropDesc::EEnumerationForm;
    CMTPTypeObjectPropDescEnumerationForm* expectedEnumForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedEnumForm);
    TUint16 values[] = {EMTPFormatCodeEXIFJPEG};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedEnumForm->AppendSupportedValueL(data);
        }
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeRepresentativeSampleFormat, info, expectedEnumForm);
    desc->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeRepresentativeSampleFormat));
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    CleanupStack::PopAndDestroy(expectedEnumForm);
    
    iCapabilityList->AppendL(frmCap);
    CleanupStack::Pop(frmCap);
    CleanupStack::Pop(interDesc);
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::BuildFormatExifJpegL"));
    }


CMTPTypeObjectPropDesc* CMTPImageDpGetFormatCapabilities::ServiceProtectionStatusL()
    {
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::ServiceProtectionStatusL"));
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
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeProtectionStatus, *expectedForm);
    ret->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeProtectionStatus));
    CleanupStack::Pop(1, ret);
    CleanupStack::PopAndDestroy(expectedForm);
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::ServiceProtectionStatusL"));
    return ret;
   
    }

CMTPTypeObjectPropDesc* CMTPImageDpGetFormatCapabilities::ServiceNonConsumableL()
    {
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::ServiceNonConsumableL"));
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT8);
    CleanupStack::PushL(expectedForm);
    TUint8 values[] = {0,1};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint8 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeNonConsumable, *expectedForm);
    ret->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, GetPropertyGroupNumber(EMTPObjectPropCodeNonConsumable));
    CleanupStack::Pop(1, ret);
    CleanupStack::PopAndDestroy(expectedForm);
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::ServiceNonConsumableL"));
    return ret;
    }


TMTPResponseCode CMTPImageDpGetFormatCapabilities::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::CheckRequestL"));
    iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    if((iFormatCode != EMTPFormatCodeEXIFJPEG) && (iFormatCode != KMTPFormatsAll))
        {
        return EMTPRespCodeInvalidObjectFormatCode;
        }
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::CheckRequestL"));   
    return EMTPRespCodeOK; 
    }
    

CMTPImageDpGetFormatCapabilities::CMTPImageDpGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    }
    
void CMTPImageDpGetFormatCapabilities::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPPictureDpGetFormatCapabilities::ConstructL"));
    __FLOG(_L8("<< CMTPPictureDpGetFormatCapabilities::ConstructL"));
    }

TUint16 CMTPImageDpGetFormatCapabilities::GetPropertyGroupNumber(const TUint16 aPropCode) const
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
   


