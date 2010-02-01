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
#include <e32keys.h>
#include "clienthandler.h"
#include "TOBEXCON.H"
#include "btextnotifiers.h"
#include "obexconstantsinternal.h"

#define EPOCIDENT _L8("EPOC32 ER5")



CObexClientHandler::CObexClientHandler(CActiveConsole* aParent)
    : CActive(EPriorityNormal), iParent(aParent), iState(EIdle)
    {
    }

CObexClientHandler* CObexClientHandler::NewL(CActiveConsole* aParent, TTransport aTransport)
    {
    CObexClientHandler* self = new (ELeave) CObexClientHandler(aParent);

    CleanupStack::PushL (self);
    self->ConstructL(aTransport);
    CActiveScheduler::Add (self);
    CleanupStack::Pop ();
    return (self);
    }

void CObexClientHandler::ConstructL(TTransport aTransport)
    {
	if (aTransport == EBluetooth)
		{
		// Check if an address has been provided on the cmd line.
		TBuf<20> cmdline;
		User::CommandLine(cmdline);

		TInt ret = iDevAddr.SetReadable(cmdline);
		if (ret < KErrNone)
			{
			iParent->Console()->Printf(_L("\nNo address found on command line\n"));

			//Ask user which device address we should connect to...
			RNotifier notify;
			User::LeaveIfError(notify.Connect());
			TBTDeviceSelectionParamsPckg pckg;
			TBTDeviceResponseParamsPckg resPckg;
			TRequestStatus stat;
			notify.StartNotifierAndGetResponse(stat, KDeviceSelectionNotifierUid, pckg, resPckg);
			User::WaitForRequest(stat);
			notify.CancelNotifier(KDeviceSelectionNotifierUid);
			notify.Close();
			User::LeaveIfError(stat.Int()); 

			iDevAddr = resPckg().BDAddr();
			}
			
		// Find UUID to search for
		iParent->Console()->Printf(_L("\nPress F: FTP, Any other key: OPP"));
		TChar code (iParent->Console()->Getch());
		TUUID searchUUID;
		switch(code)
			{
		case 'f': case 'F':
			searchUUID = TUUID(0x1106); //FTP
			break;
		case 'o': case 'O':
		default:
			searchUUID = TUUID(0x1105); //OPP
			}

		//start the SDP Query
		delete iSdpServiceFinder;
		iSdpServiceFinder=0;
		iSdpServiceFinder = CRFCOMMServiceFinder::NewL(searchUUID, iDevAddr, *this);
		iSdpServiceFinder->FindPortL();
		iParent->Console()->Printf(_L("\nSearching for SDP service....\n"));

		// For bluetooth the client is created in SearchResult().
		}
	else if (aTransport == EIrda)
		{
		TObexIrProtocolInfo aInfo;
		aInfo.iTransport= KObexIrTTPProtocolV2;
		aInfo.iClassName     = _L8("OBEX");					//same for unicode and narrow builds
		aInfo.iAttributeName = _L8("IrDA:TinyTP:LsapSel");
		aInfo.iDiscoverySlots = iParent->iDiscoverySlots;
 		aInfo.iDiscoveryAttempts = iParent->iDiscoveryAttempts;	
		//now create the obex client...
	    iClient = CObexClient::NewL (aInfo);
	    iClient->SetFinalPacketObserver(this);
#ifdef ERROR_RESOLUTION_EXTENSION
		iClientErrorResolver = CObexClientErrorResolver::NewL(*iClient);
#endif // ERROR_RESOLUTION_EXTENSION
		}
	else if (aTransport == EUsb)
		{
		TObexUsbProtocolInfo aInfo;
		aInfo.iTransport = KObexUsbProtocol;
		aInfo.iInterfaceStringDescriptor = _L("TOBEX Client Interface");
		//now create the obex client...
	    iClient = CObexClient::NewL (aInfo);
	    iClient->SetFinalPacketObserver(this);
#ifdef ERROR_RESOLUTION_EXTENSION
		iClientErrorResolver = CObexClientErrorResolver::NewL(*iClient);
#endif // ERROR_RESOLUTION_EXTENSION
		}
	else if (aTransport == EWin32Usb)
		{
		TObexUsbProtocolInfo aInfo;
		aInfo.iTransport = _L("Win32Usb");
		//now create the obex client...
	    iClient = CObexClient::NewL (aInfo);
	    iClient->SetFinalPacketObserver(this);
#ifdef ERROR_RESOLUTION_EXTENSION
		iClientErrorResolver = CObexClientErrorResolver::NewL(*iClient);
#endif // ERROR_RESOLUTION_EXTENSION
		}
	else
		{
		User::Invariant();
		}

#ifdef EMPTY_HEADERS_TEST
	iObexEmptyHeaderTest = CObexEmptyHeaderTest::NewL();
#endif //EMPTY_HEADERS_TEST
    iFileObject = CObexFileObject::NewL();
    iObjectBuffer = CBufFlat::NewL(8);
    iObject = CObexBufObject::NewL(iObjectBuffer);
	iObexName = _L("");
	iGetType = _L("text/x-vcard");

	iFilename1 = _L("Contacts.vcf"); 
	iFilename2 = _L("Contacts2.vcf");
	iFilename3 = _L("Contacts3.vcf");

	iChallengePassword = _L("SecretPassword");
	iResponsePassword = _L("SecretPassword");

	iTargetHeaderObject = CObexNullObject::NewL();
	iTargetHeaderObject->SetTargetL(KRefTarget);
	iTargetHeaderObject->SetHeaderMask(KObexHdrTarget);
    }


