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
// Implementation of sbeconfig
// 
//

/**
 @file
*/
#include <e32std.h>
#include <connect/panic.h>
#include "sbeconfig.h"
#include "sblog.h"
#include <xml/parser.h>

namespace conn
	{
	const TInt KSIDLength = 8;
	
	// XML type
	_LIT8(KMimeType, "text/xml");
	
	// elements
	_LIT8(KConfig, "sbe_config");
	_LIT8(KHeap, "heap");
	_LIT8(KCentRep, "central_repository");
	_LIT8(KDrives, "exclude_drives");
	_LIT8(KAppCloseDelay, "app_close_delay");
	
	_LIT8(KSize, "size");
	_LIT8(KUid, "uid");
	_LIT8(KList, "list");
	
	_LIT8(KRedFact, "reduction_factor");
	_LIT8(KMaxRetries, "max_retries");
	
	_LIT8(KDelay, "delay");
	
	// default setting if no file found
	const TInt KSBEGSHDefaultSize = 2097152;
	const TInt KSBEGSHReductionFactor = 2;
	const TInt KSBEGSHMaxRetries = 5;
	
	const TInt KMinHeapSize = 131072;
	
	const TInt KDefaultDelay = 0;
	
	_LIT_SECURE_ID(KCentRepSID,0x10202BE9);
	_LIT(KConfigFile, "sbeconfig.xml");
	
	/**
	Symbian Constructor
	@param RFs& reference to RFs
	@return CSBEConfig* pointer to CSBEConfig
	*/
	CSBEConfig* CSBEConfig::NewL(RFs& aRFs)
		{
		CSBEConfig* self = new (ELeave) CSBEConfig(aRFs);
		return self;
		}
	
	/**
	C++ Constructor
	*/
	CSBEConfig::CSBEConfig(RFs& aRFs) : iRFs(aRFs), iFileName(KConfigFile), iConfigTagVisited(EFalse)
		{
		SetDefault();
		}
	/** 
	Destructor
	*/
	CSBEConfig::~CSBEConfig()
		{
		delete iConverter;
		}
	
	/**
	Heap Values
	@param TInt& aMaxSize of the heap to try to allocate
	@param TInt& aReductionFactor in case allocation fail
	@param TInt& number of retries to try to reduce the heap by the ReductionFactor
	*/
	void CSBEConfig::HeapValues(TInt& aMaxSize, TInt& aReductionFactor, TInt& aMaxRetries) const
		{
		aMaxSize = iSBEGSHMaxSize;
		aReductionFactor = iReductionFactor;
		aMaxRetries = iMaxRetries;
		}
		
	/**
	Secure Id for central repository, needed deprecated use of centrep tag in xml
	@return TSecureId& aSecureId
	*/	
	TSecureId CSBEConfig::CentRepId() const
		{
		return iCentRepId;
		}
	
	/**
	Exclude list of drives from backup/restore
	@return TDriveList& aDriveList
	*/	
	const TDriveList& CSBEConfig::ExcludeDriveList() const
		{
		return iDrives;
		}
	
	/**
	Extra time delay to close all non-system apps 
	@return TInt& iAppCloseDelay
	*/
	TUint CSBEConfig::AppCloseDelay() const
		{
		return iAppCloseDelay;
		}

	/**
	Set the values to Defaults
	*/	
	void CSBEConfig::SetDefault()
		{
		iSBEGSHMaxSize = KSBEGSHDefaultSize;
		iCentRepId = KCentRepSID;
		iDrives.SetLength(KMaxDrives);
		iDrives.FillZ();
		iDrives[EDriveZ] = ETrue;
		iReductionFactor = KSBEGSHReductionFactor;
		iMaxRetries = KSBEGSHMaxRetries;
		iAppCloseDelay = KDefaultDelay;
		}
		
	/**
	Method to convert string of drives (eg. cdez) to member variable TDriveList
	@param const TDesC8& reference to string
	*/	
	TInt CSBEConfig::StringToDrives(const TDesC8& aDes)
		{
		iDrives.SetLength(KMaxDrives);
		iDrives.FillZ();
		
		TInt err = KErrNone;
		TInt length = aDes.Length();
		for (TInt i = 0; i < length; ++i)
			{
			TInt pos;
			err = iRFs.CharToDrive(aDes.Ptr()[i], pos);
			if (err != KErrNone)
				{
				break;
				}
			iDrives[pos] = ETrue;
			}
		return err;
		}
	
	/**
	Parses the config file if found
	@leave with System wide Error Codes
	*/	
	void CSBEConfig::ParseL()
		{
		iRFs.PrivatePath(iFileName);
		TFindFile findFile(iRFs);
		User::LeaveIfError(findFile.FindByPath(KConfigFile, &iFileName));
		
		iFileName = findFile.File();
		// Connect to the parser
		CParser* parser = CParser::NewLC(KMimeType, *this);
		
		// Parse the file
		Xml::ParseL(*parser, iRFs, iFileName);
		
		CleanupStack::PopAndDestroy(parser);
		}
		
	/**
	A method to handle attributes
	@param RAttributeArray& aAttributes 
	@return TInt System Wide Error
	*/	
	TInt CSBEConfig::HandleAttributesElement(const RAttributeArray& aAttributes)
		{
		TInt err = KErrNone;
		// Loop through reading out attribute values
		const TUint count = aAttributes.Count();
		for (TInt x = 0; x < count && err == KErrNone; x++)
			{
			TPtrC8 attrib = aAttributes[x].Attribute().LocalName().DesC();
			TPtrC8 value = aAttributes[x].Value().DesC();
			if (!attrib.CompareF(KDelay))
				{
				TLex8 lex(value);
				TInt appCloseDelay = 0;
				err = lex.Val(appCloseDelay);
				if (appCloseDelay < 0)
					{
					__LOG("CSBEConfig::HandleAttributesElement() - Configuration Error: the time delay is negative");
					err = KErrCorrupt;
					}
				else
					{
					iAppCloseDelay = appCloseDelay;
					}
				}
			if (!attrib.CompareF(KRedFact))
				{
				TLex8 lex(value);
				err = lex.Val(iReductionFactor);
				if (iReductionFactor < 0)
					{
					__LOG("CSBEConfig::HandleAttributesElement() - Configuration Error: the reductionFactor is negative");
					err = KErrCorrupt;
					}
				}
			else if (!attrib.CompareF(KMaxRetries))
				{
				TLex8 lex(value);
				err = lex.Val(iMaxRetries);
				if (iMaxRetries < 0)
					{
					__LOG("CSBEConfig::HandleAttributesElement() - Configuration Error: the maxRetries is negative");
					err = KErrCorrupt;
					}
				}
			if (!attrib.CompareF(KSize))
				{
				TLex8 lex(value);
				err = lex.Val(iSBEGSHMaxSize);
				if (iSBEGSHMaxSize < KMinHeapSize)
					{
					__LOG1("CSBEConfig::HandleAttributesElement() - Configuration Error: heap size is less then minimum %d", KMinHeapSize);
					err = KErrCorrupt;
					}
				} // if
			else if (!attrib.CompareF(KUid))
				{
				TLex8 lex;
				if (value.Length() >= KSIDLength)
					{
					lex = value.Right(KSIDLength);
					err = lex.Val(iCentRepId.iId, EHex);
					if (iCentRepId.iId == 0)
						{
						err = KErrCorrupt;
						}
					}
				if (err != KErrNone)
					{
					__LOG("CSBEConfig::HandleAttributesElement() - Configuration Error: central_repostiory is NOT a HEX number");
					err = KErrCorrupt;
					}
				} // else if
			else if (!attrib.CompareF(KList))
				{
				err = StringToDrives(value);
				if (err != KErrNone)
					{
					__LOG("CSBEConfig::HandleAttributesElement() - Configuration Error: list doesn't have valid characters from a-z");
					}
				} // else if
				
			} // for x
		return err;
		}
		
// From MContentHandler
	void CSBEConfig::OnStartDocumentL(const RDocumentParameters& /*aDocParam*/, TInt /*aErrorCode*/)
	/**
	Start of the document, creates Character Converter to convert from/to unicode
	
	@see MContentHandler::OnStartDocumentL()
	@leave if fails to set encoding
	*/
		{
		// Create a converter for converting strings to Unicode
		iConverter = CCnvCharacterSetConverter::NewL();

		// We only convert from UTF-8 to UTF-16
		if (iConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUtf8, iRFs) == CCnvCharacterSetConverter::ENotAvailable)
			{
			User::Leave(KErrNotFound);
			}
		}
		
	void CSBEConfig::OnEndDocumentL(TInt /*aErrorCode*/)
	/**
	End of document. destroys converter object
	
	@see MContentHandler::OnEndDocumentL()
	*/
		{
		// We've finished parsing the document, hence destroy the converter object
		delete iConverter;
		iConverter = NULL;
		}
		
	void CSBEConfig::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt /*aErrCode*/)
	/**
	Element to parse on the start
	
	@see MContentHandler::OnStartElementL()
	
	@param aElement RTagInfo&
	@param aAttributes RAttributeArray&
	*/
		{
		TInt err = KErrNone;
		TPtrC8 localName(aElement.LocalName().DesC());
		if (!localName.CompareF(KConfig))
			{
			iConfigTagVisited = ETrue;
			} // if
		else if (iConfigTagVisited)
			{
			if (!localName.CompareF(KHeap) || !localName.CompareF(KCentRep) || !localName.CompareF(KDrives) || !localName.CompareF(KAppCloseDelay))
				{
				err = HandleAttributesElement(aAttributes);
				} // if
			else
				{
				err = KErrCorrupt;
				} // else if
			} // else if
		else
			{
			err = KErrCorrupt;
			}
		User::LeaveIfError(err);
		}
		
	void CSBEConfig::OnEndElementL(const RTagInfo& /*aElement*/, TInt /*aErrorCode*/)
	/**
	Element to parse at the end
	
	@see MContentHandler::OnEndElementL()
	@param const aElement RTagInfo&
	*/
		{
		}
		
	void CSBEConfig::OnContentL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnContentL()
	*/
		{
		}
		
	void CSBEConfig::OnStartPrefixMappingL(const RString& /*aPrefix*/, const RString& /*aUri*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnStartPrefixMappingL()
	*/
		{
		}
		
	void CSBEConfig::OnEndPrefixMappingL(const RString& /*aPrefix*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnEndPrefixMappingL()
	*/
		{
		}
		
	void CSBEConfig::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnIgnorableWhiteSpaceL()
	*/
		{
		}
		
	void CSBEConfig::OnSkippedEntityL(const RString& /*aName*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnSkippedEntityL()
	*/
		{
		}
		
	void CSBEConfig::OnProcessingInstructionL(const TDesC8& /*aTarget*/, const TDesC8& /*aData*/, TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnProcessingInstructionL()
	*/
		{
		}
		
	void CSBEConfig::OnError(TInt /*aErrorCode*/)
	/** 
	@see MContentHandler::OnError()
	*/
		{
		}
		
	TAny* CSBEConfig::GetExtendedInterface(const TInt32 /*aUid*/)
	/** 
	@see MContentHandler::GetExtendedInterface()
	*/
		{
		return NULL;
		}
		
	
	}
