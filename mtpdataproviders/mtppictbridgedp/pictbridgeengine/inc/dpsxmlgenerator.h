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
* Description:  This class creates the dps xml script. 
*
*/


#ifndef DPSXMLGENERATOR_H
#define DPSXMLGENERATOR_H

#include <e32base.h>
#include <s32file.h>
#include "dpsdefs.h"
#include "dpsconst.h"

class CDpsEngine;

/**
*   This class creates dps scripts.
*/
NONSHARABLE_CLASS(CDpsXmlGenerator) : public CBase
    {
    public:
        /**
        *   Two phase constructor
        *   @param aEngine a pointer to dps engine
        *   @return a CDpsXmlGenerator instance
        */
        static CDpsXmlGenerator* NewL(CDpsEngine* aEngine);    
    	
        /**
        *   Destructor
        */
        ~CDpsXmlGenerator();
    	
        /**
        *	Creates the Dps result script
        *   @param aEvent a dps event, can be empty
        *   @param aScript dps script content. after this function is
        *   returned, aScript has the valid content
        *   @param aResult the operation result
        */
        void CreateResultScriptL(TDpsEvent aEvent,
		                         RWriteStream& aScript,
	           		             const TDpsResult& aResult) const;
	    
        /**
        *   Creates the dps reply script
        */ 
        void CreateReplyScriptL(TDpsOperation aOperation, 
                               RWriteStream& aScript,
                               const TDpsResult& aResult,
                               const TDpsArg& aArg) const;
       		         	
    private:
        /**
        *   Default constructor
        *   @param aEngine a pointer to dps engine
        */
        CDpsXmlGenerator(CDpsEngine* aEngine);
	      
    public:	      
        /**
        *   Fillin the start part of a script
        *   @param aScript the content of a script
        */                 
        void StartDocumentL(RWriteStream& aScript) const;
	    
        /**
        *   Fillin the end part of a script
        *   @param aScript the content of a script
        */
        void EndDocumentL(RWriteStream& aScript) const;
	    
        /**
        *   Fillin the start part of a input in the script
        *   @param aScript the content of a script
        */
        void StartInputL(RWriteStream& aScript) const;          	 
		
        /**
        *   Fillin the end part of a input in the script
        *   @param aScript the content of a script
        */
        void EndInputL(RWriteStream& aScript) const;
		
        /**
        *   Fillin the start part of the result in the script
        *   @param aScript the content of a script
        *   @param aResult the result to be filled
        */
        void StartResultL(RWriteStream& aScript, const TDpsResult& aResult) const;
		
        /**
        *   Fillin the end part of the result in the script
        *   @param aScript the content of a script
        */
        void EndResultL(RWriteStream& aScript) const;
		
        /**
        *   Fillin the start part of the operation in the script
        *   @param aOperation the operation enumeration
        *   @param aScript the content of a script
        *   @param aEnd ETrue if the operation does not have an argument,
        *   EFalse otherwise	       
        */
        void StartOperationL(TDpsOperation aOperation, RWriteStream& aScript, 
                            TBool aEnd = EFalse) const; 
		
        /**
        *   Fillin the end part of the operation in the script
        *   @param aOperation the operation enumeration
        *   @param aScript the content of a script
        */
        void EndOperationL(TDpsOperation aOperation, RWriteStream& aScript) const;
		
        /**
        *   Fillin the event parameter in the script
        *   @param aScript the content of a script.
        *   @param aEvent the event enumeration
        */
        void CreateEventL(RWriteStream& aScript, TDpsEvent aEvent) const;
		
        /**
        *   Fillin the arguments
        *   @param aScript the content of a script
        *   @param aArgument arguments to be used to fill the script
        *   @param aAttrib the attribute
        */
        void CreateArgL(RWriteStream& aScript, const TDpsArg& aArgument, 
                        TDpsAttribute aAttrib = 0) const;
        
        /**
        *   Fillin the start part of an element
        *   @param aElement the element enumeration
        *   @param aScript the content of a script
        */
        void StartElementL(TDpsElement aElement, RWriteStream& aScript) const;
		
        /**
        *   Fillin the end part of an element
        *   @param aElement the element enumeration
        *   @param aScript the content of a script
        */
        void EndElementL(TDpsElement aElement, RWriteStream& aScript) const;
							
    private:
        // not owned by this class
        CDpsEngine* iEngine;
    };

#endif
