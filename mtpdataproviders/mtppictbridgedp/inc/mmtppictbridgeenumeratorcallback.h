// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef MMTPPICTBRIDGEENUMERATORCALLBACK_H
#define MMTPPICTBRIDGEENUMERATORCALLBACK_H

#include <e32std.h>

/** 
Defines the PictBridgeDP enumeration complete notification interface.
@internalComponent
*/
class MMTPPictBridgeEnumeratorCallback
	{
public:
	/**
	Notifies that storage enumeration is completed.
	*/
	virtual void NotifyStorageEnumerationCompleteL() = 0;

	/**
	Notifies that enumeration of the specified storage is completed.
    @param aStorageId The MTP StorageID of the enumerated storage.
	@param aError The enumeration completion error status.
	*/
	virtual void NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError) = 0;

	};
	
#endif // MMTPPICTBRIDGEENUMERATORCALLBACK_H
