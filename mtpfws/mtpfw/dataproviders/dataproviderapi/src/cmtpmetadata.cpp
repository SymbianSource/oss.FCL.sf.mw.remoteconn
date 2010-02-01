// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/cmtpmetadata.h>
#include <e32hashtab.h>
#include <e32debug.h>

#ifdef _DEBUG
/**
CMTPMetaData panic category.
*/
_LIT(KMTPPanicCategory, "CMTPMetaData");

/**
CMTPMetaData panic reasons.
*/
enum TMTPPanicReasons
    {
    EPanicBadLayout     = 0,
    EPanicTypeMismatch  = 1,
    EPanicTypeUnknown   = 2,
    };

/**
Creates a CMTPMetaData category panic condition.
@param The panic condition reason code.
@panic CMTPMetaData aReason Always.
*/
LOCAL_C void Panic(TInt aReason)
    {
    User::Panic(KMTPPanicCategory, aReason);
    }
#endif // _DEBUG

/**
Destructor.
*/
EXPORT_C CMTPMetaData::~CMTPMetaData()
    {
    iElementsDesC.ResetAndDestroy();
    iElementsDesCArray.ResetAndDestroy();
    iElementsDesCArrayType.Close();	
    iElementsDesCArrayType.Reset();
    iElementsInt.Reset();
    TInt count(iElementsIntArray.Count());
    while (count)
        {
        DeleteIntArray(--count);
        }
    iElementsIntArray.Reset();
    
    iElementsUint.Reset();
    count = (iElementsUintArray.Count());
    while (count)
        {
        DeleteUintArray(--count);
        }
    iElementsUintArray.Reset();	
	iPathHash.Close();
    } 
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C const TDesC& CMTPMetaData::DesC(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EDesC), Panic(EPanicTypeMismatch));
    return *iElementsDesC[iElements[aId].iOffset];
    }    
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C const CDesCArray& CMTPMetaData::DesCArray(TUint aId)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EDesCArray), Panic(EPanicTypeMismatch));
    return *iElementsDesCArray[iElements[aId].iOffset];
    }
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C TInt CMTPMetaData::Int(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EInt), Panic(EPanicTypeMismatch));
    return iElementsInt[iElements[aId].iOffset];
    }
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C const RArray<TInt>& CMTPMetaData::IntArray(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EIntArray), Panic(EPanicTypeMismatch));
    return *(reinterpret_cast<const RArray<TInt>*> (iElementsIntArray[iElements[aId].iOffset]));
    }
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C TUint CMTPMetaData::Uint(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUint), Panic(EPanicTypeMismatch));
    return iElementsUint[iElements[aId].iOffset];
    }
    
/**
Provides the value of the specified element.
@param aId The element identifier.
@return The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C const RArray<TUint>& CMTPMetaData::UintArray(TUint aId) const
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUintArray), Panic(EPanicTypeMismatch));
    return *(reinterpret_cast<const RArray<TUint>*> (iElementsUintArray[iElements[aId].iOffset]));
    }

/**
Provides a copy of the specified element.
@param aId The element identifier.
@param aValue On successful completion, a copy of the element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::GetIntArrayL(TUint aId, RArray<TInt>& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUintArray), Panic(EPanicTypeMismatch));
    CopyL(IntArray(aId), aValue);
    }

/**
Provides a copy of the specified element.
@param aId The element identifier.
@param aValue On successful completion, a copy of the element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::GetUintArrayL(TUint aId, RArray<TUint>& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUintArray), Panic(EPanicTypeMismatch));
    CopyL(UintArray(aId), aValue);
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::SetDesCL(TUint aId, const TDesC& aValue)
    {
    const TElementMetaData& KElement(iElements[aId]);
    __ASSERT_DEBUG((KElement.iType == EDesC), Panic(EPanicTypeMismatch));
    delete iElementsDesC[KElement.iOffset];
    iElementsDesC[KElement.iOffset] = NULL;
	iElementsDesC[KElement.iOffset] = aValue.AllocL();	
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::SetDesCArrayL(TUint aId, const CDesCArrayFlat& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EDesCArray), Panic(EPanicTypeMismatch));
    TUint idx(iElements[aId].iOffset);
    delete iElementsDesCArray[idx];
    iElementsDesCArray[idx] = NULL;
    iElementsDesCArray[idx] = new(ELeave) CDesCArrayFlat(KGranularity);
	iElementsDesCArrayType[idx]=EDesCArrayFlat;
    CopyL(aValue, *iElementsDesCArray[idx]);
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::SetDesCArrayL(TUint aId, const CDesCArraySeg& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EDesCArray), Panic(EPanicTypeMismatch));
    TUint idx(iElements[aId].iOffset);
    delete iElementsDesCArray[idx];
    iElementsDesCArray[idx] = NULL;
    iElementsDesCArray[idx] = new(ELeave) CDesCArraySeg(KGranularity);
	iElementsDesCArrayType[idx]=EDesCArraySeg;
    CopyL(aValue, *iElementsDesCArray[idx]);
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C void CMTPMetaData::SetInt(TUint aId, TInt aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EInt), Panic(EPanicTypeMismatch));
    iElementsInt[iElements[aId].iOffset] = aValue;
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::SetIntArrayL(TUint aId, const RArray<TInt>& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EIntArray), Panic(EPanicTypeMismatch));
    TUint idx(iElements[aId].iOffset);
    DeleteIntArray(idx);
    iElementsIntArray[idx] = new(ELeave) RArray<TInt>;
    CopyL(aValue, *(reinterpret_cast<RArray<TInt>*>(iElementsIntArray[iElements[aId].iOffset])));
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
*/
EXPORT_C void CMTPMetaData::SetUint(TUint aId, TUint aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUint), Panic(EPanicTypeMismatch));
    iElementsUint[iElements[aId].iOffset] = aValue;
    }
    
