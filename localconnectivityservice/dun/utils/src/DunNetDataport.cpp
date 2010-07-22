/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dataport specific network resource accessor implementation
*
*/


#include <mmtsy_names.h>
#include "DunNetDataport.h"
#include "DunUtils.h"
#include "DunDebug.h"

_LIT(DUN_GGP_DATAPORT_CSY_PORT, "::DUN");
_LIT(DUN_GGP_DATAPORT_CSY, "DATAPORT");

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunNetDataport* CDunNetDataport::NewL( TInt aNumOfMaxChannels )
    {
    CDunNetDataport* self = new (ELeave) CDunNetDataport( aNumOfMaxChannels );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunNetDataport::~CDunNetDataport()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::~CDunNetDataport()")));
    TInt i;
    TInt count = iEntities.Count();
    for ( i=0; i<count; i++ )
        {
        DeleteNetworkEntity( i, ETrue );
        }
    DeleteNetwork();
    FTRACE(FPrint(_L( "CDunNetDataport::~CDunNetDataport() complete")));
    }

// ---------------------------------------------------------------------------
// From class MDunNetwork (MDunNetDataport -> MDunNetwork).
// Initializes network for Dataport
// Must be called before any other operation
// ---------------------------------------------------------------------------
//
void CDunNetDataport::InitializeL()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::InitializeL()")));
    AllocatePhoneObjectsL();
    FTRACE(FPrint(_L( "CDunNetDataport::InitializeL() complete")));
    }

