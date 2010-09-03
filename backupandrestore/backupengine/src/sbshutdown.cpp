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
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbshutdownTraces.h"
#endif

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
		OstTraceFunctionEntry0( CSBSHUTDOWN_CSBSHUTDOWN_CONS_ENTRY );
		OstTraceFunctionExit0( CSBSHUTDOWN_CSBSHUTDOWN_CONS_EXIT );
		}

	void CSBShutdown::ConstructL()
	/**
	Construct this instance of CSBShutdown.
	*/
		{
		OstTraceFunctionEntry0( CSBSHUTDOWN_CONSTRUCTL_ENTRY );
		CTimer::ConstructL();
		CActiveScheduler::Add(this);
		OstTraceFunctionExit0( CSBSHUTDOWN_CONSTRUCTL_EXIT );
		}

	void CSBShutdown::Start()
	/** Starts the timer. */
		{
		OstTraceFunctionEntry0( CSBSHUTDOWN_START_ENTRY );
		After(KShutdownDelay);
		OstTraceFunctionExit0( CSBSHUTDOWN_START_EXIT );
		}

	void CSBShutdown::RunL()
	/** Called after the timer has expired.
	
	Stop the active scheduler and shutdown the server.
	*/
		{
		OstTraceFunctionEntry0( CSBSHUTDOWN_RUNL_ENTRY );
		CActiveScheduler::Stop();
		OstTraceFunctionExit0( CSBSHUTDOWN_RUNL_EXIT );
		}
	}
