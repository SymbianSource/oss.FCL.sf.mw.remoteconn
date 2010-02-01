/**
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Declaration of CDataOwnerManager
* 
*
*/



/**
 @file
*/
#ifndef __DATAOWNERMANAGER_H__
#define __DATAOWNERMANAGER_H__

// System
#include <e32base.h>
#include <connect/sbtypes.h>
#include <f32file.h>
#include <e32property.h>
#include <swi/sisregistrysession.h>
#include <badesca.h>

#include "sbeconfig.h"
#include "sbeparserproxy.h"
#include <connect/javamanagerinterface.h>


// Forwards
class CBaBackupSessionWrapper;
class CARC4;



namespace conn
	{
	// Forwards
	class CDataOwner;
	class CABServer;
	class CBufferFileReader;
	class CPackageDataTransfer;
	class CDataOwnerManager;
	class CJavaDataOwnerManager;
	class CSecureBUREncryptKeySource;
	class CSBEDecompressAndEncrypt;
	
    /** The current request state 
    @internalComponent
    */
    enum TState 
    	{
    	ENone, /*<! The DataOwnerManager is not requesting or supplying */ 
    	ERequest, /*<! The DataOwnerManager is currently dealing with a request */
    	ESupply, /*<! The DataOwnerManager is currently dealing with a supply */
    	EBuffer /*<! The DataOwnerManager is currently dealing with buffering */
    	};

	// Globals
	_LIT(KStar, "*");
	_LIT(KPrivate, "\\private\\");
	_LIT(KBackSlash, "\\");
	_LIT(KColon, ":");
	_LIT(KBackupRegistrationFile, "backup_registration*.xml");
	_LIT(KPrimaryBackupRegistrationFile, "backup_registration.xml");
	_LIT(KImportDir, "\\private\\10202D56\\import\\packages\\");

	/**
		Simple container to map Secure ID's to there assocaited Data Owner
		@internalComponent
	*/
    class CDataOwnerContainer : public CBase
    	{
    	public:
    		static CDataOwnerContainer* NewL(TSecureId aSid, CDataOwnerManager* apDataOwnerManager);
    		~CDataOwnerContainer();
    		
    		static TInt Compare(const CDataOwnerContainer& aFirst, const CDataOwnerContainer& aSecond);
			static TBool Match(const CDataOwnerContainer& aFirst, const CDataOwnerContainer& aSecond);
	    		
    		TSecureId SecureId() const;
    		CDataOwner& DataOwner() const;
        private:
        	CDataOwnerContainer(TSecureId aSID);
        	void ConstructL(CDataOwnerManager* apDataOwnerManager);
    	private:
        	TSecureId       iSecureId; /*<! Contains the secure ID */
        	CDataOwner*     ipDataOwner; /*<! Contains the data owner.*/
    	};
    	
    /** The state of the data owner manager
    @internalComponent
    */
    class TDOMState
    	{
	public:
		TState          	iState; /*<! The current state of the DataOwnerManager */
		TSecureId       	iSID; /*<! The secure ID we are currently working with */
		};

	/**
	Manager for data owners.
	@internalTechnology
	*/
	class CDataOwnerManager : public CBase
		{
    	
	public:
	    // Construction
	    static CDataOwnerManager* NewLC();
	    ~CDataOwnerManager();
	    
	    // Methods
	    void SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
						TBackupIncType aBackupIncType);
	    void GetDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners);
	    void GetExpectedDataSizeL(CSBGenericTransferType* aGenericTransferType, TUint& aSize);
	    void GetPublicFileListL(CSBGenericDataType* aGenericDataType, TDriveNumber aDriveNumber, RFileArray& aFiles);
	    void GetRawPublicFileListL(CSBGenericDataType* aGenericDataType, TDriveNumber aDriveNumber, 
	    						   RRestoreFileFilterArray& aRestoreFileFilter);
	    void GetXMLPublicFileListL(TSecureId aSID, TDriveNumber aDriveNumber, HBufC*& aBuffer);
	    
	    void SetSIDListForPartialBURL(TDesC8& aFlatArrayPtr);
	    void SIDStatusL(RSIDStatusArray& aSIDStatus);
	    void AllSnapshotsSuppliedL();
	    void GetNextPublicFileL(CSBGenericDataType* aGenericDataType, TBool aReset,
	                           TDriveNumber aDriveNumber, TEntry& aEntry);
	    
	    // Global heap 
	    void SupplyDataL(CSBGenericTransferType* aGenericTransferType, TDesC8& aBuffer, 
	    				 TBool aLastSection);
	    void RequestDataL(CSBGenericTransferType* aGenericTransferType,
	    	    		  TPtr8& aBuffer, TBool& aLastSection);
	                         
	    // Accessors
	    void SetActiveBackupServer(CABServer* aABServer);
	    RFs& GetRFs();
	    CABServer& ABServer();
	    CDataOwner& DataOwnerL(TSecureId aSID);
	    
	    TDriveList& DriveList();
	    TBURPartType BURType() const;
	    TBackupIncType IncType() const;
		void AllSystemFilesRestoredL();
		
		CSBEConfig& Config();
		inline CSBEParserProxy& ParserProxy();
		TBool IsSetForPartialL(TSecureId aSecureId) const;

	private:
		// Constructor
		void ConstructL();
		CDataOwnerManager();
		
		// Methods
		void FindDataOwnersL();
		CDataOwnerContainer* FindL(TSecureId aSID);
		CPackageDataTransfer* FindPackageDataContainerL(TUid pid);
		void StripSecureIdL(const TDesC& aStrip, TSecureId& aSecureId);
		void UpdateDataOwnersPartialStateL();
		void FindImportPackagesL(Swi::RSisRegistrySession& aRegistry, RPointerArray<CDataOwnerInfo>& aDataOwners);
		void FindRegistrationFilesL(const TDesC& aPath, CDesCArray& aFiles);
	private:
	    RPointerArray<CDataOwnerContainer>  iDataOwners; /*<! An array of TDataOwnerContainers */
		RPointerArray<CPackageDataTransfer> iPackageDataOwners; /*<! An array of package data transfer objects */
	    TDOMState           iState; /*!< The state of the DataOwnerManager */
	    
	    // Current backup options (set in SetBURModeL -> needed for active clients)
	    TDriveList			iDriveList; /*<! Drive list for backup */
	    TBURPartType		iBURType; /*<! Backup/restore type */
	    TBackupIncType		iIncType; /*<! Incrementatl type */
	    
	    // Resources
	    CABServer*          ipABServer; /*!< The active backup server */
	    RFs					iFs; /*!< A RFs used in this class and all DataOwners */
	    CBufferFileReader*	iBufferFileReader; /*<! Used to restore registration files */
	    CSBEDecompressAndEncrypt* iDecompressor;
	    
	    /** Set to true if we need to reset the device after a restore */
	    TBool				iResetAfterRestore;
	    
	    /** Object to handle Java data owners */
		CJavaManagerInterface* iJavaDOM;
	    
	    /** List of Secure IDs for Partial Restore */
	    RSIDArray* iSIDListForPartial;
	    
	    /** Config for SBE */
	    CSBEConfig* iConfig;
	    
	    /** Base Backup Session Wrapper */
	    CBaBackupSessionWrapper* iBaBackupSession;
	    
	    /** Backup Registration Parser Proxy */
	    CSBEParserProxy*		 iParserProxy;
		};
		
    inline TDriveList& CDataOwnerManager::DriveList()
    /**
    Return the list of drives specified for the operation
    @return Drivelist
    */
    	{
    	return iDriveList;
    	}
    	
    inline TBURPartType CDataOwnerManager::BURType() const
    /**
    Return the partial backup type
    @return The partial backup type
    */
    	{
    	return iBURType;
    	}
    	
    inline TBackupIncType CDataOwnerManager::IncType() const
    /**
    Return the incremental backup type
    @return The incremental backup type
    */
    	{
    	return iIncType;
    	}
    
    /**
    Return Active Backup Server
    @return reference to Active Backup Server
    */	
	inline CABServer& CDataOwnerManager::ABServer()
		{
		return *ipABServer;
		}
		
	/**
	Return Parser Proxy
	@return reference to Parser Proxy
	*/
	inline CSBEParserProxy& CDataOwnerManager::ParserProxy()
		{
		return *iParserProxy;
		}
    	
	} // namespace conn
#endif // __DATAOWNERMANAGER_H__
