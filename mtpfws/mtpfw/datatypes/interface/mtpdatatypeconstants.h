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

#ifndef MTPDATATYPECONSTANTS_H
#define MTPDATATYPECONSTANTS_H

#include <e32def.h>

/**
The MTP datatype codes.
@publishedPartner
@released  
*/
enum TMTPTypeIds
    {
    EMTPFirstLibraryDataType                        = 0x00000000,
    
    EMTPTypeUndefined                               = EMTPFirstLibraryDataType,
    
    EMTPTypeFirstSimpleType                         = 0x00000001,
    EMTPTypeINT8                                    = EMTPTypeFirstSimpleType,
    EMTPTypeUINT8                                   = 0x00000002,
    EMTPTypeINT16                                   = 0x00000003,
    EMTPTypeUINT16                                  = 0x00000004,
    EMTPTypeINT32                                   = 0x00000005,
    EMTPTypeUINT32                                  = 0x00000006,
    EMTPTypeINT64                                   = 0x00000007,
    EMTPTypeUINT64                                  = 0x00000008,
    EMTPTypeINT128                                  = 0x00000009,
    EMTPTypeUINT128                                 = 0x0000000A,
    EMTPTypeLastSimpleType                          = EMTPTypeUINT128,
    
    EMTPTypeFirstSimpleArrayType                    = 0x00004001,
    EMTPTypeAINT8                                   = EMTPTypeFirstSimpleArrayType,
    EMTPTypeAUINT8                                  = 0x00004002,
    EMTPTypeAINT16                                  = 0x00004003,
    EMTPTypeAUINT16                                 = 0x00004004,
    EMTPTypeAINT32                                  = 0x00004005,
    EMTPTypeAUINT32                                 = 0x00004006,
    EMTPTypeAINT64                                  = 0x00004007,
    EMTPTypeAUINT64                                 = 0x00004008,
    EMTPTypeAINT128                                 = 0x00004009,
    EMTPTypeAUINT128                                = 0x0000400A,
    EMTPTypeLastSimpleArrayType                     = EMTPTypeAUINT128,
    
    EMTPTypeFirstComplexType                        = 0x0000FFFF,
    EMTPTypeString                                  = EMTPTypeFirstComplexType,
    EMTPTypeArray                                   = 0x00010001,
    EMTPTypeFile                                    = 0x00010002,
    EMTPTypeOpaqueData								= 0x00010003,
    EMTPTypeTrivialData                             = 0x00010004,
    EMTPTypeLastComplexType                         = EMTPTypeTrivialData,
    
    EMTPTypeFirstCompoundType                       = 0x00020000,
    EMTPTypeCompound                                = EMTPTypeFirstCompoundType,
    EMTPTypeDeviceInfoDataset                       = 0x00020100,
    EMTPTypeDevicePropDescDataset                   = 0x00020200,
    EMTPTypeDevicePropDescEnumerationFormDataset    = 0x00020201,
    EMTPTypeDevicePropDescRangeFormDataset          = 0x00020202,
    EMTPTypeInterdependentPropDescDataset           = 0x00020300,
    EMTPTypeInterdependentPropDescElementDataset    = 0x00020301,
    EMTPTypeObjectInfoDataset                       = 0x00020400,
    EMTPTypeObjectPropDescDataset                   = 0x00020500,
    EMTPTypeObjectPropDescEnumerationFormDataset    = 0x00020501,
    EMTPTypeObjectPropDescRangeFormDataset          = 0x00020502,
	EMTPTypeServiceObjPropExtnFormDataset			= 0x00020504,
	EMTPTypeServiceMethodParamExtnFormDataset		= 0x00020505,
    EMTPTypeObjectPropListDataset                   = 0x00020600,
    EMTPTypeServicePropListDataset                  = EMTPTypeObjectPropListDataset,
    EMTPTypeObjectPropListElementDataset            = 0x00020601,
    EMTPTypeServicePropListElementDataset           = EMTPTypeObjectPropListElementDataset,
    EMTPTypeStorageInfoDataset                      = 0x00020700,
    EMTPTypeServiceInfoDataset						= 0x00020800,
	EMTPTypeServiceFormatElementDataset				= 0x00020801,
	EMTPTypeServiceFormatListDataset				= 0x00020802,
	EMTPTypeServicePropertyElementDataset			= 0x00020803,
	EMTPTypeServicePropertyListDataset					= 0x00020804,
	EMTPTypeServiceMethodElementDataset				= 0x00020805,
	EMTPTypeServiceMethodListDataset				= 0x00020806,
	EMTPTypeServiceEventElementDataset				= 0x00020807,
	EMTPTypeServiceEventListDataset					= 0x00020808,
	EMTPTypeServiceSectionDataset					= 0x20020809,
	EMTPTypeServiceCapabilityListDataset			= 0x00020900,
	EMTPTypeFormatCapabilityListDataset             = EMTPTypeServiceCapabilityListDataset,
	EMTPTypeFormatCapabilityDataset					= 0x00020901,
	EMTPTypeServicePropDescList						= 0x00020902,
	EMTPTypeServicePropDesc							= 0x00020903,
	EMTPTypeDeleteObjectPropListDataset             = 0x00020A00,
	EMTPTypeDeleteServicePropListDataset            = 0x00020B00,
	EMTPTypeLastCompoundType						= EMTPTypeDeleteServicePropListDataset,
    
    EMTPTypeFirstFlatType                           = 0x00040000,
    EMTPTypeFlat                                    = EMTPTypeFirstFlatType,
    EMTPTypeEventDataset                            = 0x00040001,
    EMTPTypeRequestDataset                          = 0x00040002,
    EMTPTypeResponseDataset                         = 0x00040003,
    EMTPTypeDataPair                                = 0x00040004,
    EMTPTypeLastFlatType                            = EMTPTypeDataPair,
    
    EMTPTypeReference                               = 0x000FFFFF,
    EMTPLastLibraryDataType                         = EMTPTypeReference,

	/**
	The Range for USB   transport is 0x00010000 to 0x000100FF
	The Range for PTPIP transport is 0x00010100 to 0x000101FF
	*/    
    EMTPFirstTransportDataType                      = 0x00010000,
    EMTPLastTransportDataType                       = 0x0001FFFF,
    
    EMTPFirstVendorExtensionDataType                = 0x80000000,
    EMTPLastVendorExtensionDataType                 = 0xFFFFFFFF,
    };

