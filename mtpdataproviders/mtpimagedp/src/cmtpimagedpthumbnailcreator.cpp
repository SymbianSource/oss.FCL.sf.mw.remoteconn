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

#include <e32base.h>
#include <e32property.h>
#include <fbs.h>
#include <caf/content.h>
#include <icl/imagedata.h>
#include <sysutil.h>
#include <pathinfo.h> // PathInfo
#include <bautils.h> // FileExists
#include <mtp/cmtptypeopaquedata.h>
#include "cmtpimagedpthumbnailcreator.h"
#include "mtpimagedpconst.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpthumbnailcreatorTraces.h"
#endif


// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::NewL
// 2-phased constructor.
// --------------------------------------------------------------------------
//
CMTPImageDpThumbnailCreator* CMTPImageDpThumbnailCreator::NewL(CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpThumbnailCreator* self= new (ELeave) CMTPImageDpThumbnailCreator(aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
 
// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::~CMTPImageDpThumbnailCreator
// C++ destructor.
// --------------------------------------------------------------------------
//    
CMTPImageDpThumbnailCreator::~CMTPImageDpThumbnailCreator()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_CMTPIMAGEDPTHUMBNAILCREATOR_DES_ENTRY );
    Cancel();
    if(EGetting == iState)
        {
        iThumbMgr->CancelRequest(iCurrentReq);
        }
    
    delete iData;
    iData = NULL;
    	
    delete iImgEnc;
    iImgEnc = NULL;
    
#ifdef MTPTHUMBSCALING
    delete iScaler;
    iScaler = NULL;
#endif
    
    delete iBitmap;  
    iBitmap = NULL;  
    
    delete iObjectSource;
    iObjectSource = NULL;
    	
    delete iThumbMgr;
    iThumbMgr = NULL;
    
    if(iActiveSchedulerWait != NULL && iActiveSchedulerWait->IsStarted())
        {
        *iCreationErr = KErrNotReady;
        iActiveSchedulerWait->AsyncStop();
        }
    
    delete iActiveSchedulerWait;
    iActiveSchedulerWait = NULL;
    
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_CMTPIMAGEDPTHUMBNAILCREATOR_DES_EXIT );
    }
 
// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::CMTPImageDpThumbnailCreator
// C++ constructor.
// --------------------------------------------------------------------------
//    
CMTPImageDpThumbnailCreator::CMTPImageDpThumbnailCreator(CMTPImageDataProvider& aDataProvider): 
    CActive(EPriorityStandard),
    iDataProvider(aDataProvider)
    {
    OstTraceFunctionEntry0( DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_CMTPIMAGEDPTHUMBNAILCREATOR_CONS_ENTRY );
    CActiveScheduler::Add(this);  
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_CMTPIMAGEDPTHUMBNAILCREATOR_CONS_EXIT );
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::ConstructL
// 2nd phase constructor.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_CONSTRUCTL_ENTRY );
    iThumbMgr = CThumbnailManager::NewL( *this ); 
    iThumbMgr->SetThumbnailSizeL( EGridThumbnailSize );
#ifdef MTPTHUMBSCALING
    iScaler = CBitmapScaler::NewL();
#endif    
    iActiveSchedulerWait = new (ELeave) CActiveSchedulerWait();
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_CONSTRUCTL_EXIT );
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::DoCancel
// From CActive.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_DOCANCEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPTHUMBNAILCREATOR_DOCANCEL, "iState %d iStatus 0x%X", iState, iStatus.Int() );
    switch(iState)
        {
#ifdef MTPTHUMBSCALING
        case EScaling:
            iScaler->Cancel();
            break;
#endif
        case EEncoding:
            iImgEnc->Cancel();
            break;
        default:
            break;
        }
    if(iActiveSchedulerWait->IsStarted())
        {
        *iCreationErr = KErrCancel;
        iActiveSchedulerWait->AsyncStop();
        }
    // we will not continue creating thumbs.
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_DOCANCEL_EXIT );
    OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_DOCANCEL, "iState %d", iState);
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::RunL
// From CActive.
// --------------------------------------------------------------------------
//    
void CMTPImageDpThumbnailCreator::RunL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_RUNL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, "iState %d iStatus 0x%X", iState, iStatus.Int() );

    LEAVEIFERROR(iStatus.Int(),
            OstTrace1(TRACE_ERROR, DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, "iStatus %d is not correct!", iStatus.Int() ));
            
    switch (iState)
        { 
#ifdef MTPTHUMBSCALING
        case EGetted:
            {
            ScaleBitmap();
            iState = EScaling;
            break;
            }
#endif
        case EScaling:
            {
            EncodeImageL( );
            iState=EEncoding;
            break;
            }
        case EEncoding:
            {
            iState=EIdle;
            if (iThumbMgr->Flags() == CThumbnailManager::EDoNotCreate)
                {
                OstTrace1( TRACE_NORMAL, DUP2_CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, 
                        "CMTPImageDpThumbnailCreator::RunL(),EDoNotCreate; iState %d", iState );
                
                delete iData;
                iData = NULL;
                iData = HBufC8::NewL(1);
                }

            OstTrace1( TRACE_NORMAL, DUP3_CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, "iBuffer->Write(*iData); iState %d", iState );
            if(iActiveSchedulerWait->IsStarted())
                {
                iActiveSchedulerWait->AsyncStop();
                }
            break;
            }
        default:
            {
            OstTrace1( TRACE_ERROR, DUP4_CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, "Invalid iState %d", iState );
            User::Leave(KErrGeneral);
            break;
            }
        }
    OstTrace1( TRACE_NORMAL, DUP5_CMTPIMAGEDPTHUMBNAILCREATOR_RUNL, "iState %d", iState );
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_RUNL_EXIT );
    }

