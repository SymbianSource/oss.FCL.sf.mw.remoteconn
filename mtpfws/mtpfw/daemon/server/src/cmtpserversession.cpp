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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpserversessionTraces.h"
#endif



/**
Constructor.
*/
CMTPServerSession::CMTPServerSession()
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_CMTPSERVERSESSION_CONS_ENTRY );
    OstTraceFunctionExit0( CMTPSERVERSESSION_CMTPSERVERSESSION_CONS_EXIT );
    }
    
/**
Destructor.
*/
CMTPServerSession::~CMTPServerSession()
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_CMTPSERVERSESSION_DES_ENTRY );
    static_cast<CMTPServer*>(const_cast<CServer2*>(CSession2::Server()))->DropSession();
    iSingletons.Close();
    OstTraceFunctionExit0( CMTPSERVERSESSION_CMTPSERVERSESSION_DES_EXIT );
    }
    
void CMTPServerSession::CreateL()
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_CREATEL_ENTRY );
    iSingletons.OpenL();
    static_cast<CMTPServer*>(const_cast<CServer2*>(CSession2::Server()))->AddSession();
    OstTraceFunctionExit0( CMTPSERVERSESSION_CREATEL_EXIT );
    }

void CMTPServerSession::ServiceL(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_SERVICEL_ENTRY );
    switch (aMessage.Function())
        {
    case EMTPClientStartTransport:
        OstTrace0( TRACE_NORMAL, CMTPSERVERSESSION_SERVICEL, "StartTrasport message received" );      
        DoStartTransportL(aMessage);
        break;
    case EMTPClientStopTransport:      
        OstTrace0( TRACE_NORMAL, DUP1_CMTPSERVERSESSION_SERVICEL, "StopTransport message received" );       
        DoStopTransport(aMessage);
        break;
     case EMTPClientIsAvailable :                 
        OstTrace0( TRACE_NORMAL, DUP2_CMTPSERVERSESSION_SERVICEL, "IsAvailable message received" );    
        DoIsAvailableL(aMessage);
     
        break;
    default:
        
        OstTrace0( TRACE_WARNING, DUP3_CMTPSERVERSESSION_SERVICEL, "Unrecognised message received" );
        break;        
        }
    OstTraceFunctionExit0( CMTPSERVERSESSION_SERVICEL_EXIT );
    }
TBool CMTPServerSession::CheckIsAvailableL(TUid aNewUID,TUid aCurUID)
{
        OstTraceFunctionEntry0( CMTPSERVERSESSION_CHECKISAVAILABLEL_ENTRY );

	    TInt SwitchEnabled;
	    iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ETransportSwitchEnabled, SwitchEnabled);	    
	    if(!SwitchEnabled )
		    {		    
		    OstTraceFunctionExit0( CMTPSERVERSESSION_CHECKISAVAILABLEL_EXIT );
		    return EFalse;
		    }
       TBuf<30> value;
       iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ETransportHighPriorityUID,value);	    	    
	   TUint HighUID;	    
	   TLex lex(value);	    
	   TInt conErr = lex.Val(HighUID,EHex);
	   if(aCurUID.iUid == HighUID)
		    {
		    OstTraceFunctionExit0( DUP1_CMTPSERVERSESSION_CHECKISAVAILABLEL_EXIT );
		    return EFalse;	

		    }
	   else if(aNewUID.iUid ==HighUID)
		    {
	    	OstTraceFunctionExit0( DUP2_CMTPSERVERSESSION_CHECKISAVAILABLEL_EXIT );
	    	return ETrue;
		    }
	   OstTraceFunctionExit0( DUP3_CMTPSERVERSESSION_CHECKISAVAILABLEL_EXIT );
	   return EFalse;	
}
/**
Starts up the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
@leave One of the system wide error codes, if a processing error occurs.
*/
void CMTPServerSession::DoStartTransportL(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_DOSTARTTRANSPORTL_ENTRY );

    TUid newUID = TUid::Uid(aMessage.Int0()); 
    TUid curUID = iSingletons.ConnectionMgr().TransportUid();
    if(curUID !=(KNullUid))  // Another Transport is already running
		{
    	if(!CheckIsAvailableL(newUID,curUID))
    		{
    		aMessage.Complete(KErrServerBusy);
    		iSingletons.ConnectionMgr().QueueTransportL( newUID, NULL );
    		OstTraceFunctionExit0( CMTPSERVERSESSION_DOSTARTTRANSPORTL_EXIT );
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
    		LEAVEIFERROR(err, OstTrace0(TRACE_ERROR, CMTPSERVERSESSION_DOSTARTTRANSPORTL, "start transport error"));
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
    
    
    OstTraceFunctionExit0( DUP1_CMTPSERVERSESSION_DOSTARTTRANSPORTL_EXIT );
    }  


/**
Shuts down the specified MTP transport protocol.
@param aTransport The implementation UID of the transport protocol implemetation.
*/  
void CMTPServerSession::DoStopTransport(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_DOSTOPTRANSPORT_ENTRY );
   iSingletons.ConnectionMgr().StopTransport( TUid::Uid( aMessage.Int0() ), ETrue );
    aMessage.Complete(KErrNone);
    OstTraceFunctionExit0( CMTPSERVERSESSION_DOSTOPTRANSPORT_EXIT );
    }

/**
Checks whether MTP Framework shall allow the StartTransport Command for given transport.
@param aTransport The implementation UID of the transport protocol implementation.
@leave One of the system wide error codes, if a processing error occurs.
*/  
void CMTPServerSession::DoIsAvailableL(const RMessage2& aMessage)
    {
    OstTraceFunctionEntry0( CMTPSERVERSESSION_DOISAVAILABLEL_ENTRY );
   
   TUid newUID = TUid::Uid(aMessage.Int0()); 
   TUid curUID = iSingletons.ConnectionMgr().TransportUid();
   
   if(curUID !=(KNullUid))  // Another Transport is already running
		{
		if(curUID== newUID)
			{
    		aMessage.Complete(KErrAlreadyExists); 	
    		OstTraceFunctionExit0( CMTPSERVERSESSION_DOISAVAILABLEL_EXIT );
    		return;				
			}
    	else if(!CheckIsAvailableL(newUID,curUID))
    		{
    		aMessage.Complete(KErrServerBusy); 	
    		OstTraceFunctionExit0( DUP1_CMTPSERVERSESSION_DOISAVAILABLEL_EXIT );
    		return;
    		}	    
    	}

    aMessage.Complete(KErrNone);

    OstTraceFunctionExit0( DUP2_CMTPSERVERSESSION_DOISAVAILABLEL_EXIT );
    }

TBool CMTPServerSession::CheckIsBlueToothTransport(TUid aNewUid)
	{
	TInt32 bluetoothUid = 0x10286FCB;
	return aNewUid.iUid == bluetoothUid;
	}
