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
* Declaration of CDataOwner
* 
*
*/



/**
 @file
*/
#ifndef __DATAOWNER_H__
#define __DATAOWNER_H__

/// System
#include <e32base.h>
#include <connect/sbtypes.h>
#include <f32file.h>
#include <xml/contenthandler.h> // MContentHandler mix in class

// Other
#include "sbeparserproxy.h"
#include "sbebufferhandler.h"
#include "sbedataownermanager.h"

using namespace Xml;

namespace conn
	{
	
	void CleanupRPointerArray(TAny* aPtr);
	
	/** Name of Central Repository Server
	@internalTechnology
	*/
	
	_LIT(KCentRepProcessName, "centralrepositorysrv.exe");
	_LIT8(KYes, "yes");
	
	const TTimeIntervalMicroSeconds32 KABCallbackDefaultTimeout(200000000);
	
	// Forwards
	class CABServer;
	class CDataOwnerManager;
	class CBufferFileWriter;
	class CBufferFileReader;
	class CBufferSnapshotWriter;
	class CBufferSnapshotReader;
	
	/** Enumeration to specify if a selection should be included or excluded
        @internalComponent
	*/
	enum TSelectionType 
		{
		EInclude, /*<!< Is included */
		EExclude /*!< Is excluded */
		};
		
	
	/** Simple class to store registraion file selections
        @internalComponent
	*/
	class CSelection : public CBase
		{
	public:
		static CSelection* NewLC(TSelectionType aType, const TDesC& aSelection);
		~CSelection();
		TSelectionType SelectionType() const;
		const TDesC& SelectionName() const;
		
	private:
		CSelection(TSelectionType aType);
		void ConstructL(const TDesC& aSelection);
		
	private:
    	TSelectionType		iType; /*<! The type of the selection */
    	HBufC*		       	iSelection; /*<! The actual selection */
		};
	
	/** An RArray of selections
		@internalComponent
	*/
	typedef RPointerArray<CSelection> RSelections;
	
	/** Simple class to store passive information
        @internalComponent
	*/
	class TPassiveInformation
		{
	public:
		TPassiveInformation() :
			iSupported(EFalse),
			iSupportsSelective(EFalse),
			iDeleteBeforeRestore(EFalse),
			iBaseBackupOnly(EFalse)
		/** C++ Constructor
		*/
			{}
	public:
		TUint8		iSupported; /*<! supported */
		TUint8		iSupportsSelective; /*<! Supports selective */
		TUint8		iDeleteBeforeRestore; /*<! Delete before restore */
		TUint8		iBaseBackupOnly; /*<! Base backup */
		};
	
	/** Simple class to store public backup information
        @internalComponent
	*/
	class TPublicInformation
		{
	public:
		TPublicInformation() :
			iSupported(EFalse), iDeleteBeforeRestore(EFalse)
			/** C++ Constructor
			*/
			{}
	public:
		TUint8		iSupported; /*<! supported */
		TUint8		iDeleteBeforeRestore; /*<! Delete before restore */
		};
		
	/** Simple class to store system backup information
        @internalComponent
	*/
	class TSystemInformation
		{
	public:
		TSystemInformation() :
			iSupported(EFalse)
		/** C++ Constructor
		*/
			{}
	public:
		TUint8	iSupported; /*!< backup system files */
		};
		
	/** Information about proxy data owners (i.e. CentRep etc.)
	@internalComponent
	*/	
	class TProxyInformation
		{
	public:
		/** C++ Constructor
		*/
		TProxyInformation() : iDataRequested(EFalse), iDataSupplied(EFalse), iOpInProgress(EFalse)
			{}
	public:
		TSecureId 	iSecureId; /*!< The secure ID of the proxy data manager */
		TInt		iDataRequested; /*!< Data has already been requested from this data owner. While restoring it is used for Proxy data length*/
		TInt 		iDataSupplied; /*!< Data has already been supplied to this data owner. While restoring it is used for consumed/restored data length.  */
		TBool		iOpInProgress; /*!< The proxy hasn't yet completed the op and is waiting a further call. While restoring it is used for proxy finished flag and 1 means proxy data finished 0 means has more data to store  */
		};
		
	/** Simple class to store restore information
        @internalComponent
	*/
	class TRestoreInformation
		{
	public:
		/** C++ Constructor
		*/
		TRestoreInformation() :
			iSupported(EFalse), iRequiresReboot(EFalse)
			{};
	public:
		TUint8	iSupported; /*<! supported */
		TUint8	iRequiresReboot; /*<! requires reboot */
		};
		
	/** enum representing TActiveType */
    enum TActiveType
    	{
    	EActiveOnly = 0, /*<! Data Owner have Active Implementaion only */
    	EActiveAndProxyImpl = 1, /*<! Data Owner have Active and Proxy Implementation */
    	EProxyImpOnly = 2, /*<! Data Owner have only Proxy Implementation */
    	};
	
	/** Simple class to store active information
        @internalComponent
	*/
	class TActiveInformation
		{
	public:
		/** C++ Constructor
		*/
		TActiveInformation() :
			iSupported(EFalse), iRequiresDelayToPrepareData(EFalse),
			iSupportsSelective(EFalse), iSupportsIncremental(ETrue), iActiveDataOwner(EFalse), iActiveType(EActiveOnly), iCallbackDelayTime(KABCallbackDefaultTimeout)
				{
				}
	public:
		TUint8 	iSupported; /*<! supported? */
		TName	iProcessName; /*<! Active process name */
		TUint8	iRequiresDelayToPrepareData; /*<! Requires delay to prepare data */
		TUint8	iSupportsSelective; /*<! Supports selective */
		TUint8	iSupportsIncremental; /*<! Supports Incremental */
		TUint8	iActiveDataOwner; /*<! Although we say we're active, we're not really */
		TActiveType	iActiveType; /*<! Type of the Active DO */
		TTimeIntervalMicroSeconds32  iCallbackDelayTime; /*<! Callback Delay Time */
		};
		
	/** Data Owner support classes and enums */
	
	/**
	This class holds the state of the data owner by the drive
	@internalComponent
	*/
	class TDataOwnerStateByDrive
		{
	public:
		TDataOwnerStateByDrive(TDriveNumber aDrive) : iDrive(aDrive), iPassiveSnapshotReceived(EFalse),
	    	iPassiveBaseDataReceived(EFalse), iPassiveIncDataReceived(EFalse), 
	    	iPassiveSnapshotRequested(EFalse), iPassiveBaseDataRequested(EFalse), 
	    	iPassiveIncDataRequested(EFalse), iActiveSnapshotReceived(EFalse),
	    	iActiveBaseDataReceived(EFalse), iActiveIncDataReceived(EFalse), 
	    	iActiveSnapshotRequested(EFalse), iActiveBaseDataRequested(EFalse), 
	    	iActiveIncDataRequested(EFalse), iFirstActiveTransaction(ETrue),
	    	iDeleteBeforeRestorePerformed(EFalse), iOpInProgress(EFalse)
	    	{}
		
	public:
		TDriveNumber iDrive; /*!< Record whether any snapshot data has been received */
	    TUint8 iPassiveSnapshotReceived; /*!< Record whether any snapshot data has been received */
	    TUint8 iPassiveBaseDataReceived; /*!< Record whether or not any data has been received */
	    TUint8 iPassiveIncDataReceived; /*!< Record whether or not any data has been received */
	    TUint8 iPassiveSnapshotRequested; /*!< Record whether any snapshot data has been requested */
	    TUint8 iPassiveBaseDataRequested; /*!< Record whether or not any data has been requested */
	    TUint8 iPassiveIncDataRequested; /*!< Record whether or not any data has been requested */
	    TUint8 iActiveSnapshotReceived; /*!< Record whether any snapshot data has been received */
	    TUint8 iActiveBaseDataReceived; /*!< Record whether or not any data has been received */
	    TUint8 iActiveIncDataReceived; /*!< Record whether or not any data has been received */
	    TUint8 iActiveSnapshotRequested; /*!< Record whether any snapshot data has been requested */
	    TUint8 iActiveBaseDataRequested; /*!< Record whether or not any data has been requested */
	    TUint8 iActiveIncDataRequested; /*!< Record whether or not any data has been requested */
	    TUint8 iFirstActiveTransaction; /*!< Is this the first of an active transaction sequence */
	    TUint8 iDeleteBeforeRestorePerformed; /*!< Have we performed the delete before restore */
 	    TUint8 iOpInProgress; /*!< The active data owner hasn't yet completed the op and is waiting a further call */
		};


	/**
	This class holds the state of each proxy for a particular drive
	@internalComponent
	*/
	class TProxyStateByDrive
		{
	public:
		TProxyStateByDrive(TDriveNumber aDrive, TInt aProxy) : iDrive(aDrive), iProxy(aProxy),
			iDataRequested(EFalse), iDataSupplied(EFalse), iOpInProgress(EFalse)
	    	{}
		
	public:
		TDriveNumber iDrive; /*!< The drive that this state relates to */
		TInt iProxy; /*!< The proxy that this state relates to */
		TUint8 iDataRequested; /*!< Data has already been requested from this data owner */
		TUint8 iDataSupplied; /*!< Data has already been supplied to this data owner */
		TUint8 iOpInProgress; /*!< The proxy hasn't yet completed the op and is waiting a further call */
		};


    /** Class to store the state of the data owner
    @internalComponent
    */
    class TDOState
    	{
    public:
        TState          	iState; /*<! The current state of the DOM */
        TDriveNumber    	iDriveNumber; /*<! The drive number for the current request */
        TTransferDataType   iTransferType; /*<! The transfer type of the current request */
    	};
	    

    /** Class representing the snapshot
    @internalComponent
    */
    class CSnapshotHolder : public CBase
    	{
    public:
    	// Constructors
    	static CSnapshotHolder* NewL();
    	static CSnapshotHolder* NewLC();
    	~CSnapshotHolder();
	    	
    	// Members
    	TDriveNumber	iDriveNumber;
    	RSnapshots		iSnapshots;
    private:
    	CSnapshotHolder();
    	};
    	
	/** A data owner

	@internalTechnology
	*/
	class CDataOwner : public CBase, public MContentHandler, public MValidationHandler
		{
	public:
	    static CDataOwner* NewL(TSecureId aSID, CDataOwnerManager* apDataOwnerManager);
		static CDataOwner* NewLC(TSecureId aSID, CDataOwnerManager* pDataOwnerManager);
	    ~CDataOwner();
	    

	    // Methods
	    void AddRegistrationFilesL(const TDesC& aFileName);
	    void ParseFilesL();
	    void GetExpectedDataSizeL(TTransferDataType aTransferType, TDriveNumber aDriveNumber, TUint& aSize);
	    void GetPublicFileListL(TDriveNumber aDriveNumber, RFileArray& aFiles);
	    void GetRawPublicFileListL(TDriveNumber aDriveNumber, RRestoreFileFilterArray& aRestoreFileFilter);
	    void SupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TDesC8& aBuffer,
	                     TBool aLastSection);
	    void RequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, TPtr8& aBuffer,
	    				  TBool& aLastSection);
		void RestoreCompleteL();
		void GetNextPublicFileL(TBool aReset, TDriveNumber aDriveNumber, TEntry& aEntry);
   				  
	    // Accessors
	    TSecureId SecureId() const;
	    TDataOwnerStatus ReadyState();
		void SetReadyState(TDataOwnerStatus aDataOwnerStatus);
	    TCommonBURSettings CommonSettingsL();
	    TPassiveBURSettings PassiveSettingsL();
	    TActiveBURSettings ActiveSettingsL();
	    void GetDriveListL(TDriveList& aDriveList);
	    void SetBackedUpAsPartial(TBool aPartial);
	    TBool PartialAffectsMe() const;
		void StartProcessIfNecessaryL();
		void BuildDriveStateArrayL();
		void DisableSystemData();
		TActiveInformation ActiveInformation();
		TBool ValidFileL(const TDesC& aFileName);
		
		// MContentHandler
		void OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode);
		void OnEndDocumentL(TInt aErrorCode);
		void OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode);
		void OnEndElementL(const RTagInfo& aElement, TInt aErrorCode);
		void OnContentL(const TDesC8& aBytes, TInt aErrorCode);
		void OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode);
		void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);
		void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);
		void OnSkippedEntityL(const RString& aName, TInt aErrorCode);
		void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode);
		void OnError(TInt aErrorCode);
		TAny* GetExtendedInterface(const TInt32 aUid);
		// MContentHandler
		
	private:
	    // Constructors
	    CDataOwner(TSecureId aSID, CDataOwnerManager* apDataOwnerManager);
	    void ConstructL();
		
		// Methods
		void PrivatePathL(const TDesC& aFileName);
		void ParseFileL(const TDesC& aFileName);
		TInt GetDrive(const TDesC& aPath) const;
		void BuildFileListL(const RSelections& aFileSelection, 
							const TDriveNumber aDriveNumber,
							const TTransferDataType aTransferType,
							const TBool aIsPublic,
							RSnapshots* apSnapshot,
							RFileArray* apFileEntries,
							CDesCArray* apFileNames);
		void AddDBMSFilesL(TDriveNumber aDriveNumber, 
						   CDesCArray* apFileNames, 
						   RFileArray* apEntries);
		void ParseDirL(const TDesC& aDirName, 
					   const RArray<TPtrC>& aExclude, 
					   const TTransferDataType aTransferType,
					   const TBool aIsPublic,
					   RSnapshots* apSnapshots,
					   RFileArray* apFileEntries, 
					   CDesCArray* apFileNames);
		TBool IsExcluded(const TBool aIsPublic, const TDesC& aFileName, const RArray<TPtrC>& aExclude);
		void IsNewerL(const TDesC& aFileName, const TEntry& aFile, const RSnapshots* aSnapshots, TBool& aNewer);
		RSnapshots* FindSnapshot(TDriveNumber aDriveNumber);
		void ResetState();
		TDataOwnerStateByDrive& StateByDriveL(TDriveNumber& aDrive);
		TProxyStateByDrive& ProxyStateByDriveL(TDriveNumber& aDrive, TInt aProxy);
		void CleanupBeforeRestoreL(TDriveNumber& aDriveNumber);
		
		// Supply
		void SupplyPassiveSnapshotDataL(TDriveNumber aDriveNumber, TDesC8& aBuffer, TBool aLastSection);
		void SupplyPassiveBaseDataL(TDriveNumber aDriveNumber, TDesC8& aBuffer, TBool aLastSection);
		
		// Request
		void RequestPassiveSnapshotDataL(TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection);
		void RequestPassiveDataL(TTransferDataType aTransferType, TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection);
		void ProcessRequestDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
    			TPtr8& aBuffer, TBool& aLastSection);
    	void ProcessSupplyDataL(TDriveNumber aDriveNumber, TTransferDataType aTransferType, 
				TDesC8& aBuffer, TBool aLastSection);
		TInt AddProxyToList(TProxyInformation aProxy);
		
				
		// private methods for parsing
	private:
		// Element handlers 
  		void HandleBackupRegistrationL(const RAttributeArray& aAttributes);
  		void HandlePathL(const TSelectionType aType, const RAttributeArray& aAttributes, const TBool aDir);
  		TInt HandlePassiveBackup(const RAttributeArray& aAttributes);
  		TInt HandlePublicBackup(const RAttributeArray& aAttributes);
  		TInt HandleSystemBackup(const RAttributeArray& aAttributes);
  		TInt HandleCenrepBackup(const RAttributeArray& aAttributes);
 		TInt HandleProxyDataManager(const RAttributeArray& aAttributes);
  		TInt HandleDBMSBackupL(const RAttributeArray& aAttributes);
  		TInt HandleActiveBackupL(const RAttributeArray& aAttributes);
  		TInt HandleRestore(const RAttributeArray& aAttributes);
	private:
		// Internal
	    TDOState           				iState; /*<! The state of the data owner */
	    TDataOwnerStatus				iStatus; /*!< Is the DO ready for operations - only relevant for active */
	    CDesCArray*						iRegistrationFiles; /*<! A list of registration files for this data owner */
	    TUint8							iFilesParsed; /*<! Have we parsed the registration files */
	    TUint8							iPrimaryFile; /*<! Have we found a primary file */
	    TUint8							iBackupAsPartial; /*!< Will this DO be backed up as partial */
	    
	    // Options
	    TSecureId            			iSecureId; /*<! The data owners SID */
	    TPassiveInformation				iPassiveInformation; /*<! Passive backup information */
	    TPublicInformation				iPublicInformation; /*<! Public backup information */
	    TSystemInformation				iSystemInformation; /*<! System backup information */
	    TActiveInformation				iActiveInformation; /*<! Active information */
	    TRestoreInformation				iRestoreInformation; /*<! restore information */
	    RArray<TProxyInformation>		iProxyInformationArray; /*!< The various proxies that we support */
	    TBufC<KMaxPackageNameLength>	iName; /*<! The data owners nice name */
	    RArray<TDataOwnerStateByDrive>	iStateByDrive; /*<! Array storing the state of the DO's by drive */
	    RArray<TProxyStateByDrive>		iProxyStateByDrive; /*<! Array storing the state of the proxies for each drive */
		RArray<TUid>					iDBMSSelections; /*<! Array storing the list of DBMS selections */
	    
	    // Reader\Writer handlers
	    CBufferFileWriter*				iBufferFileWriter; /*<! Handles writing files to the buffer */
	    CBufferFileReader*				iBufferFileReader; /*<! Handles reading files from the buffer */
	    CBufferSnapshotWriter*			iBufferSnapshotWriter; /*<! Handles writing snapshots to the buffer */
	    CBufferSnapshotReader*			iBufferSnapshotReader; /*<! Handles reading snapshots from the buffer */
	    
	    // Selections
	    RSelections						iPassiveSelections; /*<! Passive selections */
	    RSelections						iPublicSelections; /*<! Public selections */

        // Snapshots
	    CSnapshotHolder*				iTempSnapshotHolder; /*<! Hold a temporary snapshot */
	    RPointerArray<CSnapshotHolder>	iSnapshots; /*<! Stores the snapshots we have been sent */
	    
	    // Resources
	    CDataOwnerManager*				ipDataOwnerManager; /*<! To access resources */
	 	HBufC*							iPrivatePath; /*<! The path to the private directory */
	 	
	 	/* Simple enumeration: What is our current element?
		*/
		enum TCurrentElement 			{ENoElement = 0, // No element 
										 EPassive = 1, // Passive element 
										 EPublic = 2 // Public element
										};
		
		TCurrentElement					iCurrentElement; /*<! Current element */
	 
	 	RArray<RDir>					iPublicDirStack; /*<! Stack of public directories for enumerating public files */
	 	RPointerArray<HBufC>			iPublicDirNameStack; /*<! Directory names corresponding to entries in the above stack */
		RArray<TPtrC>					iPublicExcludes; /*<! Stores list of excludes while enumerating public files */
		TInt							iPublicFileIndex; /*<! Index of current element being enumerated in public file list */
		TInt 							iCurrentProxy;  // Used to restore the proxy data
	 	
		};
		
	}
#endif // __DATAOWNER_H__
