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
#include "TOBEXCON.H"
#include "btextnotifiers.h"
#include "serverasyncao.h"
#include <obexconstants.h>
#include <obexbttransportinfo.h>
#include <obexirtransportinfo.h>
#include "obexconstantsinternal.h"

#include "serverpacketaccessui.h"

CObexServerHandlerBase::CObexServerHandlerBase(CActiveConsole* aParent)
	: iAcceptPuts(ETrue), iParent(aParent)
    {
    }


void CObexServerHandlerBase::BaseConstructL(TTransport aTransport)
    {

	iTransportLayer = aTransport;
	switch (aTransport)
		{
		case (EBluetooth):
			InitBluetoothL();
			break;
		
		case (EIrda):
			{
			if(iParent->UseIrdaNickname())
				{
				TObexIrV3TransportInfo aInfo;
				aInfo.iTransportName = KObexIrTTPProtocolV3;
				aInfo.iReceiveMtu = iParent->iRecvMTU;
				aInfo.iTransmitMtu = iParent->iTransmitMTU;
				aInfo.iAddr.SetPort(KAutoBindLSAP);//default obex server for now
				_LIT8(KClassName, "OBEX");
				_LIT8(KAttributeName, "IrDA:TinyTP:LsapSel");
				aInfo.iClassName     = KClassName;
				aInfo.iAttributeName = KAttributeName;
				aInfo.iDiscoverySlots = iParent->iDiscoverySlots;
				aInfo.iDiscoveryAttempts = iParent->iDiscoveryAttempts;
				aInfo.iLocalDeviceNickname = iParent->GetIrdaNickname();
				aInfo.iLocalDeviceNicknameValid = ETrue;
				iServer = CObexServer::NewL(aInfo);
				}
			else
				{
				TObexIrV2TransportInfo aInfo;
				aInfo.iTransportName = KObexIrTTPProtocolV2;
				aInfo.iReceiveMtu = iParent->iRecvMTU;
				aInfo.iTransmitMtu = iParent->iTransmitMTU;
				aInfo.iAddr.SetPort(KAutoBindLSAP);//default obex server for now
				_LIT8(KClassName, "OBEX");
				_LIT8(KAttributeName, "IrDA:TinyTP:LsapSel");
				aInfo.iClassName     = KClassName;
				aInfo.iAttributeName = KAttributeName;
				aInfo.iDiscoverySlots = iParent->iDiscoverySlots;
				aInfo.iDiscoveryAttempts = iParent->iDiscoveryAttempts;
				iServer = CObexServer::NewL(aInfo);
				}
			User::LeaveIfError(iServer->Start(this));
			
/*			// Uncomment this code to demonstrate CObexServer::TransportInfo 
			// API with TTP transport.
			// We get back an TObexIrTransportInfo because we asked for KObexIrTTPProtocol.
			const TObexIrTransportInfo* transportInfo = static_cast<const TObexIrTransportInfo*>(iServer->TransportInfo());
			iParent->Console()->Printf(_L("Transport info: \r\n"));
			iParent->Console()->Printf(_L("\ttransport name: %S\r\n"), &transportInfo->iTransportName);
			iParent->Console()->Printf(_L("\tversion: %d\r\n"), transportInfo->iVersion);
			iParent->Console()->Printf(_L("\treceive MTU: %d\r\n"), transportInfo->iReceiveMtu);
			iParent->Console()->Printf(_L("\ttransmitMTU: %d\r\n"), transportInfo->iTransmitMtu);
			iParent->Console()->Printf(_L("\tTinyTP port: 0x%02x\r\n"), transportInfo->iAddr.Port());
			TName wide;
			wide.Copy(transportInfo->iClassName);
			iParent->Console()->Printf(_L("\tclass name: %S\r\n"), &wide);
			wide.Copy(transportInfo->iAttributeName);
			iParent->Console()->Printf(_L("\tattribute name: %S\r\n"), &wide);
*/
			break;
			}
		
		case (EUsb):
			{
			TObexUsbProtocolInfo info;
			info.iTransport = KObexUsbProtocol;
			info.iInterfaceStringDescriptor = _L("TOBEX Server Interface");

			iServer = CObexServer::NewL (info);
			User::LeaveIfError(iServer->Start (this));
			
/*			// Uncomment this code to demonstrate CObexServer::TransportInfo 
			// API with USB transport.
			// We get back an TObexUsbTransportInfo because we asked for KObexUsbProtocol.
			const TObexUsbTransportInfo* transportInfo = static_cast<const TObexUsbTransportInfo*>(iServer->TransportInfo());
			iParent->Console()->Printf(_L("Transport info: \r\n"));
			iParent->Console()->Printf(_L("\ttransport name: %S\r\n"), &transportInfo->iTransportName);
			iParent->Console()->Printf(_L("\tversion: %d\r\n"), transportInfo->iVersion);
			iParent->Console()->Printf(_L("\treceive MTU: %d\r\n"), transportInfo->iReceiveMtu);
			iParent->Console()->Printf(_L("\ttransmitMTU: %d\r\n"), transportInfo->iTransmitMtu);
			iParent->Console()->Printf(_L("\tinterface string desc: %S\r\n"), &transportInfo->iInterfaceStringDescriptor);
*/
			break;
			}
		
		case (EWin32Usb):
			{
			TObexUsbProtocolInfo aInfo;
			aInfo.iTransport = _L("Win32Usb");
	        aInfo.iInterfaceStringDescriptor = _L("TOBEX Server Interface");
	        
			iServer = CObexServer::NewL (aInfo);
			User::LeaveIfError(iServer->Start (this));
			
/*			// Uncomment this code to demonstrate CObexServer::TransportInfo 
			// API with Win32USB transport.
			// We get back an TObexUsbTransportInfo because we asked for KObexUsbProtocol.
			const TObexUsbTransportInfo* transportInfo = static_cast<const TObexUsbTransportInfo*>(iServer->TransportInfo());
			iParent->Console()->Printf(_L("Transport info: \r\n"));
			iParent->Console()->Printf(_L("\ttransport name: %S\r\n"), &transportInfo->iTransportName);
			iParent->Console()->Printf(_L("\tversion: %d\r\n"), transportInfo->iVersion);
			iParent->Console()->Printf(_L("\treceive MTU: %d\r\n"), transportInfo->iReceiveMtu);
			iParent->Console()->Printf(_L("\ttransmitMTU: %d\r\n"), transportInfo->iTransmitMtu);
			iParent->Console()->Printf(_L("\tinterface string desc: %S\r\n"), &transportInfo->iInterfaceStringDescriptor);
*/
			break;
			}
		}
	
#ifdef ASYNC_INDICATION
    iServerAsyncAO = CServerAsyncAO::NewL(*iServer);
#endif
	}

