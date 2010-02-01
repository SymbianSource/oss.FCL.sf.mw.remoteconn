// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __OBEXINTERNALCONSTANTS_H
#define __OBEXINTERNALCONSTANTS_H

#include <e32def.h>

/**
@internalTechnology
@released
*/

// Constants describing miniumum, maximum and default OBEX pakcet sizes
// These are set to maintain backwards compatibility, so change with caution!

const TUint16 KObexPacketMaxSize		= 0xffff;	// Maximum possible packet size (64kB)
const TUint16 KObexPacketDefaultSize	= 4000;		// Fixed buffer size in previous implementations
const TUint16 KObexPacketMinSize		= 255;		// Minimum legal size of a pakcet

// ########## Constant Data ##########

const TUint8  KObexPacketFinalBit 	= 0x80;		// Most signif. Bit is final flag.//used only by obexpacket
const TUint16 KObexPacketHeaderSize		= 3;		// Head = Opcode (1) + Size (2) //this const is used all over

#endif // __OBEXINTERNALCONSTANTS_H
