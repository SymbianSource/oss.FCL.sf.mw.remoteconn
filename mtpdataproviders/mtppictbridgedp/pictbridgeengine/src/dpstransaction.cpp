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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpstransactionTraces.h"
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
    OstTraceFunctionEntry0( CDPSTRANSACTION_CDPSTRANSACTION_CONS_ENTRY );
	OstTraceFunctionExit0( CDPSTRANSACTION_CDPSTRANSACTION_CONS_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsTransaction::ConstructL()
    {
    OstTraceFunctionEntry0( CDPSTRANSACTION_CONSTRUCTL_ENTRY );
    iFile = CDpsFile::NewL();
    iXmlGen = CDpsXmlGenerator::NewL(iOperator->DpsEngine());
    iXmlPar = CDpsXmlParser::NewL(iOperator->DpsEngine());
    OstTraceFunctionExit0( CDPSTRANSACTION_CONSTRUCTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsTransaction::~CDpsTransaction()
	{
    OstTraceFunctionEntry0( DUP1_CDPSTRANSACTION_CDPSTRANSACTION_DES_ENTRY );
    delete iXmlPar;
    iXmlPar = NULL;
	
    delete iXmlGen;
    iXmlGen = NULL;
	
    delete iFile;
    iFile = NULL;

    OstTraceFunctionExit0( DUP1_CDPSTRANSACTION_CDPSTRANSACTION_DES_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::CreateRequestL(TMDpsOperation* aOperation)
	{
    OstTraceFunctionEntry0( CDPSTRANSACTION_CREATEREQUESTL_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSTRANSACTION_CREATEREQUESTL, "  the request is %d", aOperation->iOperation );
    TDpsArgArray args;
    TDpsEleArray elements; 
    TDpsAttribute attrib = 0;
    CleanupClosePushL(args);
    CleanupClosePushL(elements);
    RFileWriteStream writer;
    writer.PushL();
    TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
    defaultPath.Append(KDpsDeviceRequestFileName);
    LEAVEIFERROR(writer.Replace(iFile->FileSession(), defaultPath, EFileWrite),
            OstTraceExt2( TRACE_ERROR, DUP2_CDPSTRANSACTION_CREATEREQUESTL, 
                    "Creates %S for stream failed ! error code %d", defaultPath, munged_err ));
    OstTrace0( TRACE_NORMAL, DUP1_CDPSTRANSACTION_CREATEREQUESTL, "*** file created  " );
    LEAVEIFERROR(aOperation->FillReqArgs(args, elements, attrib, this),
            OstTrace1( TRACE_ERROR, DUP3_CDPSTRANSACTION_CREATEREQUESTL, "FillReqArgs failed! error code %d", munged_err ));
    aOperation->CreateReqScriptL(args, elements, attrib, writer, this);    
    writer.CommitL();
    writer.Pop();
    writer.Release();
    LEAVEIFERROR(iOperator->ScriptSender()->SendScript(EFalse),
            OstTrace1( TRACE_ERROR, DUP4_CDPSTRANSACTION_CREATEREQUESTL, "SendScript failed! error code %d", munged_err ));
    CleanupStack::PopAndDestroy(&elements);
    CleanupStack::PopAndDestroy(&args);
	OstTraceFunctionExit0( CDPSTRANSACTION_CREATEREQUESTL_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::Filter(TDes8& aScript)
    {    
    OstTraceFunctionEntry0( CDPSTRANSACTION_FILTER_ENTRY );
    TInt size = aScript.Size();
    OstTrace1( TRACE_NORMAL, CDPSTRANSACTION_FILTER, "size %d", size);

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
    OstTrace1( TRACE_NORMAL, DUP1_CDPSTRANSACTION_FILTER, "size %d", size);
    OstTraceFunctionExit0( CDPSTRANSACTION_FILTER_EXIT );
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsTransaction::ParseScriptL(TBool aReply)
    {
    OstTraceFunctionEntry0( CDPSTRANSACTION_PARSESCRIPTL_ENTRY );
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
        OstTraceExt2( TRACE_NORMAL, CDPSTRANSACTION_PARSESCRIPTL, "element %d content %S", args[i].iElement, print );
        }
#endif
   
    if (aReply)
        {
        TMDpsOperation* op = iOperator->MOperation();
        LEAVEIFERROR(op->FillRepArgs(args, this),
                OstTrace1( TRACE_ERROR, DUP1_CDPSTRANSACTION_PARSESCRIPTL, 
                        "FillRepArgs failed! error code %d", munged_err ));
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
                LEAVEIFERROR(iOperator->DpsEngine()->Event()->
                    iJobEvent.FillRepArgs(args, this),
                    OstTrace1( TRACE_ERROR, DUP2_CDPSTRANSACTION_PARSESCRIPTL, 
                            "iJobEvent.FillRepArgs failed! error code %d", munged_err ));                   
                }
            else
                {
                LEAVEIFERROR(iOperator->DpsEngine()->Event()->
                    iPrinterEvent.FillRepArgs(args, this),
                    OstTrace1( TRACE_ERROR, DUP3_CDPSTRANSACTION_PARSESCRIPTL, 
                            "iPrinterEvent.FillRepArgs failed! error code %d", munged_err ));                    
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
                OstTrace1( TRACE_ERROR, DUP4_CDPSTRANSACTION_PARSESCRIPTL, 
                        "The operation %d doesn't equal to EDpsOpGetFileID", iOperator->Operation());
                User::Leave(KErrNotSupported);
                }    		    
            CreateRequestReplyL(args, result);    
            }	
        }
    CleanupStack::PopAndDestroy(parser);
    CleanupStack::PopAndDestroy(script);
    OstTraceFunctionExit0( CDPSTRANSACTION_PARSESCRIPTL_EXIT );
    }
        
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsTransaction::HandleHostRequestError(TInt aErr)
    {
    OstTraceFunctionEntry0( CDPSTRANSACTION_HANDLEHOSTREQUESTERROR_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSTRANSACTION_HANDLEHOSTREQUESTERROR, "Error %d", aErr );
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
            OstTrace0( TRACE_NORMAL, DUP1_CDPSTRANSACTION_HANDLEHOSTREQUESTERROR, "unknown err" );
            OstTraceFunctionExit0( CDPSTRANSACTION_HANDLEHOSTREQUESTERROR_EXIT );
            return;    
        }
    TRAP_IGNORE(CreateEventReplyL(iXmlPar->Event(), result));
 
    OstTraceFunctionExit0( DUP1_CDPSTRANSACTION_HANDLEHOSTREQUESTERROR_EXIT );
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
         OstTrace0( TRACE_ERROR, CDPSTRANSACTION_CONVERTVERSION, "convert error 1" );
         return error;
        }
    OstTrace1( TRACE_NORMAL, DUP1_CDPSTRANSACTION_CONVERTVERSION, "verion major %d", aVersion.iMajor );
    aParser.Inc();
    TPtrC8 tokenDe = aParser.Remainder();
    converter.Assign(tokenDe);
    error = converter.Val(aVersion.iMinor);
    if (error != KErrNone)
        {
        OstTrace0( TRACE_ERROR, DUP2_CDPSTRANSACTION_CONVERTVERSION, "convert error 2" );
        return error;
        }
    OstTrace1( TRACE_NORMAL, DUP3_CDPSTRANSACTION_CONVERTVERSION, "verion minor %d", aVersion.iMinor );
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
	OstTraceFunctionEntry0( CDPSTRANSACTION_CREATEEVENTREPLYL_ENTRY );
	OstTrace1( TRACE_NORMAL, CDPSTRANSACTION_CREATEEVENTREPLYL, "the operation reply is %d", iOperator->Operation() );
    OstTrace1( TRACE_NORMAL, DUP1_CDPSTRANSACTION_CREATEEVENTREPLYL, "the event reply is %d", iOperator->Event());
    
    RFileWriteStream writer;
    writer.PushL();
    TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
    defaultPath.Append(KDpsDeviceResponseFileName);
    LEAVEIFERROR(writer.Replace(iFile->FileSession(), defaultPath, EFileWrite),
            OstTraceExt2( TRACE_ERROR, DUP3_CDPSTRANSACTION_CREATEEVENTREPLYL, 
                    "Creates %S for stream failed ! error code %d", defaultPath, munged_err ));
    OstTrace0( TRACE_NORMAL, DUP2_CDPSTRANSACTION_CREATEEVENTREPLYL, "*** file created  " );
    
    iXmlGen->CreateResultScriptL(aEvent, writer, aResult);
    writer.CommitL();
    writer.Pop();
    writer.Release();
	LEAVEIFERROR(iOperator->ScriptSender()->SendScript(ETrue),
	        OstTrace1( TRACE_ERROR, DUP4_CDPSTRANSACTION_CREATEEVENTREPLYL, 
	                "SendScript failed! error code %d", munged_err ));
	OstTraceFunctionExit0( CDPSTRANSACTION_CREATEEVENTREPLYL_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsTransaction::CreateRequestReplyL(const TDpsArgArray& aArgs, 
                                          const TDpsResult& aResult)
    {
    OstTraceFunctionEntry0( CDPSTRANSACTION_CREATEREQUESTREPLYL_ENTRY );
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
                LEAVEIFERROR(converter.Val(basePathId),
                        OstTrace1( TRACE_ERROR, DUP2_CDPSTRANSACTION_CREATEREQUESTREPLYL, 
                                "Parses the string failed! error code %d", munged_err ));
            break;
            
            case EDpsArgFilePath:
                filePath.Copy(aArgs[i].iContent);
            break;
            
            default:
                OstTrace0( TRACE_ERROR, CDPSTRANSACTION_CREATEREQUESTREPLYL, "***wrong args" );
                User::Leave(KErrArgument);
            break;
            }
        TUint32 fileId;
        
        SubstitutePath(filePath);
        TBuf<KMaxArgLen> file;  
        file.Copy(filePath);
        (iOperator->DpsEngine()->
            Ptp().GetObjectHandleByName(file, fileId));
        TDpsArg arg;
        arg.iElement = EDpsArgFileID;
        arg.iContent.AppendNumFixedWidth(fileId, EHex, KFullWordWidth);
        RFileWriteStream writer;
        writer.PushL();
        TFileName defaultPath = iOperator->DpsEngine()->DpsFolder();
        defaultPath.Append(KDpsDeviceResponseFileName);
        (writer.Replace(iFile->FileSession(), defaultPath, 
                                          EFileWrite));
        OstTrace0( TRACE_NORMAL, DUP1_CDPSTRANSACTION_CREATEREQUESTREPLYL, "*** file created  " );
        iXmlGen->CreateReplyScriptL(EDpsOpGetFileID, writer, aResult, arg);   
	    (iOperator->ScriptSender()->SendScript(ETrue));
	    writer.CommitL();
        writer.Pop();
        writer.Release();    
        } 
    OstTraceFunctionExit0( CDPSTRANSACTION_CREATEREQUESTREPLYL_EXIT );
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
    OstTraceFunctionEntry0( CDPSTRANSACTION_SUBSTITUTEPATH_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CDPSTRANSACTION_SUBSTITUTEPATH, "path %s", aPath );

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
    OstTraceExt1( TRACE_NORMAL, DUP1_CDPSTRANSACTION_SUBSTITUTEPATH, "path %s", aPath );        
    OstTraceFunctionExit0( CDPSTRANSACTION_SUBSTITUTEPATH_EXIT );
    }
