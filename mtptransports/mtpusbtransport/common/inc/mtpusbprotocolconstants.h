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

#ifndef MTPUSBPROTOCOLCONSTANTS_H
#define MTPUSBPROTOCOLCONSTANTS_H

#include <e32def.h>

/** 
Defines constant values specified USB MTP device class protocol.
@internalComponent
*/

/**
MTP USB device class device status datacodes. For a description of these 
datacodes, please refer to the "Universal Serial Bus Still Image Capture 
Device Definition" specification.
*/
enum TMTPUsbContainerTypeCode
    {
    EMTPUsbContainerTypeUndefined       = 0x0000,
    EMTPUsbContainerTypeCommandBlock    = 0x0001,
    EMTPUsbContainerTypeDataBlock       = 0x0002,
    EMTPUsbContainerTypeResponseBlock   = 0x0003,
    EMTPUsbContainerTypeEventBlock      = 0x0004,
    };

/**
MTP USB device class control request datacodes. For a description of these 
datacodes, please refer to the "Universal Serial Bus Still Image Capture 
Device Definition" specification.
*/
enum TMTPUsbControlRequestCode
    {
    EMTPUsbControlRequestCancel             = 0x64,
    EMTPUsbControlRequestDeviceReset        = 0x66,
    EMTPUsbControlRequestDeviceStatus       = 0x67,  
    };

/**
MTP USB device class device status datacodes. For a description of these 
datacodes, please refer to the "Universal Serial Bus Still Image Capture 
Device Definition" specification.
*/
enum TMTPUsbDeviceCode
    {
    EMTPUsbDeviceStatusOK                   = 0x2001,  
    EMTPUsbDeviceStatusBusy                 = 0x2019,  
    EMTPUsbDeviceStatusTransactionCancelled = 0x201F,  
    };

/**
The MTP USB device class interface description string.
*/
_LIT16(KMTPUsbInterfaceString, "MTP");

/**
Still Image Capture (SIC) device class definitions, as specified in the 
"Universal Serial Bus Still Image Capture Device Definition" specification.
*/
const TUint8 KMTPUsbInterfaceClassSIC(0x06);
const TUint8 KMTPUsbInterfaceSubClassSIC(0x01);
const TUint8 KMTPUsbInterfaceProtocolSIC(0x01);

/**
MTP device class definitions.
*/
const TUint8 KMTPUsbInterfaceClassUnknown(0x00);

/**
The maximum value of the USB generic containrer dataset ContainerLength field.
*/
const TUint32 KMTPUsbContainerLengthMax(0xFFFFFFFF);

/*
 * Usb transport implementation UID
 */
const TUint KMTPUsbTransportImplementationUid  = 0x102827B2;

#endif // MTPUSBPROTOCOLCONSTANTS_H
