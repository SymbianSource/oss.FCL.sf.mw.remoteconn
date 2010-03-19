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

#include <usbman.h>
#include <usbstates.h>
#include "cmtpconnectionmgr.h"

#include "cmtpconnection.h"
#include "cmtptransportplugin.h"
#include "mmtptransportconnection.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ConnectionMgr");)

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
    iSuspendedTransports.Reset();
    iSuspendedTransports.Close();
    delete iTransportTrigger;
    __FLOG_CLOSE;
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
    __FLOG(_L8("ConnectionL - Entry"));
    
    TInt idx(ConnectionFind(aConnectionId));
    
    __FLOG_VA((_L8("idx is %d "), idx));
    __ASSERT_ALWAYS((idx != KErrNotFound), User::Invariant());
    
    __FLOG(_L8("ConnectionL - Exit"));
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
    __FLOG(_L8("ConnectionCloseComplete - Entry"));
    ResumeSuspendedTransport();
    __FLOG(_L8("ConnectionCloseComplete - exit"));
    }

EXPORT_C void CMTPConnectionMgr::StartTransportL(TUid aTransport)
    {
    
    TInt32 bluetoothUid = 0x10286FCB;
    
    RUsb usb;
    User::LeaveIfError(usb.Connect());
    TInt usbMode;
    TUsbServiceState usbStat;
    TInt err = usb.GetCurrentPersonalityId(usbMode);
    __FLOG_1(_L8("The return value of GetCurrentPersonalityId is %d"), err);
    
    err = usb.GetServiceState(usbStat);
    __FLOG_1(_L8("The return value of GetServiceState is %d"), err);        
    
    usb.Close();
    
    __FLOG_1(_L8("The current usb mode is %d"), usbMode);
    __FLOG_1(_L8("The current usb service state is %d"), usbStat);

    TInt massStorageMode = 0x02;
    
    if(usbMode == massStorageMode && usbStat != EUsbServiceIdle)
        {
        __FLOG(_L8("StartTransportL without parameter!"));
        StartTransportL( aTransport, NULL );
        return;
        }
    
    
    
    //When USB plug out, BT will start Master mode to reconnect remote device. Else BT will start slave mode to listen connection.
    if(aTransport.iUid == bluetoothUid && iRemoteDevice.iDeviceAddr != 0 && aTransport != iTransportUid)
        {
        __FLOG(_L8("StartTransportL with parameter!"));
        TMTPBTRemoteDeviceBuf tmpdata(iRemoteDevice);
        StartTransportL( aTransport, &tmpdata );
        iRemoteDevice.iDeviceAddr = 0;
        iRemoteDevice.iDeviceServicePort = 0;
        }
    else
        {
        __FLOG(_L8("StartTransportL without parameter!"));
        StartTransportL( aTransport, NULL );
        }
    
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
    __FLOG(_L8("StartTransportL - Entry"));
    
    TInt32 bluetoothUid = 0x10286FCB;
    
    if (iTransport)
        {
        __FLOG(_L8("The transport is not none."));
        if (aTransport != iTransportUid)
            {
            // Multiple transports not currently supported.
            __FLOG(_L8("Multiple transports are not supported now!"));
            User::Leave(KErrNotSupported);
            }
        else
            {
            __FLOG_1(_L8("Relaunch the transport 0x%X"), iTransportUid.iUid);
            if(aTransport.iUid == bluetoothUid)
                {
                iTransport->Stop(*this);
                delete iTransport;
                
                iTransport = CMTPTransportPlugin::NewL(aTransport, aParameter);
                
                TRAPD(err, iTransport->StartL(*this));
                if (err != KErrNone)
                    {
                    __FLOG_VA( ( _L8("StartTransportL error, error code = %d"), err) );
                    delete iTransport;
                    iTransport = NULL;
                    User::Leave(err);
                    }
                iTransportUid = aTransport;       
             
                iTransportCount++;
                }

            }
        }
    else
        {
        __FLOG(_L8("begin start transport."));
        iTransport = CMTPTransportPlugin::NewL(aTransport, aParameter);

        TRAPD(err, iTransport->StartL(*this));
        if (err != KErrNone)
            {
            __FLOG_VA( ( _L8("StartTransportL error, error code = %d"), err) );
            delete iTransport;
            iTransport = NULL;
            User::Leave(err);
            }
        iTransportUid = aTransport;       
        
        iTransportCount++;
        
        if(iTransportUid.iUid != bluetoothUid)
            {
            UnsuspendTransport( iTransportUid );
            }
        else 
            {
            //Suspend BT transport to handle switching with Mass Storage 
            SuspendTransportL( iTransportUid);
            }
        }
		
	__FLOG(_L8("StartTransportL - Exit"));
    }

