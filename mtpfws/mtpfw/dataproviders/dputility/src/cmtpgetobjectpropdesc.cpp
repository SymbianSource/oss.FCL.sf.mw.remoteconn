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
#include "cmtpgetobjectpropdesc.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"

 
_LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPGetObjectPropDesc::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPGetObjectPropDesc* self = new (ELeave) CMTPGetObjectPropDesc(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/		
EXPORT_C CMTPGetObjectPropDesc::~CMTPGetObjectPropDesc()
	{	
	delete iObjectProperty;
	}

/**
Standard c++ constructor
*/	
CMTPGetObjectPropDesc::CMTPGetObjectPropDesc(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
	{
	}

/**
check format code and property code
*/
TMTPResponseCode CMTPGetObjectPropDesc::CheckRequestL()
	{
	TMTPResponseCode response = CMTPRequestProcessor::CheckRequestL(); 
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
    /**
     * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
     * but it does not really own the format 0x3002.
     * 
     * Make the same behavior betwen 0x3000 and 0x3002.
     */
	if((formatCode != EMTPFormatCodeUndefined) && (formatCode != EMTPFormatCodeAssociation) &&( EMTPFormatCodeScript != formatCode ))
		{
		response = EMTPRespCodeInvalidObjectFormatCode;
		}
	else
		{
		if(propCode != EMTPObjectPropCodeAssociationType && propCode != EMTPObjectPropCodeAssociationDesc)
			{			
			const TInt count = sizeof(KMTPDpSupportedProperties) / sizeof(TUint16);
			TInt i = 0;
			for(i = 0; i < count; i++)
				{
				if(KMTPDpSupportedProperties[i] == propCode)
					{
					break;
					}
				}
			if(i == count)
				{
				response = EMTPRespCodeInvalidObjectPropCode;
				}
			}
		else
			{
			if(formatCode != EMTPFormatCodeAssociation)
			response = EMTPRespCodeInvalidObjectPropCode;
			}
		}
	return response;	
	}
	
/**
GetObjectPropDesc request handler
*/	
void CMTPGetObjectPropDesc::ServiceL()
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
		case EMTPObjectPropCodeAssociationType:
			ServiceAssociationTypeL();
			break;
		case EMTPObjectPropCodeAssociationDesc:
			ServiceAssociationDescL();
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
		case EMTPObjectPropCodeHidden:
		    ServiceHiddenL();
		    break;
		default:
		    User::Leave( KErrNotSupported );
			break;
		}
	__ASSERT_DEBUG(iObjectProperty, Panic(EMTPDpObjectPropertyNull));
	iObjectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode,GetPropertyGroupNumber(propCode));
	SendDataL(*iObjectProperty);	
	}


/**
Second-phase construction
*/			
void CMTPGetObjectPropDesc::ConstructL()
	{
	}
		

void CMTPGetObjectPropDesc::ServiceStorageIdL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID);	
	}
	
void CMTPGetObjectPropDesc::ServiceObjectFormatL()
	{	
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat);	
	}
	
void CMTPGetObjectPropDesc::ServiceProtectionStatusL()
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
	
void CMTPGetObjectPropDesc::ServiceObjectSizeL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize);	
	}
	
void CMTPGetObjectPropDesc::ServiceAssociationTypeL()
	{
	CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);

    const TUint16 KMtpValues[] = { EMTPAssociationTypeGenericFolder };
    const TUint KNumMtpValues(sizeof(KMtpValues) / sizeof(KMtpValues[0])); 
    const TUint16 KPtpValues[] = {EMTPAssociationTypeUndefined, EMTPAssociationTypeGenericFolder, EMTPAssociationTypeAlbum, EMTPAssociationTypeTimeSequence, EMTPAssociationTypeHorizontalPanoramic, EMTPAssociationTypeVerticalPanoramic, EMTPAssociationType2DPanoramic,EMTPAssociationTypeAncillaryData};
    const TUint KNumPtpValues(sizeof(KPtpValues) / sizeof(KPtpValues[0]));

    TUint numValues(KNumMtpValues);
    const TUint16* values = KMtpValues;
    if (EModeMTP != iFramework.Mode())
    	 {
    	 numValues = KNumPtpValues;
    	 values = KPtpValues;            
    	 }
   
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }  

    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeAssociationType, *expectedForm);
    CleanupStack::PopAndDestroy(expectedForm);
	}

void CMTPGetObjectPropDesc::ServiceAssociationDescL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeAssociationDesc);	
	}
	
void CMTPGetObjectPropDesc::ServiceFileNameL()
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
	
void CMTPGetObjectPropDesc::ServiceDateModifiedL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified);
	}
	
void CMTPGetObjectPropDesc::ServiceParentObjectL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject);
	}
	
void CMTPGetObjectPropDesc::ServicePuidL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
	}
	
void CMTPGetObjectPropDesc::ServiceNameL()
	{
	iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName);	
	}
	
void CMTPGetObjectPropDesc::ServiceNonConsumableL()
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

void CMTPGetObjectPropDesc::ServiceHiddenL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    TUint16 values[] = {EMTPVisible,EMTPHidden};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    iObjectProperty = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeHidden, *expectedForm);
    CleanupStack::PopAndDestroy(expectedForm);
    }
	
TUint16  CMTPGetObjectPropDesc::GetPropertyGroupNumber(const TUint16 aPropCode) const
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







	

	


   	

	






