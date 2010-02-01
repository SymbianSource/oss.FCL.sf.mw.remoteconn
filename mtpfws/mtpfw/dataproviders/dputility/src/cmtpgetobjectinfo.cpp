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
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypestring.h>

#include "cmtpgetobjectinfo.h"
#include "mtpdppanic.h"

/**
Verification data for GetObjectInfo request
*/
const TMTPRequestElementInfo KMTPGetObjectInfoPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 0, 0, 0}
    };

/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
EXPORT_C MMTPRequestProcessor* CMTPGetObjectInfo::NewL(MMTPDataProviderFramework& aFramework,											
											MMTPConnection& aConnection)
	{
	CMTPGetObjectInfo* self = new (ELeave) CMTPGetObjectInfo(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPGetObjectInfo::~CMTPGetObjectInfo()
	{	
	delete iObjectInfoToBuild;
	iDpSingletons.Close();
	}

/**
Standard c++ constructor
*/	
CMTPGetObjectInfo::CMTPGetObjectInfo(MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectInfoPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectInfoPolicy),
	iRfs(aFramework.Fs())
	{
	}

/**
GetObjectInfo request handler
*/		
void CMTPGetObjectInfo::ServiceL()
	{
	BuildObjectInfoL();
	SendDataL(*iObjectInfoToBuild);	
	}

/**
Second-phase construction
*/		
void CMTPGetObjectInfo::ConstructL()
	{
	iObjectInfoToBuild = CMTPTypeObjectInfo::NewL();
	iDpSingletons.OpenL(iFramework);
	}

/**
Populate the object info dataset
*/		
void CMTPGetObjectInfo::BuildObjectInfoL()	
	{
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	//does not take owernship
	CMTPObjectMetaData* objectInfoFromStore = iRequestChecker->GetObjectInfo(objectHandle);
	__ASSERT_DEBUG(objectInfoFromStore, Panic(EMTPDpObjectNull));
	
	TEntry fileEntry;
	User::LeaveIfError(iRfs.Entry(objectInfoFromStore->DesC(CMTPObjectMetaData::ESuid), fileEntry));
			
	//1. storage id
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EStorageID, objectInfoFromStore->Uint(CMTPObjectMetaData::EStorageId));	
	
	//2. object format
	iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EObjectFormat, objectInfoFromStore->Uint(CMTPObjectMetaData::EFormatCode));
	
	//3. protection status,
	iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EProtectionStatus, fileEntry.IsReadOnly());
	
	//4. object compressed size
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EObjectCompressedSize, fileEntry.FileSize());
	
	//5. thumb format	
	iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EThumbFormat, 0);
	
	//6. thumb compressed size
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbCompressedSize, 0);
	
	//7. thumb pix width
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbPixWidth, 0);
	
	//8, thumb pix height
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbPixHeight, 0);
	
	//9. image pix width
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImagePixWidth, 0);
	
	//10. image pix height
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImagePixHeight, 0);
	
	//11. image bit depth
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImageBitDepth, 0);
	
	//12. Parent object
	TUint32 parentHandle = objectInfoFromStore->Uint(CMTPObjectMetaData::EParentHandle);
	// refer to 5.3.1.9 of MTP Spec 1.0 
	if (parentHandle == KMTPHandleNoParent)
	    {
	    parentHandle = 0;
	    }
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EParentObject, parentHandle);
		
	//13 and 14. Association type and description
	iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EAssociationType, objectInfoFromStore->Uint(CMTPObjectMetaData::EFormatSubCode));	
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EAssociationDescription, 0);
		
	//15. sequence number
	iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::ESequenceNumber, 0);
	
	//16. file name
	iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EFilename, fileEntry.iName);
	
	//17. Date created
	TBuf<64> dateString;	
	iDpSingletons.MTPUtility().TTime2MTPTimeStr(fileEntry.iModified, dateString);
	//file modified time
	iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EDateModified, dateString);
	
	//18. Date modified
	//file creation time, set it as the same as modified time, as Symbian does not support this field
	iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EDateCreated, dateString);	
		
	//19. keyword
	iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EKeywords, KNullDesC);
	}

