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
* Declaration of CABServer class.
* 
*
*/



/**
 @file
*/

#ifndef __ABSERVER_H__
#define __ABSERVER_H__

#include <e32base.h>
#include <connect/sbtypes.h>

namespace conn
	{

	/** @defgroup ABServer Active Backup Server Documentation
	*/

	/** Constant for defining an unset Secure ID 
	@internalTechnology
	*/
	const TUint32 KNullABSid = 0;
	
	class CDataOwnerManager;
	class CABSessionMap;

	class CABServer :	public CPolicyServer

	/**
    @internalComponent
	@ingroup ABServer
	
	Active Backup Server to support Active Backup sessions
	*/
		{
		public:
			~CABServer();
			static CABServer* NewLC(CDataOwnerManager* aDOM);
			void AddSession();
			void DropSession();
			void RemoveElement(TSecureId aSecureId);
			CDataOwnerManager& DataOwnerManager() const;
			void SupplyDataL(TSecureId aSID, TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
				TDesC8& aBuffer, TBool aLastSection, TBool aSuppressInitDataOwner = EFalse, 
				TSecureId aProxySID = KNullABSid);
			void RequestDataL(TSecureId aSID, TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
				TPtr8& aBuffer, TBool& aLastSection, TBool aSuppressInitDataOwner = EFalse, 
				TSecureId aProxySID = KNullABSid);
		    void GetExpectedDataSizeL(TSecureId aSID, TDriveNumber aDriveNumber, TUint& aSize);
		    void AllSnapshotsSuppliedL(TSecureId aSID);
		    TDataOwnerStatus SessionReadyStateL(TSecureId aSID);
		    void RestoreCompleteL(TSecureId aSID, TDriveNumber aDrive);
		    void InvalidateABSessions();//Invalidate the CABSession for each ab client
		    
		private:
			CABServer(CDataOwnerManager* aDOM);
			void ConstructL();
			
			//
			// From CServer2
			virtual CSession2* NewSessionL(const TVersion& aVersion,
				const RMessage2& aMessage) const;
			
			TInt RunError(TInt aError);
			void PanicClient(TInt aPanic) const;

		private:
			/** Active client sessions. */
			TInt iSessionCount;
			
			/** Pointer to the Data Owner Manager */
			CDataOwnerManager* iDOM;
			
			/** Pointer to the session map */
			CABSessionMap* iSessionMap;
		};
		
	inline CDataOwnerManager& CABServer::DataOwnerManager() const
	/**
	Return a reference to the data owner manager
	
	@return Reference to the data owner manager object
	*/
		{
		return *iDOM;
		}
		
	}

#endif //__ABSERVER_H__
