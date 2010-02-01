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
* Declaration of CSBEServer class.
* 
*
*/



/**
 @file
*/

#ifndef __SBESERVER_H__
#define __SBESERVER_H__

#include "sbshutdown.h"
#include "sbheapwrapper.h"

namespace conn
	{

	/** @defgroup SBEServer SBE Server Documentation
	*/

	class CSBShutdown;
	class CDataOwnerManager;
	

	class CSBEServer :	public CPolicyServer
	/** The SBE Server.
    @internalComponent
	@ingroup SBEServer
	*/
		{
		public:
			~CSBEServer();
			static CSBEServer* NewLC(CDataOwnerManager* aDOM);
			void AddSession();
			void DropSession();
			inline RChunk& GlobalSharedHeap();
			inline CHeapWrapper& GSHInterface();
			inline CDataOwnerManager& DataOwnerManager();

		private:
			CSBEServer(CDataOwnerManager* aDOM);
			void ConstructL();
			
			//
			// From CServer2
			virtual CSession2* NewSessionL(const TVersion& aVersion,
				const RMessage2& aMessage) const;
			
			TInt RunError(TInt aError);
			void PanicClient(TInt aPanic) const;
			void AllocateGlobalSharedHeapL();

		private:
			/** Active client sessions. */
			TInt iSessionCount;
			
			/** Shutdown timer. */
			CSBShutdown iShutdown;
			
			/** Global shared heap for passing large amounts of data between client and server
			without having to use IPC */
			RChunk iGlobalSharedHeap;
			
			/** Interface to the global shared heap */
			CHeapWrapper* iGSHInterface;
			
			/** Pointer to the Data owner manager */
			CDataOwnerManager* iDOM;
		};

		inline RChunk& CSBEServer::GlobalSharedHeap()
		/* Get the global shared heap
		
		@return the global shared heap
		*/
			{
			return iGlobalSharedHeap;
			}
			
		inline CHeapWrapper& CSBEServer::GSHInterface()
			/** Get the global shared heap interface
			
			@return the global shared heap interface
			*/
			{
			return *iGSHInterface;
			}
			
		inline CDataOwnerManager& CSBEServer::DataOwnerManager()
			/* Get the data owner manager
			
			@return the data owner manager.
			*/
			{
			
			
			return *iDOM;
			}
	}

#endif //__SBESERVER_H__
