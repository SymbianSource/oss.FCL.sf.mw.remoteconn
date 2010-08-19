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

#ifndef MTPPROTOCOLCONSTANTS_H
#define MTPPROTOCOLCONSTANTS_H

#include <e32def.h>

/** 
Defines constant values specified by the MTP protocol.
@publishedPartner
@released 
*/

/**
The null MTP 32-bit value.
@publishedPartner
@released 
*/
const TUint32 KMTPNotSpecified32(0x00000000);

/**
The MTP ObjectFormatCode that indicates that an operation or event applies to 
all valid MTP ObjectFormatCodes.
@publishedPartner
@released 
*/
const TUint32 KMTPFormatsAll(0x00000000);

/**
The MTP parameter value that indicates that free space counted in number of 
objects on a storage is not supported.
@publishedPartner
@released 
*/
const TUint32 KMTPFreeSpaceUnsupported(0xFFFFFFFF);

/**
The MTP ObjectHandle that indicates that an operation or event applies to all 
valid ObjectHandles.
@publishedPartner
@released 
*/
const TUint32 KMTPHandleAll(0xFFFFFFFF);

/**
The MTP ObjectHandle that indicates that an operation or event applies to all 
valid root level ObjectHandles.
@publishedPartner
@released 
*/
const TUint32 KMTPHandleAllRootLevel(0x00000000);

/**
The null MTP ObjectHandle value.
@publishedPartner
@released 
*/
const TUint32 KMTPHandleNone(0x00000000);

/**
The MTP ObjectHandle that indicates that an data object does have an assigned 
parent data object.
@publishedPartner
@released 
*/
const TUint32 KMTPHandleNoParent(0xFFFFFFFF);

/**
The MTP ObjectPropCode value that indicates an operation or event applies to all 
all valid MTP ObjectPropCodeS.
@publishedPartner
@released 
*/
const TUint32 KMTPObjectPropCodeAll(0xFFFFFFFF);

/**
The MTP SessionID value that indicates an operation or event applies to all 
active MTP sessions.
@publishedPartner
@released 
*/
const TUint32 KMTPSessionAll(0xFFFFFFFF);

/**
The MTP SessionID value that indicates an operation or event occurs outside of
an active MTP sessions
@publishedPartner
@released 
*/
const TUint32 KMTPSessionNone(0x00000000);

/**
The MTP StorageID that indicates that an operation or event applies to all 
valid MTP StorageIDs.
@publishedPartner
@released 
*/
const TUint32 KMTPStorageAll(0xFFFFFFFF);

/**
The MTP StorageID that indicates that an operation or event applies to the 
default MTP StorageID.
@publishedPartner
@released 
*/
const TUint32 KMTPStorageDefault(0x00000000);

/**
The null MTP TransactionID value.
@publishedPartner
@released 
*/
const TUint32 KMTPTransactionIdNone(0x00000000);

/**
The first in the sequence of valid MTP TransactionIDs. TransactionIDs increment
sequentially from this value to KMTPTransactionIdLast and then wrap around to 
this value.
@publishedPartner
@released 
*/
const TUint32 KMTPTransactionIdFirst(0x00000001);

/**
The last in the sequence of valid MTP TransactionIDs. TransactionIDs increment 
sequentially from KMTPTransactionIdFirst to this value and then wrap around to 
KMTPTransactionIdFirst.
@publishedPartner
@released 
*/
const TUint32 KMTPTransactionIdLast(0xFFFFFFFF);


/**
The MTP DataProvider that indicates that an Data base searching operation is applied to objects
owned by any data providers.
@publishedPartner
@released 
*/
const TUint32 KMTPDataProviderAll(0xFFFFFFFF);


const TUint32 KMTPNewPicKey(0x2001FCAE);
const TUint32 KMTPServerUID(0x102827A2);


