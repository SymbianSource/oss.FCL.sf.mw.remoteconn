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
* Description:  Hid remconbearer scan codes
 *
*/


#ifndef HIDREMCONBEARERBEARERSCANCODES_H
#define HIDREMCONBEARERBEARERSCANCODES_H

/**
 *  Side volume Key Codes
 */
const TInt KPSVolumeDownReleased = -3;
const TInt KPSVolumeDownPressed = -2;
const TInt KPSVolumeDownClicked = -1;
const TInt KPSVolumeNoKey = 0;
const TInt KPSVolumeUpClicked = 1;
const TInt KPSVolumeUpPressed = 2;
const TInt KPSVolumeUpReleased = 3;
const TInt KPSMuteClicked = 4;
const TInt KPSMutePressed = 5;
const TInt KPSMuteReleased = 6;
// Hook keys keys
const TInt KPSAnsweClicked = 1;
const TInt KPSHangUpClicked = 2;
const TInt KPSRedial = 3;
const TInt KPSVoiceDial = 4;

/**
 *  Media Key codes
 */
enum TMediaKeyValues
    {
    EPlayReleased = 0,
    EPlayPressed,
    EPlayClicked,
    EStopReleased,
    EStopPressed,
    EStopClicked,
    EForwardReleased,
    EForwardPressed,
    EForwardClicked,
    ERewindReleased,
    ERewindPressed,
    ERewindClicked,
    EFastForwardReleased,
    EFastForwardPressed,
    EFastForwardClicked,
    EBackwardReleased,
    EBackwardPressed,
    EBackwardClicked,
    EMuteReleased,
    EMutePressed,
    EMuteClicked
    };

#endif // HIDREMCONBEARERBEARERSCANCODE_H
//End of File
