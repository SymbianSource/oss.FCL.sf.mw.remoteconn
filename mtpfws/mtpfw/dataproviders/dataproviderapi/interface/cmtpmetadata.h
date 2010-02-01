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
 @publishedPartner
 @released
*/

#ifndef CMTPMETADATA_H
#define CMTPMETADATA_H

#include <badesca.h>
#include <e32base.h>

/** 
Defines the MTP meta-data base class.
@publishedPartner
@released
*/
class CMTPMetaData : public CBase
    {        
public:
    /* 
     * Store the hash code for the folder
     */
    struct TPathHash
    	{
    	TUint32 iHash;
    	TUint   iIndex;
    	};

public:
    
    IMPORT_C virtual ~CMTPMetaData();
    
    IMPORT_C const TDesC& DesC(TUint aId) const;
    IMPORT_C const CDesCArray& DesCArray(TUint aId);
    IMPORT_C TInt Int(TUint aId) const;
    IMPORT_C const RArray<TInt>& IntArray(TUint aId) const;
    IMPORT_C TUint Uint(TUint aId) const;
    IMPORT_C const RArray<TUint>& UintArray(TUint aId) const;
    
    IMPORT_C void GetIntArrayL(TUint aId, RArray<TInt>& aValue);
    IMPORT_C void GetUintArrayL(TUint aId, RArray<TUint>& aValue);
    
    IMPORT_C void SetDesCL(TUint aId, const TDesC& aValue);
    IMPORT_C void SetDesCArrayL(TUint aId, const CDesCArrayFlat& aValue);
    IMPORT_C void SetDesCArrayL(TUint aId, const CDesCArraySeg& aValue);
    IMPORT_C void SetInt(TUint aId, TInt aValue);
    IMPORT_C void SetIntArrayL(TUint aId, const RArray<TInt>& aValue);
    IMPORT_C void SetUint(TUint aId, TUint aValue);
    IMPORT_C void SetUintArrayL(TUint aId, const RArray<TUint>& aValue);
    IMPORT_C void SetHashPath(const TDesC16& aExclusionPath, TUint aIndex);
    IMPORT_C const RArray<TPathHash>& GetHashPathArray();
    
    IMPORT_C TAny* GetExtendedInterface(TUid aInterfaceUid);
    IMPORT_C static TInt CompareTPathHash(const TPathHash&, const TPathHash&);
    
protected:

    /**
    The meta-data element data type identifiers.
    */
    enum TType
        {
        /**
        Unknown (null).
        */
        EUndefined  = 0,
        
        /**
        @see TDesC.
        */
        EDesC       = 1,
        
        /**
        @see CDesCArray.
        */
        EDesCArray  = 2,
        
        /**
        @see TInt.
        */
        EInt        = 3,
        
        /**
        @see Rarray<TInt>.
        */
        EIntArray   = 4,
        
        /**
        @see TUint.
        */
        EUint       = 5,
        
        /**
        @see Rarray<TUint>.
        */
        EUintArray  = 6,
        };

    /**
    Describes the meta-data element.
    */
    struct TElementMetaData
        {
        /**
        The element buffer offset.
        */
        TUint   iOffset;
        
        /**
        The element data type identifier.
        */
        TType   iType;
        };
    
protected:

    CMTPMetaData(const TElementMetaData* aElements, TUint aCount);  
    void ConstructL();
    void ConstructL(const CMTPMetaData& aFrom);

private:
    
    CMTPMetaData();
    
    void CopyL(const CDesCArray& aFrom, CDesCArray& aTo);
    void CopyL(const RArray<TInt>& aFrom, RArray<TInt>& aTo);
    void CopyL(const RArray<TUint>& aFrom, RArray<TUint>& aTo);
    
    void DeleteIntArray(TUint aIdx);
    void DeleteUintArray(TUint aIdx);

private:  // Owned

    /**
    The default array granularity
    */
    static const TUint              KGranularity = 8;
    
    /**
    The element meta-data.
    */
    const RArray<TElementMetaData>  iElements;
    
    /**
    The DesC element data.
    */
    RPointerArray<HBufC>            iElementsDesC;
    
    /**
    The DesCArray element data.
    */
    RPointerArray<CDesCArray>       iElementsDesCArray;

    /**
    The Int element data.
    */
    RArray<TInt>                    iElementsInt;
    
    /**
    The IntArray element data.
    */
    RPointerArray<TAny>             iElementsIntArray;

    /**
    The Uint element data.
    */
    RArray<TUint>                   iElementsUint;
    
    /**
    The UintArray element data.
    */
    RPointerArray<TAny>             iElementsUintArray;
    
    /**
    The extended data object pointer.
    */
    TAny*                           iExtensionData;
    
    /**
    The extended interface object pointer.
    */
    TAny*                           iExtensionInterfaces;

    /**
    The TDesCArrayType type element data.
    */
    RArray<TInt>  iElementsDesCArrayType;
    RArray<TPathHash>               iPathHash;

	/**
    The CDesCArray element data type identifiers.
    */
    enum TDesCArrayType 
    	{
    	/**
        @see CDesCArrayFlat.
        */
    	 EDesCArrayFlat = 0,
    		
    	/**
        @see CDesCArraySeg.
        */
    	 EDesCArraySeg = 1,
    	};
    };
    
#endif // CMTPMETADATA_H
