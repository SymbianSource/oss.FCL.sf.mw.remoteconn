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

#ifndef CMTPTYPESERVICEFORMAT_H_
#define CMTPTYPESERVICEFORMAT_H_

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypelist.h>

class CMTPTypeArray;
class CMTPTypeString;

class CMTPTypeServiceFormatElement;

class CMTPTypeServiceFormatList : public CMTPTypeList
	{
public:
    
	IMPORT_C static CMTPTypeServiceFormatList* NewL();
	IMPORT_C static CMTPTypeServiceFormatList* NewLC();
	IMPORT_C virtual ~CMTPTypeServiceFormatList();      
    IMPORT_C void AppendL(CMTPTypeServiceFormatElement* aElement);    
    IMPORT_C CMTPTypeServiceFormatElement& ElementL(TUint aIndex) const; 
     
private:

	CMTPTypeServiceFormatList();
    void ConstructL();
   
	};


class CMTPTypeServiceFormatElement : public CMTPTypeCompoundBase
	{
public:
	
	enum TMTPServiceFormatElement
		{
		EFormatCode,
		EFormatGUID,
		EFormatName,
		EFormatBase,
		EFormatMIMEType,
		ENumElements
		};
	
public:
	
	IMPORT_C static CMTPTypeServiceFormatElement* NewL();
	IMPORT_C static CMTPTypeServiceFormatElement* NewLC();
	IMPORT_C static CMTPTypeServiceFormatElement* NewL(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType);
	IMPORT_C static CMTPTypeServiceFormatElement* NewLC(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType);
    
	IMPORT_C virtual ~CMTPTypeServiceFormatElement();   
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

private:
	CMTPTypeServiceFormatElement();
    void ConstructL();
    
    void ConstructL(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType);
    
    MMTPType* NewFlatChunkL(const TElementInfo& aElementInfo);
    MMTPType* NewStringChunkL(const TElementInfo& aElementInfo);
    
private:

    enum TWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Streaming the property descriptor triplet dataset data chunk.
        */
        EFlat1Chunk,
                
        EFormatNameChunk,
        
        EFlat2Chunk,
        
        EFormatMIMETypeChunk
        
        };
    
	/**
    Data chunk identifiers.
    */
    enum TChunkIds
    	{
    	/**
        The flat data chunk ID.
        */
        EIdFlat1Chunk,
               
        EIdFormatNameChunk,
        
        EIdFlat2Chunk,
        
        EIdFormatMIMETypeChunk,
        
        /**
        The number of data chunks.
        */
        EIdNumChunks
    	};	
    /**
    The write data stream error state. 
    */
    TUint                                           iWriteSequenceErr;
    
    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;
    
    /**
    The write data stream completion state. This will be set to EFlat2Chunk for 
    ObjectPropDesc datasets with an EForm of ENone, otherwise this will be 
    EFormChunk.
    */
    TUint                                           iWriteSequenceCompletionState;
    
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
    
    RMTPTypeCompoundFlatChunk						iChunkFlat2;

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
    CMTPTypeString*                                       iChunkFormatNameString;   
    
    CMTPTypeString*										 iChunkMIMETypeString;
    
	};

#endif /*CMTPTYPESERVICEFORMAT_H_*/
