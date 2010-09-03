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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpgetthumbTraces.h"
#endif



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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETTHUMB_CMTPIMAGEDPGETTHUMB_ENTRY );
    delete iThumb;    
    delete iObjectMeta;
    OstTraceFunctionExit0( CMTPIMAGEDPGETTHUMB_CMTPIMAGEDPGETTHUMB_EXIT );
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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETTHUMB_CONSTRUCTL_ENTRY );
    iThumb = CMTPTypeOpaqueData::NewL();    
    iObjectMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPIMAGEDPGETTHUMB_CONSTRUCTL_EXIT );
    }


TMTPResponseCode CMTPImageDpGetThumb::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETTHUMB_CHECKREQUESTL_ENTRY );
    TMTPResponseCode result = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
    OstTraceFunctionExit0( CMTPIMAGEDPGETTHUMB_CHECKREQUESTL_EXIT );
    return result;	
    }
    

/**
GetObject request handler
*/
void CMTPImageDpGetThumb::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETTHUMB_SERVICEL_ENTRY );
    TInt err = KErrNone;
    
    //at first, try to query thumbnail from property manager
    HBufC8* thumbnailData = imgDp.PropertyMgr().Thumbnail(iObjectMeta->Uint(CMTPObjectMetaData::EHandle));
    if (thumbnailData == NULL)
        {
        OstTrace0( TRACE_NORMAL, CMTPIMAGEDPGETTHUMB_SERVICEL, "CMTPImageDpGetThumb::ServiceL-  fail to query thumbnail from cache" );
        TEntry fileEntry;
        
        LEAVEIFERROR(iFramework.Fs().Entry(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), fileEntry),
                OstTraceExt2( TRACE_ERROR, DUP1_CMTPIMAGEDPGETTHUMB_SERVICEL, 
                        "Gets the entry details for %S failed! error code %d", iObjectMeta->DesC(CMTPObjectMetaData::ESuid), munged_err));
        
        CMTPImageDpThumbnailCreator* tnc = imgDp.ThumbnailManager();
        if(tnc != NULL)
            {
            tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
            if(fileEntry.FileSize() > KFileSizeMax)
                {
            	OstTrace0( TRACE_NORMAL, DUP2_CMTPIMAGEDPGETTHUMB_SERVICEL, "fileEntry.FileSize() > KFileSizeMax" );
                tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
                }
            
            tnc->GetThumbnailL(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), thumbnailData, err);
            imgDp.PropertyMgr().StoreThunmnail(iObjectMeta->Uint(CMTPObjectMetaData::EHandle), thumbnailData);
            }
        //Transfer ownership of thumbnailData to Property Manager       
        }
    if(thumbnailData != NULL)
        {
        iThumb->Write(*thumbnailData);
        }
    SendDataL(*iThumb);
    OstTraceFunctionExit0( CMTPIMAGEDPGETTHUMB_SERVICEL_EXIT );
    }

TBool CMTPImageDpGetThumb::DoHandleCompletingPhaseL()
    {
    return CMTPRequestProcessor::DoHandleCompletingPhaseL();
    }


// End Of File
