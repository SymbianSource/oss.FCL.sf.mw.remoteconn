/*
* Copyright (c) 2004-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Generic hid implementation
 *
*/


#include <etel3rdparty.h>
#include <e32std.h>
#include <e32svr.h>
#include <coedef.h>
#include <eiksvdef.h>
#include <apgcli.h>
#include <apgtask.h>
#include <e32property.h>
#ifdef NO101APPDEPFIXES
#include <alarmuidomainpskeys.h>
#else   //NO101APPDEPFIXES
const TUid KPSUidAlarmExtCntl = { 0x102072D4 }; // reusing an AlarmUI dll UID
const TUint32 KAlarmStopKey = 0x00000001;
enum TAlarmUIStopAlarm
    {
    EAlarmUIStopAlarmUninitialized = 0,
    EAlarmUIStopAlarm
    };
const TUint32 KAlarmSnoozeKey = 0x00000002;
enum TAlarmUISnoozeAlarm
    {
    EAlarmUISnoozeAlarmUninitialized = 0,
    EAlarmUISnoozeAlarm
    };
#endif  //NO101APPDEPFIXES

#include <mpxplaybackmessage.h>
#include <mpxmessagegeneraldefs.h>
#include <mpxplaybackmessagedefs.h>

#include "hidremconbearerinternalpskeys.h"
#include "hidremconbearerscancodes.h"
#include "headset.h"
#include "finder.h"

#include "debug.h"

