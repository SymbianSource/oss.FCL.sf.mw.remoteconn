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
* Description:  CSConAppInstaller header file
*
*/


#ifndef _SCONINSTALLER_H_
#define _SCONINSTALLER_H_

// INCLUDES

#include <e32base.h>
#include <e32cons.h>
#include <SWInstApi.h>

#include "sconinstqueue.h"

class CSConUninstall;

//============================================================
// Class CSConAppInstaller declaration
//============================================================	
NONSHARABLE_CLASS ( CSConAppInstaller ): public CActive
	{
	public:
		/**
		 * Constructor
		 * @param aQueue The address of CSConInstallerQueu
    	 * @return none
		 */
		CSConAppInstaller( CSConInstallerQueue* aQueue, RFs& aFs );
		
		/**
		 * Destructor
		 * @return none
		 */
		~CSConAppInstaller();
		
		/**
		 * Starts the installer task
		 * @param aTaskId Task number
    	 * @return none
		 */
		void StartInstaller( TInt& aTaskId );
		/**
		 * Stops the installer task
    	 * @return none
		 */
		void StopInstaller( TInt& aTaskId );
		
		/**
		 * Returns the active status of the installer
		 * @return ETrue if installer is active, else EFalse
		 */
		TBool InstallerActive() const;
		
	private:
		/**
		 * Implementation of CActive::DoCancel()
		 * @return none
		 */
		void DoCancel();
		/**
		 * Implementation of CActive::RunL()
		 * @return none
		 */
		void RunL();
		/**
		 * Executes ListInstalledApps task
		 * @return none
		 */
		void ProcessListInstalledAppsL();
		/**
		 * Execures UnInstall task
		 * @param CSConUninstall uninstall params
    	 * @return none
		 */
		void ProcessUninstallL( const CSConUninstall& aUninstallParams );
		
		void UninstallSisL( const CSConUninstall& aUninstallParams );
		void UninstallJavaL( const TUid& aUid, const TSConInstallMode aMode );
		void UninstallWidget( const TUid& aUid, const TSConInstallMode aMode );
		void DeleteFile( const TDesC& aPath );
		
	private:
	    enum TInstallerState
	        {
	        EIdle = 0,
	        EInstalling,
	        ESilentInstalling,
	        EUninstalling,
	        ESilentUninstalling,
	        ECustomUninstalling,
	        ESilentCustomUnistalling,
	        EListingInstalledApps
	        };
	    TInstallerState                 iInstallerState;
		CSConInstallerQueue*			iQueue; // Not owned
		SwiUI::RSWInstLauncher			iSWInst;
		SwiUI::TInstallOptions          iOptions;
        SwiUI::TInstallOptionsPckg      iOptionsPckg;   
		TInt							iCurrentTask;
		RFs&                            iFs;
	};
	
#endif // _SCONINSTALLER_H_

// End of file
