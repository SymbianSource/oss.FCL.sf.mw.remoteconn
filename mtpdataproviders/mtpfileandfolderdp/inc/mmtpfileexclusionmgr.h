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

/**
 @file
 @internalTechnology
*/

#ifndef __MMTPFILEEXCLUSIONMGR_H__
#define __MMTPFILEEXCLUSIONMGR_H__

#include <e32std.h>

/** 
Defines an interface to check if a filename is excluded or not.

@internalTechnology

*/
class MMTPFileExclusionMgr
	{
public:
	/**
	check wheather a filename is excluded or not
	@param aFileName The full path name of the file
	@return ETrue if the file is not excluded, otherwise EFalse
	*/
	virtual TBool IsItemAccepted(const TDesC& aFileName) const = 0;
	};
	


#endif // __MMTPFILEEXCLUSIONMGR_H__

