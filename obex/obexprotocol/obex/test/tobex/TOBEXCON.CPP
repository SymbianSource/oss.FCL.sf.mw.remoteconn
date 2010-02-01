// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <e32keys.h>
#include "TOBEXCON.H"
#include "clienthandler.h"
#include "serverhandlerbase.h"
#include "serverhandler.h"
#include "serverhandlerfile.h"
#include "btextnotifiers.h"

#ifdef EMPTY_HEADERS_TEST
#include "emptyheaderstest.h"
#endif //EMPTY_HEADERS_TEST

#define EPOCIDENT _L8("EPOC32 ER5")


CActiveConsole::CActiveConsole(CConsoleBase* aConsole) 
    : CActive(EPriorityNormal),
      iConsole(aConsole),
      iTestMode(E_Inactive),
      iDiscoverySlots(KObexIrdaDefaultDiscoverySlotsToUse),
      iDiscoveryAttempts(KObexIrdaDefaultDiscoveryAttempts),
      iRecvMTU(KObexPacketDefaultSize),
      iTransmitMTU(KObexPacketDefaultSize)
    {
#ifdef EMPTY_HEADERS_TEST
	iDisplayHeaders = ETrue;
#endif //EMPTY_HEADERS_TEST    
    }

CActiveConsole* CActiveConsole::NewLC(CConsoleBase* aConsole)
    {
    CActiveConsole* self = new (ELeave) CActiveConsole(aConsole);
	CleanupStack::PushL(self);
	self->ConstructL();
    return self;
    }

void CActiveConsole::ConstructL ()
    { 
    iRefHandler = CReferenceHandler::NewL();
    iConsole->Printf(_L("\r\n"));
    CActiveScheduler::Add(this);			// Add to active scheduler
    
    // Create private path and set default Inbox path
    RFs fs;
    User::LeaveIfError(fs.Connect());
	fs.CreatePrivatePath(EDriveC);
	fs.SetSessionToPrivate(EDriveC);	
	fs.SessionPath(iInboxRoot);
	fs.Close();
    }

CActiveConsole::~CActiveConsole()
	{
	// Make sure we're cancelled
	Cancel();

	// safe to delete NULL
	delete iObexClientHandler;
	delete iObexServerHandler;
	delete iObexServerHandlerFile;
	delete iRefHandler;
	}

void  CActiveConsole::DoCancel()
    {
    iConsole->ReadCancel();
    }

void  CActiveConsole::RunL()
    {
	// Save the current test mode in case the next action leaves
	TUint oldTestMode = iTestMode;
    TRAPD(err,ProcessKeyPressL(TChar(iConsole->KeyCode())));
	if(err != KErrNone)
		{
		iConsole->Printf(_L("Failed. Error: %d\r\n"),err);
		// Put the test mode back again
		iTestMode = oldTestMode;
		RequestCharacter();
		}
    }