const TInt KHidUndefinedKeyCode = 0;
const TInt KHidNotSetKeyValue = 0;
const TInt KDoubleClickTimeout = 900000; // 0,9 seconds
const TInt KDoubleClickTimeoutRing = 500000; // 0,5 seconds
const TInt KScanClickTimeout = 500000; // 0,5 seconds
const TInt KLongClickTimeout = 3000000; // 3 seconds

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CHidHeadsetDriver()
// ---------------------------------------------------------------------------
//
CHidHeadsetDriver::CHidHeadsetDriver( MDriverAccess* aGenericHid ) :
    iGenericHid( aGenericHid ), iFieldList(), iDriverState( EUninitialised ),
            iConnectionId( 0 ), iSupportedFieldCount( 0 ), iForwardStatus(
                    EScanNotPressed ), iBackwardStatus( EScanNotPressed )
    {
    TRACE_FUNC_ENTRY_THIS
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CHidHeadsetDriver* CHidHeadsetDriver::NewLC( MDriverAccess* aGenericHid )
    {
    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::NewLC(0x%08x)"), aGenericHid));
    CHidHeadsetDriver* self = new ( ELeave ) CHidHeadsetDriver( aGenericHid );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CHidHeadsetDriver* CHidHeadsetDriver::NewL( MDriverAccess* aGenericHid )
    {
    CHidHeadsetDriver* self = CHidHeadsetDriver::NewLC( aGenericHid );
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ConstructL()
    {
    TRACE_FUNC_THIS
    iTelephony = CTelephony::NewL();
    User::LeaveIfError( iAlarmServerSession.Connect() );
    iMusicPlayer = MMPXPlaybackUtility::UtilityL( KPbModeActivePlayer );
    iMusicPlayer->AddObserverL( *this );
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CHidHeadsetDriver::~CHidHeadsetDriver()
    {
    TRACE_FUNC_THIS
    iAlarmServerSession.Close();
    delete iTelephony;
    iFieldList.Reset();
    iFieldList.Close();
    delete iDoubleClicktimer;
    if ( iMusicPlayer )
        {
        iMusicPlayer->Close();
        }
    delete iScanPreviousTimer;
    delete iScanNextTimer;
    delete iLongClicktimer;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// StartL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::StartL( TInt /*aConnectionId*/)
    {
    TRACE_FUNC
    // Ready to process headset events:
    iDriverState = EInitialised;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// InitialiseL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::InitialiseL( TInt aConnectionId )
    {
    TRACE_FUNC
    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::InitialiseL(%d)"),
                        aConnectionId));
    // Store the connection ID:
    iConnectionId = aConnectionId;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// Stop()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::Stop()
    {
    TRACE_FUNC
    iDriverState = EDisabled;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// DataIn()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::DataIn( CHidTransport::THidChannelType aChannel,
        const TDesC8& aPayload )
    {
    TInt retVal = KErrNone;
    switch ( aChannel )
        {
        case CHidTransport::EHidChannelInt:
            if ( EInitialised == iDriverState )
                {
                retVal = InterruptData( aPayload );
                }
            break;

        case CHidTransport::EHidChannelCtrl:
            retVal = KErrNotSupported;
            break;

        default:
            retVal = KErrNotSupported;
            break;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// CommandResult()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::CommandResult( TInt /*aCmdAck*/)
    {
    // No implementation as we don't issue any requests to be acknowledged
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// Disconnected()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::Disconnected( TInt aReason )
    {
    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::Disconnected(%d)"), aReason));
    aReason = aReason;
    Stop();
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// SetInputHandlingReg()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::SetInputHandlingReg(
        CHidInputDataHandlingReg* aHandlingReg )
    {
    iInputHandlingReg = aHandlingReg;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// InterruptData()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::InterruptData( const TDesC8& aPayload )
    {
    // If the report has a report ID, it is in the first byte.
    // If not, this value is ignored (see CField::IsInReport()).
    //
    TInt retVal = KErrNone;
    TInt ret = KErrNotSupported;
    TInt firstByte = aPayload[0];

    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::InterruptData(), report \
            0x%x (%d), length %d"),firstByte, firstByte, aPayload.Length()));

    if ( iFieldList.Count() == 0 )
        {
        retVal = KErrNotSupported;
        }
    for ( TInt i = 0; i < iFieldList.Count(); i++ )
        {
        if ( iFieldList[i]->IsInReport( firstByte ) )
            {
            ret = HandleReport( aPayload, iFieldList[i] );
            if ( ret == KErrNone )
                {
                retVal = KErrNone;
                }
            }
        else
            {
            retVal = KErrNotSupported;
            }
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleReport()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::HandleReport( const TDesC8& aReport,
        const CField* aField )
    {
    TRACE_FUNC_ENTRY
    TReportTranslator report( aReport, aField );
    TInt retVal = KErrNotSupported;

    // release if key if it pressed and relased
    ReleasePressedKeys( report );

    if ( HandleTelephonyUsage( report ) == KErrNone )
        {
        retVal = KErrNone;
        }

    if ( HandleControlUsage( report ) == KErrNone )
        {
        retVal = KErrNone;
        }

    if ( HandleVolumeUsage( report ) == KErrNone )
        {
        retVal = KErrNone;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleTelephonyUsage()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::HandleTelephonyUsage( TReportTranslator& aReport )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNotSupported;
    TInt hookSwitch = 0;
    TInt mute = 0;
    TInt poc = 0;

    // P&S key variables
    TUint key = KHidUndefinedKeyCode;
    TInt value = KHidNotSetKeyValue;

    if ( aReport.GetValue( mute, ETelephonyUsagePhoneMute ) == KErrNone
            && mute == 1 )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send mute \
                command"));
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageTelephony,
                ETelephonyUsagePhoneMute ) )
            {
            key = KHidMuteKeyEvent;
            value = KPSMuteClicked;
            iInputHandlingReg->AddHandledEvent( EUsagePageTelephony,
                    ETelephonyUsagePhoneMute );
            }
        }
    else if ( aReport.GetValue( hookSwitch, ETelephonyUsageHookSwitch )
            == KErrNone && hookSwitch == 1 )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send hook \
                switch command (disabled)"));
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageTelephony,
                ETelephonyUsageHookSwitch ) )
            {
            HookKeyPres( ETrue );
            iInputHandlingReg->AddHandledEvent( EUsagePageTelephony,
                    ETelephonyUsageHookSwitch );
            }
        }
    else if ( aReport.GetValue( poc, ETelephonyUsagePoC ) == KErrNone && poc
            == 1 )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send PoC \
                command (disabled)"));
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageTelephony,
                ETelephonyUsagePoC ) )
            {
            key = KHidMuteKeyEvent;
            value = KPSMuteClicked;
            iInputHandlingReg->AddHandledEvent( EUsagePageTelephony,
                    ETelephonyUsagePoC );
            }
        }
    if ( key != KHidUndefinedKeyCode && value != KHidNotSetKeyValue )
        {
        TInt err = RProperty::Set( KPSUidHidEventNotifier, key, value );
        retVal = KErrNone;
        }

    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleTelephonyUsage()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::HandleControlUsage( TReportTranslator& aReport )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNotSupported;
    TInt playPause = 0;
    TInt scanNext = 0;
    TInt scanPrev = 0;
    TInt stop = 0;

    // P&S key variables
    TUint key = KHidUndefinedKeyCode;
    TInt value = KHidNotSetKeyValue;

    if ( aReport.GetValue( playPause, EConsumerUsagePlayPause ) == KErrNone
            && playPause )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsagePlayPause ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send \
                    play/pause command (currently only play)"));
            key = KHidControlKeyEvent;
            value = EPlayClicked;
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsagePlayPause );
            }
        }
    else if ( aReport.GetValue( scanNext, EConsumerUsageScanNext )
            == KErrNone && scanNext )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageScanNext ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send \
                    scan next command"));
            TRAP_IGNORE( HandleScanNextPressL() );
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageScanNext );
            }
        }
    else if ( aReport.GetValue( scanPrev, EConsumerUsageScanPrev )
            == KErrNone && scanPrev )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageScanPrev ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Scan \
                    prev command"));
            TRAP_IGNORE( HandleScanPrevPressL());
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageScanPrev );
            }
        }
    else if ( aReport.GetValue( stop, EConsumerUsageStop ) == KErrNone
            && stop )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageStop ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send \
                    stop command"));
            key = KHidControlKeyEvent;
            value = EStopClicked;
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageScanPrev );
            }
        }
    if ( key != KHidUndefinedKeyCode && value != KHidNotSetKeyValue )
        {
        TInt err = RProperty::Set( KPSUidHidEventNotifier, key, value );
        retVal = KErrNone;
        }

    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleVolumeUsage()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::HandleVolumeUsage( TReportTranslator& aReport )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNotSupported;

    TInt volUp = 0;
    TInt volDown = 0;
    TInt speakermute = 0;

    // P&S key variables
    TUint key = KHidUndefinedKeyCode;
    TInt value = KHidNotSetKeyValue;

    if ( aReport.GetValue( volUp, EConsumerUsageVolumeInc ) == KErrNone
            && volUp )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageVolumeInc ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): \
                    Send volume up command"));
            key = KHidAccessoryVolumeEvent;
            iVolumeUpPressed = ETrue;
            value = KPSVolumeUpPressed;
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageScanPrev );
            }
        }
    else if ( aReport.GetValue( volDown, EConsumerUsageVolumeDec )
            == KErrNone && volDown )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageVolumeDec ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send \
                    volume down command"));
            key = KHidAccessoryVolumeEvent;
            iVolumeDownPressed = ETrue;
            value = KPSVolumeDownPressed;
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageVolumeDec );
            }
        }
    else if ( ( aReport.GetValue( speakermute, EConsumerUsageMute )
            == KErrNone && speakermute == 1 ) )
        {
        if ( iInputHandlingReg->AllowedToHandleEvent( EUsagePageConsumer,
                EConsumerUsageMute ) )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Send \
                    mute command"));
            key = KHidMuteKeyEvent;
            value = KPSMuteClicked;
            iInputHandlingReg->AddHandledEvent( EUsagePageConsumer,
                    EConsumerUsageMute );
            }
        }

    if ( key != KHidUndefinedKeyCode && value != KHidNotSetKeyValue )
        {
        TInt err = RProperty::Set( KPSUidHidEventNotifier, key, value );
        retVal = KErrNone;
        }

    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleScanNextPressL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleScanNextPressL()
    {
    TRACE_FUNC_ENTRY
    iForwardStatus = EScanPressed;

    delete iScanNextTimer;
    iScanNextTimer = NULL;

    /**
     * Scan key has two different behaviour if you are short click buttun or long 
     * click button. In short press next track command is sent and whit long press
     * is sending seek forward command. iScanNextTimer is used to detect if click
     * is long click or short click.
     */

    iScanNextTimer = CKeyPressTimer::NewL( this, TTimeIntervalMicroSeconds32(
            KScanClickTimeout ), EScanNextPressTimer );

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleScanNextPressL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleScanPrevPressL()
    {
    TRACE_FUNC_ENTRY
    iBackwardStatus = EScanPressed;

    delete iScanPreviousTimer;
    iScanPreviousTimer = NULL;

    /**
     * Scan key has two different behaviour if you are short click buttun or 
     * long click button. In short press previous track command is sent and 
     * whit long press is sending seek forward command. iScanPreviousTimer 
     * is used to detect if click is long click or short click.
     */
    iScanPreviousTimer = CKeyPressTimer::NewL( this,
            TTimeIntervalMicroSeconds32( KScanClickTimeout ),
            EScanPrevPressTimer );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ReleasePressedVolumeKeys
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ReleasePressedVolumeKeys(
        TReportTranslator& aReportTranslator )
    {
    TInt volUp = 0;
    TInt volDown = 0;
    TUint key = KHidUndefinedKeyCode;
    TInt value = KHidNotSetKeyValue;

    if ( iVolumeUpPressed )
        {
        if ( aReportTranslator.GetValue( volUp, EConsumerUsageVolumeInc )
                == KErrNone && volDown == 0 )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::ReleasepressedKeys(): \
                    Volume up released"));
            key = KHidAccessoryVolumeEvent;
            iVolumeUpPressed = EFalse;
            value = KPSVolumeUpReleased;
            }
        }
    else if ( iVolumeDownPressed )
        {
        if ( aReportTranslator.GetValue( volDown, EConsumerUsageVolumeDec )
                == KErrNone && volDown == 0 )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::ReleasepressedKeys(): \
                    Volume downkey realeased"));
            key = KHidAccessoryVolumeEvent;
            iVolumeDownPressed = EFalse;
            value = KPSVolumeDownReleased;
            }
        }
    if ( key != KHidUndefinedKeyCode && value != KHidNotSetKeyValue )
        {
        TInt err = RProperty::Set( KPSUidHidEventNotifier, key, value );

        }
    }

