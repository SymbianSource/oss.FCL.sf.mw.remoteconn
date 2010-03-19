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

#include <f32file.h>
#include <bautils.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptypeevent.h>
#include "cmtpfsexclusionmgr.h"
#include "cmtpfsenumerator.h"
#include "mmtpenumerationcallback.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "mtpframeworkconst.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"FSEnumerator");)

const TUint KMTPMaxFullFileName = 259;
/*
 * 
 */
#define KMAX_FILECOUNT_ENUMERATINGPHASE1 5000

/**
 * the files should not be owned by any dp. 
 */
#define FILES_OWNED_BY_NONE             0

/**
 * the missed files of other dps should be owned by file dp
 */
#define MISSED_FILES_OWNED_BY_FILE_DP   1

/**
 * the missed files of other dps should be owned by counterparter dps
 */
#define MISSED_FILES_OWNED_BY_OTHER_DP  2

/**
 * the files of other dps should be owned by counterparter dps
 */
#define FILES_OWNED_BY_OTHER_DP         3

/**
Two-phase construction
@param aFramework    data provider framework of data provider
@param aObjectMgr    the reference to the object manager
@param aExclusionMgr    the reference to the exclusion manager 
@param aCallback callback to be called when enumeration is complete
*/
EXPORT_C CMTPFSEnumerator* CMTPFSEnumerator::NewL(MMTPDataProviderFramework& aFramework, CMTPFSExclusionMgr& aExclusionMgr, MMTPEnumerationCallback& aCallback, TInt aProcessLimit)
    {
    CMTPFSEnumerator* self = new (ELeave) CMTPFSEnumerator(aFramework, aExclusionMgr, aCallback, aProcessLimit);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
destructor
*/    
EXPORT_C CMTPFSEnumerator::~CMTPFSEnumerator()
	{
	Cancel();	
	iDir.Close();
	iDirStack.Close();
	iStorages.Close();
	iDpSingletons.Close();
	iSingletons.Close();
	delete iObject;
	__FLOG_CLOSE; 
	}

/**
Kick off the enumeration on the specified storage
@param aStorageId storage to be enumerated
*/
EXPORT_C void CMTPFSEnumerator::StartL(TUint32 aStorageId, TBool aOnlyRoot)
	{
	__ASSERT_DEBUG(!IsActive(), User::Invariant());
	iNumOfFoldersAndFiles = 0;
	iOnlyScanRoot = aOnlyRoot;
	__FLOG_VA((_L8("iOnlyScanRoot == %d "), iOnlyScanRoot));
	
	MMTPStorageMgr& storageMgr(iFramework.StorageMgr());
	if (aStorageId == KMTPStorageAll)
	    {
        // Retrieve the available logical StorageIDs
        RPointerArray<const CMTPStorageMetaData> storages;
        CleanupClosePushL(storages);
        TMTPStorageMgrQueryParams params(KNullDesC, CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
        storageMgr.GetLogicalStoragesL(params, storages);
        
        // Construct the StorageIDs list.
        const TUint KCount(storages.Count());
        for (TUint i(0); (i < KCount); i++)
            {
            iStorages.Insert(storages[i]->Uint(CMTPStorageMetaData::EStorageId),0);
            __FLOG_VA((_L8("FileEnumerator is doing storage id = %x\r\n"), storages[i]->Uint(CMTPStorageMetaData::EStorageId) ));
            }
        CleanupStack::PopAndDestroy(&storages);
	    }
    else if (aStorageId != KMTPNotSpecified32)
        {
		__ASSERT_DEBUG(storageMgr.ValidStorageId(aStorageId), User::Invariant());
		const CMTPStorageMetaData& storage(storageMgr.StorageL(aStorageId));
		if (storage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
		    {
    	    if (storageMgr.LogicalStorageId(aStorageId))
    		    {
    		    // Logical StorageID.
    			iStorages.AppendL(aStorageId);
    		    }
    		else
    		    {
    		    // Physical StorageID. Enumerate all eligible logical storages.
    		    const RArray<TUint>& logicalIds(storage.UintArray(CMTPStorageMetaData::EStorageLogicalIds));
    		    const TUint KCountLogicalIds(logicalIds.Count());
                for (TUint i(0); (i < KCountLogicalIds); i++)
                    {
                    iStorages.AppendL(logicalIds[i]);
                    }
    		    }
		    }
		}

	iStorageId = aStorageId;
	iSkipCurrentStorage = EFalse;
	
	if (iStorages.Count() > 0)
		{
		 TRAPD(err, ScanStorageL(iStorages[0]));        
		 if(err != KErrNone)
			 {
			 if( !storageMgr.ValidStorageId(iStorages[0]) )
				 {
				 //Scan storage leave because storage(memory card) removed.
				 //Scan next specified storage in RunL, if there is.
				 __FLOG_VA(_L8("StartL - iSkipCurrentStorage - ETrue."));
				 iSkipCurrentStorage = ETrue;
				TRequestStatus* status = &iStatus;
				User::RequestComplete(status, iStatus.Int());
				 SetActive();
				 }
			 else
				 {
				 User::Leave(err);
				 }
			 }
		}
	else
		{
		if((!iIsFileEnumerator) &&(iNumOfFoldersAndFiles > KMAX_FILECOUNT_ENUMERATINGPHASE1))
			{
			iSingletons.DpController().SetNeedEnumeratingPhase2(ETrue);
			}
		
		iCallback.NotifyEnumerationCompleteL(iStorageId, KErrNone);
		
		TMTPTypeEvent event;
		
		event.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeUnreportedStatus);
		event.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
		
		iFramework.SendEventL(event);
		}
	}
	
/**
Cancel the enumeration process
*/    
void CMTPFSEnumerator::DoCancel()
	{
	iDir.Close();
	}

void CMTPFSEnumerator::ScanStorageL(TUint32 aStorageId)
	{
	__FLOG_VA(_L8("ScanStorageL - entry"));
    const CMTPStorageMetaData& storage(iFramework.StorageMgr().StorageL(aStorageId));
    __ASSERT_DEBUG((storage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem), User::Invariant());
    TFileName root(storage.DesC(CMTPStorageMetaData::EStorageSuid));
    
    #ifdef __FLOG_ACTIVE    
	TBuf8<KMaxFileName> tmp;
	tmp.Copy(root);
 	__FLOG_VA((_L8("StorageSuid - %S"), &tmp));	
	#endif // __FLOG_ACTIVE
 	
 	if ( iExclusionMgr.IsFolderAcceptedL(root, aStorageId) )
 	    {
 	    iParentHandle = KMTPHandleNoParent;
 	    iPath.Set(root, NULL, NULL);
 	    User::LeaveIfError(iDir.Open(iFramework.Fs(), iPath.DriveAndPath(), KEntryAttNormal | KEntryAttHidden | KEntryAttDir));
 	    ScanDirL();
 	    }
 	else
 	    {
 	    TRequestStatus* status = &iStatus;
 	    User::RequestComplete(status, iStatus.Int());
 	    SetActive();
 	    }
 	__FLOG_VA(_L8("ScanStorageL - exit"));
	}

/**
Scans directory at aPath recursing into subdirectories on a depth first basis.

Directory entries are kept in iDirStack - which is a LIFO stack.
The current path, needed since TEntries don't keep track of it, 
is kept in iPath.

The algorithm works as follows:

1. Read directory entries.
2. ProcessEntriesL is called if no error occurs and >= 1 entries are read.
3. ProcessEntriesL adds entries to database, if entry is directory add to iDirStack.
4. When all entries are processed pop entry off the dirstack, 
	if entry is empty TEntry remove one directory from iPath.
5. Append entry name onto iPath - to update path with new depth (parent/subdir).
6. Push an empty TEntry onto iDirStack - this tells us we have recursed one,
	think of it as pushing the '\' separator onto iDirStack.
7. Repeat 1-7 until iDirStack is empty.
*/

void CMTPFSEnumerator::ScanDirL()
	{
	__FLOG_VA(_L8("ScanDirL - entry"));
	iFirstUnprocessed = 0;
	iDir.Read(iEntries, iStatus);
	SetActive();
	__FLOG_VA(_L8("ScanDirL - exit"));
	}

void CMTPFSEnumerator::ScanNextStorageL()
	{
	__FLOG_VA(_L8("ScanNextStorageL - entry"));
	// If there are one or more unscanned storages left
	// (the currently scanned one is still on the list)
	if (iStorages.Count() > 1)
		{
		iStorages.Remove(0);
		ScanStorageL(iStorages[0]);
		}
	else
		{
		// We are done
		iStorages.Reset();
		if((!iIsFileEnumerator) &&(iNumOfFoldersAndFiles > KMAX_FILECOUNT_ENUMERATINGPHASE1))
			{
			iSingletons.DpController().SetNeedEnumeratingPhase2(ETrue);
			}
		iCallback.NotifyEnumerationCompleteL(iStorageId, KErrNone);
		
		}
	__FLOG_VA(_L8("ScanNextStorageL - exit"));
	}

void CMTPFSEnumerator::ScanNextSubdirL()
	{
	__FLOG_VA(_L8("ScanNextSubdirL - entry"));
	// A empty (non-constructed) TEntry is our marker telling us to pop a directory 
	// from iPath when we see this
	iDirStack.AppendL(TEntry());
			
	// Leave with KErrNotFound if we don't find the object handle since it shouldn't be on the 
	// dirstack if the entry wasn't added
	TPtrC suid = iPath.DriveAndPath().Left(iPath.DriveAndPath().Length());
	// Update the current parentId with object of the directory
	iParentHandle = iFramework.ObjectMgr().HandleL(suid);
				
	// Kick-off a scan of the next directory
	iDir.Close();
	User::LeaveIfError(iDir.Open(iFramework.Fs(), iPath.DriveAndPath(), KEntryAttNormal | KEntryAttHidden | KEntryAttDir));
	ScanDirL();
	__FLOG_VA(_L8("ScanNextSubdirL - exit"));
	}

/**
Recurse into the next directory on the stack
and scan it for entries.
*/

void CMTPFSEnumerator::ScanNextL()
	{
	__FLOG_VA(_L8("ScanNextL - entry"));
	TInt count = iDirStack.Count();
	
	if ((count == 0) || iOnlyScanRoot )
		{
		// No more directories on the stack, try the next storage
		ScanNextStorageL();
		}
	else
		{
		TEntry& entry = iDirStack[count - 1];
		
		// Empty TEntry, no more subdirectories in
		// the current path
		if (entry.iName == KNullDesC)
			{
			// Remove current dir from path
			iPath.PopDir();
			iDirStack.Remove(count - 1);
			iDir.Close();
			
			// Scan the next directory of the parent
			ScanNextL();
			}
			
		// Going into a subdirectory of current
		else 
			{
			// Add directory to path		
			iPath.AddDir(entry.iName);
			// Remove directory so we don't think it's a subdirectory
			iDirStack.Remove(count - 1);
	
			ScanNextSubdirL();
			}
		}
	__FLOG_VA(_L8("ScanNextL - exit"));
	}

void CMTPFSEnumerator::RunL()
	{
	__FLOG_VA(_L8("RunL - entry"));
	if(iSkipCurrentStorage)
		{
		__FLOG_VA(_L8("RunL - iSkipCurrentStorage - ETrue."));
		iSkipCurrentStorage = EFalse;
		ScanNextStorageL();
		}
	else if (iEntries.Count() == 0)
		{
		// No entries to process, scan next dir or storage
		ScanNextL();
		}
	else if (iFirstUnprocessed < iEntries.Count())
		{
		ProcessEntriesL();
		
		// Complete ourselves with current TRequestStatus
		// since we need to run again to either scan a new dir or drive
		// or process more entries
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, iStatus.Int());
		SetActive();
		}
	else
		{
		switch (iStatus.Int())
			{
			case KErrNone:
				// There are still entries left to be read
				ScanDirL();
				break;
			
			case KErrEof:
				// There are no more entries
			default:
				// Error, ignore and continue with next dir
				ScanNextL();
				break;
			}
		}
	__FLOG_VA(_L8("RunL - exit"));
	}

/**
Ignore the error, continue with the next one
*/    
TInt CMTPFSEnumerator::RunError(TInt aError)
	{
	__FLOG_VA((_L8("RunError - entry with error %d"), aError));
	 if(!iFramework.StorageMgr().ValidStorageId(iStorages[0]))
		 {
		 __FLOG_VA((_L8("Invalid StorageID = %d"),iStorages[0] ));
		 if (iStorages.Count()>1)
			 {
			 //Not necessary to process any entry on the storage, since the storage removed.
			 //Then need to start from root dir of next storage if there is.
			 //So, the dir stack is popped to bottom.
			 iDirStack.Reset();
			 }
		 iSkipCurrentStorage = ETrue;
		 }
	
	// Reschedule ourselves
	TRequestStatus* status = &iStatus;
	User::RequestComplete(status, aError);
	SetActive();
	
	__FLOG(_L8("RunError - Exit"));
	return KErrNone;
	}
	
/**
Standard c++ constructor
*/	
CMTPFSEnumerator::CMTPFSEnumerator(MMTPDataProviderFramework& aFramework, CMTPFSExclusionMgr& aExclusionMgr, MMTPEnumerationCallback& aCallback, TInt aProcessLimit)
	: CActive(EPriorityLow),
  	iFramework(aFramework),
  	iExclusionMgr(aExclusionMgr),
    iCallback(aCallback),
    iProcessLimit(aProcessLimit)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	CActiveScheduler::Add(this);
	}

