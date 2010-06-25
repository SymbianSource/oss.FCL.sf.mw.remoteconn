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
* Description:  This class creates and parses dps operations. 
*
*/


#ifndef DPSTRANSACTION_H
#define DPSTRANSACTION_H

#include <e32base.h>
#include "dpsdefs.h"
#include "dpsoperation.h"

class 	CDpsFile;
class 	CDpsEngine;
class   CDpsXmlGenerator;
class	CDpsXmlParser;
class 	CDpsScriptSender;
class   TDpsVersion;
class   CDpsStateMachine;
class   TMDpsOperation;

/**
*   This class creates and parses dps operation (request and reply)
*/
NONSHARABLE_CLASS(CDpsTransaction) : public CBase
    {
    public:
        /**
        *   Two phase constructor
        *   
        *   @param iOperator the pointer to dps operation object
        *   @return the dps transaction instance
        */
        static CDpsTransaction* NewL(CDpsStateMachine* iOperator);    

        /**
        *   Destructor
        */        							 
        ~CDpsTransaction();
        
    public:
        /**
        *   Creates the dps operation request
        *   @param aParam the dps operation parameters
        */
    	void CreateRequestL(TMDpsOperation* aOperation);
    	
    	/**
    	*   Parses dps operation script
    	*   @param aReply if the script is the reply from the host
    	*   aReply is ETrue, otherwise the script must be the request
    	*   from the device and aReply is EFalse
    	*/
    	void ParseScriptL(TBool aReply);
    	
    	/**
    	*   @return ETrue if the script is the reply from the host,
    	*   EFalse if the script is the request from the host
    	*/
    	inline TBool IsReply();
    	
    	/**
    	*   @return the CDpsXmlParser object
    	*/
    	inline CDpsXmlParser* Parser();
    	    
    	/**
    	*   @return the CDpsXmlGenerator object
    	*/
    	inline CDpsXmlGenerator* Generator();    
    	    
    	/**
    	*   @return the CDpsEngine object
    	*/    
        inline CDpsEngine* Engine();
    	
    	    					 
		/**
    	*   Converts the version in descriptor tpye to TDpsVersion type
    	*   @param aParser the version in descriptor 
    	*   @param aVersoin the version in TDpsVersion and will be returned
    	*   @return KErrNone if OK, other system error if failed
    	*/        
        TInt ConvertVersion(TLex8& aParser, TDpsVersion& aVersion);    
        
        /**
        *   @return CDpsStateMachine pointer to dps state machine object
        */
        inline CDpsStateMachine* Operator();
        
        /**
        *   Parses the string of percentage to integer
        *   @param aPer the string of percentage to be parsed
        *   @return TInt the percentage in integer
        */    			
        TInt ParsePercentage(const TDes8& aPer);
        
        /**
        *   @return the error of result in integer
        */
        inline TInt ResultErr();
                    
            
        /**
        *   @return CDpsFile pointer to dps file object
        */
        inline CDpsFile* FileHandle();
        
        /**
        *
        */
        void HandleHostRequestError(TInt aErr);
                    
    private:
        /**
        *   Default constructor
        *   
        *   @param iOperator the pointer to the dps operator 
        */	
    	CDpsTransaction(CDpsStateMachine* iOperator);    
    	
    	/**
    	*   Second phase constructor. 
    	*/
    	void ConstructL();
    	
    	/**
    	*   Creates the dps event reply.
    	*   @param aArguments dps event arguments
    	*   @param aResult the result of the reply. See Dps spec for
    	*   detail result value
    	*/
    	void CreateEventReplyL(TDpsEvent aEvent, const TDpsResult& aResult);
    	
    	/**
    	*   Creates the dps request reply based on host dps request.
    	*   There is only one dps request from host (others are from device) -
    	*   GetFileID
    	*   @param aArgs the dps argument from the dps xml script
    	*   @param aResult the reply result to be filling to the dps xml script
    	*/
    	void CreateRequestReplyL(const TDpsArgArray& aArgs, 
                                 const TDpsResult& result);
        
        /**
        *   Removes the unprintable chars (LF, CR, TAB and spaces) between 
        *   two XML attributes. It seems
        *   sybmian XML framework does filter out these character even though
        *   they are not belong to the attribute. Some printers (Cannon) send
        *   Dps request (XML script) in human readable format, e.g. including
        *   LF, CR and spaces among XML attributes.
        *   @param aScript the XML script to be filtered out
        */
    	void Filter(TDes8& aScript);
    	
    	/**
    	*   Changes the file path for GetFileID request
    	*
    	*/
    	void SubstitutePath(TDes8& aPath);
    	
    private:
        
    	// the pointer to dps operator object
    	// not owned by this class
        CDpsStateMachine* iOperator;
        // the pointer to xml generator object, owned by this class
        CDpsXmlGenerator *iXmlGen;
        // the pointer to xml parser object, owned by this class
        CDpsXmlParser *iXmlPar; 
        // the pointer to file generator object, owned by this class
        CDpsFile *iFile;
                
        // the current parsed script is reply or request
        TBool iReply;  
    };

#include "dpstransaction.inl"
#endif
