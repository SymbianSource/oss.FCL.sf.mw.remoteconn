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

#ifndef OBEXREADACTIVITYOBSERVER_H
#define OBEXREADACTIVITYOBSERVER_H

#include <e32std.h>

/**
This mixin class must be inherited by any class interested in read activity notifications. 

@publishedPartner
@released
*/

class MObexReadActivityObserver
	{
public:

	/** Indicates read activity.
	This is guaranteed to be called when the transport completes the first 
	transport read in an Obex operation (which may correspond to less than a 
	complete Obex packet), and may be called at additional times before the 
	operation is completed.
	Note that the upcall behaviour is dependent on transport layer and 
	version -- even if experiments show more frequent upcalls, the only 
	behaviour which may be assumed is a single upcall per operation.
	
	@publishedPartner
	@released
	*/
	virtual void MoraoReadActivity() {};
	};

#endif // OBEXREADACTIVITYOBSERVER_H

