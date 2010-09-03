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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetreferences.cpp

#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpsvcgetreferences.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetreferencesTraces.h"
#endif



EXPORT_C MMTPRequestProcessor* CMTPSvcGetReferences::NewL(MMTPDataProviderFramework& aFramework, 
														MMTPConnection& aConnection, 
														MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetReferences* self = new (ELeave) CMTPSvcGetReferences(aFramework, aConnection, aDataProvider);
	return self;
	}

/**
Destructor
*/
EXPORT_C CMTPSvcGetReferences::~CMTPSvcGetReferences()
	{
	OstTraceFunctionEntry0( CMTPSVCGETREFERENCES_CMTPSVCGETREFERENCES_DES_ENTRY );
	delete iReferences;
	delete iReceivedObjectMetaData;
	OstTraceFunctionExit0( CMTPSVCGETREFERENCES_CMTPSVCGETREFERENCES_DES_EXIT );
	}

/**
Standard c++ constructor
*/
CMTPSvcGetReferences::CMTPSvcGetReferences(MMTPDataProviderFramework& aFramework, 
										MMTPConnection& aConnection, 
										MMTPServiceDataProvider& aDataProvider)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL), 
	iDataProvider(aDataProvider)
	{
	OstTraceFunctionEntry0( CMTPSVCGETREFERENCES_CMTPSVCGETREFERENCES_CONS_ENTRY );
	OstTraceFunctionExit0( CMTPSVCGETREFERENCES_CMTPSVCGETREFERENCES_CONS_EXIT );
	}

TMTPResponseCode CMTPSvcGetReferences::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETREFERENCES_CHECKREQUESTL_ENTRY );

	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		// Check object handle
		MMTPObjectMgr& objMgr(iFramework.ObjectMgr());
		// Check whether object handle is valid
		if (objMgr.ObjectL(objectHandle, *iReceivedObjectMetaData))
			{
			// Check whether the owner of this object is correct data provider
			if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId())
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				OstTrace0( TRACE_WARNING, DUP1_CMTPSVCGETREFERENCES_CHECKREQUESTL, "CheckRequestL - DataProviderId dismatch" );
				}
			else
				{
				// Check format code
				TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				if (!iDataProvider.ObjectHandler(formatCode))
					{
					responseCode = EMTPRespCodeInvalidObjectHandle;
					}
				}
			}
		else
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETREFERENCES_CHECKREQUESTL, "Exit with code: 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETREFERENCES_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcGetReferences::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETREFERENCES_SERVICEL_ENTRY );
	TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
	delete iReferences;
	iReferences = NULL;
	iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	iDataProvider.ObjectHandler(formatCode)->GetObjectReferenceL(*iReceivedObjectMetaData, *iReferences);
	SendDataL(*iReferences);
	OstTraceFunctionExit0( CMTPSVCGETREFERENCES_SERVICEL_EXIT );
	}
