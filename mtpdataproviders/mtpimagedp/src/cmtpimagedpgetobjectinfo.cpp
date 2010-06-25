// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypestring.h>

#include "cmtpimagedpgetobjectinfo.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedp.h"
#include "mtpimagedputilits.h"

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDpGetObjectInfo");)

/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPImageDpGetObjectInfo::NewL(MMTPDataProviderFramework& aFramework,											
                                            MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetObjectInfo* self = new (ELeave) CMTPImageDpGetObjectInfo(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor
*/	
CMTPImageDpGetObjectInfo::~CMTPImageDpGetObjectInfo()
    {
    __FLOG(_L8(">> ~CMTPImageDpGetObjectInfo"));
    delete iObjectInfoToBuild;
    delete iObjectMeta;
    __FLOG(_L8("<< ~CMTPImageDpGetObjectInfo"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/	
CMTPImageDpGetObjectInfo::CMTPImageDpGetObjectInfo(MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) : 
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iObjectPropertyMgr(aDataProvider.PropertyMgr())
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPImageDpGetObjectInfo::CMTPImageDpGetObjectInfo"));
    }

TMTPResponseCode CMTPImageDpGetObjectInfo::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObject::CheckRequestL"));
    TMTPResponseCode result = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
    __FLOG(_L8("<< CMTPImageDpGetObject::CheckRequestL"));
    return result;
    }
    
/**
GetObjectInfo request handler
*/		
void CMTPImageDpGetObjectInfo::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::ServiceL"));
    BuildObjectInfoL();
    SendDataL(*iObjectInfoToBuild);	
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::ServiceL"));
    }
    
/**
Second-phase construction
*/		
void CMTPImageDpGetObjectInfo::ConstructL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::ConstructL"));
    iObjectInfoToBuild = CMTPTypeObjectInfo::NewL();
    iObjectMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::ConstructL"));
    }
    
/**
Populate the object info dataset
*/		
void CMTPImageDpGetObjectInfo::BuildObjectInfoL()	
    {
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::BuildObjectInfoL"));
    iObjectPropertyMgr.SetCurrentObjectL(*iObjectMeta, EFalse);
    
    //1. storage id
    TUint32 storageId;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeStorageID, storageId);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EStorageID, storageId);	
    
    //2. object format
    TUint16 format;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectFormat, format);
    iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EObjectFormat, format);
    
    //3. protection status,
    TUint16 protection;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeProtectionStatus, protection);
    iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EProtectionStatus, protection);
    
    //4. object compressed size
    // see SetFileSizeDateL
    
    //5. thumb format	
    //6. thumb compressed size
    //7. thumb pix width
    //8, thumb pix height
    //9. image pix width
    //10. image pix height
    //11. image bit depth
    TUint16 thumbFormat;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleFormat, thumbFormat);
    iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EThumbFormat, thumbFormat);
    TUint32 value(0);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleSize, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbCompressedSize, value);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleWidth, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbPixWidth, value);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeRepresentativeSampleHeight, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EThumbPixHeight, value);
    
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeWidth, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImagePixWidth, value);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeHeight, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImagePixHeight, value);
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeImageBitDepth, value);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EImageBitDepth, value);
    
    //12. Parent object
    TUint32 parent;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeParentObject, parent);
    if(KMTPHandleNoParent == parent)
        {
        parent = KMTPHandleNone;
        }
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EParentObject, parent);
        
    //13 and 14. Association type and description
    iObjectInfoToBuild->SetUint16L(CMTPTypeObjectInfo::EAssociationType, 0);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EAssociationDescription, 0);
        
    //15. sequence number
    //TUint32 sequenceNum;
    //iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeAssociationDesc, sequenceNum);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::ESequenceNumber, 0);
    
    //16. file name
    SetFileNameL();
    
    //4, compressed size, 17 Date created, and 18 Date modified
    SetFileSizeDateL();
    
    //18. keyword
    SetKeywordL();
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::BuildObjectInfoL"));
    }
/**
Set the file name of the current object in the data set
@param aObjectInfo The object info of the current object
*/	
void CMTPImageDpGetObjectInfo::SetFileNameL()
    {
    //use the name without full path specification
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::SetFileNameL"));
    CMTPTypeString* fileName = CMTPTypeString::NewLC();
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectFileName, *fileName);
    iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EFilename, fileName->StringChars());
    CleanupStack::PopAndDestroy(fileName);
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::SetFileNameL"));
    }
    
/**
Set the file size and modified/created date in the data set
@param aObjectInfo The object info of the current object
*/	
void CMTPImageDpGetObjectInfo::SetFileSizeDateL()
    {
    //file size
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::SetFileSizeDateL"));
    TUint64 fileSize;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeObjectSize, fileSize);
    TUint32 shortFileSize = (fileSize > KMaxTUint32) ? KMaxTUint32 : static_cast<TUint32>(fileSize);
    iObjectInfoToBuild->SetUint32L(CMTPTypeObjectInfo::EObjectCompressedSize, shortFileSize);
    
    //file modified time
    CMTPTypeString* dateString = CMTPTypeString::NewLC();
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeDateModified, *dateString);
    iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EDateModified, dateString->StringChars());
    
    CMTPTypeString* createdString = CMTPTypeString::NewLC();
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeDateCreated, *createdString);
    iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EDateCreated, createdString->StringChars());	
    
    CleanupStack::PopAndDestroy(2); // createdString, dateString
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::SetFileSizeDateL"));
    }
    
/**
Set the keyword of the current object in the data set
@param aObjectInfo The object info of the current object
*/	
void CMTPImageDpGetObjectInfo::SetKeywordL()
    {
    //empty keyword
    __FLOG(_L8(">> CMTPImageDpGetObjectInfo::SetKeywordL"));
    iObjectInfoToBuild->SetStringL(CMTPTypeObjectInfo::EKeywords, KNullDesC);
    __FLOG(_L8("<< CMTPImageDpGetObjectInfo::SetKeywordL"));
    }
