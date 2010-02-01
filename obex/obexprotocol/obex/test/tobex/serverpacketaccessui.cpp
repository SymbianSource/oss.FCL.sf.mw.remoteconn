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

#include "serverpacketaccessui.h"

#include <e32cons.h>
#include <obex/extensionapis/obexserverpacketaccessextension.h>
#include <obexserver.h>
#include <obexheaders.h>
#include "serverhandlerbase.h"


CObexServerPacketAccessUi* CObexServerPacketAccessUi::NewL(CObexServerHandlerBase& aHandler, CObexServer& aServer)
	{
	CObexServerPacketAccessUi* self = new (ELeave) CObexServerPacketAccessUi(aHandler, aServer);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
CObexServerPacketAccessUi::~CObexServerPacketAccessUi()
	{
	Cancel();
	
	delete iPacketAccess;
	delete iConsole;
	}

CObexServerPacketAccessUi::CObexServerPacketAccessUi(CObexServerHandlerBase& aHandler, CObexServer& aServer)
	: CActive(EPriorityNormal), iHandler(aHandler), iServer(aServer)
	{
	CActiveScheduler::Add(this);
	}
	
void CObexServerPacketAccessUi::ConstructL()
	{	
	TRAPD(err, iConsole = Console::NewL(_L("Request Packets"),TSize(50,40)));
	if (err != KErrNone)
		{
		TRAPD(err, iConsole = Console::NewL(_L("Request Packets"),TSize(50,15)));
		if (err != KErrNone)
			{
			// we had a problem creating the console, probably because it was too big
			// so try again and make it full screen. If we leave this time then just
			// go with it.
			iConsole = Console::NewL(_L("Request Packets"),TSize(KConsFullScreen,KConsFullScreen));
			}
		}
	iPacketAccess = CObexServerPacketAccessExtension::NewL(iServer, *this);
	
	SetKeypressActive();
	}
	
void CObexServerPacketAccessUi::RunL()
	{
    TRAPD(err, ProcessKeyPressL(TChar(iConsole->KeyCode())));
	if(err != KErrNone)
		{
		iConsole->Printf(_L("Error: %d\r\n"),err);
		}
	SetKeypressActive();
	}

void CObexServerPacketAccessUi::DoCancel()
	{
	iConsole->ReadCancel();
	}
	
void CObexServerPacketAccessUi::SetKeypressActive()
	{
	iConsole->Read(iStatus); 
	SetActive();
	}
	
void CObexServerPacketAccessUi::ProcessKeyPressL(TChar aChar)
	{
	if (aChar == EKeyEscape)
		{
		iHandler.PacketAccessUiL(EFalse);
		return;
		}

	switch (aChar)
		{
		case 'E': case 'e': // Enable
			{
			delete iPacketAccess;
			iPacketAccess = CObexServerPacketAccessExtension::NewL(iServer, *this);
			}
			break;
		case 'D': case 'd': // Disable
			{
			delete iPacketAccess;
			iPacketAccess = NULL;
			}
			break;
		case 'I': case 'i': // Interactive mode toggle
			{
			iInteractive = !iInteractive;
			}
			break;
		default:
			{
			iConsole->Printf(_L("Invalid Key - valid keys 'e','d','i','Esc'\r\n"));
			}
			break;
		}
	}
	
TBool CObexServerPacketAccessUi::RequestPacket(TObexRequestCode aRequest, TObexResponse& aResponse)
	{
	TBool requestPacketResult(ETrue);
	
	iConsole->Printf(_L("Request code %d\r\n"), aRequest);
	
	// Use Packet Headers now
	CObexHeaderSet* headerSet = NULL;
	iServer.PacketHeaders(headerSet);
	if (headerSet)
		{
		// Ensures that if the function leaves then the only option is to 
		// report a false operation.  As RequestPacket function signiture is published partner
		TRAPD(err,PrintHeadersL(*headerSet));
		if (err != KErrNone)
			{
			requestPacketResult = EFalse;						
			}
		delete headerSet;
		}
	
	if (requestPacketResult)
		{
		requestPacketResult = RequestPacketReturn(aResponse);
		}
		
	// Provide function result to caller	
	return requestPacketResult;
	}
	
TBool CObexServerPacketAccessUi::RequestPacket(TObexRequestCode aRequest, TObexConnectInfo& aConnectInfo, TObexResponse& aResponse)
	{
	TBool requestPacketResult(ETrue);
	
	iConsole->Printf(_L("Request code %d - Connect\r\n"), aRequest);
	iConsole->Printf(_L("\tMajor version=%d, Minor version=%d\r\n"), aConnectInfo.VersionMajor(), aConnectInfo.VersionMinor());
	iConsole->Printf(_L("\tConnection flags=0x%02x\r\n"), aConnectInfo.iFlags);
	if (aConnectInfo.iWho.Length() > 0)
		{
		iConsole->Printf(_L("\tWho attribute=%S\r\n"), &(aConnectInfo.iWho));
		}
	if (aConnectInfo.iTargetHeader.Length() > 0)
		{
		iConsole->Printf(_L("\tTarget header=%S\r\n"), &(aConnectInfo.iTargetHeader));
		}
		
	// Use Packet Headers now
	CObexHeaderSet* headerSet = NULL;
	iServer.PacketHeaders(headerSet);
	if (headerSet)
		{
		// Ensures that if the function leaves then the only option is to 
		// report a false operation.  As RequestPacket function signiture is published partner
		TRAPD(err,PrintHeadersL(*headerSet));
		if (err != KErrNone)
			{
			requestPacketResult = EFalse;						
			}
		delete headerSet;
		}
			
	if (requestPacketResult)
		{
		requestPacketResult = RequestPacketReturn(aResponse);
		}
		
	// Provide function result to caller
	return requestPacketResult;
	}
	
TBool CObexServerPacketAccessUi::RequestPacket(TObexRequestCode aRequest, CObex::TSetPathInfo& aSetPathInfo, TObexResponse& aResponse)
	{
	TBool requestPacketResult(ETrue);
			
	iConsole->Printf(_L("Request code %d - SetPath\r\n"), aRequest);
	iConsole->Printf(_L("\tParent=%d\r\n"), aSetPathInfo.Parent());
	iConsole->Printf(_L("\tSetPath flags=0x%02x, constants=0x%02x\r\n"), aSetPathInfo.iFlags, aSetPathInfo.iConstants);
	if (aSetPathInfo.iNamePresent)
		{
		iConsole->Printf(_L("\tDestination directory=%S\r\n"), &(aSetPathInfo.iName));
		}
	
	// Use Packet Headers now
	CObexHeaderSet* headerSet = NULL;
	iServer.PacketHeaders(headerSet);
	if (headerSet)
		{
		// Ensures that if the function leaves then the only option is to 
		// report a false operation.  As RequestPacket function signiture is published partner
		TRAPD(err,PrintHeadersL(*headerSet));
		if (err != KErrNone)
			{
			requestPacketResult = EFalse;						
			}
		delete headerSet;
		}
	
	if (requestPacketResult)
		{
		requestPacketResult = RequestPacketReturn(aResponse);
		}
	
	// Provide function result to caller
	return requestPacketResult; 
	}
	
TBool CObexServerPacketAccessUi::RequestPacketReturn(TObexResponse& aResponse)
	{
	if (iInteractive)
		{
		TKeyCode key(EKeyNull);
		TBuf<2> buf;
		aResponse = ERespBadRequest; 
		
		Cancel(); // cancel getting key presses will in "submenu"
		
		iConsole->Printf(_L("Adandon Packet? (y/n) "));
		while (key != 'y' && key != 'Y' && key != 'n' && key != 'N')
			{
			key = iConsole->Getch();
			}
		iConsole->Printf(_L("%c"),key);
		if (key == 'n' || key == 'N')
			{
			SetKeypressActive(); // reactivate key handling
			iConsole->Printf(_L("\r\n"));
			return ETrue;
			}
		
		// If here then we are abandoning the packet
		iConsole->Printf(_L("\r\nResponse to send? 0x"));
		
		while ((key = iConsole->Getch()) != EKeyEnter)
			{
			if (key == EKeyBackspace && buf.Length()!=0)
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
		iConsole->Printf(_L("\r\n"));
		
		TLex lex(buf);
		TUint value;
		TInt lex_err = lex.Val(value, EHex); 
		
		if(lex_err == KErrNone)
			{
			aResponse = static_cast<TObexResponse>(value);
			}
		else 
			{
			iConsole->Printf(_L("Input parsing failed, use ERespBadRequest as default response\r\n"));
			}
		SetKeypressActive(); // reactivate key handling
		return EFalse;
		}
	else
		{
		return ETrue;
		}
	}
	
CConsoleBase& CObexServerPacketAccessUi::Console() const
	{
	return *iConsole;
	}
	
void CObexServerPacketAccessUi::PrintHeadersL(CObexHeaderSet& aHeaderSet) const
	{
	CObexHeader* header = CObexHeader::NewL();
	CleanupStack::PushL(header);
	
	aHeaderSet.First();
	TInt err(aHeaderSet.This(header));
	if (err == KErrNone)
		{
		iConsole->Printf(_L("Packet header contents...\r\n"));
		}
	while (err == KErrNone)
		{
		// get the next header
		err = aHeaderSet.This(header);
		if (err != KErrNone)
			{
			break;
			}
		
		PrintHeader(*header);
		
		err = aHeaderSet.Next();
		}
		
	CleanupStack::PopAndDestroy(header);
	}
	
void CObexServerPacketAccessUi::PrintHeader(CObexHeader& aHeader) const
	{
		// appropriately print the header
		iConsole->Printf(_L("\tHI: %d\r\n"),aHeader.HI());
		
		CObexHeader::THeaderType type = aHeader.Type();
		switch(type)
			{
			case CObexHeader::EUnicode:
				{
				const TDesC16& headerdes = aHeader.AsUnicode();
				const TUint8* bytes = reinterpret_cast<const TUint8*>(headerdes.Ptr());
				PrintHex(bytes, headerdes.Size());
				}		
				break;
			case CObexHeader::EByteSeq:
				{
				const TDesC8& headerbyteseq = aHeader.AsByteSeq();
				PrintHex(headerbyteseq.Ptr(), headerbyteseq.Size());
				}
				break;
			case CObexHeader::EByte:
				{
				const TUint8 headerbyte = aHeader.AsByte();
				PrintHex(&headerbyte, 1);
				}
				break;
			case CObexHeader::EFourByte:
				{
				const TUint32 headerbytes = aHeader.AsFourByte();
				const TUint8* bytes = reinterpret_cast<const TUint8*>(&headerbytes);
				PrintHex(bytes, 4);
				}
				break;
			default:
				// panic here maybe??
				break;
			}
	}
	
void CObexServerPacketAccessUi::PrintHex(const TUint8* aPtr, TInt aLength) const
	{
	// local constants
	const TUint32 widthC = 8;
	_LIT(KPrintHexByte, "%02x ");
	_LIT(KPrintHexTab, "\t");
	_LIT(KPrintHexChar, "%c ");
	_LIT(KPrintHexNonchar, ". ");
	_LIT(KPrintHexNewline, "\r\n");
	_LIT(KPrintHex2Spaces, "  ");
	_LIT(KPrintHex3Spaces, "   ");
	
	// function begins...
	TInt length = 0;
	TUint32 cursor = 0;
	
	TUint8 printArray[widthC];
	
	while (length < aLength)
		{
		printArray[cursor] = *aPtr;
		
		// move along.
		aPtr++;
		length++;
		cursor = length % widthC;
		
		if (cursor == 0)
			{
			// we have a full array so print it.
			iConsole->Printf(KPrintHexTab());
			for (TUint i=0;i<widthC;i++)
				{
				iConsole->Printf(KPrintHexByte(),printArray[i]);
				}
			iConsole->Printf(KPrintHexTab());
			for (TUint i=0;i<widthC;i++)
				{
				TUint8 c = printArray[i];
				if (c >= 32 && c <= 126)
					{
					iConsole->Printf(KPrintHexChar(), c);
					}
				else
					{
					iConsole->Printf(KPrintHexNonchar());
					}
				}
			iConsole->Printf(KPrintHexNewline());
			}
		}
		
		// if we haven't completely finished a set -> finish it
		if (cursor > 0)
			{
			iConsole->Printf(KPrintHexTab());
			for (TUint i=0;i<widthC;i++)
				{
				if (i >= cursor)
					{
					iConsole->Printf(KPrintHex3Spaces());
					}
				else
					{
					iConsole->Printf(KPrintHexByte(),printArray[i]);
					}
				}
			iConsole->Printf(KPrintHexTab());
			for (TUint i=0;i<widthC;i++)
				{
				if (i >= cursor)
					{
					iConsole->Printf(KPrintHex2Spaces());
					}
				else
					{
					TUint8 c = printArray[i];
					if (c >= 32 && c <= 126)
						{
						iConsole->Printf(KPrintHexChar(), c);
						}
					else
						{
						iConsole->Printf(KPrintHexNonchar());
						}
					}
				}
			iConsole->Printf(KPrintHexNewline());
			}
			
		// should be printed now
	}
