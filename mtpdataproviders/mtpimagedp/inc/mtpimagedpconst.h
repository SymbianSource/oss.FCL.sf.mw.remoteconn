// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#ifndef MTPIMAGEDPCONST_H
#define MTPIMAGEDPCONST_H

#include <e32std.h>

#include <mtp/mtpprotocolconstants.h>

/*
 * [Thumbnail SIZE]: performance improvement
 */
const TUint32 KThumbWidht=104;//avoid to scale width to 160
const TUint32 KThumbHeigth=74;//avoid to scale width to 120
const TUint32 KThumbCompressedSize=KThumbWidht * KThumbHeigth * 4; // from TNM
const TUint32 KThumbFormatCode = 0x3801;
const TUint32 KFileSizeMax = 10 * 1000 * 1000;
const TUint32 KImageDpNotifyDelay = 1000000 * 15;// set delay time, in microseconds, default is 15s.

_LIT8(KPtpMimeJPEG, "image/jpeg");

/**
Define all the operations that are supported by the picture data provider
*/
static const TUint16 KMTPImageDpSupportedOperations[] = 
  	{
  	EMTPOpCodeGetObjectInfo,
  	EMTPOpCodeGetObject,
  	EMTPOpCodeGetPartialObject,	
    EMTPOpCodeGetThumb,	
  	EMTPOpCodeDeleteObject,
  	EMTPOpCodeSendObjectInfo,
  	EMTPOpCodeSendObject,
  	EMTPOpCodeMoveObject,
  	EMTPOpCodeCopyObject,
  	EMTPOpCodeGetObjectReferences,
  		
  	//MTP op codes
  	EMTPOpCodeGetObjectPropsSupported,
  	EMTPOpCodeGetObjectPropDesc,
  	EMTPOpCodeGetObjectPropValue,
  	EMTPOpCodeSetObjectPropValue,
  	EMTPOpCodeSetObjectReferences,

  	//MTP enhanced operation codes
  	EMTPOpCodeGetObjectPropList,
  	EMTPOpCodeSetObjectPropList,
  	EMTPOpCodeSendObjectPropList,

    EMTPOpCodeGetFormatCapabilities,
    
    //Deprecated operation
    EMTPOpCodeSetObjectProtection
  	};	

/**
Define all the object properties that are supported by the picture data provider
*/
static const TUint16 KMTPImageDpSupportedProperties[] =
  	{
    //Stored in MTP MetaData Object
  	EMTPObjectPropCodeStorageID,
  	EMTPObjectPropCodeObjectFormat,
    EMTPObjectPropCodeProtectionStatus,	
  	EMTPObjectPropCodeObjectFileName,
  	EMTPObjectPropCodeParentObject,
  	EMTPObjectPropCodePersistentUniqueObjectIdentifier,

    //Stored in MDE object
    EMTPObjectPropCodeName,
    EMTPObjectPropCodeObjectSize,
    EMTPObjectPropCodeDateCreated, 
    EMTPObjectPropCodeDateModified,
  	EMTPObjectPropCodeWidth,
  	EMTPObjectPropCodeHeight,
  	EMTPObjectPropCodeImageBitDepth,
  	EMTPObjectPropCodeRepresentativeSampleFormat,
  	EMTPObjectPropCodeRepresentativeSampleSize,
  	EMTPObjectPropCodeRepresentativeSampleHeight,
  	EMTPObjectPropCodeRepresentativeSampleWidth,
  	EMTPObjectPropCodeRepresentativeSampleData,
  	EMTPObjectPropCodeNonConsumable,
  	EMTPObjectPropCodeHidden
  	};

/**
Define the supported property group number .
*/
static const TUint16 KMTPImageDpPropertyGroupOneNumber = 1;

