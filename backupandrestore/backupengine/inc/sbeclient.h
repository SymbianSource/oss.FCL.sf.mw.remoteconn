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
* Declaration of CSBEClient
* 
*
*/



/**
 @file
 @released
*/

#ifndef __SBECLIENT_H__
#define __SBECLIENT_H__

#include <e32base.h>
#include <e32cmn.h>
#include <connect/sbtypes.h>

namespace conn
	{ 
	class RSBEClientSession;

	class CSBEClient : public CBase
		/** The client to the Secure Backup Engine.
		This class is for use by a Secure Backup Server.  It provides backup data and receives
		restore data.  It encapsulates some state information as backup or restore operations require
		a range of actions to be undertaken in the correct order.

		It is loaded into the same process as the Secure Backup Server - this is the client interface
		of the server that is the Secure Backup Engine.

		Methods that transfer large amounts of data are asynchronous (to allow the Secure Backup Engine
		to carry out asynchronous operations) but synchronous versions are included for the benefit of
		clients that cannot handle asynchronous methods.  The asynchronous methods should be used if 
		possible.

		Because of the potential large size of backup data it is not copied into descriptors
		but is located in a transfer buffer.

		When requesting backup data the asynchronous call is made and the relevant AO (whose
		TRequestStatus was supplied) will be activated when the data is available and the
		completion code will indicate success or failure.  If successful, the data can be 
		accessed by means of the GetTransferDataInfo() method and the SignalTransferDataComplete() 
		method is used to indicate that the transfer is complete and the transfer buffer can be re-used.

		When supplying snapshot data or restore data the data is copied into a transfer buffer
		and the asynchronous call is made.  Then the relevant AO will be activated when the data
		has been absorbed by the Secure Backup Engine.  An error completion code is supplied.
		The Secure Backup Server must assume that the transfer buffer is in use until the AO
		is completed.

		During a backup operation, all snapshot data should be provided (or data owners told that
		they will be doing a base backup) before any data is requested.  This is because providing
		snapshots allows data owners to calculate their data and their data sizes.  We cannot provide
		the expected data sizes until we have received all snapshots.
 
		Once all snapshots have been provided, the order in which backup data is requested is not
		constrained (except that one multi-chunk set of data must be fully retrieved before the next
		is requested.

		During a restore operation, the Secure Backup Engine expects data in the following order:

		1) Registration files

		2) Hash data for signed system files.

		3) System files for all required data owners.

		4) Data for data owners.
    
		Stages (1) through (3) are required in a fixed order.  Once stage (4) has been reached
		the SBE does not impose an order between data owners and does not require that all data
		from a specific data owner be restored before the next one be started, i.e. it is possible
		to send base data to all data owners and then increments (but this is not recommended).

		@released
		@publishedPartner
		*/
		{
	public:
		IMPORT_C static CSBEClient* NewL();
		IMPORT_C ~CSBEClient();
		IMPORT_C void ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners);
		IMPORT_C void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles);
		IMPORT_C void RawPublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType,
										 RRestoreFileFilterArray& aFileFilter);
		IMPORT_C void PublicFileListXMLL(TDriveNumber aDrive, TSecureId aSID, HBufC*& aFileList);
		IMPORT_C void SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
								  TBackupIncType aBackupIncType);
		IMPORT_C void SetSIDListForPartialBURL(RSIDArray& aSIDs);
		IMPORT_C void SIDStatusL(RSIDStatusArray& aSIDStatus);		
		IMPORT_C TPtr8& TransferDataAddressL();
		IMPORT_C TPtrC8& TransferDataInfoL(CSBGenericTransferType*& aGenericTransferType,
										   TBool& aFinished);
		IMPORT_C void RequestDataL(CSBGenericTransferType& aGenericTransferType,
								   TRequestStatus& aStatus);
		IMPORT_C void RequestDataL(CSBGenericTransferType& aGenericTransferType);
		IMPORT_C void SupplyDataL(CSBGenericTransferType& aGenericTransferType,
								  TBool aFinished, TRequestStatus& aStatus);
		IMPORT_C void SupplyDataL(CSBGenericTransferType& aGenericTransferType,
								  TBool aFinished);
		IMPORT_C void AllSnapshotsSuppliedL();								  
		IMPORT_C TUint ExpectedDataSizeL(CSBGenericTransferType& aGenericTransferType);
		IMPORT_C void AllSystemFilesRestored();
		IMPORT_C TUint DataChecksum(TDriveNumber aDrive, TSecureId aSID);
		
		IMPORT_C void ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners, TRequestStatus& aStatus);
		IMPORT_C void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles, TRequestStatus& aStatus);
		IMPORT_C void SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
								  TBackupIncType aBackupIncType, TRequestStatus& aStatus);
		IMPORT_C void AllSnapshotsSuppliedL(TRequestStatus& aStatus);
		IMPORT_C void AllSystemFilesRestoredL(TRequestStatus& aStatus);

		IMPORT_C void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, 
										RPointerArray<CSBEFileEntry>& aFileList, TBool& aFinished,
										TInt aTotalListCursor, TInt aMaxResponseSize, TRequestStatus& aStatus);

	private:
		CSBEClient();
		void ConstructL();

	private:
		/** Pointer to the client session R class wrapped by this class */
		RSBEClientSession* iClientSession;
		};

	} // end namespace

#endif // __SBECLIENT_H__
