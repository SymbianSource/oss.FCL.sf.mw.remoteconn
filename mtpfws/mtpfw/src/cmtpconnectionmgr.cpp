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

#include "cmtpconnectionmgr.h"

#include "cmtpconnection.h"
#include "cmtptransportplugin.h"
#include "mmtptransportconnection.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpconnectionmgrTraces.h"
#endif


// Class constants.

const TUint KMTPUsbTransportUid  = 0x102827B2;
const TUint KMTPBTTransportUid  = 0x10286FCB;

/**
CMTPConnectionMgr factory method.
@leave If a failure occurs, one of the system wide error codes.
*/
CMTPConnectionMgr* CMTPConnectionMgr::NewL()
    {
    CMTPConnectionMgr* self = new(ELeave) CMTPConnectionMgr();
    return self;
    }

/**
Destructor.
*/  
CMTPConnectionMgr::~CMTPConnectionMgr()
    {
    StopTransport( iTransportUid, ETrue );
    iConnections.ResetAndDestroy();    
    iSuspendedTransports.Close();
    delete iTransportTrigger;
    }

/**
Provides a reference to the connection with the specified connection identifier.
@param aConnectionId The connection identifier.
@return The connection reference.
@leave KErrNotFound If a connection with the specified identifier does not 
exist.
*/
EXPORT_C CMTPConnection& CMTPConnectionMgr::ConnectionL(TUint aConnectionId) const
    {   
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_CONNECTIONL_ENTRY );
    
    
    TInt idx(ConnectionFind(aConnectionId));
    
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_CONNECTIONL, "idx is %d", idx );    
    __ASSERT_ALWAYS((idx != KErrNotFound), User::Invariant());
    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_CONNECTIONL_EXIT );
    return *iConnections[idx];
    }

/**
Provides a count of the number of currently open connections.
@return The count of currently open connections.
*/  
TUint CMTPConnectionMgr::ConnectionCount() const
    {
    return iConnections.Count();
    }

/**
Provide a non-const reference to the located at the specified position within 
the connection table.
@return A non-const reference to the required connection.
*/
CMTPConnection& CMTPConnectionMgr::operator[](TInt aIndex) const
    {
    return *iConnections[aIndex];
    }
    
/**
Returns the current transportID.
@return The CMTPTransportPlugin interface implementation UID.
*/
EXPORT_C TUid CMTPConnectionMgr::TransportUid()
    {
    return iTransportUid;
    }

void CMTPConnectionMgr::ConnectionCloseComplete(const TUint& /*aConnUid*/)
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_CONNECTIONCLOSECOMPLETE_ENTRY );
    
    if (iTransportUid.iUid != KMTPUsbTransportUid )
        {
        ResumeSuspendedTransport();
        }
    else
        {
        iResumeCalled = ETrue;
        }
    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_CONNECTIONCLOSECOMPLETE_EXIT );
    }

EXPORT_C void CMTPConnectionMgr::StartTransportL(TUid aTransport)
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_ENTRY );
    
    //When USB plug out, BT will start Master mode to reconnect remote device. Else BT will start slave mode to listen connection.
    if(aTransport.iUid == KMTPBTTransportUid && iRemoteDevice.iDeviceAddr != 0 && aTransport != iTransportUid)
        {
        OstTrace0( TRACE_NORMAL, CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT, "StartTransportL with parameter!" );
        TMTPBTRemoteDeviceBuf tmpdata(iRemoteDevice);
        StartTransportL( aTransport, &tmpdata );
        iRemoteDevice.iDeviceAddr = 0;
        iRemoteDevice.iDeviceServicePort = 0;
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT, "StartTransportL without parameter!" );
        StartTransportL( aTransport, NULL );
        }
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_EXIT );
    }