// ---------------------------------------------------------------------------
// From class MDunNetDataport.
// Called when channel was created by transporter for Dataport
// Initializes network for channel creation
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::AllocateChannel( RComm*& aComm )
    {
    FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel()")));
    TInt firstFree = InitializeFirstFreeEntity();
    if ( firstFree < 0 )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() (not found) complete")));
        return firstFree;
        }
    if ( firstFree >= iEntities.Count() )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() (firstfree failed!) complete")));
        return KErrGeneral;
        }
    TInt retTemp = iEntities[firstFree].iMobileCall.OpenNewCall( iMobileLine );
    if ( retTemp != KErrNone )
        {
        RemoveEntity( firstFree );  // remove unused initialized channel
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() OpenNewCall FAILED %d" ), retTemp));
        return KErrGeneral;
        }
    retTemp = iEntities[firstFree].iMobileCall.Connect();
    if ( retTemp != KErrNone )
        {
        RemoveEntity( firstFree );  // remove unused initialized channel
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() iEntities[%d]->iMobileCall.Connect FAILED %d" ), firstFree, retTemp));
        return KErrGeneral;
        }
    RCall::TCommPort portName;
    portName.iPort.Copy( DUN_GGP_DATAPORT_CSY );
    portName.iPort.Append( DUN_GGP_DATAPORT_CSY_PORT );
    retTemp = iEntities[firstFree].iMobileCall.LoanDataPort( portName );
    if ( retTemp != KErrNone )
        {
        RemoveEntity( firstFree );  // remove unused initialized channel
        if ( retTemp == KErrEtelPortNotLoanedToClient )
            {
            FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() (too big) complete")));
            return KErrTooBig;
            }
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() iEntities[%d]->iMobileCall.LoanDataPort FAILED %d" ), firstFree, retTemp));
        return KErrGeneral;
        }
    FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() Created call object at index %d" ), firstFree));
    retTemp = iEntities[firstFree].iDataport.Open( iCommServer,
                                                   portName.iPort,
                                                   ECommExclusive,
                                                   ECommRoleDTE );
    if ( retTemp != KErrNone )
        {
        RemoveEntity( firstFree );  // remove unused initialized channel
        FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() FAILED to open dataport %d"), retTemp));
        return KErrGeneral;
        }
    iEntities[firstFree].iDataport.ResetBuffers();
    iEntities[firstFree].iEntityInUse = ETrue;
    aComm = &iEntities[firstFree].iDataport;
    FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() (iEntities[%d]->iDataport) opened"), firstFree));
    FTRACE(FPrint(_L( "CDunNetDataport::AllocateChannel() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunNetDataport.
// Called when channel was deleted/closed by transporter for Dataport
// Uninitializes network for channel deletion/close
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::FreeChannel( RComm* aComm )
    {
    FTRACE(FPrint(_L( "CDunNetDataport::FreeChannel()")));
    TInt i;
    TInt count = iEntities.Count();
    for ( i=0; i<count; i++ )
        {
        if ( &iEntities[i].iDataport == aComm )
            {
            if ( iEntities[i].iEntityInUse )
                {
                break;
                }
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::FreeChannel() (not found) complete")));
        return KErrNotFound;
        }
    DeleteNetworkEntity( i, ETrue );
    FTRACE(FPrint(_L( "CDunNetDataport::FreeChannel() (iEntities[%d]->iDataport) freed"), i));
    FTRACE(FPrint(_L( "CDunNetDataport::FreeChannel() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunNetDataport.
// Gets index by network ID for Dataport
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::GetIndexById( RComm* aComm )
    {
    FTRACE(FPrint(_L( "CDunNetDataport::GetIndexById()")));
    TInt i;
    TInt count = iEntities.Count();
    for ( i=0; i<count; i++ )
        {
        TDunDataportEntity& entity = iEntities[i];
        if ( entity.iEntityInUse )
            {
            if ( &entity.iDataport == aComm )
                {
                return i;
                }
            }
        }
    FTRACE(FPrint(_L( "CDunNetDataport::GetIndexById() (not found) complete")));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// CDunNetDataport::CDunNetDataport
// ---------------------------------------------------------------------------
//
CDunNetDataport::CDunNetDataport( TInt aNumOfMaxChannels ) :
    iNumOfMaxChannels( aNumOfMaxChannels )
    {
    }

// ---------------------------------------------------------------------------
// CDunNetDataport::ConstructL
// ---------------------------------------------------------------------------
//
void CDunNetDataport::ConstructL()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::ConstructL()")));
    if ( iNumOfMaxChannels < 0 )
        {
        User::Leave( KErrGeneral );
        }
    TInt retTemp = CDunUtils::ConnectCommsServer( iCommServer );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::ConstructL() FAILED to connect comms server")));
        User::Leave( retTemp );
        }
    retTemp = iCommServer.LoadCommModule( DUN_GGP_DATAPORT_CSY );
    if ( retTemp != KErrNone )
        {
        iCommServer.Close();
        FTRACE(FPrint(_L( "CDunNetDataport::ConstructL() FAILED to load comm module")));
        User::Leave( retTemp );
        }
    FTRACE(FPrint(_L( "CDunNetDataport::ConstructL() complete")));
    }

// ---------------------------------------------------------------------------
// Allocates phone objects for use
// ---------------------------------------------------------------------------
//
void CDunNetDataport::AllocatePhoneObjectsL()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL()" ) ));
    TInt retTemp;
    retTemp = iTelServer.Connect();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL() iTelServer Connect FAILED %d" ), retTemp ));
        User::Leave( retTemp );
        }
    retTemp = iTelServer.LoadPhoneModule( KMmTsyModuleName );
    if ( retTemp!=KErrNone && retTemp!=KErrAlreadyExists )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL() LoadPhoneModule FAILED %d" ), retTemp ));
        User::Leave( retTemp );
        }
    retTemp = iMobilePhone.Open( iTelServer, KMmTsyPhoneName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL() MobilePhone Open FAILED %d" ), retTemp ));
        User::Leave( retTemp );
        }
    retTemp = iMobileLine.Open( iMobilePhone, KMmTsyDataLineName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL() MobileLine Open FAILED %d" ), retTemp ));
        User::Leave( retTemp );
        }
    FTRACE(FPrint(_L( "CDunNetDataport::AllocatePhoneObjectsL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes first free entity
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::InitializeFirstFreeEntity()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::InitializeFirstFreeEntity()")));
    TInt i;
    TInt retTemp;
    TInt count = iEntities.Count();
    for ( i=0; i<count; i++ )
        {
        if ( !iEntities[i].iEntityInUse )
            {
            FTRACE(FPrint( _L("CDunNetDataport::InitializeFirstFreeEntity() complete" )));
            return i;
            }
        }
    // Free channel not found, now create new if possible
    if ( iNumOfMaxChannels!=0 && iEntities.Count()>=iNumOfMaxChannels )
        {
        FTRACE(FPrint( _L("CDunNetDataport::InitializeFirstFreeEntity() (too big) complete" )));
        return KErrTooBig;
        }
    TDunDataportEntity emptyEntity;
    emptyEntity.iEntityInUse = EFalse;
    retTemp = iEntities.Append( emptyEntity );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunNetDataport::InitializeFirstFreeEntity() (append failed!) complete" )));
        return retTemp;
        }
    FTRACE(FPrint(_L( "CDunNetDataport::InitializeFirstFreeEntity() complete")));
    return i;
    }

// ---------------------------------------------------------------------------
// Remove network entity by index
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::RemoveEntity( TInt aIndex )
    {
    FTRACE(FPrint(_L( "CDunNetDataport::RemoveEntity()")));
    if ( aIndex < 0 ||
         aIndex >= iEntities.Count() )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::RemoveEntity() (not found) complete")));
        return KErrNotFound;
        }
    DeleteNetworkEntity( aIndex, EFalse );
    FTRACE(FPrint(_L( "CDunNetDataport::RemoveEntity() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Deletes own internal data
// ---------------------------------------------------------------------------
//
void CDunNetDataport::DeleteNetwork()
    {
    FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork()")));
    if ( iMobileLine.SubSessionHandle() )
        {
        iMobileLine.Close();
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() mobile line closed")));
        }
    if ( iMobilePhone.SubSessionHandle() )
        {
        iMobilePhone.Close();
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() mobile phone closed")));
        }
    if ( iTelServer.Handle() )
        {
        iTelServer.UnloadPhoneModule( KMmTsyModuleName );
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() phone module unloaded")));
        iTelServer.Close();
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() phone module closed")));
        }
    if ( iCommServer.Handle() )
        {
        iCommServer.UnloadCommModule( DUN_GGP_DATAPORT_CSY );
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() comm module unloaded")));
        iCommServer.Close();
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() comm module closed")));
        }
    iEntities.Close();
    FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetwork() complete")));
    }

// ---------------------------------------------------------------------------
// Deletes one network entity at index aIndex for Dataport
// ---------------------------------------------------------------------------
//
TInt CDunNetDataport::DeleteNetworkEntity( TInt aIndex, TBool aCheckFree )
    {
    FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity()")));
    if ( aIndex < 0 ||
         aIndex >= iEntities.Count() )
        {
        FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() (not found) complete"), aIndex));
        return KErrGeneral;
        }
    TDunDataportEntity& entity = iEntities[aIndex];
    if ( (aCheckFree&&entity.iEntityInUse) || !aCheckFree )
        {
        if ( entity.iDataport.SubSessionHandle() )
            {
            // The next will set KSignalDTEOutputs down twice for RComm
            // local media case because CDunSignalCopy clears them also which
            // in turn causes plugin to free channel. But this probably won't
            // cause any harm.
            entity.iDataport.SetSignals( 0, KSignalDTEOutputs );
            FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() RComm signals set")));
            entity.iDataport.Close();
            FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() RComm closed")));
            }
        if ( entity.iMobileCall.SubSessionHandle() )
            {
            entity.iMobileCall.RecoverDataPort();
            FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() Dataport recovered")));
            entity.iMobileCall.Close();
            FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() Dataport closed")));
            }
        entity.iEntityInUse = EFalse;
        }
    FTRACE(FPrint(_L( "CDunNetDataport::DeleteNetworkEntity() complete")));
    return KErrNone;
    }