void CMTPFSEnumerator::ConstructL()
	{
	iSingletons.OpenL();
	iDpSingletons.OpenL(iFramework);
	iObject = CMTPObjectMetaData::NewL();	
	iDpID = iFramework.DataProviderId();
	iIsFileEnumerator = (KMTPFileDPID == iDpID);
	}

/**
Iterates iEntries adding entries as needed to object manager and iDirStack.
*/

void CMTPFSEnumerator::ProcessEntriesL()
	{
	TBuf<KMTPMaxFullFileName> path = iPath.DriveAndPath();
	
	// Start looping through entries at where we left off
	TInt count = iEntries.Count() - iFirstUnprocessed;
	// Process no more than KProcessLimit entries
	count = Min(count, iProcessLimit);
	iFirstUnprocessed += count;		
	
	if(!iIsFileEnumerator)
		{
		iNumOfFoldersAndFiles +=count;
		}	
	
	for (TInt i = (iFirstUnprocessed - count); i < iFirstUnprocessed; ++i)
		{
		const TEntry& entry = iEntries[i];
		path.Append(entry.iName);
		
#ifdef __FLOG_ACTIVE    
		TBuf8<KMTPMaxFullFileName> tmp;
        tmp.Copy(path);
        TInt pathLen=path.Length();
        if(pathLen > KLogBufferSize)
            {
            TBuf8<KLogBufferSize> tmp1;
            tmp1.Copy(tmp.Ptr(),KLogBufferSize);
			__FLOG_VA(_L8("Entry - "));
	        __FLOG_VA((_L8("%S"), &tmp1));

	        tmp1.Copy(tmp.Ptr()+KLogBufferSize, pathLen-KLogBufferSize);
	        __FLOG_VA((_L8("%S"), &tmp1));
            }
        else
            {
            __FLOG_VA(_L8("Entry - "));
			__FLOG_VA((_L8("%S"), &tmp));
            }
#endif // __FLOG_ACTIVE
		
		TInt len = entry.iName.Length();
		TInt totalLen = path.Length();
		if(totalLen > KMaxFileName)
		    {
			// Remove filename part
		    path.SetLength(totalLen - len);
		    __FLOG_VA(_L8("Full name exceeds KMaxFileName, ignored."));
		    continue;
		    }
		TUint32 handle = 0;
		TMTPFormatCode format;
		  TParsePtrC parse(path);
		if (entry.IsDir())
			{
			if (iExclusionMgr.IsFolderAcceptedL(path, iStorages[0]))
				{
				path.Append('\\');
				++len;
				format = EMTPFormatCodeAssociation;
				AddEntryL(path, handle, format, iDpID, entry, iStorages[0], iParentHandle);
				iDirStack.AppendL(entry);
				}
			}
		else if ( iExclusionMgr.IsFileAcceptedL(path,iStorages[0]) )
			{
			format = EMTPFormatCodeUndefined;
			AddEntryL(path, handle, format, iDpID, entry, iStorages[0], iParentHandle);
			}
		else if ( parse.ExtPresent() )
		    {
		    switch(iDpSingletons.MTPUtility().GetEnumerationFlag(parse.Ext().Mid(1)))
		        {
            case MISSED_FILES_OWNED_BY_FILE_DP:
                if (KMTPHandleNone == iFramework.ObjectMgr().HandleL(path))
                    {
                    format = EMTPFormatCodeUndefined;
                    AddEntryL(path, handle, format, iDpID, entry, iStorages[0], iParentHandle);		   
                    }
                break;
                
            case MISSED_FILES_OWNED_BY_OTHER_DP:
                if (KMTPHandleNone == iFramework.ObjectMgr().HandleL(path))
                    {
                    format = iDpSingletons.MTPUtility().GetFormatByExtension(parse.Ext().Mid(1));  
                    TUint32 DpId = iDpSingletons.MTPUtility().GetDpId(parse.Ext().Mid(1), KNullDesC);
                    AddFileEntryForOtherDpL(path, handle, format, DpId, entry, iStorages[0], iParentHandle);
                    }
                break;
                
            case FILES_OWNED_BY_OTHER_DP:
                {
                format = iDpSingletons.MTPUtility().GetFormatByExtension(parse.Ext().Mid(1));  
                TUint32 DpId = iDpSingletons.MTPUtility().GetDpId(parse.Ext().Mid(1), KNullDesC);
                AddFileEntryForOtherDpL(path, handle, format, DpId, entry, iStorages[0], iParentHandle);
                }
                break;
                
//          case FILES_OWNED_BY_NONE:
            default:
                //nothing to do
                break;
		        }    
		    }
		// Remove filename part					
		path.SetLength(path.Length() - len);
		}
		
	}

