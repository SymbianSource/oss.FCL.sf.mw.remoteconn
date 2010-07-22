/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialhandling class for hid test application
*
*/


#include "alarmcontrol.h"
#include "ASShdAlarm.h"
#include "asshddefs.h"

_LIT(KSoundName, "a");    // For the alarm alert note.
const TUid KAlarmClockOne = { 0x101F793A };

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CAlarmControl* CAlarmControl::NewL( CStifLogger* aLogger )
    {
    CAlarmControl* self = CAlarmControl::NewLC(aLogger);
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CAlarmControl* CAlarmControl::NewLC( CStifLogger* aLogger )
    {
    CAlarmControl* self = new( ELeave ) CAlarmControl(aLogger);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CAlarmControl::CAlarmControl( CStifLogger* aLogger ):
iLog(aLogger)
    {
    }

// ---------------------------------------------------------------------------
// destructor
// ---------------------------------------------------------------------------
//
CAlarmControl::~CAlarmControl()
    {
    if (iAlarmServer.Handle() != KNullHandle)
        {
        iAlarmServer.Close();    
        }    
    }

// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CAlarmControl::ConstructL()
    {   
    User::LeaveIfError(iAlarmServer.Connect());
    }
// ---------------------------------------------------------------------------
// CreateClockAlarm
// ---------------------------------------------------------------------------
//
void CAlarmControl::CreateClockAlarm( TInt aTime )
    {
    STIF_LOG("CreateClockAlarm");
    TInt minute;
    TTime tmp;
    tmp.HomeTime();
    TDateTime alarmdtime = tmp.DateTime();
    minute = alarmdtime.Minute();
    minute = minute+aTime;
    alarmdtime.SetMinute(minute);
    
    STIF_LOG("CreateClockAlarm solve minutes");
    TASShdAlarm alarm;
    alarm.Category() = KAlarmClockOne;
    alarm.RepeatDefinition() = EAlarmRepeatDefintionRepeatOnce;
    alarm.OriginalExpiryTime() = alarmdtime;
    alarm.NextDueTime() = alarmdtime;
    alarm.SoundName() = KSoundName;
    
    STIF_LOG("CreateClockAlarm solve data");
    alarm.ClientFlags().Set( 0 );
    
    STIF_LOG("CreateClockAlarm solve add alarm");
    iAlarmServer.AlarmAdd(alarm);
    iAlarmID = alarm.Id();
    
    STIF_LOG("CreateClockAlarm solve add activate alarm");
    iAlarmServer.SetAlarmStatus(iAlarmID, EAlarmStatusEnabled);
    
    STIF_LOG("CreateClockAlarm completed");
    }

// ---------------------------------------------------------------------------
// CreateClockAlarm
// ---------------------------------------------------------------------------
//
void CAlarmControl::DeleteAlarm()
    {
    iAlarmServer.AlarmDelete(iAlarmID);
    }
