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

#include <bautils.h>
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include <e32const.h>
#include <e32cmn.h>
#include <imageconversion.h> 
#include <mdeconstants.h>
#include <thumbnailmanager.h>

#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypeuint32.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypeopaquedata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mtpprotocolconstants.h>

#include "cmtpimagedp.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "cmtpimagedpthumbnailcreator.h"
#include "mtpimagedppanic.h"
#include "mtpimagedputilits.h"
#include "mtpimagedpconst.h"
#include "mtpdebug.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"MTPImageDpPropertyMgr");)

/**
The properties cache table content.
*/
const CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::TElementMetaData CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::KElements[CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::ENumProperties] = 
    {
        {0, CMTPImagePropertiesCache::EUint}, // EImagePixWidth
        {1, CMTPImagePropertiesCache::EUint}, // EImagePixHeight
        {2, CMTPImagePropertiesCache::EUint}, // EImageBitDepth
        {0, CMTPImagePropertiesCache::EDesC}, // EDateCreated
    };

CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache* CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::NewL()
    {
    CMTPImagePropertiesCache* self = new(ELeave) CMTPImagePropertiesCache(KElements, ENumProperties);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::~CMTPImagePropertiesCache()
    {
    iElementsDesC.ResetAndDestroy();
    iElementsUint.Reset();
    } 

/**
Constructor.
*/
CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::CMTPImagePropertiesCache(const TElementMetaData* aElements, TUint aCount) :
    iElements(sizeof(TElementMetaData), const_cast<TElementMetaData*>(aElements), aCount)
    {
    
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::ConstructL()
    {
    const TUint KCount(iElements.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        const TElementMetaData& KElement(iElements[i]);
        switch (KElement.iType)
            {
        case EDesC:
            __ASSERT_DEBUG((iElementsDesC.Count() == KElement.iOffset), Panic(EMTPImageDpBadLayout));
            iElementsDesC.AppendL(KNullDesC().AllocLC());
            CleanupStack::Pop();
            break;

        case EUint:
            __ASSERT_DEBUG((iElementsUint.Count() == KElement.iOffset), Panic(EMTPImageDpBadLayout));
            iElementsUint.AppendL(0);
            break;

        default:
            //nothing to do
            __DEBUG_ONLY(User::Invariant());
            break;
            }         
        }
    
    iObjectHandle = KMTPHandleNone;   
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::ResetL()
    {
    iObjectHandle = KMTPHandleNone;
    SetUint(EImagePixWidth, 0);
    SetUint(EImagePixHeight, 0);
    SetUint(EImageBitDepth, 0);
    SetDesCL(EDateCreated, KNullDesC);
    }

const TDesC& CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::DesC(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EDesC), Panic(EMTPImageDpTypeMismatch));
    return *iElementsDesC[iElements[aId].iOffset];
    }  

TUint CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::Uint(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUint), Panic(EMTPImageDpTypeMismatch));
    return iElementsUint[iElements[aId].iOffset];
    }

