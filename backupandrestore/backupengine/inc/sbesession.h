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
* Declaration of CSBESession class.
* 
*
*/



/**
 @file
*/
#ifndef __SBESESSION_H__
#define __SBESESSION_H__

namespace conn
	{
	class CDataOwnerInfo;
	class CSBESession : public CSession2
	/**
    @internalComponent
	@ingroup Server

	A SBE client Session.
	*/
		{
		public:
			CSBESession();
			virtual void CreateL();

		private:
			~CSBESession();
			inline CSBEServer& Server() const;
			void ServiceL(const RMessage2& aMessage);

			void PrepDataOwnerInfoL(const RMessage2& aMessage);
			void ReturnDataOwnerInfoL(const RMessage2& aMessage);
			void PrepPublicFileListL(const RMessage2& aMessage);
			void ReturnPublicFileListL(const RMessage2& aMessage);
			void PrepPublicFileListRawL(const RMessage2& aMessage);
			void ReturnPublicFileListRawL(const RMessage2& aMessage);
			void PrepPublicFileListXMLL(const RMessage2& aMessage);
			void ReturnPublicFileListXMLL(const RMessage2& aMessage);
			void SetBURModeL(const RMessage2& aMessage);
			void SetSIDListForPartialBURL(const RMessage2& aMessage);
			void PrepSIDStatusL(const RMessage2& aMessage);
			void ReturnSIDStatusL(const RMessage2& aMessage);
			void RequestDataAsyncL(const RMessage2& aMessage);
			void RequestDataSyncL(const RMessage2& aMessage);
			void SupplyDataSyncL(const RMessage2& aMessage);
			void GetExpectedDataSizeL(const RMessage2& aMessage);
			void AllSnapshotsSuppliedL(const RMessage2& aMessage);
			void AllSystemFilesRestoredL();
			void PrepLargePublicFileListL(const RMessage2& aMessage);
			void ReturnLargePublicFileListL(const RMessage2& aMessage);
			
			void ResetTransferBuf();

		private:
			/** Heap allocated descriptor for storing an externalised array upon a 
			2 stage server-client descriptor send */
			HBufC8* iTransferBuf;
			
			/** Heap allocated descriptor for storing a text string upon a 
			2 stage server-client descriptor send */
			HBufC* iTransferTextBuf;

			/** Used in conjunction with iTransferBuf when passing back CDataOwnerInfo array 
			so that message is completed on the second IPC call with the num of elements in
			returned externalised array */
			TInt iArrayCount;
			
			/** Message member used for asynchronous IPC calls */
			RMessage2 iMessage;
			
			/** Finished flag used for storing data in a large public file list 2-part IPC call */
			TBool iFinished;
			
			/** Count used for large public file list 2-part IPC call */
			TInt iTotalEntries;
			
			/** Saves the externalised current entry for packing into the next message */
			HBufC8* iExtCurEntry;
		};
	}

#endif //__SBESESSION_H__
