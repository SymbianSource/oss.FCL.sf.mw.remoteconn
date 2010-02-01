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
// Implementation of CSBShutdown class.
// 
//

/**
 @file
*/

#include <e32std.h>
#include <e32base.h>
#include "sbshutdown.h"

namespace conn
	{

	/** Shutdown delay, in microseconds.
	 @internalComponent */
	const TUint KShutdownDelay = 0x200000;
	
	CSBShutdown::CSBShutdown() : 
	CTimer(EPriorityNormal)
    /**
    Class Constructor
    */
		{
		}

	void CSBShutdown::ConstructL()
	/**
	Construct this instance of CSBShutdown.
	*/
		{
		CTimer::ConstructL();
		CActiveScheduler::Add(this);
		}

	void CSBShutdown::Start()
	/** Starts the timer. */
		{
		After(KShutdownDelay);
		}

	void CSBShutdown::RunL()
	/** Called after the timer has expired.
	
	Stop the active scheduler and shutdown the server.
	*/
		{
		CActiveScheduler::Stop();
		}
	}