/**
The MTP INT8 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeINT8Size(1);

/**
The MTP UINT8 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeUINT8Size(1);

/**
The MTP INT16 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeINT16Size(2);

/**
The MTP UINT16 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeUINT16Size(2);

/**
The MTP INT32 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeINT32Size(4);

/**
The MTP UINT32 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeUINT32Size(4);

/**
The MTP INT64 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeINT64Size(8);

/**
The MTP UINT64 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeUINT64Size(8);

/**
The MTP INT128 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeINT128Size(16);

/**
The MTP UINT128 data type size (in bytes).
@publishedPartner
@released  
*/
const TUint KMTPTypeUINT128Size(16);

/**
The MTP String data type character size (in bytes).
@publishedPartner
@released  
*/
const TUint     KMTPCharSize(sizeof(TUint16));

/**
The MTP String data type null character.
@publishedPartner
@released  
*/
const TUint16   KMTPNullChar(0);

/**
The MTP String data type null character length.
@publishedPartner
@released  
*/
const TUint     KMTPNullCharLen(sizeof(KMTPNullChar) / KMTPCharSize);

/**
The MTP String data type absolute maximum length.
@publishedPartner
@released  
*/
const TUint     KMTPMaxStringLength(255);

/**
The MTP String data type maximum character content length.
@publishedPartner
@released  
*/
const TUint     KMTPMaxStringCharactersLength(KMTPMaxStringLength - KMTPNullCharLen);

#endif // MTPDATATYPECONSTANTS_H


