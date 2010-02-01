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
* Declaration of RSBEClientSession
* 
*
*/



/**
 @file
*/
#ifndef __SBECLIENTSESSION_H__
#define __SBECLIENTSESSION_H__

#include <connect/sbdefs.h>
#include <e32std.h>
#include <connect/sbtypes.h>
#include "sbheapwrapper.h"

namespace conn
	{
	class CSBECallbackHandler;
	/** List of Operations for Active Callback execution 
	
	@internalComponent
	*/
	enum TState
		{
		ENone,
		EListOfDataOwners,
		EPublicFileList,
		ELargePublicFileList
		};
	 
	class TServerStart;
	class CSBECallbackHandler;

    /** The client to the Secure Backup Engine
    
    This class should be accessed only through the CSBEClient class. These two classes make up 
    a facade pattern to protect against Binary Compatibility breaks. As large chunks of data are 
    transferred using the global shared heap, this class owns an instantiation of CHeapWrapper 
    that wraps access to the Global Shared Heap, providing an easier to use common interface.

	@internalComponent
    */
	class RSBEClientSession : public RSessionBase
    {
	public:
		static RSBEClientSession* NewL();
		TInt Connect();
		void Close();
		TVersion Version() const;
		TPtr8& TransferDataAddressL();
		TPtrC8& TransferDataInfoL(CSBGenericTransferType*& aGenericTransferType, TBool& aFinished);
		void ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners);
		void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles);
		void RawPublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RRestoreFileFilterArray& aFileFilter);
		void PublicFileListXMLL(TDriveNumber aDrive, TSecureId aSID, HBufC*& aFileList);
		void SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, TBackupIncType aBackupIncType);
		void SetSIDListForPartialBURL(RSIDArray& aSIDs);
		void SIDStatusL(RSIDStatusArray& aSIDStatus);
		void RequestDataL(CSBGenericTransferType& aGenericTransferType, TRequestStatus& aStatus);
		void RequestDataL(CSBGenericTransferType& aGenericTransferType);
		void SupplyDataL(CSBGenericTransferType& aGenericTransferType, TBool aFinished);
		void SupplyDataL(CSBGenericTransferType& aGenericTransferType, TBool aFinished, TRequestStatus& aStatus);
		TUint ExpectedDataSizeL(CSBGenericTransferType& aGenericTransferType);
		void AllSnapshotsSuppliedL();
		void AllSystemFilesRestored();
	
		void ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners, TRequestStatus& aStatus);
		void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles, TRequestStatus& aStatus);
		void SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
									  TBackupIncType aBackupIncType, TRequestStatus& aStatus);
		void AllSnapshotsSuppliedL(TRequestStatus& aStatus);
		void AllSystemFilesRestoredL(TRequestStatus& aStatus);
	
		void PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RPointerArray<CSBEFileEntry>& aFileList, 
			TBool& aFinished, TInt aTotalListCursor, TInt aMaxResponseSize, TRequestStatus& aStatus);

		friend class CSBECallbackHandler;

		~RSBEClientSession();

	private:
		RSBEClientSession();
		static TInt StartServer();
		TInt GetGlobalSharedHeapHandle();
		void ConstructL();

	protected:	
		// callback functions
		void PopulateListOfDataOwnersL(TUint aBufferSize);
		void PopulatePublicFileListL(TUint aBufferSize);
		void PopulateLargePublicFileListL(TInt aResult);
	
	private:
		/** Handle to the Global Shared Heap */
		RChunk iGlobalSharedHeap;
	
		/** Pointer to a CheapWrapper object that marshalls access to the Global Shared Heap */
		CHeapWrapper* iGSHInterface;
		
		/** Pointer to the Client's Data Owners Array */
		RPointerArray<CDataOwnerInfo>* 	iDataOwnersArray;
		/** Poitner to the Client's File Array */
		RFileArray*						iFileArray;
	
		/** Async callback handler for handling responses from the sbengine */
		CSBECallbackHandler* 			iCallbackHandler;
		/** Attributes that store parameters passed into async methods */
		RPointerArray<CSBEFileEntry>* iFileList;
		TBool* iFinished;
		TInt* iTotalListCursor;
    };
    
  
  /** The Callback Handler for the RSBEClientSession
    
    This class implements an Active Object interface in order to perform asynchronous calls in a callback manner.

	@internalComponent
   */
  class CSBECallbackHandler : public CActive
  	{  	
  public:
  
  	static CSBECallbackHandler* NewL(RSBEClientSession& aClientSession);
  	~CSBECallbackHandler();
  	
  	void StartL(TRequestStatus& aStatus, TState aState);
  	void CancelRequest();
  	
  private:

  	CSBECallbackHandler(RSBEClientSession& aClientSession);
  	void ConstructL();
  	
  	// From CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);
	void CompleteObserver(TInt aError);
  private:
  	/** Reference to the Client Session Class */
  	RSBEClientSession& 	iClientSession;
  	/** Observer's TRequestStatus */
  	TRequestStatus*		iObserver;
  	/** State to know which function to run */
  	TState				iState;
  	};
  	
  } // end namespace

#endif // __SBECLIENTSESSION_H__
