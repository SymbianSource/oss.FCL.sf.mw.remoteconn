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

/**
 @file
 @internalTechnology
*/
 
#ifndef CMTPHANDLEALLOCATOR_H
#define CMTPHANDLEALLOCATOR_H

#include <e32base.h>

class CMTPObjectStore;
/** 
Defines the handle allocator which centralize the allocation of object handles

@internalTechnology
 
*/
class CMTPHandleAllocator : public CBase
	{
public:
	static CMTPHandleAllocator* NewL( CMTPObjectStore& aObjectStore );
	~CMTPHandleAllocator();

	TUint32 NextIdL(TUint aDataProviderId);
	void SetIdL( TUint32 aHandleID, TInt64 aPOUID );
	TInt64 NextPOUIDL();
	
   TBool AppendHandleBlockL( TUint aDataProviderId, TUint aNextID, TInt aSpace );
	   
private:
	CMTPHandleAllocator( CMTPObjectStore& aObjectStore );	
	void ConstructL();
    void ExtendArrayL(TUint aDataProviderId);

	TUint32 NextIDFromPoolL( TUint aDataProviderId );
	
    /*
     * Defines the HandleID cache of one data provider. The HandleIDs have been allocated, but now
     * they are available because the corresponding Object have been deleted or ... 
     *  
     * When the HandleID of one data provider is over flow, we have to search the DB to get the available
     * HandleIDs that are not registed in DB.
     * 
     */
    class CDPHandleCache : public CBase
        {
    public:
        
        class THandleBlock
            {
            public:
                THandleBlock( TUint aNextID, TInt aSpace );
            public:
                TUint    iNextID;
                TInt     iSpace;
            };
        
    public:
        static TInt HanldeCacheOrderFromAscending( const CDPHandleCache& aL, const CDPHandleCache& aR);
        static TInt HanldeCacheOrderFromKeyAscending( const TUint* aL, const CDPHandleCache& aR);
        
    public:
        static CDPHandleCache* NewLC( TUint aDataProviderId );
        
        ~CDPHandleCache();
        
        TUint32 NextHandleID();
        void AppendL( const THandleBlock& aBlock );
        
        inline TUint DPID() const
            {
            return iDPID;
            }
        
        inline TBool HasCache() const
            {
            return (iBlocks.Count() != 0);
            }
        
        inline TInt  BlocksCount() const
            {
            return iBlocks.Count();
            }
        
        static inline TInt MaxNumOfBlocks()
            {
            return iNumOfBlocks;
            }
        
    private:
        CDPHandleCache(TUint aDataProviderId);
        void ConstructL();
        
    private:
        /*
         * The number of blocks 
         */
        static const TInt           iNumOfBlocks = 10;
        
        /*
         * Data Provider ID
         */
        TUint                       iDPID;
        
        /*
         * HandleID Blocks
         */
        RArray<THandleBlock>        iBlocks;
        };
    
private:
    CMTPObjectStore&                iObjectStore;
    
	RArray<TUint>					iNextIds;	

	TInt64							iNextPOUID;
	
	//For ObjectID overflow
	RPointerArray<CDPHandleCache>   iNextIDPool;
	};
#endif //CMTPHANDLEALLOCATOR_H	





