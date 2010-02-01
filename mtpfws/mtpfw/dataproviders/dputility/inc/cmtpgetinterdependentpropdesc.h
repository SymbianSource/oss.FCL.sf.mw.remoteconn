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

/**
 @file
 @internalTechnology
*/

#ifndef CMTPGETINTERDEPENDENTPROPDESC_H
#define CMTPGETINTERDEPENDENTPROPDESC_H

#include "cmtprequestprocessor.h"

class CMTPTypeInterdependentPropDesc;

/** 
Defines file data provider GetInterDependentPropDesc request processor

@internalTechnology
*/
class CMTPGetInterDependentPropDesc : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetInterDependentPropDesc();	
	
private:	
	CMTPGetInterDependentPropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);

private:	//from CMTPRequestProcessor
	virtual void ServiceL();
	
private:
	/** Dataset to store the prop descriptions*/
    CMTPTypeInterdependentPropDesc* iDataset;
	};
	
#endif

