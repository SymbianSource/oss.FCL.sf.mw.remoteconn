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
// Implementation of CSBESession class.
// 
//

/**
 @file
*/

#include <e32std.h>
#include <e32base.h>
#include "sbeserver.h"
#include "sbesession.h"
#include "sbeclientserver.h"
#include <connect/panic.h>
#include "sbedataownermanager.h"
#include <connect/sbtypes.h>
#include "sblog.h"
#include <apgcli.h>

namespace conn
	{
	
	/** This block size for the internal buffer
	@internalTechnology
	*/
	const TInt KSBBufferBlockSize = 4096;
	
	CSBESession::CSBESession() : iTransferBuf(NULL), iTransferTextBuf(NULL), iArrayCount(0)
    /**
    Class Constructor
    */
		{
		}

	CSBESession::~CSBESession()
    /**
    Class destructor
    */
		{
		//
		// If the client has detached properly, they should
		// have done this - but just in case.
		Server().DropSession();
		ResetTransferBuf();
		delete iExtCurEntry;
		}

	void CSBESession::CreateL()
	/**
	Creates a connection between the Secure Backup Engine server and the Secure
	Backup Engine session.  Increments the server's session count
	*/
		{
		//
		// Increase the servers session count.
		Server().AddSession();
		}

	void CSBESession::ServiceL(const RMessage2& aMessage)
	/**
	Called by the client server framework to service a message request
	from a client.

    @param aMessage  Reference to a RMessage2 object
	*/
		{
	#ifdef SBE_LOGGING_ENABLED
		RThread thread;
		aMessage.Client(thread);
		const TFullName threadName( thread.FullName() );
		thread.Close();
	#endif		
		
		switch(aMessage.Function())
			{
			//
			// Connection config getting/setting.
			case ESBEMsgGetGSHHandle:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetGSHHandle for thread: %S", &threadName);
				aMessage.Complete(Server().GlobalSharedHeap());
				break;
				}
			case ESBEMsgPrepDataOwnerInfo:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepDataOwnerInfo for thread: %S", &threadName);
				PrepDataOwnerInfoL(aMessage);
				break;
				}
			case ESBEMsgGetDataOwnerInfo:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetDataOwnerInfo for thread: %S", &threadName);
				ReturnDataOwnerInfoL(aMessage);
				break;
				}
			case ESBEMsgPrepPublicFiles:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepPublicFiles for thread: %S", &threadName);
				PrepPublicFileListL(aMessage);
				break;
				}
			case ESBEMsgGetPublicFiles:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetPublicFiles for thread: %S", &threadName);
				ReturnPublicFileListL(aMessage);
				break;
				}
			case ESBEMsgPrepPublicFilesRaw:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepPublicFilesRaw for thread: %S", &threadName);
				PrepPublicFileListRawL(aMessage);
				break;
				}
			case ESBEMsgGetPublicFilesRaw:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetPublicFilesRaw for thread: %S", &threadName);
				ReturnPublicFileListRawL(aMessage);
				break;
				}
			case ESBEMsgPrepPublicFilesXML:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepPublicFilesXML for thread: %S", &threadName);
				PrepPublicFileListXMLL(aMessage);
				break;
				}
			case ESBEMsgGetPublicFilesXML:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetPublicFilesXML for thread: %S", &threadName);
				ReturnPublicFileListXMLL(aMessage);
				break;
				}
			case ESBEMsgSetBURMode:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgSetBURMode for thread: %S", &threadName);
				SetBURModeL(aMessage);
				break;
				}
			case ESBEMsgSetSIDListPartial:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgSetSIDListPartial for thread: %S", &threadName);
				SetSIDListForPartialBURL(aMessage);
				break;
				}
			case ESBEMsgPrepSIDStatus:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepSIDStatus for thread: %S", &threadName);
				PrepSIDStatusL(aMessage);
				break;
				}
			case ESBEMsgGetSIDStatus:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetSIDStatus for thread: %S", &threadName);
				ReturnSIDStatusL(aMessage);
				break;
				}
			case ESBEMsgRequestDataSync:
			case ESBEMsgRequestDataAsync:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgRequestDataSync / ESBEMsgRequestDataAsync for thread: %S", &threadName);
				TRAPD(reqDataErr, RequestDataAsyncL(aMessage));
				if (reqDataErr != KErrNone)
					{
					Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);
					User::Leave(reqDataErr);
					}
				break;
				}
			case ESBEMsgSupplyDataSync:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgSupplyDataSync for thread: %S", &threadName);
				TRAPD(supDataErr, SupplyDataSyncL(aMessage));
				if (supDataErr != KErrNone)
					{
					Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);
					User::Leave(supDataErr);
					}
				break;
				}
			case ESBEMsgAllSnapshotsSupplied:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgAllSnapshotsSupplied for thread: %S", &threadName);
				AllSnapshotsSuppliedL(aMessage);
				break;
				}
			case ESBEMsgGetExpectedDataSize:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetExpectedDataSize for thread: %S", &threadName);
				GetExpectedDataSizeL(aMessage);
				break;
				}
			case ESBEMsgAllSystemFilesRestored:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgAllSystemFilesRestored for thread: %S", &threadName);
				AllSystemFilesRestoredL();
				aMessage.Complete(KErrNone);
				break;
				}
			case ESBEMsgPrepLargePublicFiles:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgPrepPublicFiles for thread: %S", &threadName);
				PrepLargePublicFileListL(aMessage);
				break;
				}
			case ESBEMsgGetLargePublicFiles:
				{
        		__LOG1("CSBESession::ServiceL() - ESBEMsgGetPublicFiles for thread: %S", &threadName);
				ReturnLargePublicFileListL(aMessage);
				break;
				}
			default:
				{
        		__LOG1("CSBESession::ServiceL() - UNKNOWN OP CODE for thread: %S", &threadName);
				User::Leave(KErrNotSupported);
				}
			}

		__LOG1("CSBESession::ServiceL() - Completed OK for thread: %S", &threadName);
		}

	inline CSBEServer& CSBESession::Server() const
	/**
	Returns a non-cost reference to this CServer object.

	@return The non-const reference to this.
	*/
		{
		return *static_cast<CSBEServer*>(const_cast<CServer2*>(CSession2::Server()));
		}
		
	void CSBESession::PrepDataOwnerInfoL(const RMessage2& aMessage)
	/**
	Prepares the 
	*/
		{
		TInt err = KErrNone;
		ResetTransferBuf();
		RPointerArray<CDataOwnerInfo> doiArray;
		TRAP(err,
			{
			Server().DataOwnerManager().GetDataOwnersL(doiArray);

			iTransferBuf = HBufC8::NewL(KSBBufferBlockSize);
			TPtr8 transferBufPtr(iTransferBuf->Des());
			
			iArrayCount = doiArray.Count();
			
			HBufC8* dataOwnerBuffer = NULL;
			for (TInt index = 0; index < iArrayCount; index++)
				{
				dataOwnerBuffer = doiArray[index]->ExternaliseLC();
				
				if (transferBufPtr.Size() + dataOwnerBuffer->Size() > transferBufPtr.MaxSize())
					{
					iTransferBuf = iTransferBuf->ReAllocL(transferBufPtr.MaxSize() + KSBBufferBlockSize);
					transferBufPtr.Set(iTransferBuf->Des());
					}
					
				// Append the flattened data owner to the IPC transfer buffer	
				transferBufPtr.Append(*dataOwnerBuffer);
				
				CleanupStack::PopAndDestroy(dataOwnerBuffer);
				}
			}
			); // TRAP
		
		doiArray.ResetAndDestroy();
		doiArray.Close();

		User::LeaveIfError(err);
		
		// complete the message with the total size of the buffer so that the 
		// client can create the appropriate sized descriptor for receiving the data
		aMessage.Complete(iTransferBuf->Des().Size());
		}
		
	void CSBESession::ReturnDataOwnerInfoL(const RMessage2& aMessage)
	/**
	Return the previously populated buffer to the client
	@param aMessage The message sent by the client to the server
	*/
		{
		__ASSERT_DEBUG(iTransferBuf, Panic(KErrNotFound));
		
		// return the previously allocated transfer buffer
		aMessage.WriteL(0, *iTransferBuf);		
		aMessage.Complete(iArrayCount);
		
		ResetTransferBuf();
		}
		
	void CSBESession::PrepPublicFileListL(const RMessage2& aMessage)
	/**
	Compile a list of public files owned by a particular SID to be backed up
	@param aMessage The message sent by the client to the server
	*/
		{
		RFileArray fileArray;
		CleanupClosePushL(fileArray);
		HBufC8* pGenericDataTypeBuffer = HBufC8::NewLC(aMessage.GetDesLengthL(1));
		
		TPtr8 genericDataTypeBuffer(pGenericDataTypeBuffer->Des());
		aMessage.ReadL(1, genericDataTypeBuffer);
		
		TDriveNumber drive = static_cast<TDriveNumber>(aMessage.Int0());
		
		CSBGenericDataType* pGenericDataType = CSBGenericDataType::NewL(*pGenericDataTypeBuffer);
		CleanupStack::PopAndDestroy(pGenericDataTypeBuffer);
		CleanupStack::PushL(pGenericDataType);
		Server().DataOwnerManager().GetPublicFileListL(pGenericDataType, drive, fileArray);
		CleanupStack::PopAndDestroy(pGenericDataType);
		
		iTransferBuf = fileArray.ExternaliseL();

		CleanupStack::PopAndDestroy(&fileArray);

		// complete the message with the total size of the buffer so that the 
		// client can create the appropriate sized descriptor for receiving the data
		aMessage.Complete(iTransferBuf->Des().MaxSize());
		}
		
	void CSBESession::ReturnPublicFileListL(const RMessage2& aMessage)
	/** Return the previously populated buffer to the client
	@param aMessage The message sent by the client to the server */
		{
		__ASSERT_DEBUG(iTransferBuf, Panic(KErrArgument));
		
		// return the previously allocated transfer buffer
		aMessage.WriteL(0, *iTransferBuf);
		
		aMessage.Complete(KErrNone);
		
		ResetTransferBuf();
		}

	void CSBESession::PrepPublicFileListRawL(const RMessage2& aMessage)
	/**
	Compile a raw list of public files owned by a particular SID to be backed up
	@param aMessage The message sent by the client to the server
	*/
		{
		RRestoreFileFilterArray fileFilterArray;
		CleanupClosePushL(fileFilterArray);
		TPckgBuf<TDriveNumber> driveNumPkg;
		HBufC8* pGenericDataTypeBuffer = HBufC8::NewLC(aMessage.GetDesLengthL(1));
		
		TPtr8 genericDataTypeBuffer(pGenericDataTypeBuffer->Des());
		
		aMessage.ReadL(0, driveNumPkg);
		aMessage.ReadL(1, genericDataTypeBuffer);
		
		CSBGenericDataType* pGenericDataType = CSBGenericDataType::NewL(*pGenericDataTypeBuffer);
		CleanupStack::PopAndDestroy(pGenericDataTypeBuffer);
		CleanupStack::PushL(pGenericDataType);
		
		Server().DataOwnerManager().GetRawPublicFileListL(pGenericDataType, driveNumPkg(), fileFilterArray);
		CleanupStack::PopAndDestroy(pGenericDataType);
		
		iTransferBuf = fileFilterArray.ExternaliseL();

		CleanupStack::PopAndDestroy(&fileFilterArray);

		// complete the message with the total size of the buffer so that the 
		// client can create the appropriate sized descriptor for receiving the data
		aMessage.Complete(iTransferBuf->Des().MaxSize());
		}
		
	void CSBESession::ReturnPublicFileListRawL(const RMessage2& aMessage)
	/** Return the previously populated buffer to the client
	@param aMessage The message sent by the client to the server */
		{
		__ASSERT_DEBUG(iTransferBuf, Panic(KErrArgument));
		
		// return the previously allocated transfer buffer
		aMessage.WriteL(0, *iTransferBuf);
		
		aMessage.Complete(KErrNone);
		
		ResetTransferBuf();
		}

	void CSBESession::PrepPublicFileListXMLL(const RMessage2& aMessage)
	/**
	*/
		{
		TPckgBuf<TDriveNumber> driveNumPkg;
		TPckgBuf<TSecureId> sidPkg;

		ResetTransferBuf();

		iTransferTextBuf = HBufC::NewL(0);

		Server().DataOwnerManager().GetXMLPublicFileListL(sidPkg(), driveNumPkg(), iTransferTextBuf);
		
		iTransferBuf = HBufC8::NewL(0);
		
		aMessage.Complete(iTransferBuf->Des().MaxSize());
		}
		
	void CSBESession::ReturnPublicFileListXMLL(const RMessage2& aMessage)
	/**
	Return the previously populated buffer to the client
	@param aMessage The message sent by the client to the server
	*/
		{
		__ASSERT_DEBUG(iTransferBuf, Panic(KErrArgument));
		
		// return the previously allocated transfer buffer
		aMessage.WriteL(0, *iTransferTextBuf);

		aMessage.Complete(KErrNone);
		
		ResetTransferBuf();
		}
		
	void CSBESession::SetBURModeL(const RMessage2& aMessage)
	/**	
	Set the Backup and Restore mode on/off and configure the BUR options

	@param aMessage The message sent by the client to the server
	*/
		{
		TDriveList driveList;
		
		aMessage.ReadL(0, driveList);
		TBURPartType burType = static_cast<TBURPartType>(aMessage.Int1());
		TBackupIncType incType = static_cast<TBackupIncType>(aMessage.Int2());
		
		
		// Unlock the global heap
		Server().GSHInterface().ResetHeap(Server().GlobalSharedHeap());

		Server().DataOwnerManager().SetBURModeL(driveList, burType, incType);
		
		aMessage.Complete(KErrNone);
		}

	void CSBESession::SetSIDListForPartialBURL(const RMessage2& aMessage)
	/**
	Set a list of SID's that require base backup regardless of the device backup mode
	
	@param aMessage The message sent by the client to the server
	*/
		{
		HBufC8* flatArray = HBufC8::NewLC(aMessage.GetDesLengthL(0));	
		TPtr8 flatArrayPtr(flatArray->Des());
		
		aMessage.ReadL(0, flatArrayPtr);

		Server().DataOwnerManager().SetSIDListForPartialBURL(flatArrayPtr);

		CleanupStack::PopAndDestroy(flatArray);
		
		aMessage.Complete(KErrNone);
		}

	void CSBESession::PrepSIDStatusL(const RMessage2& aMessage)
	/**
	Extract the array of SID's of which the status is required
	
	@param aMessage The message sent from the client to the server
	*/
		{
		ResetTransferBuf();
		iTransferBuf = HBufC8::NewL(aMessage.GetDesLengthL(0));
		
		TPtr8 transBuf(iTransferBuf->Des());
		// Copy the IPC'd buffer into our transfer buffer
		aMessage.ReadL(0, transBuf);
		
		aMessage.Complete(KErrNone);
		}
		
	void CSBESession::ReturnSIDStatusL(const RMessage2& aMessage)
	/**
	Populate and return the array of SID's complete with their statuses
	
	@param aMessage The message sent from the client to the server
	*/
		{
		RSIDStatusArray* pStatusArray = RSIDStatusArray::InternaliseL(*iTransferBuf);
		CleanupStack::PushL(pStatusArray);
		CleanupClosePushL(*pStatusArray);
		
		// We're finished with the received externalised SIDStatusArray for now, delete it
		ResetTransferBuf();
		
		// Ask DOM to populate the Statuses in the status array
		Server().DataOwnerManager().SIDStatusL(*pStatusArray);
		
		iTransferBuf = pStatusArray->ExternaliseL();
		
		CleanupStack::PopAndDestroy(2, pStatusArray);
		
		aMessage.WriteL(0, *iTransferBuf);		
		aMessage.Complete(KErrNone);

		ResetTransferBuf();
		}
		
	void CSBESession::RequestDataAsyncL(const RMessage2& aMessage)
	/**
	Handle the client's asynchronous request for data from the SBE
	*/
		{
		iMessage = aMessage;
		
        __LOG("CSBESession::RequestDataAsyncL() - START");
		TBool finished = EFalse;	// Set by DOM, initialised to eliminate warning
		HBufC8* pTransferredBuf = HBufC8::NewLC(iMessage.GetDesLengthL(0));
		
		TPtr8 transBuf(pTransferredBuf->Des());
		// Copy the IPC'd buffer into our transfer buffer
		iMessage.ReadL(0, transBuf);
		
		// Extract the generic type from the client IPC request 
		CSBGenericTransferType* pGenericType = CSBGenericTransferType::NewL(*pTransferredBuf);
		CleanupStack::PopAndDestroy(pTransferredBuf);
		CleanupStack::PushL(pGenericType);
		
		TPtr8& writeBuf = Server().GSHInterface().WriteBufferL(Server().GlobalSharedHeap());

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(ETrue);
		
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).GenericTransferTypeBuffer() = pGenericType->Externalise();
		
		// Call the DOM to populate the GSH with the data to return
		Server().DataOwnerManager().RequestDataL(pGenericType, writeBuf, finished);
		
		CleanupStack::PopAndDestroy(pGenericType);
		
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).iFinished = finished;
        __LOG2("CSBESession::RequestDataAsyncL() - Server-side data length: %d, address: 0x%08x", writeBuf.Length(), writeBuf.Ptr());
        //__LOGDATA("CSBESession::RequestDataAsyncL() -       %S", writeBuf.Ptr(), writeBuf.Length());

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);
		
		if (!iMessage.IsNull())
			{
			iMessage.Complete(KErrNone);
			}
		
        __LOG("CSBESession::RequestDataAsyncL() - END");
		}
		
	void CSBESession::RequestDataSyncL(const RMessage2& aMessage)
	/**
	Handle the client's synchronous request for data from the SBE
	*/
		{
        __LOG("CSBESession::RequestDataSyncL() - START");
		TBool finished = EFalse;	// Set by DOM, initialised to eliminate warning
		HBufC8* pTransferredBuf = HBufC8::NewLC(aMessage.GetDesLengthL(0));
		
		TPtr8 transBuf(pTransferredBuf->Des());
		// Copy the IPC'd buffer into our transfer buffer
		aMessage.ReadL(0, transBuf);
		
		// Extract the generic type from the client IPC request 
		CSBGenericTransferType* pGenericType = CSBGenericTransferType::NewL(*pTransferredBuf);
		CleanupStack::PopAndDestroy(pTransferredBuf);
		CleanupStack::PushL(pGenericType);
		
		TPtr8& writeBuf = Server().GSHInterface().WriteBufferL(Server().GlobalSharedHeap());

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(ETrue);

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).GenericTransferTypeBuffer() = pGenericType->Externalise();
		
		// Call the DOM to populate the GSH with the data to return
		Server().DataOwnerManager().RequestDataL(pGenericType, writeBuf, finished);
		
		CleanupStack::PopAndDestroy(pGenericType);
		
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).iFinished = finished;

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);
        __LOG2("CSBESession::RequestDataSyncL() - Server-side data length: %d, address: 0x%08x", writeBuf.Length(), writeBuf.Ptr());
        //__LOGDATA("CSBESession::RequestDataSyncL()        - %S", writeBuf.Ptr(), writeBuf.Length());

		aMessage.Complete(KErrNone);
        __LOG("CSBESession::RequestDataSyncL() - END");
		}
	
	void CSBESession::SupplyDataSyncL(const RMessage2& aMessage)
	/**
	Handle the client's synchronous request to supply data to the SBE
	*/
		{
        __LOG("CSBESession::SupplyDataSyncL() - START");
		TBool finished = aMessage.Int0();

		CSBGenericTransferType* pGenericType = CSBGenericTransferType::NewL(
			Server().GSHInterface().Header(Server().GlobalSharedHeap())
			.GenericTransferTypeBuffer());
		CleanupStack::PushL(pGenericType);

		TPtrC8& readBuf = Server().GSHInterface().ReadBufferL(Server().GlobalSharedHeap());

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(ETrue);
        __LOG2("CSBESession::SupplyDataSyncL() - Server-side data length: %d, address: 0x%08x", readBuf.Length(), readBuf.Ptr());
        //__LOGDATA("CSBESession::SupplyDataSyncL()         - %S", readBuf.Ptr(), readBuf.Length());

		Server().DataOwnerManager().SupplyDataL(pGenericType, readBuf, finished);

		
		CleanupStack::PopAndDestroy(pGenericType);

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);

		aMessage.Complete(KErrNone);
        __LOG("CSBESession::SupplyDataSyncL() - END");
		}
		
	void CSBESession::GetExpectedDataSizeL(const RMessage2& aMessage)
	/**
	Return the size of data that the DOM expects to send back to the PC
	*/
		{
		TUint size;
		TPckg<TUint> sizePkg(size);

		TInt bufferLen = aMessage.GetDesLengthL(0);
		HBufC8* transTypeBuf = HBufC8::NewL(bufferLen);
		CleanupStack::PushL(transTypeBuf);

		TPtr8 transTypeBufDes(transTypeBuf->Des());
		aMessage.ReadL(0, transTypeBufDes);

		CSBGenericTransferType* pGenericType = CSBGenericTransferType::NewL(transTypeBufDes);
		CleanupStack::PopAndDestroy(transTypeBuf);
		CleanupStack::PushL(pGenericType);
		
		aMessage.ReadL(1, sizePkg);
		
		Server().DataOwnerManager().GetExpectedDataSizeL(pGenericType, sizePkg());
		CleanupStack::PopAndDestroy(pGenericType);	
	
		aMessage.WriteL(1, sizePkg);

		aMessage.Complete(KErrNone);
		}
		
	void CSBESession::AllSnapshotsSuppliedL(const RMessage2& aMessage)
	/**
	*/
		{
		Server().DataOwnerManager().AllSnapshotsSuppliedL();

		aMessage.Complete(KErrNone);
		}
		
	void CSBESession::AllSystemFilesRestoredL()
	/**
	Sent by the client to indicate that all registration files 
	@param aMessage IPC message sent from the client
	*/
		{
		Server().DataOwnerManager().AllSystemFilesRestoredL();
		}

	void CSBESession::PrepLargePublicFileListL(const RMessage2& aMessage)
	/**
	Compile a list of public files owned by a particular SID to be backed up
	@param aMessage The message sent by the client to the server
	*/
		{
		HBufC8* pGenericDataTypeBuffer = HBufC8::NewLC(aMessage.GetDesLengthL(1));		
		TPtr8 genericDataTypeBuffer(pGenericDataTypeBuffer->Des());
		
		TDriveNumber drive = static_cast<TDriveNumber>(aMessage.Int0());
		aMessage.ReadL(1, genericDataTypeBuffer);
		TInt cursor = aMessage.Int2();
		TInt maxSize = aMessage.Int3();
		
		CSBGenericDataType* pGenericDataType = CSBGenericDataType::NewL(*pGenericDataTypeBuffer);
		CleanupStack::PopAndDestroy(pGenericDataTypeBuffer);
		CleanupStack::PushL(pGenericDataType);
		
		// Create a transfer type (ignored by the client side, just here for future reference
		CSBGenericTransferType* transType = NULL;
		
		switch (pGenericDataType->DerivedTypeL())
			{
			case ESIDDerivedType:
				{
				
				CSBSecureId* sid = CSBSecureId::NewL(pGenericDataType);
				CleanupStack::PushL(sid);
				transType = CSBSIDTransferType::NewL(sid->SecureIdL(), drive, EPublicFileListing);
				CleanupStack::PopAndDestroy(sid);
				break;
				}
			case EPackageDerivedType:
				{
				CSBPackageId* pkg = CSBPackageId::NewL(pGenericDataType);
				CleanupStack::PushL(pkg);
				transType = CSBPackageTransferType::NewL(pkg->PackageIdL(), drive, ESystemData);
				CleanupStack::PopAndDestroy(pkg);
				break;
				}
			case EJavaDerivedType:
				{
				CSBJavaId* java = CSBJavaId::NewL(pGenericDataType);
				CleanupStack::PushL(java);
				transType = CSBJavaTransferType::NewL(java->SuiteHashL(), drive, EJavaMIDletData);
				CleanupStack::PopAndDestroy(java);
				break;
				}
			default:
				{
				__LOG1("Unknown generic data type supplied, leaving with KErrUnknown (%d)", KErrUnknown);
				User::Leave(KErrUnknown);
				}
			}
		CleanupStack::PushL(transType);
		
		// Get a writeable buffer from the global shared heap
		TPtr8& writeBuf = Server().GSHInterface().WriteBufferL(Server().GlobalSharedHeap());
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(ETrue);
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).GenericTransferTypeBuffer() = transType->Externalise();

		// This is required to look up the MIME type of the file entry and convert it into a textual representation	
		RApaLsSession apaSession;
		User::LeaveIfError(apaSession.Connect());
		CleanupClosePushL(apaSession);

		// Copy file entries from the cursor until either the end of the list is reached or the 
		// max response size ceiling is reached. Store the number added for the call to ReturnLargePublicFileListL
		iFinished = EFalse;
		TInt currentIndex = cursor;
		TInt currentSize = 0;
		iTotalEntries = 0;

		if (currentIndex == 0)
			{
			// Starting from the beginning, remove previously saved entry
			delete iExtCurEntry;
			iExtCurEntry = 0;
			}

		while (currentSize < maxSize)
			{
			if (!iExtCurEntry)
				{
				TEntry entry;
				Server().DataOwnerManager().GetNextPublicFileL(pGenericDataType, (currentIndex == 0), drive, entry);
				if (entry.iName.Length())
					{
					++currentIndex;
								
					// Pull the next entry out of the list and translate into CSBEFileEntry
					CSBEFileEntry* pEntry = CSBEFileEntry::NewLC(entry, apaSession);
					
					// Pack it into the GSH
					iExtCurEntry = pEntry->ExternaliseLC();
					CleanupStack::Pop(iExtCurEntry);
		
					CleanupStack::PopAndDestroy(pEntry);
					}
				}
			// else - use the remaining entry from the previous call
			
			if (iExtCurEntry)
				{
				// If this entry isn't going to bust our max RAM parameter, then pack it in
				currentSize += iExtCurEntry->Size();
				if (currentSize <= maxSize)
					{
					writeBuf.Append(*iExtCurEntry);
					delete iExtCurEntry;
					iExtCurEntry = 0;
					++iTotalEntries;
					}
				// else - leave iExtCurEntry until the next call
				}
			else
				{
				// No more entries left, must have reached the end of the list
				iFinished = ETrue;
				break;					
				}
			}
		CleanupStack::PopAndDestroy(&apaSession);
        __LOG2("CSBESession::PrepLargePublicFileListL() - Server-side data length: %d, address: 0x%08x", writeBuf.Length(), writeBuf.Ptr());
		
		Server().GSHInterface().Header(Server().GlobalSharedHeap()).iFinished = iFinished;

		Server().GSHInterface().Header(Server().GlobalSharedHeap()).SetLockedFlag(EFalse);

		CleanupStack::PopAndDestroy(transType);
		CleanupStack::PopAndDestroy(pGenericDataType);

		aMessage.Complete(KErrNone);
		}
		
	void CSBESession::ReturnLargePublicFileListL(const RMessage2& aMessage)
	/** Return the previously populated buffer to the client
	@param aMessage The message sent by the client to the server */
		{
		TPckg<TBool> finPkg(iFinished);
		TPckg<TInt> totalPkg(iTotalEntries);
		aMessage.WriteL(0, finPkg);
		aMessage.WriteL(1, totalPkg);
		aMessage.Complete(KErrNone);
		}
				
	void CSBESession::ResetTransferBuf()
	/**
	*/
		{
		if (iTransferBuf != NULL)
			{
			delete iTransferBuf;
			iTransferBuf = NULL;
			}

		if (iTransferTextBuf != NULL)
			{
			delete iTransferTextBuf;
			iTransferTextBuf = NULL;	
			}
			
		iArrayCount = 0;
		}
	}
