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

/*
 * 
 */
#define KMAX_FILECOUNT_ENUMERATINGPHASE1 1

#define KMAX_FILECOUNT_ENUMERATINGPHASE2 0x7FFFFFFF

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
EXPORT_C void CMTPFSEnumerator::StartL(TUint32 aStorageId, TBool aScanAll)
    {
    __ASSERT_DEBUG(!IsActive(), User::Invariant());
    iScanAll = aScanAll;
    iAllRootScaned = EFalse;
    iStorages.Reset();
    if(iScanAll)
        {
        iObjectNeedToScan = KMAX_FILECOUNT_ENUMERATINGPHASE2;
        }
    else
        {
        iObjectNeedToScan = KMAX_FILECOUNT_ENUMERATINGPHASE1;
        }
    __FLOG_VA((_L8("iScanAll %d files %d Storage 0x%x"), iScanAll, iObjectNeedToScan, aStorageId));
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
            iStorages.InsertL(storages[i]->Uint(CMTPStorageMetaData::EStorageId),0);
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
        iScanPos = 0;
        TRAPD(err, ScanStorageL(iStorages[iScanPos]));        
        if(err != KErrNone)
            {
            if( !storageMgr.ValidStorageId(iStorages[iScanPos]) )
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
        iCurrentPath = root;
        User::LeaveIfError(iDir.Open(iFramework.Fs(), iCurrentPath, KEntryAttNormal | KEntryAttHidden | KEntryAttDir));
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
    iDirStack.Reset();
    __FLOG_VA(_L8("ScanNextStorageL - entry"));
    // If there are one or more unscanned storages left
    // (the currently scanned one is still on the list)
    if (++ iScanPos < iStorages.Count())
        {
        ScanStorageL(iStorages[iScanPos]);
        }
    else
        {
        // We are done
        iScanPos = 0;
        
        if(iScanAll) //all object scaned or first time limit reached.
            {
            iStorages.Reset();
            if(iObjectNeedToScan <= 0)
                {
                iSingletons.DpController().SetNeedEnumeratingPhase2(ETrue);
                }
            iDir.Close();
            iCallback.NotifyEnumerationCompleteL(iStorageId, KErrNone);
            }
        iAllRootScaned = ETrue;
        if(!iScanAll)
            {
            if(iObjectNeedToScan > 0)//partial scan didn't finish means root didn't have KMAX_FILECOUNT_ENUMERATINGPHASE1 files
                {
                iScanAll = ETrue;
                iObjectNeedToScan = KMAX_FILECOUNT_ENUMERATINGPHASE1;
                ScanStorageL(iStorages[iScanPos]);
                }
            else //root has more than KMAX_FILECOUNT_ENUMERATINGPHASE1 files
                {
                iDir.Close();
                iStorages.Reset();
                iSingletons.DpController().SetNeedEnumeratingPhase2(ETrue);
                iCallback.NotifyEnumerationCompleteL(iStorageId, KErrNone);
                }
            }
        }
    __FLOG_VA(_L8("ScanNextStorageL - exit"));
    }

void CMTPFSEnumerator::ScanNextSubdirL()
	{
	__FLOG_VA(_L8("ScanNextSubdirL - entry"));
	// A empty (non-constructed) TEntry is our marker telling us to pop a directory 
	// from iPath when we see this
	//iDirStack.AppendL(TEntry());
			
	// Leave with KErrNotFound if we don't find the object handle since it shouldn't be on the 
	// dirstack if the entry wasn't added
	//TPtrC suid = iPath.DriveAndPath().Left(iPath.DriveAndPath().Length());
	// Update the current parentId with object of the directory
	iParentHandle = iDirStack[iDirStack.Count() - 1].iHandle;//iFramework.ObjectMgr().HandleL(suid);
	iCurrentPath = iDirStack[iDirStack.Count() - 1].iPath;
	iDirStack.Remove(iDirStack.Count() - 1);
	__FLOG_VA((_L8("ScanNextSubdirL path %S"), &iCurrentPath));		
	// Kick-off a scan of the next directory
	iDir.Close();
	User::LeaveIfError(iDir.Open(iFramework.Fs(), iCurrentPath, KEntryAttNormal | KEntryAttHidden | KEntryAttDir));    
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
	
	if ((count == 0) || !iScanAll)
		{
		// No more directories on the stack, try the next storage
		ScanNextStorageL();
		}
	else
		{
		// Remove directory so we don't think it's a subdirectory
		ScanNextSubdirL();
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
	 if(!iFramework.StorageMgr().ValidStorageId(iStorages[iScanPos]))
		 {
		 __FLOG_VA((_L8("Invalid StorageID = %d"),iStorages[iScanPos] ));
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
	}

/**
Iterates iEntries adding entries as needed to object manager and iDirStack.
*/

void CMTPFSEnumerator::ProcessEntriesL()
    {
    // Start looping through entries at where we left off
    TInt count = iEntries.Count() - iFirstUnprocessed;
    // Process no more than KProcessLimit entries
    count = Min(count, iProcessLimit);
    iFirstUnprocessed += count;		

    for (TInt i = (iFirstUnprocessed - count); i < iFirstUnprocessed; ++i)
        {
        const TEntry& entry = iEntries[i];
        TInt len = entry.iName.Length();
        if(iCurrentPath.Length()  + len > KMaxFileName)
            {
            __FLOG_VA(_L8("Full name exceeds KMaxFileName, ignored."));
            continue;
            }
        
        iCurrentPath.Append(entry.iName);

        __FLOG_VA((_L("Process path %S name %S"), &iCurrentPath, &entry.iName));
#ifdef __FLOG_ACTIVE    
        TBuf8<KMTPMaxFullFileName> tmp;
        tmp.Copy(iCurrentPath);
        TInt pathLen=iCurrentPath.Length();
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

        TUint32 handle = 0;
        TMTPFormatCode format;
        if(-- iObjectNeedToScan <=0 && iAllRootScaned)
            {
            iSkipCurrentStorage = ETrue;
            return;
            }
        if (entry.IsDir())
            {
            if (iExclusionMgr.IsFolderAcceptedL(iCurrentPath, iStorages[iScanPos]))
                {
                iCurrentPath.Append('\\');
                ++len;
                format = EMTPFormatCodeAssociation;
                AddEntryL(iCurrentPath, handle, format, KMTPDeviceDPID, entry, iStorages[iScanPos], iParentHandle);
                iDirStack.AppendL(TStackItem(iCurrentPath, handle));
                }
            }
        else
            {
            if ( iExclusionMgr.IsFileAcceptedL(iCurrentPath,iStorages[iScanPos]) )
                {
                format = EMTPFormatCodeUndefined;
                AddEntryL(iCurrentPath, handle, format, iDpID, entry, iStorages[iScanPos], iParentHandle);
                }
            else 
                {
                    TParsePtrC parse(iCurrentPath);
                    if ( parse.ExtPresent() )
                    {
                    switch(iDpSingletons.MTPUtility().GetEnumerationFlag(parse.Ext().Mid(1)))
                        {
                    case MISSED_FILES_OWNED_BY_FILE_DP:
                        if (KMTPHandleNone == iFramework.ObjectMgr().HandleL(iCurrentPath))
                            {
                            format = EMTPFormatCodeUndefined;
                            AddEntryL(iCurrentPath, handle, format, iDpID, entry, iStorages[iScanPos], iParentHandle);		   
                            }
                        break;
                        
                    case MISSED_FILES_OWNED_BY_OTHER_DP:
                        if (KMTPHandleNone == iFramework.ObjectMgr().HandleL(iCurrentPath))
                            {
                            format = iDpSingletons.MTPUtility().GetFormatByExtension(parse.Ext().Mid(1));  
                            TUint32 DpId = iDpSingletons.MTPUtility().GetDpIdL(parse.Ext().Mid(1), KNullDesC);
                            AddFileEntryForOtherDpL(iCurrentPath, handle, format, DpId, entry, iStorages[iScanPos], iParentHandle);
                            }
                        break;
                        
                    case FILES_OWNED_BY_OTHER_DP:
                        {
                        _LIT( KTxtExtensionODF, ".odf" );
                        TUint32 DpId = iFramework.DataProviderId();
                        if (parse.Ext().CompareF(KTxtExtensionODF)==0)
                            {
                            HBufC* mime = iDpSingletons.MTPUtility().ContainerMimeType(iCurrentPath);
                            CleanupStack::PushL(mime);
                            if ( mime != NULL )
                                {
                                __FLOG_VA((_L("mime %S"), mime));
                                DpId = iDpSingletons.MTPUtility().GetDpIdL(parse.Ext().Mid(1),*mime);
                                __FLOG_VA((_L("DpId find %d"), DpId));
                                }
                            format = iDpSingletons.MTPUtility().GetFormatCodeByMimeTypeL(parse.Ext().Mid(1),*mime);
                            AddFileEntryForOtherDpL(iCurrentPath, handle, format, DpId, entry, iStorages[iScanPos], 
                                    iParentHandle, iDpSingletons.MTPUtility().GetSubFormatCodeL(parse.Ext().Mid(1),*mime));
                            CleanupStack::PopAndDestroy(mime);
                            }
                        else
                            {
                            format = iDpSingletons.MTPUtility().GetFormatByExtension(parse.Ext().Mid(1));  
                            TUint32 DpId = iDpSingletons.MTPUtility().GetDpIdL(parse.Ext().Mid(1), KNullDesC);
                            AddFileEntryForOtherDpL(iCurrentPath, handle, format, DpId, entry, iStorages[iScanPos], iParentHandle);
                            }
                        }
                        break;
                        
        //          case FILES_OWNED_BY_NONE:
                    default:
                        //nothing to do
                        break;
                        }    
                    }
                }
            }
        // Remove filename part					
        iCurrentPath.SetLength(iCurrentPath.Length() - len);
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
    
    //if(iExclusionMgr.IsFormatValid(format))
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
        aHandle = iObject->Uint(CMTPObjectMetaData::EHandle);
        
        }
	__FLOG_VA(_L8("AddEntryL - exit"));	
	}

void CMTPFSEnumerator::AddFileEntryForOtherDpL(const TDesC& aPath, TUint32 &aHandle, TMTPFormatCode format, TUint32 aDPId, const TEntry& /*aEntry*/, TUint32 aStorageId, TUint32 aParentHandle, TUint16 aSubFormatCode/* = 0*/)
    {
#ifdef __FLOG_ACTIVE    
    TBuf8<KMaxFileName> tmp;
    tmp.Copy(aPath);
    
    __FLOG_VA((_L8("AddFileEntryForOtherDpL - entry: %S"), &tmp));
#endif // __FLOG_ACTIVE

    TParsePtrC pathParser(aPath);
    TPtrC name(pathParser.Name());    
    
    aHandle = KMTPHandleNone;
    
    iObject->SetUint(CMTPObjectMetaData::EDataProviderId, aDPId);
    iObject->SetUint(CMTPObjectMetaData::EFormatCode, format);
    iObject->SetUint(CMTPObjectMetaData::EStorageId, aStorageId);
    iObject->SetDesCL(CMTPObjectMetaData::ESuid, aPath);
    iObject->SetUint(CMTPObjectMetaData::EFormatSubCode, aSubFormatCode);
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


