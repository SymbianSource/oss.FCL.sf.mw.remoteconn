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

#include <obex/transport/obexconnector.h>

/**
This function is a placeholder for future use. If iFuture1 variable is 
used it will need this function for any allocation required.

To prevent binary compatibility breaks if the iFuture1 variable is used, 
this function must be called from the NewL of derived classes.
*/
EXPORT_C void CObexConnector::BaseConstructL()
	{
	}

/**
This function is part of the extension pattern and must be implemented by 
all derived instantiable classes.
By default this returns null.  Any derived class that is required to 
extend its interface and that of this base class returns its new interface 
in the form of an M class, that it extends, if and only if  the 
corresponding TUid, aUid, is received. Otherwise the function calls the 
base class implementation, returning NULL.

@return TAny* The M Class representing the extension to the interface 
otherwise NULL
@param  aUid The uid associated with the M Class that is being implemented
@publishedPartner
@released
*/		
EXPORT_C TAny* CObexConnector::GetInterface(TUid /*aUid*/)
	{
	return NULL;
	}

/** Constructor.
@param aObserver The MObexTransportNotify object to notify of 
transport-related events. This object will typically be the concrete 
CObexTransportControllerBase.
*/
EXPORT_C CObexConnector::CObexConnector(MObexTransportNotify& aObserver)
:	iObserver(aObserver)
	{
	}

/**
Destructor.
Currently does no work (NB BaseConstructL currently does no work either). If 
BaseConstructL ever allocates memory into iFuture1, we'll need to free it 
here. But by then licensee connectors' vtables will already have been 
built, *pointing at CBase's destructor*. We need this destructor here so 
that iFuture1, if it's ever allocated, will be freed correctly.
*/
EXPORT_C CObexConnector::~CObexConnector()
	{
	}

/** Accessor for the observer (an MObexTransportNotify).
@return The MObexTransportNotify to notify of transport-related events.
*/
EXPORT_C MObexTransportNotify& CObexConnector::Observer()
	{
	return iObserver;
	}

