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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpobjectpropertymgrTraces.h"
#endif


// Indicate how many cache can be stored
const TUint KCacheThreshold = 16;

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
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
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
    }

void CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::ResetL()
    {
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
    }

void CMTPImageDpObjectPropertyMgr::ConstructL(MMTPDataProviderFramework& /*aFramework*/)
    {
    iMetaDataSession = CMdESession::NewL(*this);
    }
    
CMTPImageDpObjectPropertyMgr::~CMTPImageDpObjectPropertyMgr()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_CMTPIMAGEDPOBJECTPROPERTYMGR_DES_ENTRY );
    delete iObject;
    delete iMetaDataSession;
    delete iThumbnailCache.iThumbnailData;
    
    //Clear propreties cache map
    ClearAllCache();
    iPropretiesCacheMap.Close();
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_CMTPIMAGEDPOBJECTPROPERTYMGR_DES_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::SetCurrentObjectL(CMTPObjectMetaData& aObjectInfo, TBool aRequireForModify, TBool aSaveToCache)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETCURRENTOBJECTL_ENTRY );
    iObjectInfo = &aObjectInfo;

    /**
     * Image DP property manager will not directly modify properties which stored in the MdS
     */
    if(!aRequireForModify)
        {
        /**
         * determine whether the cache hit is occured
         */
        iCacheHit = FindPropertiesCache(iObjectInfo->Uint(CMTPObjectMetaData::EHandle));
        if (!iCacheHit)
            {
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
            if (FindPropertiesCache(objectHandle))
                {
                OstTrace1( TRACE_NORMAL, CMTPIMAGEDPOBJECTPROPERTYMGR_SETCURRENTOBJECTL, 
                        "SetCurrentObjectL - find object in cache:%u", objectHandle );
                iCurrentPropertiesCache->ResetL();
                }
            else
                {
                if (iPropretiesCacheMap.Count() > KCacheThreshold)
                    {
                    // Find the first object handle from cache map and then destory it
                    RHashMap<TUint, CMTPImagePropertiesCache*>::TIter iter(iPropretiesCacheMap);
                    DestroyPropertiesCahce(*iter.NextKey());
                    OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDPOBJECTPROPERTYMGR_SETCURRENTOBJECTL, 
                            "SetCurrentObjectL - destory object:%u", objectHandle ); 
                    }                
                
                iCurrentPropertiesCache = CMTPImagePropertiesCache::NewL();
                iPropretiesCacheMap.Insert(objectHandle, iCurrentPropertiesCache);
                OstTrace1( TRACE_NORMAL, DUP2_CMTPIMAGEDPOBJECTPROPERTYMGR_SETCURRENTOBJECTL, 
                        "SetCurrentObjectL - create new object:%u", objectHandle );
                }
            }
        }

    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETCURRENTOBJECTL_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint8 aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT8_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodeNonConsumable) 
        {
        iObjectInfo->SetUint(CMTPObjectMetaData::ENonConsumable, aValue);
        }
    else
        {
        OstTrace1( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL, 
                "property code %d not equal to EMTPObjectPropCodeNonConsumable", aProperty );
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT8_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint16 aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT16_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    switch(aProperty)
        {
    case EMTPObjectPropCodeObjectFormat:
        iObjectInfo->SetUint(CMTPObjectMetaData::EFormatCode, aValue);
        break;        
    case EMTPObjectPropCodeProtectionStatus://this property does not supported by image dp
        //nothing to do
        break;
    case EMTPObjectPropCodeHidden:
        {
		__ASSERT_ALWAYS(( EMTPHidden == aValue )||( EMTPVisible == aValue ), User::Leave(KErrArgument));
        TEntry entry;
		User::LeaveIfError(iFramework.Fs().Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry));
        if (( EMTPHidden == aValue ) && ( !entry.IsHidden()))
            {
            entry.iAtt &= ~KEntryAttHidden;
            entry.iAtt |= KEntryAttHidden;
            User::LeaveIfError(iFramework.Fs().SetAtt(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry.iAtt, ~entry.iAtt));
            }
        else if (( EMTPVisible == aValue )&&( entry.IsHidden()))
            {
            entry.iAtt &= ~KEntryAttHidden;
            User::LeaveIfError(iFramework.Fs().SetAtt(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry.iAtt, ~entry.iAtt));
            }
        } 
        break;
    default:
        //nothing to do
        break;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT16_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TUint32 aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT32_ENTRY );
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
        if (iCurrentPropertiesCache != NULL)
            {
            iCurrentPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixWidth, aValue);
            }
        break;
    case EMTPObjectPropCodeHeight:
        if (iCurrentPropertiesCache != NULL)
            {        
            iCurrentPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixHeight, aValue);
            }
        break; 
    case EMTPObjectPropCodeImageBitDepth:
        if (iCurrentPropertiesCache != NULL)
            {        
            iCurrentPropertiesCache->SetUint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImageBitDepth, aValue);
            }
        break;          
    default:
        //nothing to do
        break;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TUINT32_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::SetPropertyL(TMTPObjectPropertyCode aProperty, const TDesC& aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TDESC_ENTRY );
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
        LEAVEIFERROR(iFs.Rename(oldUri.FullName(), newUri),
                OstTraceExt3( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_TDESC_SETPROPERTYL, 
                        "Rename %S to %S failed! error code %d", oldUri.FullName(), newUri, munged_err ));
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
        if (iCurrentPropertiesCache != NULL)
            {
            iCurrentPropertiesCache->SetDesCL(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EDateCreated, aValue);
            }
        break;
        
    default:
        //nothing to do
        break;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_SETPROPERTYL_TDESC_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint8 &aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT8_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodeNonConsumable) 
        {
        aValue = iObjectInfo->Uint(CMTPObjectMetaData::ENonConsumable);       
        }
    else
        {
        OstTrace1( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_TUINT8_GETPROPERTYL, 
                "property code %d not equal to EMTPObjectPropCodeNonConsumable", aProperty );
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }     
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT8_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint16 &aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT16_ENTRY );
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
    case EMTPObjectPropCodeHidden:
        {
        TInt err = iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry);        
        if ( err == KErrNone && entry.IsHidden())
            {
            aValue = EMTPHidden;
            }
        else
            {
            aValue = EMTPVisible;
            }        
        } 
        break;
    default:
        aValue = 0;//initialization
        //ingore the failure if we can't get properties form MdS
        TRAP_IGNORE(GetPropertyFromMdsL(aProperty, &aValue));
        break;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT16_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint32 &aValue, TBool alwaysCreate/* = ETrue*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT32_ENTRY );
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
        OstTraceExt1( TRACE_NORMAL, CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT32, 
                "Query smaple size from MdS - URI:%S", iObjectInfo->DesC(CMTPObjectMetaData::ESuid) );
        ClearThumnailCache();                                
        /**
         * try to query thumbnail from TNM, and then store thumbnail to cache
         */
        TEntry fileEntry;
        TInt err = iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), fileEntry);
        CMTPImageDpThumbnailCreator* tnc = iDataProvider.ThumbnailManager();
        if (err == KErrNone && tnc != NULL)
            {
            
            if(fileEntry.FileSize() > KFileSizeMax || !alwaysCreate)
                {
                tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
                }
            else
                {
                tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
                }
            
            /**
             * trap the leave to avoid return general error when PC get object property list
             */
            TRAP(err, tnc->GetThumbnailL(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), iThumbnailCache.iThumbnailData, err));
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
        else
            {
            aValue = KThumbCompressedSize;
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
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT32_EXIT );
    }
    
