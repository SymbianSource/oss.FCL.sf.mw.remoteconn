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


#ifndef CMTPIMAGEDP_H
#define CMTPIMAGEDP_H

#include <e32property.h> 
#include <w32std.h> // PTP server/stack
#include <e32hashtab.h> 
#include <e32def.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <comms-infras/commsdebugutility.h>

#include <mtp/cmtpdataproviderplugin.h>
#include "mmtpenumerationcallback.h"
#include "cmtpimagedpobjectpropertymgr.h"

class MMTPRequestProcessor;
class CMTPImageDpThumbnailCreator;
class CMTPImageDpObjectPropertyMgr;
class CMTPImageDpMdeObserver;
class CRepository;
class CMTPImageDpRenameObject;
class CMTPImageDpNewPicturesNotifier;

const TInt KMaxExtNameLength = 4;
const TInt KMaxMimeNameLength = 10;

/** 
Implements the picture data provider plugin.
@internalComponent
*/
class CMTPImageDataProvider : public CMTPDataProviderPlugin,
                              public MMTPEnumerationCallback
    {
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPImageDataProvider();
    
public:
    CMTPImageDpObjectPropertyMgr& PropertyMgr()const;
    CMTPImageDpThumbnailCreator&  ThumbnailManager() const;
    CRepository& Repository() const;
    
    TMTPFormatCode FindFormat(const TDesC& aExtension);
    const TDesC& FindMimeType(const TDesC& aExtension);
    TBool GetCacheParentHandle(const TDesC& aParentPath, TUint32& aParentHandle);
    void  SetCacheParentHandle(const TDesC& aParentPath, TUint32 aParentHandle);
    void AppendDeleteObjectsArrayL(const TDesC& aSuid);
    void HandleDeleteObjectsArray();
    void IncreaseNewPictures(TInt aCount);
    void DecreaseNewPictures(TInt aCount);
    void HandleMdeSessionCompleteL(TInt aError);
    
public: // From CMTPDataProviderPlugin
    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;
    void SupportedL(TMTPSupportCategory aCategory, CDesCArray& aStrings) const;
    
public: // From MMTPEnumerationCallback    
    void NotifyStorageEnumerationCompleteL();
    void NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError);
           
private:
    CMTPImageDataProvider(TAny* aParams);
    void ConstructL();

    TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    TInt LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);    
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);
    void RenameObjectL(const TMTPNotificationParamsHandle& aParam);
    
    TUint QueryImageObjectCountL();
    
private:
    struct SMTPImageDpParentCache
        {        
        TPath   iPath;
        TUint32 iHandle;
        };
    
private:
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMTPImageDpObjectPropertyMgr*       iPropertyMgr;    
    CMTPImageDpThumbnailCreator*        iThumbnailManager;
    CMTPImageDpMdeObserver*             iMdeObserver;
    CRepository*                        iRepository;
    CMTPImageDpRenameObject*            iRenameObject;
    CMTPImageDpNewPicturesNotifier*     iNewPicNotifier;
    
    /**
    The active request processors table.
    */ 
    RPointerArray<MMTPRequestProcessor> iActiveProcessors;
    
    /**
     * contain the mapping image'extension to formatcode
     */
    RHashMap<TBuf<KMaxExtNameLength>, TMTPFormatCode> iFormatMappings;
    
    /**
     * contain the mapping image's extension to mime type
     */
    RHashMap<TBuf<KMaxExtNameLength>, TBuf<KMaxMimeNameLength> > iMimeMappings;    
    
    SMTPImageDpParentCache  iParentCache;
    
    TInt                    iActiveProcessor;
    TBool                   iActiveProcessorRemoved;    
    TBool                   iEnumerated;
    TBool                   iEnumerationNotified;
	
	RPointerArray<HBufC>    iDeleteObjectsArray;
    };
    
#endif // CMTPIMAGEDP_H