/**
Loads and starts up the MTP transport plug-in with the specified 
CMTPTransportPlugin interface implementation UID. Only one MTP transport 
plug-in can be loaded at any given time.
@param The CMTPTransportPlugin interface implementation UID.
@leave KErrNotSupported If an attempt is made to load a second MTP transport
plug-in.
@leave One of the system wide error codes, if a processing failure occurs.
@see StopTransport
*/
EXPORT_C void CMTPConnectionMgr::StartTransportL(TUid aTransport, const TAny* aParameter)
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER_ENTRY );
    
    if (iTransport)
        {
        OstTrace0( TRACE_NORMAL, CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, "The transport is not none." );
        
        
        if (aTransport != iTransportUid)
            {
            // Multiple transports not currently supported.
            OstTrace0( TRACE_NORMAL, DUP1_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, "Multiple transports are not supported now!" );
            User::Leave(KErrNotSupported);
            }
        else
            {
            OstTrace1( TRACE_NORMAL, DUP2_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, "Relaunch the transport 0x%X",  iTransportUid.iUid); 
            if(aTransport.iUid == KMTPBTTransportUid)
                {
                iTransport->Stop(*this);
                delete iTransport;
                iTransport = NULL;
                iTransportCount--;
                iTransport = CMTPTransportPlugin::NewL(aTransport, aParameter);
                
                TRAPD(err, iTransport->StartL(*this));
                if (err != KErrNone)
                    {
                    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, DUP3_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, 
                            "StartTransportL error, error code = %d",  err); 
                    delete iTransport;
                    iTransport = NULL;
                    
                    User::Leave(err);
                    }
                iTransportCount++;
                iTransportUid = aTransport;       
             
                }

            }
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP4_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, "begin start transport." );        
        iTransport = CMTPTransportPlugin::NewL(aTransport, aParameter);

        TRAPD(err, iTransport->StartL(*this));
        if (err != KErrNone)
            {
            OstTrace1( TRACE_NORMAL, DUP5_CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER, "StartTransportL error, error code = %d",  err); 
            delete iTransport;
            iTransport = NULL;
            User::Leave(err);
            }
        iTransportUid = aTransport;       
        
        iTransportCount++;
        
        if(iTransportUid.iUid != KMTPBTTransportUid)
            {
            UnsuspendTransport( iTransportUid );
            }
        else 
            {
            //Suspend BT transport to handle switching with Mass Storage 
            SuspendTransportL( iTransportUid);
            }
        }
		    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_STARTTRANSPORTL_TUINT_TANYPOINTER_EXIT );
    }

/**
Queue the transport to start when there is no running transport
@param aTransport, The CMTPTransportPlugin interface implementation UID.
@param aParameter, reserved
@leave One of the system wide error codes, if the operation fails.
*/
EXPORT_C void CMTPConnectionMgr::QueueTransportL( TUid aTransport, const TAny* /*aParameter*/ )
    {
    OstTraceFunctionEntry0(CMTPCONNECTIONMGR_QUEUETRANSPORTL_ENTRY);
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_QUEUETRANSPORTL, "QueueTransportL( 0x%08X )", aTransport.iUid);
    
    __ASSERT_DEBUG( ( KErrNotFound == iSuspendedTransports.Find( aTransport ) ), User::Invariant() );
    iSuspendedTransports.InsertL( aTransport, 0 );
    OstTraceFunctionExit0(CMTPCONNECTIONMGR_QUEUETRANSPORTL_EXIT);
    }

EXPORT_C void CMTPConnectionMgr::SetClientSId(TUid aSecureId)
	{
	iSecureId=aSecureId;
	}

/**
Shuts down and unloads the MTP transport plug-in with the specified 
CMTPTransportPlugin interface implementation UID.
@param The CMTPTransportPlugin interface implementation UID.
*/
EXPORT_C void CMTPConnectionMgr::StopTransport(TUid aTransport)
    {
    StopTransport( aTransport, EFalse );
    }

/**
Shuts down and unloads the MTP transport plug-in with the specified 
CMTPTransportPlugin interface implementation UID.
@param aTransport The CMTPTransportPlugin interface implementation UID.
@param aByBearer If ETrue, it means the transport plugin is stopped because the bearer is turned off or not activated.
*/
EXPORT_C void CMTPConnectionMgr::StopTransport( TUid aTransport, TBool aByBearer )
    {
	OstTraceFunctionEntry0( CMTPCONNECTIONMGR_STOPTRANSPORT_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_STOPTRANSPORT, "aTransport is 0x%X", aTransport.iUid);

    TInt transportId = iTransportUid.iUid;

    if ( aByBearer )
        {
        UnsuspendTransport( aTransport );
        }
    
    if ( ( iTransport ) && ( aTransport == iTransportUid ) )
        {
        if ( !aByBearer )
            {
            TRAP_IGNORE( SuspendTransportL( aTransport ) );
            }
        iTransport->Stop(*this);
        delete iTransport;
        iTransport = NULL;
        iTransportUid = KNullUid;
        iTransportCount--;
        //To be sure Resume is done after StopTransport
        if( transportId == KMTPUsbTransportUid && iResumeCalled)
            {
            ResumeSuspendedTransport();
			iResumeCalled = EFalse;            
            }

        }
    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_STOPTRANSPORT_EXIT );
    }

