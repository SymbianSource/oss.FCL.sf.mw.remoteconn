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

#ifndef CMTPSERVER_H
#define CMTPSERVER_H

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#include "mtpdebug.h"
#include "rmtpframework.h"

class CMTPShutdown;

/**
Implements the MTP daemon server, which houses the MTP framework components, 
and implements the server side portion of the MTP Client API as a standard 
Symbian OS client/server interface. 
@internalTechnology
*/
class CMTPServer : public CPolicyServer
	{
public:

	~CMTPServer();

    static void RunServerL();
	void AddSession();
	void DropSession();
	
private: // From CPolicyServer
	
	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

private:

	static CMTPServer* NewLC();
	CMTPServer();
	void ConstructL();
	
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The MTP framework singletons.
    */
    RMTPFramework   iFrameworkSingletons;
	
	/**
	The active MTP client API session count.
	*/
	TInt            iSessionCount;
	
	/**
	The daemon server process shutdown timer
	*/
	CMTPShutdown*   iShutdown;
	};

/** 
MTP server panic codes.
*/
enum TMTPPanic
	{
	EPanicBadDescriptor         = -1,
	EPanicIllegalFunction       = -2,
	EPanicAlreadyReceiving      = -3,
	EPanicErrArgument           = -4
	};

void PanicClient(const RMessagePtr2& aMessage, TMTPPanic aPanic);
	
#endif // CMTPSERVER_H