void CObexServerHandlerBase::InitBluetoothL()
	{
	TBTServiceSecurity serv;
	serv.SetUid(KObexTestUid);
	
	// UPF - change security here for Bluetooth security testing
	serv.SetAuthentication(EFalse);
	serv.SetAuthorisation(EFalse);
	serv.SetEncryption(EFalse);

	// now set up Obex...
	TObexBluetoothProtocolInfo info;
	info.iTransport = KObexRfcommProtocol;
	info.iAddr.SetPort(KRfcommPassiveAutoBind);
	info.iAddr.SetSecurity(serv);

	TObexProtocolPolicy policy;
	policy.SetReceiveMtu(iParent->iRecvMTU);
	policy.SetTransmitMtu(iParent->iTransmitMTU);

	iServer  = CObexServer::NewL(info, policy);
	iServer->SetReadActivityObserver(this);

	User::LeaveIfError(iServer->Start (this));
	// We get back an TObexBtTransportInfo because we asked for KObexRfcommProtocol.
	const TObexBtTransportInfo* transportInfo = static_cast<const TObexBtTransportInfo*>(iServer->TransportInfo());
	iParent->Console()->Printf(_L("Transport info: \r\n"));
	iParent->Console()->Printf(_L("\ttransport name: %S\r\n"), &transportInfo->iTransportName);
	iParent->Console()->Printf(_L("\tversion: %d\r\n"), transportInfo->iVersion);
	iParent->Console()->Printf(_L("\treceive MTU: %d\r\n"), transportInfo->iReceiveMtu);
	iParent->Console()->Printf(_L("\ttransmitMTU: %d\r\n"), transportInfo->iTransmitMtu);
	iParent->Console()->Printf(_L("\tRFCOMM port: 0x%02x\r\n"), transportInfo->iAddr.Port());
	
	// SDP registration 
	User::LeaveIfError(iSdp.Connect());
	User::LeaveIfError(iSdpdb.Open(iSdp));

	TSdpServRecordHandle handle;
	iSdpdb.CreateServiceRecordL(TUUID(0x1105), handle);
	iSdpdb.UpdateAttributeL(handle, 0x100, _L8("Obex push server"));
	

	TSdpServRecordHandle ftphandle;
	iSdpdb.CreateServiceRecordL(TUUID(0x1106), ftphandle);
	iSdpdb.UpdateAttributeL(ftphandle, 0x100, _L8("File transfer server"));
			
	// Protocol Descriptor List (same for both profiles)
	iProtDescList = CSdpAttrValueDES::NewDESL(0);

	iProtDescList
	->StartListL()
		->BuildDESL()
		->StartListL()
			->BuildUUIDL(TUUID(TUint16(0x0100))) // L2CAP
		->EndListL()
		->BuildDESL()
		->StartListL()
			->BuildUUIDL(TUUID(TUint16(0x0003))) // RFCOMM
			->BuildUintL(TSdpIntBuf<TInt8>(transportInfo->iAddr.Port()))
		->EndListL()
		->BuildDESL()
		->StartListL()
			->BuildUUIDL(TUUID(TUint16(0x0008))) // OBEX
		->EndListL()
	->EndListL();

	iSdpdb.UpdateAttributeL(handle, 4, *iProtDescList);
	iSdpdb.UpdateAttributeL(ftphandle, 4, *iProtDescList); // put into both records
	delete iProtDescList;
	iProtDescList = NULL;

	// Supported formats list (OPP only)
	iProtDescList = CSdpAttrValueDES::NewDESL(0);

	iProtDescList->StartListL()
		->BuildUintL(TSdpIntBuf<TUint8>(0xFF)) // Accept any type of object
	->EndListL();

	iSdpdb.UpdateAttributeL(handle, 0x0303, *iProtDescList);
	
	delete iProtDescList;
	iProtDescList = NULL;
	
	// Add OPP to the supported profiles list
	iProtDescList = CSdpAttrValueDES::NewDESL(0);

	iProtDescList->StartListL()
		->BuildDESL()
		->StartListL()
			->BuildUUIDL(TUUID(0x1105)) // OPP Profile
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // Profile version
		->EndListL()
	->EndListL();

	iSdpdb.UpdateAttributeL(handle, 9, *iProtDescList); // profile list

	delete iProtDescList;
	iProtDescList = NULL;

	// Add FTP to the supported profiles list
	iProtDescList = CSdpAttrValueDES::NewDESL(0);

	iProtDescList->StartListL()
		->BuildDESL()
		->StartListL()
			->BuildUUIDL(TUUID(0x1106)) // FTP Profile
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // Profile version
		->EndListL()
	->EndListL();

	iSdpdb.UpdateAttributeL(ftphandle, 9, *iProtDescList); // profile list

	delete iProtDescList;
	iProtDescList = NULL;
	
	// Set host name
	RSocketServ ss;
	ss.Connect();

	RHostResolver hr;
	User::LeaveIfError(hr.Open(ss,KBTAddrFamily,KBTLinkManager));
	User::LeaveIfError(hr.SetHostName(_L("Symbian Push Server")));
	ss.Close();

	// Enable enquiry and page scan
	User::LeaveIfError(RProperty::Set(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothSetScanningStatus, EInquiryAndPageScan));
	}