void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TUint64& aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT64_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));

    if (aProperty == EMTPObjectPropCodeObjectSize) 
        {
        TEntry entry;
        iFs.Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), entry);
        aValue = entry.FileSize();            
        }
    else
        {
        OstTrace1( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT64, 
                "property code %d not equal to EMTPObjectPropCodeObjectSize", aProperty );
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }    
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TUINT64_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, TMTPTypeUint128& aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TMTPTYPEUINT128_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));
    
    if (aProperty == EMTPObjectPropCodePersistentUniqueObjectIdentifier) 
        {
        TUint32 handle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);
        aValue = iObjectMgr.PuidL(handle);
        }
    else
        {
        OstTrace1( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TMTPTYPEUINT128, 
                "property code %d not equal to EMTPObjectPropCodeObjectSize", aProperty );
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_TMTPTYPEUINT128_EXIT );
    }
    
void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeString& aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_CMTPTYPESTRING_ENTRY );
    __ASSERT_DEBUG(iObjectInfo, Panic(EMTPImageDpObjectNull));

    switch(aProperty)
        {    
    case EMTPObjectPropCodeObjectFileName:
        {
        TFileName name;
        LEAVEIFERROR(BaflUtils::MostSignificantPartOfFullName(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), name),
                OstTraceExt2( TRACE_ERROR, DUP1_CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL, 
                        "Can't get most significant part of %S! error code %d", iObjectInfo->DesC(CMTPObjectMetaData::ESuid), munged_err));
                
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
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_CMTPTYPESTRING_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyL(TMTPObjectPropertyCode aProperty, CMTPTypeArray& aValue, TBool alwaysCreate /*= ETrue*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_CMTPTYPEARRAY_ENTRY );    
    
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
            CMTPImageDpThumbnailCreator* tnc = iDataProvider.ThumbnailManager();
            if (err == KErrNone && tnc != NULL)
                {
                
                if(fileEntry.FileSize() > KFileSizeMax || !alwaysCreate)
                    {
                    tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDoNotCreate);
                    }
                else
                    {
                    tnc->GetThumbMgr()->SetFlagsL(CThumbnailManager::EDefaultFlags);
                    }
                
                /**
                 * trap the leave to avoid return general error when PC get object property list
                 */
                TRAP(err, tnc->GetThumbnailL(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), iThumbnailCache.iThumbnailData, err));
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
        OstTrace1( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_CMTPTYPEARRAY, 
                "property code %d not equal to EMTPObjectPropCodeRepresentativeSampleData", aProperty );
        User::Leave(EMTPRespCodeObjectPropNotSupported);
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYL_CMTPTYPEARRAY_EXIT );
    }