void CActiveConsole::RequestCharacter()
    {
    // A request is issued to the CConsoleBase to accept a
    // character from the keyboard.
    if (iTestMode == E_Client)
		{
		iConsole->Printf(_L("Commands :- \r\n"));
		iConsole->Printf(_L(" A  -  IrObex Abort\r\n"));
		iConsole->Printf(_L(" 1  -  IrObex Connect Menu\r\n"));
		iConsole->Printf(_L(" 2  -  IrObex Disconnect\r\n"));
		iConsole->Printf(_L(" 3  -  IrObex Put obj 1 : %S\r\n"), &(iObexClientHandler->iFilename1));
		iConsole->Printf(_L(" 4  -  IrObex Put Obj 2 : %S\r\n"), &(iObexClientHandler->iFilename2));
		iConsole->Printf(_L(" z  -  IrObex Put Obj 3 : %S\r\n"), &(iObexClientHandler->iFilename3));
		iConsole->Printf(_L(" y  -  Enter HTTP/App param Test Menu\r\n")); //E_Client_HTTP_Menu
		iConsole->Printf(_L(" 5  -  IrObex Get by name\r\n"));
		iConsole->Printf(_L(" 6  -  IrObex Get by type\r\n"));
		iConsole->Printf(_L(" 7  -  IrObex SetPath\r\n"));
		iConsole->Printf(_L(" 8  -  IrObex Put reference object\r\n"));
		iConsole->Printf(_L(" 9  -  IrObex Get reference object\r\n"));
		iConsole->Printf(_L(" 0  -  IrObex Setup OBEX\r\n"));
		iConsole->Printf(_L(" h  -  Toggle display of object headers\r\n"));
#ifdef ERROR_RESOLUTION_EXTENSION
		iConsole->Printf(_L(" l  -  Display last error\r\n"));
#endif
#ifdef EMPTY_HEADERS_TEST
		iConsole->Printf(_L(" t  -  Empty headers test menu\r\n"));
#endif //EMPTY_HEADERS_TEST
		iConsole->Printf(_L(" s  -  kill the Obex Client\r\n"));
		}
	if ( iTestMode == E_Client_HTTP_Menu )
		{ 
		iConsole->Printf(_L(" 1  -  HTTP Multiple Packets, will all arrive\r\n"));
		iConsole->Printf(_L(" 2  -  HTTP Multiple Packets, too many arrive in two parts\r\n"));
		iConsole->Printf(_L(" 3  -  HTTP Single Packet too large, nothing will arrive\r\n"));
		iConsole->Printf(_L(" 4  -  HTTP Single Packet too large with several smaller packets which will arrive\r\n"));
		iConsole->Printf(_L(" 5  -  App parameters test\r\n"));
		iConsole->Printf(_L(" s  -  Back to Client Menu\r\n"));
		}
	if ( iTestMode == E_Client_Connect_Menu )
		{
		iConsole->Printf(_L(" 0  -  IrObex Disconnect\r\n"));
		iConsole->Printf(_L(" 1  -  IrObex Connect\r\n"));
		iConsole->Printf(_L(" 2  -  IrObex Connect with Authentication\r\n"));
		iConsole->Printf(_L(" 3  -  IrObex Connect with Target Header\r\n"));
		iConsole->Printf(_L(" 4  -  IrObex Connect with Authentication AND Target Header\r\n"));
		iConsole->Printf(_L(" 5  -  Get remote address\r\n"));
		iConsole->Printf(_L(" s  -  Back to Client Menu\r\n"));
		}
	if ( iTestMode == E_Client_Setup_Menu )
		{
		iConsole->Printf(_L(" 1  -  Change Put Object 1\r\n"));
		iConsole->Printf(_L(" 2  -  Change Put Object 2\r\n"));
		iConsole->Printf(_L(" 3  -  Change Obex Get Name : %S\r\n"), &(iObexClientHandler->iObexName));
		iConsole->Printf(_L(" 4  -  Change Obex Get Type : %S\r\n"), &(iObexClientHandler->iGetType));
		iConsole->Printf(_L(" 5  -  Enable password support (so they can challenge us)\r\n"));
		iConsole->Printf(_L(" 6  -  Set challenge password : %S\r\n"), &(iObexClientHandler->iChallengePassword));
		iConsole->Printf(_L(" 7  -  Change Authentication Challenge Header\r\n"));
		iConsole->Printf(_L(" 8  -  Set Command Time-out\r\n"));
		iConsole->Printf(_L(" s  -  Back to Client Menu\r\n"));
		iConsole->Printf(_L(" p  -  Set default file path : %S\r\n"), &iInboxRoot);
		}
	if ( iTestMode == E_Client_Authentication_Header_Menu )
		{
		iConsole->Printf(_L(" 1  -  Suppress Options Header\r\n"));
		iConsole->Printf(_L(" 2  -  Suppress Realm Header\r\n"));
		iConsole->Printf(_L(" 3  -  Suppress Options and Realm Headers\r\n"));
		iConsole->Printf(_L(" 4  -  Reset to Default Authentication Headers\r\n"));
		iConsole->Printf(_L(" 5  -  Attempt Invalid Authentication Headers Value (i)\r\n"));
		iConsole->Printf(_L(" s  -  Back to Client Menu\r\n"));
		}
	if (iTestMode == E_Server)
		{
		iConsole->Printf(_L(" 1  -  Make OBEX Challenge on all incoming connections\r\n"));
		iConsole->Printf(_L(" 2  -  Disable authentication\r\n"));
		iConsole->Printf(_L(" 3  -  Enable password support (so they can challenge us)\r\n"));
		iConsole->Printf(_L(" 4  -  Set FTP local who (insist on target header\r\n"));
		iConsole->Printf(_L(" 5  -  Change challenge password : %S\r\n"), &(iObexServerHandler->iChallengePassword));
		iConsole->Printf(_L(" 6  -  Get remote address\r\n"));
        iConsole->Printf(_L(" 7  -  Set Target Checking to NoChecking\r\n"));
        iConsole->Printf(_L(" 8  -  Set Target Checking to IfPresent\r\n"));
        iConsole->Printf(_L(" 9  -  Set Target Checking to Always\r\n"));
		iConsole->Printf(_L(" h  -  Toggle display of object headers\r\n"));
		iConsole->Printf(_L(" p  -  Set Inbox root directory : %S\r\n"), &iInboxRoot);
#ifdef PACKET_ACCESS_EXTENSION
		iConsole->Printf(_L(" a  -  Enable displaying of request packets\r\n"));
#endif // PACKET_ACCESS_EXTENSION
		if(iTransport == EIrda)
			{
			iConsole->Printf(_L(" o  -  start suppressing IrDA discovery requests\r\n"));
			iConsole->Printf(_L(" c  -  stop suppressing IrDA discovery requests\r\n"));
			}

		iConsole->Printf(_L("press 's' to stop the Obex Server\r\n"));
		iConsole->Printf(_L("press 'S' to start the Obex Server\r\n"));
		}
	if (iTestMode == E_Server_File)
		{
		iConsole->Printf(_L(" Started Obex server, receiving into file-backed buffer object\r\n"));
		iConsole->Printf(_L(" 1  -  Make OBEX Challenge on all incoming connections\r\n"));
		iConsole->Printf(_L(" 2  -  Disable authentication\r\n"));
		iConsole->Printf(_L(" 3  -  Enable password support (so they can challenge us)\r\n"));
		iConsole->Printf(_L(" 4  -  Set FTP local who (insist on target header\r\n"));
		iConsole->Printf(_L(" 5  -  Change challenge password : %S\r\n"), &(iObexServerHandlerFile->iChallengePassword));
		iConsole->Printf(_L(" 6  -  Get remote address\r\n"));
		iConsole->Printf(_L(" 7  -  Toggle buffered file writes\r\n"));
		iConsole->Printf(_L(" p  -  Set Inbox root directory : %S\r\n"), &iInboxRoot);
		iConsole->Printf(_L("s - Stop the Obex Server\r\n"));
		iConsole->Printf(_L("g - Start the Obex Server\r\n"));
		iConsole->Printf(_L("press 'k' to kill the Obex Server\r\n"));
		}

    if(iTestMode == E_Inactive)
		{
		iConsole->Printf(_L("press '1' to start the Obex Client over Bluetooth\r\n"));
		iConsole->Printf(_L("press '2' to start the Obex Server over Bluetooth\r\n"));
		iConsole->Printf(_L("press '3' to start the Obex Client over Irda\r\n"));
		iConsole->Printf(_L("press 'a' to set the number of discovery slots to use in Irda\r\n"));
 		iConsole->Printf(_L("press 'b' to set the number of discovery attempts to make in Irda\r\n"));
		iConsole->Printf(_L("press '4' to start the Obex Server over Irda\r\n"));
		iConsole->Printf(_L("press '5' to start the Obex Server over Irda (filename backed buffer object)\r\n"));
		iConsole->Printf(_L("press 'r' to start the Obex Server over Irda (RFile backed buffer object)\r\n"));
		iConsole->Printf(_L("press '6' to start the Obex Server over USB\r\n"));
		iConsole->Printf(_L("press '7' to start the Obex Client over USB\r\n"));
		iConsole->Printf(_L("press 'n' to set the IrDA Nickname\r\n"));
		iConsole->Printf(_L("press 'm' to set the Obex Recv MTU\r\n"));
		iConsole->Printf(_L("press 'M' to set the Obex Transmit MTU\r\n"));
		iConsole->Printf(_L("press 'p' to toggle performance logging\r\n"));
		}
#ifdef EMPTY_HEADERS_TEST
	if ( iTestMode == E_Empty_Headers_Accept_Test )
		{
		iConsole->Printf(_L(" 1  -  Sent a packet with NOT empty following headers:\n\t[ Name, Type, Description, Target, Application Parameters ]\r\n\n"));
		iConsole->Printf(_L(" 2  -  Sent a packet with existing but empty following headers:\n\t[ Name, Type, Description, Target, Application Parameters ]\r\n\n"));
		iConsole->Printf(_L(" s  -  Back to Client Menu\r\n"));
		}
#endif //EMPTY_HEADERS_TEST
	if(iTestMode == E_GetIrDANickname && iIrDANicknameBuffer.Length() == 0)
		{
		iConsole->Printf(_L("Enter new IrDA nickname: "));
		}
	if(iTestMode == E_EnterRecvMTU && iRecvMTU == 0)
		{
		iConsole->Printf(_L("Enter new Receive MTU value (m=max, d=default): "));
		}
	if(iTestMode == E_EnterTransmitMTU && iTransmitMTU == 0)
		{
		iConsole->Printf(_L("Enter new Transmit MTU value (m=max, d=default): "));
		}
	//if(E_SdpQuery) // no extra options

    if(iTestMode != E_GetIrDANickname &&
       iTestMode != E_EnterRecvMTU &&
       iTestMode != E_EnterTransmitMTU)
    	{
    	iConsole->Printf(_L("press Escape to quit\r\n\r\n"));
    	}
    iConsole->Read(iStatus); 
    SetActive();
    }

