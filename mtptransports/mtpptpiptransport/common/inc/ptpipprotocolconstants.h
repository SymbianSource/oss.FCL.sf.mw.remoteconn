// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#ifndef PTPIPROTOCOLCONSTANTS_H
#define PTPIPROTOCOLCONSTANTS_H

#include <e32def.h>

/** 
Defines constant values specified in the PTPIP protocol.
@internalComponent
*/

/**
The PTPIP Protocol constants as defined in the PTPIP specification. 
*/
enum TPTPIPPacketTypeCode
	{
	EPTPIPPacketTypeUndefined = 		0x0000,
	EPTPIPPacketTypeCmdReq = 			0x0001,
	EPTPIPPacketTypeCmdAck = 			0x0002,
	EPTPIPPacketTypeEventReq = 			0x0003,
	EPTPIPPacketTypeEventAck = 			0x0004,
	EPTPIPPacketTypeEventFail = 		0x0005,
	EPTPIPPacketTypeOperationRequest = 	0x0006,
	EPTPIPPacketTypeOperationResponse = 0x0007,
	EPTPIPPacketTypeEvent = 			0x0008,
	EPTPIPPacketTypeStartData = 		0x0009,
	EPTPIPPacketTypeData = 				0x000A,
	EPTPIPPacketTypeCancel = 			0x000B,
	EPTPIPPacketTypeEndData = 			0x000C,
	EPTPIPPacketTypeProbeRequest = 		0x000D,
	EPTPIPPacketTypeProbeResponse = 	0x000E,
	};

/*
 * PTPIP transport implementation UID
 */
const TUint KMTPPTPIPTransportImplementationUid  = 0xA0004A60;

#endif // PTPIPROTOCOLCONSTANTS_H
