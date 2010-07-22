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
* Description:  Hid timeout timer
*
*/

#ifndef C_TIMER_H
#define C_TIMER_H

#include <e32base.h>


/**
 * HID headset driver class
 *
 * This class will notify an object after a specified timeout.
 * 
 * @since S60 v3.2
 */        
NONSHARABLE_CLASS ( CWaitTimer ) : public CTimer
    {
public:
    /**
     * Two-phased constructor.
     * @param aPriority priority to use for this timer
     * @param aTimeOutNotify object to notify of timeout event
     */
    static CWaitTimer* NewL( TTimeIntervalMicroSeconds32 aTimeOutTime );

    /**
     * Two-phased constructor.
     * @param aPriority priority to use for this timer
     * @param aTimeOutNotify object to notify of timeout event
     */    
    static CWaitTimer* NewLC(TTimeIntervalMicroSeconds32 aTimeOutTime );

    /**
    * Destructor
    */
    ~CWaitTimer();

protected: 

    /**
     * From CTimer
     * Invoked when a timeout occurs
     * 
     * @since S60 v3.2
     * @return None.
     */
    virtual void RunL();

private:
    CWaitTimer();
    void ConstructL( TTimeIntervalMicroSeconds32 aTimeOutTime );

private: // Member variables
    CActiveSchedulerWait    iSyncWaiter;
    
    };
#endif // C_TIMER_H
