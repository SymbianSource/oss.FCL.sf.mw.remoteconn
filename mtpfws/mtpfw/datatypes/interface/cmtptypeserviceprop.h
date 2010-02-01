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


#ifndef CMTPTYPESERVICEPROP_H_
#define CMTPTYPESERVICEPROP_H_

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypelist.h>
#include <mtp/cmtptypeserviceproplist.h>

class CMTPTypeString;
class CMTPTypeServicePropertyElement;

class CMTPTypeServicePropertyList : public CMTPTypeList
	{
public:
    
	IMPORT_C static CMTPTypeServicePropertyList* NewL();
    IMPORT_C static CMTPTypeServicePropertyList* NewLC();

    IMPORT_C virtual ~CMTPTypeServicePropertyList();  
    
    IMPORT_C void AppendL(CMTPTypeServicePropertyElement* aElement);
    
    IMPORT_C CMTPTypeServicePropertyElement& ElementL(TUint aIndex) const; 
     
private:

	CMTPTypeServicePropertyList();
    void ConstructL();

	};

class CMTPTypeServicePropertyElement : public CMTPTypeCompoundBase
	{
public:
	
	enum TMTPServicePropertyElement
		{
		EPropertyCode,
		ENameSpace,
		EPkeyID,
		EPorpertyName,
		ENumElements
		};
	
public:
    
	IMPORT_C static CMTPTypeServicePropertyElement* NewL();
	IMPORT_C static CMTPTypeServicePropertyElement* NewLC();
    IMPORT_C static CMTPTypeServicePropertyElement* NewL(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName);
    IMPORT_C static CMTPTypeServicePropertyElement* NewLC(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName);
    
	IMPORT_C virtual ~CMTPTypeServicePropertyElement();   
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

private:

	CMTPTypeServicePropertyElement();
    void ConstructL();
    
    void ConstructL(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName);
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
               
        
        EIdServicePropNameChunk,
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
       
    /**
    The MTP string type data chunks (EStorageDescription, and EVolumeIdentifier).
    */
    CMTPTypeString*                                          iChunkString;    
	};

#endif /*CMTPTYPESERVICEPROP_H_*/