void CObexClientHandler::SearchResult(TInt aError, TUint8 aPort, TInt aProfileVersion)
	{
	if (aError != KErrNone)
		{
		iParent->Console()->Printf(_L("\r\n Could not find SDP service in remote device : error %d \r\n"),aError);	
		iParent->ClientErrored();
		return;
		}

	if (aProfileVersion >= 0)	
		{	
		iParent->Console()->Printf(_L("\nProfile Version 0x%04x\n"), aProfileVersion);	
		}	
	else	
		{	
		iParent->Console()->Printf(_L("\nProfile Version not supplied\n"));	
		}	
	iParent->Console()->Printf(_L("RFCOMM Port %d\n"), aPort);	

	TObexBluetoothProtocolInfo aInfo;
	aInfo.iAddr.SetBTAddr(iDevAddr);
	aInfo.iAddr.SetPort(aPort); 
	aInfo.iTransport     = KObexRfcommProtocol;

	TObexProtocolPolicy policy;
	policy.SetReceiveMtu(iParent->iRecvMTU);
	policy.SetTransmitMtu(iParent->iTransmitMTU);

	//now create the obex client...
	TRAP(aError, iClient = CObexClient::NewL(aInfo, policy));
	if (aError)
		{
		iParent->Console()->Printf(_L("\r\n Could not create client! : error %d \r\n"),aError);
		iParent->ClientErrored();
		return;
		}
	iClient->SetFinalPacketObserver(this);
#ifdef ERROR_RESOLUTION_EXTENSION
	TRAP(aError,iClientErrorResolver = CObexClientErrorResolver::NewL(*iClient));
	if (aError != KErrNone)
		{
		iParent->Console()->Printf(_L("\r\n Could not create client error resolver! : error %d \r\n"),aError);
		iParent->ClientErrored();
		return;
		}
#endif // ERROR_RESOLUTION_EXTENSION
	iParent->Console()->Printf(_L("\nSDP search complete OK!\n"));
	iParent->iTestMode = E_Client;

	iParent->Cancel(); // cancel request for key
	iParent->RequestCharacter(); // re-request, to re-display menu
	}

