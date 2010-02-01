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

#include "serverasyncao.h"
#include <e32cons.h>
#include <obexserver.h>

#include <obexobjects.h>


CServerAsyncAO* CServerAsyncAO::NewL(CObexServer& aObexServer)
	{
	CServerAsyncAO* self = new(ELeave) CServerAsyncAO(aObexServer);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}
	
CServerAsyncAO::CServerAsyncAO(CObexServer& aObexServer)
:CActive(EPriorityStandard),
 iObexServer(aObexServer)
	{
	CActiveScheduler::Add(this);
	}

void CServerAsyncAO::ConstructL()
	{
	iConsole = Console::NewL(_L("OBEX DLL Test Code"),TSize(55,12));
	}

CServerAsyncAO::~CServerAsyncAO()
	{
	delete iConsole;
	}

void CServerAsyncAO::CompletionIndication(CObex::TOperation aOpcode, TObexResponse aResponse)
	{
	_LIT(KPutIndication, "PutCompleteIndication\n");
	_LIT(KGetIndication, "GetCompleteIndication\n");
	_LIT(KSetPathIndication, "SetPathIndication\n");
	
	iState = ECompleteIndication;
	iDefaultResponse = aResponse;
	
	switch(aOpcode)
		{
		case CObex::EOpPut:
			iConsole->Printf(KPutIndication);
			break;
		case CObex::EOpGet:
			iConsole->Printf(KGetIndication);
			break;
		case CObex::EOpSetPath:
			iConsole->Printf(KSetPathIndication);
			break;
		default:
			User::Invariant();
		}

	iConsole->Printf(_L("Please enter the response code: 0x%x"), aResponse);
	iConsole->Read(iStatus);	
	SetActive();
	}

void CServerAsyncAO::RequestIndication(CObex::TOperation aOpcode, CObexBaseObject *aObject)
	{
	if(aOpcode == CObex::EOpPut)
		{
		iConsole->Printf(_L("Put"));
		}
	else if(aOpcode == CObex::EOpGet)
		{
		iConsole->Printf(_L("Get"));
		}
	else
		{
		User::Invariant();
		}
	iState = ERequestIndication;
	iObject = aObject;
	iConsole->Printf(_L("RequestIndication, press any key to continue\n"));
	iConsole->Read(iStatus);
	SetActive();
	}

void CServerAsyncAO::RunL()
	{
	TObexResponse resp = ERespSuccess; 
	TBuf<2> buf;

	TKeyCode key = iConsole->KeyCode();
	
	if(iState == ECompleteIndication)
		{
		buf.AppendFormat(_L("%x"), iDefaultResponse);
		do	{
		
			if(key == EKeyBackspace&&buf.Length()!=0)
				{
				buf.SetLength(buf.Length()-1);
				}
			else if( buf.Length() < buf.MaxLength())
				{
				buf.Append(key);
				}
			else 
				continue;
			iConsole->Printf(_L("%c"),key);
			}
		while((key = iConsole->Getch())!=EKeyEnter);
		iConsole->Printf(_L("\n"));
		
		TLex lex(buf);
		TUint value;
		TInt lex_err = lex.Val(value, EHex); 
		
		if(lex_err == KErrNone)
			{
			resp = static_cast<TObexResponse>(value);
			}
		else 
			{
			iConsole->Printf(_L("Input parsing failed, use success as default response\n"));
			}
			
		TInt ret = iObexServer.RequestCompleteIndicationCallback(resp);
		iConsole->Printf(_L("Server returned with error code %d\n\n"), ret);
		}
	else
		{
		TInt ret = iObexServer.RequestIndicationCallback(iObject);
		iConsole->Printf(_L("Server returned with error code %d\n\n"), ret);
		}
	}

void CServerAsyncAO::DoCancel()
	{
	iConsole->ReadCancel();
	}

