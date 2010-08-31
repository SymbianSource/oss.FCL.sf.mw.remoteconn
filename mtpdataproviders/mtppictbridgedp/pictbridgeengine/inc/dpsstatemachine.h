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
* Description:  This class defines the dps state machine. 
*
*/


#ifndef DPSSTATEMACHINE_H
#define DPSSTATEMACHINE_H

#include "dpsconst.h"
#include "dpsdefs.h"

class CDpsTransaction;
class CDpsEngine;
class CDpsScriptReceiver;
class MDpsState;
class TMDpsOperation;
class CDpsScriptSender;

/**
*   This class defines the dps state machine using the state design pattern
*/
NONSHARABLE_CLASS(CDpsStateMachine) : public CBase
    {
	    
    public:
        /**
        *   Two phase constructor
        *   @param aEngine a pointer to dps engine object
        *   @return the CDpsOperator instance
        */
        static CDpsStateMachine* NewL(CDpsEngine *aEngine);
        
        /**
        *   Destructor
        */
        ~CDpsStateMachine();   
        
        /**
        *   Creates the dps transaction. It further calls CreateRequest()
        *   to create dps device request script.
        *   @param aParam the dps operation object, passed from UI
        */
        void StartTransactionL(TMDpsOperation* aOperation);
        
        /**
        *   Initializes the state machine
        */
        void Initialize();
        
        /**
        *   Handles errors
        */    
        inline void Error(TInt err);
                
        /**
        *   Notifies script sent
        */
        inline void ScriptSentNotifyL(TBool aReply);
            
        /**
        *   Notifies script received
        */    
        inline void ScriptReceivedNotifyL(TBool aReply);
         
        /**
        *   Sets the current state
        *   @param aState the state to be set
        */    
        inline void SetState(MDpsState* aState);
          
        /**
        *   Gets the idle state
        */    
        inline MDpsState* IdleState() const; 
          
        /**
        *   @return the sending request state object
        */    
        inline MDpsState* SendingReqState() const;
          
        /**
        *   @return the waiting for reply state object
        */    
        inline MDpsState* WaitingRepState() const;
             
        /**
        *   @return the sending reply state object
        */    
        inline MDpsState* SendingRepState() const;
          
        /**
        *   @return the CDpsScriptReceiver object
        */    
        inline CDpsScriptReceiver* ScriptReceiver() const;
        
        /**
        *   @return CDpsScriptSender pointer 
        */
        inline CDpsScriptSender* ScriptSender() const;
          
        /**
        *   @return the CDpsTransaction object
        */    
        inline CDpsTransaction* Trader() const;
          
        /**
        *   @return the current Dps operation enum 
        */    
        inline TDpsOperation Operation() const;
          
        /**
        *   Sets the current Dps operation enum
        *   @param aOp the Dps operation enum to be set
        */    
        inline void SetOperation(TDpsOperation aOp);
          
        /**
        *   @return the Dps operation object
        */    
        inline TMDpsOperation* MOperation() const;
          
        /**
        *   @return the Dps event enum
        */    
        inline TDpsEvent Event() const;
          
        /**
        *   Sets the current Dps event
        *   @param aEvent the Dps event to be set
        */    
        inline void SetEvent(TDpsEvent aEvent);
        
        /**
        *   @return the current state.
        */
        inline MDpsState* CurState() const;
        
        /**
        *   @return the dps engine object.
        */
        inline CDpsEngine* DpsEngine() const;
        
        /**
        *
        */
        inline TInt CurError() const;
            
    private:
        /**
        *   Default constructor
        *   @param aEngine a pointer to dps engine object
        */
        CDpsStateMachine(CDpsEngine* aEngine);
        
        /**
        *   Two phase constructor. The functions which called in constructor
        *   and might leave should be called here
        */
        void ConstructL();     
        
                                    
    private:
        // owned by this class
        MDpsState* iIdleState;
        // owned by this class
        MDpsState* iSendingReqState;
        // owned by this class
        MDpsState* iWaitingRepState;
        // owned by this class
        MDpsState* iSendingRepState;
        // pointer to the current state object
        MDpsState* iCurState;
        // not owned by this class
        TMDpsOperation* iMOperation;
     
        // not owned by this class  
        CDpsEngine  *iEngine;
        // the current dps operation, can be empty
        TDpsOperation iOperation;
        // the current dps event, can be empty
        TDpsEvent iEvent;
        // the pointer to dps operation object, which takes
        // care of creating and parsing dps script
         // owned by this class
        CDpsTransaction *iTrader;
        // the pointer to dps script receiving notifier
        // owned by this class
        CDpsScriptReceiver* iScriptReceiver;
        
        // the pointer to script sender object, owned by this class
        CDpsScriptSender* iScriptSender;
        TInt iCurError;
    };

#include "dpsstatemachine.inl"
#endif
