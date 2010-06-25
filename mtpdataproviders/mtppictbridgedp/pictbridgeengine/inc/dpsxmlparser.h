/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class parses the dps xml script. 
*
*/


#ifndef DPSXMLPARSER_H
#define DPSXMLPARSER_H

#include <e32base.h>
#include <xml/wbxmlextensionhandler.h>
#include <contenthandler.h>
#include "dpsdefs.h"
#include "dpsconst.h"

using namespace Xml;

class CDpsEngine;

/**
*   The interface acts as a hook between the xml framework and the detailed 
*   implementation
*/
class MDpsExtensionHandler
    {
    public:
        enum 
        {
        // dps engine dll uid 
        EInterfaceUid = 0x10274798
        };
    };

/**
*   The parser for dps script
*/
NONSHARABLE_CLASS(CDpsXmlParser) : public CBase, public MContentHandler, 
                                   public MDpsExtensionHandler
    {
    public:
        /**
        *   Two phase constructor
        *   @param aEngine the dps engine pointer
        *   @return a CDpsXmlParaser instance
        */
        static CDpsXmlParser* NewL(CDpsEngine* aEngine);
    	
        /**
        *   Destructor
        */
        ~CDpsXmlParser();  
    	
        /**
        *   @return ETrue if the script is a dps notification, 
        *   otherwise EFalse
        */
        inline TBool IsEvent() const;
    	
        /**
        *   Gets the dps operation result from the parser and sets it to the 
        *   dps reply
        *   @param aResult the dps operatoin result
        */
        inline void SetOperationResult(TDpsResult& aResult) const;
    	
        /**
        *   Gets the dps operation enumeration of this script
        *   @return the dps operation enumeration
        */
        inline TDpsOperation Operation() const;
    	    
        /**
        *   Sets the dps operation arguments
        *   @param aParams the arguments to be set
        */
        inline void SetParameters(TDpsArgArray& aParams);
    	
        /**
        *   Gets the dps event of this script
        *   @return the dps event enumeration
        */
        inline TDpsEvent Event() const;
    	    
        /**
        *   Gets the dps attribute
        *   @param aAttrib the attribute got
        */
        inline void GetAttribute(TDpsAttribute& aAttrib) const;
    	
        /**
        *   @return ETrue if this dps operation has attribue, othewise EFalse
        */
        inline TBool HasAttribute() const;
    	
        /**
        *   Gets the dps arguments
        *   @param aParams the arguments to be returned
        */
        inline void GetParameters(TDpsArgArray& aParams) const;
        
        /**
        *   Resets the member variables 
        */
        void Reset();
		
    	    
    private:
    // From MContentHandler
        /**
        *   @see MContentHandler
        */
        void OnStartDocumentL(const RDocumentParameters& aDocParam, 
                              TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnEndDocumentL(TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnStartElementL(const RTagInfo& aElement, 
                             const RAttributeArray& aAttributes, 
                             TInt aErrCode);
		
        /**
        *   @see MContentHandler
        */
        void OnEndElementL(const RTagInfo& aElement, TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnContentL(const TDesC8& aBytes, TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnStartPrefixMappingL(const RString& aPrefix, 
                                   const RString& aUri, 
                                   TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnEndPrefixMappingL(const RString& aPrefix, TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnSkippedEntityL(const RString& aName, TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnProcessingInstructionL(const TDesC8& aTarget, const TDesC8& aData, 
                                      TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        void OnError(TInt aErrorCode);
		
        /**
        *   @see MContentHandler
        */
        TAny* GetExtendedInterface(const TInt32 aUid); 
		        				
    private:
        /**
        *   Default constructor
        *   @param aEngine a pointer to the dps engine
        */
        CDpsXmlParser(CDpsEngine* aEngine);
        
        /**
        *
        */
        void ParseAttributesL(const RAttributeArray& aAttributes, 
                              const TDesC8& aTag);

		
    private:
        TDpsXmlAction iAction;
        TDpsAttribute iAttrib;
		
        TDpsEvent iDpsEvent;
        TDpsOperation iDpsOperation;	
        TDpsResult iDpsResult;
        // only one
        TDpsElement iElement; 
        TDpsArgArray iDpsArgs;
        // not owned by this class
        CDpsEngine* iEngine;
    };
#include "dpsxmlparser.inl"
#endif
