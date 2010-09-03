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
#include "sbepanic.h"
#include "sbeconfig.h"
#include <xml/parser.h>
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbeconfigTraces.h"
#endif

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
		OstTraceFunctionEntry0( CSBECONFIG_NEWL_ENTRY );
		CSBEConfig* self = new (ELeave) CSBEConfig(aRFs);
		OstTraceFunctionExit0( CSBECONFIG_NEWL_EXIT );
		return self;
		}
	
	/**
	C++ Constructor
	*/
	CSBEConfig::CSBEConfig(RFs& aRFs) : iRFs(aRFs), iFileName(KConfigFile), iConfigTagVisited(EFalse)
		{
		OstTraceFunctionEntry0( CSBECONFIG_CSBECONFIG_CONS_ENTRY );
		SetDefault();
		OstTraceFunctionExit0( CSBECONFIG_CSBECONFIG_CONS_EXIT );
		}
	/** 
	Destructor
	*/
	CSBEConfig::~CSBEConfig()
		{
		OstTraceFunctionEntry0( CSBECONFIG_CSBECONFIG_DES_ENTRY );
		delete iConverter;
		OstTraceFunctionExit0( CSBECONFIG_CSBECONFIG_DES_EXIT );
		}
	
	/**
	Heap Values
	@param TInt& aMaxSize of the heap to try to allocate
	@param TInt& aReductionFactor in case allocation fail
	@param TInt& number of retries to try to reduce the heap by the ReductionFactor
	*/
	void CSBEConfig::HeapValues(TInt& aMaxSize, TInt& aReductionFactor, TInt& aMaxRetries) const
		{
		OstTraceFunctionEntry0( CSBECONFIG_HEAPVALUES_ENTRY );
		aMaxSize = iSBEGSHMaxSize;
		aReductionFactor = iReductionFactor;
		aMaxRetries = iMaxRetries;
		OstTraceFunctionExit0( CSBECONFIG_HEAPVALUES_EXIT );
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
		OstTraceFunctionEntry0( CSBECONFIG_SETDEFAULT_ENTRY );
		iSBEGSHMaxSize = KSBEGSHDefaultSize;
		iCentRepId = KCentRepSID;
		iDrives.SetLength(KMaxDrives);
		iDrives.FillZ();
		iDrives[EDriveZ] = ETrue;
		iReductionFactor = KSBEGSHReductionFactor;
		iMaxRetries = KSBEGSHMaxRetries;
		iAppCloseDelay = KDefaultDelay;
		OstTraceFunctionExit0( CSBECONFIG_SETDEFAULT_EXIT );
		}
		
	/**
	Method to convert string of drives (eg. cdez) to member variable TDriveList
	@param const TDesC8& reference to string
	*/	
	TInt CSBEConfig::StringToDrives(const TDesC8& aDes)
		{
		OstTraceFunctionEntry0( CSBECONFIG_STRINGTODRIVES_ENTRY );
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
		OstTraceFunctionExit0( CSBECONFIG_STRINGTODRIVES_EXIT );
		return err;
		}
	
	/**
	Parses the config file if found
	@leave with System wide Error Codes
	*/	
	void CSBEConfig::ParseL()
		{
		OstTraceFunctionEntry0( CSBECONFIG_PARSEL_ENTRY );
		iRFs.PrivatePath(iFileName);
		TFindFile findFile(iRFs);
		TInt err = findFile.FindByPath(KConfigFile, &iFileName);
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CSBECONFIG_PARSEL, "Leave: %d", err));
		
		iFileName = findFile.File();
		// Connect to the parser
		CParser* parser = CParser::NewLC(KMimeType, *this);
		
		// Parse the file
		Xml::ParseL(*parser, iRFs, iFileName);
		
		CleanupStack::PopAndDestroy(parser);
		OstTraceFunctionExit0( CSBECONFIG_PARSEL_EXIT );
		}
		
	/**
	A method to handle attributes
	@param RAttributeArray& aAttributes 
	@return TInt System Wide Error
	*/	
	TInt CSBEConfig::HandleAttributesElement(const RAttributeArray& aAttributes)
		{
		OstTraceFunctionEntry0( CSBECONFIG_HANDLEATTRIBUTESELEMENT_ENTRY );
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
				    OstTrace0(TRACE_NORMAL, CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: the time delay is negative");
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
				    OstTrace0(TRACE_NORMAL, DUP1_CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: the reductionFactor is negative");
					err = KErrCorrupt;
					}
				}
			else if (!attrib.CompareF(KMaxRetries))
				{
				TLex8 lex(value);
				err = lex.Val(iMaxRetries);
				if (iMaxRetries < 0)
					{
				    OstTrace0(TRACE_NORMAL, DUP2_CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: the maxRetries is negative");
					err = KErrCorrupt;
					}
				}
			if (!attrib.CompareF(KSize))
				{
				TLex8 lex(value);
				err = lex.Val(iSBEGSHMaxSize);
				if (iSBEGSHMaxSize < KMinHeapSize)
					{
				    OstTrace1(TRACE_NORMAL, DUP3_CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: heap size is less then minimum %d", KMinHeapSize);
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
				    OstTrace0(TRACE_NORMAL, DUP4_CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: central_repostiory is NOT a HEX number");
					err = KErrCorrupt;
					}
				} // else if
			else if (!attrib.CompareF(KList))
				{
				err = StringToDrives(value);
				if (err != KErrNone)
					{
				    OstTrace0(TRACE_NORMAL, DUP5_CSBECONFIG_HANDLEATTRIBUTESELEMENT, "Configuration Error: list doesn't have valid characters from a-z");
					}
				} // else if
				
			} // for x
		OstTraceFunctionExit0( CSBECONFIG_HANDLEATTRIBUTESELEMENT_EXIT );
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
		OstTraceFunctionEntry0( CSBECONFIG_ONSTARTDOCUMENTL_ENTRY );
		// Create a converter for converting strings to Unicode
		iConverter = CCnvCharacterSetConverter::NewL();

		// We only convert from UTF-8 to UTF-16
		if (iConverter->PrepareToConvertToOrFromL(KCharacterSetIdentifierUtf8, iRFs) == CCnvCharacterSetConverter::ENotAvailable)
			{
		    OstTrace0(TRACE_ERROR, CSBECONFIG_ONSTARTDOCUMENTL, "Leave: KErrNotFound");
			User::Leave(KErrNotFound);
			}
		OstTraceFunctionExit0( CSBECONFIG_ONSTARTDOCUMENTL_EXIT );
		}
		
	void CSBEConfig::OnEndDocumentL(TInt /*aErrorCode*/)
	/**
	End of document. destroys converter object
	
	@see MContentHandler::OnEndDocumentL()
	*/
		{
		OstTraceFunctionEntry0( CSBECONFIG_ONENDDOCUMENTL_ENTRY );
		// We've finished parsing the document, hence destroy the converter object
		delete iConverter;
		iConverter = NULL;
		OstTraceFunctionExit0( CSBECONFIG_ONENDDOCUMENTL_EXIT );
		}
		
	void CSBEConfig::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt /*aErrCode*/)
	/**
	Element to parse on the start
	
	@see MContentHandler::OnStartElementL()
	
	@param aElement RTagInfo&
	@param aAttributes RAttributeArray&
	*/
		{
		OstTraceFunctionEntry0( CSBECONFIG_ONSTARTELEMENTL_ENTRY );
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
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CSBECONFIG_ONSTARTELEMENTL, "Leave: %d", err));
		OstTraceFunctionExit0( CSBECONFIG_ONSTARTELEMENTL_EXIT );
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
