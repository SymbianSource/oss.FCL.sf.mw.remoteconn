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


#include <mtp/mtpprotocolconstants.h>
#include "cmtphandleallocator.h"
#include "tmtptypeobjecthandle.h"
#include "dbutility.h"
#include "cmtpobjectstore.h"
    
static const TUint KMTPMinimumId = 0x00000000;
static const TUint KMTPMaximumId = 0x00FFFFFE;
static const TInt64 KMTPPOUIDStart = -1;
static const TInt  KMTPMaxDataProviderGranularity = 8;
#if defined(_DEBUG)
static const TInt  KMTPMaxDataProviderId = 0x00000100;
#endif


/**
Two-phase object construction
*/
CMTPHandleAllocator* CMTPHandleAllocator::NewL( CMTPObjectStore& aObjectStore )
	{
	CMTPHandleAllocator* self = new (ELeave) CMTPHandleAllocator(aObjectStore);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
/**
destructor
*/	
CMTPHandleAllocator::~CMTPHandleAllocator()
	{
	iNextIds.Close();
	
    iNextIDPool.ResetAndDestroy();
	}

/**
Get the next object identifier (Handle) for the specified data provider.
@param aDataProviderId	The Id of the data provider 
@return the next object identifier (Handle)
@leave KErrOverFlow	if the object identifier has been exhausted for the data provider
*/	
TUint32 CMTPHandleAllocator::NextIdL(TUint aDataProviderId)
	{
	__ASSERT_DEBUG(aDataProviderId < KMTPMaxDataProviderId, User::Invariant());
	if (aDataProviderId >= iNextIds.Count())
	    {
	    ExtendArrayL(aDataProviderId);
	    }
	
	TUint result = ++iNextIds[aDataProviderId];
	if(result > KMTPMaximumId)
		{
		--iNextIds[aDataProviderId];
        result = NextIDFromPoolL(aDataProviderId);
		}
	
	TMTPTypeObjectHandle handle(result,  aDataProviderId);
	return handle.Value(); 
	}

TInt64 CMTPHandleAllocator::NextPOUIDL()
{
	return ++iNextPOUID;
}
/**
Initialize the allocator for this DP.  This must be called if and only if the
DP uses persistent objects, and some persistent objects exist in the object
store, so that later allocations do not duplicate earlier object IDs.
If it is called, it must therefore be called before any new objects are
allocated for this DP.
@param aDataProviderId	Id of the data provider 
@param aObjectId Object Id of the largest object already allocated,
or KMTPHandleNone if no objects have been allocated
*/

void CMTPHandleAllocator::SetIdL( TUint32 aHandleID, TInt64 aPOUID )
	{
	TUint dpID = TMTPTypeObjectHandle::DataProviderID(aHandleID);
	TUint objID = TMTPTypeObjectHandle::ObjectID(aHandleID);

	__ASSERT_DEBUG(dpID < KMTPMaxDataProviderId, User::Invariant());
	
	if ( dpID >= iNextIds.Count())
	    {
	    ExtendArrayL(dpID);
	    }
	
	if( objID > iNextIds[dpID] )
	    {
	    iNextIds[dpID] = objID ;
	    }
	if( aPOUID > iNextPOUID )
	    {
	    iNextPOUID = aPOUID;
	    }
	}

/**
Standard C++ constructor
*/	
CMTPHandleAllocator::CMTPHandleAllocator( CMTPObjectStore& aObjectStore):
	iObjectStore(aObjectStore),
	iNextIds(KMTPMaxDataProviderGranularity),
	iNextPOUID(KMTPPOUIDStart)
	{
	}

/**
Extends the array to handle more data providers. DP IDs are allocated
sequentially, so this way the array never needs to be much larger
than necessary.  This must only be called if the array size needs to be increased.
@param aDataProviderId the array must be sized to fit this data provider ID
*/	
void CMTPHandleAllocator::ExtendArrayL(TUint aDataProviderId)
	{
	__ASSERT_DEBUG(aDataProviderId >= iNextIds.Count(), User::Invariant());
	TInt increase = aDataProviderId - iNextIds.Count() + KMTPMaxDataProviderGranularity;
	while (increase--)
		{
		User::LeaveIfError(iNextIds.Append(KMTPMinimumId));
		}
	}
	
/**
Second-phase construction
*/	
void CMTPHandleAllocator::ConstructL()
	{
	ExtendArrayL(0);
	}

TBool CMTPHandleAllocator::AppendHandleBlockL( TUint aDataProviderId, TUint aNextID, TInt aSpace )
    {
    TInt index = iNextIDPool.FindInOrder( aDataProviderId, CDPHandleCache::HanldeCacheOrderFromKeyAscending );
    if( index == KErrNotFound )
        {
        CDPHandleCache* cache = CDPHandleCache::NewLC(aDataProviderId);
        iNextIDPool.InsertInOrderL(cache , TLinearOrder<CDPHandleCache>(CDPHandleCache::HanldeCacheOrderFromAscending));
        CleanupStack::Pop(cache);
        
        index = iNextIDPool.FindInOrder( aDataProviderId, CDPHandleCache::HanldeCacheOrderFromKeyAscending );
        }
    
    iNextIDPool[index]->AppendL( CDPHandleCache::THandleBlock( TMTPTypeObjectHandle::ObjectID(aNextID), aSpace) );
    
    
    if(iNextIDPool[index]->BlocksCount() < CDPHandleCache::MaxNumOfBlocks() )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

TUint32 CMTPHandleAllocator::NextIDFromPoolL( TUint aDataProviderId )
    {
    TInt index = iNextIDPool.FindInOrder( aDataProviderId, CDPHandleCache::HanldeCacheOrderFromKeyAscending );
    if( index == KErrNotFound )
        {
        iObjectStore.CalcFreeHandlesL(aDataProviderId);
        
        index = iNextIDPool.FindInOrder( aDataProviderId, CDPHandleCache::HanldeCacheOrderFromKeyAscending );
        if( index == KErrNotFound )
            {
            User::Leave(KErrOverflow);
            }
        }
    
    TUint32 ret = iNextIDPool[index]->NextHandleID();
    if( !iNextIDPool[index]->HasCache() )
        {
        iNextIDPool.Remove(index);
        }
    
    if(ret > KMTPMaximumId )
        {
        User::Leave(KErrOverflow);
        }
    
    return ret;
    }

CMTPHandleAllocator::CDPHandleCache::THandleBlock::THandleBlock(TUint aNextID, TInt aSpace):
    iNextID(aNextID),
    iSpace(aSpace)
    {
    }

TInt CMTPHandleAllocator::CDPHandleCache::HanldeCacheOrderFromAscending( const CDPHandleCache& aL, const CDPHandleCache& aR)
    {
    return aL.DPID() - aR.DPID();
    }

TInt CMTPHandleAllocator::CDPHandleCache::HanldeCacheOrderFromKeyAscending( const TUint* aL, const CDPHandleCache& aR)
    {
    return (*aL) - aR.DPID();
    }

CMTPHandleAllocator::CDPHandleCache::CDPHandleCache(TUint aDataProviderId ):
    iDPID(aDataProviderId)
    {
    }

CMTPHandleAllocator::CDPHandleCache::~CDPHandleCache()
    {
    iBlocks.Close();
    }

void CMTPHandleAllocator::CDPHandleCache::ConstructL()
    {
    }

CMTPHandleAllocator::CDPHandleCache* CMTPHandleAllocator::CDPHandleCache::NewLC( TUint aDataProviderId )
    {
    CDPHandleCache* self = new (ELeave) CDPHandleCache(aDataProviderId);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

TUint32 CMTPHandleAllocator::CDPHandleCache::NextHandleID()
    {
    TUint32 ret = KMTPMaximumId + 1;
    if(iBlocks.Count() == 0)
        {
        return ret;
        }
    
    ret = iBlocks[0].iNextID;
    --iBlocks[0].iSpace;
    
    if( 0 == iBlocks[0].iSpace )
        {
        iBlocks.Remove(0);
        }
    else
        {
        iBlocks[0].iNextID++;
        }
    
    return ret;
    }

void CMTPHandleAllocator::CDPHandleCache::AppendL( const THandleBlock& aBlock )
    {
    iBlocks.AppendL( aBlock );
    }