/**
Add a file entry to the object store
@param aEntry    The file Entry to be added
@param aPath    The full path name of the entry
@return MTP object handle, or KMTPHandleNone if entry was not accepted
*/    
void CMTPFSEnumerator::AddEntryL(const TDesC& aPath, TUint32 &aHandle, TMTPFormatCode format, TUint32 aDPId, const TEntry& aEntry, TUint32 aStorageId, TUint32 aParentHandle)
	{
#ifdef __FLOG_ACTIVE    
	TBuf8<KMaxFileName> tmp;
	tmp.Copy(aPath);
	
	__FLOG_VA((_L8("AddEntryL - entry: %S"), &tmp));
#endif // __FLOG_ACTIVE

    TUint16 assoc;
    TPtrC name;
    if (format == EMTPFormatCodeAssociation)
        {
        assoc = EMTPAssociationTypeGenericFolder;
        TParsePtrC pathParser(aPath.Left(aPath.Length() - 1)); // Ignore the trailing "\".
		name.Set(aEntry.iName);
        }
    else
        {
        assoc = EMTPAssociationTypeUndefined;
        TParsePtrC pathParser(aPath);
		name.Set(pathParser.Name());	
        }
    
    if(iExclusionMgr.IsFormatValid(format))
        {
        aHandle = KMTPHandleNone;
        
        iObject->SetUint(CMTPObjectMetaData::EDataProviderId, aDPId);
        iObject->SetUint(CMTPObjectMetaData::EFormatCode, format);
        iObject->SetUint(CMTPObjectMetaData::EStorageId, aStorageId);
        iObject->SetDesCL(CMTPObjectMetaData::ESuid, aPath);
        iObject->SetUint(CMTPObjectMetaData::EFormatSubCode, assoc);
        iObject->SetUint(CMTPObjectMetaData::EParentHandle, aParentHandle);
        iObject->SetUint(CMTPObjectMetaData::ENonConsumable, EMTPConsumable);
        iObject->SetDesCL(CMTPObjectMetaData::EName, name);
        iFramework.ObjectMgr().InsertObjectL(*iObject);
        }
	__FLOG_VA(_L8("AddEntryL - exit"));	
	}