// ---------------------------------------------------------------------------
// ReleasePressedScanKeys()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ReleasePressedScanKeys(
        TReportTranslator& aReportTranslator )
    {
    TInt scanNext = 0;
    TInt scanPrev = 0;
    TUint key = KHidUndefinedKeyCode;
    TInt value = KHidNotSetKeyValue;

    if ( iForwardStatus != EScanNotPressed )
        {
        if ( aReportTranslator.GetValue( scanNext, EConsumerUsageScanNext )
                == KErrNone && scanNext == 0 )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Scan \
                    next command released"));
            HandleScanNextRelease( key, value );
            }
        }
    else if ( iBackwardStatus != EScanNotPressed )
        {
        if ( aReportTranslator.GetValue( scanPrev, EConsumerUsageScanPrev )
                == KErrNone && scanPrev == 0 )
            {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Scan \
                    prev command released"));
            HandleScanPreviousRelease( key, value );
            }
        }
    if ( key != KHidUndefinedKeyCode && value != KHidNotSetKeyValue )
        {
        TInt err = RProperty::Set( KPSUidHidEventNotifier, key, value );
        }
    }
// ---------------------------------------------------------------------------
// ReleasePressedKeys()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ReleasePressedKeys(
        TReportTranslator& aReportTranslator )
    {
    TRACE_FUNC_ENTRY

    TInt hookSwitch = 0;
    /*
     * Check hook key release here, because hook key long press
     * is handled different way.
     */
    if ( iOnHookPressed || iOffHookPressed || iNoneHookPressed )
        {
        if ( aReportTranslator.GetValue( hookSwitch,
                ETelephonyUsageHookSwitch ) == KErrNone && hookSwitch == 0 )
            {
            ReleaseHookKey();
            }
        }
    ReleasePressedVolumeKeys( aReportTranslator );

    ReleasePressedScanKeys( aReportTranslator );

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleScanNextRelease()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleScanNextRelease( TUint& aKeyCode, TInt& aValue )
    {
    TRACE_FUNC_ENTRY
    aKeyCode = KHidControlKeyEvent;
    if ( iForwardStatus == EScanPressed )
        {
        aValue = EForwardClicked;
        }
    else //long press
        {
        aValue = EFastForwardReleased;
        }
    iForwardStatus = EScanNotPressed;

    delete iScanNextTimer;
    iScanNextTimer = NULL;

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleScanPreviousRelease()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleScanPreviousRelease( TUint& aKeyCode,
        TInt& aValue )
    {
    TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::HandleReport(): Scan prev \
            command released"));
    aKeyCode = KHidControlKeyEvent;
    if ( iBackwardStatus == EScanPressed )
        {
        aValue = EBackwardClicked;
        }
    else //long press
        {
        aValue = ERewindReleased;
        }
    iBackwardStatus = EScanNotPressed;

    delete iScanPreviousTimer;
    iScanPreviousTimer = NULL;

    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------------
// HookKeyPres()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HookKeyPres( TBool aStatus )
    {
    TRACE_FUNC_ENTRY

    THookHandling hookStatus = HookStatus();
    if ( !iAlarmStatus )
        {
        iAlarmStatus = AlarmStatus();
        }

    iPlayingStatus = MusicPlayingStatus();

    if ( aStatus )
        {
        switch ( hookStatus )
            {
            case EOnHook:
                if ( !iIncomingCallStatus )
                    {
                    // For the first click, trigger the timer 
                    // single click is handled in ExpiredDoubleClickTimer
                    if ( iDoubleClicktimer )
                        {
                        delete iDoubleClicktimer;
                        iDoubleClicktimer = NULL;
                        }
                    TRAP_IGNORE( iDoubleClicktimer = CKeyPressTimer::NewL( this,
                        TTimeIntervalMicroSeconds32( KDoubleClickTimeoutRing ),
                        EDoubleClickTimer ) );
                    if ( iDoubleClicktimer )
                        {
                        iIncomingCallStatus = ETrue;
                        }
                    else // If fail to create timer, handle as single click, 
                    // for double click case, the next click will hang off
                        {
                        iIncomingCallStatus = EFalse;
                        iOnHookPressed = ETrue;
                        }
                    break; // switch
                    }
                else
                    {
                    iIncomingCallStatus = EFalse;
                    if ( iDoubleClicktimer )
                        {
                        delete iDoubleClicktimer;
                        iDoubleClicktimer = NULL;
                        }
                    // This is the double click case, handle as EOffHook
                    }
                // No break here
            case EOffHook:
                TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook Off Pressed"));
                iOffHookPressed = ETrue;
                break;
            case ENoHook:
                TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook None Pressed")); 
                TRAP_IGNORE( HandleNoneHookPressL() );                
                break;
            default:
                TRACE_INFO(_L("CHidHeadsetDriver::HookKeyPres : Not \
                        supported"));                
            }
        }
    else
        {
        ReleaseHookKey();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ReleaseHookKey()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ReleaseHookKey()
    {
    TRACE_FUNC_ENTRY

    if ( iOnHookPressed )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook On released"));
        iOnHookPressed = EFalse;
        // Incoming call
        RProperty::Set( KPSUidHidEventNotifier, KHidHookKeyEvent,
                KPSAnsweClicked );
        }
    if ( iOffHookPressed )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook Off released"));
        iOffHookPressed = EFalse;
        // Disconnect connected call
        RProperty::Set( KPSUidHidEventNotifier, KHidHookKeyEvent,
                KPSHangUpClicked );
        }
    if ( iNoneHookPressed && !iAlarmStatus )
        {
        TRAP_IGNORE( HandleIdleHookReleaseL() );
        }
    if ( iNoneHookPressed && iAlarmStatus )
        {
        TRAP_IGNORE( HandleAlarmHookReleaseL() );
        }
    if ( iLongClicktimer )
        {
        iLongPress = EFalse;
        }
    delete iLongClicktimer;
    iLongClicktimer = NULL;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleNoneHook()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleNoneHookPressL()
    {
    TRACE_FUNC_ENTRY
    /**  Start long press timer is alarm is ongoing, or phone
     *   is not alarming and thre are no hook key press whitout
     *   release
     */
    if ( iAlarmStatus || ( !iNoneHookPressed && !iAlarmStatus ) )
        {
        iNoneHookPressed = ETrue;
        iLongPress = EFalse;

        delete iLongClicktimer;
        iLongClicktimer = NULL;

        iLongClicktimer = CKeyPressTimer::NewL( this,
                TTimeIntervalMicroSeconds32( KLongClickTimeout ),
                ELongPressTimer );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleAlarmHookReleaseL()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleAlarmHookReleaseL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook None released"));
    /**
     * Hook key is released when there is active alarm. Start double
     * click timer for monitoring double click.
     */
    if ( !iDoubleClicktimer && !iLongPress )
        {
        iDoubleClicktimer = CKeyPressTimer::NewL( this,
                TTimeIntervalMicroSeconds32( KDoubleClickTimeout ),
                EDoubleClickTimer );
        }
    /**
     * Stot alar when hook key is pressed long time during alarm.
     */
    else if ( iLongPress )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Stop alarm"));
        RProperty::Set( KPSUidAlarmExtCntl, KAlarmStopKey, 
                EAlarmUIStopAlarm );
        iAlarmStatus = EFalse;
        iLongPress = EFalse;
        }
    /**
     * Double click timer exsist and is it not long press, so 
     * this is double click release. Then snooze alarm.
     */
    else if ( !iLongPress )
        {
        delete iDoubleClicktimer;
        iDoubleClicktimer = NULL;
        iAlarmStatus = EFalse;
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Snooze"));
        RProperty::Set( KPSUidAlarmExtCntl, KAlarmSnoozeKey,
                EAlarmUISnoozeAlarm );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// HandleIdleHookReleaseL
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandleIdleHookReleaseL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Hook None released"));
    iNoneHookPressed = EFalse;
    /**
     * Hook key is released when there is music playing ongoing. Start double
     * click timer for monitoring double click.
     */
    if ( !iDoubleClicktimer && !iLongPress )
        {
        iDoubleClicktimer = CKeyPressTimer::NewL( this,
                TTimeIntervalMicroSeconds32( KDoubleClickTimeout ),
                EDoubleClickTimer );
        }
    /**
     * Hook key is released when there is not music playing ongoing. 
     * Because double click timer is active this is second release in
     * short beriod and redial needs to be issued.
     */
    else if ( !iLongPress && !iPlayingStatus )
        {
        delete iDoubleClicktimer;
        iDoubleClicktimer = NULL;
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver Redial"));
        RProperty::Set( KPSUidHidEventNotifier, KHidHookKeyEvent, 
                KPSRedial );
        }
    /**
     * Hook key is released when there is music playing ongoing. 
     * Because double click timer is active, is send forward command sent
     * and DoubleNextClick set to true for sending next command.
     */
    else if ( iPlayingStatus && !iLongPress )
        {
        delete iDoubleClicktimer;
        iDoubleClicktimer = NULL;
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver next after next"));
        if ( !iLongPress )
            {
            iDoubleNextClick = ETrue; //set to true and when player change 
                                      //track press next again 
            }
        RProperty::Set( KPSUidHidEventNotifier, KHidControlKeyEvent,
                EForwardClicked ); //next after next
        }
    /**
     * Long press release when there are no actie calls, alarms, and music
     * playing is stopped, activates voice dialing.
     */
    else if ( iLongPress )
        {
        delete iDoubleClicktimer;
        iDoubleClicktimer = NULL;
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver VoiceDial"));
        RProperty::Set( KPSUidHidEventNotifier, KHidHookKeyEvent,
                KPSVoiceDial );
        iLongPress = EFalse;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// GetHookStatus()
// ---------------------------------------------------------------------------
//
CHidHeadsetDriver::THookHandling CHidHeadsetDriver::HookStatus()
    {
    TRACE_FUNC_ENTRY
    THookHandling retVal = EOffHook;
    TInt ret = KErrNone;
    CTelephony::TCallStatusV1 callStatusV1;
    CTelephony::TCallStatusV1Pckg callStatusV1Pckg( callStatusV1 );
    ret = iTelephony->GetLineStatus( CTelephony::EVoiceLine,
                    callStatusV1Pckg );
    if ( ret != KErrNone )
        {
        retVal = ENoHook;
        }
    CTelephony::TCallStatus voiceLineStatus = callStatusV1.iStatus;
    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver GetHookStatus voiceline \
            status %d"), voiceLineStatus));

    if ( voiceLineStatus == CTelephony::EStatusHold || voiceLineStatus
            == CTelephony::EStatusRinging )
        {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver GetHookStatus on hook"));
        retVal = EOnHook;
        }
    else if ( voiceLineStatus == CTelephony::EStatusUnknown
            || voiceLineStatus == CTelephony::EStatusIdle )
        {
            TRACE_INFO(_L("[HID]\tCHidHeadsetDriver GetHookStatus no hook"));
        retVal = ENoHook;
        }
        TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// CanHandleReportL()
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::CanHandleReportL( CReportRoot* aReportRoot )
    {
    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::CanHandleReport(0x%08x)"),
                        aReportRoot));

    THidFieldSearch search;

    THeadsetFinder headsetFinder;
    search.SearchL( aReportRoot, &headsetFinder );

    iSupportedFieldCount = headsetFinder.FieldCount();
    for ( TInt i = 0; i < headsetFinder.FieldCount(); i++ )
        {
        User::LeaveIfError( iFieldList.Append( headsetFinder.GetFieldAtIndex(
                i ) ) );
        TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::CanHandleReportL, field \
                pointer: 0x%08x"), iFieldList[i]));
        }

    TInt valid = KErrHidUnrecognised;

    if ( headsetFinder.Found() )
        {
        valid = KErrNone;
        }
    // empty finder's field list before going out of scope.
    headsetFinder.EmptyList();

    TRACE_INFO((_L("[HID]\tCHidHeadsetDriver::CanHandleReport() returning \
            %d"), valid));
    return valid;
    }

// ---------------------------------------------------------------------------
// From class CHidDriver
// SupportedFieldCount
// ---------------------------------------------------------------------------
//
TInt CHidHeadsetDriver::SupportedFieldCount()
    {
    return iSupportedFieldCount;
    }
// ---------------------------------------------------------------------------
// From class MTimerNotifier
// TimerExpired()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::TimerExpired( TTimerType aTimerType )
    {
    TRACE_FUNC_ENTRY
    switch ( aTimerType )
        {
        case EDoubleClickTimer:
            ExpiredDoubleClickTimer();
            break;
        case ELongPressTimer:
            ExpiredLongClickTimer();
            break;
        case EScanNextPressTimer:
            iForwardStatus = EScanLongPress;
            RProperty::Set( KPSUidHidEventNotifier, KHidControlKeyEvent,
                    EFastForwardPressed );
            break;
        case EScanPrevPressTimer:
            iBackwardStatus = EScanLongPress;
            RProperty::Set( KPSUidHidEventNotifier, KHidControlKeyEvent,
                    ERewindPressed );
            break;
        default:
            TRACE_INFO(_L("CHidHeadsetDriver::TimerExpired : Not supported"))
            ;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ExpiredDoubleClickTimer()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ExpiredDoubleClickTimer()
    {
    TRACE_FUNC_ENTRY
    if ( iDoubleClicktimer )
        {
        delete iDoubleClicktimer;
        iDoubleClicktimer = NULL;
        
        if ( iIncomingCallStatus )
            {
            iIncomingCallStatus = EFalse;
            iOnHookPressed = ETrue;
            ReleaseHookKey();
            }
        if ( iAlarmStatus )
            {
            RProperty::Set( KPSUidAlarmExtCntl, KAlarmStopKey,
                    EAlarmUIStopAlarm );
            iAlarmStatus = EFalse;
            }
        if ( iPlayingStatus )
            {
            RProperty::Set( KPSUidHidEventNotifier, KHidControlKeyEvent,
                    EForwardClicked );
            iPlayingStatus = EFalse;
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ExpiredLongClickTimer()
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::ExpiredLongClickTimer()
    {
    TRACE_FUNC_ENTRY
    if ( iLongClicktimer )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::TimerExpired long click"));
        delete iLongClicktimer;
        iLongClicktimer = NULL;
        iLongPress = ETrue;
        ReleaseHookKey();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// AlarmStatus()
// ---------------------------------------------------------------------------
//
TBool CHidHeadsetDriver::AlarmStatus()
    {
    TRACE_FUNC_ENTRY
    TInt activealarmcount = 0;
    TBool retVal = EFalse;
    activealarmcount = iAlarmServerSession.AlarmCountByState(
            EAlarmStateNotifying );
    if ( activealarmcount > 0 )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::AlarmStatus active alarms"));
        retVal = ETrue;
        }
    else
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::AlarmStatus no active \
                alarms"));
        retVal = EFalse;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// AlarmStatus()
// ---------------------------------------------------------------------------
//
TBool CHidHeadsetDriver::MusicPlayingStatus()
    {
    TRACE_FUNC_ENTRY
    TBool retVal = EFalse;
    TMPXPlaybackState state = EPbStateNotInitialised;
    TRAPD( err, state = iMusicPlayer->StateL() );
    if ( state == EPbStatePlaying && err == KErrNone )
        {
        TRACE_INFO(_L("[HID]\tCHidHeadsetDriver::MusicPlayingStatus play \
                active"));
        retVal = ETrue;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandlePlaybackMessage
// ---------------------------------------------------------------------------
//
void CHidHeadsetDriver::HandlePlaybackMessage( CMPXMessage* aMsg, TInt /*aErr*/)
    {
    TRACE_FUNC_ENTRY

    TMPXMessageId id( aMsg->ValueTObjectL<TMPXMessageId> (
            KMPXMessageGeneralId ) );
    // send nect track whit double hook click when track is changed
    if ( id == KMPXMessagePbMediaChanged && iDoubleNextClick )
        {
        TRACE_INFO(_L("[HID]\tHandlePlaybackMessage: PbMediaChangeg"));
        RProperty::Set( KPSUidHidEventNotifier, KHidControlKeyEvent,
                EForwardClicked );
        iDoubleNextClick = EFalse;
        }
    TRACE_FUNC_EXIT
    }

