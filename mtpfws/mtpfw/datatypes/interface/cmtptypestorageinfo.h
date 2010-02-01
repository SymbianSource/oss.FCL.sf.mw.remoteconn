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
 @internalComponent
*/

#ifndef CMTPTYPESTORAGEINFO_H
#define CMTPTYPESTORAGEINFO_H

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeString;

/** 
Defines the MTP StorageInfo dataset. The StorageInfo dataset provides a 
description of the MTP device and is generated in response to a GetStorageInfo
operation request.
@internalComponent 
*/ 
class CMTPTypeStorageInfo : public CMTPTypeCompoundBase
    {
public:    

    /**
    MTP StotageInfo dataset element identifiers.
    */
     enum TMTPStorageInfoElement
        {
        /**
        The physical nature of the storage described by the dataset.
        */ 
        EStorageType,
        
        /**
        The logical file system in use on the storage.
        */ 
        EFileSystemType,
        
        /**
        The global write-protection status of the storage.
        */
        EAccessCapability,
        
        /**
        The maximum capacity of the storage in bytes.
        */
        EMaxCapacity,
        
        /**
        The available capacity of the storage in objects.
         */
        EFreeSpaceInBytes,
        
        /**
        The available capacity of the storage in objects. This field is only 
        used if there is a reasonable expectation that the number of objects 
        that remain to be written can be accurately predicted.
        */
        EFreeSpaceInObjects,
        
        /**
        The human-readable string identifying the storage.
        */
        EStorageDescription,
        
        /**
        The unique volume identifier of the storage, e.g. serial number.
        */
        EVolumeIdentifier,
        
        /**
        The number of dataset elements.
        */
        ENumElements
        };  

public:  
    
    IMPORT_C static CMTPTypeStorageInfo* NewL();
    IMPORT_C static CMTPTypeStorageInfo* NewLC();
    
    IMPORT_C virtual ~CMTPTypeStorageInfo();  
     
public: // From MMTPType

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const; 
    
private:

    CMTPTypeStorageInfo();        
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
        The flat data chunk ID.
        */
        EIdFlatChunk,
        
        /**
        The EStorageDescription element data chunk ID.
        */
        EIdStorageDescriptionChunk,
        
        /**
        The EVolumeIdentifier element data chunk ID.
        */
        EIdVolumeIdentifierChunk,
        
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
    The MTP string type data chunks (EStorageDescription, and 
    EVolumeIdentifier).
    */
    RPointerArray<CMTPTypeString>                   iStringChunks;
    };
    
#endif // CMTPTYPESTORAGEINFO_H