/**
Queue the transport to start when there is no running transport
@param aTransport, The CMTPTransportPlugin interface implementation UID.
@param aParameter, reserved
@leave One of the system wide error codes, if the operation fails.
*/
EXPORT_C void CMTPConnectionMgr::QueueTransportL( TUid aTransport, const TAny* /*aParameter*/ )
    {
    __FLOG_VA( ( _L8("+QueueTransportL( 0x%08X )"), aTransport.iUid ) );
    __ASSERT_DEBUG( ( KErrNotFound == iSuspendedTransports.Find( aTransport ) ), User::Invariant() );
    iSuspendedTransports.InsertL( aTransport, 0 );
    __FLOG( _L8("-QueueTransportL") );
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
	__FLOG(_L8("StopTransport - Entry"));
	

    __FLOG_1(_L8("aTransport is 0x%X"), aTransport.iUid);
    __FLOG_1(_L8("iTransportUid is 0x%X"), aTransport.iUid);

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
        }
    
	__FLOG(_L8("StopTransport - Exit"));
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
    __FLOG(_L8("ConnectionClosed - Entry"));
    
    TInt idx(ConnectionFind(aTransportConnection.BoundProtocolLayer().ConnectionId()));
    __FLOG_VA((_L8("idx is %d "), idx));
    __ASSERT_DEBUG((idx != KErrNotFound), User::Invariant());
    
    CMTPConnection* connection(iConnections[idx]);
    
    __FLOG(_L8("ConnectionClosed - Exit"));
    return connection->ConnectionSuspended();
    }
    
void CMTPConnectionMgr::ConnectionOpenedL(MMTPTransportConnection& aTransportConnection)
    {   
    __FLOG(_L8("ConnectionOpenedL - Entry"));
    
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
    
    __FLOG(_L8("ConnectionOpenedL - Exit"));
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    iRemoteDevice.iDeviceAddr = 0;
    iRemoteDevice.iDeviceServicePort = 0;
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
    __FLOG_VA((_L8("ConnectionFind - Entry with connectionId %d "), aConnectionId));
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
    __FLOG(_L8("ConnectionFind - Exit"));    
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
    __FLOG_1( _L8("+SuspendTransportL( 0x%08X )"), aTransport );
    if ( KErrNotFound == iSuspendedTransports.Find( aTransport ) )
        {
        iSuspendedTransports.AppendL( aTransport );
        }
    __FLOG( _L8("-SuspendTransportL") );
    }

/**
Remove transport from the suspended transports list
@param aTransport, The CMTPTransportPlugin interface implementation UID 
*/
void CMTPConnectionMgr::UnsuspendTransport( TUid aTransport )
    {
    __FLOG_1( _L8("+UnsuspendTransport( 0x%08X )"), aTransport.iUid );
    TInt idx = iSuspendedTransports.Find( aTransport );
    if ( KErrNotFound != idx )
        {
        __FLOG_1( _L8("Remove the number %d suspended transport"), idx );
        iSuspendedTransports.Remove( idx );
        }
    __FLOG( _L8("-UnsuspendTransport") );
    }

/**
Prepare to resume suspended transport
*/
void CMTPConnectionMgr::ResumeSuspendedTransport()
    {
    __FLOG( _L8("+ResumeSuspendedTransport") );
    const TInt count = iSuspendedTransports.Count();
    __FLOG_1(_L8("The count number is %d"), count);
    __FLOG_1(_L8("The transportport id is 0x%X"), iTransportUid.iUid);
    
    TInt32 bluetoothUid = 0x10286FCB;
    
    if ( ( count > 0 )
        // If the transport was just switched and suspended, it shouldn't be resumed.
        && (( iTransportUid != iSuspendedTransports[count-1] ) || iTransportUid.iUid == bluetoothUid))
        {
        __FLOG( _L8("Found suspended transport(s).") );
        if ( !iTransportTrigger )
            {
            iTransportTrigger = new( ELeave ) CAsyncCallBack( CActive::EPriorityStandard );
            }
        __ASSERT_DEBUG( ( !iTransportTrigger->IsActive() ), User::Invariant() );
        TCallBack callback( CMTPConnectionMgr::DoResumeSuspendedTransport, this );
        iTransportTrigger->Set( callback );
        iTransportTrigger->CallBack();
        }
    __FLOG( _L8("-ResumeSuspendedTransport") );
    }

/**
Resume suspended transport
@param aSelf, The memory address of the CMTPConnectionMgr instance
@return KErrNone, but the value is ignored.
*/
TInt CMTPConnectionMgr::DoResumeSuspendedTransport( TAny* aSelf )
    {
    CMTPConnectionMgr* self = reinterpret_cast< CMTPConnectionMgr* >( aSelf );
    __ASSERT_DEBUG( ( self->iSuspendedTransports.Count() > 0 ), User::Invariant() );
    TRAP_IGNORE( self->StartTransportL( self->iSuspendedTransports[self->iSuspendedTransports.Count()-1] ) );
    return KErrNone;
    }

