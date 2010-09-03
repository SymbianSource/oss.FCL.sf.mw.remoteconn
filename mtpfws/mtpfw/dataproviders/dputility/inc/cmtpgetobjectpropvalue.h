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

#ifndef __CMTPGETOBJECTPROPVALUE_H__
#define __CMTPGETOBJECTPROPVALUE_H__

#include "cmtprequestprocessor.h"
#include <mtp/cmtpobjectmetadata.h>
#include <f32file.h> 

class TMTPTypeUint8;
class TMTPTypeUint16;
class TMTPTypeUint32;
class TMTPTypeUint64;
class TMTPTypeUint128;
class CMTPTypeString;

/** 
Defines file data provider GetObjectPropsSupported request processor

@internalTechnology
*/
class CMTPGetObjectPropValue : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection);	
	IMPORT_C ~CMTPGetObjectPropValue();	
	
private:	
	CMTPGetObjectPropValue(
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
	void ServiceObjectAssociationTypeL();
	void ServiceObjectAssociationDescL();
	void ServiceFileNameL();
	void ServiceDateModifiedL();
	void ServiceParentObjectL();
	void ServicePuidL();
	void ServiceNameL();
	void ServiceNonConsumableL();
	void ServiceHiddenL();
	
private:
	TMTPTypeUint8			iMTPTypeUint8;
	TMTPTypeUint16			iMTPTypeUint16;
	TMTPTypeUint32			iMTPTypeUint32;
	TMTPTypeUint64			iMTPTypeUint64;
	TMTPTypeUint128			iMTPTypeUint128;
	CMTPTypeString*			iMTPTypeString;
	CMTPObjectMetaData*		iObjMeta;
	TEntry 					iFileEntry;
	RFs&					iRfs;
	};
	
#endif

