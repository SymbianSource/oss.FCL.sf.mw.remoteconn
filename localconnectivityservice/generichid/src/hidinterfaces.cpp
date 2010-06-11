/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Hid interface implementation
*
*/


#include <hidinterfaces.h>

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
//    
EXPORT_C CHidDriver::CHidDriver()
     {
     }

// -----------------------------------------------------------------------------
// Desturctor
// -----------------------------------------------------------------------------
//     
EXPORT_C CHidDriver::~CHidDriver()
    {    
    REComSession::DestroyedImplementation( iDtor_ID_Key );
    }

// -----------------------------------------------------------------------------
// NewL()
// -----------------------------------------------------------------------------
//  
EXPORT_C CHidDriver* CHidDriver::NewL( 
    TUid aImplementationUid,
    MDriverAccess* aHid )
    {
    TAny* ptr;
    TInt32 keyOffset = _FOFF( CHidDriver, iDtor_ID_Key );   
    ptr = REComSession::CreateImplementationL(
        aImplementationUid,
        keyOffset,
        aHid
        );    
    return reinterpret_cast<CHidDriver*> (ptr);
    }    
    

// -----------------------------------------------------------------------------
// NewL()
// -----------------------------------------------------------------------------
//      
CHidInputDataHandlingReg* CHidInputDataHandlingReg::NewL()
    {
    CHidInputDataHandlingReg* self = new (ELeave) CHidInputDataHandlingReg();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );    
    return self;    
    
    }    
    
// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
//      
CHidInputDataHandlingReg::CHidInputDataHandlingReg()
    {        
    }        

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
//      
void CHidInputDataHandlingReg::ConstructL()
    {        
    iEventArray =  new ( ELeave ) CArrayFixFlat<THidEvent>( 2 );    
    }        
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//      
CHidInputDataHandlingReg::~CHidInputDataHandlingReg()
    { 
    Reset();           
    delete iEventArray;
    }       

// -----------------------------------------------------------------------------
// AddHandledEvent
// -----------------------------------------------------------------------------
//    
EXPORT_C void  CHidInputDataHandlingReg::AddHandledEvent( TInt aUsagePage, TInt aUsage )
    {
    THidEvent event;
    event.iUsagePage = aUsagePage;    
    event.iKeyCode = aUsage;
    TRAP_IGNORE(iEventArray->AppendL( event ));
    }

// -----------------------------------------------------------------------------
// AllowedToHandleEvent
// -----------------------------------------------------------------------------
//        
EXPORT_C TBool  CHidInputDataHandlingReg::AllowedToHandleEvent(TInt aUsagePage, TInt aUsage)
    {
    TInt i;    
    for (i = 0; i< iEventArray->Count() ;i++)
        {
        if ( iEventArray->At(i).iUsagePage == aUsagePage && iEventArray->At(i).iKeyCode == aUsage )
            {
            return EFalse;    
            }
        }   
    return ETrue;     
    }    
   
// -----------------------------------------------------------------------------
// Reset
// -----------------------------------------------------------------------------
//      
void  CHidInputDataHandlingReg::Reset()
    {
    iEventArray->Reset();    
    }

    
