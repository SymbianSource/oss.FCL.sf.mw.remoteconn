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
* Description:  Hid headset driver 
 *
*/


#ifndef C_HIDHEADSETDRIVER_H
#define C_HIDHEADSETDRIVER_H

#include <e32std.h>
#include <asclisession.h>
#include <mpxplaybackutility.h>
#include <mpxplaybackobserver.h>
#include <mpxmessage.h>
#include <hidinterfaces.h>
#include <hidtranslate.h>
#include "keypresstimer.h"

class CField;
class CHidHeadsetDriver;
class CTelephony;

/**
 *  Scan status
 */
enum TScanStatus
    {
    EScanNotPressed, EScanPressed, EScanLongPress
    };

/**
 * HID headset driver class
 *
 * HidDriver interface for handlign headset related 
 * functionality.
 *
 */

class CHidHeadsetDriver : public CHidDriver,
        public MTimerNotifier,
        public MMPXPlaybackObserver
    {

private:
    /**
     *  Initialisation states 
     */
    enum THeadsetDriverState
        {
        EUninitialised, // Driver has not been initialised 
        EInitialised, // Driver is initialised 
        EDisabled
        // The driver will not respond to interrupt data 
        };

    /**
     * Hook handling
     */
    enum THookHandling
        {
        EOnHook, EOffHook, ENoHook
        };
public:

    /**
     * Two-phased constructor.
     * @param aHid The generic HID layer that requested the driver     
     */
    static CHidHeadsetDriver* NewL( MDriverAccess* aHid );

    /**
     * Two-phased constructor.
     * @param aHid The generic HID layer that requested the driver     
     */
    static CHidHeadsetDriver* NewLC( MDriverAccess* aHid );

    /**
     * Destructor
     */
    virtual ~CHidHeadsetDriver();

private:

    // from base class CHidDriver
    /**     
     * From class CHidDriver
     * Called by the Generic HID layer to see if the driver can is able to
     * use reports from a newly-connected device. Supported hid fields van 
     * be later request with SupportedFieldCount function
     *  
     * @param aReportRoot HID report descriptor
     * @return KErrNone The driver will handle reports from the report 
     *         descriptor and KErrHidUnrecognised The driver cannot 
     *         handle reports from the device
     */
    TInt CanHandleReportL( CReportRoot* aReportRoot );

    /**     
     * From class CHidDriver
     * Called by the Generic HID layer when a device has been removed, prior
     * to the driver being removed.  This allows the driver to notify any 
     * applications of disconnection, if required
     *     
     * @param aReason The reason for device disconnection     
     */
    void Disconnected( TInt aReason );

    /**     
     * From class CHidDriver
     * Called by the Generic HID layer when data has been received from the 
     * device handled by this driver. 
     *     
     * @param aChannel The channel on which the data was received 
     *                                   (as defined by the transport layer)
     * @param aPayload A pointer to the data buffer     
     */
    TInt DataIn( CHidTransport::THidChannelType aChannel,
            const TDesC8& aPayload );

    /**     
     * From class CHidDriver
     * Called by the transport layers to inform the generic HID of the 
     * success of the last Set... command. 
     *     
     * @param aCmdAck Status of the last Set... command     
     */
    void CommandResult( TInt aCmdAck );

    /**     
     * From class CHidDriver
     * Called after a driver is sucessfully created by the Generic HID, when 
     * a device is connected 
     *     
     * @param aConnectionId An number used to identify the device in
     *                      subsequent calls from the driver to the 
     *                      generic HI     
     */
    void InitialiseL( TInt aConnectionId );

    /**     
     * From class CHidDriver
     * Resets the internal state of the driver (any pressed keys are released) 
     * and enables the driver  
     *     
     * @param aConnectionId An number used to identify the device in
     *                      subsequent calls from the driver to the 
     *                      generic HI     
     */
    void StartL( TInt aConnectionId );

    /**     
     * From class CHidDriver
     * Cancels all pressed keys and disables the driver (so it will not
     * process interrupt data)  
     *
     */
    void Stop();

    /**
     * From class CHidDriver
     * Return count of supported fields
     *          
     * @return Number of supported fields
     */
    TInt SupportedFieldCount();

    /**
     * From class CHidDriver
     * Set input handling registy 
     *          
     * @param aHandlingReg  a Input handling registry     
     */
    void SetInputHandlingReg( CHidInputDataHandlingReg* aHandlingReg );

    // from base class MTimerNotifier    
    /**     
     * From class MTimerNotifier
     * The function to be called when a timeout occurs.
     *     
     * @param aTimerType a Type of timer
     */
    void TimerExpired( TTimerType aTimerType );

    /**
     *  From class MMPXPlaybackObserver
     *  Handle playback message.    
     *   
     *  @param aMsg playback message, ownership not transferred. 
     *         Please check aMsg is not NULL before using it. If aErr is not 
     *         KErrNone, plugin might still call back with more info in the 
     *         aMsg.
     *  @param aErr system error code.
     */
    void HandlePlaybackMessage( CMPXMessage* aMsg, TInt /*aErr*/);

private:
    /**
     * Constructor
     * @param aHid The generic HID layer that requested the driver 
     */
    CHidHeadsetDriver( MDriverAccess* aHid );
    /**
     * ConstructL
     */
    void ConstructL();

private:

    /**
     * Called from within DataIn to handle interrupt and control channel data
     *
     * @param aPayload a Payload data
     * @return error if data can't be handled
     */
    TInt InterruptData( const TDesC8& aPayload );

    /**
     * Handles the report
     *
     * @param aReport HID report descriptor
     * @param aField HID field
     * @return KErrNotSupported if report can't be handled
     */
    TInt HandleReport( const TDesC8& aReport, const CField* aField );

    /**
     * Handles the report
     *
     * @param aReportTranslator Report Translator
     */
    void ReleasePressedKeys( TReportTranslator& aReportTranslator );

    /**
     * Handle hook key pres
     *     
     * @param aStatus Hook key status
     */
    void HookKeyPres( TBool aStatus );

    /**
     * Release hook key 
     *
     */
    void ReleaseHookKey();

    /**
     * Get Hook Status
     *     
     * @return hook handling status. 
     */
    THookHandling HookStatus();

    /**
     * Get alarm status
     *     
     * @return ETrue if ui is showing alarm, EFalse if not. 
     */
    TBool AlarmStatus();

    /**
     * MusicPlayingStatus
     *    
     * @return ETrue if plyaer is playing, EFalse if not. 
     */
    TBool MusicPlayingStatus();

    /**
     * HandleNoneHookPress
     * Handle hook press when there are phone call is in idle state
     *      
     */
    void HandleNoneHookPressL();

    /**
     * ReleasePressedVolumeKeys
     * Release volume keys if they are pressed
     *     
     * @param aReportTranslator HID report translator
     */
    void ReleasePressedVolumeKeys( TReportTranslator& aReportTranslator );

    /**
     * ReleasePressedScanKeys
     * Release scan keys if they are pressed
     *     
     * @param aReportTranslator HID report report translator     
     */
    void ReleasePressedScanKeys( TReportTranslator& aReportTranslator );
    /**
     * HandleAlarmHookReleaseL
     * Handle hook release when phone is alarming
     *     
     */
    void HandleAlarmHookReleaseL();

    /**
     * HandleIdleHookReleaseL
     * Handle hook release when phone is not alarming 
     * and call is in idle state
     *  
     */
    void HandleIdleHookReleaseL();

    /**
     * Handle doubleclick timer expiration
     *      
     */
    void ExpiredDoubleClickTimer();

    /**
     * Handle long press timer expiration
     *      
     */
    void ExpiredLongClickTimer();

    /**
     * Handle scan next pressed
     *      
     */
    void HandleScanNextPressL();

    /**
     * Handle scan prev pressed
     *      
     */
    void HandleScanPrevPressL();

    /**
     * Handle scan next release  
     *   
     * @param aKeyCode a Key id
     * @param aValue  a Key value      
     */
    void HandleScanNextRelease( TUint& aKeyCode, TInt& aValue );

    /**
     * Handle scan previous release  
     *    
     * @param aKeyCode a Key id
     * @param aValue a Key value    
     */
    void HandleScanPreviousRelease( TUint& aKeyCode, TInt& aValue );

    /**
     * Handle telephony usage page
     *   
     * @param aReport a Report translator
     * @return KErrNone if report can be handled.  
     */
    TInt HandleTelephonyUsage( TReportTranslator& aReport );

    /**
     * Handle controller usage page
     *    
     * @param aReport a Report translator
     * @return KErrNone if report can be handled.  
     */
    TInt HandleControlUsage( TReportTranslator& aReport );

    /**
     * Handle volume usage
     *    
     * @param aReport a Report translator
     * @return KErrNone if report can be handled.  
     */
    TInt HandleVolumeUsage( TReportTranslator& aReport );

private:

    /**
     * The Generic HID layer
     * Not own.
     */
    MDriverAccess *iGenericHid;

    /**
     * Field list array
     * Own. 
     */
    RPointerArray<CField> iFieldList;

    /**
     * Driver state
     */
    THeadsetDriverState iDriverState;

    /**
     * Connection id
     */
    TInt iConnectionId;

    /**
     * Application menu id 
     */
    TInt iAppMenuId;

    /**
     * On hook status 
     */
    TBool iOnHookPressed;

    /**
     * Off hook 
     */
    TBool iOffHookPressed;

    /**
     * No active calls
     */
    TBool iNoneHookPressed;

    /**
     * Telephony service
     * Own.
     */
    CTelephony* iTelephony;

    /**
     * Double click timer
     * Own.
     */
    CKeyPressTimer* iDoubleClicktimer;

    /**
     * Long click timer
     * Own.
     */
    CKeyPressTimer* iLongClicktimer;

    /**
     * Long click timer     
     */
    TBool iLongPress;

    /**
     * Alarm server session     
     */
    RASCliSession iAlarmServerSession;

    /**
     * Incoming call status
     */
     TBool iIncomingCallStatus;

    /**
     * Alarm statusd     
     */
    TBool iAlarmStatus;

    /**
     * Music player utility
     * Own.     
     */
    MMPXPlaybackUtility* iMusicPlayer;

    /**
     * Playing status     
     */
    TBool iPlayingStatus;

    /**
     * Double next Click  status
     */
    TBool iDoubleNextClick;

    /**
     * Count of supported fields
     */
    TInt iSupportedFieldCount;

    /**
     * Volume up key pressed down
     */
    TBool iVolumeUpPressed;

    /**
     * Volume down key pressed down
     */
    TBool iVolumeDownPressed;

    /**
     * Forward press status
     */
    TScanStatus iForwardStatus;

    /**
     * Backward press status
     */
    TScanStatus iBackwardStatus;

    /**
     * Scan next timer
     * Own.
     */
    CKeyPressTimer* iScanNextTimer;

    /**
     * Scan previous timer
     * Own.
     */
    CKeyPressTimer* iScanPreviousTimer;

    /**
     * Input handling reg
     * Not own.
     */
    CHidInputDataHandlingReg* iInputHandlingReg;

    };

#endif