CObexServerHandlerBase::~CObexServerHandlerBase ()
    {
    Stop();

	if (iTransportLayer == EBluetooth)
		{
		// security now cleared with socket closure
		iSdpdb.Close();
		iSdp.Close();
		}
#ifdef PACKET_ACCESS_EXTENSION
	delete iPacketAccessUi;
#endif // PACKET_ACCESS_EXTENSION

#ifdef ASYNC_INDICATION		
	delete iServerAsyncAO;
#endif
    delete iServer;
	delete iProtDescList;
    }

void CObexServerHandlerBase::RemoteAddress()
	{
	TSockAddr aAddr;
	iServer->RemoteAddr( aAddr);
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


void CObexServerHandlerBase::Start()
    {
    if (iServer)
		{
		iServer->Start(this);
		}
    }

void CObexServerHandlerBase::Stop()
    {
	if (iServer)
		{
		iServer->Stop();
		}
    }

void CObexServerHandlerBase::SetTargetChecking(CObexServer::TTargetChecking aValue)
	{
    iServer->SetTargetChecking(aValue);
	}



// MObexServerNotify interface functions

void CObexServerHandlerBase::ErrorIndication (TInt aError)
    {
    iParent->Console()->Printf(_L("Obex Server Error: %d\r\n"), aError);
    }

void CObexServerHandlerBase::TransportUpIndication ()
    {
    iParent->Console()->Printf(_L("\r\nTransport layer now connected\r\n\r\n"));
    }

void CObexServerHandlerBase::TransportDownIndication ()
    {
    iParent->Console()->Printf(_L("\r\nTransport layer has gone down\r\n\r\n"));
    
    if (iObject)
		{
		if(!iObject->BytesReceived())
			{
			return;
			}
		
		iParent->Console()->Printf(_L("\r\nWe have received part of an Obex object\r\n\r\n"));

		iObject->Reset();//close file handle
		}
    }
#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& /*aInfo*/)
#else
TInt CObexServerHandlerBase::ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& /*aInfo*/)
#endif
    {
	iParent->Console()->Printf(_L("\r\nCObexServerHandler::ObexConnectIndication"));
	iParent->Console()->Printf(_L("\r\nConnected to machine with OBEX version %d.%d\r\n"), 
		     aRemoteInfo.VersionMajor (), aRemoteInfo.VersionMinor ());

	if ( aRemoteInfo.iWho.Length() > 0 )
		{
		iParent->Console()->Printf(_L("EWho received from remote side ="));

		for(TInt count = 0; count < aRemoteInfo.iWho.Length(); count++) 
			{

			TChar aChar;

			aChar = (aRemoteInfo.iWho[count] & 0xf0)>>4;
			if (aChar <= 9)
				aChar += 0x30;
			else if ((aChar >= 0x0a) && (aChar <= 0x0f))
				aChar += 'A' - 0x0a;
			iParent->Console()->Printf(_L("%C"),(TUint)aChar);

			aChar = (aRemoteInfo.iWho[count] & 0x0f);
			if (aChar <= 9)
				aChar += 0x30;
			else if ((aChar >= 0x0a) && (aChar <= 0x0f))
				aChar += 'A' - 0x0a;
			iParent->Console()->Printf(_L("%C"),(TUint)aChar);

			}
		iParent->Console()->Printf(_L("\r\n"));
		}
	else
		iParent->Console()->Printf(_L("No Who Header received\r\n"));

	if ( aRemoteInfo.iTargetHeader.Length() > 0 )
		{
		iParent->Console()->Printf(_L("ETarget received from remote side ="));
		for(TInt count = 0; count < aRemoteInfo.iTargetHeader.Length(); count++) 
			{
			TChar aChar;

			aChar = (aRemoteInfo.iTargetHeader[count] & 0xf0)>>4;
			if (aChar <= 9)
				aChar += 0x30;
			else if ((aChar >= 0x0a) && (aChar <= 0x0f))
				aChar += 'A' - 0x0a;
			iParent->Console()->Printf(_L("%C"),(TUint)aChar);

			aChar = (aRemoteInfo.iTargetHeader[count] & 0x0f);
			if (aChar <= 9)
				aChar += 0x30;
			else if ((aChar >= 0x0a) && (aChar <= 0x0f))
				aChar += 'A' - 0x0a;
			iParent->Console()->Printf(_L("%C"),(TUint)aChar);
			}
		iParent->Console()->Printf(_L("\r\n"));
		}
	else
		iParent->Console()->Printf(_L("No Target Header received\r\n"));

#ifndef ASYNC_INDICATION
	return KErrNone;
#endif	
    } 

