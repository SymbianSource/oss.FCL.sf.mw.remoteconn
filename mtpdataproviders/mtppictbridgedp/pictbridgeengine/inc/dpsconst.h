/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  This class defines the dps constance. 
*
*/



#ifndef DPSCONST_H
#define DPSCONST_H
#include <e32base.h>
#include <usbpersonalityids.h>

_LIT(KDpsEnginePanicCat, "Dps Engine");
const TInt KShiftLength = 16;
const TInt KFullWordWidth = 8;

// <dps><input|output><operation|event|result></result|/event|/operation>
//	</output|/input></dps> 
enum TDpsXmlAction
    {
    EDpsXmlEmpty = 0,
    EDpsXmlStart,
    EDpsXmlInput,
    EDpsXmlOutput,
    EDpsXmlOperation,
    EDpsXmlEvent,
    EDpsXmlResult
    };    

enum TDpsOperation
    {
    EDpsOpEmpty = 0,
    EDpsOpConfigPrintService,
    EDpsOpGetCapability,
    EDpsOpGetJobStatus,
    EDpsOpGetDeviceStatus,
    EDpsOpStartJob,
    EDpsOpAbortJob,
    EDpsOpContinueJob,
    EDpsOpGetFileID,
    EDpsOpGetFileInfo,
    EDpsOpGetFile,
    EDpsOpGetPartialFile,
    EDpsOpGetFileList,
    EDpsOpGetThumb,
    EDpsOpMax
    };

_LIT8(KDpsXmlResult, "result");
_LIT8(KDpsXmlInput, "input");
_LIT8(KDpsXmlOutput, "output");
_LIT8(KDpsXmlPaperTypes, "paperTypes");
_LIT8(KDpsXmlLayouts, "layouts");
_LIT8(KDpsXmlPaperSize, "paperSize");
_LIT8(KDpsXml, "dps");

_LIT8(KDpsXmlHeader, "<?xml version=\"1.0\"?>");
_LIT8(KDpsXmlNS, "<dps xmlns=\"http://www.cipa.jp/dps/schema/\">");
_LIT8(KDpsXmlBraceOpen, "<");
_LIT8(KDpsXmlBraceClose, ">");
_LIT8(KDpsXmlSlash, "/");
_LIT8(KDpsXmlSpace, " ");
_LIT8(KDpsXmlEqual, "=");
_LIT8(KDpsXmlQuote, "\"");

const TUint KSlash = 0x2F;
const TUint KBackSlash = 0x5C;
const TUint KSOH = 0x1;
const TUint KSpace = 0x20;

const TUint32 KDpsMajorMask = 0xffff0000;
const TUint32 KDpsMinorMask = 0x0000ffff;


_LIT8(KDpsLowZero, "0000");
_LIT(KDpsScriptFile, ".DPS" );
_LIT(KDpsDeviceResponseFileName, "DRSPONSE.DPS");
_LIT(KDpsDeviceRequestFileName, "DREQUEST.DPS");
_LIT(KDpsHostResponseFileName, "HRSPONSE.DPS");
_LIT(KDpsHostRequestFileName, "HREQUEST.DPS");
_LIT8(KDpsXmlMimeType, "text/xml");

const TInt KDpsResourceVersion = 0;
_LIT(KDpsResource, "resource\\dps.rsc");


#endif // DPSDEFS_H


