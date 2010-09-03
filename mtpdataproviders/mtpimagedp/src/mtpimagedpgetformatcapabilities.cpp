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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "mtpimagedpgetformatcapabilitiesTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_CMTPIMAGEDPGETFORMATCAPABILITIES_ENTRY );
    delete iCapabilityList;
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_CMTPIMAGEDPGETFORMATCAPABILITIES_EXIT );
    }

void CMTPImageDpGetFormatCapabilities::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEL_ENTRY );
    delete iCapabilityList;
    iCapabilityList = NULL;
    iCapabilityList = CMTPTypeFormatCapabilityList::NewL();
    
    if (iFormatCode == KMTPFormatsAll)
        {
        TInt count(sizeof(KMTPImageDpSupportFormatCodes) / sizeof(KMTPImageDpSupportFormatCodes[0]));
        for (TInt i(0); i<count; i++ )
            {
            ServiceOneFormatCapabilitiesL(KMTPImageDpSupportFormatCodes[i]);
            }    
        }
    else
        {
        ServiceOneFormatCapabilitiesL(iFormatCode);
        }

    SendDataL(*iCapabilityList); 
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEL_EXIT );
    }
    
void CMTPImageDpGetFormatCapabilities::ServiceOneFormatCapabilitiesL(TUint aFormatCode)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEONEFORMATCAPABILITIESL_ENTRY );
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC(aFormatCode, interDesc);
    
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
    
    //EMTPObjectPropCodeHidden
    frmCap->AppendL( ServiceHiddenL() ); 
    
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
    
    const TUint32 KMaxLength = 0x0000FFFF;
    TMTPTypeUint32 maxLengthForm(KMaxLength);
    info.iDataType     = EMTPTypeAUINT8;
    info.iFormFlag     = CMTPTypeObjectPropDesc::EByteArrayForm;       
    desc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeRepresentativeSampleData, info, &maxLengthForm);
    frmCap->AppendL(desc);
    CleanupStack::Pop(1, desc);
    
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
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEONEFORMATCAPABILITIESL_EXIT );
    }


CMTPTypeObjectPropDesc* CMTPImageDpGetFormatCapabilities::ServiceProtectionStatusL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEPROTECTIONSTATUSL_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICEPROTECTIONSTATUSL_EXIT );
    return ret;
   
    }

CMTPTypeObjectPropDesc* CMTPImageDpGetFormatCapabilities::ServiceNonConsumableL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICENONCONSUMABLEL_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_SERVICENONCONSUMABLEL_EXIT );
    return ret;
    }


TMTPResponseCode CMTPImageDpGetFormatCapabilities::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_CHECKREQUESTL_ENTRY );
    TMTPResponseCode response = EMTPRespCodeOK;
    iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    
    if (iFormatCode != KMTPFormatsAll)
        {
        response = EMTPRespCodeInvalidObjectFormatCode;
        TInt count(sizeof(KMTPImageDpSupportFormatCodes) / sizeof(KMTPImageDpSupportFormatCodes[0]));
        for (TInt i(0); i<count; i++ )
            {
            if (iFormatCode == KMTPImageDpSupportFormatCodes[i])
                {
                response = EMTPRespCodeOK;
                break;
                }
            }        
        } 
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPGETFORMATCAPABILITIES_CHECKREQUESTL, "response = 0x%x", response );    
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_CHECKREQUESTL_EXIT );
    
    return response; 
    }
    

CMTPImageDpGetFormatCapabilities::CMTPImageDpGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    }
    
void CMTPImageDpGetFormatCapabilities::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETFORMATCAPABILITIES_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_CONSTRUCTL_EXIT );
    }

CMTPTypeObjectPropDesc* CMTPImageDpGetFormatCapabilities::ServiceHiddenL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    TUint16 values[] = {EMTPVisible, EMTPHidden};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeHidden, *expectedForm);     
    CleanupStack::PopAndDestroy(expectedForm);
    return ret;
    }
	
TUint16 CMTPImageDpGetFormatCapabilities::GetPropertyGroupNumber(const TUint16 aPropCode) const
    {
    for( TInt propCodeIndex = 0 ; propCodeIndex < KMTPImageDpGroupOneSize ; propCodeIndex++)
        {
            if(KMTPImageDpGroupOneProperties[propCodeIndex] == aPropCode)
                {
                OstTraceFunctionExit0( CMTPIMAGEDPGETFORMATCAPABILITIES_GETPROPERTYGROUPNUMBER_EXIT );
                return KMTPImageDpPropertyGroupOneNumber;
                }
        }
    
    // if not foud, the group number should be 0.
    return 0;
    }
   


