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

#ifndef CMTPCONTROLLERBASE_H_
#define CMTPCONTROLLERBASE_H_

#include <e32base.h>
#include <locodbearer.h>
#include "cmtpbearermonitor.h"

NONSHARABLE_CLASS( CMTPControllerBase ) : public CBase
    {
public:
    TLocodBearer Bearer() const;
    
    virtual void ManageService( TBool aStatus ) = 0;
    
protected:
    CMTPControllerBase( CMTPBearerMonitor& aMon, TLocodBearer aBearer );
    
    CMTPBearerMonitor& Monitor() const;
    
private:
    CMTPBearerMonitor &iMon;
    TLocodBearer iBearer;
    };

#endif /* CMTPCONTROLLERBASE_H_ */