/**
Shuts down and unloads all active MTP transport plug-ins.
*/
EXPORT_C void CMTPConnectionMgr::StopTransports()
    {
    if (iTransport)
        {
        iTransport->Stop(*this);
        delete iTransport;
        iTransport = NULL;
        iTransportUid = KNullUid;
        iTransportCount--;
        }
    }

/**
Returns the number of active Transports.
@return Number of active transports
*/
EXPORT_C TInt CMTPConnectionMgr::TransportCount() const
    {
    return iTransportCount;
    }

/*
Record the remote device bluetooth address when connection setup.
*/
EXPORT_C void CMTPConnectionMgr::SetBTResumeParameter(const TBTDevAddr& aBTAddr, const TUint16& aPSMPort)
    {
    TInt64 addr(0);
    TUint8 i(0);
    addr += aBTAddr[i++];
    for(; i<KBTDevAddrSize; ++i)
        {
        addr <<= 8;
        addr += aBTAddr[i];
        }
    
    iRemoteDevice.iDeviceAddr = addr;
    iRemoteDevice.iDeviceServicePort = aPSMPort;
    }

TBool CMTPConnectionMgr::ConnectionClosed(MMTPTransportConnection& aTransportConnection)
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_CONNECTIONCLOSED_ENTRY );
    
    TInt idx(ConnectionFind(aTransportConnection.BoundProtocolLayer().ConnectionId()));
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_CONNECTIONCLOSED, "idx is %d", idx );
    __ASSERT_DEBUG((idx != KErrNotFound), User::Invariant());
    
    CMTPConnection* connection(iConnections[idx]);
    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_CONNECTIONCLOSED_EXIT);
    return connection->ConnectionSuspended();
    }
    
void CMTPConnectionMgr::ConnectionOpenedL(MMTPTransportConnection& aTransportConnection)
    {   
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_CONNECTIONOPENEDL_ENTRY );
  
    TUint impUid = aTransportConnection.GetImplementationUid();
    TInt idx = ConnectionFind(impUid);
    CMTPConnection* connection = NULL;
    
    if (idx == KErrNotFound)
        {
        // take transport connection implementation UID as connection ID
        connection = CMTPConnection::NewLC(impUid, aTransportConnection);
        iConnections.InsertInOrder(connection, iConnectionOrder);
        CleanupStack::Pop(connection); 
        }
    else
        {
        connection = iConnections[idx];
        }
    connection->ConnectionResumedL(aTransportConnection);
    
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_CONNECTIONOPENEDL_EXIT );
    }

EXPORT_C TUid CMTPConnectionMgr::ClientSId()
	{
	return iSecureId;
	}
/**
Constructor.
*/
CMTPConnectionMgr::CMTPConnectionMgr() :
    iConnectionOrder(ConnectionOrderCompare),
    iShutdownConnectionIdx(KErrNotFound),
	iTransportUid(KNullUid)
    {
    iRemoteDevice.iDeviceAddr = 0;
    iRemoteDevice.iDeviceServicePort = 0;
    iResumeCalled = EFalse;
    }

/**
Provides the connections table index of the connection with the specified 
connection identifier.
@param The identifier of the required connection.
@return The connection table index of the required connection, or KErrNotFound
if the connection identifier could not be found.
*/ 
TInt CMTPConnectionMgr::ConnectionFind(TUint aConnectionId) const
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_CONNECTIONFIND_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_CONNECTIONFIND, "connectionId %d", aConnectionId);
    
    TInt ret(KErrNotFound);
    
    const TUint noConnections = iConnections.Count();
    for (TUint i(0); ((i < noConnections) && (ret == KErrNotFound)); i++)
        {
        TInt id(iConnections[i]->ConnectionId());
        if (aConnectionId == id)
            {
            ret = i;
            break;
            }
        }
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_CONNECTIONFIND_EXIT );
    return ret;
    }

