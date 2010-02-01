// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __OBEXCONSTANTSINTERNAL_H
#define __OBEXCONSTANTSINTERNAL_H

#include <e32def.h>

//common to request & resp digests

/** @internalComponent */
const TInt KMinChallResponseSize = 18; //Nonce(16) + tag(1) + size(1)
/** @internalComponent */
const TInt KObexNonceSize = 16;
/** @internalComponent */
const TUint8 KObexRequireUID = 0x01;

//request or chall
/** @internalComponent */
const TUint KObexChallNonceTag = 0;
/** @internalComponent */
const TUint8 KObexChallOptionsTag = 0x01;
/** @internalComponent */
const TUint8 KObexChallRealmTag = 0x02;
/** @internalComponent */
const TInt KObexChallOptionSize = 1; //this can't be changed from one without making
/** @internalComponent */
const TUint KObexChallHeaderSize = 23; //assuming no Realm data

//response
/** @internalComponent */
const TUint KObexRespTag = 0;
/** @internalComponent */
const TUint8 KObexRespUserIDTag = 0x01;
/** @internalComponent */
const TUint8 KObexRespNonceTag = 0x02;

/** @internalComponent */
const TUint8 KObexHeaderTypeMask = 0xc0;
/** @internalComponent */
const TUint8 KObexHeaderTypeOffset = 6;


#ifdef _UNICODE
/** @internalComponent */
const TInt KUidOBEXTransportModule = 0x10003d56;
#else
/** @internalComponent */
const TInt KUidOBEXTransportModule = 0x1000169c;
#endif

/** @internalComponent */
const TUint8 KFourByteTimeHeaderAddress = 0xc4;

// Header mask definitions.
// Old style accessor for which headers are present or
// should be sent by an object.  Now deprecated, use the
// header iterator instead.
/** @internalComponent */
const TUint16 KObexHdrName				= 0x0001;
/** @internalComponent */
const TUint16 KObexHdrType				= 0x0002;
/** @internalComponent */
const TUint16 KObexHdrLength			= 0x0004;
/** @internalComponent */
const TUint16 KObexHdrTime				= 0x0008;
/** @internalComponent */
const TUint16 KObexHdrDescription		= 0x0010;
/** @internalComponent */
const TUint16 KObexHdrTarget			= 0x0020;
/** @internalComponent */
const TUint16 KObexHdrConnectionID		= 0x0040;
/** @internalComponent */
const TUint16 KObexHdrBody				= 0x0080;
/** @internalComponent */
const TUint16 KObexHdrEndOfBody			= 0x0100;
/** @internalComponent */
const TUint16 KObexHdrHttp				= 0x0200;
/** @internalComponent */
const TUint16 KObexHdrAppParam			= 0x0400;
/** @internalComponent */
const TUint16 KObexHdrUserDefined		= 0x0800;
/** @internalComponent */
const TUint16 KObexHdrCount				= 0x1000;
/** @internalComponent */
const TUint16 KObexHdrCreatorID			= 0x2000;
/** @internalComponent */
const TUint16 KObexHdrWanUUID			= 0x4000;
/** @internalComponent */
const TUint16 KObexHdrObjectClass		= 0x8000;


/** @internalComponent */
const TUint8  KObexUserDefinedHdrAddrMin	 = 0x30; //start of user defined header address range
/** @internalComponent */
const TUint8  KObexUserDefinedHdrAddrMax	 = 0x3F; //end of user defined header address range

/** @internalComponent */
const TUint8 KObexVersion           	= 0x10;		//< Version 1.0 (still correct for Obex spec v 1.2)

/** @internalComponent */
const TUint8 KObexObjectFieldSize   	= 127;

// Flags used by SetPath
/** @internalComponent */
const TUint8 KObexSetPathParent	= 0x01;

#endif // __OBEXCONSTANTSINTERNAL_H