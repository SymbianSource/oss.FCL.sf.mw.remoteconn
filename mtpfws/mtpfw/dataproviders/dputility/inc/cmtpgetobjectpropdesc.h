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

#ifndef __CMTPGETOBJECTPROPDESC_H__
#define __CMTPGETOBJECTPROPDESC_H__

#include "cmtprequestprocessor.h"

class CMTPTypeObjectPropDesc;

/** 
Defines file data provider GetObjectPropDesc request processor

@internalTechnology
*/
class CMTPGetObjectPropDesc : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetObjectPropDesc();	
	
private:	
	CMTPGetObjectPropDesc(
					MMTPDataProviderFramework& aFramework,
					MMTPConnection& aConnection);
	void ConstructL();

private:	//from CMTPRequestProcessor
	virtual TMTPResponseCode CheckRequestL();
	virtual void ServiceL();	

private:
	//helper
	void ServiceStorageIdL();
	void ServiceObjectFormatL();
	void ServiceProtectionStatusL();
	void ServiceObjectSizeL();
	void ServiceAssociationTypeL();
	void ServiceAssociationDescL();
	void ServiceFileNameL();
	void ServiceDateModifiedL();
	void ServiceParentObjectL();
	void ServicePuidL();
	void ServiceNameL();
	void ServiceNonConsumableL();
	TUint16 GetPropertyGroupNumber(const TUint16 aPropCode) const;
	
private:
	CMTPTypeObjectPropDesc*	iObjectProperty;
	};
	
#endif