void CObexClientHandler::SetCurrentTestNumber()
{
	iCurrentTestNumber = 1;
}

void CObexClientHandler::ResetCurrentTestNumber()
{
	iCurrentTestNumber = 0;
}

void CObexClientHandler::MofpoFinalPacketStarted()
	{
	iParent->Console()->Printf(_L("\nFinal packet started\n"));
	}

void CObexClientHandler::MofpoFinalPacketFinished()
	{
	iParent->Console()->Printf(_L("\nFinal packet finished\n"));
	}

CObexClientHandler::~CObexClientHandler()
    {
    Cancel();
#ifdef EMPTY_HEADERS_TEST    
    delete iObexEmptyHeaderTest;
#endif //EMPTY_HEADERS_TEST
    delete iObject;
	delete iFileObject;
    delete iClient;
#ifdef ERROR_RESOLUTION_EXTENSION
	delete iClientErrorResolver;
#endif // ERROR_RESOLUTION_EXTENSION

	delete iObjectBuffer;
	delete iTargetHeaderObject;
	delete iSdpServiceFinder;
    }

void CObexClientHandler::Abort()
    {
    if((iState != EPutting)&&(iState != EGetting))
		iParent->Console()->Printf(_L("\r\n NOTE: Wrong Obex state for issuing abort\r\n"));
	iClient->Abort();
    }



void CObexClientHandler::Connect()
    {
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
   
    TObexConnectInfo iLocalInfo = iClient->LocalInfo();
    iLocalInfo.iWho = _L8("");
    iLocalInfo.iWho = EPOCIDENT;  
    iLocalInfo.iWho.Append(_L8(" EikIrOBEXFile "));

    iClient->Connect(iStatus);
    SetActive();
    iState = EConnecting;
    }

void CObexClientHandler::RemoteAddress()
	{
	TSockAddr aAddr;
	iClient->RemoteAddr( aAddr);
	TBTDevAddr addr = static_cast <TBTSockAddr>(aAddr).BTAddr();
	
	TChar aChar;

	iParent->Console()->Printf(_L("Remote Address = "));
	for(TInt count = 0; count < addr.Des().Length(); count++) 
		{
		aChar = (addr.Des()[count] & 0xf0)>>4;
		if (aChar <= 9)
			aChar += 0x30;
		else if ((aChar >= 0x0a) && (aChar <= 0x0f))
			aChar += 'A' - 0x0a;
		iParent->Console()->Printf(_L("%C"),(TUint)aChar);

		aChar = (addr.Des()[count] & 0x0f);
		if (aChar <= 9)
			aChar += 0x30;
		else if ((aChar >= 0x0a) && (aChar <= 0x0f))
			aChar += 'A' - 0x0a;
		iParent->Console()->Printf(_L("%C"),(TUint)aChar);
		}
    iParent->Console()->Printf(_L("\r\n"));

	}

void CObexClientHandler::Disconnect()
    {
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
    
	iClient->Disconnect(iStatus);
    SetActive();
    iState = EDisconnecting;
    }

void CObexClientHandler::GetByNameL()
    {
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);
    iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
    }


void CObexClientHandler::GetByTypeL()
    {
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	TBuf8<300> buf;
	buf.Copy(iGetType);
	buf.Append(0);
	iObject->SetTypeL (buf);
    iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;

    }

void CObexClientHandler::Put(TDes& aFilename)
    {
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

	TInt err;

	err = SetUpObject (aFilename);

	if( err != KErrNone)
		{
		iParent->Console()->Printf(_L("\r\n Couldnt set up object : error %d \r\n"),err);
		return;
		}

	// Start the timer.
	iStartTime.HomeTime();
	
	iClient->Put(*iFileObject,iStatus);
	SetActive();
	iState = EPutting;
	}

void CObexClientHandler::GetReferenceL(TInt aReferenceId)
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
	iParent->iRefHandler->CreateReferenceL(*iObject, *iObjectBuffer, aReferenceId, iParent->iTransport);
	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGettingReference;
	}