void CObexServerHandlerBase::ObexDisconnectIndication (const TDesC8& /*aInfo*/)
    {
    iParent->Console()->Printf(_L("\r\nObex Disconnected\r\n\r\n"));
    }

#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::PutRequestIndication ()
#else
CObexBufObject* CObexServerHandlerBase::PutRequestIndication ()
#endif
    {
	iStartTime.HomeTime();
	
    iParent->Console()->Printf(_L("Receiving object...\r\n"));
    iObject->Reset ();
#ifdef ASYNC_INDICATION
	iServerAsyncAO->RequestIndication(CObex::EOpPut, iAcceptPuts?iObject:NULL);
#else
	return iAcceptPuts?iObject:NULL;
#endif
    }

TInt CObexServerHandlerBase::PutPacketIndication ()
    {
    // Only output % received if performance logging is disabled.
	if(!iParent->iPerformanceLoggingEnabled)
		{
		TUint length = iObject->Length();
		TUint received = iObject->BytesReceived();
		TUint8 percent = 0;
	    if (length > 0)
			{
			percent = TUint8((100 * received) / length);
			iParent->Console()->Printf(_L("\r%d %%      "), percent);
			}
	    else
			{
			iParent->Console()->Printf(_L("\r%d Bytes      "), iObject->BytesReceived ());
			}
		}
    return (KErrNone);
    }
    
