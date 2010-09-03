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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcsetreferences.cpp


#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpsvcsetreferences.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcsetreferencesTraces.h"
#endif


EXPORT_C MMTPRequestProcessor* CMTPSvcSetReferences::NewL(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcSetReferences* self = new (ELeave) CMTPSvcSetReferences(aFramework, aConnection, aDataProvider);
	return self;
	}

EXPORT_C CMTPSvcSetReferences::~CMTPSvcSetReferences()
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_CMTPSVCSETREFERENCES_DES_ENTRY );
	delete iReferences;
	delete iReceivedObjectMetaData;
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_CMTPSVCSETREFERENCES_DES_EXIT );
	}

/**
Standard c++ constructor
*/    
CMTPSvcSetReferences::CMTPSvcSetReferences(MMTPDataProviderFramework& aFramework, 
										MMTPConnection& aConnection, 
										MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), 
	iDataProvider(aDataProvider)
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_CMTPSVCSETREFERENCES_CONS_ENTRY );
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_CMTPSVCSETREFERENCES_CONS_EXIT );
	}

/**
SetReferences request handler
start receiving reference data from the initiator
*/
void CMTPSvcSetReferences::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_SERVICEL_ENTRY );
	delete iReferences;
	iReferences = NULL;
	iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	ReceiveDataL(*iReferences);
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_SERVICEL_EXIT );
	}

TBool CMTPSvcSetReferences::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_DOHANDLERESPONSEPHASEL_ENTRY );
	if(!VerifyReferenceHandlesL())
		{
		SendResponseL(EMTPRespCodeInvalidObjectReference);
		}
	else
		{
		TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
		TMTPResponseCode responseCode = EMTPRespCodeOK;
		responseCode = (iDataProvider.ObjectHandler(formatCode))->SetObjectReferenceL(*iReceivedObjectMetaData, *iReferences);
		SendResponseL(responseCode);
		}
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}

TBool CMTPSvcSetReferences::HasDataphase() const
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_HASDATAPHASE_ENTRY );
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_HASDATAPHASE_EXIT );
	return ETrue;
	}

TBool CMTPSvcSetReferences::VerifyReferenceHandlesL() const
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_VERIFYREFERENCEHANDLESL_ENTRY );
	__ASSERT_DEBUG(iReferences, User::Invariant());
	TBool result = ETrue;
	TInt count = iReferences->NumElements();
	CMTPObjectMetaData* object = CMTPObjectMetaData::NewLC();
	MMTPObjectMgr& objectMgr = iFramework.ObjectMgr();
	for(TInt i = 0; i < count; i++)
		{
		TMTPTypeUint32 handle;
		iReferences->ElementL(i, handle);
		if(!objectMgr.ObjectL(handle, *object))
			{
			result = EFalse;
			break;
			}
		}
	CleanupStack::PopAndDestroy(object);
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_VERIFYREFERENCEHANDLESL_EXIT );
	return result;
	}

TMTPResponseCode CMTPSvcSetReferences::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETREFERENCES_CHECKREQUESTL_ENTRY );

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
				OstTrace0( TRACE_WARNING, DUP1_CMTPSVCSETREFERENCES_CHECKREQUESTL, "CheckRequestL - DataProviderId dismatch" );	
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
    OstTrace1( TRACE_NORMAL, CMTPSVCSETREFERENCES_CHECKREQUESTL, "Exit with code: 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCSETREFERENCES_CHECKREQUESTL_EXIT );
	
	return responseCode;
	}
