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
 @internalComponent
*/

#ifndef MTPCOMMONCONST_H
#define MTPCOMMONCONST_H

#ifdef __SERIES60_
#include <uiklafinternalcrkeys.h>
#else
//Following is copied from 'UiklafInternalCRKeys.h', which is
//exported from S60 sdk, in Symbian CBR, this file does not exist.
const TUid KCRUidUiklaf = { 0x101F8774 };
const TUint32 KUikOODDiskFreeSpaceWarningNoteLevel = 0x00000006;
const TUint32 KUikOODDiskFreeSpaceWarningNoteLevelMassMemory = 0x00000007;
#endif

#ifdef __MTP_NAVIENGINE_TEST
//The free size of C-Drive of NaviEngine env is less than 21M,
//it will block many ONB test cases
const TInt KFreeSpaceThreshHoldDefaultValue(0);
const TInt KFreeSpaceExtraReserved(0);

#else
//MTP should reserve some disk space to prevent OOD(Out of Disk) monitor 
//popup 'Out of memory' note.When syncing music through ovi player,
//sometimes device screen get freeze with this note
//Be default, this value is read from Central Respository, if error while
//reading, use this one
const TInt KFreeSpaceThreshHoldDefaultValue(20*1024*1024);//20M bytes

//Beside the OOD threshold value, we need to reserve extra disk space
//for harvest server,Thumbnail manager to store their data, set this as 7M
const TInt KFreeSpaceExtraReserved(7*1024*1024);//7M bytes

#endif
#endif //MTPCOMMONCONST_H
