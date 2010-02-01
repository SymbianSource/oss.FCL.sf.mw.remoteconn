/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
* Declaration of CSBShutdown class.
* 
*
*/



/**
 @file
*/
#ifndef __SBSHUTDOWN_H__
#define __SBSHUTDOWN_H__

namespace conn
	{
	class CSBShutdown : public CTimer
	/**
    @internalComponent
	@ingroup Server

	Timer class used to shutdown the server after a specific interval.
	*/
		{
		public:
			CSBShutdown();
			void ConstructL();
			void Start();

		private:
			void RunL();
		};
	}

#endif //__SBSHUTDOWN_H__
