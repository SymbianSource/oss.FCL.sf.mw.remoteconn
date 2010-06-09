/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
* Constant definitions
* 
*
*/



/**
 @file
*/
#ifndef __SBEPARSERDEFS_H__
#define __SBEPARSERDEFS_H__

namespace conn
	{
	// Max number of characters that the field value for a hex UID can be i.e. 0x12345678
	const TInt KSBEMaxHexStringLength = 10;
	
	// XML type
	_LIT8(KMimeType, "text/xml");
	
	// Elements
	_LIT8(KSupportsSelective, "supports_selective"); 
	_LIT8(KPassiveBackup, "passive_backup");
	_LIT8(KPublicBackup, "public_backup");
	_LIT8(KIncludeFile, "include_file");
	_LIT8(KIncludeDirectory, "include_directory");
	_LIT8(KExclude, "exclude");
	_LIT8(KBackupRegistration, "backup_registration");
	_LIT8(KSystemBackup, "system_backup");
	_LIT8(KCenrepBackup, "cenrep_backup");
	_LIT8(KDBMSBackup, "dbms_backup");
	_LIT8(KActiveBackup, "active_backup");
	_LIT8(KRestore, "restore");
	_LIT8(KProxyDataManager, "proxy_data_manager");
	_LIT8(KJavaXMLElementMIDletSuite, "midlet_suite");
	_LIT8(KJavaXMLElementIncludeDirectory, "java_include_directory");
	_LIT8(KJavaXMLElementJavaBackupMIDlet, "java_backup_midlet");
	
	_LIT8(KProxySID, "SID");
	_LIT8(KHexLeader, "0x");
	_LIT8(KDatabase, "database");
	_LIT8(KPolicy, "policy");
	
	_LIT8(KProcessName, "process_name");
	_LIT8(KRequiresDelay, "requires_delay_to_prepare_data");
	_LIT8(KSupportsInc, "supports_incremental");
	_LIT8(KActiveType, "active_type");
	
	_LIT8(KActiveOnly, "activeonly");
	_LIT8(KActiveAndProxy, "activeandproxy");
	_LIT8(KProxyOnly, "proxyonly");
	_LIT8(KRequiresReboot, "requires_reboot");
	_LIT8(KDeleteBeforeRestore, "delete_before_restore");
	_LIT8(KBaseBackupOnly, "base_backup_only");
	
	_LIT8(KCallbackDelayTime, "callback_delay_time");

	}
#endif // __SBEPARSERDEFS_H__