TUint CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::ObjectHandle() const
    {
    return iObjectHandle;
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::SetDesCL(TUint aId, const TDesC& aValue)
    {
    const TElementMetaData& KElement(iElements[aId]);
    __ASSERT_DEBUG((KElement.iType == EDesC), Panic(EMTPImageDpTypeMismatch));
    delete iElementsDesC[KElement.iOffset];
    iElementsDesC[KElement.iOffset] = NULL;
    iElementsDesC[KElement.iOffset] = aValue.AllocL();  
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::SetUint(TUint aId, TUint aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUint), Panic(EMTPImageDpTypeMismatch));
    iElementsUint[iElements[aId].iOffset] = aValue;
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::SetObjectHandle(TUint aObjectHandle)
    {
    iObjectHandle = aObjectHandle;
    }

CMTPImageDpObjectPropertyMgr* CMTPImageDpObjectPropertyMgr::NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpObjectPropertyMgr* self = new (ELeave) CMTPImageDpObjectPropertyMgr(aFramework, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    return self;
    }

CMTPImageDpObjectPropertyMgr::CMTPImageDpObjectPropertyMgr(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider) :
    iFramework(aFramework),
    iDataProvider(aDataProvider),
    iFs(aFramework.Fs()),
    iObjectMgr(aFramework.ObjectMgr())
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

void CMTPImageDpObjectPropertyMgr::ConstructL(MMTPDataProviderFramework& /*aFramework*/)
    {
    __FLOG(_L8("CMTPImageDpObjectPropertyMgr::ConstructL - Entry"));
    iPropertiesCache = CMTPImagePropertiesCache::NewL();    
    iMetaDataSession = CMdESession::NewL(*this);
    __FLOG(_L8("CMTPImageDpObjectPropertyMgr::ConstructL - Exit"));
    }
    
CMTPImageDpObjectPropertyMgr::~CMTPImageDpObjectPropertyMgr()
    {
    __FLOG(_L8("CMTPImageDpObjectPropertyMgr::~CMTPImageDpObjectPropertyMgr - Entry"));
    delete iPropertiesCache;
    delete iObject;
    delete iMetaDataSession;
    delete iThumbnailCache.iThumbnailData;
    __FLOG(_L8("CMTPImageDpObjectPropertyMgr::~CMTPImageDpObjectPropertyMgr - Exit"));
    __FLOG_CLOSE;
    }

void CMTPImageDpObjectPropertyMgr::SetCurrentObjectL(CMTPObjectMetaData& aObjectInfo, TBool aRequireForModify, TBool aSaveToCache)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::SetCurrentObjectL"));
    iObjectInfo = &aObjectInfo;

    /**
     * Image DP property manager will not directly modify properties which stored in the MdS
     */
    if(!aRequireForModify)
        {
        /**
         * determine whether the cache hit is occured
         */
        if (iPropertiesCache->ObjectHandle() == iObjectInfo->Uint(CMTPObjectMetaData::EHandle))
            {
            iCacheHit = ETrue;
            }
        else
            {            
            iCacheHit = EFalse;

            /**
             * if cache miss, we should clear the cache content
             */
            ClearCacheL();
            
			//need parse image file by our self if fail to get properties from MdS
            iNeedParse = ETrue;

			//clear previous Mde object
            delete iObject;
            iObject = NULL;            
            }        
        }    
    else
        {        
        /**
         * Set image object properties, because the cached properties are all readonly,
         * so only sendobjectproplist/sendobjectinfo operations can use cache mechanism, 
         * other operations will not use cache, such as setobjectvalue/setobjectproplist
         */
        if (aSaveToCache)
            {
            TUint objectHandle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);
            if (iPropertiesCache->ObjectHandle() != objectHandle)
                {
                iPropertiesCache->ResetL();
                }
            iPropertiesCache->SetObjectHandle(objectHandle);            
            }
        }
    
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::SetCurrentObjectL"));
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint8 aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodeNonConsumable) 
        {
        iObjectInfo->SetUint(CMTPObjectMetaData::ENonConsumable, aValue);
        }
    else
        {
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint16 aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    switch(aProperty)
        {
    case EMTPObjectPropCodeObjectFormat:
        iObjectInfo->SetUint(CMTPObjectMetaData::EFormatCode, aValue);
        break;        
    case EMTPObjectPropCodeProtectionStatus://this property does not supported by image dp
        //nothing to do
        break;
    default:
        //nothing to do
        break;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint32 aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    switch(aProperty)
        {
    case EMTPObjectPropCodeStorageID:
        iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, aValue);
        break;
    case EMTPObjectPropCodeParentObject:
        iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, aValue);
        break;
    case EMTPObjectPropCodeWidth:
        iPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixWidth, aValue);
        break;
    case EMTPObjectPropCodeHeight:
        iPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixHeight, aValue);
        break; 
    case EMTPObjectPropCodeImageBitDepth:
        iPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImageBitDepth, aValue);
        break;          
    default:
        //nothing to do
        break;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TDesC& aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    switch(aProperty)
        {
    case EMTPObjectPropCodeObjectFileName:
        {
        TParsePtrC oldUri = TParsePtrC(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
        
        //calculate new file name length
        TInt len = oldUri.DriveAndPath().Length() + aValue.Length();
        
        //allocate memory for the new uri
        RBuf  newUri;
        newUri.CleanupClosePushL();
        newUri.CreateL(len);
        
        //create the new uri
        newUri.Append(oldUri.DriveAndPath());
        newUri.Append(aValue);
        newUri.Trim();
        
        //ask fs to rename file, leave if err returned from fs
        User::LeaveIfError(iFs.Rename(oldUri.FullName(), newUri));
        iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, newUri);
        CleanupStack::PopAndDestroy(&newUri);        
        }
        break;
        
    case EMTPObjectPropCodeName:
        iObjectInfo->SetDesCL(CMTPObjectMetaData::EName, aValue);
        break;
              
    case EMTPObjectPropCodeDateModified:
        {
        TTime modifiedTime;
        ConvertMTPTimeStr2TTimeL(aValue, modifiedTime);
        iFs.SetModified(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), modifiedTime);            
        }
        break;
      
    case EMTPObjectPropCodeDateCreated://MdS property
        iPropertiesCache->SetDesCL(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EDateCreated, aValue);
        break;
        
    default:
        //nothing to do
        break;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::SetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint8 &aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodeNonConsumable) 
        {
        aValue = iObjectInfo->Uint(CMTPObjectMetaData::ENonConsumable);       
        }
    else
        {
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }     
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint16 &aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    TEntry entry;
    switch(aProperty)
        {
    case EMTPObjectPropCodeObjectFormat:
        aValue = iObjectInfo->Uint(CMTPObjectMetaData::EFormatCode);
        break;
    case EMTPObjectPropCodeRepresentativeSampleFormat:
        aValue = KThumbFormatCode;
       break;        
    case EMTPObjectPropCodeProtectionStatus:
        {
        TInt err = iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry);        
        if ( err == KErrNone && entry.IsReadOnly())
            {
            aValue = EMTPProtectionReadOnly;
            }
        else
            {
            aValue = EMTPProtectionNoProtection;
            }        
        }    
        break;    
    default:
        aValue = 0;//initialization
        //ingore the failure if we can't get properties form MdS
        TRAP_IGNORE(GetPropertyFromMdsL(aProperty, &aValue));
        break;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint32 &aValue, TBool alwaysCreate/* = ETrue*/)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    switch(aProperty)
        {
    case EMTPObjectPropCodeStorageID:
        aValue = iObjectInfo->Uint(CMTPObjectMetaData::EStorageId);
        break;
        
    case EMTPObjectPropCodeParentObject:
        aValue = iObjectInfo->Uint(CMTPObjectMetaData::EParentHandle);
        break;        
       
    case EMTPObjectPropCodeRepresentativeSampleSize:
        {
        __FLOG_VA((_L16("Query smaple size from MdS - URI:%S"), &iObjectInfo->DesC(CMTPObjectMetaData::ESuid)));
        ClearThumnailCache();                                
        /**
         * try to query thumbnail from TNM, and then store thumbnail to cache
         */
        TEntry fileEntry;
        TInt err = iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), fileEntry);
        if (err == KErrNone)
            {
            if(fileEntry.FileSize() > KFileSizeMax || !alwaysCreate)
                {
                iDataProvider.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
                }
            else
                {
                iDataProvider.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
                }
            
            /**
             * trap the leave to avoid return general error when PC get object property list
             */
            TRAP(err, iDataProvider.ThumbnailManager().GetThumbnailL(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), iThumbnailCache.iThumbnailData, err));
            if (err == KErrNone)
                {
                iThumbnailCache.iObjectHandle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);                        
                if (iThumbnailCache.iThumbnailData != NULL)
                    {
                    aValue = static_cast<TUint32>(iThumbnailCache.iThumbnailData->Size());
                    }
                                            
                if (aValue <= 0)
                    {
                    //trigger initiator to re-query thumbnail again if the thumbnail size of response is zero
                    aValue = KThumbCompressedSize;
                    }
                }
            }
        }
        break;       
       
    case EMTPObjectPropCodeRepresentativeSampleHeight:
        aValue = KThumbHeigth;
        break;
       
    case EMTPObjectPropCodeRepresentativeSampleWidth:
        aValue = KThumbWidht;
        break;
              
    default:
        aValue = 0;//initialization
        //ingore the failure if we can't get properties form MdS
        TRAP_IGNORE(GetPropertyFromMdsL(aProperty, &aValue));
        break;  
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }
    