//multiple HTTP packets sent in a single Obex packet
void CObexClientHandler::HTTPTest1L() //tam
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);

	//now add in the HTTP headers

	iObject->AddHttpL(_L8("Test 1 HTTP header 1\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 2\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 3\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 4\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 5\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 6\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 7\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 8\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 9\r\n"));
	iObject->AddHttpL(_L8("Test 1 HTTP header 10\r\n"));
	
    
	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
	}

//multiple HHTP packets sent in multiple Obex packets
void CObexClientHandler::HTTPTest2L()
	{


	TBuf8<520> localBuf;

    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}


    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);

	for ( TUint x = 0; x < 10; x++ ) //should be 10*512 HTTP packets,
		{							//		which is in excess of the 4K Tranport size
		localBuf = _L8("Test 2 HTTP Header");
		localBuf.AppendFill(TUint(x + '1'), 500);

		iObject->AddHttpL(localBuf);
		}
	
	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
	}

//single HHTP packet too large for an Obex packet will not go
void CObexClientHandler::HTTPTest3L()
	{
	HBufC8* localBuf = HBufC8::NewL(5020);


	TPtr8 ptr = localBuf->Des();

    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);
    

	ptr = _L8("Test 3 HTTP Header ");
	ptr.AppendFill(TUint('A'), 5000);

	iObject->AddHttpL(ptr);

	
	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
	}

//a single (too) large HTTP packet, with multiple HTTP packets
//large one ignored, multiple smaller should all go
void CObexClientHandler::HTTPTest4L()
	{
	HBufC8* localBuf = HBufC8::NewL(5020);

	TPtr8 ptr = localBuf->Des();
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);


	ptr = _L8("Test 4 HTTP header 1");
	iObject->AddHttpL(ptr);

    //now for the excessively large header
	ptr = _L8("Test 4 HTTP header 2");
	ptr.AppendFill(TUint('B'), 5000);
	iObject->AddHttpL(ptr);

	//and a reasonable header again

	ptr = _L8("Test 4 HTTP header 3");
	iObject->AddHttpL(ptr);

	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
	}


void CObexClientHandler::AppParamsTestL()
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

    iObject->Reset ();
	SetName(iObexName);
    iObject->SetNameL (iObexName);

	TBuf8<40> param;
	param.Copy(_L("\x04"));
	param.Append(_L("\x03"));
	param.Append(_L("App params 123"));
	iObject->SetAppParamL(param);


	iClient->Get(*iObject, iStatus);
    SetActive();
    iState = EGetting;
	
	
	}

void CObexClientHandler::ChangeAuthenticationChallengeHeaders(TChar aChar)
	{
	switch(aChar)
		{
	case '1':
		// Suppress 'Options'
		iClient->SuppressAuthenticationHeaderElements(CObex::EObexSuppressChallengeOptionsAuthElement);
		break;
	case '2':
		// Suppress 'Realm'
		iClient->SuppressAuthenticationHeaderElements(CObex::EObexSuppressRealmAuthElement);
		break;
	case '3':
		// Supress both
		iClient->SuppressAuthenticationHeaderElements(CObex::EObexSuppressAllAuthElements);
		break;
	case '4':
		// Reset to default
		iClient->SuppressAuthenticationHeaderElements(CObex::EObexNoSuppressedAuthElements);
		break;
	case '5':
		// Invalid enum (magic number)
		iClient->SuppressAuthenticationHeaderElements(static_cast<CObex::TObexSuppressedAuthElements>(0x08));
		break;
	default:
		break;
		}
	}

void CObexClientHandler::PutReferenceL(TInt aReferenceId)
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

	iParent->iRefHandler->CreateReferenceL(*iObject, *iObjectBuffer, aReferenceId, iParent->iTransport);
	iClient->Put(*iObject,iStatus);
	SetActive();
	iState = EPuttingReference;
	}

