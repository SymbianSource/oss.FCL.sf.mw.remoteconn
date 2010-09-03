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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgedpgetobjectinfoTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECTINFO_CMTPPICTBRIDGEDPGETOBJECTINFO_DES_ENTRY );
    delete iObjectInfoToBuildP;
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECTINFO_CMTPPICTBRIDGEDPGETOBJECTINFO_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECTINFO_SERVICEL_ENTRY );
    BuildObjectInfoL();
    SendDataL(*iObjectInfoToBuildP);    
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECTINFO_SERVICEL_EXIT );
    }

/**
Second-phase construction
*/      
void CMTPPictBridgeDpGetObjectInfo::ConstructL()
    {
	OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECTINFO_CONSTRUCTL_ENTRY );
    iObjectInfoToBuildP = CMTPTypeObjectInfo::NewL();
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECTINFO_CONSTRUCTL_EXIT );
    }

/**
Populate the object info dataset
*/      
void CMTPPictBridgeDpGetObjectInfo::BuildObjectInfoL()  
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL_ENTRY );
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPPictBridgeDpRequestCheckNull));

    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);

    OstTrace1( TRACE_NORMAL, CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL, "object handle 0x%x", objectHandle );
    
    //1. storage id
    TUint32 storageId(iFramework.StorageMgr().DefaultStorageId()); // we always use default storage for DPS

    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EStorageID, storageId); 

    //2. object format
    TUint16 format(EMTPFormatCodeScript); // we only handle DPS script
    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EObjectFormat, format); 
    OstTrace1( TRACE_NORMAL, DUP1_CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL, "format ok 0x%x", format );

    //3. protection status,
    TUint16 protection(EMTPProtectionNoProtection); // we do not care about protection

    iObjectInfoToBuildP->SetUint16L(CMTPTypeObjectInfo::EProtectionStatus, protection);
    OstTrace1( TRACE_NORMAL, DUP2_CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL, " protection(%d) ok", protection );

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
    // refer to 5.3.1.9 of MTP Spec 1.0 
    if (parent == KMTPHandleNoParent)
	{
	parent = 0;
	} 
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
    LEAVEIFERROR( parse.Set(objectP->DesC(CMTPObjectMetaData::ESuid), NULL, NULL),
            OstTraceExt2( TRACE_ERROR, DUP3_CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL, 
                    "Sets up TParse object for %S failed! error code %d", objectP->DesC(CMTPObjectMetaData::ESuid), munged_err));
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EFilename, parse.NameAndExt());
    
    //4, compressed size, 17 Date created, and 18 Date modified
    SetFileSizeDateL(objectP->DesC(CMTPObjectMetaData::ESuid), (objectHandle==iPictBridgeDP.PtpServer()->DeviceDiscoveryHandle())); 
    
    //18. keyword
    //empty keyword
    iObjectInfoToBuildP->SetStringL(CMTPTypeObjectInfo::EKeywords, KNullDesC);
    CleanupStack::PopAndDestroy(objectP);
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECTINFO_BUILDOBJECTINFOL_EXIT );
    }


/**
Set file properties
*/ 
void CMTPPictBridgeDpGetObjectInfo::SetFileSizeDateL(const TDesC& aFileName, TBool aDiscoveryFile)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, 
            "aDiscoveryFile %d %S", aDiscoveryFile, aFileName );

    // open the file for retrieving information
    RFile file;
    TInt size(0);
    TTime modifiedTime=0;
    
    if (!aDiscoveryFile)
        {
        LEAVEIFERROR(file.Open(iFramework.Fs(), aFileName, EFileShareReadersOnly),
                OstTraceExt2( TRACE_ERROR, DUP3_CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, 
                        "Open %S failed! error code %d", aFileName, munged_err ));
        CleanupClosePushL(file);
        
        //file size
        LEAVEIFERROR(file.Size(size),
                OstTrace1( TRACE_ERROR, DUP4_CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, 
                        "Get current file size failed! error code %d", munged_err));

        //file modified time
        LEAVEIFERROR(file.Modified(modifiedTime),
                OstTrace1( TRACE_ERROR, DUP5_CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, 
                        "Gets local date and time the file was last modified failed! error code %d", munged_err));
        }
    else
        {
        //file modified time
        modifiedTime.HomeTime();
        }

    //file size
    TUint32 fileSize=size;
    iObjectInfoToBuildP->SetUint32L(CMTPTypeObjectInfo::EObjectCompressedSize, fileSize);
    OstTrace1( TRACE_NORMAL, DUP2_CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, "file size %d", fileSize );

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

    OstTraceExt1( TRACE_NORMAL, DUP1_CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL, "%S", modifiedTimeBuffer );
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECTINFO_SETFILESIZEDATEL_EXIT );
    }

