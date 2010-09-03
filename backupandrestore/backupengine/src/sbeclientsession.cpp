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
// Implementation of RSBEClientSession class.
// 
//

/**
 @file
*/

#include "sbeclientsession.h"
#include "sbeclientserver.h"
#include <connect/sbtypes.h>
#include "sbepanic.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbeclientsessionTraces.h"
#endif

namespace conn
	{
	RSBEClientSession* RSBEClientSession::NewL()
	/** Symbian OS constructor 
	@return pointer to an instantiated RSBEClientSession object */
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_NEWL_ENTRY );
		RSBEClientSession* self = new (ELeave) RSBEClientSession();
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( RSBECLIENTSESSION_NEWL_EXIT );
		return self;
		}

	void RSBEClientSession::ConstructL()
	/** Symbian second phase constructor */
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_CONSTRUCTL_ENTRY );
        iGSHInterface = CHeapWrapper::NewL();
		OstTraceFunctionExit0( RSBECLIENTSESSION_CONSTRUCTL_EXIT );
		}
		
	RSBEClientSession::RSBEClientSession() : iCallbackHandler(NULL)
	/** Class constructor. */
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_RSBECLIENTSESSION_CONS_ENTRY );
		OstTraceFunctionExit0( RSBECLIENTSESSION_RSBECLIENTSESSION_CONS_EXIT );
		}

	RSBEClientSession::~RSBEClientSession()
	/** Class destructor. */
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_RSBECLIENTSESSION_DES_ENTRY );
		delete iGSHInterface;
		delete iCallbackHandler;
		OstTraceFunctionExit0( RSBECLIENTSESSION_RSBECLIENTSESSION_DES_EXIT );
		}

	void RSBEClientSession::Close()
	/** Closes the Secure Backup Engine handle. */
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_CLOSE_ENTRY );
		iGlobalSharedHeap.Close();
		RSessionBase::Close();
		OstTraceFunctionExit0( RSBECLIENTSESSION_CLOSE_EXIT );
		}

	TInt RSBEClientSession::Connect()
	/** Connects the handle to the Secure Backup Engine.

	@return KErrNone if successful, KErrCouldNotConnect otherwise
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_CONNECT_ENTRY );
        TInt nRetry = KSBERetryCount;
		TInt nRet = KErrNotFound;

		while(nRetry > 0 && nRet != KErrNone)
			{
		    const TSecurityPolicy policy(static_cast<TSecureId>(KSBServerUID3));
			nRet = CreateSession(KSBEServerName, Version(), KSBEASyncMessageSlots, EIpcSession_Unsharable,&policy);
			if(nRet == KErrNotFound || nRet == KErrServerTerminated)
				{
				StartServer();
				}
			nRetry--;
			}
			
		// If we were succesful, then get a handle to the server created and owned GSH
		if (nRet == KErrNone)
			{
			nRet = GetGlobalSharedHeapHandle();
			}

		OstTraceFunctionExit0( RSBECLIENTSESSION_CONNECT_EXIT );
		return nRet;
		}

	TVersion RSBEClientSession::Version() const
	/** Returns the version of this API

	@return The version of this API
	*/
		{
	    return TVersion (KSBEMajorVersionNumber,
							KSBEMinorVersionNumber,
							KSBEBuildVersionNumber);
	  	}

	//
	// Server startup code
	TInt RSBEClientSession::StartServer()
	/** Start the server as a thread on WINS or a process on ARM.
	
	Called by Connect when the kernel is unable to create a session
	with the SBE server (as its not running).

	@return Standard Symbian OS code from RProcess/RThread create.
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_STARTSERVER_ENTRY );
        //
		// Servers UID
		const TUidType serverUid(KNullUid, KNullUid, KSBServerUID3);
		
	
		RProcess server;
    	TInt nRet=server.Create(KSBImageName,KNullDesC,serverUid);
    	if (nRet != KErrNone)
    	    {
    		OstTraceFunctionExit0( RSBECLIENTSESSION_STARTSERVER_EXIT );
    		return nRet;
    		}
    		
    	TRequestStatus stat;
    	server.Rendezvous(stat);
    	if (stat != KRequestPending)
    		{
    		server.Kill(0);
    		}
    	else
    		{
    		server.Resume();
    		}
    	User::WaitForRequest(stat);
    	nRet = (server.ExitType() == EExitPanic) ? KErrGeneral : stat.Int();
    	OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_STARTSERVER_EXIT );
    	return nRet;
		
		}


	void RSBEClientSession::ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners)
	/**
	Return the list of private data owners on the device that have backup registration files.
	If a leave does occur, then aDataOwners
	
	@param aDataOwners Pointer array holding the list of Data owners requiring backup functionality.
			Any items present in this array will be lost
	*/
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_LISTOFDATAOWNERSL_ENTRY );
		// Get the server to construct the flattened array and return the size of it		
		TInt result = SendReceive(ESBEMsgPrepDataOwnerInfo);
		
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_LISTOFDATAOWNERSL, "Leave: %d", result));
		
		iDataOwnersArray = &aDataOwners;
		
		PopulateListOfDataOwnersL(result);
		OstTraceFunctionExit0( RSBECLIENTSESSION_LISTOFDATAOWNERSL_EXIT );
		}
		
	void RSBEClientSession::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, 
												RFileArray& aFiles)
	/**
	Get the list of public files to backup for a particular Data Owner on a particular drive
	
	@param aDrive The drive that the public files exist on
	@param aGenericDataType Reference to the generic data type that is being passed to the SBEngine.
	@param aFiles An empty array of file information that will be filled with details of the public files
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_PUBLICFILELISTL_ENTRY );
		// request the public file list
		TInt result = SendReceive(ESBEMsgPrepPublicFiles, TIpcArgs(aDrive, 
						&(aGenericDataType.Externalise())));
		
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_PUBLICFILELISTL, "Leave: %d", result));

		iFileArray = &aFiles;
		
		PopulatePublicFileListL(result);
		OstTraceFunctionExit0( RSBECLIENTSESSION_PUBLICFILELISTL_EXIT );
		}
		
	void RSBEClientSession::RawPublicFileListL(	TDriveNumber aDrive, 
												CSBGenericDataType& aGenericDataType, 
												RRestoreFileFilterArray& aFileFilter)
	/**
	Get the list of public files to backup for a particular Data Owner on a particular drive for 
	a partial restore
	
	@param aDrive The drive that the public files exist on
	@param aGenericDataType Reference to the generic data type that is passed to the SBEngine.
	@param aFileFilter Empty array that will be filled with the files/directories to be backed up 
	on return
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_RAWPUBLICFILELISTL_ENTRY );
        // ensure that the array is cleared out before populating with externalised data
		aFileFilter.Reset();
		
		TPckgC<TDriveNumber> drive(aDrive);

		// request the public file list
		TInt result = SendReceive(ESBEMsgPrepPublicFilesRaw, TIpcArgs(&drive, 
				&(aGenericDataType.Externalise())));
		
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_RAWPUBLICFILELISTL, "Leave: %d", result));

		// Create a descriptor big enough for the array to be externalised into
		HBufC8* pFileArray = HBufC8::NewL(result);
		CleanupStack::PushL(pFileArray);
		
		TPtr8 fileArray(pFileArray->Des());
		result = SendReceive(ESBEMsgGetPublicFilesRaw, TIpcArgs(&fileArray));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, DUP1_RSBECLIENTSESSION_RAWPUBLICFILELISTL, "Leave: %d", result));
		
		RRestoreFileFilterArray* pFileFilter = RRestoreFileFilterArray::InternaliseL(fileArray);
		CleanupStack::PushL(pFileFilter);
		CleanupClosePushL(*pFileFilter);
		
		TInt count = pFileFilter->Count();
		for (TInt x = 0; x < count; x++)
			{
				aFileFilter.AppendL((*pFileFilter)[x]);
			} // for x

		CleanupStack::PopAndDestroy(pFileFilter); // CleanupClosePushL(*pFileFilter)
		CleanupStack::PopAndDestroy(pFileFilter); // CleanupStack::PushL(pFileFilter)
		CleanupStack::PopAndDestroy(pFileArray);
		OstTraceFunctionExit0( RSBECLIENTSESSION_RAWPUBLICFILELISTL_EXIT );
		}
	
	void RSBEClientSession::PublicFileListXMLL(TDriveNumber aDrive, TSecureId aSID, HBufC*& aFileList)
	/**
	Get the list of public files in XML format

	@param aDrive The drive to get the list for
	@param aSID The SID of the data owner to get the public files for
	@param aFileList The descriptor to populate on return should be NULL
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_PUBLICFILELISTXMLL_ENTRY );
        TPckgC<TDriveNumber> drive(aDrive);
		TPckgC<TSecureId> sid(aSID);

		// request the public file list
		TInt result = SendReceive(ESBEMsgPrepPublicFilesXML, TIpcArgs(&drive, &sid));
		
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_PUBLICFILELISTXMLL, "Leave: %d", result));

		// Create a descriptor big enough for the array to be externalised into
		aFileList = HBufC::NewL(result);
	
		TPtr fileList(aFileList->Des());
		result = SendReceive(ESBEMsgPrepPublicFilesXML, TIpcArgs(&fileList));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, DUP1_RSBECLIENTSESSION_PUBLICFILELISTXMLL, "Leave: %d", result));
		OstTraceFunctionExit0( RSBECLIENTSESSION_PUBLICFILELISTXMLL_EXIT );
		}
	
	void RSBEClientSession::SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
						  TBackupIncType aBackupIncType)
	/**
	Set the Backup and Restore mode on/off and configure the BUR options
	
	@param aDriveList Array of drives that are to be backed up during the operations
	@param aBURType Set the device into Full/Partial BUR or normal operation
	@param aBackupIncType Base/Incremental backup
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_SETBURMODEL_ENTRY );
		TInt result = SendReceive(ESBEMsgSetBURMode, TIpcArgs(&aDriveList, aBURType, aBackupIncType));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_SETBURMODEL, "Leave: %d", result));
		OstTraceFunctionExit0( RSBECLIENTSESSION_SETBURMODEL_EXIT );
		}
		
	void RSBEClientSession::SetSIDListForPartialBURL(RSIDArray& aSIDs)
	/**
	If a partial backup is required then this sets the list of data owners.
	This method must only be called when the device has just been put into backup or restore mode.
	It must only be called once for a backup or restore operation.

	@param aSIDs array of affected data owners.
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_SETSIDLISTFORPARTIALBURL_ENTRY );
        HBufC8* pFlattenedArray = aSIDs.ExternaliseL();
		CleanupStack::PushL(pFlattenedArray);
		
		TPtrC8 flatArray(pFlattenedArray->Des());
		
		TInt result = SendReceive(ESBEMsgSetSIDListPartial, TIpcArgs(&flatArray));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_SETSIDLISTFORPARTIALBURL, "Leave: %d", result));

		CleanupStack::PopAndDestroy(pFlattenedArray);
		OstTraceFunctionExit0( RSBECLIENTSESSION_SETSIDLISTFORPARTIALBURL_EXIT );
		}
		
	void RSBEClientSession::SIDStatusL(RSIDStatusArray& aSIDStatus)
	/**
	Gets the status of a set of data owners.
	This method must only be called in backup or restore mode.

	@param aSIDStatus an array of structures for information about data owners. On return
	the status information is filled in.
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_SIDSTATUSL_ENTRY );
        HBufC8* pExternalisedArray = aSIDStatus.ExternaliseL();
		CleanupStack::PushL(pExternalisedArray);
		
		TPtr8 externArray(pExternalisedArray->Des());
		TInt result = SendReceive(ESBEMsgPrepSIDStatus, TIpcArgs(&externArray));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_SIDSTATUSL, "Leave: %d", result));
		
		// Reset the descriptor, ready for getting the returned externalised array
		externArray.Zero();
		
		result = SendReceive(ESBEMsgGetSIDStatus, TIpcArgs(&externArray));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, DUP1_RSBECLIENTSESSION_SIDSTATUSL, "Leave: %d", result));
		RSIDStatusArray* pInternalisedArray = RSIDStatusArray::InternaliseL(externArray);

		CleanupStack::PopAndDestroy(pExternalisedArray); // pExternalisedArray
		
		CleanupStack::PushL(pInternalisedArray);
		CleanupClosePushL(*pInternalisedArray);
		
		aSIDStatus.Reset();

		// Copy the returned array into the passed array
		TInt count = pInternalisedArray->Count();
		for (TInt index = 0; index < count; index++)
			{
			aSIDStatus.AppendL((*pInternalisedArray)[index]);
			}
		CleanupStack::PopAndDestroy(pInternalisedArray);	// pInternalisedArray->Close()
		CleanupStack::PopAndDestroy(pInternalisedArray); // pInternalisedArray
		OstTraceFunctionExit0( RSBECLIENTSESSION_SIDSTATUSL_EXIT );
		}
	
	TPtr8& RSBEClientSession::TransferDataAddressL()
	/**
	Provides access to the base of the global chunk used to transfer data between
	the Secure Backup Engine and a Secure Backup Server.  This method should be used
	when the Secure Backup Server is providing data to the Secure Backup Engine (either as part 
	of a restore operation or when supplying snapshots during a backup operation.

	The Secure Backup Engine only uses one global chunk at a time. It is not permissible to
	try to carry out multiple backup or restore operations in parallel.  Normally a chunk
	of global heap would be protected by a mutex.  In this case, all the methods of the 
	CSecureBackupEngine must be regarded as synchronous and mutually exclusive - it is not
	permissible to make parallel calls.

	The global chunk used during a backup or restore operation may change and so the address must
	be requested whenever required rather than being cached.

	@return Pointer to the start of the buffer for writing
	*/
		{
        return iGSHInterface->WriteBufferL(iGlobalSharedHeap);
		}

	TPtrC8& RSBEClientSession::TransferDataInfoL(CSBGenericTransferType*& aGenericTransferType,
												 TBool& aFinished)
	/**
	Provides access to the data received from the Secure Backup Engine during a backup operation.

	This method should be called after a synchronous or asynchronous request for data has
	completed.

	@param aGenericTransferType Pointer reference that a Generic Transfer Type is allocated to
	@param aFinished Flag that will be set to ETrue if the data on the GSH is the last in the series
	@return Pointer to the start of the buffer for reading
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_TRANSFERDATAINFOL_ENTRY );
        TPtrC8& returnedBuf = iGSHInterface->ReadBufferL(iGlobalSharedHeap);
		
		TDesC8& genTypeBuffer = iGSHInterface->Header(iGlobalSharedHeap).GenericTransferTypeBuffer();
		if (genTypeBuffer.Size() == 0)
			{
		    OstTrace0(TRACE_ERROR, RSBECLIENTSESSION_TRANSFERDATAINFOL, "Leave: KErrNotReady");
			User::Leave(KErrNotReady);
			}
		
		// Create a new client-side transfer type and pass ownership
		aGenericTransferType = CSBGenericTransferType::NewL(genTypeBuffer);
		CleanupStack::PushL(aGenericTransferType);
		
		aFinished = iGSHInterface->Header(iGlobalSharedHeap).iFinished;

		CleanupStack::Pop(aGenericTransferType);
		
		OstTraceFunctionExit0( RSBECLIENTSESSION_TRANSFERDATAINFOL_EXIT );
		return returnedBuf;
		}

	TInt RSBEClientSession::GetGlobalSharedHeapHandle()
	/**
	Requests the handle for the Global Anonymous Shared Heap from the server that owns it and
	sets the member RChunk with it.
	
	@return An error code resulting from the server request for the handle, KErrNone if ok
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_GETGLOBALSHAREDHEAPHANDLE_ENTRY );
        TInt ret = SendReceive(ESBEMsgGetGSHHandle);
		
		// ret is negative if an error has ocurred
		if (ret > KErrNone)
			{
			ret = iGlobalSharedHeap.SetReturnedHandle(ret);
			
			// Since a handle was returned, there were no errors
			ret = KErrNone;
			}
		
		OstTraceFunctionExit0( RSBECLIENTSESSION_GETGLOBALSHAREDHEAPHANDLE_EXIT );
		return ret;
		}
		
	void RSBEClientSession::RequestDataL(CSBGenericTransferType& aGenericTransferType, 
		TRequestStatus& aStatus)
	/**
	Asynchronous request of the Secure Backup Engine to supply data for a particular data owner.
	When the supplied TRequestStatus has been completed by the server, TransferDataInfoL should
	be called to retrieve the requested data.
	
	@param aGenericTransferType Reference to the identifier of the data requested
	@param aStatus TRequestStatus object used by the server to signal the client that a response 
	is ready
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_REQUESTDATAL_ENTRY );
        const TDesC8& transBuf = aGenericTransferType.Externalise();
		SendReceive(ESBEMsgRequestDataAsync, TIpcArgs(&transBuf), aStatus);
		OstTraceFunctionExit0( RSBECLIENTSESSION_REQUESTDATAL_EXIT );
		}

	void RSBEClientSession::RequestDataL(CSBGenericTransferType& aGenericTransferType)
	/**
	Synchronous request of the Secure Backup Engine to supply data for a particular data owner.
	When the supplied TRequestStatus has been completed by the server, TransferDataInfoL should
	be called to retrieve the requested data.
	
	@param aGenericTransferType Reference to the identifier of the data requested
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_REQUESTDATAL_ENTRY );
        TInt result = SendReceive(ESBEMsgRequestDataSync, TIpcArgs(&(aGenericTransferType.Externalise())));
        LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, DUP1_RSBECLIENTSESSION_REQUESTDATAL, "Leave: %d", result));
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_REQUESTDATAL_EXIT );
		}
		
	void RSBEClientSession::SupplyDataL(CSBGenericTransferType& aGenericTransferType, 
		TBool aFinished, TRequestStatus& aStatus)
	/**
	Synchronous method for signalling to the server that the client has placed an amount 
	of data in the Global Shared Heap and
	
	@param aGenericTransferType Information about the data that has been transferred
	@param aFinished ETrue indicates that additional SupplyDataL calls will be made as 
	part of this transfer operation
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_SUPPLYDATAL_ENTRY );
        iGSHInterface->Header(iGlobalSharedHeap).GenericTransferTypeBuffer() 
			= aGenericTransferType.Externalise();

		SendReceive(ESBEMsgSupplyDataSync, TIpcArgs(aFinished), aStatus);
		OstTraceFunctionExit0( RSBECLIENTSESSION_SUPPLYDATAL_EXIT );
		}

	void RSBEClientSession::SupplyDataL(CSBGenericTransferType& aGenericTransferType, 
		TBool aFinished)
	/**
	Synchronous method for signalling to the server that the client has placed an amount 
	of data in the Global Shared Heap and
	
	@param aGenericTransferType Information about the data that has been transferred
	@param aFinished ETrue indicates that additional SupplyDataL calls will be made as 
	part of this transfer operation
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_SUPPLYDATAL_ENTRY );
        iGSHInterface->Header(iGlobalSharedHeap).GenericTransferTypeBuffer() 
			= aGenericTransferType.Externalise();

        TInt result = SendReceive(ESBEMsgSupplyDataSync, TIpcArgs(aFinished));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, DUP1_RSBECLIENTSESSION_SUPPLYDATAL, "Leave: %d", result));
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_SUPPLYDATAL_EXIT );
		}
		
	void RSBEClientSession::AllSnapshotsSuppliedL()
	/**
	This methods informs the data owner that all snapshots have been supplied.

	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_ALLSNAPSHOTSSUPPLIEDL_ENTRY );
        TInt result = SendReceive(ESBEMsgAllSnapshotsSupplied);
        LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_ALLSNAPSHOTSSUPPLIEDL, "Leave: %d", result));
	
		OstTraceFunctionExit0( RSBECLIENTSESSION_ALLSNAPSHOTSSUPPLIEDL_EXIT );
		}
		
	TUint RSBEClientSession::ExpectedDataSizeL(CSBGenericTransferType& aGenericTransferType)
	/**
	Get the expected total size of the data to be returned by the SBE for the purposes 
	of calculating progress information
	
	@param aGenericTransferType Reference to the identifier of the data to be retrieved
	@leave If a synchronous IPC call to the SBEngine returns an error code (i.e. if SBEngine leaves)
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_EXPECTEDDATASIZEL_ENTRY );
        TPckgBuf<TUint> sizePkg;

		TPtrC8 genType(aGenericTransferType.Externalise());
		
		TInt result = SendReceive(ESBEMsgGetExpectedDataSize, TIpcArgs(&genType, &sizePkg));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_EXPECTEDDATASIZEL, "Leave: %d", result));
			
		TUint size = sizePkg();
		OstTraceFunctionExit0( RSBECLIENTSESSION_EXPECTEDDATASIZEL_EXIT );
		return size;
		}
		
	void RSBEClientSession::AllSystemFilesRestored()
	/**
	Signal the Secure Backup Engine that registration files are to be parsed and Active data owners 
	are to be started
	*/
		{
        OstTraceFunctionEntry0( RSBECLIENTSESSION_ALLSYSTEMFILESRESTORED_ENTRY );
        SendReceive(ESBEMsgAllSystemFilesRestored);
		OstTraceFunctionExit0( RSBECLIENTSESSION_ALLSYSTEMFILESRESTORED_EXIT );
		}
	
	/**
	Return the list of private data owners on the device that have backup registration files.
	If a leave does occur, then aDataOwners
	
	@param aDataOwners Pointer array holding the list of Data owners requiring backup functionality.
			Any items present in this array will be lost
	@param aStatus is TRequestStatus&
	*/
	void RSBEClientSession::ListOfDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners, TRequestStatus& aStatus)
		{
		OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_LISTOFDATAOWNERSL_ENTRY );
		if (iCallbackHandler == NULL)
			{
			iCallbackHandler = CSBECallbackHandler::NewL(*this);				
			}

		if (iCallbackHandler->IsActive())
			{
		    OstTrace0(TRACE_ERROR, DUP1_RSBECLIENTSESSION_LISTOFDATAOWNERSL, "Leave: KErrInUse");
			User::Leave(KErrInUse);
			}
		else
			{
			iDataOwnersArray = &aDataOwners;
			SendReceive(ESBEMsgPrepDataOwnerInfo, iCallbackHandler->iStatus);
			iCallbackHandler->StartL(aStatus, EListOfDataOwners);
			}
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_LISTOFDATAOWNERSL_EXIT );
		}
	
	/**
	Get the list of public files to backup for a particular Data Owner on a particular drive
	
	@param aDrive The drive that the public files exist on
	@param aGenericDataType Reference to the generic data type that is being passed to the SBEngine.
	@param aFiles An empty array of file information that will be filled with details of the public files
	@param aStatus A reference to TRequestStatus
	
	*/	
	void RSBEClientSession::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, RFileArray& aFiles, TRequestStatus& aStatus)
		{
		OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_PUBLICFILELISTL_ENTRY );
		if (iCallbackHandler == NULL)
			{
			iCallbackHandler = CSBECallbackHandler::NewL(*this);				
			}
	
		if (iCallbackHandler->IsActive())
			{
		    OstTrace0(TRACE_ERROR, DUP1_RSBECLIENTSESSION_PUBLICFILELISTL, "Leave: KErrInUse");
			User::Leave(KErrInUse);
			}
		else
			{
			iFileArray = &aFiles;
			// request the public file list
			SendReceive(ESBEMsgPrepPublicFiles, TIpcArgs(aDrive, &(aGenericDataType.Externalise())), iCallbackHandler->iStatus);
			iCallbackHandler->StartL(aStatus,EPublicFileList);
			}
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_PUBLICFILELISTL_EXIT );
		}
		
	void RSBEClientSession::SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
								  TBackupIncType aBackupIncType, TRequestStatus& aStatus)
	/**
	Set the Backup and Restore mode on/off and configure the BUR options asynchronously.
	
	@param aDriveList Array of drives that are to be backed up during the operations
	@param aBURType Set the device into Full/Partial BUR or normal operation
	@param aBackupIncType Base/Incremental backup
	@param aStatus A reference to TRequestStatus
	*/

		{
		OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_SETBURMODEL_ENTRY );
		SendReceive(ESBEMsgSetBURMode, TIpcArgs(&aDriveList, aBURType, aBackupIncType), aStatus);
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_SETBURMODEL_EXIT );
		}
	
	/**
	This methods informs the data owner that all snapshots have been supplied.

	@param aStatus A reference to TRequestStatus
	*/	
	void RSBEClientSession::AllSnapshotsSuppliedL(TRequestStatus& aStatus)
		{
		OstTraceFunctionEntry0( DUP1_RSBECLIENTSESSION_ALLSNAPSHOTSSUPPLIEDL_ENTRY );
		SendReceive(ESBEMsgAllSnapshotsSupplied, aStatus);
		OstTraceFunctionExit0( DUP1_RSBECLIENTSESSION_ALLSNAPSHOTSSUPPLIEDL_EXIT );
		}
	
	/**
	Signal the Secure Backup Engine that registration files are to be parsed and Active data owners 
	are to be started
	
	@param aStatus A reference to TRequestStatus
	*/	
	void RSBEClientSession::AllSystemFilesRestoredL(TRequestStatus& aStatus)
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_ALLSYSTEMFILESRESTOREDL_ENTRY );
		SendReceive(ESBEMsgAllSystemFilesRestored, aStatus);
		OstTraceFunctionExit0( RSBECLIENTSESSION_ALLSYSTEMFILESRESTOREDL_EXIT );
		}
				
	
	/**
	Method to perform and IPC call to populate list of data owners.
	@param aBufferSize Size of the buffer needed to be allocated for the IPC call
	
	@InternalTechnology
	*/
	void RSBEClientSession::PopulateListOfDataOwnersL(TUint aBufferSize)
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_POPULATELISTOFDATAOWNERSL_ENTRY );
		__ASSERT_DEBUG(iDataOwnersArray, Panic(KErrBadHandle));
		iDataOwnersArray->ResetAndDestroy();
					
		// Create a descriptor that's appropriate to hold the buffer to be returned		
		HBufC8* pReturnedBuf = HBufC8::NewL(aBufferSize);
		
		CleanupStack::PushL(pReturnedBuf);

		TPtr8 returnedBuf(pReturnedBuf->Des());
		// Request that the server returns the previously packed array
		TInt result = SendReceive(ESBEMsgGetDataOwnerInfo, TIpcArgs(&returnedBuf));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_POPULATELISTOFDATAOWNERSL, "Leave: %d", result));
		
		TInt offset = 0;
		
		for (TInt index = 0; index < result; index++)
			{
			CDataOwnerInfo* pDOI = CDataOwnerInfo::NewL(returnedBuf.Mid(offset));
			CleanupStack::PushL(pDOI);

			iDataOwnersArray->AppendL(pDOI);

			CleanupStack::Pop(pDOI);
			
			offset += (*iDataOwnersArray)[index]->Size();
			}
			
		CleanupStack::PopAndDestroy(pReturnedBuf);
		OstTraceFunctionExit0( RSBECLIENTSESSION_POPULATELISTOFDATAOWNERSL_EXIT );
		}
	
	/**
	Method to perform and IPC call to populate list of public files.
	@param aBufferSize Size of the buffer needed to be allocated for the IPC call
	
	@InternalTechnology
	*/	
	void RSBEClientSession::PopulatePublicFileListL(TUint aBufferSize)
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_POPULATEPUBLICFILELISTL_ENTRY );
		__ASSERT_DEBUG(iFileArray, Panic(KErrBadHandle));
		iFileArray->Reset();
		
		// Create a descriptor big enough for the array to be externalised into
		HBufC8* pFileArray = HBufC8::NewL(aBufferSize);
		CleanupStack::PushL(pFileArray);
		
		TPtr8 fileArray(pFileArray->Des());
		TInt result = SendReceive(ESBEMsgGetPublicFiles, TIpcArgs(&fileArray));
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_POPULATEPUBLICFILELISTL, "Leave: %d", result));
		
		RFileArray* pFiles = RFileArray::InternaliseL(fileArray);
		CleanupStack::PopAndDestroy(pFileArray);
		CleanupStack::PushL(pFiles);
		CleanupClosePushL(*pFiles);
		
		TInt count = pFiles->Count();
		for (TInt x = 0; x < count; x++)
			{
			iFileArray->AppendL((*pFiles)[0]);
			pFiles->Remove(0);					// We're running out of memory, hence be frugal
			} // for x
		
		CleanupStack::PopAndDestroy(pFiles); // CleanupClosePushL(*pFiles)
		CleanupStack::PopAndDestroy(pFiles); // CleanupStack::PushL(pFiles)
		OstTraceFunctionExit0( RSBECLIENTSESSION_POPULATEPUBLICFILELISTL_EXIT );
		}

	void RSBEClientSession::PublicFileListL(TDriveNumber aDrive, CSBGenericDataType& aGenericDataType, 
								RPointerArray<CSBEFileEntry>& aFileList, TBool& aFinished,
								TInt aTotalListCursor, TInt aMaxResponseSize, TRequestStatus& aStatus)
	/** 
	This asynchronous method is used to retrieve the list of public files for the specified data owner
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
		OstTraceFunctionEntry0( DUP2_RSBECLIENTSESSION_PUBLICFILELISTL_ENTRY );
		if (iCallbackHandler == NULL)
			{
			iCallbackHandler = CSBECallbackHandler::NewL(*this);				
			}
	
		if (iCallbackHandler->IsActive())
			{
		    OstTrace0(TRACE_ERROR, DUP2_RSBECLIENTSESSION_PUBLICFILELISTL, "Leave: KErrInUse");
			User::Leave(KErrInUse);
			}
		else
			{
			iFileList = &aFileList;
			iFinished = &aFinished;
			iTotalListCursor = &aTotalListCursor;
/*			TPckgC<TDriveNumber> drivePkg(aDrive);
			TPckgC<TInt> cursorPkg(aTotalListCursor);
			TPckgC<TInt> maxResp(aMaxResponseSize);*/
			SendReceive(ESBEMsgPrepLargePublicFiles, TIpcArgs(static_cast<TInt>(aDrive), 
				&(aGenericDataType.Externalise()), aTotalListCursor, aMaxResponseSize), 
				iCallbackHandler->iStatus);
			iCallbackHandler->StartL(aStatus,ELargePublicFileList);
			}
		OstTraceFunctionExit0( DUP2_RSBECLIENTSESSION_PUBLICFILELISTL_EXIT );
		}
	
	void RSBEClientSession::PopulateLargePublicFileListL(TInt aResult)
	/** 
	Callback following the asynchronous completion of the request for the public file list
	@param aResult The error code returned by the engine as a result of the initial request
	*/
		{
		OstTraceFunctionEntry0( RSBECLIENTSESSION_POPULATELARGEPUBLICFILELISTL_ENTRY );
		// Retrieve the return parameters (finished flag and entry count) from SBE
		if (KErrNone == aResult)
			{
			TBool finishedFlag;
			TInt numEntries;
			TPckg<TBool> finishPkg(finishedFlag);
			TPckg<TInt> numEntriesPkg(numEntries);
			TInt result = SendReceive(ESBEMsgGetLargePublicFiles, TIpcArgs(&finishPkg, &numEntriesPkg));
			LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, RSBECLIENTSESSION_POPULATELARGEPUBLICFILELISTL, "Leave: %d", result));
			
			*iFinished = finishPkg();
			TInt numberOfReturnedEntries = numEntriesPkg();
			
			iFileList->ResetAndDestroy();
			TInt cursor = 0;
			TPtrC8 returnedBuf(iGSHInterface->ReadBufferL(iGlobalSharedHeap));
			
			// Retrieve the file list from GSH
			// Pack into the previously supplied array
			for (TInt entryIndex = 0; entryIndex < numberOfReturnedEntries; ++entryIndex)
				{
				TInt bytesRead = 0;
				CSBEFileEntry* nextEntry = CSBEFileEntry::NewLC(returnedBuf.Mid(cursor), bytesRead);
				cursor += bytesRead;
				iFileList->AppendL(nextEntry);
				CleanupStack::Pop(nextEntry);
				}
			}
		else
			{
			*iFinished = EFalse;
			}
		OstTraceFunctionExit0( RSBECLIENTSESSION_POPULATELARGEPUBLICFILELISTL_EXIT );
		}


	//
	// CSBECallbackHandler //
	//	
	
	/** Symbian OS constructor 
	@param aClientSession reference to a ClientSession to call callbacks on
	@return pointer to an instantiated RSBEClientSession object 
	*/
	CSBECallbackHandler* CSBECallbackHandler::NewL(RSBEClientSession& aClientSession)
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_NEWL_ENTRY );
		CSBECallbackHandler* self = new (ELeave) CSBECallbackHandler(aClientSession);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_NEWL_EXIT );
		return self;
		}
		
	/** Symbian second phase constructor */
	void CSBECallbackHandler::ConstructL()
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_CONSTRUCTL_ENTRY );
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_CONSTRUCTL_EXIT );
		}
		
	/** Class constructor. */
	CSBECallbackHandler::CSBECallbackHandler(RSBEClientSession& aClientSession)
		: CActive(EPriorityNormal), iClientSession(aClientSession)
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_CSBECALLBACKHANDLER_CONS_ENTRY );
		CActiveScheduler::Add(this);
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_CSBECALLBACKHANDLER_CONS_EXIT );
		}
	/** Class destructor. */
	CSBECallbackHandler::~CSBECallbackHandler()
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_CSBECALLBACKHANDLER_DES_ENTRY );
		Cancel();
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_CSBECALLBACKHANDLER_DES_EXIT );
		}
	
	/** Starts Callback Handler
	
	@param aStatus Reference to the Client's request Status
	@param aState State in order to make a relevant callback
	
	*/	
	void CSBECallbackHandler::StartL(TRequestStatus& aStatus, TState aState)
		{	
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_STARTL_ENTRY );
		aStatus = KRequestPending;
		iObserver = &aStatus;
		iState = aState;
		SetActive();
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_STARTL_EXIT );
		}
		
	/**
	Cancels outsanding request
	*/
  	void CSBECallbackHandler::CancelRequest()
  		{
  		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_CANCELREQUEST_ENTRY );
  		Cancel();
  		OstTraceFunctionExit0( CSBECALLBACKHANDLER_CANCELREQUEST_EXIT );
  		}
	/**
	CActive::RunL() implementation
	*/	
	void CSBECallbackHandler::RunL()
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_RUNL_ENTRY );
		TInt result = iStatus.Int();
		if (result >= KErrNone)
			{			
			switch (iState)
				{
			case EListOfDataOwners:
				iClientSession.PopulateListOfDataOwnersL(result);
				break;
			case EPublicFileList:
				iClientSession.PopulatePublicFileListL(result);
				break;
			case ELargePublicFileList:
				iClientSession.PopulateLargePublicFileListL(result);
				break;
			default:
				result = KErrNotSupported;
				break;
				} //switch
			} // if
			
		LEAVEIFERROR(result, OstTrace1(TRACE_ERROR, CSBECALLBACKHANDLER_RUNL, "Leave: %d", result));
		
		CompleteObserver(KErrNone);
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_RUNL_EXIT );
		}
	
	/**
	CActive::DoCancel() implmenation
	Completes observer's status with KErrCancel and sets the state to None
	*/	
	void CSBECallbackHandler::DoCancel()
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_DOCANCEL_ENTRY );
		iState = ENone;
		// just to avoid repeating the code
		CompleteObserver(KErrCancel);
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_DOCANCEL_EXIT );
		}
	
	/**
	Method for completing Client's request status
	@param aError Completion Error
	*/
	void CSBECallbackHandler::CompleteObserver(TInt aError)
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_COMPLETEOBSERVER_ENTRY );
		if(iObserver)
			{
			User::RequestComplete(iObserver, aError);
			iObserver = NULL;
			}
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_COMPLETEOBSERVER_EXIT );
		}
		
	/**
	If RunL() leaves a CompleteObserver() method called
	
	@aError Error code
	*/
	TInt CSBECallbackHandler::RunError(TInt aError)
		{
		OstTraceFunctionEntry0( CSBECALLBACKHANDLER_RUNERROR_ENTRY );
		CompleteObserver(aError);
		OstTraceFunctionExit0( CSBECALLBACKHANDLER_RUNERROR_EXIT );
		return KErrNone;
		}
		

	} // conn namespace
