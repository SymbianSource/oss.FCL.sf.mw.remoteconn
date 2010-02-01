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
#include "sblog.h"

namespace conn
	{
	CSBEParserProxy::CSBEParserProxy( RFs& aFsSession )
        : iFsSession( aFsSession )
	/** Standard C++ constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	*/
		{
		}

	CSBEParserProxy::~CSBEParserProxy()
	/** Standard C++ destructor
	*/
		{
		delete iConverter;
		delete iParser;
		}

	void CSBEParserProxy::ConstructL()
	/** Symbian 2nd stage constructor */
		{
		iParser = CParser::NewL(KMimeType, *this);

		// We only convert from UTF-8 to UTF-16
		iConverter = CCnvCharacterSetConverter::NewL();
		if  ( iConverter->PrepareToConvertToOrFromL( KCharacterSetIdentifierUtf8, iFsSession ) == CCnvCharacterSetConverter::ENotAvailable )
			{
			User::Leave(KErrNotFound);
			}
		}
		
	CSBEParserProxy* CSBEParserProxy::NewL( RFs& aFsSession )
	/** Symbian OS static constructor

	@param aSID secure id of data owner
	@param apDataOwnerManager data owner manager to access resources
	@return a CBackupRegistrationParser object
	*/
		{
		CSBEParserProxy* self = new(ELeave) CSBEParserProxy( aFsSession );
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);

		return self;
		}

	void CSBEParserProxy::ParseL( const TDesC& aFileName, MContentHandler& aObserver )
	/** Parsing API */
        {
        // Store transient observer (the entity that we will route callbacks to)
        iTransientParsingError = KErrNone;
        iTransientObserver = &aObserver;

        // Do XML parsing of the specified file. Callbacks will occur to client via the XML
        // callback API.
		Xml::ParseL( *iParser, iFsSession, aFileName );

        // Handle any errors received during callbacks
		User::LeaveIfError( iTransientParsingError );
        }

	TInt CSBEParserProxy::ConvertToUnicodeL( TDes16& aUnicode, const TDesC8& aForeign )
        {
        const TInt error = iConverter->ConvertToUnicode( aUnicode, aForeign, iConverterState );

#ifdef SBE_LOGGING_ENABLED
        if  ( error != KErrNone )
            {
            HBufC* copy = HBufC::NewL( aForeign.Length() * 2 );
            copy->Des().Copy( aForeign );
			__LOG2("CSBEParserProxy::ConvertToUnicode() - error: %d when converting: %S", error, copy );
            delete copy;
            }
#endif

        return error;
        }
		
	//	
	//  MContentHandler Implementaion //
	//

	/** MContentHandler::OnStartDocumentL()
	*/
	void CSBEParserProxy::OnStartDocumentL(const RDocumentParameters& aDocParam, TInt aErrorCode)
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CBackupRegistrationParser::OnStartDocumentL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}

        iTransientObserver->OnStartDocumentL( aDocParam, aErrorCode );
		}
		
	/** MContentHandler::OnEndDocumentL()
	*/
	void CSBEParserProxy::OnEndDocumentL(TInt aErrorCode)
		{
		if (aErrorCode != KErrNone)
			{
			// just to satifsy UREL compiler
			(void) aErrorCode;
			__LOG1("CBackupRegistrationParser::OnEndDocumentL() - error = %d", aErrorCode);
			}

        iTransientObserver->OnEndDocumentL( aErrorCode );
		}
		
	/** MContentHandler::OnStartElementL()

	@leave KErrUnknown an unknown element
	*/		
	void CSBEParserProxy::OnStartElementL(const RTagInfo& aElement, const RAttributeArray& aAttributes, TInt aErrorCode)
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CBackupRegistrationParser::OnStartElementL() - error = %d", aErrorCode);
			User::LeaveIfError(aErrorCode);
			}

        iTransientObserver->OnStartElementL( aElement, aAttributes, aErrorCode );
		}

	/** MContentHandler::OnEndElementL()
	*/
	void CSBEParserProxy::OnEndElementL(const RTagInfo& aElement, TInt aErrorCode)
		{
		if (aErrorCode != KErrNone)
			{
			__LOG1("CBackupRegistrationParser::OnEndElementL() - error = %d", aErrorCode);
			User::Leave(aErrorCode);
			}

        iTransientObserver->OnEndElementL( aElement, aErrorCode );
		}

	/** MContentHandler::OnContentL()
	*/
	void CSBEParserProxy::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
		{
        iTransientObserver->OnContentL( aBytes, aErrorCode );
		}

	/** MContentHandler::OnStartPrefixMappingL()
	*/
	void CSBEParserProxy::OnStartPrefixMappingL(const RString& aPrefix, const RString& aUri, TInt aErrorCode)
		{
        iTransientObserver->OnStartPrefixMappingL( aPrefix, aUri, aErrorCode );
		}

	/** MContentHandler::OnEndPrefixMappingL()
	*/
	void CSBEParserProxy::OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode)
		{
        iTransientObserver->OnEndPrefixMappingL( aPrefix, aErrorCode );
		}

	/** MContentHandler::OnIgnorableWhiteSpaceL()
	*/
	void CSBEParserProxy::OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode)
		{
        iTransientObserver->OnIgnorableWhiteSpaceL( aBytes, aErrorCode );
		}

	/** MContentHandler::OnSkippedEntityL()
	*/
	void CSBEParserProxy::OnSkippedEntityL(const RString& aName, TInt aErrorCode)
		{
        iTransientObserver->OnSkippedEntityL( aName, aErrorCode );
		}

	/** MContentHandler::OnProcessingInstructionL()
	*/
	void CSBEParserProxy::OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, TInt aErrorCode)
		{
        iTransientObserver->OnProcessingInstructionL( aTarget, aData, aErrorCode );
		}

	/** MContentHandler::OnError()

	@leave aErrorCode
	*/
	void CSBEParserProxy::OnError(TInt aErrorCode)
		{
		__LOG1("CBackupRegistrationParser::OnError() - error = %d", aErrorCode);
		iTransientParsingError = aErrorCode;
        iTransientObserver->OnError( aErrorCode );
		}

	/** MContentHandler::OnEndPrefixMappingL()
	*/
	TAny* CSBEParserProxy::GetExtendedInterface(const TInt32 aUid)
		{
        return iTransientObserver->GetExtendedInterface( aUid );
		}

    } // namespace conn
