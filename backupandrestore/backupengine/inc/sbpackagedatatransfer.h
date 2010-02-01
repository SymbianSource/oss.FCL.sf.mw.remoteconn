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
* Declaration of CPackageDataTransfer
* 
*
*/



/**
 @file
*/

#ifndef __PACKAGEDATATRANSFER_H__
#define __PACKAGEDATATRANSFER_H__

// System
#include <e32base.h>
#include <connect/sbtypes.h>
#include <swi/sisregistryentry.h>
#include <swi/sisregistrysession.h>
#include <swi/backuprestore.h>

// MContentHandler mix in class
#include <xml/contenthandler.h>

#include "sbebufferhandler.h"
#include "sbedataownermanager.h"
#include "sbedataowner.h"

namespace conn
	{
	using namespace Swi;
	// Forwards
	class CSnapshotHolder;
	
	/** Class to store the state of the data owner
    @internalComponent
    */
    class TPDState
    	{
    public:
        TState          	iState; /*<! The current state of the DOM */
        TDriveNumber    	iDriveNumber; /*<! The drive number for the current request */
        TPackageDataType    iTransferType; /*<! The transfer type of the current request */
    	};

	/** A Package data owner
	
	@internalComponent
	*/
	class CPackageDataTransfer : public CBase, public MContentHandler
		{
	public:
		static CPackageDataTransfer* NewL(TUid aPid, CDataOwnerManager* aDOM);
		static CPackageDataTransfer* NewLC(TUid aPid, CDataOwnerManager* aDOM);
		~CPackageDataTransfer();
		
		static TInt Compare(const CPackageDataTransfer& aFirst, const CPackageDataTransfer& aSecond);
		static TBool Match(const CPackageDataTransfer& aFirst, const CPackageDataTransfer& aSecond);
		
		void GetExpectedDataSizeL(TPackageDataType aTransferType, TDriveNumber aDriveNumber, TUint& aSize);
	    void SupplyDataL(TDriveNumber aDriveNumber, TPackageDataType aTransferType, TDesC8& aBuffer,
	                     TBool aLastSection);
	    void RequestDataL(TDriveNumber aDriveNumber, TPackageDataType aTransferType, TPtr8& aBuffer,
	    				  TBool& aLastSection);
		TUid PackageId() const;
		TCommonBURSettings CommonSettingsL();
	    TPassiveBURSettings PassiveSettingsL();
	    TActiveBURSettings ActiveSettingsL();
	    void GetDriveListL(TDriveList& aDriveList);
	    void ParseL();
	    void SetRegistrationFileL(const TDesC& aFileName);
	    
	    void GetPublicFileListL(TDriveNumber aDriveNumber, RFileArray& aFiles);
	    void GetRawPublicFileListL(TDriveNumber aDriveNumber, RRestoreFileFilterArray& aRestoreFileFilter);
	    
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
		CPackageDataTransfer(TUid aPid, CDataOwnerManager* aDOM);
		void ConstructL();
		
		void BuildPackageFileList();
		void WriteData(TAny* aItem, TPtr8& aBuffer, TInt aSize);
		void ReadData(TAny* aItem, const TDesC8& aBuffer, TInt aSize);
	    void DoSupplyDataL(TDriveNumber aDriveNumber, const TDesC8& aBuffer, TBool aLastSection);
		void SupplySnapshotL(TDriveNumber aDriveNumber, const TDesC8& aBuffer, TBool aLastSection);
	    void DoRequestDataL(TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection);
		void RequestSnapshotL(TDriveNumber aDriveNumber, TPtr8& aBuffer, TBool& aLastSection);
		void SupplyFileDataL( const TDesC8& aBuffer, TBool aLastSection);
		void Cleanup();
		TBool IsDataOnDrive(TDriveNumber aDrive);
		
		void ParseDirL(const TDesC& aDirName, const RArray<TPtrC>& aExclude, RFileArray& apFileEntries);
		TBool IsExcluded(const TBool aIsPublic, const TDesC& aFileName, const RArray<TPtrC>& aExclude);
		
		// Element handlers 
  		void HandleBackupRegistrationL(const RAttributeArray& aAttributes);
  		void HandlePathL(const TSelectionType aType, const RAttributeArray& aAttributes, const TBool aDir);
  		
  		TInt HandlePublicBackup(const RAttributeArray& aAttributes);
  		TInt HandleSystemBackup(const RAttributeArray& aAttributes);
  		
	private:
		TUint8					iGotFileName; /*<! Have we got the filename */
		TUint8					iFixedHeaderRead; /*<! Have we got the header information */
		TUint8					iFileNameRead; /*<! Is the filename read */
		RFile					iFileHandle; /*<! The file handle */
		TUint8					iFileOpen; /*<! Is the file open */
		TFileFixedHeader 		iFixedHeader; /*<! The fixed header */
		HBufC*			 		iFileName; /*<! The file name */
		HBufC*					iTempFileName; /*<! A tempory filename */
		TPDState              	iState; /*<! The state of the object */
		RFs						iFs; /*<! File system handle to use */
		CBufferSnapshotReader* 	iBufferSnapshotReader; /*<! Handles writing snapshots to the buffer */
		CBufferFileWriter*  	iBufferFileWriter; /*<! Handles reading files from the buffer */
		CBufferSnapshotWriter* 	iBufferSnapshotWriter; /*<! Handles reading snapshots from the buffer */
		TUid					iPackageID; /*<! The package id */
		RRestoreSession			iSWIRestore; /*<! Handle to software restore */
		RBackupSession			iSWIBackup; /*<! Handle to backup session */
		
        // Snapshots
	    CSnapshotHolder*		iSnapshot; /*<! Holds a tempory snapshot */
	    TDriveList			 	iDriveList; /*<! Drives on which pages exists */
	    RPointerArray<HBufC>	iFiles; /*<! System files */
	    HBufC8*					iMetaData; /*<! Meta data for a package */
	    TPublicInformation		iPublicInformation; /*<! Public backup information */
	    TSystemInformation		iSystemInformation; /*<! System backup information */
	    
	    ///////// FOR Parsing ////////////////
	 	
	 	/* Simple enumeration: What is our current element?
		*/
		enum TCurrentElement 	{ENoElement = 0, // No element 
								 EPassive = 1, // Passive element 
								 EPublic = 2 // Public element
								};
		
		TCurrentElement			iCurrentElement; /*<! Current element */
		
		RSelections				iPublicSelections; /*<! Public selections */
		HBufC*					iRegistrationFile; /*<! Path to registration file */
		CDataOwnerManager*		ipDataOwnerManager; /*<! To access resources */
		TUint8					iRestored; /*<! Used for speed optimization if we already restored this package */
		TUint					iBytesRead; /*<! Number of bytes read */
		TInt 					iMetaDataSize; /*<! Meta Data Size */
		TInt 					iMetaDataLeft; /*<! Meta Data Size left to write */
		};
	} // namespace conn

			

#endif __PACKAGEDATATRANSFER_H__