TInt CObexClientHandler::SetUpObject(TDes& filename)
    {
//	iFileObject->SetDataFileL(KNullDesC);
	TRAPD (err, iFileObject->InitFromFileL (filename));

	if (err != KErrNone)
		{
		RFs fs;
		RFile f;
		if ((fs.Connect () != KErrNone) || 
			(f.Create (fs, filename, EFileShareExclusive | EFileWrite) != KErrNone))
			iParent->Console()->Printf(_L("\r\nError reading '%s'.\r\nI tried to create this file for you, but failed to do that too. Sorry.\r\n\r\n"), filename.PtrZ ());
		else
			{
			f.Write (_L8("Test file for sending from EPOC\r\n\r\nLooks like obex is sending OK!!\r\n"));
			f.Close ();
			iParent->Console()->Printf(_L("\r\nFile '%s' did not exist, so I've created one.\r\nPlease try again.\r\n\r\n"), filename.PtrZ ());
			}
		fs.Close ();
		}
	
	//iFileObject->SetNameL(iObexName);

	return err;
    }


void CObexClientHandler::RunL ()
    {
    if (iStatus != KErrNone)
		{// Handle error
		}

    switch (iState)
		{
	case EConnecting:
		iParent->Console()->Printf(_L("\r\nConnect completed with error code: %d\r\n\r\n"),iStatus.Int());
		iState = EConnected;//may not be connected actually
		break;
		
	case EPutting:
		{
		TTime finishTime;
		finishTime.HomeTime();
		TTimeIntervalMicroSeconds diff = finishTime.MicroSecondsFrom(iStartTime);	

		iParent->Console()->Printf(_L("\nObject Sent in %d"), diff.Int64());
		
		iState = EConnected;
		iParent->Console()->Printf(_L("\r\nPut completed with error code: %d\r\n\r\n"),iStatus.Int());
		}
		break;
		
	case EGetting:
		iState = EConnected;
		iParent->Console()->Printf(_L("\r\nGet completed with error code: %d\r\n\r\n"),iStatus.Int());
		DisplayObjectL();
		SaveObject();
		iObject->Reset ();
		break;

	case EGettingReference:
		{
		iState = EConnected;
		iParent->Console()->Printf(_L("\r\nGet completed with error code: %d\r\n\r\n"),iStatus.Int());
		if(iStatus == KErrNone)
			{
			DisplayObjectL();
			TInt objComp = iParent->iRefHandler->CompareObjectToReferenceL(*iObject, *iObjectBuffer, iParent->iTransport);
			ASSERT(objComp == 0); // Make sure what we sent (the ref obj) matches what we got
			(void) objComp; // avoid build warning
	//		iParent->Console()->Printf(_L("\r\nReference Object Comparison Result: %d\r\n\r\n"), objComp);
			iParent->Console()->Printf(_L("\r\nReference Object Comparison OK\r\n\r\n"));
			}
		else
			{
			iParent->Console()->Printf(_L("GET REFERENCE OBJECT FAILED\n"));
			}
		iObject->Reset();

		if (iCurrentTestNumber < KMaxNumOfTests)
			GetReferenceL(++iCurrentTestNumber);
		else
			{
			iParent->Console()->Printf(_L("All GET tests completed\n"));
			ResetCurrentTestNumber();
			}
		break;
		}
	case EPuttingReference:
		iState = EConnected;
		iParent->Console()->Printf(_L("\r\nReference Put completed with error code: %d\r\n\r\n"),iStatus.Int());
		if(iStatus != KErrNone)
			{
			iParent->Console()->Printf(_L("PUT REFERENCE OBJECT FAILED\n"));
			}
		if (iCurrentTestNumber < KMaxNumOfTests)
			PutReferenceL(++iCurrentTestNumber);
		else
			{
			iParent->Console()->Printf(_L("All PUT tests completed\n"));
			ResetCurrentTestNumber();
			}

		break;

	case EDisconnecting:
		iParent->Console()->Printf(_L("\r\nDisconnect completed with error code: %d\r\n\r\n"),iStatus.Int());
		iState = EIdle;
		break;
	case ESettingPath:
		iParent->Console()->Printf(_L("\r\nSetPath completed with error code: %d\r\n\r\n"),iStatus.Int());
		iState = EConnected;
		break;
	default:
		iParent->Console()->Printf(_L("\r\nTest Code is in an incorrect state: %d\r\n\r\n"),iState);
		}
    }

