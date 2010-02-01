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

#include <bautils.h>  // FileExists

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtptypeopaquedata.h>
#include <thumbnailmanager.h>

#include "cmtpimagedpgetthumb.h"
#include "mtpimagedppanic.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedpthumbnailcreator.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "cmtpimagedp.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ImageDpGetThumb");)


/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
MMTPRequestProcessor* CMTPImageDpGetThumb::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetThumb* self = new (ELeave) CMTPImageDpGetThumb(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/	
CMTPImageDpGetThumb::~CMTPImageDpGetThumb()
    {
    __FLOG(_L8(">> CMTPImageDpGetThumb::~CMTPImageDpGetThumb"));
    delete iThumb;
    delete iObjectMeta;
    __FLOG(_L8("<< CMTPImageDpGetThumb::~CMTPImageDpGetThumb"));
    __FLOG_CLOSE;
    }
    
/**
Standard c++ constructor
*/	
CMTPImageDpGetThumb::CMTPImageDpGetThumb(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) : 
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),imgDp(aDataProvider)
    {
    
    }

/**
Second-phase constructor.
*/        
void CMTPImageDpGetThumb::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPImageDpGetThumb::ConstructL"));
    iThumb = CMTPTypeOpaqueData::NewL();
    iObjectMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("CMTPImageDpGetThumb::ConstructL"));
    }


TMTPResponseCode CMTPImageDpGetThumb::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpGetThumb::CheckRequestL"));
    TMTPResponseCode result = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
    __FLOG(_L8("<< CMTPImageDpGetThumb::CheckRequestL"));
    return result;	
    }
    

/**
GetObject request handler
*/
void CMTPImageDpGetThumb::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpGetThumb::ServiceL"));
    TInt err = KErrNone;
    TEntry fileEntry;
    
    User::LeaveIfError(iFramework.Fs().Entry(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), fileEntry));
    imgDp.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
    if(fileEntry.iSize > KFileSizeMax)
        {
        __FLOG(_L8(">> CMTPImageDpGetThumb::ServiceL, fileEntry.iSize > KFileSizeMax"));
        imgDp.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
        }
    imgDp.ThumbnailManager().GetThumbnailL(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), *iThumb, err);
    User::LeaveIfError(err);
    SendDataL(*iThumb);
    __FLOG(_L8("<< CMTPImageDpGetThumb::ServiceL"));
    }

TBool CMTPImageDpGetThumb::DoHandleCompletingPhaseL()
    {
    return CMTPRequestProcessor::DoHandleCompletingPhaseL();
    }


// End Of File
