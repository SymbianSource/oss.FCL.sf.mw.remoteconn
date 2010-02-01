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
* Main entry point in the process.
* 
*
*/



/**
 @file
*/

#ifndef __SBMAIN_H__
#define __SBMAIN_H__

#include <e32base.h>


/** @mainpage Secure Backup Documentation

This is the documentation for Secure Backup.


@section UsingDoc How to use this documentation

The documentation can be split into three logical groups and they are as follows:


@par Secure Backup Modules

@li @ref SBEServer  This is the Secure Backup Engine module documentation

@li @ref ABServer  This is Active Backup module documentation

*/


/**
@namespace conn

This namespace is the global Symbian Connect namespace which encapsulates 
all of the connectivity components within Symbian OS.
*/

namespace conn
	{
	
	class CSBActiveScheduler : public CActiveScheduler
	/**
	@internalComponent
    
    The active scheduler for secure backup.
	*/	
		{
		public:
		    /** C++ Constructor */
			CSBActiveScheduler() : CActiveScheduler() { }
			void Error(TInt aError) const;
		};
	}	
		
	
#endif //__SBMAIN_H__
