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
* Description:  This class creates and parses dps operations. 
*
*/


#include <e32base.h>
#include <e32debug.h>
#include <pathinfo.h>
#include <s32file.h>
#include "dpstransaction.h"
#include "dpsscriptsender.h"
#include "dpsdefs.h"
#include "dpsxmlparser.h"
#include "dpsxmlgenerator.h"
#include "dpsconst.h"
#include "dpsfile.h"
#include "dpsstatemachine.h"
#include "dpsscriptreceiver.h"
#include "pictbridge.h"
#include <xml/parser.h>
#include "dpsparam.h"
#include "dpsoperation.h"
#include "dpsxmlstring.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TInt KPathLength = 3;
const TInt KPercentagePosition = 3;
const TInt KPercentage = 100;
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsTransaction* CDpsTransaction::NewL(CDpsStateMachine* aOperator)
    {
    IF_DEBUG(Print(_L("CDpsTransaction::NewL")));
    CDpsTransaction* self = new(ELeave) CDpsTransaction(aOperator);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsTransaction::CDpsTransaction(CDpsStateMachine* aOperator) :
    iOperator(aOperator), iReply(EFalse)
	{
    IF_DEBUG(Print(_L("CDpsTransaction::Ctor")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsTransaction::ConstructL()
    {
    IF_DEBUG(Print(_L(">>>CDpsTransaction::ConstructL")));	
    iFile = CDpsFile::NewL();
    iXmlGen = CDpsXmlGenerator::NewL(iOperator->DpsEngine());
    iXmlPar = CDpsXmlParser::NewL(iOperator->DpsEngine());
    IF_DEBUG(Print(_L("<<<CDpsTransaction::ConstructL")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsTransaction::~CDpsTransaction()
	{
    IF_DEBUG(Print(_L(">>>~CDpsTransaction")));
    delete iXmlPar;
    iXmlPar = NULL;
	
    delete iXmlGen;
    iXmlGen = NULL;
	
    delete iFile;
    iFile = NULL;
	
    IF_DEBUG(Print(_L("<<<~CDpsTransaction")));
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::CreateRequestL(TMDpsOperation* aOperation)
	{
    IF_DEBUG(Print(_L(">>>CDpsTransaction::CreateRequest ")));
    IF_DEBUG(Print(_L("  the request is %d"), aOperation->iOperation));
    TDpsArgArray args;
    TDpsEleArray elements; 
    TDpsAttribute attrib = 0;
    CleanupClosePushL(args);
    CleanupClosePushL(elements);
    RFileWriteStream writer;
    writer.PushL();
    TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
    defaultPath.Append(KDpsDeviceRequestFileName);
    User::LeaveIfError(writer.Replace(iFile->FileSession(), defaultPath, 
                                      EFileWrite));
    IF_DEBUG(Print(_L("*** file created  ")));                                  
    User::LeaveIfError(aOperation->FillReqArgs(args, elements, attrib, this));
    aOperation->CreateReqScriptL(args, elements, attrib, writer, this);    
    writer.CommitL();
    writer.Pop();
    writer.Release();
    User::LeaveIfError(iOperator->ScriptSender()->SendScript(EFalse));
    CleanupStack::PopAndDestroy(&elements);
    CleanupStack::PopAndDestroy(&args);
    IF_DEBUG(Print(_L("<<<CDpsTransaction::CreateRequest "))); 
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::Filter(TDes8& aScript)
    {    
    TInt size = aScript.Size();
    IF_DEBUG(Print(_L(">>>CDpsTransaction::Filter size %d"), size));
    
    for (TInt i = 0; i < size; )
        {
        // removes any unprintalbe char between two XML attributes, e.g. 
        // between > and <            
        if (aScript[i] >= KSOH && aScript[i] <= KSpace && aScript[i -1] == '>')    
            {
            aScript.Delete(i, 1);
            size--;
            }
        else
            {
            i++;
            }
        }
    IF_DEBUG(Print(_L("<<<CDpsTransaction::Filter size %d"), size));
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsTransaction::ParseScriptL(TBool aReply)
    {
    IF_DEBUG(Print(_L(">>>CDpsTransaction::ParseScript")));
    iReply = aReply;
    TInt size;
    iFile->FileSizeL(iOperator->ScriptReceiver()->FileNameAndPath(), size);
    HBufC8* script = HBufC8::NewLC(size);
    TPtr8 ptr_script = script->Des();
    iFile->GetContentL(iOperator->ScriptReceiver()->FileNameAndPath(), 
                       ptr_script);
    Filter(ptr_script);    
	iXmlPar->Reset();
    CParser* parser = CParser::NewLC(KDpsXmlMimeType, *iXmlPar);
    Xml::ParseL(*parser, ptr_script);
    TDpsArgArray args; iXmlPar->GetParameters(args);
    
#ifdef _DEBUG
    //print what we get now
    TBuf<KMaxArgLen> print;
    for (TInt i = 0; i < args.Count(); i++)
        {
        print.Copy(args[i].iContent);
        IF_DEBUG(Print(_L("element %d content %S"), 
            args[i].iElement, &print));
        }
#endif
   
    if (aReply)
        {
        TMDpsOperation* op = iOperator->MOperation();
        User::LeaveIfError(op->FillRepArgs(args, this));
        iXmlPar->SetOperationResult(op->iResult);    
        }  
    else 
        {
        TDpsResult result; 
   	    result.iMajorCode = EDpsResultOk; 
        result.iMinorCode = EDpsResultNone;       			
        if (iXmlPar->IsEvent())
            {
            TDpsEvent event = iXmlPar->Event();
            iOperator->SetEvent(event);
            if (event == EDpsEvtNotifyJobStatus)
                {
                User::LeaveIfError(iOperator->DpsEngine()->Event()->
                    iJobEvent.FillRepArgs(args, this));
                }
            else
                {
                User::LeaveIfError(iOperator->DpsEngine()->Event()->
                    iPrinterEvent.FillRepArgs(args, this));
                }
            CreateEventReplyL(event, result);    
            }
        else
            {
            // the request from the host is only this one:
            // GetFileID and used by DPOF printing 
            TDpsOperation ope = iXmlPar->Operation();
            iOperator->SetOperation(ope);
            if (iOperator->Operation() != EDpsOpGetFileID)
                {
                User::Leave(KErrNotSupported);
                }    		    
            CreateRequestReplyL(args, result);    
            }	
        }
    CleanupStack::PopAndDestroy(parser);
    CleanupStack::PopAndDestroy(script);
    IF_DEBUG(Print(_L("<<<CDpsTransaction::ParseScript")));	
    }
        
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::HandleHostRequestError(TInt aErr)
    {
    IF_DEBUG(Print(_L(">>>CDpsTransaction::HandleHostRequestError %d"), aErr));
    TDpsResult result; 
    // here we need to map the aErr to Dps standard error
    switch (aErr)
        {
        case KErrNotSupported: 
            result.iMajorCode = EDpsResultNotRecognized; 
            result.iMinorCode = EDpsResultNone;         
        break;
        
        case KErrArgument:
            result.iMajorCode = EDpsResultNotSupported; 
            result.iMinorCode = EDpsResultillegalParam;         
        break;
        
        default:
            IF_DEBUG(Print(_L("unknown err")));
            return;    
        }
    TRAP_IGNORE(CreateEventReplyL(iXmlPar->Event(), result));
    
    IF_DEBUG(Print(_L("<<<CDpsTransaction::HandleHostRequestError")));    
    return;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsTransaction::ConvertVersion(TLex8& aParser, TDpsVersion& aVersion)
    {
    aParser.Mark();
    while (!aParser.Eos())
        {
        TChar c = aParser.Peek();
        if (!c.IsDigit())
            {
            break;
            }
        aParser.Inc();    
        }
    TPtrC8 token = aParser.MarkedToken();
    TLex8 converter(token);    
    TInt error = converter.Val(aVersion.iMajor);
                        
    if (error != KErrNone)
        {
         IF_DEBUG(Print(_L("\t convert error 1")));
         return error;
        }
    IF_DEBUG(Print(_L("verion major %d"), aVersion.iMajor));
    aParser.Inc();
    TPtrC8 tokenDe = aParser.Remainder();
    converter.Assign(tokenDe);
    error = converter.Val(aVersion.iMinor);
    if (error != KErrNone)
        {
        IF_DEBUG(Print(_L("\t convert error 2")));
        return error;
        }
    IF_DEBUG(Print(_L("verion minor %d"), aVersion.iMinor));        
    return KErrNone;
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
TInt CDpsTransaction::ParsePercentage(const TDes8& aPer)  
    {
    TLex8 parser(aPer);
    parser.Mark();
    parser.Inc(KPercentagePosition);
    TPtrC8 digital = parser.MarkedToken();
    TLex8 converter(digital);
    TInt num1; 
    TInt error = converter.Val(num1);
    if (error != KErrNone)
        {
        return error;
        }
    
    parser.Inc(1);
    parser.Mark();
    parser.Inc(KPercentagePosition);
    digital.Set(parser.MarkedToken());
    converter.Assign(digital);
    TInt num2; 
    error = converter.Val(num2);    
    if (error != KErrNone)
        {
        return error;
        }
    
    TReal per = (TReal)num1 / (TReal)num2 * KPercentage;
    return (TInt)per;
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsTransaction::CreateEventReplyL(TDpsEvent aEvent, 
                                        const TDpsResult& aResult)
                                   
	{
	IF_DEBUG(Print(_L(">>>CDpsTransaction::CreateReply")));
	IF_DEBUG(Print(_L
	    ("  the operation reply is %d"), iOperator->Operation()));
	IF_DEBUG(Print(_L("\t the event reply is %d"), iOperator->Event()));
    RFileWriteStream writer;
    writer.PushL();
    TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
    defaultPath.Append(KDpsDeviceResponseFileName);
    User::LeaveIfError(writer.Replace(iFile->FileSession(), defaultPath, 
                                      EFileWrite));
    IF_DEBUG(Print(_L("*** file created  ")));     
    iXmlGen->CreateResultScriptL(aEvent, writer, aResult);
    writer.CommitL();
    writer.Pop();
    writer.Release();
	User::LeaveIfError(iOperator->ScriptSender()->SendScript(ETrue));
    IF_DEBUG(Print(_L("<<<CDpsTransaction::CreateReply")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsTransaction::CreateRequestReplyL(const TDpsArgArray& aArgs, 
                                          const TDpsResult& aResult)
    {
    IF_DEBUG(Print(_L(">>>CDpsTransaction::CreateRequestReply")));
    TInt count = aArgs.Count();
    TInt basePathId;
    TBuf8<KMaxArgLen> filePath;
    TLex8 converter;
    
    for (TInt i = 0; i < count; i++)
        {
        switch (aArgs[i].iElement)
            {
            case EDpsArgBasePathID:
                converter.Assign(aArgs[i].iContent);
                User::LeaveIfError(converter.Val(basePathId));
            break;
            
            case EDpsArgFilePath:
                filePath.Copy(aArgs[i].iContent);
            break;
            
            default:
                __IF_DEBUG(Print(_L("***wrong args")));
                User::Leave(KErrArgument);
            break;
            }
        TUint32 fileId;
        
        SubstitutePath(filePath);
        TBuf<KMaxArgLen> file;  
        file.Copy(filePath);
        User::LeaveIfError(iOperator->DpsEngine()->
            Ptp().GetObjectHandleByName(file, fileId));
        TDpsArg arg;
        arg.iElement = EDpsArgFileID;
        arg.iContent.AppendNumFixedWidth(fileId, EHex, KFullWordWidth);
        RFileWriteStream writer;
        writer.PushL();
        TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
        defaultPath.Append(KDpsDeviceResponseFileName);
        User::LeaveIfError(writer.Replace(iFile->FileSession(), defaultPath, 
                                          EFileWrite));
        IF_DEBUG(Print(_L("*** file created  ")));        
        iXmlGen->CreateReplyScriptL(EDpsOpGetFileID, writer, aResult, arg);   
	    User::LeaveIfError(iOperator->ScriptSender()->SendScript(ETrue));
	    writer.CommitL();
        writer.Pop();
        writer.Release();    
        } 
    IF_DEBUG(Print(_L("<<<CDpsTransaction::CreateRequestReply")));
    }
    
// ---------------------------------------------------------------------------
// The aPath is not the full file path, at least the
// driver letter is not included. The structure of the DPOF filePath is
// ../path/childpath/imagePrinted.jpg and GetObjectHandleByName
// requires the full path file as e:\images\image1.jpg
// the basePathId is not useful because it is always the fileId
// of AUTPRINT.MRK. But since DPOF is always used for the removable
// media, we assume that images are only stored in removable media.
// If the assumption is true (must be true), we need here first to
// get the removable drive (hopefully only one). Then substitute
// the / by the \ in the filePath. Finally, insert the removable drive
// letter and : at the beginning of the filePath. The new full path
// file can be used by GetObjectHandleByName
// ---------------------------------------------------------------------------
//        
void CDpsTransaction::SubstitutePath(TDes8& aPath)
    {
    IF_DEBUG(Print(_L(">>>CDpsTransaction::SubstitutePath %S"), &aPath));
    TInt size = aPath.Size();
    for (TInt i = 0; i < size; i++)
        {
        if (aPath[i] == KSlash)
            {
            aPath[i] = KBackSlash;
            }
        }
    TBuf<KPathLength> driveEWide = PathInfo::MemoryCardRootPath();
    TBuf8<KPathLength> driveENarrow;
    driveENarrow.Copy(driveEWide);    
    aPath.Replace(0, KPathLength - 1, driveENarrow);
    IF_DEBUG(Print(_L("<<<CDpsTransaction::SubstitutePath %S"), &aPath));
    }
