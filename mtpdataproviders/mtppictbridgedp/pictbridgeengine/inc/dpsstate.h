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
* Description:  These classes define the dps states. 
*
*/


#ifndef DPSSTATE_H
#define DPSSTATE_H

#include <e32base.h>
class CDpsStateMachine;

/**
*   This class is the base classes for all dps state classes.
*   We follow state design pattern here so the state class defines all
*   transactions among states as member functions.
*/
class MDpsState
    {
    public:
                
        /**
        *   Script sent notification
        */
        virtual void ScriptSentNotifyL(TBool aReply) = 0;
        
        /**
        *   Script received notification
        */
        virtual void ScriptReceivedNotifyL(TBool aReply) = 0;
        
        /**
        *   Error handling of the transaction
        */
        virtual void Error(TInt aErr) = 0; 
    };

/**
*   Idle state class
*/
NONSHARABLE_CLASS(TDpsIdleState) : public MDpsState
    {
    public:
        /**
        *   Constructor
        */
        TDpsIdleState(CDpsStateMachine* aStateMachine);
    public:
                
        /**
        *   @see MDpsState
        */
        void ScriptSentNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */
        void ScriptReceivedNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */    
        void Error(TInt aErr);

        
    private:
        CDpsStateMachine* iStateMachine;
    };

/**
*   Sending Request state class. Device sending request starts form this state
*/
NONSHARABLE_CLASS(TDpsSendingReqState) : public MDpsState
    {
    public:
        /**
        *   Constructor
        */
        TDpsSendingReqState(CDpsStateMachine* aStateMachine);
            
    public:
        
        /**
        *   @see MDpsState
        */    
        void ScriptSentNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */    
        void ScriptReceivedNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */    
        void Error(TInt aErr);        
            
    private:
        CDpsStateMachine* iStateMachine;
    };

/**
*   Waiting Reply state class (script has been sent)
*/    
NONSHARABLE_CLASS(TDpsWaitingRepState) : public MDpsState
    {
    public:
        /**
        *   Constructor
        */
        TDpsWaitingRepState(CDpsStateMachine* aStateMachine);
            
    public:
               
        /**
        *   @see MDpsState
        */    
        void ScriptSentNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */
        void ScriptReceivedNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */    
        void Error(TInt aErr);        
            
    private:
        CDpsStateMachine* iStateMachine;
    };
    
/**
*   Sending Reply state class. The device starts replying the host request 
*   in this state.
*/     
NONSHARABLE_CLASS(TDpsSendingRepState) : public MDpsState
    {
    public:
        /**
        *   Constructor
        */
        TDpsSendingRepState(CDpsStateMachine* aStateMachine);
            
    public:
          
        /**
        *   @see MDpsState
        */    
        void ScriptSentNotifyL(TBool aReply);
         
        /**
        *   @see MDpsState
        */    
        void ScriptReceivedNotifyL(TBool aReply);
        
        /**
        *   @see MDpsState
        */    
        void Error(TInt aErr);
                    
    private:
        CDpsStateMachine* iStateMachine;
    };
    
#endif