void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint64& aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));

    if (aProperty == EMTPObjectPropCodeObjectSize) 
        {
        TEntry entry;
        iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry);
        aValue = entry.FileSize();            
        }
    else
        {
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }    
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TMTPTypeUint128& aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodePersistentUniqueObjectIdentifier) 
        {
        TUint32 handle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);
        aValue = iObjectMgr.PuidL(handle);
        }
    else
        {
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }
    
void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeString& aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));

    switch(aProperty)
        {    
    case EMTPObjectPropCodeObjectFileName:
        {
        TFileName name;
        User::LeaveIfError(BaflUtils::MostSignificantPartOfFullName(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), name));     
        aValue.SetL(name);
        }
        break;
        
    case EMTPObjectPropCodeName:
        {
        aValue.SetL(iObjectInfo->DesC(CMTPObjectMetaData::EName));
        }
        break;
        
    case EMTPObjectPropCodeDateModified:
        {
        TBuf<64> dateString;
        TEntry entry;
        iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry);
        
        _LIT(KTimeFormat, "%F%Y%M%DT%H%T%S"); 
        entry.iModified.FormatL(dateString, KTimeFormat);        
        aValue.SetL(dateString);
        }
        break;                 
        
    default:
        //ingore the failure if we can't get properties form MdS
        TRAP_IGNORE(GetPropertyFromMdsL(aProperty, &aValue));
        break;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyL"));
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeArray& aValue, TBool alwaysCreate /*= ETrue*/)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyL -- SmapleData"));       
    
    if (aProperty == EMTPObjectPropCodeRepresentativeSampleData)
        {
        HBufC8* tnBuf = Thumbnail(iObjectInfo->Uint(CMTPObjectMetaData::EHandle));    
        if (tnBuf != NULL)
            {    
            aValue.SetByDesL(*tnBuf);
            }
        else
            {    
            ClearThumnailCache();                                
            /**
             * try to query thumbnail from TNM, and then store thumbnail to cache
             */
            TEntry fileEntry;
            TInt err = iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), fileEntry);
            if (err == KErrNone)
                {
                
                if(fileEntry.FileSize() > KFileSizeMax || !alwaysCreate)
                    {
                    iDataProvider.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
                    }
                else
                    {
                    iDataProvider.ThumbnailManager().GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
                    }
                
                /**
                 * trap the leave to avoid return general error when PC get object property list
                 */
                TRAP(err, iDataProvider.ThumbnailManager().GetThumbnailL(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), iThumbnailCache.iThumbnailData, err));
                if (err == KErrNone)
                    {
                    iThumbnailCache.iObjectHandle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);                        
                    if (iThumbnailCache.iThumbnailData != NULL)
                        {
                        aValue.SetByDesL(*iThumbnailCache.iThumbnailData);
                        }                                
                    }
                }
            }
        }
    else
        {
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyFromMdsL(TMTPObjectPropertyCode aProperty, TAny* aValue)
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetPropertyFromMdsL"));
    
    TInt err = KErrNone;
      
    if (iCacheHit)
        {
        /**
         * The object hit the cache, so we query properties from cache
         */
        switch (aProperty)
            {
        case EMTPObjectPropCodeDateCreated:
            (*(static_cast<CMTPTypeString*>(aValue))).SetL(iPropertiesCache->DesC(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EDateCreated));            
            break;
          
        case EMTPObjectPropCodeWidth:
            *static_cast<TUint32*>(aValue) = iPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixWidth);            
            break;
            
        case EMTPObjectPropCodeHeight:
            *static_cast<TUint32*>(aValue) = iPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixHeight);           
            break;
            
        case EMTPObjectPropCodeImageBitDepth:
            *static_cast<TUint32*>(aValue) = iPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImageBitDepth);            
            break;
            
        default:
            //nothing to do
            break;
            }
        }
    else
        {
        /**
         * The object miss cache, so we should open Mde object to query properties
         */
        OpenMdeObjectL();
        
        CMdENamespaceDef& defaultNamespace = iMetaDataSession->GetDefaultNamespaceDefL();
        CMdEObjectDef& imageObjDef = defaultNamespace.GetObjectDefL( MdeConstants::Image::KImageObject );
        CMdEProperty* mdeProperty = NULL;
        
        switch (aProperty)
            {        
        case EMTPObjectPropCodeDateCreated:
            {
            if (iObject)
                {        
                CMdEPropertyDef& creationDatePropDef = imageObjDef.GetPropertyDefL(MdeConstants::Object::KCreationDateProperty);
                TInt err = iObject->Property( creationDatePropDef, mdeProperty );  
                if (err >= KErrNone) 
                    {
                    TBuf<KMaxTimeFormatSpec*2> timeValue;
                    // locale independent YYYYMMSSThhmmss, as required by the MTP spec
                    _LIT(KTimeFormat, "%F%Y%M%DT%H%T%S");
                    mdeProperty->TimeValueL().FormatL(timeValue, KTimeFormat);
                    (*(static_cast<CMTPTypeString*>(aValue))).SetL(timeValue);
                    
                    __FLOG_VA((_L16("GetPropertyFromMdsL - from MdS: URI:%S, DateCreated:%S"), &iObjectInfo->DesC(CMTPObjectMetaData::ESuid), &timeValue));
                    }
                }
            }
           break;  
           
        case EMTPObjectPropCodeWidth:
            {
            if (iObject)
                {
                CMdEPropertyDef& imageWidthPropDef = imageObjDef.GetPropertyDefL(MdeConstants::MediaObject::KWidthProperty);
                err = iObject->Property( imageWidthPropDef, mdeProperty );  
                if (err >= KErrNone) 
                    {
                    *static_cast<TUint32*>(aValue) = mdeProperty->Uint16ValueL();
                    }
                else
                    {
                    *static_cast<TUint32*>(aValue) = 0;
                    }
                }
            else
                {
                *static_cast<TUint32*>(aValue) = 0;
                }
            }
           break; 
           
        case EMTPObjectPropCodeHeight:
            {
            if (iObject)
                {
                CMdEPropertyDef& imageHeightPropDef = imageObjDef.GetPropertyDefL(MdeConstants::MediaObject::KHeightProperty);
                err = iObject->Property( imageHeightPropDef, mdeProperty );  
                if (err >= KErrNone) 
                    {
                    *static_cast<TUint32*>(aValue) = mdeProperty->Uint16ValueL();
                    }
                else
                    {
                    *static_cast<TUint32*>(aValue) = 0;
                    }
                }
            else
                {
                *static_cast<TUint32*>(aValue) = 0;
                }
            }
           break; 
           
        case EMTPObjectPropCodeImageBitDepth:
            {
            if (iObject)
                {
                CMdEPropertyDef& imageBitDepth = imageObjDef.GetPropertyDefL(MdeConstants::Image::KBitsPerSampleProperty);
                err = iObject->Property( imageBitDepth, mdeProperty );  
                if (err >= KErrNone) 
                    {
                    *static_cast<TUint32*>(aValue) = mdeProperty->Uint16ValueL();               
                    }
                else
                    {
                    *static_cast<TUint32*>(aValue) = 0;
                    }  
                }
            else
                {
                *static_cast<TUint32*>(aValue) = 0;
                }        
            }     
           break; 
           
        default:
            //nothing to do
            break;
            }
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetPropertyFromMdsL"));
    }