#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::PutCompleteIndication()
#else
TInt CObexServerHandlerBase::PutCompleteIndication()
#endif
    {
	TTime finishTime;
	finishTime.HomeTime();
	TTimeIntervalMicroSeconds diff = finishTime.MicroSecondsFrom(iStartTime);	

	iParent->Console()->Printf(_L("\nObject Received in %d"), diff.Int64());

    TPtrC name=iObject->Name();
	TBuf<100> type;
	type.Copy(iObject->Type());
	iParent->Console()->Printf(_L("\r\nSuccessfully received '%S'\r\nType[%d]: '%S'\r\n"), &name, type.Length(), &type);
	
	TInt err = KErrNone;
	if (iParent->DisplayHeaders())
		{
		// No need to put header on the cleanup stack, as there are no leaving functions during its lifetime
		CObexHeader* header = NULL;
		TRAP(err, header = CObexHeader::NewL());
		if (err)
			{
			iParent->iConsole->Printf(_L("Error allocating memory to display\r\n"));
			}

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
					TRAPD(headerErr, buf = HBufC16::NewL((header->AsUnicode()).Size()));
					if (headerErr)
						{
						iParent->iConsole->Printf(_L("Unicode Header (0x%x)- Error allocating memory to display\r\n"), header->HI());
						}
					else
						{
						TPtr16 type(buf->Des());
						type.Copy(header->AsUnicode());

						iParent->iConsole->Printf(_L("Unicode Header (0x%x) =  : \"%S\"\r\n"), header->HI(), &type);
						delete buf;
						}
					break;
					}
				case (0x01): // ByteSeq
					{
					HBufC16* buf = NULL;
					TRAPD(headerErr, buf = HBufC16::NewL((header->AsByteSeq()).Size()));
					if (headerErr)
						{
						iParent->iConsole->Printf(_L("ByteSeq Header (0x%x)- Error allocating memory to display\r\n"), header->HI());
						}
					else
						{
						TPtr16 type(buf->Des());
						type.Copy(header->AsByteSeq());

						iParent->iConsole->Printf(_L("ByteSeq Header (0x%x) =  : \"%S\"\r\n"), header->HI(), &type);
						delete buf;
						}
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
		
		delete header;
		}	

	TDateTime dt = iObject->Time().DateTime();
	iParent->Console()->Printf(_L("\r\nTimestamp: %d/%d/%d, %d:%d:%d\r\n\r\n"),
				   dt.Day()+1, dt.Month()+1, dt.Year(), dt.Hour(), dt.Minute(), dt.Second());

	// Reset err to no error;
	err = KErrNone;

	if ((iObject->Name() == KRef1Name) ||
	    (iObject->Name() == KRef2Name) ||
	    (iObject->Name() == KRef3Name) ||
	    (iObject->Name() == KRef4Name) ||
	    (iObject->Name() == KRef5Name) ||
	    (iObject->Name() == KRef6Name) ||
	    (iObject->Name() == KRef7Name) ||
	    (iObject->Name() == KRef8Name) ||
	    (iObject->Name() == KRef9Name))
		{
		iParent->Console()->Printf(_L("\r\nReference Object Received"));
		TInt objComp = 0;
		TRAP(err, objComp = iParent->iRefHandler->CompareObjectToReferenceL(*iObject, *iBuf, iParent->iTransport));
		if ((objComp != 0) || (err != KErrNone)) // Make sure the objects match
			{
			User::Panic(_L("TOBEX - OBJCOMP"), err);
			}
		iParent->Console()->Printf(_L("\r\nReference Object Comparison OK\r\n\r\n"));
		}
	else
		{
		iParent->Console()->Printf(_L("Size of received object = %d\n"),iBuf->Size());

	    // Only output packet contents info if performance logging is disabled.
		if(!iParent->iPerformanceLoggingEnabled)
			{
		    TBuf8<1024> tempBuffer;
			iBuf->Read(0, tempBuffer, tempBuffer.MaxSize() < iBuf->Size() ? tempBuffer.MaxSize() : iBuf->Size());

			// Printf fails with Descriptor beigger than X hundred bytes so write byte at a time
			for(TInt count = 0; count < tempBuffer.Size(); count++) 
				{
				iParent->Console()->Printf(_L("%C"),tempBuffer[count]);
				}
			}

		if (iObject->Name() != KNullDesC )
			{
			TFileName filename;
			filename = iParent->iInboxRoot;
			
			TInt bufSpaceLeft = filename.MaxLength() - filename.Length();
			TInt objNameLen = iObject->Name().Length();
			TInt numToCopy = objNameLen;
			if (bufSpaceLeft < objNameLen)
				{
				numToCopy = bufSpaceLeft;
				iParent->Console()->Printf(_L("Truncated resulting local filename\n"));
				}
			filename.Append(iObject->Name().Ptr(), numToCopy);
			
			err = iObject->WriteToFile(filename);

			if (err == KErrAlreadyExists)
				{
				iParent->Console()->Printf(_L("\r\nWrite failed, File Already Exists\n"));
				}
			else if (err != KErrNone)
				{
				iParent->Console()->Printf(_L("\r\nWrite failed with error %d\n"), err);
				}
			}
		else
			{
			err = KErrNone;
			iParent->Console()->Printf(_L("\r\nReceived object with empty Name Header.\nNot writing to a file\n"));
			}
		}

	TObexHeaderMask headers = iObject->ValidHeaders();
	iParent->Console()->Printf(_L("\n"));
	if (headers & KObexHdrBody)
		iParent->Console()->Printf(_L("Body header received\n"));
	if (headers & KObexHdrEndOfBody)
		iParent->Console()->Printf(_L("EndOfBody header received\n"));

    iObject->Reset ();
#ifdef ASYNC_INDICATION    
	if(err)
		{
		iServerAsyncAO->CompletionIndication(CObex::EOpPut, ERespNotAcceptable);
		}
	else
		{
		iServerAsyncAO->CompletionIndication(CObex::EOpPut, ERespSuccess);
		}
		
#else
	return err;
#endif
    }

