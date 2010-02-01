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
#include "serverhandlerfile.h"
#include "TOBEXCON.H"
#include "btextnotifiers.h"

#define EPOCIDENT _L8("EPOC32 ER5")


CObexServerHandlerFile::CObexServerHandlerFile(CActiveConsole* aParent)
	: CObexServerHandlerBase(aParent)
    {
    }

CObexServerHandlerFile* CObexServerHandlerFile::NewL(CActiveConsole* aParent, TTransport aTransport, TBool aUseRFile)
    {
    CObexServerHandlerFile* self = new (ELeave) CObexServerHandlerFile(aParent);

    CleanupStack::PushL(self);
    self->ConstructL(aTransport, aUseRFile);
    CleanupStack::Pop();
    return (self);
    }

void CObexServerHandlerFile::ConstructL(TTransport aTransport, TBool aUseRFile)
    {
    BaseConstructL(aTransport);
    
    iUseRFile = aUseRFile;
   
    iBuf = CBufFlat::NewL(10);//Was 5000, reduced to force more writes to file

	// Required to configure buffer for memory backed file writes
	iBuf->ResizeL(10); //was 5000

	// New style construction
	iObject = CObexBufObject::NewL(NULL);

	if(!iUseRFile)
		{
		iBuffering = ETrue;

		iObject->SetDataBufL(static_cast<const TDesC>(KFileName), iBuf);		
		}
	else
		{
		//Create the RFile to be used
		User::LeaveIfError(iFs.Connect());
		iFs.Delete(KRFileTestFileName); //Try deleting the file, in case it wasn't cleaned up previously.
		User::LeaveIfError(iFile.Create(iFs,KRFileTestFileName,EFileWrite));
		iParent->Console()->Printf(_L("\r\nCreated RFile '%S'\r\n"),&KRFileTestFileName);
		
		//Create a TObexRFileBackedBuffer object
		TObexRFileBackedBuffer details(*iBuf,iFile,CObexBufObject::ESingleBuffering);
	
		//Call CObexBufObject::SetDataBuf
		iObject->SetDataBufL(details);	
		}

	iParent->Console()->Printf(_L("\r\nStarting server\r\n"));
	}

CObexServerHandlerFile::~CObexServerHandlerFile ()
    {
	delete iObject; iObject = 0;
    delete iBuf; iBuf = 0;
    
	if(iUseRFile)
		{
		//Clean up the RFile we created for this test
		//File will already have been closed by obex.
		iFs.Delete(KRFileTestFileName);
		iFs.Close();	
		}
    }

void CObexServerHandlerFile::ToggleWriteBuffering()
	{
	iBuffering = !iBuffering;
	
	if (iBuffering)
		{
		iParent->Console()->Printf(_L("Enabling buffering...\r\n"));
		iObject->SetDataBufL(static_cast<const TDesC>(KFileName), iBuf);//Qualified
		}
	else
		{
		iParent->Console()->Printf(_L("Disabling buffering...\r\n"));
		iObject->SetDataBufL(static_cast<const TDesC>(KFileName));//Qualified
		}
	}