TBool CMTPImageDpObjectPropertyMgr::GetYear(const TDesC& aDateString, TInt& aYear) const
    {
  __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetYear"));
    aYear = 0;
    TLex dateBuf(aDateString.Left(4));
  __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetYear"));
    return dateBuf.Val(aYear) == KErrNone;
    }

TBool CMTPImageDpObjectPropertyMgr::GetMonth(const TDesC& aDateString, TMonth& aMonth) const
    {
      __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetMonth"));
    TBool result = EFalse;
    aMonth = EJanuary;
    TInt month = 0;
    TLex dateBuf(aDateString.Mid(4, 2));
    if(dateBuf.Val(month) == KErrNone && month > 0 && month < 13)
        {
        month--;
        aMonth = (TMonth)month;
        result = ETrue;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetMonth"));
    return result;
    }

TBool CMTPImageDpObjectPropertyMgr::GetDay(const TDesC& aDateString, TInt& aDay) const
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetDay"));
    TBool result = EFalse;
    aDay = 0;
    TLex dateBuf(aDateString.Mid(6, 2));
    if(dateBuf.Val(aDay) == KErrNone && aDay > 0 && aDay < 32)
        {
        aDay--;
        result = ETrue;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetDay"));
    return result;	
    }

TBool CMTPImageDpObjectPropertyMgr::GetHour(const TDesC& aDateString, TInt& aHour) const
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetHour"));
    aHour = 0;
    TLex dateBuf(aDateString.Mid(9, 2));
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetHour"));
    return (dateBuf.Val(aHour) == KErrNone && aHour >=0 && aHour < 60);
    }
                
