/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  HID field descriptor definition
*
*/

#ifndef __HIDREPORTS_H
#define __HIDREPORTS_H


#include <e32base.h>
#include <e32cmn.h>

const TUint headsetconnnection[]={
        0x95, 0x01,      //Report count 1
        0x05, 0x0b,      //Usage page telephony
        0x09, 0x01,      // Usage Phone
        0xa1, 0x02,      // Collection (logical)
        0x05, 0x09,      //Usage button
        0xc0             //End collection        
};

#endif

