/**
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Declaration of CSBEParserProxy
* 
*
*/



/**
 @file
*/
#ifndef SBEPARSERPROXY_H
#define SBEPARSERPROXY_H

// System
#include <f32file.h>
#include <xml/contenthandler.h> // MContentHandler mix in class
#include <xml/parser.h>

// Classes referenced
class CCnvCharacterSetConverter;

using namespace Xml;

namespace conn
	{
	/** Class used as a proxy to redirect CParser calls to the relevant data owner

	@internalTechnology
	*/
	class CSBEParserProxy : public CBase, public MContentHandler
		{
	public:
	    static CSBEParserProxy* NewL( RFs& aFsSession );
	    ~CSBEParserProxy();
	    // API
	    void ParseL( const TDesC& aFileName, MContentHandler& aObserver );
        TInt ConvertToUnicodeL( TDes16& aUnicode, const TDesC8& aForeign );
	    
	private: // Constructors
	    CSBEParserProxy( RFs& aFsSession );
	    void ConstructL();

 		// From MContentHandler
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

    private: // Internal
        RFs& iFsSession; /*<! File server session */
	    CParser* iParser; /*<! Parser for xml */
        MContentHandler* iTransientObserver; /*<! XML observer that we will call back */
        TInt iTransientParsingError; /*<! Transient parsing error code, supplied to XML parsing callbacks */
        CCnvCharacterSetConverter* iConverter; /*<! Convert from UTF-8 to unicode */
		TInt iConverterState; /* State for converter object */
		};
	}

#endif // SBEPARSERPROXY_H
