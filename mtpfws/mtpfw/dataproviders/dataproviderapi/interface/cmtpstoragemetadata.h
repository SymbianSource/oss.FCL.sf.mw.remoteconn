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

#ifndef CMTPSTORAGEMETADATA_H
#define CMTPSTORAGEMETADATA_H

#include <mtp/cmtpmetadata.h>

/** 
Defines the MTP storage meta-data.
@publishedPartner
@released
*/
class CMTPStorageMetaData : public CMTPMetaData
    {
public:

    /**
    Defines the MTP storage meta-data element identifiers.
    */      
    enum TElementId
        {         
        /**
        The storage attribute flags (@see TStorageAttribute). This element is of type 
        @see TUint.
        */
        EAttributes         = 0,
             
        /**
        The list of areas on the storage which contain data objects which must
        not be exposed by the MTP protocol and which must be excluded during
        data provider object enumeration. If @see EStorageSystemType is 
        specified as @see ESystemTypeGenericHierachal then entries in this list
        are interpreted and validated as Symbian OS folder pathnames (e.g. 
        "C:\Private", or "Z:\System"). This element is of type @see CDesCArray.
        */
        EExcludedAreas      = 1,
             
        /**
        The MTP StorageID. This element is of type @see TUint.
        */
        EStorageId          = 2,
             
        /**
        The MTP set of logical MTP StorageIDs associated with this storage. 
        This set will always be empty if @see EStorageID is a logical MTP 
        StorageID. This element is of type @see RArray<TUint>.
        */
        EStorageLogicalIds  = 3,
        
        /**
        The storage System Unique IDentifier (SUID). If @see EStorageSystemType 
        is specified as @see ESystemTypeGenericHierachal then this value is 
        interpreted and validated as a Symbian OS folder pathname (e.g. 
        C:\Media", or "E:\") representing the root point of the MTP storage. 
        This element is of type @see TDesC.
        */
        EStorageSuid        = 4,
          
        /**
        The storage system (@see TStorageSystemType). This element is of type 
        @see TUint.
        */
        EStorageSystemType  = 5,
        
        /**
        The number of elements.        
        */
        ENumElements,
        };

    /**
    Defines the MTP storage attribute bit flags. These are encoded as 
    @see TUint. Flags in the least significant 16-bit range are reserved,
    those in the most significant 16-bit range are available for use as 
    extension values by storage owners.
    */
    enum TStorageAttribute
        {        
        /**
        The null attribute.
        */
        EAttrNone           = 0x00000000,
        
        /**
        The extension attribute flags mask.
        */
        EAttrMaskExtensions = 0xFFFF0000,
        
        /**
        The reserved attribute flags mask.
        */
        EAttrMaskReserved   = 0x0000FFFF,
        };
        
    /**
    Defines the storage system type identifier code space. This 16-bit code 
    space is partitioned and encoded identically to the MTP "Filesystem Type", 
    as described in the "Media Transfer Protocol Enhanced" specification. 
    */
    enum TStorageSystemType
        {      
        /**
        The undefined storage system type identifier code.
        */
        ESystemTypeUndefined            = 0x00000000,
        
        /**
        The generic flat storage system type identifier code.
        */
        ESystemTypeGenericFlat          = 0x00000001,
        
        /**
        The generic hierachal storage system type identifier code.
        */
        
        ESystemTypeGenericHierachal     = 0x00000002,
        
        /**
        The DRM Content Format (DCF) storage system type identifier code.
        */
        ESystemTypeDCF                  = 0x00000003,
        
        /**
        The start of the reserved storage system type identifier code space.
        */
        ESystemTypeMaskReservedStart    = 0x00000004,
        
        /**
        The end of the reserved storage system type identifier code space.
        */
        ESystemTypeMaskReservedEnd      = 0x00007FFF,
        
        /**
        The MTP vendor extension storage system type identifier code flags.
        */
        ESystemTypeMTPVendorExtension   = 0x00008000,
        
        /**
        The MTP defined storage system type identifier code flags.
        */
        ESystemTypeMTPDefined           = 0x0000C000,
        
        /**
        The 16-bit storage system type identifier code mask.
        */
        ESystemTypeMask                 = 0x0000FFFF,
        
        /**
        The default hierachal file system (@see RFs) based storage type 
        identifier. If this storage type is specified then:
        
            1.  The storage is assumed to reside on the file system.
            2.  The @see EStorageSuid is interpreted and validated as a Symbian 
                OS folder pathname (e.g. C:\Media", or "E:\") representing the 
                root point of the MTP storage.
            3.  Entries in the @see EExcludedAreas are interpreted and 
                validated as Symbian OS folder pathnames (e.g. "C:\Private", or
                "Z:\System").
        */
        ESystemTypeDefaultFileSystem    = ESystemTypeGenericHierachal,
        };
        
public:
    
    IMPORT_C static CMTPStorageMetaData* NewL();
    IMPORT_C static CMTPStorageMetaData* NewLC();
    IMPORT_C static CMTPStorageMetaData* NewL(TUint aStorageSystemType, const TDesC& aStorageSuid);
    IMPORT_C static CMTPStorageMetaData* NewLC(TUint aStorageSystemType, const TDesC& aStorageSuid);
    IMPORT_C static CMTPStorageMetaData* NewL(const CMTPStorageMetaData& aStorage);
    IMPORT_C static CMTPStorageMetaData* NewLC(const CMTPStorageMetaData& aStorage);
    IMPORT_C ~CMTPStorageMetaData();
    
private:
    
    CMTPStorageMetaData();
    void ConstructL(TUint aStorageSystemType, const TDesC& aStorageSuid);

private:  // Owned
    
    /**
    The element meta-data.
    */
    static const TElementMetaData   KElements[];
    };
    
#endif // CMTPSTORAGEMETADATA_H