TBool CMTPImageDpObjectPropertyMgr::GetMinute(const TDesC& aDateString, TInt& aMinute) const
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetMinute"));
    aMinute = 0;
    TLex dateBuf(aDateString.Mid(11, 2));
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetMinute"));
    return (dateBuf.Val(aMinute) == KErrNone && aMinute >=0 && aMinute < 60);
    }

TBool CMTPImageDpObjectPropertyMgr::GetSecond(const TDesC& aDateString, TInt& aSecond) const
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetSecond"));
    aSecond = 0;
    TLex dateBuf(aDateString.Mid(13, 2));
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetSecond"));
    return (dateBuf.Val(aSecond) == KErrNone && aSecond >= 0 && aSecond < 60);
    }

TBool CMTPImageDpObjectPropertyMgr::GetTenthSecond(const TDesC& aDateString, TInt& aTenthSecond) const
    {
    __FLOG(_L8(">> CMTPImageDpObjectPropertyMgr::GetTenthSecond"));
    TBool result = EFalse;
    aTenthSecond = 0;
    TInt dotPos = aDateString.Find(_L("."));
    if(dotPos != KErrNotFound && dotPos < aDateString.Length() - 1)
        {
        TLex dateBuf(aDateString.Mid(dotPos + 1, 1));
        result = (dateBuf.Val(aTenthSecond) == KErrNone && aTenthSecond >=0 && aTenthSecond < 10);
        }
    else
        {
        result = ETrue;
        }
    __FLOG(_L8("<< CMTPImageDpObjectPropertyMgr::GetTenthSecond"));
    return result;	
    }

