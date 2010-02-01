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



#include <mtp/mtpdatatypeconstants.h>
#include <mtp/cmtptypeinterdependentpropdesc.h>
#include "cmtpgetinterdependentpropdesc.h"



/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/  
EXPORT_C MMTPRequestProcessor* CMTPGetInterDependentPropDesc::NewL(MMTPDataProviderFramework& aFramework,
														MMTPConnection& aConnection)
	{
	CMTPGetInterDependentPropDesc* self = new (ELeave) CMTPGetInterDependentPropDesc(aFramework, aConnection);
	return self;	
	}

/**
 Destructor
*/
EXPORT_C CMTPGetInterDependentPropDesc::~CMTPGetInterDependentPropDesc()
	{
	delete iDataset;
	}

/**
Standard c++ constructor
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
*/
CMTPGetInterDependentPropDesc::CMTPGetInterDependentPropDesc(MMTPDataProviderFramework& aFramework,
															MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL)														
	{
	}


/**
 GetInterDependentPropDesc request handler
*/
void CMTPGetInterDependentPropDesc::ServiceL()
	{
	// Clear the data set.
	delete iDataset;
	iDataset = NULL;
	iDataset = CMTPTypeInterdependentPropDesc::NewL();
    
	// Send the dataset.
	SendDataL(*iDataset);
	}
	

