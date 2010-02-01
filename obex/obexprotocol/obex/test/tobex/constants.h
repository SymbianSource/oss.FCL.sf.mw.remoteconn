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

#ifndef _TOBEXCONSTANTS_H
#define _TOBEXCONSTANTS_H

// Required to receive into a file (inc. buffered writes)
_LIT(KFileName, "obex-temp.tmp");
_LIT(KRFileTestFileName, "obex-RFile-temp.txt");


_LIT(KRef1Name, "EPOCREF1OBJ");
_LIT(KRef2Name, "EPOCREF2OBJ");
_LIT(KRef3Name, "EPOCREF3OBJ");
_LIT(KRef4Name, "EPOCREF4OBJ");
_LIT(KRef5Name, "EPOCREF5OBJ");
_LIT(KRef6Name, "EPOCREF6OBJ");
_LIT(KRef7Name, "EPOCREF7OBJ");
_LIT(KRef8Name, "EPOCREF8OBJ");
_LIT(KRef9Name, "EPOCREF9OBJ");

_LIT8(KRefTarget, "\xF9\xEC\x7B\xC4\x95\x3C\x11\xd2\x98\x4E\x52\x54\x00\xDC\x9E\x09"); // Folder Browsing

const TInt KMaxNumOfTests = 1;

static const TUid KObexTestUid = {0x12342468};

enum TestMode
	{
	E_Inactive,
	E_Server,
	E_Server_File,
	E_Client,
	E_Client_Connect_Menu,
	E_Client_Setup_Menu,
	E_Client_Authentication_Header_Menu,
	E_SdpQuery,
	E_Auto,
	E_Client_HTTP_Menu,
	E_GetIrDANickname,
	E_EnterRecvMTU,
	E_EnterTransmitMTU,
#ifdef EMPTY_HEADERS_TEST
	E_Empty_Headers_Accept_Test,
#endif //EMPTY_HEADERS_TEST
	};

enum TTransport
	{
	EBluetooth,
	EIrda,
	EUsb,
	
	//Place holder should a Win32 emulator USB transport be created
	EWin32Usb
	};

#endif // _TOBEXCONSTANTS_H
