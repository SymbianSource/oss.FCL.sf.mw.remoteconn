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
// Implementation of CSBEServer class.
// 
//

/**
 @file
*/

#include <e32std.h>
#include <e32base.h>
#include "sbeclientserver.h"
#include "sbeserver.h"
#include "sbesession.h"
#include "sbepanic.h"
#include "sbedataownermanager.h"
#include "sblog.h"
//#include <stdlib.h>

namespace conn
	{


	/** Secure Backup Engine security request ranges
	
	This is a breakdown of the SBE requests into ranges
	for security checking purposes.

	@internalComponent
	*/
	const TInt mySBERanges[] = {0,23};

	/** Secure Backup Engine range count

	The number of different security ranges for SBEs request numbers

	@internalComponent
	*/
	const TUint mySBERangeCount = sizeof(mySBERanges)/sizeof(mySBERanges[0]);

	/** Secure Backup Engine security action array

	An array with a one-to-one mapping with the range array
	specifiying what security action to take for each server request.

	@internalComponent
	*/
	const TUint8 mySBEElementsIndex[mySBERangeCount] =
		{
		0,
		CPolicyServer::ENotSupported,
		};

	/**
	@internalComponent
	*/
	const CPolicyServer::TPolicyElement mySBEElements[] =
		{
			{_INIT_SECURITY_POLICY_C2(ECapabilityWriteDeviceData,ECapabilityReadDeviceData)
				, CPolicyServer::EFailClient},
		};

	/**
	@internalComponent
	*/
	const CPolicyServer::TPolicy mySBEPolicy =
		{
		0,
		mySBERangeCount,
		mySBERanges,
		mySBEElementsIndex,
		mySBEElements,
		};

	CSBEServer::CSBEServer(CDataOwnerManager* aDOM)
		: CPolicyServer(EPriorityNormal,mySBEPolicy), iDOM(aDOM)
    /** 
    Class constructor
    */
		{
		__ASSERT_DEBUG(aDOM, Panic(KErrArgument));
		}

	CSBEServer::~CSBEServer()
    /**
    Class destructor
    */
		{
		iGlobalSharedHeap.Close();
		delete iGSHInterface;
		}
		
	CSBEServer* CSBEServer::NewLC(CDataOwnerManager* aDOM)
	/**
	Constructs a new instance of the CSBEServer, calls ConstructL, 
	and returns it to the caller leaving it on the cleanup stack.

	@return The new instance of CSBEServer.
	*/
		{
		CSBEServer* pSelf = new (ELeave) CSBEServer(aDOM);
		CleanupStack::PushL(pSelf);
		pSelf->ConstructL();
		return pSelf;
		}

	void CSBEServer::ConstructL()
	/**
	Construct this instance of CSBEServer.
	*/
		{
		AllocateGlobalSharedHeapL();

		iGSHInterface = CHeapWrapper::NewL();
		
		// Initialise the locked flag to be unlock
		iGSHInterface->ResetHeap(iGlobalSharedHeap);
		//
		// Start the server and a timer to stop it if nothing happens.
		StartL(KSBEServerName);
		iShutdown.ConstructL();
		#ifndef _DEBUG
			iShutdown.Start();
		#endif
		}
		
	void CSBEServer::AllocateGlobalSharedHeapL()
	/**
	Attempts to allocate the GSH. If initial attempts fail, it tries to allocate with
	progressively smaller chunk sizes
	*/
		{	
		TInt attemptedSize;
		TInt retryCount;
		TInt redFactor;
		iDOM->Config().HeapValues(attemptedSize, retryCount, redFactor);
		
		TInt result = KErrNone;
		
		for (; retryCount > 0; retryCount--)
			{
			result = iGlobalSharedHeap.CreateGlobal(KNullDesC, attemptedSize, attemptedSize);
			
			if (result == KErrNone)
				{
				// We have succesfully allocated a GSH
				break;
				}
			else
				{
				// Reduce the size of the GSH by a scale factor
				attemptedSize = attemptedSize / redFactor;
				}
			}
			
		User::LeaveIfError(result);
		}

	void CSBEServer::AddSession()
	/** Increments the server session count.
	
	The server will shutdown when its 
	session count drops to zero.
	*/
		{
		++iSessionCount;
		iShutdown.Cancel();
		}

	void CSBEServer::DropSession()
	/** Decrements the server session count.  
	
	The server will shutdown when its 
	session count drops to zero.
	*/
		{		
		if(--iSessionCount == 0)
			{
			iShutdown.Start();
			}
		}


	CSession2* CSBEServer::NewSessionL(const TVersion& aVersion,
		const RMessage2& /*aMessage*/) const
	/** Constructs a new SBE server session.
	
	Querys the supplied version infomation from the client
	with that of this server, and leaves if they are incompatable.

	@param aVersion The clients version information
	@param aMessage Is ignored
	@return A new instance of CSBESession
	@leave KErrNotSupported if the version passed in aVersion is not the same as this one
	*/
		{
		TVersion thisVersion(KSBEMajorVersionNumber, 
								KSBEMinorVersionNumber,
								KSBEBuildVersionNumber);
		
	    if (!User::QueryVersionSupported(thisVersion, aVersion))
			{
			User::Leave(KErrNotSupported);
			}

		return new (ELeave) CSBESession();
		}

	TInt CSBEServer::RunError(TInt aError)
	/** Called when this active objects RunL leaves. 
	
	May be due to a bad client or the server itself.  In either 
	case, complete the last outstanding message with the error 
	code and continue handling client requests.

    @param aError  Standard Symbian OS error code
	@return The error code to be passed back to the active scheduler framework.
	*/
		{
		//
		// A Bad descriptor is a bad client - panic it.
		if(aError == KErrBadDescriptor)
			{
			PanicClient(KErrBadDescriptor);
			}

		//
		// Complete the message and continue handling requests.
		Message().Complete(aError);
		ReStart();
		return KErrNone;
		}


	void CSBEServer::PanicClient(TInt aPanic) const
	/** Panic a client.

	@param aPanic The panic code.
	*/
		{
		__DEBUGGER()
		_LIT(KPanicCategory,"SBE Server");
		RThread client;
		Message().Client(client);
		client.Panic(KPanicCategory, aPanic);
		}

	} // end namespace
