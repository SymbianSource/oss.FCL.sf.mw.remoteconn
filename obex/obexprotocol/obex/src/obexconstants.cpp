// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <obexconstants.h>
#include "obexconstantsinternal.h"
#include <obex/internal/obexinternalconstants.h>
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/**
@file
@internalComponent
*/

/**
Sets up class with default values
@publishedAll
@released
*/
EXPORT_C TObexProtocolPolicy::TObexProtocolPolicy ()
	{
	LOG_LINE
	LOG_FUNC

	// Default set-up for version 0
	// This is set for backwards compatibility, so change with caution!
	iVersion = 0;
	iReceiveMtu = KObexPacketDefaultSize;
	iTransmitMtu = KObexPacketDefaultSize;
	}

/**
Sets the maximum size of the receive packet buffer for the policy.
For the IrDA transport, if the new receive buffer size is different to
KObexPacketDefaultSize the receive packet size is set to equal the new
buffer size.  Otherwise the packet size is set to the protocol defined limit.
@param aReceiveMtu The new buffer size
@return KErrNone on success. KErrArgument if the packet size is not supported
@publishedAll
@released
*/
EXPORT_C TInt TObexProtocolPolicy::SetReceiveMtu ( TUint16 aReceiveMtu )
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\taReceiveMtu = %d"), aReceiveMtu);

	// Can't set size greater than the maximum, so only check if it's too small
	if ( aReceiveMtu >= KObexPacketMinSize )
		iReceiveMtu = aReceiveMtu;
	
	TInt ret = ( iReceiveMtu == aReceiveMtu ) ? KErrNone : KErrArgument;	
	LOG1(_L8("\tret = %d"), ret);
	return ret;
	}

/**
Sets the maximum size of the transmit packet buffer for the policy
@param aTransmitMtu The new buffer size
@return KErrNone on success. KErrArgument if the packet size is not supported
@publishedAll
@released
*/
EXPORT_C TInt TObexProtocolPolicy::SetTransmitMtu ( TUint16 aTransmitMtu )
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\taTransmitMtu = %d"), aTransmitMtu);

	// Can't set size greater than the maximum, so only check if it's too small
	if ( aTransmitMtu >= KObexPacketMinSize )
		iTransmitMtu = aTransmitMtu;

	TInt ret = ( iTransmitMtu == aTransmitMtu ) ? KErrNone : KErrArgument;
	LOG1(_L8("\tret = %d"), ret);
	return ret;
	}

/**
Gets the version of the protocol policy format that this class supports
@return Protocol policy version
@publishedAll
@released
*/
EXPORT_C TUint16 TObexProtocolPolicy::Version () const
	{
	LOG_LINE
	LOG_FUNC

	LOG1(_L8("\tiVersion = %d"), iVersion);
	return iVersion;
	}

/**
Gets the policy's receive packet buffer size
@return Receive packet buffer size
@publishedAll
@released
*/
EXPORT_C TUint16 TObexProtocolPolicy::ReceiveMtu () const
	{
	LOG_LINE
	LOG_FUNC
	   
	LOG1(_L8("\tiReceiveMtu = %d"), iReceiveMtu);
	return iReceiveMtu;
	}

/**
Gets the policy's transmit packet buffer size
@return Transmit packet buffer size
@publishedAll
@released
*/
EXPORT_C TUint16 TObexProtocolPolicy::TransmitMtu () const
	{
	LOG_LINE
	LOG_FUNC

	LOG1(_L8("\tiTransmitMtu = %d"), iTransmitMtu);
	return iTransmitMtu;
	}

// TObexConnectInfo
/**
@publishedAll
@released
*/
EXPORT_C TObexConnectInfo::TObexConnectInfo() 
	{
	LOG_LINE
	LOG_FUNC

	iVersion = KObexVersion;
	iFlags = 0;
	}

/**
@publishedAll
@released
*/
EXPORT_C TUint8 TObexConnectInfo::VersionMajor() const 
	{
	LOG_LINE
	LOG_FUNC

	return(STATIC_CAST(TUint8,(iVersion & 0xF0) >> 4));
	}

/**
@publishedAll
@released
*/
EXPORT_C TUint8 TObexConnectInfo::VersionMinor() const 
	{ 
	LOG_LINE
	LOG_FUNC

	return(STATIC_CAST(TUint8,(iVersion & 0x0F)));
	}