void CObexServerHandlerBase::HandleGetReferenceObjL(CObexBaseObject* aRequiredObject)
	{
	CObexBufObject& obj = *static_cast<CObexBufObject*>(aRequiredObject);
	CBufFlat* dummyBufPtr = CBufFlat::NewL(0);
	CleanupStack::PushL(dummyBufPtr);

	// See if what we got in the Get request is the same as the reference object...
	TInt objComp = iParent->iRefHandler->CompareObjectToReferenceL(obj, *dummyBufPtr, iParent->iTransport);
	CleanupStack::PopAndDestroy(dummyBufPtr);
	ASSERT(objComp == CReferenceHandler::EDifferentBuf);

	// Everythings OK so set up the reference object to be returned
	iParent->Console()->Printf(_L("Obex Get Reference Object Request\r\n"));


	if (aRequiredObject->Name() == KRef1Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 1, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef2Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 2, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef3Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 3, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef4Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 4, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef5Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 5, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef6Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 6, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef7Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 7, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef8Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 8, iParent->iTransport);
	else if (aRequiredObject->Name() == KRef9Name)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 9, iParent->iTransport);
	else // capture error condition (use reference 1)
		iParent->iRefHandler->CreateReferenceL(*iObject, *iBuf, 1, iParent->iTransport);

	}

