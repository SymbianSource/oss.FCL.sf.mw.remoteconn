/*
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
* Description:  Hid timeout timer
 *
*/

#ifndef C_KEYPRESSTIMER_H
#define C_KEYPRESSTIMER_H

#include <e32base.h>

enum TTimerType
    {
    EDoubleClickTimer,
    ELongPressTimer,
    EScanNextPressTimer,
    EScanPrevPressTimer
    };

/**
 * HID headset driver class
 *
 * This class specifies the function to be called when a timeout occurs.
 * Used in conjunction with CTimeOutTimer class
 * 
 */
class MTimerNotifier
    {
public:
    /**     
     * The function to be called when a timeout occurs.
     *     
     * @param aTimerType a Type of timer
     */
    virtual void TimerExpired( TTimerType aTimerType ) = 0;
    };

/**
 * HID headset driver class
 *
 * This class will notify an object after a specified timeout.
 *
 */
class CKeyPressTimer : public CTimer
    {
public:
    /**
     * Two-phased constructor.     
     * @param aTimeOutNotify object to notify of timeout event
     * @param aTimeOutTime a Timeout time
     * @paran aTimerType a Type of timer
     * @return keypress timer
     */
    static CKeyPressTimer* NewL( MTimerNotifier* aTimeOutNotify,
            TTimeIntervalMicroSeconds32 aTimeOutTime, TTimerType aTimerType );

    /**
     * Two-phased constructor.
     * @param aTimeOutNotify object to notify of timeout event
     * @param aTimeOutTime a Timeout time
     * @paran aTimerType a Type of timer
     * @return keypress timer 
     */
    static CKeyPressTimer* NewLC( MTimerNotifier* aTimeOutNotify,
            TTimeIntervalMicroSeconds32 aTimeOutTime, TTimerType aTimerType );

    /**
     * Destructor
     */
    virtual ~CKeyPressTimer();

protected:

    /**
     * From CTimer
     * Invoked when a timeout occurs
     *     
     */
    virtual void RunL();

private:

    /**
     * Constructor
     * @param aTimeOutNotify object to notify of timeout event
     * @paran aTimerType a Type of timer
     */
    CKeyPressTimer( MTimerNotifier* aTimeOutNotify, TTimerType aTimerType );
    /**
     * Two-phased constructor.     
     * @param aTimeOutTime a Timeout time     
     */
    void ConstructL( TTimeIntervalMicroSeconds32 aTimeOutTime );

private:
    // Member variables

    /**
     *  The observer for this objects events
     *  Not own. 
     */
    MTimerNotifier* iNotify;

    /**
     * Type of timer
     */
    TTimerType iTimerType;
    };
#endif // C_KEYPRESSTIMER_H
