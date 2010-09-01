/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CSConAppInstaller implementation
*
*/


// INCLUDE FILES

#include <pathinfo.h>
#include <swi/sisregistryentry.h>
#include <swi/sisregistrypackage.h>
#include <swi/sisregistrysession.h>
#include <SWInstDefs.h>
#include <mmf/common/mmfcontrollerpluginresolver.h>
#include <javaregistry.h>

using namespace Java;

#include "debug.h"
#include "sconinstaller.h"
#include "sconpcdconsts.h"
#include "sconpcdutility.h"

_LIT8( KWidgetMimeType, "application/x-nokia-widget");

const TInt KSConSeConUidValue = 0x101f99f6;
const TUid KSConSeConUid = {KSConSeConUidValue};

// ============================= MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSConAppInstaller::CSConAppInstaller( CSConInstallerQueue* aQueue )
// Constructor
// -----------------------------------------------------------------------------
//
CSConAppInstaller::CSConAppInstaller( CSConInstallerQueue* aQueue, RFs& aFs ) :
    CActive( EPriorityStandard ), iQueue( aQueue ), iFs( aFs )
    {
    TRACE_FUNC;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::~CSConAppInstaller()
// Destructor
// -----------------------------------------------------------------------------
//
CSConAppInstaller::~CSConAppInstaller()
    {
    TRACE_FUNC;
    iSWInst.Close();
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::StartInstaller( TInt& aTaskId )
// Starts the installer task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::StartInstaller( TInt& aTaskId )
    {
    TRACE_FUNC_ENTRY;
    CSConTask* task = NULL;
    TRequestStatus* status = NULL;
    TInt err( KErrNone );
    
    TInt ret = iQueue->GetTask( aTaskId, task );
    
    if( aTaskId > 0 && ret != KErrNotFound )
        {
        if ( iInstallerState != EIdle || IsActive() )
            {
            LOGGER_WRITE("WARNING! SConAppInstaller was not on idle state!");
            iQueue->CompleteTask( aTaskId, KErrInUse );
            TRACE_FUNC_EXIT;
            return;
            }
        
        
        iCurrentTask = aTaskId;
        iQueue->SetTaskProgress( aTaskId, KSConCodeProcessingStarted );
        
        switch( task->GetServiceId() )
            {
            case EInstall :
                iQueue->ChangeQueueProcessStatus();
                err = iSWInst.Connect();
                
                if( err == KErrNone )
                    {
                    if ( task->iInstallParams->iMode == ESilentInstall )
                    	{
                    	LOGGER_WRITE( "Begin silent installation.. " );
                    	iOptions.iUntrusted = SwiUI::EPolicyNotAllowed;
                        iOptions.iOCSP = SwiUI::EPolicyNotAllowed;
                        iOptionsPckg = iOptions;
                        iInstallerState = ESilentInstalling;
                    	iSWInst.SilentInstall( iStatus, task->iInstallParams->iPath, iOptionsPckg );
                    	}
                    else
                    	{
                    	LOGGER_WRITE( "Begin to install.. " );
                    	iInstallerState = EInstalling;
                        iSWInst.Install( iStatus, task->iInstallParams->iPath );
                    	}
                    }
                
                break;
            case EUninstall :
                iQueue->ChangeQueueProcessStatus();
                err = iSWInst.Connect();
                
                if( err == KErrNone )
                    {
                    LOGGER_WRITE( "Begin to uninstall.. " );
                    
                    TRAP( err, ProcessUninstallL( *task->iUninstallParams ) );
                    if( err != KErrNone )
                        {
                        LOGGER_WRITE_1( "StartInstaller ProcessUninstallL err: %d", err );
                        status = &iStatus;
                        User::RequestComplete( status, err );
                        }
                    }
                
                break;
            case EListInstalledApps :
                iQueue->ChangeQueueProcessStatus();
                iInstallerState = EListingInstalledApps;
                TRAP( err, ProcessListInstalledAppsL() );
                status = &iStatus;
                User::RequestComplete( status, err );
                break;
            default :
                break;
            }

        SetActive();
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::StopInstaller( TInt& aTaskId )
// Stops the installer task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::StopInstaller( TInt& aTaskId )
    {
    TRACE_FUNC_ENTRY;
    //If the task is the current task, cancel it first
    if( iCurrentTask == aTaskId )
        {
        Cancel();
        iSWInst.Close();
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// TBool CSConAppInstaller::InstallerActive()
// returns installer activity status
// -----------------------------------------------------------------------------
//
TBool CSConAppInstaller::InstallerActive() const
    {
    if ( iInstallerState == EIdle )
        {
        return EFalse;
        }
    else
        {
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::DoCancel()
// Implementation of CActive::DoCancel()
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::DoCancel()
    {
    TRACE_FUNC_ENTRY;
    
    switch (iInstallerState)
        {
        case EInstalling:
            LOGGER_WRITE("Cancel normal install");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestInstall );
            break;
        case ESilentInstalling:
            LOGGER_WRITE("Cancel silent install");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestSilentInstall );
            break;
        case EUninstalling:
            LOGGER_WRITE("Cancel normal uninstall");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestUninstall );
            break;
        case ESilentUninstalling:
            LOGGER_WRITE("Cancel silent uninstall");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestSilentUninstall );
            break;
        case ECustomUninstalling: 
            LOGGER_WRITE("Cancel custom uninstall");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestCustomUninstall );
            break;
        case ESilentCustomUnistalling:
            LOGGER_WRITE("Cancel silent custom uninstall");
            iSWInst.CancelAsyncRequest( SwiUI::ERequestSilentCustomUninstall );
            break;
        default:
            LOGGER_WRITE("WARNING! Unknown state");
            break;
        }
    iInstallerState = EIdle;
    
    // find and complete current task
    CSConTask* task = NULL;
    TInt ret = iQueue->GetTask( iCurrentTask, task );

    if ( iCurrentTask > 0 && ret != KErrNotFound )
        {

        switch( task->GetServiceId() )
            {
            case EInstall :
                iQueue->CompleteTask( iCurrentTask, KErrCancel );
                break;
            case EUninstall :
                iQueue->CompleteTask( iCurrentTask, KErrCancel );
                break;
            default :
                break;
            }
        }
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::RunL()
// Implementation of CActive::RunL()
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::RunL()
    {
    TRACE_FUNC_ENTRY;
    iInstallerState = EIdle;
    iSWInst.Close();
    iQueue->ChangeQueueProcessStatus();
    TInt err( iStatus.Int() );
    LOGGER_WRITE_1( "CSConAppInstaller::RunL() iStatus.Int() : returned %d", err );
    
    CSConTask* task = NULL;
    TInt taskErr = iQueue->GetTask( iCurrentTask, task );
    
    LOGGER_WRITE_1( "CSConAppInstaller::RunL() GetTask %d", taskErr );
        
    if( taskErr == KErrNone )
        {
        if( task->GetServiceId() == EInstall && err == KErrNone )
            {
            LOGGER_WRITE( "CSConAppInstaller::RunL() : before DeleteFile" );
            //delete sis after succesfull install
            DeleteFile( task->iInstallParams->iPath );
            }
        }
    
    iQueue->CompleteTask( iCurrentTask, err );
    TRACE_FUNC_EXIT;
    }


// -----------------------------------------------------------------------------
// CSConAppInstaller::ProcessUninstallL( const CSConUninstall& aUninstallParams )
// Execures UnInstall task
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::ProcessUninstallL( const CSConUninstall& aUninstallParams )
    {
    TRACE_FUNC_ENTRY;
    LOGGER_WRITE_1( "aUid: 0x%08x", aUninstallParams.iUid.iUid );
    LOGGER_WRITE_1( "aName: %S", &aUninstallParams.iName );
    LOGGER_WRITE_1( "aVendor: %S", &aUninstallParams.iVendor );
    LOGGER_WRITE_1( "aType: %d", aUninstallParams.iType );
    LOGGER_WRITE_1( "aMode: %d", aUninstallParams.iMode );
    switch ( aUninstallParams.iType )
	    {
	    case ESisApplication:
	    case ESisAugmentation:
	    	UninstallSisL( aUninstallParams );
	    	break;
	    case EJavaApplication:
	    	UninstallJavaL( aUninstallParams.iUid,
    			aUninstallParams.iMode);
	    	break;
	    case EWidgetApplication:
	    	UninstallWidget( aUninstallParams.iUid,
	    		aUninstallParams.iMode );
	    	break;
	    default:
	    	User::Leave( KErrNotSupported );
	    }
    
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::UninstallSisL( const CSConUninstall& aUninstallParams )
// Uninstall sis package or augmentation
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::UninstallSisL( const CSConUninstall& aUninstallParams )
	{
	TRACE_FUNC_ENTRY;

	if ( aUninstallParams.iUid == KSConSeConUid )
	    {
	    LOGGER_WRITE("Cannot uninstall itself, leave");
	    // cannot uninstall itself
	    User::Leave( SwiUI::KSWInstErrFileInUse );
	    }
	
	Swi::RSisRegistrySession sisRegistry;
    User::LeaveIfError( sisRegistry.Connect() );
    CleanupClosePushL( sisRegistry );
    //Check if uid belongs to SIS package
    if( !sisRegistry.IsInstalledL( aUninstallParams.iUid ) )
        {
        CleanupStack::PopAndDestroy( &sisRegistry );
        User::Leave( KErrNotFound );
        }
    
    Swi::RSisRegistryEntry entry;
    CleanupClosePushL(entry);
    User::LeaveIfError( entry.Open( sisRegistry, aUninstallParams.iUid ) );
    if ( aUninstallParams.iType == ESisAugmentation )
        {
        // augmentation pkg
        LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL ESisAugmentation" );
        
        TBool augmentationFound(EFalse);
        // Get possible augmentations
        RPointerArray<Swi::CSisRegistryPackage> augPackages;
        CleanupResetAndDestroyPushL( augPackages );
        entry.AugmentationsL( augPackages );
        for ( TInt j( 0 ); j < augPackages.Count() && !augmentationFound; j++ )
            {
            Swi::RSisRegistryEntry augmentationEntry;
            CleanupClosePushL( augmentationEntry );
            augmentationEntry.OpenL( sisRegistry, *augPackages[j] );
            
            HBufC* augPackageName = augmentationEntry.PackageNameL();
            CleanupStack::PushL( augPackageName );
            HBufC* augUniqueVendorName = augmentationEntry.UniqueVendorNameL();
            CleanupStack::PushL( augUniqueVendorName );
            
            if ( !augmentationEntry.IsInRomL() 
                && augmentationEntry.IsPresentL()
                && aUninstallParams.iName.Compare( *augPackageName ) == 0
                && aUninstallParams.iVendor.Compare( *augUniqueVendorName ) == 0 )
                {
                // Correct augmentation found, uninstall it.
                augmentationFound = ETrue;
                TInt augmentationIndex = augPackages[j]->Index();
                LOGGER_WRITE_1( "CSConAppInstaller::ProcessUninstallL augmentationIndex %d", augmentationIndex );
        
                SwiUI::TOpUninstallIndexParam params;
                params.iUid = aUninstallParams.iUid;
                params.iIndex = augmentationIndex;
                SwiUI::TOpUninstallIndexParamPckg pckg( params );
                SwiUI::TOperation operation( SwiUI::EOperationUninstallIndex );
                if( aUninstallParams.iMode == ESilentInstall )
                    {
                    LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL : silent aug-sis-uninstall" );
                    SwiUI::TUninstallOptionsPckg options;
                    iInstallerState = ESilentCustomUnistalling;
                    iSWInst.SilentCustomUninstall( iStatus, operation, options, pckg, KSISMIMEType );
                    }
                else
                    {
                    LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL : unsilent aug-sis-uninstall" )
                    iInstallerState = ECustomUninstalling;
                    iSWInst.CustomUninstall( iStatus, operation, pckg, KSISMIMEType );
                    }
                }
            CleanupStack::PopAndDestroy( augUniqueVendorName );
            CleanupStack::PopAndDestroy( augPackageName );
            CleanupStack::PopAndDestroy( &augmentationEntry );
            }  
        CleanupStack::PopAndDestroy( &augPackages );
        
        if ( !augmentationFound )
            {
            LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL augmentation not found -> Leave" );
            User::Leave( KErrNotFound );
            }
        }
    else
        {
        // Only uninstall if not in rom and is present
        if ( !entry.IsInRomL() && entry.IsPresentL() )
            { 
            if ( aUninstallParams.iMode == ESilentInstall )
                {
                LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL : silent sis-uninstall" );
                SwiUI::TUninstallOptionsPckg options;
                iInstallerState = ESilentUninstalling;
                iSWInst.SilentUninstall( iStatus, aUninstallParams.iUid, options, KSISMIMEType );
                }
            else
                {
                LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL : unsilent sis-uninstall" )
                iInstallerState = EUninstalling;
                iSWInst.Uninstall( iStatus, aUninstallParams.iUid, KSISMIMEType );
                }
            }
        else
            {
            LOGGER_WRITE( "CSConAppInstaller::ProcessUninstallL sis not present -> Leave" );
            User::Leave( KErrNotFound );
            }
        }
    
    CleanupStack::PopAndDestroy( &entry );
	CleanupStack::PopAndDestroy( &sisRegistry );
	TRACE_FUNC_EXIT;
	}

// -----------------------------------------------------------------------------
// CSConAppInstaller::UninstallJavaL( const TUid& aUid, const TSConInstallMode aMode )
// Uninstall java package
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::UninstallJavaL( const TUid& aUid, const TSConInstallMode aMode )
	{
	TRACE_FUNC_ENTRY;
	CJavaRegistry* javaRegistry = CJavaRegistry::NewLC( );
	TBool entryExist = javaRegistry->RegistryEntryExistsL( aUid );
	CleanupStack::PopAndDestroy( javaRegistry ); 
	
    if( entryExist )
        {
        if( aMode == ESilentInstall )
            {
            LOGGER_WRITE( "CSConAppInstaller::UninstallJavaL : silent midlet-uninstall" )
            SwiUI::TUninstallOptionsPckg options;
            iInstallerState = ESilentUninstalling;
            iSWInst.SilentUninstall( iStatus, aUid, options, KMidletMIMEType );
            }
        else
            {
            LOGGER_WRITE( "CSConAppInstaller::UninstallJavaL : unsilent midlet-uninstall" )
            iInstallerState = EUninstalling;
            iSWInst.Uninstall( iStatus, aUid, KMidletMIMEType );
            }
        }
    else
        {
        LOGGER_WRITE( "CSConAppInstaller::UninstallJavaL java entry does not exist -> Leave" )
        User::Leave( KErrNotFound );
        }
    TRACE_FUNC_EXIT;
	}

// -----------------------------------------------------------------------------
// CSConAppInstaller::UninstallWidget( const TUid& aUid, const TSConInstallMode aMode )
// Uninstall widget
// -----------------------------------------------------------------------------
//
void CSConAppInstaller::UninstallWidget( const TUid& aUid, const TSConInstallMode aMode )
	{
	TRACE_FUNC_ENTRY;
	if( aMode == ESilentInstall )
        {
        LOGGER_WRITE( "CSConAppInstaller::UninstallWidget : silent uninstall" )
        SwiUI::TUninstallOptionsPckg options;
        iInstallerState = ESilentUninstalling;
        iSWInst.SilentUninstall( iStatus, aUid, options, KWidgetMimeType );
        }
    else
        {
        LOGGER_WRITE( "CSConAppInstaller::UninstallWidget : unsilent uninstall" )
        iInstallerState = EUninstalling;
        iSWInst.Uninstall( iStatus, aUid, KWidgetMimeType );
        }
	TRACE_FUNC_EXIT;
	}

    
//--------------------------------------------------------------------------------
//void CSConAppInstaller::ProcessListInstalledAppsL()
//--------------------------------------------------------------------------------
//
void CSConAppInstaller::ProcessListInstalledAppsL()
    {
    TRACE_FUNC_ENTRY;
    
    CSConTask* task = NULL;
    User::LeaveIfError( iQueue->GetTask( iCurrentTask, task ) );
    
    SConPcdUtility::ProcessListInstalledAppsL( task );
    
    TRACE_FUNC_EXIT;
    }

// -----------------------------------------------------------------------------
// CSConAppInstaller::DeleteFile( const TDesC& aPath )
// Deletes a file 
// -----------------------------------------------------------------------------
//  
void CSConAppInstaller::DeleteFile( const TDesC& aPath )    
    {
    TRACE_FUNC;
    iFs.Delete( aPath );
    }
// End of file
