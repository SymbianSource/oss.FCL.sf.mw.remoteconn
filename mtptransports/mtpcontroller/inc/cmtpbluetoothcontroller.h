// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPBLUETOOTHCONTROLLER_H_
#define CMTPBLUETOOTHCONTROLLER_H_

#include "cmtpcontrollerbase.h"
#include "cmtpoperator.h"

NONSHARABLE_CLASS( CMTPBluetoothController ) : public CMTPControllerBase, public MMTPOperatorNotifier
    {
public:
    static CMTPBluetoothController* NewL( CMTPBearerMonitor& aMon );
    ~CMTPBluetoothController();
    
private:// From CMTPControllerBase
    void ManageService( TBool aStatus );
    
private:// From MMTPOperatorNotifier
    void HandleStartTrasnportCompleteL( TInt aError );
    void HandleStopTrasnportCompleteL( TInt aError );
    
private:
    CMTPBluetoothController( CMTPBearerMonitor& aMon );
    
private:
    
    TBool iStat;
    CMTPOperator* iMTPOperator;
    };

#endif /* CMTPBLUETOOTHCONTROLLER_H_ */
