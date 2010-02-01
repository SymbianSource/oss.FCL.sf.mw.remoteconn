// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef PTPDEF_H
#define PTPDEF_H

#include <e32std.h>
#include <comms-infras/commsdebugutility.h> // for __FLOG debugging

_LIT( KPTPServer, "PtpServer" );
_LIT( KPTPClientPanicCategory, "PTPServ-Client");

__FLOG_STMT(_LIT8(KComponent,    "PictBridgeDp");)
__FLOG_STMT(_LIT8(KPtpServerLog, "PTPServer");)

const TInt KPtpServerVersionMinor = 1;
const TInt KPtpServerVersionMajor = 1;
const TInt KFileNameAndPathLength = 32;

enum TPtpPanicsClient
    {
    ECannotStartServer,
    EBadRequest,
    ERequestPending
    };

enum TPtpClientMessage 
    {
    EPtpCreateStack,
    EPtpDestroyStack,
    EPtpRegister,
    ECancelIsDpsPrinter,
    EIsDpsPrinter,
    EGetObjectHandleByName,
    EGetNameByObjectHandle,
    ESendObject,
    ECancelSendObject,
    EObjectReceivedNotify,
    ECancelObjectReceivedNotify,
    EPtpFolder
    };

#endif // PTPDEF_H

