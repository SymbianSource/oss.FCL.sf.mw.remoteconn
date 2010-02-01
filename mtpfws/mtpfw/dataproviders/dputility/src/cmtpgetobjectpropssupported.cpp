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
#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpgetobjectpropssupported.h"
#include "mtpdpconst.h"

/**
Verification data for the GetObjectPropSupported request
*/
const TMTPRequestElementInfo KMTPGetObjectPropSupportedPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeFormatCode, EMTPElementAttrNone, 1, EMTPFormatCodeUndefined, 0},
    };
 
/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
EXPORT_C MMTPRequestProcessor* CMTPGetObjectPropsSupported::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPGetObjectPropsSupported* self = new (ELeave) CMTPGetObjectPropsSupported(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPGetObjectPropsSupported::~CMTPGetObjectPropsSupported()
	{
	delete iObjectPropsSupported;
	}

/**
Standard c++ constructor
*/		
CMTPGetObjectPropsSupported::CMTPGetObjectPropsSupported(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPropSupportedPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPropSupportedPolicy)
	{
	}
									
/**
check format code
*/
TMTPResponseCode CMTPGetObjectPropsSupported::CheckRequestL()
    {
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL(); 
    
    if (EMTPRespCodeOK == responseCode)
        {
        TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        
        /**
         * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
         * but it does not really own the format 0x3002.
         * 
         * Make the same behavior betwen 0x3000 and 0x3002.
         */
        if((formatCode != EMTPFormatCodeUndefined) && (formatCode != EMTPFormatCodeAssociation) &&(EMTPFormatCodeScript != formatCode))
            {
            responseCode = EMTPRespCodeInvalidObjectFormatCode;
            }
        }

    return responseCode;
    }									
	
/**
GetObjectPropSupported request handler
*/
void CMTPGetObjectPropsSupported::ServiceL()
	{
	TInt count = sizeof(KMTPDpSupportedProperties) / sizeof(TUint16);
	for(TInt i = 0; i < count; i++)
		{
		iObjectPropsSupported->AppendUintL(KMTPDpSupportedProperties[i]);
		}
	TUint32 objectFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	if(objectFormatCode == EMTPFormatCodeAssociation)
		{
		iObjectPropsSupported->AppendUintL(EMTPObjectPropCodeAssociationType);
		iObjectPropsSupported->AppendUintL(EMTPObjectPropCodeAssociationDesc);
		}			
	SendDataL(*iObjectPropsSupported);	
	}

/**
Second-phase construction
*/		
void CMTPGetObjectPropsSupported::ConstructL()
	{
	iObjectPropsSupported = CMTPTypeArray::NewL(EMTPTypeAUINT16);	
	}
		




	
















	

	


   	

	






