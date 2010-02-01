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

#ifndef MTPUSBTRANSPORTCONSTANTS_H
#define MTPUSBTRANSPORTCONSTANTS_H

#include <d32usbc.h>
#include <e32def.h>

/**
The USB MTP device class alternate interface number.
*/
const TUint KMTPUsbAlternateInterface(0);

/**
* Endpoint address index of Endpoint descriptor
*/
const TUint KEpAddressOffsetInEpDesc(2);

/**
The USB MTP device class USB client controller number.
*/
const TUint KDefaultUsbClientController(0);


/**
The Full Speed USB MaxPacketsize for different Endpoints.
*/
const TUint KMaxPacketTypeControlFS(16);
const TUint KMaxPacketTypeBulkFS(64);

/**
The High Speed USB MaxPacketSize for different Endpoints.
*/
const TUint KMaxPacketTypeControlHS(64);
const TUint KMaxPacketTypeBulkHS(512);

/*
 * USB MaxPacketSize for interrupt Endpoint.
 */
const TUint KMaxPacketTypeInterrupt(64);

/**
The minimum number of endpoints required to support the USM MTP device class, 
as specified in the "Universal Serial Bus Still Image Capture Device 
Definition" specification. This value excludes the control, endpoint.
*/
const TUint KMTPUsbRequiredNumEndpoints(3);

/**
USB MTP device class logical endpoint bit position bits.
*/
const TUint KMTPUsbControlEpBit(KUsbcEndpoint0Bit);
const TUint KMTPUsbBulkInEpBit(KUsbcEndpoint1Bit);
const TUint KMTPUsbBulkOutEpBit(KUsbcEndpoint2Bit);
const TUint KMTPUsbInterruptEpBit(KUsbcEndpoint3Bit);

/**
USB MTP device class endpoint direction flag bits.
*/
const TUint KMTPUsbControlEpDir(KUsbEpDirBidirect);
const TUint KMTPUsbBulkInEpDir(KUsbEpDirIn);
const TUint KMTPUsbBulkOutEpDir(KUsbEpDirOut);
const TUint KMTPUsbInterruptEpDir(KUsbEpDirIn);

/**
USB MTP device class logical endpoint numbers.
*/
const TEndpointNumber KMTPUsbControlEp(EEndpoint0);
const TEndpointNumber KMTPUsbBulkInEp(EEndpoint1);
const TEndpointNumber KMTPUsbBulkOutEp(EEndpoint2);
const TEndpointNumber KMTPUsbInterruptEp(EEndpoint3);

/**
USB MTP device class logical endpoint polling intervals (Ms).
*/
const TUint KMTPUsbControlEpPoll(0);
const TUint KMTPUsbBulkInEpPoll(0);
const TUint KMTPUsbBulkOutEpPoll(0);
const TUint KMTPUsbInterruptEpPoll(5);

/**
USB MTP high speed NAK rates.
*/
const TUint KMTPUsbControlEpNAKRate(1);
const TUint KMTPUsbBulkInEpNAKRate(1);
const TUint KMTPUsbBulkOutEpNAKRate(1);
const TUint KMTPUsbInterruptEpNAKRate(1);

/**
USB MTP device class endpoint type flag bits.
*/
const TUint KMTPUsbControlEpType(KUsbEpTypeControl);
const TUint KMTPUsbBulkInEpType(KUsbEpTypeBulk);
const TUint KMTPUsbBulkOutEpType(KUsbEpTypeBulk);
const TUint KMTPUsbInterruptEpType(KUsbEpTypeInterrupt);

/**
USM MTP device class internal endpoint identifiers.
*/
enum TMTPUsbEndpointId
    {
    EMTPUsbEpControl,
    EMTPUsbEpBulkIn,
    EMTPUsbEpBulkOut,
    EMTPUsbEpInterrupt,
    EMTPUsbEpNumEndpoints,
    };

#endif // MTPUSBTRANSPORTCONSTANTS_H