void CMTPFSEnumerator::AddFileEntryForOtherDpL(const TDesC& aPath, TUint32 &aHandle, TMTPFormatCode format, TUint32 aDPId, const TEntry& /*aEntry*/, TUint32 aStorageId, TUint32 aParentHandle)
    {
#ifdef __FLOG_ACTIVE    
    TBuf8<KMaxFileName> tmp;
    tmp.Copy(aPath);
    
    __FLOG_VA((_L8("AddFileEntryForOtherDpL - entry: %S"), &tmp));
#endif // __FLOG_ACTIVE

    TUint16 assoc = EMTPAssociationTypeUndefined;
    TParsePtrC pathParser(aPath);
    TPtrC name(pathParser.Name());    
    
    aHandle = KMTPHandleNone;
    
    iObject->SetUint(CMTPObjectMetaData::EDataProviderId, aDPId);
    iObject->SetUint(CMTPObjectMetaData::EFormatCode, format);
    iObject->SetUint(CMTPObjectMetaData::EStorageId, aStorageId);
    iObject->SetDesCL(CMTPObjectMetaData::ESuid, aPath);
    iObject->SetUint(CMTPObjectMetaData::EFormatSubCode, assoc);
    iObject->SetUint(CMTPObjectMetaData::EParentHandle, aParentHandle);
    iObject->SetUint(CMTPObjectMetaData::ENonConsumable, EMTPConsumable);
    iObject->SetDesCL(CMTPObjectMetaData::EName, name);
    iFramework.ObjectMgr().InsertObjectL(*iObject);
    __FLOG_VA(_L8("AddEntryL - exit")); 
    }

void CMTPFSEnumerator::NotifyObjectAddToDP(const TUint32 aHandle,const TUint DpId)
    {
    iSingletons.DpController().NotifyDataProvidersL(DpId,EMTPObjectAdded,(TAny*)&aHandle);
    }


