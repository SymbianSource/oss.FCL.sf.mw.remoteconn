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

#ifndef MMTPOPERATORNOTIFIER_H_
#define MMTPOPERATORNOTIFIER_H_

NONSHARABLE_CLASS( MMTPOperatorNotifier )
    {
public:
    virtual void HandleStartTrasnportCompleteL( TInt aError ) = 0;
    virtual void HandleStopTrasnportCompleteL( TInt aError ) = 0;
    };

#endif /* MMTPOPERATORNOTIFIER_H_ */
