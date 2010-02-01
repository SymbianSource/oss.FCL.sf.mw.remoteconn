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

#ifndef CMTPXMLPARSERLOGGER_H
#define CMTPXMLPARSERLOGGER_H

#include <e32base.h>

_LIT(KNodeTestCase, "TestCase");
_LIT(KNodeConnect, "Connect");
_LIT(KNodeTransaction, "Transaction");
_LIT(KNodeOperationCode, "OperationCode");
_LIT(KNodeSessionID, "SessionID");
_LIT(KNodeTransactionID, "TransactionID");
_LIT(KNodeParameter1, "Parameter1");
_LIT(KNodeParameter2, "Parameter2");
_LIT(KNodeParameter3, "Parameter3");
_LIT(KNodeParameter4, "Parameter4");
_LIT(KNodeParameter5, "Parameter5");
_LIT(KNodeDataIn, "DataIn");
_LIT(KNodeDataOut, "DataOut");
_LIT(KNodeRequest, "Request");
_LIT(KNodeResponse, "Response");
_LIT(KNodeResponseCode, "ResponseCode");
_LIT(KNodeEvent, "Event");
_LIT(KNodeEventCode, "EventCode");
_LIT(KNotValid, "NotValid");
_LIT(KATTRTransactionName, "TransactionName");

_LIT(KVersionTag, "?xml version=\"1.0\" encoding=\"UTF-8\"?");

const static TInt KMaxAttributeLength = 50;
const static TInt KMaxNodeNameLength = 50;

#endif // CMTPXMLPARSERLOGGER_H