void CMTPImageDpObjectPropertyMgr::GetPropertyFromMdsL(TMTPObjectPropertyCode aProperty, TAny* aValue)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYFROMMDSL_ENTRY );
    
    TInt err = KErrNone;
      
    if (iCacheHit)
        {
        /**
         * The object hit the cache, so we query properties from cache
         */
        switch (aProperty)
            {
        case EMTPObjectPropCodeDateCreated:
            (*(static_cast<CMTPTypeString*>(aValue))).SetL(iCurrentPropertiesCache->DesC(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EDateCreated));            
            break;
          
        case EMTPObjectPropCodeWidth:
            *static_cast<TUint32*>(aValue) = iCurrentPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixWidth);            
            break;
            
        case EMTPObjectPropCodeHeight:
            *static_cast<TUint32*>(aValue) = iCurrentPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImagePixHeight);           
            break;
            
        case EMTPObjectPropCodeImageBitDepth:
            *static_cast<TUint32*>(aValue) = iCurrentPropertiesCache->Uint(CMTPImageDpObjectPropertyMgr::CMTPImagePropertiesCache::EImageBitDepth);            
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
                    
                    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYFROMMDSL, 
                            "GetPropertyFromMdsL - from MdS: URI:%S, DateCreated:%S", 
                            iObjectInfo->DesC(CMTPObjectMetaData::ESuid), timeValue );
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
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETPROPERTYFROMMDSL_EXIT );
    }

TBool CMTPImageDpObjectPropertyMgr::GetYear(const TDesC& aDateString, TInt& aYear) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETYEAR_ENTRY );
    aYear = 0;
    TLex dateBuf(aDateString.Left(4));
	OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETYEAR_EXIT );
    return dateBuf.Val(aYear) == KErrNone;
    }

TBool CMTPImageDpObjectPropertyMgr::GetMonth(const TDesC& aDateString, TMonth& aMonth) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETMONTH_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETMONTH_EXIT );
    return result;
    }

TBool CMTPImageDpObjectPropertyMgr::GetDay(const TDesC& aDateString, TInt& aDay) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETDAY_ENTRY );
    TBool result = EFalse;
    aDay = 0;
    TLex dateBuf(aDateString.Mid(6, 2));
    if(dateBuf.Val(aDay) == KErrNone && aDay > 0 && aDay < 32)
        {
        aDay--;
        result = ETrue;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETDAY_EXIT );
    return result;	
    }