void CObexClientHandler::DoCancel()
    {
    delete iClient;
    iClient = NULL;
#ifdef ERROR_RESOLUTION_EXTENSION
    delete iClientErrorResolver;
    iClientErrorResolver = NULL;
#endif // ERROR_RESOLUTION_EXTENSION
    }

void CObexClientHandler::DisplayObjectL()
    {
    // Display Contents of CBufFlat data on current console
	// This size is wrong if we abort, due to size being pre-allocated! DOH!

	iParent->Console()->Printf(_L("Size of received object = %d\n"),iObjectBuffer->Size());

	TInt err = KErrNone;
	if (iParent->DisplayHeaders())
	{
		CObexHeader* header = CObexHeader::NewL();
		CleanupStack::PushL(header);

		iObject->HeaderSet().SetMask(NULL);
		iObject->HeaderSet().First();

		while (err == KErrNone)
			{
			err = iObject->HeaderSet().This(header);

			switch (header->Type())
			{
				case (0x00): //Unicode
					{
					HBufC16* buf = NULL;
					TRAPD(err, buf = HBufC16::NewL((header->AsUnicode()).Size()));
					if (err)
						{
						iParent->iConsole->Printf(_L("Unicode Header (0x%x)- Error allocating memory to display\r\n"), header->HI());
						}
					else
						{
						TPtr16 type(buf->Des());
						type.Copy(header->AsUnicode());

						iParent->iConsole->Printf(_L("Unicode Header (0x%x) =  : \"%S\"\r\n"), header->HI(), &type);
						}
					delete buf;
					break;
					}
				case (0x01): // ByteSeq
					{
					HBufC16* buf = NULL;
					TRAPD(err, buf = HBufC16::NewL((header->AsByteSeq()).Size()));
					if (err)
						{
						iParent->iConsole->Printf(_L("ByteSeq Header (0x%x)- Error allocating memory to display\r\n"), header->HI());
						}
					else
						{
						TPtr16 type(buf->Des());
						type.Copy(header->AsByteSeq());

						iParent->iConsole->Printf(_L("ByteSeq Header (0x%x) =  : \"%S\"\r\n"), header->HI(), &type);
						}
					delete buf;
					break;
					}
				case (0x02): // Byte
					{
					iParent->iConsole->Printf(_L("Byte Header (0x%x) =  : 0x%x\r\n"), header->HI(), header->AsByte());
					break;
					}
				case (0x03): //FourByte
					{
					iParent->iConsole->Printf(_L("FourByte Header (0x%x) =  : 0x%x\r\n"), header->HI(), header->AsFourByte());
					break;
					}
				default : {break;}
			}
			
			err = iObject->HeaderSet().Next();
			}
		
		CleanupStack::Pop(header);
		delete header;
	}		

	TDateTime dt = iObject->Time().DateTime();
	iParent->Console()->Printf(_L("\r\nTimestamp: %d/%d/%d, %d:%d:%d\r\n\r\n"),
				   dt.Day()+1, dt.Month()+1, dt.Year(), dt.Hour(), dt.Minute(), dt.Second());

    TBuf8<1024> tempBuffer;
//	iObjectBuffer->Read(0, tempBuffer, iObjectBuffer->Size());
	iObjectBuffer->Read(0, tempBuffer, tempBuffer.MaxSize() < iObjectBuffer->Size() ? tempBuffer.MaxSize() : iObjectBuffer->Size());
	// Printf fails with Descriptor bigger than X hundred bytes so write byte at a time
	for(TInt count = 0; count < tempBuffer.Size(); count++) 
		{
		iParent->Console()->Printf(_L("%C"),tempBuffer[count]);
		}
    }

