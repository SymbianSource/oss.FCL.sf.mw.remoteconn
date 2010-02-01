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

#ifndef __SERVERASYNCCAO_H
#define __SERVERASYNCCAO_H

#include <obexconstants.h>
#include <obexbase.h>

class CConsoleBase;
class CObexServer;

class CServerAsyncAO : public CActive
	{
public:
    static CServerAsyncAO* NewL(CObexServer& aObexServer);
	
	~CServerAsyncAO();
	void CompletionIndication(CObex::TOperation aOpcode, TObexResponse aResponse);
    void RequestIndication(CObex::TOperation aOpcode, CObexBaseObject* aObject);

    void RunL();

private:
	CServerAsyncAO(CObexServer& aObexServer);
	void ConstructL();
	void DoCancel();
private:
    CConsoleBase* iConsole;
    CObexServer& iObexServer;
    CObexBaseObject* iObject;

    enum TIndicationState
    	{
    	ERequestIndication,
    	ECompleteIndication
    	};
    TIndicationState iState;
    TObexResponse iDefaultResponse;
    };

#endif //SERVERASYNCCAO_H