TBool CMTPImageDpObjectPropertyMgr::GetHour(const TDesC& aDateString, TInt& aHour) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETHOUR_ENTRY );
    aHour = 0;
    TLex dateBuf(aDateString.Mid(9, 2));
	OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETHOUR_EXIT );
    return (dateBuf.Val(aHour) == KErrNone && aHour >=0 && aHour < 60);
    }
                
TBool CMTPImageDpObjectPropertyMgr::GetMinute(const TDesC& aDateString, TInt& aMinute) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETMINUTE_ENTRY );
    aMinute = 0;
    TLex dateBuf(aDateString.Mid(11, 2));
	OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETMINUTE_EXIT );
    return (dateBuf.Val(aMinute) == KErrNone && aMinute >=0 && aMinute < 60);
    }

TBool CMTPImageDpObjectPropertyMgr::GetSecond(const TDesC& aDateString, TInt& aSecond) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETSECOND_ENTRY );
    aSecond = 0;
    TLex dateBuf(aDateString.Mid(13, 2));
	OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETSECOND_EXIT );
    return (dateBuf.Val(aSecond) == KErrNone && aSecond >= 0 && aSecond < 60);
    }

TBool CMTPImageDpObjectPropertyMgr::GetTenthSecond(const TDesC& aDateString, TInt& aTenthSecond) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETTENTHSECOND_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_GETTENTHSECOND_EXIT );
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
        OstTrace0( TRACE_ERROR, CMTPIMAGEDPOBJECTPROPERTYMGR_CONVERTMTPTIMESTR2TTIMEL, "Failed to extract date/time details!");
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
    OstTraceFunctionEntry0( CMTPIMAGEDPOBJECTPROPERTYMGR_REMOVEPROPERTY_ENTRY );
    TInt index;
    CMdEProperty* property;
    index = aObject.Property(aPropDef, property);
    if (index != KErrNotFound)
        {
        aObject.RemoveProperty(index);
        }
    OstTraceFunctionExit0( CMTPIMAGEDPOBJECTPROPERTYMGR_REMOVEPROPERTY_EXIT );
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

void CMTPImageDpObjectPropertyMgr::ClearAllCache()
    {
    while (iPropretiesCacheMap.Count())
        {
        RHashMap<TUint, CMTPImagePropertiesCache*>::TIter iter(iPropretiesCacheMap);
        DestroyPropertiesCahce(*iter.NextKey());
        };
    }

void CMTPImageDpObjectPropertyMgr::ClearCache(TUint aHandle)
    {
    DestroyPropertiesCahce(aHandle);    
    }

void CMTPImageDpObjectPropertyMgr::OpenMdeObjectL()
    {
    if (iObject == NULL)
        {
		OstTraceExt1( TRACE_NORMAL, CMTPIMAGEDPOBJECTPROPERTYMGR_OPENMDEOBJECTL, 
		        "OpenMdeObjectL - URI = %S", iObjectInfo->DesC(CMTPObjectMetaData::ESuid) );
		
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

TBool CMTPImageDpObjectPropertyMgr::FindPropertiesCache(TUint aObjectHandle)
    {
    TBool ret = EFalse;
    CMTPImagePropertiesCache** ppCache = iPropretiesCacheMap.Find(aObjectHandle);
    if (ppCache)
        {
        iCurrentPropertiesCache = *ppCache;
        ret = (iCurrentPropertiesCache != NULL) ? ETrue : EFalse;
        }
    else
        {
        iCurrentPropertiesCache = NULL;
        ret = EFalse;
        }
    
    return ret;
    }

void CMTPImageDpObjectPropertyMgr::DestroyPropertiesCahce(TUint aObjectHandle)
    {
    CMTPImagePropertiesCache** ppCache = iPropretiesCacheMap.Find(aObjectHandle);  
    if (ppCache)
        {
        CMTPImagePropertiesCache* pCache = *ppCache;
        delete pCache;
        iPropretiesCacheMap.Remove(aObjectHandle);
        }    
    }
