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

#ifndef CMTPIMAGEDPTHUMBNAILCREATOR_H
#define CMTPIMAGEDPTHUMBNAILCREATOR_H

// INCLUDES
#include <e32base.h>

#include <thumbnailmanager.h>
#include <thumbnailmanagerobserver.h>
#include <thumbnailobjectsource.h>
#include <thumbnaildata.h>
#include <imageconversion.h>
#include <bitmaptransforms.h>

class CMTPTypeOpaqueData;

#include "mtpdebug.h"

#define MTPTHUMBSCALING

// CLASS DECLARATION
/**
*  A class for reading image related info and creating a thumbnail from 
*  an image.
*
*  @lib ptpstack.lib
*  @since S60 3.2
*/
class CMTPImageDpThumbnailCreator : public CActive, public MThumbnailManagerObserver
    {
public:

    /**
     * Two-phased constructor.
     * @param None.
     * @return An instance of CMTPImageDpThumbnailCreator.
     */
    static CMTPImageDpThumbnailCreator* NewL();

    /**
     * C++ destructor.
     */
    ~CMTPImageDpThumbnailCreator();
    
public:

    /**
     * Gets a thumbnail from the image.
     * @since S60 3.2
     * @param aSession, Reference to file server.
     * @param aFileName, Name of the image file. Caller must ensure that the referenced object exists until the asynchronous call is completed.
     * @param aThumbName, Name of the thumbnail file to be created. Caller must ensure that the referenced object exists until the asynchronous call is completed.
     * @param aGetThumbnailStatus, status when 
     */
    void GetThumbnailL(const TDesC& aFileName, CMTPTypeOpaqueData& aThumbName, TInt& result);


    void ClearThumbnailData();
    CThumbnailManager*  GetThumbMgr() { return iThumbMgr;}

private: // From MThumbnailManagerObserver
    void ThumbnailReady( TInt aError, MThumbnailData& aThumbnail, TThumbnailRequestId aId );
    void ThumbnailPreviewReady( MThumbnailData& aThumbnail, TThumbnailRequestId aId );

private: // From CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);    
 
private:
    void EncodeImageL();    
    /**
     * Scales the image as a thumbnail size.
     * @since S60 3.2
     * @param None.
     * @return None.
     */
#ifdef MTPTHUMBSCALING
    void ScaleBitmap();
#endif
    void QueueThumbCreationL( const TDesC& aFileName );
    void CreateThumbL(const TDesC& aFileName );
    void GetThumbL(const TDesC& aFileName);

private:
    /**
     * Default C++ constructor. Not used.
     */
    CMTPImageDpThumbnailCreator();

    /**
     * 2nd phase constructor.
     */
    void ConstructL();

private:
    /**
    FLOGGER debug trace member variable.
    **/
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    enum{
        EIdle,
        EGetting,
        EGetted,
        EScaling,
        EEncoding}          iState;
    TThumbnailRequestId     iCurrentReq;
    TInt*                   iCreationErr;
    CFbsBitmap*             iBitmap;
    CImageEncoder*          iImgEnc;
#ifdef MTPTHUMBSCALING
    CBitmapScaler*          iScaler;
#endif
    HBufC8*                 iData;
    CThumbnailManager*      iThumbMgr;
    CThumbnailObjectSource* iObjectSource;
    CMTPTypeOpaqueData*     iBuffer;             //not owned
    CActiveSchedulerWait*   iActiveSchedulerWait;
    };

#endif // CMTPIMAGEDPTHUMBNAILCREATOR_H
