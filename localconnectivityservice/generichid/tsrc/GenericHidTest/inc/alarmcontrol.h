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


#ifndef C_ALARMCONTROL_H
#define C_ALARMCONTROL_H

#include <e32base.h>
#include <ASCliSession.h>
#include <TestScripterInternal.h>
#include <StifLogger.h>
#include "GenericHidTest.h"

// RDebug
#include <e32debug.h>

NONSHARABLE_CLASS ( CAlarmControl ): public CBase
{
public:
    static CAlarmControl* NewL(CStifLogger* aLogger);
    static CAlarmControl* NewLC(CStifLogger* aLogger);
    
    void CreateClockAlarm( TInt aTime );
    void DeleteAlarm();
public:

	virtual ~CAlarmControl();
	
private:
    CAlarmControl(CStifLogger* aLogger);
    void ConstructL();
    
    RASCliSession iAlarmServer;
    TInt iAlarmID;
    CStifLogger * iLog;
};

#endif /*ALARMCONTROL_H*/
