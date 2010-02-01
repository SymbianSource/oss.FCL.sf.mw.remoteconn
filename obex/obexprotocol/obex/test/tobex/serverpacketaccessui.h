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

#ifndef _SERVERPACKETACCESSUI_H
#define _SERVERPACKETACCESSUI_H

#include <e32base.h>
#include <obex/extensionapis/mobexserverrequestpacketnotify.h>


class CObexServerHandlerBase;
class CConsoleBase;
class CObexServerPacketAccessExtension;
class CObexServer;


NONSHARABLE_CLASS(CObexServerPacketAccessUi) : public CActive, MObexServerRequestPacketNotify
	{
	
public:
	static CObexServerPacketAccessUi* NewL(CObexServerHandlerBase& aHandler, CObexServer& aServer);
	~CObexServerPacketAccessUi();
	
	void SetKeypressActive();
	CConsoleBase& Console() const;

private:	
	// MObexServerRequestPacketNotify
	virtual TBool RequestPacket(TObexRequestCode aRequest, TObexResponse& aResponse);
	virtual TBool RequestPacket(TObexRequestCode aRequest, TObexConnectInfo& aConnectInfo, TObexResponse& aResponse);
	virtual TBool RequestPacket(TObexRequestCode aRequest, CObex::TSetPathInfo& aSetPathInfo, TObexResponse& aResponse);

private:
	CObexServerPacketAccessUi(CObexServerHandlerBase& aHandler, CObexServer& aServer);
	void ConstructL();
	
	void RunL();
	void DoCancel();
	
	void ProcessKeyPressL(TChar aChar);
	
	void PrintHeadersL(CObexHeaderSet& aHeaderSet) const;
	void PrintHeader(CObexHeader& aHeader) const;
	void PrintHex(const TUint8* aPtr, TInt aLength) const;
	
	TBool RequestPacketReturn(TObexResponse& aResponse);

private:
	CObexServerHandlerBase& iHandler;
	CObexServer& iServer;

	CConsoleBase* iConsole;
	CObexServerPacketAccessExtension* iPacketAccess;
	
	TBool iInteractive;
	
	};


#endif // _SERVERPACKETACCESSUI_H