// --------------------------------------------------------------------------
// RunError
// --------------------------------------------------------------------------
//    
TInt CMTPImageDpThumbnailCreator::RunError(TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_RUNERROR_ENTRY );
    OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPIMAGEDPTHUMBNAILCREATOR_RUNERROR, "err 0x%X", aErr );
    iState=EIdle;
    if(iActiveSchedulerWait->IsStarted())
        {
        *iCreationErr = aErr;
        iActiveSchedulerWait->AsyncStop();
        }
    // no need to cancel iScalerP since only leave is issued if scaler creation fails
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_RUNERROR_EXIT );
    return KErrNone;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::GetThumbnailL(const TDesC& aFileName, HBufC8*& aDestinationData,  TInt& result)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_GETTHUMBNAILL_ENTRY );
    GetThumbL(aFileName);
    iCreationErr = &result;     //reset the err flag
    *iCreationErr = KErrNone;
    iActiveSchedulerWait->Start();
    
    /**
     * transfer the ownership of iData if query successfully
     */
    if (*iCreationErr == KErrNone)
        {
        aDestinationData = iData;
        iData = NULL;        
        }
    else
        {
        aDestinationData = NULL;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_GETTHUMBNAILL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::ClearThumbnailData()
    {
    delete iData;
    iData = NULL;
    delete iBitmap;
    iBitmap = NULL;
    delete iObjectSource;
    iObjectSource = NULL;
    delete iImgEnc;
    iImgEnc = NULL;
    }

void CMTPImageDpThumbnailCreator::GetThumbL(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_GETTHUMBL_ENTRY );
    // Create an object source representing a path to a file on local
    // file system.
    delete iObjectSource;
    iObjectSource = NULL;
    
    TParsePtrC parse(aFileName);
    if (parse.Ext().Length() >= 1)
        {
        const TDesC& mimeType = iDataProvider.FindMimeType(parse.Ext().Mid(1));
        OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPTHUMBNAILCREATOR_GETTHUMBL, 
                "CMtpImageDphumbnailCreator::GetThumbL() - FileName:%S, MimeType:%S", aFileName, mimeType );
    
        iObjectSource = CThumbnailObjectSource::NewL(aFileName, mimeType);
        }
    else
        {
        iObjectSource = CThumbnailObjectSource::NewL(aFileName, KNullDesC);
        }
    iCurrentReq = iThumbMgr->GetThumbnailL( *iObjectSource );
    iState = EGetting;
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_GETTHUMBL_EXIT );
    }

#ifdef MTPTHUMBSCALING
// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::ScaleBitmapL
// Scales the bitmap to the thumbnail size.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::ScaleBitmap()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_SCALEBITMAP_ENTRY );
    TSize size( KThumbWidht, KThumbHeigth ); // size 160x120      
    // Resize image to thumbnail size 
//    iScaler->Scale( &iStatus, *iBitmap, size );
    
    /**
     * [Thumbnail SIZE]: performance improvement
     * comments scaling code, but it breaks PTP spect.
     * so if we meet any break of compatible test, we should re-scale thumbnail size
     */    
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    
    SetActive();
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_SCALEBITMAP_EXIT );
    }
#endif

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::EncodeImageL
// Encodes bitmap as a jpeg image.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::EncodeImageL( )
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_ENCODEIMAGEL_ENTRY );

    delete iData;
    iData = NULL;
    
    delete iImgEnc;
    iImgEnc = NULL;
    
    // Convert bitmap to jpg
    iImgEnc = CImageEncoder::DataNewL( iData, KPtpMimeJPEG, CImageEncoder::EPreferFastEncode );
    iImgEnc->Convert( &iStatus, *iBitmap );
    SetActive();
    OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_ENCODEIMAGEL_EXIT );
    }

//
//
void CMTPImageDpThumbnailCreator::ThumbnailReady( TInt aError, MThumbnailData& aThumbnail, TThumbnailRequestId aId )
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPTHUMBNAILCREATOR_THUMBNAILREADY_ENTRY );
    // This function must not leave.
    if(iCurrentReq != aId)
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_THUMBNAILREADY, 
                "CMTPImageDpThumbnailCreator::ThumbnailReady(),iCurrentReq != aId" );
        OstTraceFunctionExit0( CMTPIMAGEDPTHUMBNAILCREATOR_THUMBNAILREADY_EXIT );
        return;
        }
    if (aError == KErrNone)
        {
        TRAP_IGNORE(iThumbMgr->SetFlagsL(CThumbnailManager::EDefaultFlags));
        delete iBitmap;
        // Claim ownership of the bitmap instance for later use
        iBitmap = aThumbnail.DetachBitmap();
#ifdef MTPTHUMBSCALING
        iState = EGetted;
#else
        iState = EScaling;				//direct set to Scaling state jump the scaling function
#endif
        }
    else if ((iThumbMgr->Flags() == CThumbnailManager::EDoNotCreate) && (aError == KErrNotFound))
        {
        OstTrace0( TRACE_NORMAL, CMTPIMAGEDPTHUMBNAILCREATOR_THUMBNAILREADY, 
                "CMTPImageDpThumbnailCreator::ThumbnailReady(),EDoNotCreate, KErrNotFound" );
        iState = EEncoding;
        //don't trigger TNM to create thumbnail if image files are too big
        //iThumbMgr->CreateThumbnails(*iObjectSource);
        aError = KErrNone;
        }
    iStatus=KRequestPending;
    TRequestStatus* status=&iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPTHUMBNAILCREATOR_THUMBNAILREADY_EXIT );
    }

void CMTPImageDpThumbnailCreator::ThumbnailPreviewReady( MThumbnailData& /*aThumbnail*/, TThumbnailRequestId /*aId*/ )
    {
    
    }

// End of file
