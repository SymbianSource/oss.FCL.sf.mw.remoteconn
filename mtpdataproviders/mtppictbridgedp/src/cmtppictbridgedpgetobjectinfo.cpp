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


#include <f32file.h>
#include <imageconversion.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpstoragemgr.h>
#include "cmtppictbridgedpgetobjectinfo.h"
#include "mtppictbridgedpconst.h"
#include "mtppictbridgedppanic.h"
#include "cmtprequestchecker.h"
#include "cptpserver.h"

/**
Two-phase construction method
*/ 
MMTPRequestProcessor* CMTPPictBridgeDpGetObjectInfo::NewL(
    MMTPDataProviderFramework& aFramework,              
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider)
    {
    CMTPPictBridgeDpGetObjectInfo* self = new (ELeave) CMTPPictBridgeDpGetObjectInfo(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/  
CMTPPictBridgeDpGetObjectInfo::~CMTPPictBridgeDpGetObjectInfo()
    {   
    __FLOG(_L8("CMTPPictBridgeDpGetObjectInfo::~CMTPPictBridgeDpGetObjectInfo"));
    delete iObjectInfoToBuildP;
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/  
CMTPPictBridgeDpGetObjectInfo::CMTPPictBridgeDpGetObjectInfo(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL ),
    iPictBridgeDP(aDataProvider)
    {
    }

/**
GetObjectInfo request handler
*/
void CMTPPictBridgeDpGetObjectInfo::ServiceL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpGetObjectInfo::ServiceL"));
    BuildObjectInfoL();
    SendDataL(*iObjectInfoToBuildP);    
    __FLOG(_L8("<< CMTPPictBridgeDpGetObjectInfo::ServiceL"));
    }

/**
Second-phase construction
*/      
void CMTPPictBridgeDpGetObjectInfo::ConstructL()
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    iObjectInfoToBuildP = CMTPTypeObjectInfo::NewL();
    }

/**
Populate the object info dataset
*/      
void CMTPPictBridgeDpGetObjectInfo::BuildObjectInfoL()  
    {
    __FLOG(_L8(">> CMTPPictBridgeDpGetObjectInfo::BuildObjectInfoL"));
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPPictBridgeDpRequestCheckNull));

    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);

    __FLOG_VA((_L8(" object handle 0x%x"), objectHandle));    

    //1. storage id
    TUint32 storageId(iFramework.StorageMgr().DefaultStorageId()); // we always use default storage for DPS

    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EStorageID, storageId); 

    //2. object format
    TUint16 format(EMTPFormatCodeScript); // we only handle DPS script
    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EObjectFormat, format);
    __FLOG_VA((_L8(" format ok 0x%x"), format));   

    //3. protection status,
    TUint16 protection(EMTPProtectionNoProtection); // we do not care about protection

    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EProtectionStatus, protection);
    __FLOG_VA((_L8(" protection(%d) ok"), protection));   

    //4. object compressed size
    // see SetFileSizeDateL

    //5. thumb format      
    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EThumbFormat, 0);
    //6. thumb compressed size
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EThumbCompressedSize, 0);
    //7. thumb pix width
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EThumbPixWidth, 0);
    //8, thumb pix height
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EThumbPixHeight, 0);
    //9. image pix width
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EImagePixWidth, 0);
    //10. image pix height
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EImagePixHeight, 0);
    //11. image bit depth
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EImageBitDepth, 0);
   
    CMTPObjectMetaData* objectP = CMTPObjectMetaData::NewL();
    CleanupStack::PushL(objectP);

    iFramework.ObjectMgr().ObjectL(objectHandle, *objectP);

    //12. Parent object
    TUint32 parent(objectP->Uint(CMTPObjectMetaData::EParentHandle)); 
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EParentObject, parent);
        
    //13 and 14. Association type and description
    TUint16 associationType(EMTPAssociationTypeUndefined);
    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EAssociationType, associationType); 
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EAssociationDescription, 0);
        
    //15. sequence number
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::ESequenceNumber, 0);
    
    //16. file name
    //use the name without full path specification
    TParse parse;
    User::LeaveIfError( parse.Set(objectP->DesC(CMTPObjectMetaData::ESuid), NULL, NULL) );    
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EFilename, parse.NameAndExt());
    
    //4, compressed size, 17 Date created, and 18 Date modified
    SetFileSizeDateL(objectP->DesC(CMTPObjectMetaData::ESuid), (objectHandle==iPictBridgeDP.PtpServer()->DeviceDiscoveryHandle())); 
    
    //18. keyword
    //empty keyword
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EKeywords, KNullDesC);
    CleanupStack::PopAndDestroy(objectP);
    __FLOG(_L8("<< CMTPPictBridgeDpGetObjectInfo::BuildObjectInfoL"));
    }


/**
Set file properties
*/ 
void CMTPPictBridgeDpGetObjectInfo::SetFileSizeDateL(const TDesC& aFileName, TBool aDiscoveryFile)
    {
    __FLOG_VA((_L16(">> CMTPPictBridgeDpGetObjectInfo::SetFileSizeDateL aDiscoveryFile %d %S"), aDiscoveryFile, &aFileName));

    // open the file for retrieving information
    RFile file;
    TInt size(0);
    TTime modifiedTime=0;
    
    if (!aDiscoveryFile)
        {
        User::LeaveIfError(file.Open(iFramework.Fs(), aFileName, EFileShareReadersOnly));
        CleanupClosePushL(file);
        
        //file size
        User::LeaveIfError(file.Size(size));
        
        //file modified time
        User::LeaveIfError(file.Modified(modifiedTime));
        }
    else
        {
        //file modified time
        modifiedTime.HomeTime();
        }

    //file size
    TUint32 fileSize=size;
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EObjectCompressedSize, fileSize);
    __FLOG_VA((_L8(" file size %d"), fileSize));   

    //file modified time
    const TInt KTimeStringLen=0x0f;// YYYYMMDDThhmmss(.s), we exclude tenths of seconds and use length 15, (MTP 1.0 spec, section 3.2.5)
    TBuf<KTimeStringLen> modifiedTimeBuffer;
    _LIT(KTimeFormat,"%Y%M%DT%H%M%T%S");
    modifiedTime.FormatL(modifiedTimeBuffer, KTimeFormat);

    CMTPTypeString* dateString = CMTPTypeString::NewLC(modifiedTimeBuffer);
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EDateModified, dateString->StringChars());
    //file creation time, set it as the same as modified time, as Symbian does not support this field
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EDateCreated, dateString->StringChars());   
    CleanupStack::PopAndDestroy(dateString);
    
    if (!aDiscoveryFile)
        {
        CleanupStack::PopAndDestroy(&file);
        }
    
    __FLOG_VA((_L16("<< CMTPPictBridgeDpGetObjectInfo::SetFileSizeDateL %S"),&modifiedTimeBuffer));
    }