/**
Determines the relative order of two CMTPConnection objects based on their 
connection IDs.
@return Zero, if the two objects are equal; a negative value, if the aFirst 
is less than aSecond, or; a positive value, if the aFirst is greater than 
aSecond.
*/
TInt CMTPConnectionMgr::ConnectionOrderCompare(const CMTPConnection& aFirst, const CMTPConnection& aSecond)
    {
    return aFirst.ConnectionId() - aSecond.ConnectionId();
    }

/**
Append the transport to the suspended transport list
@param aTransport, The implementation UID of the suspended transport plugin
@leave One of the system wide error codes, if the operation fails.
*/
void CMTPConnectionMgr::SuspendTransportL( TUid aTransport )
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_SUSPENDTRANSPORTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_SUSPENDTRANSPORTL, "SuspendTransportL( 0x%08X )", aTransport.iUid );
    
    
    if ( KErrNotFound == iSuspendedTransports.Find( aTransport ) )
        {
        iSuspendedTransports.AppendL( aTransport );
        }
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_SUSPENDTRANSPORTL_EXIT );
    }

/**
Remove transport from the suspended transports list
@param aTransport, The CMTPTransportPlugin interface implementation UID 
*/
void CMTPConnectionMgr::UnsuspendTransport( TUid aTransport )
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_UNSUSPENDTRANSPORT_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPCONNECTIONMGR_UNSUSPENDTRANSPORT, "Transport uid is 0x%08X;", aTransport.iUid );
    
    
    TInt idx = iSuspendedTransports.Find( aTransport );
    if ( KErrNotFound != idx )
        {
        OstTrace1( TRACE_NORMAL, DUP1_CMTPCONNECTIONMGR_UNSUSPENDTRANSPORT, "Remove the number %d suspended transport", idx );
        iSuspendedTransports.Remove( idx );
        }
    OstTraceFunctionExit0( CMTPCONNECTIONMGR_UNSUSPENDTRANSPORT_EXIT );
    }

/**
Prepare to resume suspended transport
*/
void CMTPConnectionMgr::ResumeSuspendedTransport()
    {
    OstTraceFunctionEntry0( CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT_ENTRY );
    const TInt count = iSuspendedTransports.Count();
    OstTrace1(TRACE_NORMAL, CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT, "The count number is %d", count);
    OstTrace1(TRACE_NORMAL, DUP1_CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT, "The transportport id is 0x%X", iTransportUid.iUid);
    
    if ( ( count > 0 )
        // If the transport was just switched and suspended, it shouldn't be resumed.
        && (( iTransportUid != iSuspendedTransports[count-1] ) || iTransportUid.iUid == KMTPBTTransportUid))
        {
        OstTrace0(TRACE_NORMAL, DUP2_CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT, "Found suspended transport(s).");
        if ( !iTransportTrigger )
            {
            iTransportTrigger = new( ELeave ) CAsyncCallBack( CActive::EPriorityStandard );
            }
       
        if ( !iTransportTrigger->IsActive())
            {
            OstTrace0(TRACE_NORMAL, DUP3_CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT, "Set call back function!");
            TCallBack callback( CMTPConnectionMgr::DoResumeSuspendedTransport, this );
            iTransportTrigger->Set( callback );
            iTransportTrigger->CallBack();            
            }
        else
            {
            OstTrace0(TRACE_NORMAL, DUP4_CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT, "Call back has been set!");
            }

        }
    OstTraceFunctionExit0( DUP1_CMTPCONNECTIONMGR_RESUMESUSPENDEDTRANSPORT_EXIT );
    }

/**
Resume suspended transport
@param aSelf, The memory address of the CMTPConnectionMgr instance
@return KErrNone, but the value is ignored.
*/
TInt CMTPConnectionMgr::DoResumeSuspendedTransport( TAny* aSelf )
    {
    CMTPConnectionMgr* self = reinterpret_cast< CMTPConnectionMgr* >( aSelf );
    if(self->iSuspendedTransports.Count() > 0)
    	{
    	TRAP_IGNORE( self->StartTransportL( self->iSuspendedTransports[self->iSuspendedTransports.Count()-1] ) );
    	}
    return KErrNone;
    }

