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


#include <mtp/cmtptypeinterdependentpropdesc.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpdataproviderframework.h>
#include "cmtprequestprocessor.h"
#include "cmtppictbridgegetformatcapabilities.h"
#include "ptpdef.h"

/**
Two-phase construction method
@param aFramework   The data provider framework
@param aConnection  The connection object
@param aDataProvider  The pictbridge Dp
*/ 
MMTPRequestProcessor* CMTPPictBridgeDpGetFormatCapabilities::NewL(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider)
    {
    CMTPPictBridgeDpGetFormatCapabilities* self = new (ELeave) CMTPPictBridgeDpGetFormatCapabilities(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/
CMTPPictBridgeDpGetFormatCapabilities::~CMTPPictBridgeDpGetFormatCapabilities()
    {
    delete iCapabilityList;
    __FLOG_CLOSE;
    }

/**
ServiceL
*/
void CMTPPictBridgeDpGetFormatCapabilities::ServiceL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpGetFormatCapabilities::ServiceL"));
    delete iCapabilityList;
    iCapabilityList = CMTPTypeFormatCapabilityList::NewL();
    iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    BuildFormatScriptL();
 
    SendDataL(*iCapabilityList); 
    __FLOG(_L8("<< CMTPPictBridgeDpGetFormatCapabilities::ServiceL"));   
    }
    
/**
BuildFormatScriptL
*/
void CMTPPictBridgeDpGetFormatCapabilities::BuildFormatScriptL() 
    {
    __FLOG(_L8(">> CMTPPictBridgeDpGetFormatCapabilities::BuildFormatScriptL"));
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC( EMTPFormatCodeScript ,interDesc );
     
    //for group code
    //EMTPObjectPropCodeStorageID
    CMTPTypeObjectPropDesc* objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeStorageID));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodeObjectFormat
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeObjectFormat));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodeProtectionStatus
    frmCap->AppendL( ServiceProtectionStatusL() );
    
    //EMTPObjectPropCodeObjectSize
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeObjectSize));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodeObjectFileName
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFileName);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeObjectFileName));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodeDateModified
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeDateModified));
    frmCap->AppendL(  objectProperty );
    
    //EMTPObjectPropCodeParentObject
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeParentObject));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodePersistentUniqueObjectIdentifier));
    frmCap->AppendL( objectProperty );
    
    //EMTPObjectPropCodeName
    objectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName);
    objectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeName));
    frmCap->AppendL( objectProperty); 
    
    //EMTPObjectPropCodeNonConsumable
    frmCap->AppendL(ServiceNonConsumableL() );
 
    iCapabilityList->AppendL(frmCap);
    CleanupStack::Pop(frmCap);
    CleanupStack::Pop(interDesc);
    __FLOG(_L8("<< CMTPPictBridgeDpGetFormatCapabilities::BuildFormatScriptL"));
    }

/**
ServiceProtectionStatusL
*/
CMTPTypeObjectPropDesc* CMTPPictBridgeDpGetFormatCapabilities::ServiceProtectionStatusL()
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
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeProtectionStatus, *expectedForm);
    ret->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeProtectionStatus));
    CleanupStack::PopAndDestroy(expectedForm);
    return ret;
    }

/**
ServiceNonConsumableL
*/
CMTPTypeObjectPropDesc* CMTPPictBridgeDpGetFormatCapabilities::ServiceNonConsumableL()
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
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeNonConsumable, *expectedForm);
    ret->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(EMTPObjectPropCodeNonConsumable));
    CleanupStack::PopAndDestroy(expectedForm);
    
    return ret;
    }

/**
CheckRequestL
*/
TMTPResponseCode CMTPPictBridgeDpGetFormatCapabilities::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpGetFormatCapabilities::CheckRequestL"));   
    TMTPResponseCode response = CMTPRequestProcessor::CheckRequestL(); 
    if(EMTPRespCodeOK != response)
        return response;
    
    TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    
    if((formatCode != EMTPFormatCodeScript)&& (iFormatCode != KMTPFormatsAll))
        {
        return EMTPRespCodeInvalidObjectFormatCode;
        }

    __FLOG(_L8("<< CMTPPictBridgeDpGetFormatCapabilities::CheckRequestL"));   
    return EMTPRespCodeOK; 
    }
    
/**
Standard c++ constructor
*/  
CMTPPictBridgeDpGetFormatCapabilities::CMTPPictBridgeDpGetFormatCapabilities(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider):
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iPictBridgeDP(aDataProvider)
    {
    }
    
/**
ConstructL
*/      
void CMTPPictBridgeDpGetFormatCapabilities::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPPictBridgeDpGetFormatCapabilities::ConstructL")); 
    }

/**
GetPropertyGroupNumber
*/  
TUint16  CMTPPictBridgeDpGetFormatCapabilities::GetPropertyGroupNumber(const TUint16 aPropCode) const
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
