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

#ifndef CMTPFSENTRYCACHE_H
#define CMTPFSENTRYCACHE_H

#include <f32file.h> 
#include "mtpdebug.h"

/** 
This class is used to cache the fs entry information of the big file (>1G) which is being
copied/moved in fs. The cache information is used to send response to getobjectproplist 
and getobjectinfo

@internalTechnology
*/
class CMTPFSEntryCache : public CBase
	{
public:
	IMPORT_C static CMTPFSEntryCache* NewL();
	IMPORT_C ~CMTPFSEntryCache();
	
  IMPORT_C TBool IsOnGoing() const;
  IMPORT_C void SetOnGoing(TBool aOnGoing);
  IMPORT_C TUint32 TargetHandle() const;
  IMPORT_C void SetTargetHandle(TUint32 aHandle);
  IMPORT_C TEntry& FileEntry();
  IMPORT_C void SetFileEntry(const TEntry& aEntry);
	
private:
	CMTPFSEntryCache();	
	void ConstructL();

private:
  /**
  Indicate if a big file copy/move is ongoing
  */
  TBool             iIsOngoing;

  /**
  The handle of target big file that is being copied/moved
  */
  TUint32           iTargetHandle;

  /**
  The fake file entry of target big file that is being copied/moved
  Since the target big file is not created in file system, this fake entry is
  used to return to getobjectproplist and getobjectinfo operations
  */
  TEntry            iFileEntry;	
	/**
    FLOGGER debug trace member variable.
  */
  __FLOG_DECLARATION_MEMBER_MUTABLE;
	};
	
#endif // CMTPFSENTRYCACHE_H
