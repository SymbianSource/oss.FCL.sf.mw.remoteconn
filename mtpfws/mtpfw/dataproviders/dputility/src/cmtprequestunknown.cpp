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
#include "cmtprequestunknown.h"

/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPRequestUnknown::NewL( 
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPRequestUnknown* self = new (ELeave) CMTPRequestUnknown(aFramework, aConnection);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPRequestUnknown::~CMTPRequestUnknown()
	{
		
	}

/**
Standard c++ constructor
*/	
EXPORT_C CMTPRequestUnknown::CMTPRequestUnknown(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
	{
	}
	
/**
OpenSession request handler
*/	
EXPORT_C void CMTPRequestUnknown::ServiceL()	
	{
	SendResponseL(EMTPRespCodeOperationNotSupported);	
	}

/**
Override to match both the unknown requests
@param aRequest	The request to match
@param aConnection The connection from which the request comes
@return always return ETrue to process all unknown request
*/		
EXPORT_C TBool CMTPRequestUnknown::Match(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/) const
	{
	return ETrue;	
	}









	

	


   	

	






