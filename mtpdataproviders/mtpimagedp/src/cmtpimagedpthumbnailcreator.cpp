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

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDpThumbnailCreator");)
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
    __FLOG(_L8(">> ~CMTPImageDpThumbnailCreator"));
    Cancel();
    if(EGetting == iState)
        {
        iThumbMgr->CancelRequest(iCurrentReq);
        }
    delete iData;
    delete iImgEnc;
#ifdef MTPTHUMBSCALING
    delete iScaler;
#endif
    delete iBitmap;  
    delete iObjectSource;
    delete iThumbMgr;
    if(iActiveSchedulerWait->IsStarted())
        {
        *iCreationErr = KErrNotReady;
        iActiveSchedulerWait->AsyncStop();
        }
    delete iActiveSchedulerWait;
    __FLOG(_L8("<< ~CMTPImageDpThumbnailCreator"));
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPImageDpThumbnailCreator::CMTPImageDpThumbnailCreator(), begin"));
    CActiveScheduler::Add(this);  
    __FLOG(_L8("CMTPImageDpThumbnailCreator::CMTPImageDpThumbnailCreator(), end"));
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::ConstructL
// 2nd phase constructor.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::ConstructL()
    {
    __FLOG(_L8("CMTPImageDpThumbnailCreator::ConstructL(), begin"));
    iThumbMgr = CThumbnailManager::NewL( *this ); 
    iThumbMgr->SetThumbnailSizeL( EGridThumbnailSize );
#ifdef MTPTHUMBSCALING
    iScaler = CBitmapScaler::NewL();
#endif    
    iActiveSchedulerWait = new (ELeave) CActiveSchedulerWait();
    __FLOG(_L8("CMTPImageDpThumbnailCreator::ConstructL(), end"));
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::DoCancel
// From CActive.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::DoCancel()
    {
    __FLOG_VA((_L8(">> CMTPImageDpThumbnailCreator::DoCancel() iState %d iStatus 0x%X"), iState, iStatus.Int()));
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
    __FLOG_VA((_L8("<< CMTPImageDpThumbnailCreator::DoCancel() iState %d"), iState));
    }

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::RunL
// From CActive.
// --------------------------------------------------------------------------
//    
void CMTPImageDpThumbnailCreator::RunL()
    {
    __FLOG_VA((_L8(">> CMTPImageDpThumbnailCreator::RunL() iState %d iStatus %d"), iState, iStatus.Int()));
    User::LeaveIfError(iStatus.Int());
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
                __FLOG_VA((_L8("CMTPImageDpThumbnailCreator::RunL(),EDoNotCreate; iState %d"), iState));
                delete iData;
                iData = HBufC8::NewL(1);
                }
            
            __FLOG_VA((_L8("<< CMTPImageDpThumbnailCreator::RunL(),iBuffer->Write(*iData); iState %d"), iState));
            if(iActiveSchedulerWait->IsStarted())
                {
                iActiveSchedulerWait->AsyncStop();
                }
            break;
            }
        default:
            {
            User::Leave(KErrGeneral);
            break;
            }
        }
    __FLOG_VA((_L8("<< CMTPImageDpThumbnailCreator::RunL() iState %d"), iState));
    }

