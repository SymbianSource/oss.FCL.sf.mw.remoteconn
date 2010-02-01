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
* SBE Config.
* 
*
*/



/**
 @file
*/

#ifndef __SBECONFIG_H__
#define __SBECONFIG_H__

#include <e32base.h>
#include <f32file.h>
#include <xml/contenthandler.h> // MContentHandler mix in class
#include <charconv.h>
using namespace Xml;

/**
@namespace conn

This namespace is the global Symbian Connect namespace which encapsulates 
all of the connectivity components within Symbian OS.
*/

namespace conn
	{
	/**
	SBEConfig class is designed to read data from the ini file in order to make configurable the following data:
	Global Shared Heap 
	Drives not to Backup Up
	Secure Id of Central Repository - in order to support deprecated flag (cent_rep) in registration file

	@internalTechnology
	*/
	class CSBEConfig : public CBase, public MContentHandler
		{
	public:
		static CSBEConfig* NewL(RFs& aRFs);
		void ParseL();
		void SetDefault();
		~CSBEConfig();
		
	public:
	// From MContentHandler
		void OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode);
		void OnEndDocumentL(TInt aErrorCode);
		void OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrCode);
		void OnEndElementL(const RTagInfo& aElement, TInt aErrorCode);
		void OnContentL(const TDesC8& aBytes, TInt aErrorCode);
		void OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode);
		void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);
		void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);
		void OnSkippedEntityL(const RString& aName, TInt aErrorCode);
		void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode);
		void OnError(TInt aErrorCode);
		TAny* GetExtendedInterface(const TInt32 aUid);
// End MContentHandler

	public:
		//getters
		void HeapValues(TInt& aMaxSize, TInt& aReductionFactor, TInt& aMaxRetries) const;
		TSecureId CentRepId() const ;
		const TDriveList& ExcludeDriveList() const ;
		TUint AppCloseDelay() const ;
		
	private:
		CSBEConfig(RFs& aRFs);
		TInt StringToDrives(const TDesC8& aDes);
		
		TInt HandleAttributesElement(const RAttributeArray& aAttributes);
		
	private:
		/** reference to RFs */
		RFs& iRFs;
		
		/** Config File Name */
		TFileName iFileName;
		
		/** Config Tag visited */
		TBool iConfigTagVisited;
		
		/** Character converter to convert between UTF-8 and UTF-16 */
		CCnvCharacterSetConverter* iConverter;
		
		/** SBE Global Share Heap Size */
		TInt iSBEGSHMaxSize;
		
		/** Drives Not to Backup */
		TDriveList iDrives;
		
		/** Cent Rep Secure Id */
		TSecureId iCentRepId;
		
		/** Reduction factor if allocation fails */
		TInt iReductionFactor;
		
		/** Number of retries if allocation fails */
		TInt iMaxRetries;
		
		/** Extra time delay to close all non-system apps */
		TUint iAppCloseDelay;
		};
	
	}// end of namespace
		
	
#endif //__SBECONFIG_H__