#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::GetRequestIndication (CObexBaseObject* aRequiredObject)
#else
CObexBufObject* CObexServerHandlerBase::GetRequestIndication (CObexBaseObject* aRequiredObject)
#endif
    {
	//check if app params header sent across
	if ( aRequiredObject->ValidHeaders() & KObexHdrAppParam )
		{
		TBuf<30> localBuf;
		localBuf.Copy(aRequiredObject->AppParam());
		
		iParent->iConsole->Printf(_L(" App Param received =  : %S\r\n"), &localBuf);
		}
	else 
		{
		iParent->iConsole->Printf(_L(" No App Param Headers received\r\n"));
		}

	//check if any HTTP headers were sent across
	if ( aRequiredObject->ValidHeaders() & KObexHdrHttp)
		{
		//OK so lets get them out
		const RPointerArray<HBufC8>* localHttpArray = aRequiredObject->Http(); 
		//how many?
		TInt count = localHttpArray->Count();
		HBufC8* localArray;
		TBuf8<30> localBuf;
		TBuf<40> buf;
		TInt size = 30;
		for ( TInt x = 0; x < count; x++ )
			{
			localArray = (*localHttpArray)[x];
			size = Max(localArray->Size(), 30);
			localBuf.Copy(localArray->Ptr(), size);
			buf.Copy(localBuf);
			iParent->iConsole->Printf(_L(" HTTP Header Received : %S\r\n"), &buf);
			}
		}
	else 
		{
		iParent->iConsole->Printf(_L(" No HTTP Headers received\r\n"));
		}
	
	if ((aRequiredObject->Name() == KRef1Name) ||
	    (aRequiredObject->Name() == KRef2Name) ||
	    (aRequiredObject->Name() == KRef3Name) ||
	    (aRequiredObject->Name() == KRef4Name) ||
	    (aRequiredObject->Name() == KRef5Name) ||
	    (aRequiredObject->Name() == KRef6Name) ||
	    (aRequiredObject->Name() == KRef7Name) ||
	    (aRequiredObject->Name() == KRef8Name) ||
	    (aRequiredObject->Name() == KRef9Name))
		{
#ifdef _DEBUG
		TRAPD(err, HandleGetReferenceObjL(aRequiredObject));
		ASSERT(err==0);
#else
		TRAP_IGNORE(HandleGetReferenceObjL(aRequiredObject));
#endif
		}
	else
		{
		TRAPD(err,SetUpGetObjectL(aRequiredObject));
		if (err != KErrNone)
			{
			iParent->Console()->Printf(_L("\nSetUpGetObjectL() returned %d.\n"), err);
#ifdef ASYNC_INDICATION
			iServerAsyncAO->RequestIndication(CObex::EOpGet, NULL);
#else
			iServer->RequestIndicationCallbackWithError(err); //added to test PDEF097129
			return (NULL);
#endif
			}
		}
#ifdef ASYNC_INDICATION
	iServerAsyncAO->RequestIndication(CObex::EOpGet, iObject);
#else
	return (iObject);
#endif
	}

TInt CObexServerHandlerBase::GetPacketIndication ()
    {
    if (iObject->Length () > 0)
    	{
    	TInt percentComplete = 100 * iObject->BytesSent () / iObject->Length ();
    	iParent->Console()->Printf(_L("\r%d %%      "), percentComplete);
    	}
    else
    	{
    	iParent->Console()->Printf(_L("\r%d Bytes      "), iObject->BytesSent ());
    	}
    
    return (KErrNone);
    }

#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::GetCompleteIndication ()
#else
TInt CObexServerHandlerBase::GetCompleteIndication ()
#endif
    {
    iParent->Console()->Printf(_L("Obex Get Complete\r\n"));
    iObject->Reset ();
#ifdef ASYNC_INDICATION   
	iServerAsyncAO->CompletionIndication(CObex::EOpGet, ERespSuccess);
#else
	return KErrNone;
#endif
    }

#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::SetPathIndication (const CObex::TSetPathInfo& aPathInfo, const TDesC8& /*aInfo*/)
#else
TInt CObexServerHandlerBase::SetPathIndication (const CObex::TSetPathInfo& aPathInfo, const TDesC8& /*aInfo*/)
#endif
    {
    iParent->Console()->Printf(_L("Obex SetPath request:\r\n"));
    iParent->Console()->Printf(_L("   --- Flags = '%d' - Constants = '%d' - "), aPathInfo.iFlags, aPathInfo.iConstants);
    if (aPathInfo.iNamePresent)
		iParent->Console()->Printf(_L("Name = %S\r\n"), &aPathInfo.iName);
	else
		iParent->Console()->Printf(_L("> No Name Present <\r\n"));

    iParent->Console()->Printf(_L("\nReturning success...!\n"));
#ifdef ASYNC_INDICATION
	iServerAsyncAO->CompletionIndication(CObex::EOpSetPath, ERespSuccess);
#else
	return (KErrNone);
#endif
    }

void CObexServerHandlerBase::AbortIndication ()
    {
    iParent->Console()->Printf(_L("Obex Operation aborted\r\n"));
    if(iObject)
		{
		if(!iObject->BytesReceived())
		    return;
		iParent->Console()->Printf(_L("\r\nWe have received part of an Obex object\r\n\r\n"));

		iObject->Reset();//closes file handle
		}
    }