/**
Sets the value of the specified element.
@param aId The element identifier.
@param aValue The element value.
@panic CMTPMetaData 0 In debug builds, if the specified element is not of
the requested type.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPMetaData::SetUintArrayL(TUint aId, const RArray<TUint>& aValue)
    {
    __ASSERT_DEBUG((iElements[aId].iType == EUintArray), Panic(EPanicTypeMismatch));
    TUint idx(iElements[aId].iOffset);
    DeleteUintArray(idx);
    iElementsUintArray[idx] = new(ELeave) RArray<TUint>;
    CopyL(aValue, *(reinterpret_cast<RArray<TUint>*>(iElementsUintArray[iElements[aId].iOffset])));
    }

/**
Provides an MTP object manager's object meta data record extension 
interface implementation for the specified interface Uid. 
@param aInterfaceUid Unique identifier for the extension interface being 
requested.
@return Pointer to an interface instance or 0 if the interface is not 
supported. Ownership is NOT transfered.
*/
EXPORT_C TAny* CMTPMetaData::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    return iExtensionInterfaces;
    }

/**
Constructor.
*/
CMTPMetaData::CMTPMetaData(const TElementMetaData* aElements, TUint aCount) :
    iElements(sizeof(*aElements), const_cast<TElementMetaData*>(aElements), aCount),
    iElementsDesC(KGranularity)
    {

    }
    
/**
Second phase constructor.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPMetaData::ConstructL()
    {
    const TUint KCount(iElements.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        const TElementMetaData& KElement(iElements[i]);
        switch (KElement.iType)
            {
        case EDesC:
            __ASSERT_DEBUG((iElementsDesC.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            iElementsDesC.AppendL(KNullDesC().AllocLC());
            CleanupStack::Pop();
            break;
            
        case EDesCArray:
        	{
            __ASSERT_DEBUG((iElementsDesCArray.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            CDesCArrayFlat* desarray = new(ELeave) CDesCArrayFlat(KGranularity);
            CleanupStack::PushL(desarray);
            iElementsDesCArray.AppendL(desarray);
            CleanupStack::Pop(desarray);
			iElementsDesCArrayType.AppendL(EDesCArrayFlat);
            break;
        	}
            
        case EInt:
            __ASSERT_DEBUG((iElementsInt.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            iElementsInt.AppendL(0);
            break;
            
        case EIntArray:
        	{
            __ASSERT_DEBUG((iElementsIntArray.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            RArray<TInt>* intarray = new(ELeave) RArray<TInt>;
            CleanupStack::PushL(intarray);
            iElementsIntArray.AppendL(intarray);
            CleanupStack::Pop(intarray);
            break;
        	}
            
        case EUint:
            __ASSERT_DEBUG((iElementsUint.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            iElementsUint.AppendL(0);
            break;
            
        case EUintArray:
        	{
            __ASSERT_DEBUG((iElementsUintArray.Count() == KElement.iOffset), Panic(EPanicBadLayout));
            RArray<TUint>* uintarray = new(ELeave) RArray<TUint>;
            CleanupStack::PushL(uintarray);
            iElementsUintArray.AppendL(uintarray);
            CleanupStack::Pop(uintarray);
            break;
        	}
            
        default:
            __DEBUG_ONLY(Panic(EPanicTypeUnknown));
            break;
            }
        }
    }      
    
/**
Second phase copy constructor.
@param aFrom The source meta-data.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPMetaData::ConstructL(const CMTPMetaData& aFrom)
    {
    // iElementsDesC
    TUint count(aFrom.iElementsDesC.Count());
    for (TUint i(0); (i < count); i++)
        {
        iElementsDesC.AppendL(aFrom.iElementsDesC[i]->AllocLC());
        CleanupStack::Pop();
        }
    
    // iElementsDesCArray 
    count = aFrom.iElementsDesCArray.Count();
    for (TUint i(0); (i < count); i++)
        {
        if ((aFrom.iElementsDesCArrayType[i])==EDesCArrayFlat)
            {
            CDesCArrayFlat* flatarray = new(ELeave) CDesCArrayFlat(KGranularity);
        	CleanupStack::PushL(flatarray);
            iElementsDesCArray.AppendL(flatarray);
            CleanupStack::Pop(flatarray);
            }
        else
            {
            CDesCArraySeg* segarray = new(ELeave) CDesCArraySeg(KGranularity);
        	CleanupStack::PushL(segarray);
            iElementsDesCArray.AppendL(segarray);
            CleanupStack::Pop(segarray);
            }
        CopyL(*aFrom.iElementsDesCArray[i], *iElementsDesCArray[i]);
        }
    
    // iElementsInt    
    CopyL(aFrom.iElementsInt, iElementsInt);
    
    // iElementsIntArray
    count = aFrom.iElementsIntArray.Count();
    for (TUint i(0); (i < count); i++)
        {        
        RArray<TInt>* intarray = new(ELeave) RArray<TInt>;
    	CleanupStack::PushL(intarray);
        iElementsIntArray.AppendL(intarray);
        CleanupStack::Pop(intarray);
        const RArray<TInt>& from(*reinterpret_cast<RArray<TInt>*>(aFrom.iElementsIntArray[i]));
        RArray<TInt>& to(*reinterpret_cast<RArray<TInt>*>(iElementsIntArray[i]));
        CopyL(from, to);
        }
        
    // iElementsUint
    CopyL(aFrom.iElementsUint, iElementsUint);
    
    // iElementsIntArray
    count = aFrom.iElementsUintArray.Count();
    for (TUint i(0); (i < count); i++)
        {
        RArray<TUint>* array = new(ELeave) RArray<TUint>;
    	CleanupStack::PushL(array);
        iElementsUintArray.AppendL(array);
        CleanupStack::Pop(array);
        const RArray<TUint>& from(*reinterpret_cast<RArray<TUint>*>(aFrom.iElementsUintArray[i]));
        RArray<TUint>& to(*reinterpret_cast<RArray<TUint>*>(iElementsUintArray[i]));
        CopyL(from, to);
        }
    // iPathHash
    for(TInt i=0; i<aFrom.iPathHash.Count(); ++i)
    	{
    	this->iPathHash.Append(aFrom.iPathHash[i]);
    	}
    }

/**
Default constructor.
*/
CMTPMetaData::CMTPMetaData() :
    iElementsDesC(KGranularity)
    {
    
    }  
    
