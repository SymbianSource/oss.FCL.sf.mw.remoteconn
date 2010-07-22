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
* Description:  HidRemconBearer internal Publish & Subscribe keys.
 *
*/


#ifndef HIDREMCONBEARERINTERNALPSKEYS_H
#define HIDREMCONBEARERINTERNALPSKEYS_H

const TUid KPSUidHidEventNotifier =
    {
    0x101f9067
    /*0x10282c19*/}; // hid event publisher uid

// Media Keys Notification API

const TUint32 KHidControlKeyEvent = 0x00000001;
const TUint32 KHidAccessoryVolumeEvent = 0x00000002;
const TUint32 KHidMuteKeyEvent = 0x00000003;
const TUint32 KHidHookKeyEvent = 0x00000004;

#endif      // __HIDREMCONBEARERINTERNALPSKEYS_H__
