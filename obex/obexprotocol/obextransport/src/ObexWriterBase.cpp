// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <obex/transport/obexwriterbase.h>
#include <obex/internal/obexpacket.h>
#include <obex/transport/mobextransportnotify.h>
#include "logger.h"
#include "obextransportfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ObexWriterBase");
#endif

/**
Constructor
*/
EXPORT_C CObexWriterBase::CObexWriterBase(TPriority aPriority, 
										  MObexTransportNotify& aOwner, 
										  TObexConnectionInfo& aInfo)
	: CObexActiveRW (aPriority,  aOwner, aInfo)
	{
	LOG_LINE
	LOG_FUNC
	}

/**
Destructor
*/
EXPORT_C CObexWriterBase::~CObexWriterBase()
	{
	LOG_LINE
	LOG_FUNC
	}
	
/**
This function is a placeholder for future use. If the iFuture1 variable is 
used it will need this function for any allocation required.

To prevent binary compatiblity breaks if the iFuture1 variable is used, this 
function must be called from the NewL of derived classes.
*/
EXPORT_C void CObexWriterBase::BaseConstructL()
	{
	LOG_LINE
	LOG_FUNC
	}

/**
Start transfer. Calls into CObexActiveRW, which eventaully queues a write 

@param aPacket The Obex packet to write
 */
EXPORT_C void CObexWriterBase::StartTransfer (CObexPacket& aPacket)
	{
	LOG_LINE
	LOG_FUNC

	iPacketSize = aPacket.PacketSize();
	NewRequest(aPacket);

	if(iPacket->IsFinal() &&
	  (iPacket->PacketProcessNotificationEvents() & EObexFinalPacketStarted))
		{
		iOwner.SignalPacketProcessEvent(EObexFinalPacketStarted);
		}
	}

/**
Returns true if the transfer has completed

@return TBool return true if the transfer if complete otherwise return false
*/
EXPORT_C TBool CObexWriterBase::CompleteTransfer ()
	{
	LOG_LINE
	LOG_FUNC

	return (iCount >= iPacketSize);
	}

/** Performs any actions necessary on completion of a write.
*/
EXPORT_C void CObexWriterBase::OnCompleteTransfer()
	{
	iOwner.SignalPacketProcessEvent(EObexWriteCompleted);

	if(iPacket->IsFinal() &&
	  (iPacket->PacketProcessNotificationEvents() & EObexFinalPacketFinished))
		{
		iOwner.SignalPacketProcessEvent(EObexFinalPacketFinished);
		}

	// Signal this after dereferences of iPacket because it might result in 
	// our destruction.
	iOwner.SignalPacketProcessEvent(EObexWriteCompletedFinal);
	}
	
/**
Return number of bytes remaining to be sent	
@return TInt number of bytes remaining to be sent
*/
EXPORT_C TInt CObexWriterBase::Remaining ()
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (iPacketSize >= KObexPacketHeaderSize, PANIC(KPanicCat, EInvalidPacketSize));
	return (iPacketSize - iCount);
	}

/**
This function is part of the extension pattern and is implemented by all 
derived instantiable classes that need to extend its interface.

By default this returns null. Any derived class that is required to extend its 
interface and that of this base class returns its new interface in the form of 
an M class, that it extends, if and only if the corresponding TUid, aUid, is 
received. Otherwise the function calls the base class implementation, 
returning NULL.

@return TAny* The M Class representing the extension to the interface 
otherwise NULL
@param aUid The uid associated with the M Class that is being implemented
*/	
EXPORT_C TAny* CObexWriterBase::GetInterface(TUid /*aUid*/)
	{
	return NULL;
	}
