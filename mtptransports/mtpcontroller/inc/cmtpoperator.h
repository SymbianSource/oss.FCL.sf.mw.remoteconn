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

#ifndef CMTPOPERATOR_H_
#define CMTPOPERATOR_H_

#include <e32base.h>
#include <mtp/rmtpclient.h>
#include "mmtpoperatornotifier.h"
#include "mtpdebug.h"

NONSHARABLE_CLASS( CMTPOperator ) : public CActive
    {
private:
    enum TOperationType
        {
        EStartTransport,
        EStopTransport
        };
    
    struct TOperation
        {
        TInt iType;
        TUid iTransport;
        };
    
public:
    static CMTPOperator* NewL( MMTPOperatorNotifier& aNotifier );
    
    ~CMTPOperator();
    
public:
    void StartTransport( TUid aTransport );
    void StopTransport( TUid aTransport );
    
private:// From CActive
    void DoCancel();
    void RunL();
    
private:
    CMTPOperator( MMTPOperatorNotifier& aNotifier );
    void ConstructL();
    TInt AppendOperation( TOperationType aType, TUid aTransport );
    void Schedule( TInt aError );
    void HandleOperationL( const TOperation& aOperation );
private:
    /**
     * FLOGGER debug trace member variable.
     */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    MMTPOperatorNotifier& iNotifier;
    
    RArray< TOperation > iPendingOperations;
    
    RMTPClient iMTPClient;
    };

#endif /* CMTPOPERATOR_H_ */