// --------------------------------------------------------------------------
// RunError
// --------------------------------------------------------------------------
//    
TInt CMTPImageDpThumbnailCreator::RunError(TInt aErr)
    {
    __FLOG_VA((_L8(">> CMTPImageDpThumbnailCreator::RunError() err 0x%X"), aErr));
    iState=EIdle;
    if(iActiveSchedulerWait->IsStarted())
        {
        *iCreationErr = aErr;
        iActiveSchedulerWait->AsyncStop();
        }
    // no need to cancel iScalerP since only leave is issued if scaler creation fails
    __FLOG(_L8("<< CMTPImageDpThumbnailCreator::RunError()"));
    return KErrNone;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::GetThumbnailL(const TDesC& aFileName, HBufC8*& aDestinationData,  TInt& result)
    {
    __FLOG(_L8(">> CMtpImageDphumbnailCreator::GetThumbnailL()"));
    GetThumbL(aFileName);
    iCreationErr = &result;     //reset the err flag
    *iCreationErr = KErrNone;
    __FLOG(_L8("<< CMTPImageDpThumbnailCreator::CreateThumbnailL()"));
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
    __FLOG(_L8(">> CMtpImageDphumbnailCreator::GetThumbL()"));
    // Create an object source representing a path to a file on local
    // file system.
    delete iObjectSource;
    iObjectSource = NULL;
    
    TParsePtrC parse(aFileName);
    if (parse.Ext().Length() >= 1)
        {
        const TDesC& mimeType = iDataProvider.FindMimeType(parse.Ext().Mid(1));
        __FLOG_VA((_L16("CMtpImageDphumbnailCreator::GetThumbL() - FileName:%S, MimeType:%S"), &aFileName, &mimeType));
    
        iObjectSource = CThumbnailObjectSource::NewL(aFileName, mimeType);
        }
    else
        {
        iObjectSource = CThumbnailObjectSource::NewL(aFileName, KNullDesC);
        }
    iCurrentReq = iThumbMgr->GetThumbnailL( *iObjectSource );
    iState = EGetting;
    __FLOG(_L8("<< CMtpImageDphumbnailCreator::GetThumbL()"));
    }

#ifdef MTPTHUMBSCALING
// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::ScaleBitmapL
// Scales the bitmap to the thumbnail size.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::ScaleBitmap()
    {
    __FLOG(_L8("CMTPImageDpThumbnailCreator::ScaleBitmapL(), begin"));
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
    __FLOG(_L8("CMTPImageDpThumbnailCreator::ScaleBitmapL(), end"));
    }
#endif

// --------------------------------------------------------------------------
// CMTPImageDpThumbnailCreator::EncodeImageL
// Encodes bitmap as a jpeg image.
// --------------------------------------------------------------------------
//
void CMTPImageDpThumbnailCreator::EncodeImageL( )
    {
    __FLOG(_L8(">> CMTPImageDpThumbnailCreator::EncodeImageL()"));

    delete iData;
    iData = NULL;
    
    delete iImgEnc;
    iImgEnc = NULL;
    
    // Convert bitmap to jpg
    iImgEnc = CImageEncoder::DataNewL( iData, KPtpMimeJPEG, CImageEncoder::EPreferFastEncode );
    iImgEnc->Convert( &iStatus, *iBitmap );
    SetActive();
    __FLOG(_L8("<< CMTPImageDpThumbnailCreator::EncodeImageL()"));
    }

//
//
void CMTPImageDpThumbnailCreator::ThumbnailReady( TInt aError, MThumbnailData& aThumbnail, TThumbnailRequestId aId )
    {
    // This function must not leave.
    __FLOG(_L8(">> CMTPImageDpThumbnailCreator::ThumbnailReady()"));
    if(iCurrentReq != aId)
        {
        __FLOG(_L8("CMTPImageDpThumbnailCreator::ThumbnailReady(),iCurrentReq != aId"));
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
        __FLOG(_L8("CMTPImageDpThumbnailCreator::ThumbnailReady(),EDoNotCreate, KErrNotFound"));
        iState = EEncoding;
        //don't trigger TNM to create thumbnail if image files are too big
        //iThumbMgr->CreateThumbnails(*iObjectSource);
        aError = KErrNone;
        }
    iStatus=KRequestPending;
    TRequestStatus* status=&iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    __FLOG(_L8("<< CMTPImageDpThumbnailCreator::ThumbnailReady()"));
    }

void CMTPImageDpThumbnailCreator::ThumbnailPreviewReady( MThumbnailData& /*aThumbnail*/, TThumbnailRequestId /*aId*/ )
    {
    
    }

// End of file
