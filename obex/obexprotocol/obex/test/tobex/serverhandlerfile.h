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

#ifndef _SERVERHANDLERFILE_H
#define _SERVERHANDLERFILE_H

#include <es_sock.h>
#include <e32test.h>
#include <es_prot.h>
#include <e32cons.h>
#include <obex.h>
#include <btmanclient.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexinternalconstants.h>
#include <btsdp.h>
#include <obexfinalpacketobserver.h>

class CActiveConsole;

class CObexServerHandlerFile : public CObexServerHandlerBase
//-----------------------------------------------------------
	{
public:
	// Construction
	static CObexServerHandlerFile* NewL(CActiveConsole* aParent, TTransport aTransport, TBool aUseRFile=EFalse);
	void ConstructL(TTransport aTransport, TBool aUseRFile);
	~CObexServerHandlerFile();
	
	void ToggleWriteBuffering();

private:
	CObexServerHandlerFile(CActiveConsole* aParent);

private:
	TBool iUseRFile;
	RFs iFs;
	RFile iFile;
	TBool iBuffering;
	};




#endif // _SERVERHANDLERFILE_H