/**
Copies the specified array contents.
@param aFrom The source array.
@param aTo The target array.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPMetaData::CopyL(const CDesCArray& aFrom, CDesCArray& aTo)
    {
    const TUint KCount(aFrom.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        aTo.AppendL(aFrom[i]);
        }
    }
    
/**
Copies the specified array contents.
@param aFrom The source array.
@param aTo The target array.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPMetaData::CopyL(const RArray<TInt>& aFrom, RArray<TInt>& aTo)
    {
    aTo.Reset();
    const TUint KCount(aFrom.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        aTo.AppendL(aFrom[i]);
        }
    }
    
/**
Copies the specified array contents.
@param aFrom The source array.
@param aTo The target array.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPMetaData::CopyL(const RArray<TUint>& aFrom, RArray<TUint>& aTo)
    {
    aTo.Reset();
    const TUint KCount(aFrom.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        aTo.AppendL(aFrom[i]);
        }
    }
    
/**
Deletes the specified IntArray.
@param aIdx The iElementsIntArray index.
*/
void CMTPMetaData::DeleteIntArray(TUint aIdx)
    {
    RArray<TInt>* array(reinterpret_cast<RArray<TInt>*>(iElementsIntArray[aIdx]));
    array->Reset();
    delete array;
    }
    
/**
Deletes the specified IntArray.
@param aIdx The iElementsIntArray index.
*/
void CMTPMetaData::DeleteUintArray(TUint aIdx)
    {
    RArray<TUint>* array(reinterpret_cast<RArray<TUint>*>(iElementsUintArray[aIdx]));
    array->Reset();
    delete array;
    }
EXPORT_C void CMTPMetaData::SetHashPath(const TDesC16& aExclusionPath, TUint aIndex)
	{
	TFileName ex(aExclusionPath);
	ex.LowerCase();
	TUint32 HashCode=DefaultHash::Des16(ex);
	TPathHash entry;
    entry.iHash=HashCode;
    entry.iIndex=aIndex;
	
	TLinearOrder<TPathHash> order(CMTPMetaData::CompareTPathHash);
	iPathHash.InsertInOrderAllowRepeats(entry, order);
	}

EXPORT_C TInt CMTPMetaData::CompareTPathHash(const TPathHash& aVal1, const TPathHash& aVal2)
	{
	return (aVal1.iHash>aVal2.iHash)?1:((aVal1.iHash==aVal2.iHash)?0:-1);
	}

EXPORT_C const RArray<CMTPMetaData::TPathHash>& CMTPMetaData::GetHashPathArray()
	{
	return iPathHash;
	}
