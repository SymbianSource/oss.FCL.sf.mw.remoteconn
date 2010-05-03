/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Internal Central Repository keys.
*
*/


#ifndef __UIKLAF_INTERNAL_CR_KEYS_H__
#define __UIKLAF_INTERNAL_CR_KEYS_H__

const TUid KCRUidUiklaf = { 0x101F8774 };

// Helper constants for free disk space levels.
#define DISK_SPACE_OK        0
#define DISK_SPACE_WARNING  -1
#define DISK_SPACE_CRITICAL -2

/**
 * Use KUikOODDiskFreeSpaceWarningNoteLevel instead of this!!
 * Threshold for disk space warning level (bytes).
 * Read-only key. Default value: 196608
 */
const TUint32 KUikOODDiskWarningThreshold  = 0x00000001;

/**
 * Threshold for disk space critical level (bytes).
 * Read-only key. Default value: 131072
 */
const TUint32 KUikOODDiskCriticalThreshold = 0x00000002;

/**
 * Threshold for low RAM level (bytes).
 * Read-only key. Default value: 750000
 */
const TUint32 KUikOOMRamLowThreshold       = 0x00000003;

/**
 * Threshold for good RAM level (bytes).
 * Read-only key. Default value: 1000000
 */
const TUint32 KUikOOMRamGoodThreshold      = 0x00000004;

/**
 * Timeout for application exit (milliseconds).
 * Read-only key. Default value: 1000000
 */
const TUint32 KUikOOMMaxAppExitTime        = 0x00000005;

/**
 * Threshold for disk space warning note level as percent of used disk space.
 * Read-only key. Default value: 95
 */
const TUint32 KUikOODDiskFreeSpaceWarningNoteLevel = 0x00000006;

/**
 * Threshold for disk space warning note level for mass memory.
 * Read-only key. Default value: 20971520
 */
const TUint32 KUikOODDiskFreeSpaceWarningNoteLevelMassMemory = 0x00000007;


#endif __UIKLAF_INTERNAL_CR_KEYS_H__

// End of file
