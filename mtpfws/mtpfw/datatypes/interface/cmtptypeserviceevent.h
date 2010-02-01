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

#ifndef CMTPTYPESERVICEEVENT_H_
#define CMTPTYPESERVICEEVENT_H_

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypelist.h>

class CMTPTypeArray;
class CMTPTypeString;
class CMTPTypeServiceEventElement;

class CMTPTypeServiceEventList : public CMTPTypeList
	{
public:
    
	IMPORT_C static CMTPTypeServiceEventList* NewL();
	IMPORT_C static CMTPTypeServiceEventList* NewLC();

	IMPORT_C virtual ~CMTPTypeServiceEventList();  
    
    IMPORT_C void AppendL(CMTPTypeServiceEventElement* aElement);
    IMPORT_C CMTPTypeServiceEventElement& ElementL(TUint aIndex) const; 
    
private:

	CMTPTypeServiceEventList();
    void ConstructL();
    
	};

class CMTPTypeServiceEventElement : public CMTPTypeCompoundBase
	{
public:
	
	enum TMTPServiceEventElement
		{
		EEventCode,
		EEventGUID,
		EEventName,
		ENumElements
		};
	
public:
    
	IMPORT_C static CMTPTypeServiceEventElement* NewL();
	IMPORT_C static CMTPTypeServiceEventElement* NewLC();
	IMPORT_C static CMTPTypeServiceEventElement* NewL(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName);
	IMPORT_C static CMTPTypeServiceEventElement* NewLC(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName);
    
	IMPORT_C virtual ~CMTPTypeServiceEventElement();   
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

private:

	CMTPTypeServiceEventElement();
    void ConstructL();
    void ConstructL(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName);
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
               
        EIdServiceEventNameChunk,
        
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
    The size in bytes of the flat data chunk.
    */
    static const TUint                              KFlatChunkSize;
       
    /**
    The flat data chunk comprising elements EStorageType to EFreeSpaceInObjects.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat;
       
    /**
    The MTP string type data chunks (EStorageDescription, and EVolumeIdentifier).
    */
    CMTPTypeString*           						iChunkString;  

	};

#endif /*CMTPTYPESERVICEEVENT_H_*/