/*
 * Convert the MTP datatime string to TTime:
 * 
 *  MTP datatime string format: YYYYMMDDThhmmss.s  Optional(.s)
 *  TTime string format       : YYYYMMDD:HHMMSS.MMMMMM
 *  
 */
void CMTPImageDpObjectPropertyMgr::ConvertMTPTimeStr2TTimeL(const TDesC& aTimeString, TTime& aModifiedTime) const
    {
    //Convert the Time String to TDateTime
    TInt year = 0;
    TMonth month;
    TInt day = 0;
    TInt hour = 0;
    TInt minute = 0;
    TInt second = 0;
    TInt tenthSecond = 0;
    
    if(!GetYear(aTimeString,year)
           ||!GetMonth(aTimeString,month)
           ||!GetDay(aTimeString,day)
           ||!GetHour(aTimeString,hour)
           ||!GetMinute(aTimeString,minute)
           ||!GetSecond(aTimeString,second)
           ||!GetTenthSecond(aTimeString,tenthSecond))
        {        
        User::Leave(KErrArgument);
        }
    else
        {     
        TDateTime dateTime(year, month, day, hour, minute, second, tenthSecond);
        aModifiedTime = dateTime;
        } 
    }

void CMTPImageDpObjectPropertyMgr::RemoveProperty(CMdEObject& aObject, CMdEPropertyDef& aPropDef)
    {
    __FLOG(_L8("CMTPImageDpObjectPropertyMgr::RemoveProperty"));
    TInt index;
    CMdEProperty* property;
    index = aObject.Property(aPropDef, property);
    if (index != KErrNotFound)
        {
        aObject.RemoveProperty(index);
        }
    }

