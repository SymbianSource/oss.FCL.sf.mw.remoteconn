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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcSetObjPropList");)

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
	__FLOG(_L8("~CMTPSvcSetObjectPropList - Entry"));
	delete iPropertyList;
	__FLOG(_L8("~CMTPSvcSetObjectPropList - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcSetObjectPropList::CMTPSvcSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider)
	{
	}

void CMTPSvcSetObjectPropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iPropertyList = CMTPTypeObjectPropList::NewL();
	__FLOG(_L8("ConstructL - Exit"));
	}

void CMTPSvcSetObjectPropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	ReceiveDataL(*iPropertyList);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcSetObjectPropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TUint32 parameter = 0;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	responseCode = SetObjectPropListL(*iPropertyList, parameter);
	__FLOG_VA((_L8("SetObjectPropListL - ResponsCode: 0x%x, error index: %u"), responseCode, parameter));
	
	SendResponseL(responseCode, 1, &parameter);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with responseCode = 0x%04X and failed index: %u"), responseCode, parameter));
	return EFalse;
	}

TMTPResponseCode CMTPSvcSetObjectPropList::SetObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParameter)
	{
	__FLOG(_L8("SetObjectPropListL - Entry"));
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
	__FLOG_VA((_L8("SetObjectPropListL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}
