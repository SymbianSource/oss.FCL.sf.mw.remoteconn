// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <es_sock.h>
#include <ir_sock.h>
#include <bautils.h>
#include <usbman.h>
#include <d32usbc.h>
#include "serverhandlerbase.h"
#include "serverhandler.h"
#include "TOBEXCON.H"
#include "btextnotifiers.h"


CObexServerHandler::CObexServerHandler(CActiveConsole* aParent)
	: CObexServerHandlerBase(aParent)
    {}

CObexServerHandler* CObexServerHandler::NewL(CActiveConsole* aParent, TTransport aTransport)
    {
    CObexServerHandler* self = new (ELeave) CObexServerHandler(aParent);

    CleanupStack::PushL(self);
    self->ConstructL(aTransport);
    CleanupStack::Pop();
    return (self);
    }

void CObexServerHandler::ConstructL(TTransport aTransport)
    {
    BaseConstructL(aTransport);
    
    iBuf = CBufFlat::NewL(5000);//5000 - what should I put??
    iObject  = CObexBufObject::NewL(NULL);
    
    iObject->SetDataBufL(iBuf);
	}

CObexServerHandler::~CObexServerHandler()
	{
	delete iObject; iObject = 0;
    delete iBuf; iBuf = 0;
	}
