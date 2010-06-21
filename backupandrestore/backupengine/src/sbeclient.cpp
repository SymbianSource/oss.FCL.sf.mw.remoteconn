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
// Implementation of CSBEClient class.
// 
//

/**
 @file
 @released
*/

#include "sbeclient.h"
#include "sbeclientsession.h"
#include "sbepanic.h"

namespace conn
	{

	CSBEClient::CSBEClient()
	/** Class constructor. */
		{
		}


	EXPORT_C CSBEClient* CSBEClient::NewL()
	/**
	Standard creation method.

	@return a new CSBEClient.
	*/
		{
		CSBEClient* self = new (ELeave) CSBEClient();
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		return self;
		}

	void CSBEClient::ConstructL()
	/**
	Construct this instance of CSBEClient
	*/
		{
		iClientSession = RSBEClientSession::NewL();
		
		User::LeaveIfError(iClientSession->Connect());
		}


	EXPORT_C CSBEClient::~CSBEClient()
	/**
	Standard destructor
	*/
		{
		if (iClientSession)
			{
			iClientSession->Close();
			}
		
		delete iClientSession;
		iClientSession = NULL;
		}


	EXPORT_C void CSBEClient::ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners)
	/**
	Gets a list of all registered data owners.
	This method can be called regardless of backup or restore mode.
	N.B. If it is called before a restore the set may not include all data owners that 
	are to be restored.

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDataOwners on return an array of information about backup data owners.
	*/
		{
		iClientSession->ListOfDataOwnersL(aDataOwners);
		}

	EXPORT_C void CSBEClient::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles)
	/**
	Gets a list of actual public files associated with an SID for partial backup. This allows
	a client to backup only the public files associated with an SID.
	This method must only be called when the device is in backup mode as the set of files may
	not be final until then.  This method takes no account of any snapshots - the full list
	of candidate public files is returned and it is up to the Secure Backup Server to check
	file attributes if an incremental backup is required.

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDrive the drive concerned.
	@param aGenericDataType reference to the data type.
	@param aFiles on return an array of information about files.
	*/
		{
		iClientSession->PublicFileListL(aDrive, aGenericDataType, aFiles);
		}

	EXPORT_C void CSBEClient::RawPublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType,
									RRestoreFileFilterArray& aFileFilter)
	/**
	Gets a list of public files and directories associated with an SID for partial restore.
	This method must only be called when the device is in restore mode.
	This method must only be called after a registration file has been supplied and parsed.
	Directories can be distinguished from files by a terminating slash.

	During a restore operation, the client will have the archive or public files available but the
	Secure Backup Engine does not.  Therefore, the engine cannot list the exact files.  This method
	returns the list of files or directories that were specified in the backup registration file without
	further analysis and the client can use this information to filter public files for restore.

	This allows a client that has taken a full backup to be selective about restore .

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDrive the drive concerned.
	@param aGenericDataType reference to the data type.
	@param aFileFilter on return an array of names of files or directories for restore.
	*/
		{
		iClientSession->RawPublicFileListL(aDrive, aGenericDataType, aFileFilter);
		}

	EXPORT_C void CSBEClient::PublicFileListXMLL(TDriveNumber aDrive, TSecureId aSID, HBufC*& aFileList)
	/**
	Gets a list of public files and directories associated with an SID for partial backup or restore.
	This method returns the data in the form of an XML description for compatibility with older
	devices.

	This method can be called regardless of the backup or restore mode as it does not rely on the
	set of public files being final.

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDrive the drive concerned.
	@param aSID the data owner.
	@param aFileList on return a description of the list of files and directories. Must be NULL
	*/
		{
		__ASSERT_DEBUG(aFileList == NULL, Panic(KErrArgument));
		iClientSession->PublicFileListXMLL(aDrive, aSID, aFileList);
		}

	EXPORT_C void CSBEClient::SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
						  TBackupIncType aBackupIncType)
	/**
	Sets B&R on or off and sets the base / increment and full / partial mode and the affected drives

	@param aDriveList the drives affected.
	@param aBURType is a full or partial backup or restore desired or normal operation.
	@param aBackupIncType is a backup base or incremental (ignored for restore operations).
	*/
		{
		iClientSession->SetBURModeL(aDriveList, aBURType, aBackupIncType);
		}

	EXPORT_C void CSBEClient::SetSIDListForPartialBURL(RSIDArray& aSIDs)
	/**
	If a partial backup is required then this sets the list of data owners.
	This method must only be called when the device has just been put into backup or restore mode.
	It must only be called once for a backup or restore operation.

	@param aSIDs array of affected data owners.
	*/
		{
		iClientSession->SetSIDListForPartialBURL(aSIDs);
		}


	EXPORT_C void CSBEClient::SIDStatusL(RSIDStatusArray& aSIDStatus)
	/**
	Gets the status of a set of data owners.
	This method must only be called in backup or restore mode.
	
	You must populate the array with the TDataOwnerAndStatus's for each secure id the client 
	is interested in.
	
	@param aSIDStatus an array of structures for information about data owners. On return
	the status information is filled in.
	*/
		{
		iClientSession->SIDStatusL(aSIDStatus);
		}


	// Data Access Methods //

	EXPORT_C TPtr8& CSBEClient::TransferDataAddressL()
	/**
	Provides access to the base of the global chunk used to transfer data between
	the Secure Backup Engine and a Secure Backup Server.  This method should be used
	when the Secure Backup Server is providing data to the Secure Backup Engine (either as part 
	of a restore operation or when supplying snapshots during a backup operation.

	The Secure Backup Engine only uses one global chunk at a time. It is not permissible to
	try to carry out multiple backup or restore operations in parallel.  Normally a chunk
	of global heap would be protected by a mutex.  In this case, all the methods of the 
	Secure Backup Engine must be regarded as synchronous and mutually exclusive - it is not
	permissible to make parallel calls.

	The global chunk used during a backup or restore operation may change and so the address must
	be requested whenever required rather than being cached.

	@return Pointer to the start of the buffer for writing
	*/
		{
		return iClientSession->TransferDataAddressL();
		}
		
	EXPORT_C TPtrC8& CSBEClient::TransferDataInfoL(CSBGenericTransferType*& aGenericTransferType,
												   TBool& aFinished)
	/**
	Provides access to the data received from the Secure Backup Engine during a backup operation.

	This method should be called after a synchronous or asynchronous request for data has
	completed.

	@param aGenericTransferType  Reference to a CSBGenericTransferType pointer. Must be NULL
	@param aFinished ETrue if the data is the last chunk, EFalse if there is more to request.
	@return read only pointer to the global heap.
	*/
		{
		__ASSERT_DEBUG(aGenericTransferType == NULL, Panic(KErrArgument));
		return iClientSession->TransferDataInfoL(aGenericTransferType, aFinished);
		}

	EXPORT_C void CSBEClient::RequestDataL(CSBGenericTransferType& aGenericTransferType,
										   TRequestStatus& aStatus)
	/**
	This method requests package data from the Secure Backup Engine  asynchronously.
	When the relevant AO completes the returned data (and information about it) can be accessed
	by calling TransferDataInfoL().

	Some forms of backup data may be larger than can be handled in one transfer, in which case this
	method should be called again with the same arguments until all data is signalled as supplied.

	@param aGenericTransferType  Reference to a CSBGenericTransferType object
	@param aStatus the request status of an Active Object to be completed when the data has been received
	*/
		{
		iClientSession->RequestDataL(aGenericTransferType, aStatus);
		}
		
	EXPORT_C void CSBEClient::RequestDataL(CSBGenericTransferType& aGenericTransferType)
	/**
	This synchronous method requests backup data from the Secure Backup Engine.
	When the method returns the returned data (and information about it) can be accessed
	by calling TransferDataInfoL().

	Some forms of backup data may be larger than can be handled in one transfer, in which case this
	method should be called again with the same arguments until all data is signalled as supplied.

	@param aGenericTransferType  Reference to a CSBGenericTransferType object
	*/
		{
		iClientSession->RequestDataL(aGenericTransferType);
		}
	
	EXPORT_C void CSBEClient::SupplyDataL(CSBGenericTransferType& aGenericTransferType,
									 	  TBool aFinished, TRequestStatus& aStatus)
	/**
	This method supplies some form of package data during a backup or restore
	operation  asynchronously.

	When the relevant Active Object is completed the Secure Backup Engine has absorbed the
	supplied data.

	Some forms of data may be larger than can be handled in one transfer, in which case this
	method should be called again with the same arguments until all data has been supplied.

	@param aGenericTransferType  Reference to a CSBGenericTransferType object
	@param aFinished ETrue if this buffer is the last one for this package uid, drive and data type
	@param aStatus the request status of an Active Object to be completed when the data has been transferred
	*/
		{
		iClientSession->SupplyDataL(aGenericTransferType, aFinished, aStatus);
		}
		
	EXPORT_C void CSBEClient::SupplyDataL(CSBGenericTransferType& aGenericTransferType,
									 	  TBool aFinished)
	/**
	This synchronous method supplies some form of package data during a backup or
	restore operation.

	When the method returns the Secure Backup Engine has absorbed the supplied data.

	Some forms of data may be larger than can be handled in one transfer, in which case this
	method should be called again with the same arguments until all data has been supplied.

	@param aGenericTransferType  Reference to a CSBGenericTransferType object
	@param aFinished ETrue if this buffer is the last one for this package uid, drive and data type
	*/
		{
		iClientSession->SupplyDataL(aGenericTransferType, aFinished);
		}

	EXPORT_C void CSBEClient::AllSnapshotsSuppliedL()								  
	/**
	This method sets the type of backup to base for this particular data owner.
	The whole device may be subject to an incremental backup but a particular
	data owner may be subject to a base backup (if they have not been backed
	up previously). The reverse is not true.

	This method must only be called when the device has just been put into backup mode.
	It must only be called once for a SID for a backup operation.
	*/
		{
		iClientSession->AllSnapshotsSuppliedL();
		}
		
	EXPORT_C TUint CSBEClient::ExpectedDataSizeL(CSBGenericTransferType& aGenericTransferType)
	/**
	This method returns the expected size of package data that will be supplied.  The size
	data will be used for the purpose of tracking progess during a backup.  If it is
	inaccurate then the user may see irregular progress but the actual backup data will
	not be affected so it is acceptable to return an estimated value.

	This method must only be called during a backup operation.  If it is called when the 
	data owner is not ready for backup then the call will fail.

	@param aGenericTransferType  Reference to a CSBGenericTransferType object
	@return KErrNone if successful.
	*/
		{
		return (iClientSession->ExpectedDataSizeL(aGenericTransferType));
		}
										 
	EXPORT_C void CSBEClient::AllSystemFilesRestored()
	/**
	This method is called when all system files have been provided to allow the Secure
	Backup Engine to start active data owners.
	*/
		{
		iClientSession->AllSystemFilesRestored();
		}
	
	/**
	Gets a list of all registered data owners asynchronously.
	This method can be called regardless of backup or restore mode.
	N.B. If it is called before a restore the set may not include all data owners that 
	are to be restored.

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDataOwners on return an array of information about backup data owners.
	@param aStatus is TRequestStatus&
	*/	
	EXPORT_C void CSBEClient::ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners, TRequestStatus& aStatus)
		{
		iClientSession->ListOfDataOwnersL(aDataOwners, aStatus);
		}
	
	/**
	Gets a list of actual public files associated with an SID for partial backup asynchronously.
	This allows	a client to backup only the public files associated with an SID.
	This method must only be called when the device is in backup mode as the set of files may
	not be final until then.  This method takes no account of any snapshots - the full list
	of candidate public files is returned and it is up to the Secure Backup Server to check
	file attributes if an incremental backup is required.

	If the Secure Backup Engine has not parsed backup registration files then this call
	will trigger scanning and parsing.

	@param aDrive the drive concerned.
	@param aGenericDataType reference to the data type.
	@param aFiles on return an array of information about files.
	@param aStatus is TRequestStatus&
	*/	
	EXPORT_C void CSBEClient::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles, TRequestStatus& aStatus)
		{
		iClientSession->PublicFileListL(aDrive, aGenericDataType, aFiles, aStatus);
		}
	
	/**
	Sets B&R on or off and sets the base / increment and full / partial mode and the affected drives asynchronously.

	@param aDriveList the drives affected.
	@param aBURType is a full or partial backup or restore desired or normal operation.
	@param aBackupIncType is a backup base or incremental (ignored for restore operations).
	@param aStatus is TRequestStatus&
	*/	
	EXPORT_C void CSBEClient::SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
								  TBackupIncType aBackupIncType, TRequestStatus& aStatus)
		{
		iClientSession->SetBURModeL(aDriveList, aBURType, aBackupIncType, aStatus);
		}
	
	/**
	This method which sets the type of backup to base for this particular data owner asynchronously.
	The whole device may be subject to an incremental backup but a particular
	data owner may be subject to a base backup (if they have not been backed
	up previously). The reverse is not true.

	This method must only be called when the device has just been put into backup mode.
	It must only be called once for a SID for a backup operation.
	
	@param aStatus is TRequestStatus&
	*/
	EXPORT_C void CSBEClient::AllSnapshotsSuppliedL(TRequestStatus& aStatus)
		{
		iClientSession->AllSnapshotsSuppliedL(aStatus);
		}
		
	/**
	This method is called asynchronously when all system files have been provided to allow the Secure
	Backup Engine to start active data owners.
	
	@param aStatus is TRequestStatus&
	*/
	EXPORT_C void CSBEClient::AllSystemFilesRestoredL(TRequestStatus& aStatus)
		{
		iClientSession->AllSystemFilesRestoredL(aStatus);
		}

	// Test Methods //

	EXPORT_C TUint CSBEClient::DataChecksum(TDriveNumber /*aDrive*/, TSecureId /*aSID*/)
	/**
	Get a 32-bit checksum for private data.
	This routine is for test purposes.  It must be implemented but an invariant checksum 
	value can be provided.  Some tests may cause checksum values to be compared.

	@param aDrive the drive containing data being checksummed
	@param aSID the data owner.
	@return the 32-bit checksum
	*/
		{
		return 0;
		}
		
	EXPORT_C void CSBEClient::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, 
								RPointerArray<CSBEFileEntry>& aFileList, TBool& aFinished,
								TInt aTotalListCursor, TInt aMaxResponseSize, TRequestStatus& aStatus)
	/** This asynchronous method is used to retrieve the list of public files for the specified data owner
	on the specified drive. Upon completion of aStatus, the caller should check aFileList 
	@param aDrive The drive that contains the public files being retrieved
	@param aGenericDataType The identifier for the data owner that owns the public files
	@param aFileList Upon completion of aStatus, this array will contain the list of public files returned
	@param aFinished Upon completion of aStatus, this flag will be set to indicate that there are more 
						file entries available for this data owner and another call to this method should be made
	@param aTotalListCursor Specifies the index into the complete list of public files for this data owner to start 
						the next chunk of file entries from. The number of entries returned by a call to this
						method can be determined by querying the count of aFileList
	@param aMaxResponseSize The maximum total size in bytes of externalised CSBEFileEntry objects that will be returned
	@param aStatus The TRequestStatus that will be completed once the engine has fully processed this request
	*/
		{
		iClientSession->PublicFileListL(aDrive, aGenericDataType, aFileList, aFinished, aTotalListCursor, aMaxResponseSize, aStatus);
		}

	} // end of conn namespace