/**
 * Store thumbnail into cache
 */
void CMTPImageDpObjectPropertyMgr::StoreThunmnail(TUint aHandle, HBufC8* aData)
    {
    ClearThumnailCache();
    
    iThumbnailCache.iObjectHandle = aHandle;      
    iThumbnailCache.iThumbnailData = aData;
    }

/**
 * Get thumbnail from cache
 */
HBufC8* CMTPImageDpObjectPropertyMgr::Thumbnail(TUint aHandle)
    {
    if (iThumbnailCache.iObjectHandle == aHandle)
        {
        return iThumbnailCache.iThumbnailData;
        }
    else
        {
        return NULL;
        }
    }

/**
 * Return the mdesession instance
 */
CMdESession& CMTPImageDpObjectPropertyMgr::MdeSession()
    {
    return *iMetaDataSession;
    }

/**
 *  From MMdESessionObserver
 */
void CMTPImageDpObjectPropertyMgr::HandleSessionOpened(CMdESession& /*aSession*/, TInt aError)
    {   
    SetMdeSessionError(aError);
    TRAP_IGNORE(iDataProvider.HandleMdeSessionCompleteL(aError));
    }

/**
 *  From MMdESessionObserver
 */
void CMTPImageDpObjectPropertyMgr::HandleSessionError(CMdESession& /*aSession*/, TInt aError)
    {
    SetMdeSessionError(aError);
    TRAP_IGNORE(iDataProvider.HandleMdeSessionCompleteL(aError));
    }

void CMTPImageDpObjectPropertyMgr::SetMdeSessionError(TInt aError)
    {
    iMdeSessionError = aError;
    }

void CMTPImageDpObjectPropertyMgr::ClearCacheL()
    {
    iPropertiesCache->ResetL();
    }

void CMTPImageDpObjectPropertyMgr::OpenMdeObjectL()
    {
    if (iObject == NULL)
        {
        __FLOG_VA((_L16("OpenMdeObjectL - URI = %S"), &iObjectInfo->DesC(CMTPObjectMetaData::ESuid)));
		
        CMdENamespaceDef& defaultNamespace = iMetaDataSession->GetDefaultNamespaceDefL();
        CMdEObjectDef& imageObjDef = defaultNamespace.GetObjectDefL( MdeConstants::Image::KImageObject );
        
        //if we can not open MdS object for getting properties, we will not get properites which stored in MdS
        TFileName uri;
        uri.CopyLC(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
        TRAP_IGNORE((iObject = iMetaDataSession->GetObjectL(uri, imageObjDef)));      
        }
    }

void CMTPImageDpObjectPropertyMgr::ClearThumnailCache()
    {
    delete iThumbnailCache.iThumbnailData;
    iThumbnailCache.iThumbnailData = NULL;
    
    iThumbnailCache.iObjectHandle = KMTPHandleNone;
    }