void CObexClientHandler::SaveObject()
	{
	
	TFileName name;	
	TParse parser;
	TBool bIsFullPath = EFalse;
		
	_LIT(KDrive, "");
	TBufC<10> drive(KDrive);
	
	TPtr pDrive(drive.Des());
	
	TInt ret = parser.Set(iObject->Name() ,0,0);  
	 
	if(KErrNone == ret)
		{
		pDrive = parser.Drive();

		if(pDrive.Length() != 0) // if == 0, relative path
			{
			bIsFullPath = ETrue;			
			} 		
		}
		// the case ret != KErrNone is kept for legacy purpose, TODO cope with return codes
		
	if(!bIsFullPath)
		{
		name = iParent->iInboxRoot;
		}			

	name.Append(iObject->Name());
	
	TInt err = iObject->WriteToFile(name);
	if (err  == KErrAlreadyExists)
		{
		iParent->Console()->Printf(_L("\r\nWrite failed, File Already Exists\n"));
		}
	} 
	

void CObexClientHandler::SetPath()
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

	CObex::TSetPathInfo info;
	iParent->Console()->Printf(_L("Please enter path name ('..' = parent, '!' = Create, ' ' = empty name):\n"));
	info.iNamePresent = ETrue;
	SetName(info.iName);
	info.iFlags |= 2; // Set "Don't Create" flag as default.
	
	// Parent
	if (info.iName.Length() >= 2 && info.iName[0] == '.' && info.iName[1] == '.')
		{
		info.iName.Delete(0, 2);
		info.iFlags |= 1;
		}
	// Create
	if (info.iName.Length() >= 1 && info.iName[0] == '!')
		{
		info.iName.Delete(0, 1);
		info.iFlags &= ~(2); // Clear the "Don't Create" bit
		}
	// Empty Name
	if (info.iName.Length() >= 1 && info.iName[0] == ' ')
		{
		info.iName.Delete(0, 1);
		}
	else if (info.iName.Length() <= 0)
		info.iNamePresent = EFalse;
    iClient->SetPath(info,iStatus);
    SetActive();
    iState = ESettingPath;
	}

void CObexClientHandler::SetObexName()
	{
	SetName(iObexName);
	}

void CObexClientHandler::SetType()
	{
	iParent->Console()->Printf(_L("\nEnter Mime type, or 1 for text/x-vcard, 2 for x-obex/folder-listing"));
	SetName(iGetType);
	if (iGetType == _L("1"))
		iGetType = _L("text/x-vcard");
	else if (iGetType == _L("2"))
		iGetType = _L("x-obex/folder-listing");
	}


void CObexClientHandler::SetName(TDes& aName)
    {
	TBuf<64> oldName;
	oldName = aName;

	TKeyCode aCode;
	TBuf<1> aChar;
	iParent->Console()->Printf(_L("\nEnter a name: %S"),&aName);
	FOREVER
		{
		aCode = iParent->Console()->Getch();
		aChar.SetLength(0);
		aChar.Append(aCode);

		iParent->Console()->Printf(_L("%S"),&aChar);
	
		// If <CR> finish editing string
		if (aCode == EKeyEnter)
			break;
		
		// if <BS> remove last character
		if ((aCode == EKeyBackspace)&&(aName.Length() != 0))
			aName.SetLength((aName.Length()-1));
		else
			aName.Append(aCode);
		}
	iParent->Console()->Printf(_L("\n"));
//	if (aName.Length()>0)
//		iParent->Console()->Printf( _L("\n name size = %d\n"),aName.Length());
//	else
//		{
//		iParent->Console()->Printf(_L("\nERROR: name of length zero, name unchanged!\n"),aName.Length());
//		aName = oldName;
//		}

    }

