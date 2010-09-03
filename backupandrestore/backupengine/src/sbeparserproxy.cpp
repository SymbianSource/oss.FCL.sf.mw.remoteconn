// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of CBackupRegistrationParser
// 
//

/**
 @file
*/
#include "sbeparserproxy.h"

// System includes
#include <charconv.h>

// User includes
#include "sbeparserdefs.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbeparserproxyTraces.h"
#endif

namespace conn
	{
	CSBEParserProxy::CSBEParserProxy( RFs& aFsSession )
        : iFsSession( aFsSession )
	/** Standard C++ constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	*/
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_CSBEPARSERPROXY_CONS_ENTRY );
		OstTraceFunctionExit0( CSBEPARSERPROXY_CSBEPARSERPROXY_CONS_EXIT );
		}

	CSBEParserProxy::~CSBEParserProxy()
	/** Standard C++ destructor
	*/
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_CSBEPARSERPROXY_DES_ENTRY );
		delete iConverter;
		delete iParser;
		OstTraceFunctionExit0( CSBEPARSERPROXY_CSBEPARSERPROXY_DES_EXIT );
		}

	void CSBEParserProxy::ConstructL()
	/** Symbian 2nd stage constructor */
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_CONSTRUCTL_ENTRY );
		iParser = CParser::NewL(KMimeType, *this);

		// We only convert from UTF-8 to UTF-16
		iConverter = CCnvCharacterSetConverter::NewL();
		if  ( iConverter->PrepareToConvertToOrFromL( KCharacterSetIdentifierUtf8, iFsSession ) == CCnvCharacterSetConverter::ENotAvailable )
			{
		    OstTrace0(TRACE_ERROR, CSBEPARSERPROXY_CONSTRUCTL, "Leave: KErrNotFound");
			User::Leave(KErrNotFound);
			}
		OstTraceFunctionExit0( CSBEPARSERPROXY_CONSTRUCTL_EXIT );
		}
		
	CSBEParserProxy* CSBEParserProxy::NewL( RFs& aFsSession )
	/** Symbian OS static constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	@return a CBackupRegistrationParser object
	*/
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_NEWL_ENTRY );
		CSBEParserProxy* self = new(ELeave) CSBEParserProxy( aFsSession );
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);

		OstTraceFunctionExit0( CSBEPARSERPROXY_NEWL_EXIT );
		return self;
		}

	void CSBEParserProxy::ParseL( const TDesC& aFileName, MContentHandler& aObserver )
	/** Parsing API */
        {
        OstTraceFunctionEntry0( CSBEPARSERPROXY_PARSEL_ENTRY );
        // Store transient observer (the entity that we will route callbacks to)
        iTransientParsingError = KErrNone;
        iTransientObserver = &aObserver;

        // Do XML parsing of the specified file. Callbacks will occur to client via the XML
        // callback API.
		Xml::ParseL( *iParser, iFsSession, aFileName );

        // Handle any errors received during callbacks
		LEAVEIFERROR( iTransientParsingError, OstTrace1(TRACE_ERROR, DUP1_CSBEPARSERPROXY_CONVERTTOUNICODEL, "Leave: %d", iTransientParsingError) );
        OstTraceFunctionExit0( CSBEPARSERPROXY_PARSEL_EXIT );
        }

	TInt CSBEParserProxy::ConvertToUnicodeL( TDes16& aUnicode, const TDesC8& aForeign )
        {
        OstTraceFunctionEntry0( CSBEPARSERPROXY_CONVERTTOUNICODEL_ENTRY );
        const TInt error = iConverter->ConvertToUnicode( aUnicode, aForeign, iConverterState );

        if  ( error != KErrNone )
            {
            HBufC* copy = HBufC::NewL( aForeign.Length() * 2 );
            copy->Des().Copy( aForeign );
            OstTraceExt2(TRACE_NORMAL, CSBEPARSERPROXY_CONVERTTOUNICODEL, "error: %d when converting: %S", error, *copy );
            delete copy;
            }

        OstTraceFunctionExit0( CSBEPARSERPROXY_CONVERTTOUNICODEL_EXIT );
        return error;
        }
		
	//	
	//  MContentHandler Implementaion //
	//

	/** MContentHandler::OnStartDocumentL()
	*/
	void CSBEParserProxy::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode)
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_ONSTARTDOCUMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
		    OstTrace1(TRACE_ERROR, CSBEPARSERPROXY_ONSTARTDOCUMENTL, "error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}

        iTransientObserver->OnStartDocumentL( aDocParam, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONSTARTDOCUMENTL_EXIT );
		}
		
	/** MContentHandler::OnEndDocumentL()
	*/
	void CSBEParserProxy::OnEndDocumentL(TInt aErrorCode)
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_ONENDDOCUMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
			// just to satifsy UREL compiler
			(void) aErrorCode;
			OstTrace1(TRACE_NORMAL, CSBEPARSERPROXY_ONENDDOCUMENTL, "error = %d", aErrorCode);
			}

        iTransientObserver->OnEndDocumentL( aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONENDDOCUMENTL_EXIT );
		}
		
	/** MContentHandler::OnStartElementL()

	@leave KErrUnknown an unknown element
	*/		
	void CSBEParserProxy::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode)
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_ONSTARTELEMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
			LEAVEIFERROR(aErrorCode, OstTrace1(TRACE_ERROR, CSBEPARSERPROXY_ONSTARTELEMENTL, "error = %d", aErrorCode));
			}

        iTransientObserver->OnStartElementL( aElement, aAttributes, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONSTARTELEMENTL_EXIT );
		}

	/** MContentHandler::OnEndElementL()
	*/
	void CSBEParserProxy::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_ONENDELEMENTL_ENTRY );
		if (aErrorCode != KErrNone)
			{
		    OstTrace1(TRACE_ERROR, CSBEPARSERPROXY_ONENDELEMENTL, "error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}

        iTransientObserver->OnEndElementL( aElement, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONENDELEMENTL_EXIT );
		}

	/** MContentHandler::OnContentL()
	*/
	void CSBEParserProxy::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONCONTENTL_ENTRY );
        iTransientObserver->OnContentL( aBytes, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONCONTENTL_EXIT );
		}

	/** MContentHandler::OnStartPrefixMappingL()
	*/
	void CSBEParserProxy::OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONSTARTPREFIXMAPPINGL_ENTRY );
        iTransientObserver->OnStartPrefixMappingL( aPrefix, aUri, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONSTARTPREFIXMAPPINGL_EXIT );
		}

	/** MContentHandler::OnEndPrefixMappingL()
	*/
	void CSBEParserProxy::OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONENDPREFIXMAPPINGL_ENTRY );
        iTransientObserver->OnEndPrefixMappingL( aPrefix, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONENDPREFIXMAPPINGL_EXIT );
		}

	/** MContentHandler::OnIgnorableWhiteSpaceL()
	*/
	void CSBEParserProxy::OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONIGNORABLEWHITESPACEL_ENTRY );
        iTransientObserver->OnIgnorableWhiteSpaceL( aBytes, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONIGNORABLEWHITESPACEL_EXIT );
		}

	/** MContentHandler::OnSkippedEntityL()
	*/
	void CSBEParserProxy::OnSkippedEntityL(const RString& aName, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONSKIPPEDENTITYL_ENTRY );
        iTransientObserver->OnSkippedEntityL( aName, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONSKIPPEDENTITYL_EXIT );
		}

	/** MContentHandler::OnProcessingInstructionL()
	*/
	void CSBEParserProxy::OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode)
		{
        OstTraceFunctionEntry0( CSBEPARSERPROXY_ONPROCESSINGINSTRUCTIONL_ENTRY );
        iTransientObserver->OnProcessingInstructionL( aTarget, aData, aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONPROCESSINGINSTRUCTIONL_EXIT );
		}

	/** MContentHandler::OnError()

	@leave aErrorCode
	*/
	void CSBEParserProxy::OnError(TInt aErrorCode)
		{
		OstTraceFunctionEntry0( CSBEPARSERPROXY_ONERROR_ENTRY );
		OstTrace1(TRACE_NORMAL, CSBEPARSERPROXY_ONERROR, "error = %d", aErrorCode);
		iTransientParsingError = aErrorCode;
        iTransientObserver->OnError( aErrorCode );
		OstTraceFunctionExit0( CSBEPARSERPROXY_ONERROR_EXIT );
		}

	/** MContentHandler::OnEndPrefixMappingL()
	*/
	TAny* CSBEParserProxy::GetExtendedInterface(const TInt32 aUid)
		{
        return iTransientObserver->GetExtendedInterface( aUid );
		}

    } // namespace conn
