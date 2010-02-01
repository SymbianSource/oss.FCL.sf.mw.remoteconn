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

#ifndef CMTPTYPEOBJECT_H
#define CMTPTYPEOBJECT_H

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeString;

/** 
Defines the MTP ObjectInfo dataset. The ObjectInfo dataset describes the core
properties of an MTP data object, and is generated in response to a 
GetObjectInfo operation request.
@publishedPartner
@released 
*/ 
class CMTPTypeObjectInfo : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP ObjectInfo dataset element identifiers.
    */
    enum TMTPObjectInfoElement
        {
        /**
        StorageID.
        */
        EStorageID,
        
        /**
        Object Format.
        */
        EObjectFormat,
        
        /**
        Protection Status.
        */
        EProtectionStatus,
        
        /**
        Object Compressed Size.
        */
        EObjectCompressedSize,
        
        /**
        Thumb Format.
        */
        EThumbFormat,
        
        /**
        Thumb Compressed Size.
        */
        EThumbCompressedSize,
        
        /**
        Thumb Pix Width.
        */
        EThumbPixWidth,
        
        /**
        Thumb Pix Height.
        */
        EThumbPixHeight,
        
        /**
        Image Pix Width.
        */
        EImagePixWidth,
        
        /**
        Image Pix Height.
        */
        EImagePixHeight,
        
        /**
        Image Bit Depth.
        */
        EImageBitDepth,
        
        /**
        Parent Object.
        */
        EParentObject,
        
        /**
        Association Type.
        */
        EAssociationType,
        
        /**
        Association Description.
        */
        EAssociationDescription,
        
        /**
        Sequence Number.
        */
        ESequenceNumber,
        
        /**
        Filename.
        */
        EFilename,
        
        /**
        Date Created.
        */
        EDateCreated,
        
        /**
        Date Modified.
        */
        EDateModified,
        
        /**
        Keywords.
        */
        EKeywords,        
        
        /**
        The number of dataset elements.
        */
        ENumElements,
        };

public:
    
    IMPORT_C static CMTPTypeObjectInfo* NewL();
    IMPORT_C static CMTPTypeObjectInfo* NewLC();
    
    IMPORT_C virtual ~CMTPTypeObjectInfo();   
     
public: // From MMTPType

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

private:

    CMTPTypeObjectInfo();
    void ConstructL();

    MMTPType* NewFlatChunkL(const TElementInfo& aElementInfo);
    MMTPType* NewStringChunkL(const TElementInfo& aElementInfo);

private:
        
    /**
    Data chunk identifiers.
    */
    enum TChunkIds
        {
        /**
        The first flat data chunk ID.
        */
        EIdFlat1Chunk,
        
        /**
        The EFilename element data chunk ID.
        */
        EIdFilenameChunk,
        
        /**
        The EDateCreated element data chunk ID.
        */
        EIdDateCreatedChunk,
        
        /**
        The EDateModified element data chunk ID.
        */
        EIdDateModifiedChunk,
        
        /**
        The EKeywords element data chunk ID.
        */
        EIdKeywordsChunk,
        
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
    The flat data chunk comprising elements EStorageID to EParentObject.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat;
    
    /**
    MTP string type data chunks (EFilename, EDateCreated, EDateModified, and 
    EKeywords).
    */
    RPointerArray<CMTPTypeString>                   iChunksString;
    };
    
#endif // CMTPTYPEOBJECT_H
