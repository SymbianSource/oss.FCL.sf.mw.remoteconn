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

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpgetobjecthandles.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"


/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetObjectHandles::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetObjectHandles* self = new (ELeave) CMTPGetObjectHandles(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
GetObjectHandles request handler
*/	
CMTPGetObjectHandles::~CMTPGetObjectHandles()
	{
	delete iHandles;	
	}

/**
Standard c++ constructor
*/	
CMTPGetObjectHandles::CMTPGetObjectHandles(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPGetNumObjects(aFramework, aConnection)
	{
	
	}
    
/**
Second phase constructor.
*/
void CMTPGetObjectHandles::ConstructL()
    {
    CMTPGetNumObjects::ConstructL();
    }

/**
GetObjectHandles request handler
*/	
void CMTPGetObjectHandles::ServiceL()
	{
	RMTPObjectMgrQueryContext   context;
	RArray<TUint>               handles;
	TMTPObjectMgrQueryParams    params(Request().Uint32(TMTPTypeRequest::ERequestParameter1), Request().Uint32(TMTPTypeRequest::ERequestParameter2), Request().Uint32(TMTPTypeRequest::ERequestParameter3));
	CleanupClosePushL(context);
	CleanupClosePushL(handles);
	
	delete iHandles;
	iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	
	do
	    {
    	iFramework.ObjectMgr().GetObjectHandlesL(params, context, handles);
    	iHandles->AppendL(handles);
	    }
	while (!context.QueryComplete());
	
	CleanupStack::PopAndDestroy(&handles);
	CleanupStack::PopAndDestroy(&context);					
	SendDataL(*iHandles);	
	}






	

	


   	

	






