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


#ifndef CMTPTYPESERVICECAPABILITYLIST_H_
#define CMTPTYPESERVICECAPABILITYLIST_H_

#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypelist.h>

class CMTPTypeList;
class CMTPTypeObjectPropDesc;
class CMTPTypeInterdependentPropDesc;
class CMTPTypeFormatCapability;

class CMTPTypeServiceCapabilityList : public CMTPTypeList
	{
public:
	
	IMPORT_C static CMTPTypeServiceCapabilityList* NewL();
	IMPORT_C static CMTPTypeServiceCapabilityList* NewLC();
    
	IMPORT_C virtual ~CMTPTypeServiceCapabilityList(); 
    IMPORT_C void AppendL(CMTPTypeFormatCapability* aFormatCapability);  
    IMPORT_C CMTPTypeFormatCapability& ElementL(TUint aIndex) const;

private:

	CMTPTypeServiceCapabilityList();
    void ConstructL();
	};



class CMTPTypeFormatCapability : public CMTPTypeCompoundBase
    {
public:
    
    enum TMTPFormatCapabilityElement
        {
        EFormatCode,
        ENumElements,
        };
public:

    IMPORT_C static CMTPTypeFormatCapability* NewL();
    IMPORT_C static CMTPTypeFormatCapability* NewLC();
    IMPORT_C static CMTPTypeFormatCapability* NewL( const TUint16 aFormatCode, CMTPTypeInterdependentPropDesc* aInterDependentPropDesc );
    IMPORT_C static CMTPTypeFormatCapability* NewLC( const TUint16 aFormatCode, CMTPTypeInterdependentPropDesc* aInterDependentPropDesc );
    
    IMPORT_C void AppendL(CMTPTypeObjectPropDesc* aElement);
    IMPORT_C TUint32 NumberOfPropDescs() const;
    IMPORT_C CMTPTypeObjectPropDesc& ObjectPropDescL(TUint aIndex) const;  
    
    IMPORT_C CMTPTypeInterdependentPropDesc& InterdependentPropDesc() const;  
    IMPORT_C virtual ~CMTPTypeFormatCapability();   
    
public: 
    IMPORT_C TUint Type() const;

    
private: // From CMTPTypeCompoundBase
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
protected:// From CMTPTypeCompoundBase
    TBool ReadableElementL(TInt aElementId) const;
    TBool WriteableElementL(TInt aElementId ) const;
    
private:
    CMTPTypeFormatCapability( CMTPTypeInterdependentPropDesc* aInterDependentPropDesc );
    void ConstructL();
    void ConstructL( const TUint16 aFormatCode );
    MMTPType* NewFlatChunkL();
    
private:

    /**
    Data chunk identifiers.
    */
    enum TChunkIds
        {
        /**
        The flat data chunk ID.
        */
        EIdFlatChunk,
        
        /*
         * The CMTPTypeObjectPropDescList dataset.
         */
        EIdObjPropDescListChunk,
        
        /*
         The CMTPTypeInterdependentPropDesc dataset.
         */
        EIdInterDependentPropDescChunk,
        /**
        The number of data chunks.
        */
        EIdNumChunks
        };
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
       
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
     
    /**
    The flat data chunk comprising elements EStorageType to EFreeSpaceInObjects.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat;
    
private:
    
    CMTPTypeList*                       iObjectPropDescList;
    CMTPTypeInterdependentPropDesc*    iInterdependentPropDesc;
    
    };

#endif /*CMTPTYPESERVICECAPABILITYLIST_H_*/
