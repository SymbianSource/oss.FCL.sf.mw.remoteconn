// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#include <e32base.h>

#include "cmtpserver.h"
#include "mtpclientserver.h"
#include "mtpdebug.h"
#include "rmtpframework.h"

/**
Implements a single server side instance of an MTP Client API session. This 
API is implemented as a standard Symbian OS client/server interface. 
@internalTechnology
*/
class CMTPServerSession : public CSession2
    {
public:

    CMTPServerSession();
    ~CMTPServerSession();
    
public: // From CSession2

    void CreateL();  
    void ServiceL(const RMessage2& aMessage);

private:
    
    void DoStartTransportL(const RMessage2& aMessage);
    void DoStopTransport(const RMessage2& aMessage);
    void DoIsAvailableL(const RMessage2& aMessage);
    TBool CheckIsAvailableL(TUid aNewUID,TUid aCurUID);
    TBool CheckIsBlueToothTransport(TUid aNewUid);
    
private:

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The MTP framework singletons.
    */
    RMTPFramework iSingletons;
    };