/**
Define the group one supported properties.
The properties in group one are same as all properties which image dp supported,
It can avoid PC get all properties and group one properties twice while sendobjectproplist/sendobject
*/
static const TUint16 KMTPImageDpGroupOneProperties[] =
    {
    //Stored in MTP MetaData Object
    EMTPObjectPropCodeStorageID,
    EMTPObjectPropCodeObjectFormat,
    EMTPObjectPropCodeProtectionStatus, 
    EMTPObjectPropCodeObjectFileName,
    EMTPObjectPropCodeParentObject,
    EMTPObjectPropCodePersistentUniqueObjectIdentifier,

    //Stored in MDE object
    EMTPObjectPropCodeName,
    EMTPObjectPropCodeNonConsumable,
    EMTPObjectPropCodeObjectSize,
    EMTPObjectPropCodeDateCreated, 
    EMTPObjectPropCodeDateModified,
    EMTPObjectPropCodeWidth,
    EMTPObjectPropCodeHeight,
    EMTPObjectPropCodeImageBitDepth,
    EMTPObjectPropCodeRepresentativeSampleFormat,
    EMTPObjectPropCodeRepresentativeSampleSize,
    EMTPObjectPropCodeRepresentativeSampleHeight,
    EMTPObjectPropCodeRepresentativeSampleWidth,
    EMTPObjectPropCodeRepresentativeSampleData,
    
    //From file system
    EMTPObjectPropCodeHidden
    };

/**
Define the group one supported property number.
*/
static const TUint16 KMTPImageDpGroupOneSize = sizeof(KMTPImageDpGroupOneProperties)/sizeof(KMTPImageDpGroupOneProperties[0]); 

/**
Define all events that are supported by the picture data provider
*/
static const TUint16 KMTPImageDpSupportedEvents[] =
    {
    EMTPEventCodeObjectAdded,
    EMTPEventCodeObjectRemoved,
    EMTPEventCodeObjectInfoChanged
    };

static const TUint32 KPtpNoValue = 0xFFFFFF00;

static const TInt KExtensionBufferSize=5; //related to the size of strings in SMTPValidCodeExtensionMapping
typedef TBufC<KExtensionBufferSize> TMTPImageFileExtension;

// extensions of image files that are shared via MTP Picture Data Provider
struct SMTPValidCodeExtensionMapping
    {
    public:
    TMTPFormatCode iFormatCode;
    const TText*   iExtension;
    };

struct SMTPExtensionMimeTypeMapping
    {
    public:    
    const TText*   iExtension;
    const TText*   iMimeType;
    };

// Note when adding extensions: 
// same value for iExtension can be in the file only once, 
// values for iFormatCode code can be multiple times

#if defined(_UNICODE)
#define CASTING(a) ((const TText*)L ## a)
#else
#define CASTING(a) ((const TText*)(a))
#endif

static const SMTPValidCodeExtensionMapping KMTPValidCodeExtensionMappings[] =
    {
    {EMTPFormatCodeEXIFJPEG, CASTING("jpg")},  // jpeg
    {EMTPFormatCodeEXIFJPEG, CASTING("jpe")},  // jpeg
    {EMTPFormatCodeEXIFJPEG, CASTING("jpeg")}, // jpeg
    {EMTPFormatCodeBMP,      CASTING("bmp")},  // bmp
    {EMTPFormatCodeGIF,      CASTING("gif")},  // gif
    {EMTPFormatCodePNG,      CASTING("png")},  // png
//    {EMTPFormatCodeTIFF,     CASTING("tif")},  // tiff, TNM does not support
//    {EMTPFormatCodeTIFF,     CASTING("tiff")}, // tiff, TNM does not support
    };

static const SMTPExtensionMimeTypeMapping KMTPExtensionMimeTypeMappings[] =
    {
    {CASTING("jpg"),         CASTING("image/jpeg")}, // jpeg
    {CASTING("jpe"),         CASTING("image/jpeg")}, // jpeg
    {CASTING("jpeg"),        CASTING("image/jpeg")}, // jpeg
    {CASTING("bmp"),         CASTING("image/bmp")},  // bmp
    {CASTING("gif"),         CASTING("image/gif")},  // gif
    {CASTING("png"),         CASTING("image/png")},  // png
//    {CASTING("tif"),         CASTING("image/tiff")}, // tiff, TNM does not support
//    {CASTING("tiff"),        CASTING("image/tiff")}, // tiff, TNM does not support
    };

static const TMTPFormatCode KMTPImageDpSupportFormatCodes[] =
    {
    EMTPFormatCodeEXIFJPEG,
    EMTPFormatCodeBMP,
    EMTPFormatCodeGIF,
    EMTPFormatCodePNG,
//    EMTPFormatCodeTIFF
    };

/**
Defines the image dp parameters.
*/
enum TImageDpParameter
    {
    /**
    This parameter defines the new images property value. 
    
    This parameter value can be retrieved as an @see TInt value.
    */        
    ENewImagesCount                 = 0x00000000,
    };

#endif // MTPIMAGEDPCONST_H

