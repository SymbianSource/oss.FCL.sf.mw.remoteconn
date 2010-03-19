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


#ifndef CMTPIMAGEDPOBJECTPROPERTYMGR_H
#define CMTPIMAGEDPOBJECTPROPERTYMGR_H

#include <e32base.h>
#include <f32file.h>
#include <e32std.h>

#include <mdesession.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptypeuint32.h>

#include "mtpimagedpconst.h"

class MMTPObjectMgr;
class MMTPPuidMgr;
class TMTPTypeUint128;
class CMTPTypeString;
class MMTPDataProviderFramework;
class CFileStore;
class CMTPTypeString;
class CMTPObjectMetaData;
class CMTPImageDataProvider;
class CMTPTypeOpaqueData;
class CMTPTypeArray;

/** 
Manage picture object properties

@internalTechnology
@released
*/
class CMTPImageDpObjectPropertyMgr : public CBase, public MMdESessionObserver
   {    
   
public:
    static CMTPImageDpObjectPropertyMgr* NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    ~CMTPImageDpObjectPropertyMgr();
    void SetCurrentObjectL(CMTPObjectMetaData& aObjectInfo, TBool aRequireForModify, TBool aSaveToCache = EFalse);
    
    void SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint8 aValue);
    void SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint16 aValue);
    void SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint32 aValue);
    void SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint64 aValue);
    void SetPropertyL(TMTPObjectPropertyCode aProperty, const TDesC& aValue);
    
    void GetPropertyL(TMTPObjectPropertyCode aProperty, TUint8& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, TUint16& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, TUint32& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, TUint64& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, TMTPTypeUint128& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeString& aValue);
    void GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeArray& aValue);
    
    //clear the cache
    void ClearCacheL();
    void ConvertMTPTimeStr2TTimeL(const TDesC& aTimeString, TTime& aModifiedTime) const;
    
    void StoreThunmnail(TUint aHandle, HBufC8* aData);
    HBufC8* Thumbnail(TUint aHandle);
    
public:
    void SetMdeSessionError(TInt aError);
    CMdESession& MdeSession();
    
    // from MMdESessionObserver
    void HandleSessionOpened(CMdESession& aSession, TInt aError);
    void HandleSessionError(CMdESession& aSession, TInt aError);
   
private:
    CMTPImageDpObjectPropertyMgr(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    void ConstructL(MMTPDataProviderFramework& aFramework);
    
    TBool GetYear(const TDesC& aDateString, TInt& aYear) const;
    TBool GetMonth(const TDesC& aDateString, TMonth& aMonth) const;
    TBool GetDay(const TDesC& aDateString, TInt& aDay) const;
    TBool GetHour(const TDesC& aDateString, TInt& aHour) const;
    TBool GetMinute(const TDesC& aDateString, TInt& aMinute) const;
    TBool GetSecond(const TDesC& aDateString, TInt& aSecond) const;
    TBool GetTenthSecond(const TDesC& aDateString, TInt& aTenthSecond) const;     
    
    void SetProperty2CacheL(TMTPObjectPropertyCode aProperty, TAny* aValue);
    void GetPropertyFromMdsL(TMTPObjectPropertyCode aProperty, TAny* aValue);

    void RemoveProperty(CMdEObject& aObject, CMdEPropertyDef& aPropDef);
    
    void OpenMdeObjectL();
    void ClearThumnailCache();
    
private:
    
    //define property cache object
    class CMTPImagePropertiesCache : public CBase
    {
    public:
        /**
        MTP image cache properites identifiers.
        */      
        enum TElementId
            {        
            EImagePixWidth        = 0,
            EImagePixHeight       = 1,
            EImageBitDepth        = 2,
            EDateCreated          = 3,
            
            /**
            The number of elements.        
            */
            ENumProperties,
            };
        
    public:
        static CMTPImagePropertiesCache* NewL();
        virtual ~CMTPImagePropertiesCache();
        
        void ResetL();
        
        const TDesC& DesC(TUint aId) const;
        TUint Uint(TUint aId) const;
        TUint ObjectHandle() const;
        
        void SetDesCL(TUint aId, const TDesC& aValue);
        void SetUint(TUint aId, TUint aValue);  
        void SetObjectHandle(TUint aObjectHandle);
        
    private:
        
        enum TType
            {
            EUint       = 0,
            EDesC       = 1,
            };
        
        struct TElementMetaData
            {
            TUint   iOffset;
            TType   iType;
            };                
        
    private:        
        CMTPImagePropertiesCache(const TElementMetaData* aElements, TUint aCount);
        void ConstructL();
        
    private:
        /**
        The element meta-data.
        */        
        const RArray<TElementMetaData>  iElements;
        
        /**
        The DesC element data.
        */
        RPointerArray<HBufC>            iElementsDesC;
        
        /**
        The Uint element data.
        */
        RArray<TUint>                   iElementsUint;
        
        /**
        The object handle of owner
        */
        TUint                           iObjectHandle;
        
        static const TElementMetaData   KElements[];        
    };
    
    /**
     * Thumbnail cache used for winlogo test
     */
    struct TThumbnailCache
        {
        TUint    iObjectHandle;
        HBufC8*  iThumbnailData;
        };   
    
private:
    /**
    FLOGGER debug trace member variable.
    */    
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    MMTPDataProviderFramework&  iFramework;
    CMTPImageDataProvider&      iDataProvider;
    CActiveSchedulerWait*       iActiveSchedulerWait;
    CMdESession*                iMetaDataSession;
    TInt                        iMdeSessionError;
    CMdEObject*                 iObject;//used for read properties from MdS

    RFs&                        iFs;
    MMTPObjectMgr&              iObjectMgr;
    CMTPObjectMetaData*         iObjectInfo;  //not owned
    TBool                       iCacheHit;//flag to indicate cache is available
    TBool                       iNeedParse;//flag to indicate whether we need to parse image file by our self
	
    /*
     * Cache thumbnail, thumbnail size is inconsistent in winlogo test
     */
    TThumbnailCache              iThumbnailCache;
    
    /**
     * Cache the latest image properties which PC send to device,
     * it can optimize synce/reverse-sync performance due to reduction of object properties generation
     */
    CMTPImagePropertiesCache*     iPropertiesCache; 
    };
   
#endif // CMTPIMAGEDPOBJECTPROPERTYMGR_H
