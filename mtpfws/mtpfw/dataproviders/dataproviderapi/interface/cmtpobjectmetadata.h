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

#ifndef CMTPOBJECTMETADATA_H
#define CMTPOBJECTMETADATA_H

#include <mtp/cmtpmetadata.h>

/** 
Defines the MTP object meta-data.
@publishedPartner
@released
*/
class CMTPObjectMetaData : public CMTPMetaData
{
public:

    /**
    MTP object meta-data element identifiers.
    */      
    enum TElementId
        {        
        /**
        The ID of the data provider responsible for the data object. This 
        element is of type @see TUint.
        */
        EDataProviderId     = 0,
        
        /**
        The object format code. This element is of type @see TUint.
        */
        EFormatCode         = 1,
        
        /**
        The object format sub-code. Currently this field is ignored unless the 
        @see EFormatCode contains a value of @see EMTPFormatCodeAssociation, in 
        which case this field should the specify the MTP association type code.
        This element is of type @see TUint.
        */
        EFormatSubCode      = 2,
        
        /**
        The object handle. This element is of type @see TUint.
        */
        EHandle             = 3,
        
        /**
        The set of MTP operational modes in which the object is visible and 
        will be enumerated to a connected MTP Initiator. The MTP operational
        modes are represented as a set of @see TMTPOperationalMode bit flags.
        This element is of type @see TUint 
        */
        EModes              = 4,
        
        /**
        The parent object handle. This element is of type @see TUint.
        */
        EParentHandle       = 5,
        
        /**
        The MTP storage ID of the storage on which the data object resides. 
        This element is of type @see TUint.
        */
        EStorageId          = 6,
        
        /**
        The MTP storage ID of the storage on which the data object resides. 
        This element is of type @see TUint.
        */
        EStorageVolumeId    = 7,
        
        /**
        The data object's System Unique IDentifier (SUID). This element is of 
        type @see TDesC&.
        */
        ESuid               = 8,

		/**
		The data object's user rating
		This element is of type @see TUint.
		@deprecated
		*/
		EUserRating = 9,

		/** 
		The data object's BuyNow flag
		This element is of type @see TUint.
		@deprecated
		*/
		EBuyNow = 10,

		/** The data object's Played flag
		This element is of type @see TUint.
		@deprecated
		*/
		EPlayCount = 11,
		
		/**
		The data Objects Identifier as in HandleStore table
		This element is of type @see TUint.
		*/
		EIdentifier = 12,
		
		/**
		The id related to ParentHandle
		This element is of type @see TUint.
		*/
		//EParentId = 13,
		
		/**
		The flag, used in object modification, indicates if the modified object
		should be reported to initiator
		This element is of type @see TUint.
		*/
		ENonConsumable = 13,
		
		EObjectMetaDataUpdate = 14,	

		EName = 15,
        /**
        The number of elements.        
        */
        ENumElements,
        };

public:
    
    IMPORT_C static CMTPObjectMetaData* NewL();
    IMPORT_C static CMTPObjectMetaData* NewLC();
    IMPORT_C static CMTPObjectMetaData* NewL(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid);
    IMPORT_C static CMTPObjectMetaData* NewLC(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid);
    IMPORT_C ~CMTPObjectMetaData();
    
private:
    
    CMTPObjectMetaData();
    void ConstructL();
    void ConstructL(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid);
    /* @deprecated */
    void ConstructL( const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount);

public:
	/* @deprecated */
    IMPORT_C static CMTPObjectMetaData* NewL(const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount );
    /* @deprecated */
    IMPORT_C static CMTPObjectMetaData* NewLC(const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount );
    
private:  // Owned
    
    /**
    The element meta-data.
    */
    static const TElementMetaData   KElements[];
};

#endif // CMTPOBJECTMETADATA_H
