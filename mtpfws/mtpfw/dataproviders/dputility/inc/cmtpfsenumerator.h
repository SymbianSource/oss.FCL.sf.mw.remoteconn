// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef MTPFILEENUMERATOR_H
#define MTPFILEENUMERATOR_H

#include <e32base.h>
#include <f32file.h>

#include "rmtpdpsingletons.h"
#include "rmtpframework.h"

class MMTPDataProviderFramework;
class MMTPDataProviderFramework;
class MMTPObjectMgr;
class MMTPEnumerationCallback;
class CMTPObjectMetaData;
class CMTPFSExclusionMgr;

/** 
Defines file enumerator.  Enumerates all files/directories under a specified path or storage

@internalTechnology
*/
class CMTPFSEnumerator : public CActive
    {
public:
    IMPORT_C static CMTPFSEnumerator* NewL(MMTPDataProviderFramework& aFramework, CMTPFSExclusionMgr& aExclusionMgr, MMTPEnumerationCallback& aCallback, TInt aProcessLimit);
    IMPORT_C ~CMTPFSEnumerator();

	IMPORT_C void StartL(TUint32 aStorageId, TBool aScanAll = EFalse);	
	
private:
	//from CActive
	void DoCancel();
	void RunL();
	TInt RunError(TInt aError);
	
private:
	CMTPFSEnumerator(MMTPDataProviderFramework& aFramework, CMTPFSExclusionMgr& aExclusionMgr, MMTPEnumerationCallback& aCallback, TInt aProcessLimit);
	void ConstructL();
	
	void ScanDirL();
	void ScanStorageL(TUint32 aStorageId);
	void ScanNextL();
	void ScanNextStorageL();
	void ScanNextSubdirL();
	void ProcessEntriesL();
	void AddEntryL(const TDesC& aPath, TUint32 &aHandle, TMTPFormatCode format, TUint32 aDPId, const TEntry& aEntry, TUint32 aStorageId, TUint32 aParentHandle);
	void AddFileEntryForOtherDpL(const TDesC& aPath, TUint32 &aHandle, TMTPFormatCode format, TUint32 aDPId, const TEntry& aEntry, TUint32 aStorageId, TUint32 aParentHandle, TUint16 aSubFormatCode = 0);
	void NotifyObjectAddToDP(const TUint32 aHandle,const TUint DpId);
	
	
private:
    static  const TUint KMTPMaxFullFileName = 259;
    class TStackItem
        {
        public:
            TStackItem(const TDesC& aPath, const TUint32 aHandle):iPath(aPath), iHandle(aHandle)
                {
               
                }
        public:
            TBuf<KMTPMaxFullFileName> iPath;
            TUint32                   iHandle;
        };
private: 
	// Owned
	MMTPDataProviderFramework&  iFramework;
	CMTPFSExclusionMgr& 		iExclusionMgr;
	MMTPEnumerationCallback& 	iCallback;
	RMTPDpSingletons			iDpSingletons;
	TUint32 					iParentHandle;
	TBuf<KMTPMaxFullFileName>	iCurrentPath;
	RDir						iDir;
	TEntryArray					iEntries;
	TInt						iFirstUnprocessed;
	RArray<TStackItem>			iDirStack;
	RArray<TUint>				iStorages;
	TUint32						iStorageId;
	// How many entries should be processed in one go
	TInt 						iProcessLimit;
	CMTPObjectMetaData* 		iObject;
	TUint						iDpID;
	RMTPFramework               iSingletons;
	TBool                       iSkipCurrentStorage;
	TBool                       iScanAll;
	TBool                       iAllRootScaned;
	TUint                       iScanPos;
	TInt                        iObjectNeedToScan;

	

       
    };

#endif // MTPFILEENUMERATOR_H

