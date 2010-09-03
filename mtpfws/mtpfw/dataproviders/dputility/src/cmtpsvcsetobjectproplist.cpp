// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsetobjectproplist.cpp

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpsvcsetobjectproplist.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcsetobjectproplistTraces.h"
#endif



EXPORT_C MMTPRequestProcessor* CMTPSvcSetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcSetObjectPropList* self = new (ELeave) CMTPSvcSetObjectPropList(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcSetObjectPropList::~CMTPSvcSetObjectPropList()
	{
	OstTraceFunctionEntry0( CMTPSVCSETOBJECTPROPLIST_CMTPSVCSETOBJECTPROPLIST_ENTRY );
	delete iPropertyList;
	OstTraceFunctionExit0( CMTPSVCSETOBJECTPROPLIST_CMTPSVCSETOBJECTPROPLIST_EXIT );
	}

CMTPSvcSetObjectPropList::CMTPSvcSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider)
	{
	}

void CMTPSvcSetObjectPropList::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
	iPropertyList = CMTPTypeObjectPropList::NewL();
	OstTraceFunctionExit0( CMTPSVCSETOBJECTPROPLIST_CONSTRUCTL_EXIT );
	}

void CMTPSvcSetObjectPropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETOBJECTPROPLIST_SERVICEL_ENTRY );
	ReceiveDataL(*iPropertyList);
	OstTraceFunctionExit0( CMTPSVCSETOBJECTPROPLIST_SERVICEL_EXIT );
	}

TBool CMTPSvcSetObjectPropList::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
	TUint32 parameter = 0;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	responseCode = SetObjectPropListL(*iPropertyList, parameter);
	OstTraceExt2( TRACE_NORMAL, CMTPSVCSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL, 
	        "SetObjectPropListL - ResponsCode: 0x%x, error index: %u", responseCode, parameter );
	
	SendResponseL(responseCode, 1, &parameter);
    OstTraceExt2( TRACE_NORMAL, DUP1_CMTPSVCSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL, 
            "Exit with responseCode = 0x%04X and failed index: %u", responseCode, parameter );	
	OstTraceFunctionExit0( CMTPSVCSETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}

TMTPResponseCode CMTPSvcSetObjectPropList::SetObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParameter)
	{
	OstTraceFunctionEntry0( CMTPSVCSETOBJECTPROPLIST_SETOBJECTPROPLISTL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	const TUint count = aObjectPropList.NumberOfElements();
	aObjectPropList.ResetCursor();
	CMTPObjectMetaData* objectMetaData  = CMTPObjectMetaData::NewL();
	CleanupStack::PushL(objectMetaData);
	TUint32 lastHandle = 0;
	MMTPSvcObjectHandler* lastHandler = NULL; 
	TUint errIndex = 0; // Index number for each segment with the same object handle.
	TUint i;
	for (i = 0; i < count; i++)
		{
		CMTPTypeObjectPropListElement& element = aObjectPropList.GetNextElementL();
		TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);	
		if (!iFramework.ObjectMgr().ObjectL(handle, *objectMetaData))
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		else
			{
			TUint16 formatCode = objectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
			const TDesC& suid = objectMetaData->DesC(CMTPObjectMetaData::ESuid);
			MMTPSvcObjectHandler* pHandler = iDataProvider.ObjectHandler(formatCode);
			if (pHandler)
				{
				// If the handler is not the last handle, need commit all properties now
				if (lastHandle != 0 && lastHandler && lastHandle != handle)
					{
					TRAPD(err, lastHandler->CommitL());
					if (KErrNone != err)
						{
						lastHandler->RollBack();
						responseCode = EMTPRespCodeInvalidObjectHandle;
						aParameter = errIndex;
						break;
						}
					// Record the next segment's first index number
					errIndex = i;
					}
				lastHandler = pHandler;
				lastHandle = handle;
				responseCode = pHandler->SetObjectPropertyL(suid, element, EMTPOpCodeSetObjectPropList);
				}
			else
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				}
			}
		if (responseCode != EMTPRespCodeOK)
			{
			aParameter = i;
			// All properties prior to the failed property will be updated.
			if (i != errIndex && lastHandler)
				{
				TRAPD(err, lastHandler->CommitL());
				if (KErrNone != err)
					{
					lastHandler->RollBack();
					aParameter = errIndex;
					}
				}
			break;
			}
		}
	// Commit all correct properties.
	if (responseCode == EMTPRespCodeOK && lastHandler)
		{
		TRAPD(err, lastHandler->CommitL());
		if (KErrNone != err)
			{
			lastHandler->RollBack();
			responseCode = EMTPRespCodeInvalidObjectHandle;
			aParameter = errIndex;
			}
		}
	CleanupStack::PopAndDestroy(objectMetaData);
    OstTrace1( TRACE_NORMAL, CMTPSVCSETOBJECTPROPLIST_SETOBJECTPROPLISTL, "Exit with responseCode = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCSETOBJECTPROPLIST_SETOBJECTPROPLISTL_EXIT );
	return responseCode;
	}
