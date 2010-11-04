/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN USB configuration accessor and listener
*
*/


#include "DunUsbConfig.h"
#include "DunDebug.h"

const TUint KDunUsbSupportedConfigVersion = 1;

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunUsbConfig* CDunUsbConfig::NewL( MDunServerCallback* aServer,
                                    MDunUsbConfig* aCallback,
                                    TUint8 aProtocol )
    {
    CDunUsbConfig* self = new (ELeave) CDunUsbConfig( aServer,
                                                      aCallback,
                                                      aProtocol );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunUsbConfig::~CDunUsbConfig()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::~CDunUsbConfig()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunUsbConfig::~CDunUsbConfig() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::ResetData()
    {
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // NewL()
    iAcmProperty.Close();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Compares initialized protocol number to configuration by index
// ---------------------------------------------------------------------------
//
TInt CDunUsbConfig::GetConfigValidityByIndex( TInt aIndex, TBool& aValidity )
    {
    FTRACE(FPrint( _L("CDunUsbConfig::GetConfigValidityByIndex()" )));
    if ( IsActive() )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::GetConfigValidityByIndex() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iConfigExist )
        {
        TInt retTemp = GetConfiguration( iConfig );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L("CDunUsbConfig::GetConfigValidityByIndex() (get failed) complete" )));
            return retTemp;
            }
        }
    if ( aIndex < 0 ||
         aIndex >= iConfig.iAcmCount ||
         aIndex >= TPublishedAcmConfigs::KAcmMaxFunctions )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::GetConfigValidityByIndex() (not found) complete" )));
        return KErrNotFound;
        }
    if ( iConfig.iAcmConfig[aIndex].iProtocol == iProtocol )
        {
        aValidity = ETrue;
        }
    else
        {
        aValidity = EFalse;
        }
    FTRACE(FPrint( _L("CDunUsbConfig::GetConfigValidityByIndex() complete (%d/%d)" ), aIndex, aValidity));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Starts listening for ACM configuration changes
// ---------------------------------------------------------------------------
//
TInt CDunUsbConfig::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::IssueRequest()" )));
    if ( IsActive() )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::IssueRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    iAcmProperty.Subscribe( iStatus );
    SetActive();
    FTRACE(FPrint( _L("CDunUsbConfig::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops listening for ACM configuration changes
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::Stop()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::Stop()" )));
    Cancel();
    FTRACE(FPrint( _L("CDunUsbConfig::Stop() complete" )));
    }

// ---------------------------------------------------------------------------
// CDunUsbConfig::CDunUsbConfig
// ---------------------------------------------------------------------------
//
CDunUsbConfig::CDunUsbConfig( MDunServerCallback* aServer,
                              MDunUsbConfig* aCallback,
                              TUint8 aProtocol ) :
    CActive( EPriorityStandard ),
    iServer( aServer ),
    iCallback( aCallback ),
    iProtocol( aProtocol )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunUsbConfig::ConstructL
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::ConstructL()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::ConstructL()" )));
    if ( !iServer || !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    User::LeaveIfError( iAcmProperty.Attach(KUidSystemCategory,KAcmKey) );
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunUsbConfig::ConstructL() complete" )));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::Initialize()
    {
    // Don't initialize iCallback here (it is set through NewL)
    // Don't initialize iProtocol here (it is set through NewL)
    iConfig.iAcmConfigVersion = 0;
    iConfig.iAcmCount = 0;
    iConfigExist = EFalse;
    }

// ---------------------------------------------------------------------------
// Gets current ACM configuration
// ---------------------------------------------------------------------------
//
TInt CDunUsbConfig::GetConfiguration( TPublishedAcmConfigs& aConfig )
    {
    FTRACE(FPrint( _L("CDunUsbConfig::GetConfiguration()" )));
    TPckgBuf<TPublishedAcmConfigs> configBuf;
    TInt retTemp = iAcmProperty.Get( configBuf );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::GetConfiguration() (ERROR) complete" )));
        return retTemp;
        }
    aConfig = configBuf();
    // Check that version is that which we currently support
    if ( aConfig.iAcmConfigVersion != KDunUsbSupportedConfigVersion )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::GetConfiguration() (not supported) complete" )));
        return KErrNotSupported;
        }
    // Check that count is within bounds
    if ( aConfig.iAcmCount > TPublishedAcmConfigs::KAcmMaxFunctions )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::GetConfiguration() (overflow) complete" )));
        return KErrOverflow;
        }
    iConfigExist = ETrue;
    FTRACE(FPrint( _L("CDunUsbConfig::GetConfiguration() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when ACM configuration changes
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::RunL()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::RunL()" )));

    TPublishedAcmConfigs newConfig;
    TInt retTemp = GetConfiguration( newConfig );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::RunL() (ERROR) complete (%d)" ), retTemp));
        iServer->NotifyPluginCloseRequest( KDunUsbPluginUid, ETrue );
        return;
        }
    // Change that is same is not possible so check that first
    if ( newConfig.iAcmCount == iConfig.iAcmCount )
        {
        FTRACE(FPrint( _L("CDunUsbConfig::RunL() (no change) complete" )));
        iServer->NotifyPluginCloseRequest( KDunUsbPluginUid, ETrue );
        return;
        }

    // Now we have the changed configuration so find out are there added or
    // removed USB ACMs

    TInt i;
    if ( newConfig.iAcmCount > iConfig.iAcmCount )  // addition
        {
        // Addition is always done to end of ACM queue by N entry addition
        // Scan through new array and report change if necessary
        for ( i=iConfig.iAcmCount; i<newConfig.iAcmCount; i++ )
            {
            if ( newConfig.iAcmConfig[i].iProtocol == iProtocol )
                {
                iCallback->NotifyConfigAddition( i );
                }
            }
        }
    else  // removal ( newConfig.iAcmCount < iConfig.iAcmCount )
        {
        // Removal is always done to end of ACM queue by N entry removal
        // Scan through old array and report change if necessary
        for ( i=newConfig.iAcmCount; i<iConfig.iAcmCount; i++ )
            {
            if ( iConfig.iAcmConfig[i].iProtocol == iProtocol )
                {
                iCallback->NotifyConfigRemoval( i );
                }
            }
        }

    // Update config and restart listening
    iConfig = newConfig;
    IssueRequest();

    FTRACE(FPrint( _L("CDunUsbConfig::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunUsbConfig::DoCancel()
    {
    FTRACE(FPrint( _L("CDunUsbConfig::DoCancel()" )));
    iAcmProperty.Cancel();
    FTRACE(FPrint( _L("CDunUsbConfig::DoCancel() complete" )));
    }
