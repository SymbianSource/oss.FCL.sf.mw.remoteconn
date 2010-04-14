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
//

/**
 @file
 @internalTechnology
*/

#include "cmtpconnectionmgr.h"
#include "cmtpserversession.h"
#include "rmtpframework.h"
#include "cmtpframeworkconfig.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpparserrouter.h"
#include "e32def.h"
__FLOG_STMT(_LIT8(KComponent,"ServerSession");)


/**
Constructor.
*/
CMTPServerSession::CMTPServerSession()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPServerSession - Entry"));
    __FLOG(_L8("CMTPServerSession - Exit"));
    }
    
/**
Destructor.
*/
CMTPServerSession::~CMTPServerSession()
    {
    __FLOG(_L8("~CMTPServerSession - Entry"));
    static_cast<CMTPServer*>(const_cast<CServer2*>(CSession2::Server()))->DropSession();
    iSingletons.Close();
    __FLOG(_L8("~CMTPServerSession - Exit"));
    __FLOG_CLOSE;
    }
    
void CMTPServerSession::CreateL()
    {
    __FLOG(_L8("CreateL - Entry"));
    iSingletons.OpenL();
    static_cast<CMTPServer*>(const_cast<CServer2*>(CSession2::Server()))->AddSession();
    __FLOG(_L8("CreateL - Exit"));
    }

void CMTPServerSession::ServiceL(const RMessage2& aMessage)
    {
    __FLOG(_L8("ServiceL - Entry"));
    switch (aMessage.Function())
        {
    case EMTPClientStartTransport:
        __FLOG(_L8("StartTransport message received"));        
        DoStartTransportL(aMessage);
        break;
    case EMTPClientStopTransport:
        __FLOG(_L8("StopTransport message received"));
        DoStopTransport(aMessage);
        break;
     case EMTPClientIsAvailable :
        __FLOG(_L8("IsAvailable message received"));
        DoIsAvailableL(aMessage);
     
        break;
    default:
        __FLOG(_L8("Unrecognised message received"));
        break;        
        }
    __FLOG(_L8("ServiceL - Exit"));
    }
TBool CMTPServerSession::CheckIsAvailableL(TUid aNewUID,TUid aCurUID)
{

	    TInt SwitchEnabled;
	    iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ETransportSwitchEnabled, SwitchEnabled);	    
	    if(!SwitchEnabled )
		    {		    
		    return EFalse;
		    }
       TBuf<30> value;
       iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ETransportHighPriorityUID,value);	    	    
	   TUint HighUID;	    
	   TLex lex(value);	    
	   TInt conErr = lex.Val(HighUID,EHex);
	   if(aCurUID.iUid == HighUID)
		    {
		    return EFalse;	

		    }
	   else if(aNewUID.iUid ==HighUID)
		    {
	    	return ETrue;
		    }
	   return EFalse;	
}
/**
Starts up the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
@leave One of the system wide error codes, if a processing error occurs.
*/
void CMTPServerSession::DoStartTransportL(const RMessage2& aMessage)
    {
    __FLOG(_L8("DoStartTransportL - Entry"));
    TUid newUID = TUid::Uid(aMessage.Int0()); 
    TUid curUID = iSingletons.ConnectionMgr().TransportUid();
    if(curUID !=(KNullUid))  // Another Transport is already running
		{
    	if(!CheckIsAvailableL(newUID,curUID))
    		{
    		aMessage.Complete(KErrServerBusy);
    		iSingletons.ConnectionMgr().QueueTransportL( newUID, NULL );
    		return;
    		}
	    iSingletons.ConnectionMgr().StopTransport(curUID);
    	}
    
    TUid secureid=aMessage.SecureId();    
    iSingletons.ConnectionMgr().SetClientSId(secureid);
    
    TInt length = aMessage.GetDesLength( 1 );

   
    if (length > 0)
    	{
    	HBufC8* paramHbuf = HBufC8::NewLC(length);
    	TPtr8 bufptr = paramHbuf->Des();
    	aMessage.ReadL( 1, bufptr, 0);
    	TRAPD(err, iSingletons.ConnectionMgr().StartTransportL( newUID, static_cast<TAny*>(paramHbuf) ));
    	if ( KErrArgument == err )
    		{
    		PanicClient( aMessage, EPanicErrArgument );
    		}
    	else if ( KErrNone == err )
    		{
    		aMessage.Complete(KErrNone);
    		}
    	else 
    		{
    		User::LeaveIfError( err );
    		}
    	CleanupStack::PopAndDestroy(paramHbuf);
    	}
    else
    	{
    	iSingletons.ConnectionMgr().StartTransportL(newUID, NULL);
    	aMessage.Complete(KErrNone);
    	}
    
    // Fix TSW error MHAN-7ZU96Z
    if((!CheckIsBlueToothTransport(newUID) || (length!=0)) && (iSingletons.DpController().Count()==0))
    	{
    	iSingletons.DpController().LoadDataProvidersL();
    	iSingletons.Router().ConfigureL();
    	}
    
    
    __FLOG(_L8("DoStartTransportL - Exit"));
    }  


/**
Shuts down the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
*/  
void CMTPServerSession::DoStopTransport(const RMessage2& aMessage)
    {
    __FLOG(_L8("DoStopTransport - Entry"));
   iSingletons.ConnectionMgr().StopTransport( TUid::Uid( aMessage.Int0() ), ETrue );
    aMessage.Complete(KErrNone);
    __FLOG(_L8("DoStopTransport - Exit"));
    }

/**
Checks whether MTP Framework shall allow the StartTransport Command for given transport.
@param aTransport The implementation UID of the transport protocol implementation.
@leave One of the system wide error codes, if a processing error occurs.
*/  
void CMTPServerSession::DoIsAvailableL(const RMessage2& aMessage)
    {
    __FLOG(_L8("DoStopTransport - Entry"));
   
   TUid newUID = TUid::Uid(aMessage.Int0()); 
   TUid curUID = iSingletons.ConnectionMgr().TransportUid();
   
   if(curUID !=(KNullUid))  // Another Transport is already running
		{
		if(curUID== newUID)
			{
    		aMessage.Complete(KErrAlreadyExists); 	
    		return;				
			}
    	else if(!CheckIsAvailableL(newUID,curUID))
    		{
    		aMessage.Complete(KErrServerBusy); 	
    		return;
    		}	    
    	}

    aMessage.Complete(KErrNone);
    __FLOG(_L8("DoStopTransport - Exit"));
    }

TBool CMTPServerSession::CheckIsBlueToothTransport(TUid aNewUid)
	{
	TInt32 bluetoothUid = 0x10286FCB;
	return aNewUid.iUid == bluetoothUid;
	}