/**
The MTP functional modes. For a description of these modes, please refer to the 
"Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPFunctionalMode
    {
    EMTPFunctionalModeStandard              = 0x0000,
    EMTPFunctionalModeSleepState            = 0x0001,
    EMTPFunctionalModeReservedStart         = 0x0002,
    EMTPFunctionalModeReservedEnd           = 0x7FFF,
    EMTPFunctionalModeNonResponsivePlayback = 0xC001,
    EMTPFunctionalModeResponsivePlayback    = 0xC002,
    EMTPFunctionalModeVendorExtStart        = 0x8000,
    EMTPFunctionalModeVendorExtEnd          = 0xBFFF,
    EMTPFunctionalModeStandardStart         = 0xC000,
    EMTPFunctionalModeStandardEnd           = 0xFFFF
    };

/** 
The MTP Datacode Summary. For a description of these datacodes, please refer
to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/

enum TMTPDatacodeSummary
    {
    EMTPCodeUndefined1Start                 = 0x0000,
    EMTPCodeUndefined1End                   = 0x0FFF,
    
    EMTPCodePTPOperationCodeStart           = 0x1000,
    EMTPCodePTPOperationCodeEnd             = 0x1FFF,
    
    EMTPCodePTPResponseCodeStart            = 0x2000,
    EMTPCodePTPResponseCodeEnd              = 0x2FFF,
    
    EMTPCodePTPObjFormatCodeStart           = 0x3000,
    EMTPCodePTPObjFormatCodeEnd             = 0x3FFF,
    
    EMTPCodePTPEventCodeStart               = 0x4000,
    EMTPCodePTPEventCodeEnd                 = 0x4FFF,
    
    EMTPCodePTPDevicePropCodeStart          = 0x5000,
    EMTPCodePTPDevicePropCodeEnd            = 0x5FFF,
    
    EMTPCodePTPReserved1CodeStart           = 0x6000,
    EMTPCodePTPReserved1CodeEnd             = 0x6FFF,
    
    EMTPCodePTPReserved2CodeStart           = 0x7000,
    EMTPCodePTPReserved2CodeEnd             = 0x7FFF,
    
    EMTPCodeUndefined2CodeStart             = 0x8000,
    EMTPCodeUndefined2CodeEnd               = 0x8FFF,
    
    EMTPCodeVenderExtnOpCodeStart           = 0x9000,
    EMTPCodeVenderExtnOpCodeEnd             = 0x97FF,
    
    EMTPCodeMTPOperationCodeStart           = 0x9800,
    EMTPCodeMTPOperationCodeEnd             = 0x9FFF,
    
    EMTPCodeVenderExtnRespCodeStart         = 0xA000,
    EMTPCodeVenderExtnRespCodeEnd           = 0xA7FF,
    
    EMTPCodeMTPRespCodeStart                = 0xA800,
    EMTPCodeMTPRespCodeEnd                  = 0xAFFF,
    
    EMTPCodeVenderExtnObjFormatCodeStart    = 0xB000,
    EMTPCodeVenderExtnObjFormatCodeEnd      = 0xB7FF,
    
    
    EMTPCodeMTPObjFormatCodeStart           = 0xB800,
    EMTPCodeMTPObjFormatCodeEnd             = 0xBFFF,
    
    EMTPCodeVenderExtnEventCodeStart        = 0xC000,
    EMTPCodeVenderExtnEventCodeEnd          = 0xC7FF,
    
    EMTPCodeMTPEventCodeStart               = 0xC800,
    EMTPCodeMTPEventCodeEnd                 = 0xCFFF,
    
    EMTPCodeVenderExtnDevicePropCodeStart   = 0xD000,
    EMTPCodeVenderExtnDevicePropCodeEnd     = 0xD3FF,
    

    
    EMTPCodeMTPDevicePropCodeStart          = 0xD400,
    EMTPCodeMTPDevicePropCodeEnd            = 0xD7FF,
    
    EMTPCodeVenderExtnObjectPropCodeStart   = 0xD800,
    EMTPCodeVenderExtnObjectPropCodeEnd     = 0xDBFF,
    
    EMTPCodeMTPObjectPropCodeStart          = 0xDC00,
    EMTPCodeMTPObjectPropCodeEnd            = 0xDFFF,
    
    EMTPCodePTPReserved3CodeStart           = 0xE000,
    EMTPCodePTPReserved3CodeEnd             = 0xEFFF,
    
    EMTPCodePTPReserved4CodeStart           = 0xF000,
    EMTPCodePTPReserved4CodeEnd             = 0xFFFF,
    };


/**
The MTP operation datacodes. For a description of these datacodes, please refer
to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPOperationCode
    {
    EMTPOpCodePTPStart                  = 0x1000,
    EMTPOpCodeGetDeviceInfo             = 0x1001,
    EMTPOpCodeOpenSession               = 0x1002,
    EMTPOpCodeCloseSession              = 0x1003,
    EMTPOpCodeGetStorageIDs             = 0x1004,
    EMTPOpCodeGetStorageInfo            = 0x1005,
    EMTPOpCodeGetNumObjects             = 0x1006,
    EMTPOpCodeGetObjectHandles          = 0x1007,
    EMTPOpCodeGetObjectInfo             = 0x1008,
    EMTPOpCodeGetObject                 = 0x1009,
    EMTPOpCodeGetThumb                  = 0x100A,
    EMTPOpCodeDeleteObject              = 0x100B,
    EMTPOpCodeSendObjectInfo            = 0x100C,
    EMTPOpCodeSendObject                = 0x100D,
    EMTPOpCodeInitiateCapture           = 0x100E,
    EMTPOpCodeFormatStore               = 0x100F,
    EMTPOpCodeResetDevice               = 0x1010,
    EMTPOpCodeSelfTest                  = 0x1011,
    EMTPOpCodeSetObjectProtection       = 0x1012,
    EMTPOpCodePowerDown                 = 0x1013,
    EMTPOpCodeGetDevicePropDesc         = 0x1014,
    EMTPOpCodeGetDevicePropValue        = 0x1015,
    EMTPOpCodeSetDevicePropValue        = 0x1016,
    EMTPOpCodeResetDevicePropValue      = 0x1017,
    EMTPOpCodeTerminateOpenCapture      = 0x1018,
    EMTPOpCodeMoveObject                = 0x1019,
    EMTPOpCodeCopyObject                = 0x101A,
    EMTPOpCodeGetPartialObject          = 0x101B,
    EMTPOpCodeInitiateOpenCapture       = 0x101C,
    EMTPOpCodePTPEnd                    = 0x1FFF,
    EMTPOpCodeVendorExtensionStart      = 0x9000,
    EMTPOpCodeGetServiceIDs             = 0x9301,
    EMTPOpCodeGetServiceInfo            = 0x9302,
    EMTPOpCodeGetServiceCapabilities    = 0x9303,
    EMTPOpCodeGetServicePropDesc        = 0x9304,
    EMTPOpCodeGetServicePropList        = 0x9305,
    EMTPOpCodeSetServicePropList        = 0x9306,
    EMTPOpCodeUpdateObjectPropList      = 0x9307,
    EMTPOpCodeDeleteObjectPropList      = 0x9308,
    EMTPOpCodeDeleteServicePropList     = 0x9309,
    EMTPOpCodeGetFormatCapabilities     = 0x930A,
    EMTPOpCodeVendorExtextensionEnd     = 0x97FF,
    EMTPOpCodeMTPStart                  = 0x9800,
    EMTPOpCodeGetObjectPropsSupported   = 0x9801,
    EMTPOpCodeGetObjectPropDesc         = 0x9802,
    EMTPOpCodeGetObjectPropValue        = 0x9803,
    EMTPOpCodeSetObjectPropValue        = 0x9804,
    EMTPOpCodeGetObjectReferences       = 0x9810,
    EMTPOpCodeSetObjectReferences       = 0x9811,
    EMTPOpCodeSkip                      = 0x9820,
    EMTPOpCodeGetObjectPropList         = 0x9805,
    EMTPOpCodeSetObjectPropList         = 0x9806,
    EMTPOpCodeGetInterdependentPropDesc = 0x9807,
    EMTPOpCodeSendObjectPropList        = 0x9808,
    EMTPOpCodeGetSanAlertInfo           = 0x93E8,
    EMTPOpCodeMTPEnd                    = 0x9FFF    
    };

/**
The MTP response datacodes. For a description of these datacodes, please refer 
to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPResponseCode
    {   
    EMTPRespCodePTPStart                                = 0x2000,
    EMTPRespCodeUndefined                               = 0x2000,
    EMTPRespCodeOK                                      = 0x2001,
    EMTPRespCodeGeneralError                            = 0x2002,
    EMTPRespCodeSessionNotOpen                          = 0x2003,
    EMTPRespCodeInvalidTransactionID                    = 0x2004,
    EMTPRespCodeOperationNotSupported                   = 0x2005,
    EMTPRespCodeParameterNotSupported                   = 0x2006,
    EMTPRespCodeIncompleteTransfer                      = 0x2007,
    EMTPRespCodeInvalidStorageID                        = 0x2008,
    EMTPRespCodeInvalidObjectHandle                     = 0x2009,
    EMTPRespCodeDevicePropNotSupported                  = 0x200A,
    EMTPRespCodeInvalidObjectFormatCode                 = 0x200B,
    EMTPRespCodeStoreFull                               = 0x200C,
    EMTPRespCodeObjectWriteProtected                    = 0x200D,
    EMTPRespCodeStoreReadOnly                           = 0x200E,
    EMTPRespCodeAccessDenied                            = 0x200F,
    EMTPRespCodeNoThumbnailPresent                      = 0x2010,
    EMTPRespCodeSelfTestFailed                          = 0x2011,
    EMTPRespCodePartialDeletion                         = 0x2012,
    EMTPRespCodeStoreNotAvailable                       = 0x2013,
    EMTPRespCodeSpecificationByFormatUnsupported        = 0x2014,
    EMTPRespCodeNoValidObjectInfo                       = 0x2015,
    EMTPRespCodeInvalidCodeFormat                       = 0x2016,
    EMTPRespCodeUnknownVendorCode                       = 0x2017,
    EMTPRespCodeCaptureAlreadyTerminated                = 0x2018,
    EMTPRespCodeDeviceBusy                              = 0x2019,
    EMTPRespCodeInvalidParentObject                     = 0x201A,
    EMTPRespCodeInvalidDevicePropFormat                 = 0x201B,
    EMTPRespCodeInvalidDevicePropValue                  = 0x201C,
    EMTPRespCodeInvalidParameter                        = 0x201D,
    EMTPRespCodeSessionAlreadyOpen                      = 0x201E,
    EMTPRespCodeTransactionCancelled                    = 0x201F,
    EMTPRespCodeSpecificationOfDestinationUnsupported   = 0x2020,
    EMTPRespCodePTPEnd                                  = 0x2FFF,
    EMTPRespCodeVendorExtensionStart                    = 0xA000,
    EMTPRespCodeInvalidServiceID                        = 0xA301,
    EMTPRespCodeInvalidServicePropCode                  = 0xA302,
    EMTPRespCodeVendorExtensionEnd                      = 0xA7FF,
    EMTPRespCodeMTPStart                                = 0xA800,
    EMTPRespCodeInvalidObjectPropCode                   = 0xA801,
    EMTPRespCodeInvalidObjectPropFormat                 = 0xA802,
    EMTPRespCodeInvalidObjectPropValue                  = 0xA803,
    EMTPRespCodeInvalidObjectReference                  = 0xA804,
    EMTPRespCodeGroupNotSupported						= 0xA805,
    EMTPRespCodeInvalidDataset                          = 0xA806,
    EMTPRespCodeSpecificationByGroupUnsupported         = 0xA807,
    EMTPRespCodeSpecificationByDepthUnsupported         = 0xA808,
    EMTPRespCodeObjectTooLarge                          = 0xA809,
    EMTPRespCodeObjectPropNotSupported                  = 0xA80A,
    EMTPRespCodeSANStored                               = 0xA391,
    EMTPRespCodeUserRejectedSAN                         = 0xA392,
    EMTPRespCodeSANParseError                           = 0xA393,
    EMTPRespCodeProfileMissing                          = 0xA394,
    EMTPRespCodeMTPEnd                                  = 0xAFFF   
    };

/**
The MTP object format datacodes. For a description of these datacodes, please 
refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPFormatCode
    {
    EMTPFormatCodePTPStart                          = 0x3000,
    EMTPFormatCodeUndefined                         = 0x3000,
    EMTPFormatCodeAssociation                       = 0x3001,
    EMTPFormatCodeScript                            = 0x3002,
    EMTPFormatCodeExecutable                        = 0x3003,
    EMTPFormatCodeText                              = 0x3004,
    EMTPFormatCodeHTML                              = 0x3005,
    EMTPFormatCodeDPOF                              = 0x3006,
    EMTPFormatCodeAIFF                              = 0x3007,
    EMTPFormatCodeWAV                               = 0x3008,
    EMTPFormatCodeMP3                               = 0x3009,
    EMTPFormatCodeAVI                               = 0x300A,
    EMTPFormatCodeMPEG                              = 0x300B, 
    EMTPFormatCodeASF                               = 0x300C,
    EMTPFormatCodeImageDefined                      = 0x3800,
    EMTPFormatCodeEXIFJPEG                          = 0x3801,
    EMTPFormatCodeTIFFEP                            = 0x3802,
    EMTPFormatCodeFlashPix                          = 0x3803,
    EMTPFormatCodeBMP                               = 0x3804,
    EMTPFormatCodeCIFF                              = 0x3805,
    EMTPFormatCodeImageUndefined1                   = 0x3806,
    EMTPFormatCodeGIF                               = 0x3807,
    EMTPFormatCodeJFIF                              = 0x3808,
    EMTPFormatCodeCD                                = 0x3809,
    EMTPFormatCodePICT                              = 0x380A,
    EMTPFormatCodePNG                               = 0x380B,
    EMTPFormatCodeImageUndefined2                   = 0x380C,
    EMTPFormatCodeTIFF                              = 0x380D,
    EMTPFormatCodeTIFFIT                            = 0x380E,
    EMTPFormatCodeJP2                               = 0x380F,
    EMTPFormatCodeJPX                               = 0x3810,
    EMTPFormatCodePTPEnd                            = 0x3FFF,
    EMTPFormatCodeVendorExtStart                    = 0xB000,
    
    EMTPFormatCodeVendorExtDynamicStart            = EMTPFormatCodeVendorExtStart,

    
    EMTPFormatCodeVendorExtEnd                      = 0xB7FF,
    EMTPFormatCodeVendorExtDynamicEnd              = EMTPFormatCodeVendorExtEnd,
    
    EMTPFormatCodeMTPStart                          = 0xB800,
    EMTPFormatCodeUndefinedFirmware                 = 0xB802,
    EMTPFormatCodeWindowsImageFormat                = 0xB881,
    EMTPFormatCodeUndefinedAudio                    = 0xB900,
    EMTPFormatCodeWMA                               = 0xB901,
    EMTPFormatCodeOGG                               = 0xB902,
    EMTPFormatCodeAAC                               = 0xB903,
    EMTPFormatCodeAudible                           = 0xB904,
    EMTPFormatCodeFLAC                              = 0xB906,
    EMTPFormatCodeUndefinedVideo                    = 0xB980,
    EMTPFormatCodeWMV                               = 0xB981,
    EMTPFormatCodeMP4Container                      = 0xB982,
    EMTPFormatCodeMP2                               = 0xB983,
    EMTPFormatCode3GPContainer                      = 0xB984,
    EMTPFormatCodeAVCHD								= 0xB985,
    EMTPFormatCodeUndefinedCollection               = 0xBA00,
    EMTPFormatCodeAbstractMultimediaAlbum           = 0xBA01,
    EMTPFormatCodeAbstractImageAlbum                = 0xBA02,
    EMTPFormatCodeAbstractAudioAlbum                = 0xBA03,
    EMTPFormatCodeAbstractVideoAlbum                = 0xBA04,
    EMTPFormatCodeAbstractAudioVideoPlaylist        = 0xBA05,
    EMTPFormatCodeAbstractContactGroup              = 0xBA06,
    EMTPFormatCodeAbstractMessageFolder             = 0xBA07,
    EMTPFormatCodeAbstractChapteredProduction       = 0xBA08,
    EMTPFormatCodeAbstractAudioPlaylist             = 0xBA09,
    EMTPFormatCodeAbstractVideoPlaylist             = 0xBA0A,
    EMTPFormatCodeWPLPlaylist                       = 0xBA10,
    EMTPFormatCodeM3UPlaylist                       = 0xBA11,
    EMTPFormatCodeMPLPlaylist                       = 0xBA12,
    EMTPFormatCodeASXPlaylist                       = 0xBA13,
    EMTPFormatCodePLSPlaylist                       = 0xBA14,
    EMTPFormatCodeUndefinedDocument                 = 0xBA80,
    EMTPFormatCodeAbstractDocument                  = 0xBA81,
    EMTPFormatCodeXMLDocument                       = 0xBA82,
    EMTPFormatCodeMicrosoftWordDocument             = 0xBA83,
    EMTPFormatCodeMHTCompiledHTMLDocument           = 0xBA84,
    EMTPFormatCodeMicrosoftExcelSpreadsheet         = 0xBA85,
    EMTPFormatCodeMicrosoftPowerpointPresentation   = 0xBA86,
    EMTPFormatCodeUndefinedMessage                  = 0xBB00,
    EMTPFormatCodeAbstractMessage                   = 0xBB01,
    EMTPFormatCodeUndefinedContact                  = 0xBB80,
    EMTPFormatCodeAbstractContact                   = 0xBB81,
    EMTPFormatCodeVCard2                            = 0xBB82,
    EMTPFormatCodeMTPEnd                            = 0xBFFF 
    };

/**
The MTP event datacodes. For a description of these datacodes, please refer to 
the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPEventCode
    {
    EMTPEventCodePTPStart                   = 0x4000,
    EMTPEventCodeUndefined                  = 0x4000,
    EMTPEventCodeCancelTransaction          = 0x4001,
    EMTPEventCodeObjectAdded                = 0x4002,
    EMTPEventCodeObjectRemoved              = 0x4003,
    EMTPEventCodeStoreAdded                 = 0x4004,
    EMTPEventCodeStoreRemoved               = 0x4005,
    EMTPEventCodeDevicePropChanged          = 0x4006,
    EMTPEventCodeObjectInfoChanged          = 0x4007,
    EMTPEventCodeDeviceInfoChanged          = 0x4008,
    EMTPEventCodeRequestObjectTransfer      = 0x4009,
    EMTPEventCodeStoreFull                  = 0x400A,
    EMTPEventCodeDeviceReset                = 0x400B,
    EMTPEventCodeStorageInfoChanged         = 0x400C,
    EMTPEventCodeCaptureComplete            = 0x400D,
    EMTPEventCodeUnreportedStatus           = 0x400E,
    EMTPEventCodePTPEnd                     = 0x4FFF,
    EMTPEventCodeVendorExtensionStart       = 0xC000,
    EMTPEventCodeServiceAdded               = 0xC301,
    EMTPEventCodeServiceRemoved             = 0xC302,
    EMTPEventCodeServicePropChanged         = 0xC303,
    EMTPEventCodeMethodComplete             = 0xC304,
    EMTPEventCodeVendorExtensionEnd         = 0xC7FF,
    EMTPEventCodeMTPStart                   = 0xC800,
    EMTPEventCodeObjectPropChanged          = 0xC801,
    EMTPEventCodeObjectPropDescChanged      = 0xC802,
    EMTPEventCodeObjectReferencesChanged    = 0xC803,
    EMTPEventCodeRequestSyncObjectTransfer  = 0xC390,
    EMTPEventCodeRequestSanAlertInfo        = 0xC391,
    EMTPEventCodeMTPEnd                     = 0xCFFF,
    };

/**
The MTP device property datacodes. For a description of these datacodes, please 
refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPDevicePropertyCode
    {
    EMTPDevicePropCodePTPStart                      = 0x5000,
    EMTPDevicePropCodeUndefined                     = 0x5000,
    EMTPDevicePropCodeBatteryLevel                  = 0x5001,
    EMTPDevicePropCodeFunctionalMode                = 0x5002,
    EMTPDevicePropCodeImageSize                     = 0x5003,
    EMTPDevicePropCodeCompressionSetting            = 0x5004,
    EMTPDevicePropCodeWhiteBalance                  = 0x5005,
    EMTPDevicePropCodeRGBGain                       = 0x5006,
    EMTPDevicePropCodeFNumber                       = 0x5007,
    EMTPDevicePropCodeFocalLength                   = 0x5008,
    EMTPDevicePropCodeFocusDistance                 = 0x5009,
    EMTPDevicePropCodeFocusMode                     = 0x500A,
    EMTPDevicePropCodeExposureMeteringMode          = 0x500B,
    EMTPDevicePropCodeFlashMode                     = 0x500C,
    EMTPDevicePropCodeExposureTime                  = 0x500D,
    EMTPDevicePropCodeExposureProgramMode           = 0x500E,
    EMTPDevicePropCodeExposureIndex                 = 0x500F,
    EMTPDevicePropCodeExposureBiasCompensation      = 0x5010,
    EMTPDevicePropCodeDateTime                      = 0x5011,
    EMTPDevicePropCodeCaptureDelay                  = 0x5012,
    EMTPDevicePropCodeStillCaptureMode              = 0x5013,
    EMTPDevicePropCodeContrast                      = 0x5014,
    EMTPDevicePropCodeSharpness                     = 0x5015,
    EMTPDevicePropCodeDigitalZoom                   = 0x5016,
    EMTPDevicePropCodeEffectMode                    = 0x5017,
    EMTPDevicePropCodeBurstNumber                   = 0x5018,
    EMTPDevicePropCodeBurstInterval                 = 0x5019,
    EMTPDevicePropCodeTimelapseNumber               = 0x501A,
    EMTPDevicePropCodeTimelapseInterval             = 0x501B,
    EMTPDevicePropCodeFocusMeteringMode             = 0x501C,
    EMTPDevicePropCodeUploadURL                     = 0x501D,
    EMTPDevicePropCodeArtist                        = 0x501E,
    EMTPDevicePropCodeCopyrightInfo                 = 0x501F,
    EMTPDevicePropCodePTPEnd                        = 0x5FFF,
    EMTPDevicePropCodeVendorExtensionStart          = 0xD000,
    EMTPDevicePropCodeVendorExtensionEnd            = 0xD3FF,
    EMTPDevicePropCodeMTPStart                      = 0xD400,
    EMTPDevicePropCodeSynchronizationPartner        = 0xD401,
    EMTPDevicePropCodeDeviceFriendlyName            = 0xD402,
    EMTPDevicePropCodeVolume                        = 0xD403,
    EMTPDevicePropCodeSupportedFormatsOrdered       = 0xD404,
    EMTPDevicePropCodeDeviceIcon                    = 0xD405,
    EMTPDevicePropCodePlaybackRate                  = 0xD410,
    EMTPDevicePropCodePlaybackObject                = 0xD411,
    EMTPDevicePropCodePlaybackContainerIndex        = 0xD412,
    EMTPDevicePropCodePlaybackPosition              = 0xD413,
    EMTPDevicePropCodeSessionInitiatorVersionInfo   = 0xD406,
    EMTPDevicePropCodePerceivedDeviceType           = 0xD407,
    EMTPDevicePropCodeMTPEnd                        = 0xD7FF,
    EMTPDevicePropCodeFunctionalID					= 0xD301,
    EMTPDevicePropCodeModelID						= 0xD302,    
    EMTPDevicePropCodeUseDeviceStage				= 0xD303
    };

/**
The MTP device property datacodes. For a description of these datacodes, please 
refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPObjectPropertyCode    
    {
    EMTPObjectPropCodeUndefined                         = 0x0000,
    EMTPObjectPropCodeVendorExtensionStart              = 0xD800,
    EMTPObjectPropCodeVendorExtensionEnd                = 0xDBFF,
    EMTPObjectPropCodeMTPStart                          = 0xDC01,
    EMTPObjectPropCodeStorageID                         = EMTPObjectPropCodeMTPStart,
    EMTPObjectPropCodeObjectFormat                      = 0xDC02,
    EMTPObjectPropCodeProtectionStatus                  = 0xDC03,
    EMTPObjectPropCodeObjectSize                        = 0xDC04,
    EMTPObjectPropCodeAssociationType                   = 0xDC05,
    EMTPObjectPropCodeAssociationDesc                   = 0xDC06,
    EMTPObjectPropCodeObjectFileName                    = 0xDC07,
    EMTPObjectPropCodeDateCreated                       = 0xDC08,
    EMTPObjectPropCodeDateModified                      = 0xDC09,
    EMTPObjectPropCodeKeywords                          = 0xDC0A,
    EMTPObjectPropCodeParentObject                      = 0xDC0B,
    EMTPObjectPropCodeAllowedFolderContents             = 0xDC0C,
    EMTPObjectPropCodeHidden                            = 0xDC0D,
    EMTPObjectPropCodeSystemObject                      = 0xDC0E,
    EMTPObjectPropCodePersistentUniqueObjectIdentifier  = 0xDC41,
    EMTPObjectPropCodeSyncID                            = 0xDC42,
    EMTPObjectPropCodePropertyBag                       = 0xDC43,
    EMTPObjectPropCodeName                              = 0xDC44,
    EMTPObjectPropCodeCreatedBy                         = 0xDC45,
    EMTPObjectPropCodeArtist                            = 0xDC46,
    EMTPObjectPropCodeDateAuthored                      = 0xDC47,
    EMTPObjectPropCodeDescription                       = 0xDC48,
    EMTPObjectPropCodeURLReference                      = 0xDC49,
    EMTPObjectPropCodeLanguageLocale                    = 0xDC4A,
    EMTPObjectPropCodeCopyrightInformation              = 0xDC4B,
    EMTPObjectPropCodeSource                            = 0xDC4C,
    EMTPObjectPropCodeOriginLocation                    = 0xDC4D,
    EMTPObjectPropCodeDateAdded                         = 0xDC4E,
    EMTPObjectPropCodeNonConsumable                     = 0xDC4F,
    EMTPObjectPropCodeCorruptUnplayable                 = 0xDC50,
    EMTPObjectPropCodeProducerSerialNumber              = 0xDC51,
    EMTPObjectPropCodeRepresentativeSampleFormat        = 0xDC81,
    EMTPObjectPropCodeRepresentativeSampleSize          = 0xDC82,
    EMTPObjectPropCodeRepresentativeSampleHeight        = 0xDC83,
    EMTPObjectPropCodeRepresentativeSampleWidth         = 0xDC84,
    EMTPObjectPropCodeRepresentativeSampleDuration      = 0xDC85,
    EMTPObjectPropCodeRepresentativeSampleData          = 0xDC86,
    EMTPObjectPropCodeWidth                             = 0xDC87,
    EMTPObjectPropCodeHeight                            = 0xDC88,
    EMTPObjectPropCodeDuration                          = 0xDC89,
    EMTPObjectPropCodeRating                            = 0xDC8A,
    EMTPObjectPropCodeTrack                             = 0xDC8B,
    EMTPObjectPropCodeGenre                             = 0xDC8C,
    EMTPObjectPropCodeCredits                           = 0xDC8D,
    EMTPObjectPropCodeLyrics                            = 0xDC8E,
    EMTPObjectPropCodeSubscriptionContentID             = 0xDC8F,
    EMTPObjectPropCodeProducedBy                        = 0xDC90,
    EMTPObjectPropCodeUseCount                          = 0xDC91,
    EMTPObjectPropCodeSkipCount                         = 0xDC92,
    EMTPObjectPropCodeLastAccessed                      = 0xDC93,
    EMTPObjectPropCodeParentalRating                    = 0xDC94,
    EMTPObjectPropCodeMetaGenre                         = 0xDC95,
    EMTPObjectPropCodeComposer                          = 0xDC96,
    EMTPObjectPropCodeEffectiveRating                   = 0xDC97,
    EMTPObjectPropCodeSubtitle                          = 0xDC98,
    EMTPObjectPropCodeOriginalReleaseDate               = 0xDC99,
    EMTPObjectPropCodeAlbumName                         = 0xDC9A,
    EMTPObjectPropCodeAlbumArtist                       = 0xDC9B,
    EMTPObjectPropCodeMood                              = 0xDC9C,
    EMTPObjectPropCodeDRMStatus                         = 0xDC9D,
    EMTPObjectPropCodeSubDescription                    = 0xDC9E,
    EMTPObjectPropCodeIsCropped                         = 0xDCD1,
    EMTPObjectPropCodeIsColourCorrected                 = 0xDCD2,
    EMTPObjectPropCodeImageBitDepth                     = 0xDCD3,
    EMTPObjectPropCodeFnumber                           = 0xDCD4,
    EMTPObjectPropCodeExposureTime                      = 0xDCD5,
    EMTPObjectPropCodeExposureIndex                     = 0xDCD6,
    EMTPObjectPropCodeTotalBitRate                      = 0xDE91,
    EMTPObjectPropCodeBitrateType                       = 0xDE92,
    EMTPObjectPropCodeSampleRate                        = 0xDE93,
    EMTPObjectPropCodeNumberOfChannels                  = 0xDE94,
    EMTPObjectPropCodeAudioBitDepth                     = 0xDE95,
    EMTPObjectPropCodeScanType                          = 0xDE97,
    EMTPObjectPropCodeAudioWAVECodec                    = 0xDE99,
    EMTPObjectPropCodeAudioBitRate                      = 0xDE9A,
    EMTPObjectPropCodeVideoFourCCCodec                  = 0xDE9B,
    EMTPObjectPropCodeVideoBitRate                      = 0xDE9C,
    EMTPObjectPropCodeFramesPerThousandSeconds          = 0xDE9D,
    EMTPObjectPropCodeKeyFrameDistance                  = 0xDE9E,
    EMTPObjectPropCodeBufferSize                        = 0xDE9F,
    EMTPObjectPropCodeEncodingQuality                   = 0xDEA0,
    EMTPObjectPropCodeEncodingProfile                   = 0xDEA1,
    EMTPObjectPropCodeDisplayName                       = 0xDCE0,
    EMTPObjectPropCodeBodyText                          = 0xDCE1,
    EMTPObjectPropCodeSubject                           = 0xDCE2,
    EMTPObjectPropCodePriority                          = 0xDCE3,
    EMTPObjectPropCodeGivenName                         = 0xDD00,
    EMTPObjectPropCodeMiddleNames                       = 0xDD01,
    EMTPObjectPropCodeFamilyName                        = 0xDD02,
    EMTPObjectPropCodePrefix                            = 0xDD03,
    EMTPObjectPropCodeSuffix                            = 0xDD04,
    EMTPObjectPropCodePhoneticGivenName                 = 0xDD05,
    EMTPObjectPropCodePhoneticFamilyName                = 0xDD06,
    EMTPObjectPropCodeEmailPrimary                      = 0xDD07,
    EMTPObjectPropCodeEmailPersonal1                    = 0xDD08,
    EMTPObjectPropCodeEmailPersonal2                    = 0xDD09,
    EMTPObjectPropCodeEmailBusiness1                    = 0xDD0A,
    EMTPObjectPropCodeEmailBusiness2                    = 0xDD0B,
    EMTPObjectPropCodeEmailOthers                       = 0xDD0C,
    EMTPObjectPropCodePhoneNumberPrimary                = 0xDD0D,
    EMTPObjectPropCodePhoneNumberPersonal               = 0xDD0E,
    EMTPObjectPropCodePhoneNumberPersonal2              = 0xDD0F,
    EMTPObjectPropCodePhoneNumberBusiness               = 0xDD10,
    EMTPObjectPropCodePhoneNumberBusiness2              = 0xDD11,
    EMTPObjectPropCodePhoneNumberMobile                 = 0xDD12,
    EMTPObjectPropCodePhoneNumberMobile2                = 0xDD13,
    EMTPObjectPropCodeFaxNumberPrimary                  = 0xDD14,
    EMTPObjectPropCodeFaxNumberPersonal                 = 0xDD15,
    EMTPObjectPropCodeFaxNumberBusiness                 = 0xDD16,
    EMTPObjectPropCodePagerNumber                       = 0xDD17,
    EMTPObjectPropCodePhoneNumberOthers                 = 0xDD18,
    EMTPObjectPropCodePrimaryWebAddress                 = 0xDD19,
    EMTPObjectPropCodePersonalWebAddress                = 0xDD1A,
    EMTPObjectPropCodeBusinessWebAddress                = 0xDD1B,
    EMTPObjectPropCodeInstantMessengerAddress           = 0xDD1C,
    EMTPObjectPropCodeInstantMessengerAddress2          = 0xDD1D,
    EMTPObjectPropCodeInstantMessengerAddress3          = 0xDD1E,
    EMTPObjectPropCodePostalAddressPersonalFull         = 0xDD1F,
    EMTPObjectPropCodePostalAddressPersonalLine1        = 0xDD20,
    EMTPObjectPropCodePostalAddressPersonalLine2        = 0xDD21,
    EMTPObjectPropCodePostalAddressPersonalCity         = 0xDD22,
    EMTPObjectPropCodePostalAddressPersonalRegion       = 0xDD23,
    EMTPObjectPropCodePostalAddressPersonalPostalCode   = 0xDD24,
    EMTPObjectPropCodePostalAddressPersonalCountry      = 0xDD25,
    EMTPObjectPropCodePostalAddressBusinessFull         = 0xDD26,
    EMTPObjectPropCodePostalAddressBusinessLine1        = 0xDD27,
    EMTPObjectPropCodePostalAddressBusinessLine2        = 0xDD28,
    EMTPObjectPropCodePostalAddressBusinessCity         = 0xDD29,
    EMTPObjectPropCodePostalAddressBusinessRegion       = 0xDD2A,
    EMTPObjectPropCodePostalAddressBusinessPostalCode   = 0xDD2B,
    EMTPObjectPropCodePostalAddressBusinessCountry      = 0xDD2C,
    EMTPObjectPropCodePostalAddressOtherFull            = 0xDD2D,
    EMTPObjectPropCodePostalAddressOtherLine1           = 0xDD2E,
    EMTPObjectPropCodePostalAddressOtherLine2           = 0xDD2F,
    EMTPObjectPropCodePostalAddressOtherCity            = 0xDD30,
    EMTPObjectPropCodePostalAddressOtherRegion          = 0xDD31,
    EMTPObjectPropCodePostalAddressOtherPostalCode      = 0xDD32,
    EMTPObjectPropCodePostalAddressOtherCountry         = 0xDD33,
    EMTPObjectPropCodeOrganizationName                  = 0xDD34,
    EMTPObjectPropCodePhoneticOrganizationName          = 0xDD35,
    EMTPObjectPropCodeRole                              = 0xDD36,
    EMTPObjectPropCodeBirthdate                         = 0xDD37,
    EMTPObjectPropCodeMessageTo                         = 0xDD40,
    EMTPObjectPropCodeMessageCC                         = 0xDD41,
    EMTPObjectPropCodeMessageBCC                        = 0xDD42,
    EMTPObjectPropCodeMessageRead                       = 0xDD43,
    EMTPObjectPropCodeMessageReceivedTime               = 0xDD44,
    EMTPObjectPropCodeMessageSender                     = 0xDD45,
    EMTPObjectPropCodeActivityBeginTime                 = 0xDD50,
    EMTPObjectPropCodeActivityEndTime                   = 0xDD51,
    EMTPObjectPropCodeActivityLocation                  = 0xDD52,
    EMTPObjectPropCodeActivityRequiredAttendees         = 0xDD54,
    EMTPObjectPropCodeActivityOptionalAttendees         = 0xDD55,
    EMTPObjectPropCodeActivityResources                 = 0xDD56,
    EMTPObjectPropCodeActivityAccepted                  = 0xDD57,
    EMTPObjectPropCodeActivityTentative                 = 0xDD58,
    EMTPObjectPropCodeActivityDeclined                  = 0xDD59,
    EMTPObjectPropCodeActivityReminderTime              = 0xDD5A,
    EMTPObjectPropCodeActivityOwner                     = 0xDD5B,
    EMTPObjectPropCodeActivityStatus                    = 0xDD5C,
    EMTPObjectPropCodeMTPEnd = EMTPObjectPropCodeActivityStatus    
    };

/**
The MTP association type datacodes. For a description of these datacodes, 
please refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPAssociationType
    {    
    EMTPAssociationTypeUndefined            = 0x0000,
    EMTPAssociationTypeGenericFolder        = 0x0001,
    EMTPAssociationTypeAlbum                = 0x0002,
    EMTPAssociationTypeTimeSequence         = 0x0003,
    EMTPAssociationTypeHorizontalPanoramic  = 0x0004,
    EMTPAssociationTypeVerticalPanoramic    = 0x0005,
    EMTPAssociationType2DPanoramic          = 0x0006,
    EMTPAssociationTypeAncillaryData        = 0x0007
    };

/**
The MTP protection status atacodes. For a description of these datacodes, 
please refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPProtectionStatus
    {
    EMTPProtectionNoProtection      = 0x0000,
    EMTPProtectionReadOnly          = 0x0001,
    EMTPProtectionReadOnlyData      = 0x8002, 
    EMTPProtectionNonTransferable   = 0x8003
    };

/**
The MTP consumable status datacodes. For a description of these datacodes, 
please refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPNonConsumable
	{
	EMTPConsumable		= 0x00,
	EMTPNonConsumable	= 0x01,
	};

/**
The MTP storage type datacodes. For a description of these datacodes, please 
refer to the "Media Transfer Protocol Enhanced" specification.
*/
enum TMTPStorageType
	{
	EMTPStorageUndefined 	= 0x0000,
	EMTPStorageFixedROM 	= 0x0001,
	EMTPStorageRemovableROM = 0x0002,
	EMTPStorageFixedRAM 	= 0x0003,
	EMTPStorageRemovableRAM = 0x0004	
	};

/**
The MTP filesystem type datacodes. For a description of these datacodes, please 
refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPFileSystemType
	{
	EMTPFileSystemUndefined 			= 0x0000,
	EMTPFileSystemGenericFlat 			= 0x0001,
	EMTPFileSystemGenericHierarchical 	= 0x0002,
	EMTPFileSystemDCF					= 0x0003
	};

/**
The MTP storage access capability type datacodes. For a description of these datacodes, 
please refer to the "Media Transfer Protocol Enhanced" specification.
@publishedPartner
@released 
*/
enum TMTPAccessCapability
	{	
	EAccessCapabilityReadWrite						= 0x0000,
	EAccessCapabilityReadOnlyWithoutDeletion		= 0x0001,	
	EAccessCapabilityReadOnlyWithDeletion 			= 0x0002
	};

#endif // MTPPROTOCOLCONSTANTS_H