void CObexServerHandlerBase::MoraoReadActivity()
	{
    iParent->Console()->Printf(_L("Obex Server: 'Read Activity' signalled\r\n"));
	}
	
void CObexServerHandlerBase::SetUpGetObjectL(CObexBaseObject *aRequestedObject)
	{
	TFileName name;
	name = iParent->iInboxRoot;
	name.Append(aRequestedObject->Name ());
	if(name.Length())
		{
		iParent->Console()->Printf(_L("Obex Get Request for name '%s'\r\n"), name.PtrZ ());
		}
	else if (aRequestedObject->Type().Length())
		{
		name = iParent->iInboxRoot;
		TBuf<32> temp;
		temp.Copy(aRequestedObject->Type());
		name.Append(temp);
		iParent->Console()->Printf(_L("Obex Get Request for type '%s'\r\n"), name.PtrZ ());
		TInt pos;
		while((pos=name.Locate('/')) >= 0)
			name[pos] = '_';
		pos=name.Length()-1;
		if(pos>=0 && name[pos] == 0)
			name.SetLength(pos); // Work around last character possibly being NULL
		}
	else
		{
		iParent->Console()->Printf(_L("Obex Get Request unknown details\r\n"));
		User::Leave(KErrNotSupported);
		}
	iObject->Reset();

	RFs fs;
	RFile f;
	if ((fs.Connect () != KErrNone) || 
		(f.Open (fs, name, EFileShareReadersOnly | EFileRead) != KErrNone))
		{
		iParent->Console()->Printf(_L("\r\nError reading '%S'."), &name);
		User::Leave(KErrNotFound);
		}

	TInt size = 0;
	User::LeaveIfError(f.Size (size));
	iBuf->ResizeL(size);
	TPtr8 data (iBuf->Ptr(0));
	f.Read (data);
	if (iBuf->Size() < size)
		User::Leave(KErrGeneral);
	iObject->SetNameL(name);
	iObject->SetLengthL(size);
	TTime time;
	if (f.Modified(time) == KErrNone)
		iObject->SetTimeL(time);
	}

void CObexServerHandlerBase::EnableAuthentication()
	{
	TRAPD(err, iServer->SetChallengeL(iChallengePassword));
	if (err == KErrNone)
		{
		iIsAuthenticationEnabled = ETrue;
		}
	else
		{
		iParent->iConsole->Printf(_L("Failed to set authentication password\r\n"));
		}
	}

void CObexServerHandlerBase::DisableAuthentication()
	{
	iServer->ResetChallenge();
	iIsAuthenticationEnabled = EFalse;
	}

void CObexServerHandlerBase::EnablePassword()
	{
	iServer->SetCallBack(*this);
	}

void CObexServerHandlerBase::ChangeChallengePassword(TDesC* aPassword)
	{
	if (aPassword)
		iChallengePassword = *aPassword;
	else
		iParent->SetPassword(iChallengePassword);

	if (iIsAuthenticationEnabled)
		{
		DisableAuthentication();
		EnableAuthentication();
		}
	}

void CObexServerHandlerBase::ChangeResponsePassword()
	{
	iParent->Cancel();
	iParent->SetPassword(iResponsePassword);
	iParent->RequestCharacter(); // re-request, to re-display menu
	}

void CObexServerHandlerBase::GetUserPasswordL(const TDesC& aUserID)
	{
	iParent->Console()->Printf(_L("\r\nServer has been challenged by %S"), &aUserID);
	ChangeResponsePassword(); //get the password from user
	iServer->UserPasswordL(iResponsePassword);
	}

void CObexServerHandlerBase::SetLocalWho()
	{
	iServer->SetLocalWho(KRefTarget);
	}

#ifdef ASYNC_INDICATION
void CObexServerHandlerBase::CancelIndicationCallback()
	{
	iServerAsyncAO->Cancel();
	}
#endif

#ifdef PACKET_ACCESS_EXTENSION	
void CObexServerHandlerBase::PacketAccessUiL(TBool aEnable)
	{
	if (aEnable)
		{
		iPacketAccessUi = CObexServerPacketAccessUi::NewL(*this, *iServer);
		}
	else
		{
		delete iPacketAccessUi;
		iPacketAccessUi = NULL;
		}
	}
#endif // PACKET_ACCESS_EXTENSION