void CActiveConsole::ProcessKeyPressL(TChar aChar)
    {
    if (aChar == EKeyEscape)
		{
		CActiveScheduler::Stop();
		return;
		}

    if(iTestMode ==E_Inactive)
		{
		switch (aChar)
			{
		case '1': 
			iTestMode = E_SdpQuery;
			iTransport = EBluetooth;
			iObexClientHandler = CObexClientHandler::NewL(this, EBluetooth);
			break;
		case'3':
			iTestMode = E_Client;
			iTransport = EIrda;
			iObexClientHandler = CObexClientHandler::NewL(this, EIrda);
			break;
		case 'a':
 			SetNumber(iDiscoverySlots);
 			break;
 		case 'b':
 			SetNumber(iDiscoveryAttempts);
 			break;	
		case 'n':
			iTestMode = E_GetIrDANickname;
			iIrDANicknameBuffer.Zero();
			break;
		case 'p':
			// Toggle the internal state.
			iPerformanceLoggingEnabled = !iPerformanceLoggingEnabled;
			if(iPerformanceLoggingEnabled)
				{
				iConsole->Printf(_L("\r\nPerformance logging enabled.\r\n\r\n"));
				}
			else
				{
				iConsole->Printf(_L("\r\nPerformance logging disabled.\r\n\r\n"));
				}
			break;
		case '2':
			iTestMode = E_Server;
			iTransport = EBluetooth;
			iObexServerHandler = CObexServerHandler::NewL(this, EBluetooth);
			break;
		case '4':
			iTestMode = E_Server;
			iTransport = EIrda;
			iObexServerHandler = CObexServerHandler::NewL(this, EIrda);
			break;
		case '5':
			iTestMode = E_Server_File;	// Added test for file-backed buffer object receive
			iTransport = EIrda;
			iObexServerHandlerFile = CObexServerHandlerFile::NewL(this,EIrda);
			break;
		case 'r':
			iTestMode = E_Server_File;	// Added test for file-backed buffer object receive
			iTransport = EIrda;
			iObexServerHandlerFile = CObexServerHandlerFile::NewL(this,EIrda,ETrue);
			break;
		case '6':
			{
			iTestMode = E_Server;
			
#ifndef __WINS__
			iTransport = EUsb;

			// For USB, we unfortunately need to start it, then register our
			// interfaces, then do a soft connect.
			TInt err = User::LoadLogicalDevice(_L("EUSBC"));
			if (err != KErrNone && err != KErrAlreadyExists)
				{
				User::Leave(err);
				}
			
			RUsb usb;
			User::LeaveIfError(usb.Connect());
			TRequestStatus status;
			usb.TryStart(2, status);
			User::WaitForRequest(status);
			iConsole->Printf(_L("Starting USB returned %d\n"), status.Int());
            User::LeaveIfError(status.Int());
			usb.Close();
			
			RDevUsbcClient usbDriver;
			User::LeaveIfError(usbDriver.Open(0));
			User::LeaveIfError(usbDriver.DeviceDisconnectFromHost());
			
			iObexServerHandler = CObexServerHandler::NewL(this, EUsb);

			User::LeaveIfError(usbDriver.DeviceConnectToHost());
			usbDriver.Close();

#else

			iTransport = EWin32Usb;
			iObexServerHandler = CObexServerHandler::NewL(this, EWin32Usb);
#endif

			}
			break;
		case '7':
			{
			iTestMode = E_Client;
			
#ifdef __WINS__
			iTransport = EWin32Usb;
			iObexClientHandler = CObexClientHandler::NewL(this, EWin32Usb);
			
#else

			iTransport = EUsb;
			
			// For USB, we unfortunately need to start it, then register our
			// interfaces, then do a soft connect.
			TInt loaded = User::LoadLogicalDevice(_L("EUSBC"));
			iConsole->Printf(_L("Loading ldd returned %d\n"), loaded);
			
			RUsb usb;
			User::LeaveIfError(usb.Connect());
			iConsole->Printf(_L("Connected to USB\n"));
			TRequestStatus status;
			usb.TryStart(2,status);
			User::WaitForRequest(status);
			iConsole->Printf(_L("Starting USB returned %d\n"), status.Int());
		    User::LeaveIfError(status.Int());
			usb.Close();
			
			RDevUsbcClient usbDriver;
			User::LeaveIfError(usbDriver.Open(0));
			User::LeaveIfError(usbDriver.DeviceDisconnectFromHost());
			
			iObexClientHandler = CObexClientHandler::NewL(this, EUsb);

			User::LeaveIfError(usbDriver.DeviceConnectToHost());
			usbDriver.Close();
#endif

			}
			break;
		case 'm':
			// Reset MTU value.
			iRecvMTU = 0;
			iTestMode = E_EnterRecvMTU;
			break;

		case 'M':
			// Reset MTU value.
			iTransmitMTU = 0;
			iTestMode = E_EnterTransmitMTU;
			break;
			
		default:
			break;
			}
		}

    else if(iTestMode == E_Server)
		{
		switch (aChar)
			{
		case 's': 
			iObexServerHandler->Stop();
			break;
		case 'S':
			iObexServerHandler->Start();
			break;
		case 'g': case 'G':
			iObexServerHandler->Start();
			break;
		case 'h': case 'H':
			iDisplayHeaders = !iDisplayHeaders;
			break;
		case 'k': case 'K':
			delete iObexServerHandler;
			iObexServerHandler = NULL;
			iTestMode = E_Inactive;
			break;
		case 'o':
			if(iTransport == EIrda)
				{
				iSupp.Open();
				}
			break;
		case 'c':
			if(iTransport == EIrda)
				{
				iSupp.Close();
				}
			break;
		case '1':
			iObexServerHandler->EnableAuthentication();
			break;
		case '2':
			iObexServerHandler->DisableAuthentication();
			break;
		case '3':
			iObexServerHandler->EnablePassword();
			break;
		case '4':
			iObexServerHandler->SetLocalWho();
			break;
		case '5':
			iObexServerHandler->ChangeChallengePassword();
			break;
		case '6':
			iObexServerHandler->RemoteAddress();
			break;
        case '7':
			iObexServerHandler->SetTargetChecking(CObexServer::ENoChecking);
			break;
		case '8':
			iObexServerHandler->SetTargetChecking(CObexServer::EIfPresent);
			break;
		case '9':
            iObexServerHandler->SetTargetChecking(CObexServer::EAlways);
			break;
		case 'p':
			SetInboxRoot();
			break;
#ifdef PACKET_ACCESS_EXTENSION
		case 'a':
			iObexServerHandler->PacketAccessUiL(ETrue);
			break;
#endif // PACKET_ACCESS_EXTENSION
		default:
			iConsole->Printf(_L("\r\nUnknown Command\r\n\r\n"));
			break;
			}
		}

	else if(iTestMode == E_Server_File)
		{
		switch (aChar)
			{
		case 's': case 'S':
			iObexServerHandlerFile->Stop();
			delete iObexServerHandlerFile;
			iObexServerHandlerFile = NULL;
			iTestMode = E_Inactive;
			break;
		case '1':
			iObexServerHandlerFile->EnableAuthentication();
			break;
		case '2':
			iObexServerHandlerFile->DisableAuthentication();
			break;
		case '3':
			iObexServerHandlerFile->EnablePassword();
			break;
		case '4':
			iObexServerHandlerFile->SetLocalWho();
			break;
		case '5':
			iObexServerHandlerFile->ChangeChallengePassword();
			break;
		case '6':
			iObexServerHandlerFile->RemoteAddress();
			break;
		case '7':
			iObexServerHandlerFile->ToggleWriteBuffering();
			break;
		case 'p':
			SetInboxRoot();
			break;
		
		default:
			iConsole->Printf(_L("\r\nUnknown Command\r\n\r\n"));
			break;
			}
		}
	
	else if (iTestMode == E_Client_Connect_Menu )
		{
		switch (aChar)
			{
		case '0':
			iObexClientHandler->Disconnect();
			break;
		case '1':
			iObexClientHandler->Connect();
			break;
		case '2':
			iObexClientHandler->ConnectWithAuthenticationL();
			break;
		case '3':
			iObexClientHandler->ConnectWithTarget();
			break;
		case '4':
			iObexClientHandler->ConnectWithAuthenticationAndTargetL();
			break;
		case '5':
			iObexClientHandler->RemoteAddress();
			break;
		default:
			iTestMode = E_Client;
			break;
			}
		}
	else if ( iTestMode == E_Client_HTTP_Menu )
		{
		switch (aChar)	//tam
			{
		case '1':	//HTTP Multiple Packets;
			iObexClientHandler->HTTPTest1L();
			break;
		case '2':	//HTTP Multiple Packets, too many, will arrive in two parts
			iObexClientHandler->HTTPTest2L();
			break;
		case '3':	//HTTP Single Packet too large, nothing will arrive
			iObexClientHandler->HTTPTest3L();
			break;
		case '4':	//HTTP Single Packet too large
					//with several smaller packets which will arrive
			iObexClientHandler->HTTPTest4L();
			break;
		case '5':	//App parameters test
			iObexClientHandler->AppParamsTestL();
		default:
			iTestMode = E_Client;
			break;
			}

		}
	else if (iTestMode == E_Client_Setup_Menu )
		{
		switch (aChar)
			{
		case '1': 
			iObexClientHandler->SetName(iObexClientHandler->iFilename1);
			break;
		case '2':
			iObexClientHandler->SetName(iObexClientHandler->iFilename2);
			break;
		case '3':
			iObexClientHandler->SetObexName();
			break;
		case '4': 
			iObexClientHandler->SetType();
			break;
		case '5':
			iObexClientHandler->EnablePassword();
			break;
		case '6':
			iObexClientHandler->ChangeChallengePassword();
			break;
		case '7':
			iTestMode = E_Client_Authentication_Header_Menu;
			break;
		case '8':
			iObexClientHandler->SetCommandTimeout();
			break;
		case 'p':
			SetInboxRoot();
			break;
		default:
			iTestMode = E_Client;
			break;
			}
		}
	else if(iTestMode == E_Client)
		{
		switch (aChar)
			{
		case 'a': case 'A':
			iObexClientHandler->Abort();
			break;
		case '1': 
			iTestMode = E_Client_Connect_Menu;
			break;
		case '2':
			iObexClientHandler->Disconnect();
			break;
		case '3': 
			iObexClientHandler->Put(iObexClientHandler->iFilename1);
			break;
		case '4': 
			iObexClientHandler->Put(iObexClientHandler->iFilename2);
			break;
		case 'y':case 'Y':
			iTestMode = E_Client_HTTP_Menu;
			break;
		case 'z':case 'Z':
			iObexClientHandler->Put(iObexClientHandler->iFilename3);
			break;
		case '5':
			iObexClientHandler->GetByNameL();
			break;
		case '6':
			iObexClientHandler->GetByTypeL();
			break;
		case '7':
			iObexClientHandler->SetPath();
			break;
		case '8':
			iObexClientHandler->SetCurrentTestNumber(); 
			iObexClientHandler->PutReferenceL(10);
			break;
		case '9':
			iObexClientHandler->SetCurrentTestNumber(); 
			iObexClientHandler->GetReferenceL(8);
			break;
		case '0': 
			iTestMode = E_Client_Setup_Menu;
			break;
		case 'h': case 'H':
			iDisplayHeaders = !iDisplayHeaders;
			break;
#ifdef ERROR_RESOLUTION_EXTENSION
		case 'l': case 'L':
			iObexClientHandler->LastError();
			break;
#endif // ERROR_RESOLUTION_EXTENSION

#ifdef EMPTY_HEADERS_TEST
		case 't': 
			iTestMode = E_Empty_Headers_Accept_Test;
			break;
#endif //EMPTY_HEADERS_TEST

		case 's': case 'S':
			delete iObexClientHandler;
			iObexClientHandler = NULL;
			iTestMode = E_Inactive;
			break;
		default:
			iConsole->Printf(_L("\r\nUnknown Command\r\n\r\n"));
			break;
			}
		}
	else if(iTestMode == E_Client_Authentication_Header_Menu)
		{
		switch (aChar)
			{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			iObexClientHandler->ChangeAuthenticationChallengeHeaders(aChar);
			break;
		case 's': case 'S':
			iTestMode = E_Client_Setup_Menu;
			break;
		default:
			iConsole->Printf(_L("\r\nUnknown Command\r\n\r\n"));
			break;
			}
		}
	else if(iTestMode == E_GetIrDANickname)
		{
		if(aChar.IsControl() ||
		   iIrDANicknameBuffer.Length() == iIrDANicknameBuffer.MaxSize())
			{
			// Return to main menu.
			iTestMode = E_Inactive;
			iConsole->Printf(_L("\nNickname set to: %S\n\n"), &iIrDANicknameBuffer);
			}
		else
			{
			iConsole->Printf(_L("%c"), aChar.operator TUint());
			iIrDANicknameBuffer.Append(aChar);
			}
		}
	else if(iTestMode == E_EnterRecvMTU)
		{
		if(aChar.IsDigit())
			{
			iRecvMTU *= 10;
			iRecvMTU += aChar.GetNumericValue();
			iConsole->Printf(_L("%c"), aChar.operator TUint());			
			if(iRecvMTU >= KObexPacketMaxSize)
				{
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			}
		else
			{
			if(aChar == 'm')
				{
				// Set max MTU
				iRecvMTU = KObexPacketMaxSize;
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			else if(aChar == 'd')
				{
				// Set defualt MTU
				iRecvMTU = 4000;
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			else if(aChar.IsControl())
				{
				// Return to main menu.
				iTestMode = E_Inactive;
				}

			iConsole->Printf(_L("\nReceive MTU set to: %d\n\n"), iRecvMTU);
			}
		}
	else if(iTestMode == E_EnterTransmitMTU)
		{
		if(aChar.IsDigit())
			{
			iTransmitMTU *= 10;
			iTransmitMTU += aChar.GetNumericValue();
			iConsole->Printf(_L("%c"), aChar.operator TUint());			
			if(iTransmitMTU >= KObexPacketMaxSize)
				{
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			}
		else
			{
			if(aChar == 'm')
				{
				// Set max MTU
				iTransmitMTU = KObexPacketMaxSize;
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			else if(aChar == 'd')
				{
				// Set defualt MTU
				iTransmitMTU = 4000;
				// Return to main menu.
				iTestMode = E_Inactive;
				}
			else if(aChar.IsControl())
				{
				// Return to main menu.
				iTestMode = E_Inactive;
				}

			iConsole->Printf(_L("\nTransmit MTU set to: %d\n\n"), iTransmitMTU);
			}
		}
#ifdef EMPTY_HEADERS_TEST
	else if ( iTestMode == E_Empty_Headers_Accept_Test )
		{
		switch (aChar)
			{
				case '1' : 
					iObexClientHandler->EmptyHeaderTestL(_L("Name header"), _L8("Type header"), _L("Description header"), _L8("Target header"), _L8("Application param header"));
				break;

				case '2': 
					iObexClientHandler->EmptyHeaderTestL(_L(""), _L8(""), _L(""), _L8(""), _L8(""));
				break;
				
				case 's': 
				 iTestMode = E_Client;
				break;
				
				default :
					iConsole->Printf(_L("\r\nUnknown Command\r\n\r\n"));
			}
		
		}
#endif	//EMPTY_HEADERS_TEST	

	else
		{
		CActiveScheduler::Stop();
		return;
		}
	RequestCharacter ();
	return;
    }

CConsoleBase* CActiveConsole::Console()
	{
	return iConsole;
	}

void CActiveConsole::SetPassword(TDes& aPassword)
    {
	iConsole->Printf(_L("\nEnter a password: %S"),&aPassword);

	GetString(aPassword);

	iConsole->Printf(_L("\n"));
	iConsole->Printf( _L("\n Password size = %d\n"),aPassword.Length());
    }


void CActiveConsole::SetInboxRoot()
	{
	iConsole->Printf(_L("\nEnter a path: %S"), &iInboxRoot);

	GetString(iInboxRoot);

	iConsole->Printf(_L("\n"));
	}


TBool CActiveConsole::DisplayHeaders()
	{
	return iDisplayHeaders;
	}
	
TBool CActiveConsole::SetNumber(TUint& aNumber)
 	{
 	TKeyCode code;
 	TBuf<1> ch;
 	TBuf<64> input;
 	TUint oldNumber = aNumber;
 
 	input.Num(aNumber);
 	iConsole->Printf(_L("\nEnter a number: %S"), &input);
 
 	FOREVER
 		{
 		code = iConsole->Getch();
 		ch.SetLength(0);
 		ch.Append(code);
 
 		iConsole->Printf(_L("%S"), &ch);
 
 		// If <CR> finish editing string.
 		if (code == EKeyEnter)
 			{
 			break;
 			}
 
 		// If <BS> remove last character.
 		if ((code == EKeyBackspace) && (input.Length() != 0))
 			{
 			input.SetLength((input.Length() - 1));
 			}
 		else
 			{
 			input.Append(code);
 			}
 		}
 	iConsole->Printf(_L("\n"));
 
 	TLex lex(input);
 	if (lex.Val(aNumber) != KErrNone)
 		{
 		iConsole->Printf(_L("\r\nError: You did not enter a number.\r\n"));
 		aNumber = oldNumber;
 		return EFalse;
 		}
 		
 	return ETrue;
	}
 
void CActiveConsole::GetString(TDes& aString)
	{
	TKeyCode code;
	TBuf<1> character;
	
	FOREVER
		{
		code = iConsole->Getch();
		character.SetLength(0);
		character.Append(code);

		iConsole->Printf(_L("%S"),&character);
	
		// If <CR> finish editing string
		if (code == EKeyEnter)
			break;
		
		// if <BS> remove last character
		if ((code == EKeyBackspace) && (aString.Length() != 0))
			{
			aString.SetLength((aString.Length()-1));
			}
		else if (aString.Length() < aString.MaxLength())
			{
			aString.Append(code);
			}
		}
	}

void CActiveConsole::ClientErrored()
	{
	iTestMode = E_Inactive;
	Cancel();
	RequestCharacter();
	delete iObexClientHandler;
	iObexClientHandler = 0;
	}

TBool CActiveConsole::UseIrdaNickname() const
	{
	return (iIrDANicknameBuffer.Length() != 0);
	}
	
const TDesC& CActiveConsole::GetIrdaNickname() const
	{
	return iIrDANicknameBuffer;
	}