void CObexClientHandler::ConnectWithAuthenticationL()
	{
    if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
    
    iClient->ConnectL(iChallengePassword, iStatus);
    SetActive();
    iState = EConnecting;
	}

void CObexClientHandler::ConnectWithTarget()
	{
	if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
	iClient->Connect(*iTargetHeaderObject, iStatus);
	SetActive();
	iState = EConnecting;
	}

void CObexClientHandler::ConnectWithAuthenticationAndTargetL()
	{
	if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
	iClient->ConnectL(*iTargetHeaderObject, iChallengePassword, iStatus);
	SetActive();
	iState = EConnecting;
	}


void CObexClientHandler::EnablePassword()
	{
	iClient->SetCallBack(*this);
	}

void CObexClientHandler::ChangeChallengePassword()
	{
	iParent->SetPassword(iChallengePassword);
	}
	
void CObexClientHandler::SetCommandTimeout()
	{
	TUint timeout = 0;
	if (iParent->SetNumber(timeout))
		{
		const TTimeIntervalMicroSeconds32 temp = timeout;
		iClient->CObexClient::SetCommandTimeOut(temp);
		iParent->Console()->Printf(_L("\r\nCommand Sucessful\r\n"));
		}
	}

void CObexClientHandler::ChangeResponsePassword()
	{
	iParent->Cancel();
	iParent->SetPassword(iResponsePassword);
	iParent->RequestCharacter();
	}

void CObexClientHandler::GetUserPasswordL(const TDesC& aUserID)
	{
	iParent->Console()->Printf(_L("\r\nClient has been challenged by %S"), &aUserID);
	ChangeResponsePassword(); //get the password from user
//	iParent->Console()->Printf(_L("\r\nUser Password requested for %S"), &aUserID);
	iClient->UserPasswordL(iResponsePassword); 
	}

#ifdef ERROR_RESOLUTION_EXTENSION
void CObexClientHandler::LastError()
	{
	if (iClientErrorResolver == NULL)
		{
		iParent->Console()->Printf(_L("Client not started\r\n"));
		}
	else
		{
		switch (iClientErrorResolver->LastError())
			{
			case EObexNoExtendedError:
				{
				iParent->Console()->Printf(_L("No extended error\r\n"));
				}
				break;
			case EObexRequestAccepted:
				{
				iParent->Console()->Printf(_L("Request accepted\r\n"));
				}
				break;
			case EObexRequestNotAccepted:
				{
				iParent->Console()->Printf(_L("Request not accepted\r\n"));
				}
				break;
			case EObexRequestTimeout:
				{
				iParent->Console()->Printf(_L("Request timeout\r\n"));
				}
				break;
			case EObexRequestLocalInterruption:
				{
				iParent->Console()->Printf(_L("Request local interruption\r\n"));
				}
				break;
			case EObexRequestLinkInterruption:
				{
				iParent->Console()->Printf(_L("Request link interruption\r\n"));
				}
				break;
			default:
				{
				iParent->Console()->Printf(_L("Last Error value returned from client not recognised\r\n"));
				}
			}
		}
	}
#endif // ERROR_RESOLUTION_EXTENSION

#ifdef EMPTY_HEADERS_TEST
void CObexClientHandler::EmptyHeaderTestL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam)
	{
	if(IsActive())
		{
		iParent->Console()->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}
		iObexEmptyHeaderTest->SetHeadersL(aName, aType, aDesc, aTarget, aAppParam);
		iClient->Put(*(iObexEmptyHeaderTest->ObexObject()),iStatus);
		SetActive();
		iState = EPutting;
	}
#endif //EMPTY_HEADERS_TEST
