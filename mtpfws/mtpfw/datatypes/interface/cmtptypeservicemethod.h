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

#ifndef CMTPTYPESERVICEMETHOD_H_
#define CMTPTYPESERVICEMETHOD_H_

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypelist.h>

class CMTPTypeArray;
class CMTPTypeString;
class CMTPTypeServiceMethodElement;

class CMTPTypeServiceMethodList : public CMTPTypeList
	{
public:
    
	IMPORT_C static CMTPTypeServiceMethodList* NewL();
	IMPORT_C static CMTPTypeServiceMethodList* NewLC();
	IMPORT_C virtual ~CMTPTypeServiceMethodList();  
    
    IMPORT_C void AppendL(CMTPTypeServiceMethodElement* aElement);
    IMPORT_C CMTPTypeServiceMethodElement& ElementL(TUint aIndex) const; 
    
private:

	CMTPTypeServiceMethodList();
    void ConstructL();
   
	};


class CMTPTypeServiceMethodElement : public CMTPTypeCompoundBase
	{
public:
	
	enum TMTPServiceMethodElement
		{
		EMethodCode,
		EMethodGUID,
		EMethodName,
		EMethodAssociateFormate,
		ENumElements
		};
	
public:
    
	IMPORT_C static CMTPTypeServiceMethodElement* NewL();
	IMPORT_C static CMTPTypeServiceMethodElement* NewLC();
	IMPORT_C static CMTPTypeServiceMethodElement* NewL(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate);
	IMPORT_C static CMTPTypeServiceMethodElement* NewLC(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate);
    
	IMPORT_C virtual ~CMTPTypeServiceMethodElement();   
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
private:

	CMTPTypeServiceMethodElement();
    void ConstructL();
    void ConstructL(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate);
    MMTPType* NewFlatChunkL(const TElementInfo& aElementInfo);
    
private:
    
	/**
    Data chunk identifiers.
    */
    enum TChunkIds
    	{
    	/**
        The flat data chunk ID.
        */
        EIdFlat1Chunk,
               
        EIdServiceMethodNameChunk,
        
        EIdFlat2Chunk,
        
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
    The flat data chunk comprising elements .
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat1;
    
    RMTPTypeCompoundFlatChunk                       iChunkFlat2;
    
    /**
    The size in bytes of the first flat data chunk.
    */
    static const TUint                              KFlat1ChunkSize;
    
    /**
    The size in bytes of the second flat data chunk.
    */
    static const TUint                              KFlat2ChunkSize;
       
    /**
    The MTP string type data chunks (EStorageDescription, and EVolumeIdentifier).
    */
    CMTPTypeString*                                         iChunkString;       
    
	};

#endif /*CMTPTYPESERVICEMETHOD_H_*/
