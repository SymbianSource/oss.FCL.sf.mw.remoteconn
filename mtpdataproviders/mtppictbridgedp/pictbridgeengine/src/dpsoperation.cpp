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
* Description:  These classes implement dps operations replies. 
*
*/


#include <e32debug.h>
#include "dpsoperation.h"
#include "dpsconst.h"
#include "dpstransaction.h"
#include "pictbridge.h"
#include "dpsxmlparser.h"
#include "dpsxmlgenerator.h"
#include "dpsstatemachine.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TInt KConfigPrintService = 5;
const TInt KCopyFileWidth = 3;
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C void TMDpsOperation::CreateReqScriptL(
                                      const TDpsArgArray& aArguments, 
                                      const TDpsEleArray& /*aElements*/,
                                      TDpsAttribute /*aAttribute*/, 
                                      RWriteStream& aScript, 
                                      CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TMDpsOperation::CreateReqScript")));        
    CDpsXmlGenerator* g = aTrader->Generator();
    g->StartDocumentL(aScript);
    g->StartInputL(aScript);
    TInt count = aArguments.Count();
    if (!count)
	    {
		g->StartOperationL((TDpsOperation)iOperation, aScript, ETrue);
		}
	else
		{
		g->StartOperationL((TDpsOperation)iOperation, aScript);
		for (TInt i = 0; i < count; i++)
			{
			g->CreateArgL(aScript, aArguments[i]);
			}
		g->EndOperationL((TDpsOperation)iOperation, aScript);	
		}
			
    g->EndInputL(aScript);
	g->EndDocumentL(aScript);	
	IF_DEBUG(Print(_L("<<<TMDpsOperation::CreateReqScript")));        	
    }
    
// ---------------------------------------------------------------------------
// Some request parameters should be checked
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TDpsStartJob::FillReqArgs(TDpsArgArray& aArgs, 
                                        TDpsEleArray& aElements,
                                        TDpsAttribute& /*aAttrib*/, 
                                        CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsStartJob::FillReqArgs")));                    
    TInt imageCount = iReqParam.iPrintInfo.Count();
    if (!imageCount) 
        {
        return KErrUnknown;
        }
    TUint32* objectHandles = new TUint32[imageCount];
    if (!objectHandles)
        {
        return KErrNoMemory;
        }
    for (TInt i = 0; i < imageCount; i++)
        {
        TInt err = aTrader->Engine()->Ptp().
             GetObjectHandleByName(
                 iReqParam.iPrintInfo[i].iFile, objectHandles[i], ETrue); 
        if (err != KErrNone)
            {
            IF_DEBUG(Print(_L("---error %d"), err));
            
            delete[] objectHandles;
            return err;
            }
        }
    TDpsArg* argsP = new TDpsArg[iReqParam.GetParamNum()];
    if (!argsP)
        {
		delete[] objectHandles;
        return KErrNoMemory;
        }
    TDpsEle* elemsP = new TDpsEle[1 + imageCount];
    if (!elemsP)
        {
		delete[] argsP;
		delete[] objectHandles;
        return KErrNoMemory;
        }
    // jobConfig    
    TInt count = iReqParam.iJobConfig.Count();
              
    elemsP[0].iElement = EDpsJobConfig;
    elemsP[0].iNum = count;
    aElements.Append(elemsP[0]);
    for (TInt i = 0; i < count; i++)
        {
        argsP[i].iElement = iReqParam.iJobConfig[i].iElement;
    	// the parameter only has high bytes (0x1234)
    	argsP[i].iContent.AppendNumUC(iReqParam.iJobConfig[i].iContent, EHex);
    	// we have to append the low bytes (0000)
    	argsP[i].iContent.Append(KDpsLowZero);
    	aArgs.Append(argsP[i]);
    	IF_DEBUG(Print(_L("the element is %d"), argsP[i].iElement));
    	IF_DEBUG(Print(_L("the content is %S"), &(argsP[i].iContent)));
    	}    
    	
    // printInfo
    for (TInt j = 0; j < imageCount; j++)
        {
    	// there is at least one argument for fileID
    	TInt k = 1;
    	elemsP[j + 1].iElement = EDpsPrintInfo;
    	argsP[count].iElement = EDpsArgFileID;
    	argsP[count].iContent.AppendNumFixedWidth(objectHandles[j], EHex, 
    	                                          KFullWordWidth);
    	aArgs.Append(argsP[count]);
    
    	IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
    	count++;
    	
    	// not empty
    	if (iReqParam.iPrintInfo[j].iFileName)
    	    {
    	    argsP[count].iElement = EDpsArgFileName;
    	    argsP[count].iContent.Copy(iReqParam.iPrintInfo[j].iFile);
    	    aArgs.Append(argsP[count]);
    	    
    	    IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	    IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
    	    
    	    k++; count++;
    	    }
    	// not empty    
    	if (iReqParam.iPrintInfo[j].iDate.Size())
    	    {
            argsP[count].iElement = EDpsArgDate;
            argsP[count].iContent.Copy(iReqParam.iPrintInfo[j].iDate);
            aArgs.Append(argsP[count]);
            IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	    IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
            k++; count++;
    	    }
    	if (iReqParam.iPrintInfo[j].iCopies != 0)
    	    {
            argsP[count].iElement = EDpsArgCopies;
            argsP[count].iContent.AppendNumFixedWidthUC(
                iReqParam.iPrintInfo[j].iCopies, EDecimal, KCopyFileWidth);
            aArgs.Append(argsP[count]);
            IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	    IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
    	    k++; count++;
    	    }    
    	if (iReqParam.iPrintInfo[j].iPrtPID != 0)
    	    {
            argsP[count].iElement = EDpsArgPrtPID;
            argsP[count].iContent.AppendNumUC(iReqParam.iPrintInfo[j].iPrtPID);
            aArgs.Append(argsP[count]);
            IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	    IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
    	    k++; count++;
    	    }
    	if (iReqParam.iPrintInfo[j].iCopyID != 0)
    	    {
            argsP[count].iElement = EDpsArgCopyID;
            argsP[count].iContent.AppendNumUC(iReqParam.iPrintInfo[j].iCopyID);
            aArgs.Append(argsP[count]);
            IF_DEBUG(Print(_L("the element is %d"), argsP[count].iElement));
    	    IF_DEBUG(Print(_L("the content is %S"), &(argsP[count].iContent)));
    	    k++; count++;
    	    }
        
    	elemsP[j + 1].iNum = k;
    	aElements.Append(elemsP[j + 1]);    
    	}
    	        
    delete[] objectHandles;	
    delete[] argsP;
    delete[] elemsP;
    IF_DEBUG(Print(_L("<<<TDpsStartJob::FillReqArgs")));                        
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
// 
EXPORT_C void TDpsStartJob::CreateReqScriptL(const TDpsArgArray& aArguments, 
                                            const TDpsEleArray& aElements,
                                            TDpsAttribute /*aAttribute*/,  
                                            RWriteStream& aScript, 
                                            CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsStartJob::CreateReqScript")));                        
    CDpsXmlGenerator* g = aTrader->Generator();
    g->StartDocumentL(aScript);
    g->StartInputL(aScript);
    g->StartOperationL((TDpsOperation)iOperation, aScript);
    TInt count = aElements.Count();
    TInt limit = 0;
    for (TInt i = 0; i < count; i++)
	    {
		g->StartElementL(aElements[i].iElement, aScript);
		TInt init = limit;
		limit = (i == 0 ? aElements[0].iNum : limit + aElements[i].iNum);
		for (TInt j = init; j < limit; j++)
			{
			g->CreateArgL(aScript,aArguments[j]);
			}
		g->EndElementL(aElements[i].iElement, aScript);	
		}
	g->EndOperationL((TDpsOperation)iOperation, aScript);
	g->EndInputL(aScript);
	g->EndDocumentL(aScript);	
	IF_DEBUG(Print(_L("<<<TDpsStartJob::CreateReqScript")));                        	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsAbortJob::FillReqArgs(TDpsArgArray& aArgs, 
                                        TDpsEleArray& /*aElems*/,
                                        TDpsAttribute& /*aAttrib*/, 
                                        CDpsTransaction* /*aParam*/)
    {
    IF_DEBUG(Print(_L(">>>TDpsAbortJob::FillReqArgs")));                            
    TDpsArg arg;
    arg.iElement = EDpsArgAbortStyle;
    arg.iContent.AppendNumUC(iReqParam.iAbortStyle, EHex);
    arg.iContent.Append(KDpsLowZero);
    aArgs.Append(arg);
    IF_DEBUG(Print(_L("<<<TDpsAbortJob::FillReqArgs")));                            
    return KErrNone;
    }
    
// ---------------------------------------------------------------------------
// Some request parameters should be checked
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsGetCapability::FillReqArgs(TDpsArgArray& aArgs, 
                                             TDpsEleArray& aElements,
                                             TDpsAttribute& aAttrib, 
                                             CDpsTransaction* /*aParam*/)
    {
    IF_DEBUG(Print(_L(">>>TDpsGetCapability::FillReqArgs")));                                
    // only one element
    TDpsEle elems;
    elems.iElement = EDpsCapability;
    elems.iNum = 1;
    aElements.Append(elems);
    
    // only one parameter
    TDpsArg argsP;
    argsP.iElement = iReqParam.iCap;
    aArgs.Append(argsP);
    if (EDpsArgPaperTypes == iReqParam.iCap || EDpsArgLayouts == iReqParam.iCap)
        {
        if (iReqParam.iAttribute != 0)
            {
            aAttrib = iReqParam.iAttribute;
            }
        }
    IF_DEBUG(Print(_L("<<<TDpsGetCapability::FillReqArgs")));                                    
    return KErrNone;    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
EXPORT_C void TDpsGetCapability::CreateReqScriptL(
                                             const TDpsArgArray& aArguments, 
                                             const TDpsEleArray& aElements,
                                             TDpsAttribute aAttribute, 
                                             RWriteStream& aScript, 
                                             CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsGetCapability::CreateReqScript")));    
    CDpsXmlGenerator* g = aTrader->Generator();
    g->StartDocumentL(aScript);
    g->StartInputL(aScript);
    g->StartOperationL((TDpsOperation)iOperation, aScript);
	g->StartElementL(aElements[0].iElement, aScript);
	const TInt count = aArguments.Count();
	for (TInt i = 0; i < count; i++)
		{
		g->CreateArgL(aScript, aArguments[i], aAttribute);
		}
	g->EndElementL(aElements[0].iElement, aScript);
	g->EndOperationL((TDpsOperation)iOperation, aScript);	
	g->EndInputL(aScript);
	g->EndDocumentL(aScript);
	IF_DEBUG(Print(_L("<<<TDpsGetCapability::CreateReqScript")));    		
    }
        
// ---------------------------------------------------------------------------
// Some request parameters should be checked
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsConfigPrintService::FillReqArgs(TDpsArgArray& aArgs, 
                                                  TDpsEleArray& /*aElems*/,
                                                  TDpsAttribute& /*aAttrib*/, 
                                                  CDpsTransaction* /*aParam*/)
    {
    IF_DEBUG(Print(_L(">>>TDpsConfigPrintService::FillReqArgs")));
    TDpsArg* argsP = new TDpsArg[KConfigPrintService]; 
    if (!argsP)
        {
        return KErrNoMemory;
        }
    TInt count;     
    for (TInt i = 0; i < KConfigPrintService; i++)
        {
        switch (i)
            {
        // there might be several versions
        case EDpsArgDpsVersions:
            argsP[i].iElement = EDpsArgDpsVersions;
            count = iReqParam.iDpsVersions.Count();
            for (TInt j = 0; j < count; j++)
                {
                argsP[i].iContent.AppendNumUC
                    (iReqParam.iDpsVersions[j].iMajor);
                argsP[i].iContent.Append(_L("."));    
                argsP[i].iContent.AppendNumUC
                    (iReqParam.iDpsVersions[j].iMinor);    
                }
            aArgs.Append(argsP[i]);
        break;    
        
        case EDpsArgVendorName: // vender name
            
            argsP[i].iElement = EDpsArgVendorName;
            argsP[i].iContent.Append(iReqParam.iVendorName);
            aArgs.Append(argsP[i]);
                        
        break;  
        
        case EDpsArgVendorSpecificVersion: // vender version (optional)
            if (iReqParam.iVendorVersion.iMajor && 
                iReqParam.iVendorVersion.iMinor)
                {
                argsP[i].iElement = EDpsArgVendorSpecificVersion;
                argsP[i].iContent.AppendNumUC(iReqParam.iVendorVersion.iMajor);
                argsP[i].iContent.Append(_L("."));    
                argsP[i].iContent.AppendNumUC(iReqParam.iVendorVersion.iMinor);
                aArgs.Append(argsP[i]);        
                }
            
        break;
        
        case EDpsArgProductName: // produce name
            argsP[i].iElement = EDpsArgProductName;
            argsP[i].iContent.Append(iReqParam.iProductName);
            aArgs.Append(argsP[i]);
        break;
        
        case EDpsArgSerialNo: // serialNo (optional)
            if (iReqParam.iSerialNo.Length())
                {
                argsP[i].iElement = EDpsArgSerialNo;
                argsP[i].iContent.Append(iReqParam.iSerialNo);    
                aArgs.Append(argsP[i]);
                }
            
        break;
        
        default:
            IF_DEBUG(Print(_L("***Wrong parameter")));
            delete[] argsP;
            return KErrArgument;
            }
        
        }
    delete[] argsP;
    IF_DEBUG(Print(_L("<<<TDpsConfigPrintService::FillReqArgs")));    
    return KErrNone;    
    }

// ---------------------------------------------------------------------------
//  Not all parameters are checked. If the invalid parameter can pass the 
//  the complaint test, it should not be checked. Instead, the client will see
//  it as unknown error. Otherwise, there might be unnecessary code and the
//  client cannot be informed. 
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsGetCapability::FillRepArgs(const TDpsArgArray& aArgs, 
                                             CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsGetCapability::FillRepArgs")));    
    CDpsXmlParser* XmlPar = aTrader->Parser();
    
    if (aArgs.Count())
        {
        if (EDpsArgPaperTypes == aArgs[0].iElement || 
            EDpsArgLayouts == aArgs[0].iElement)
            {
            if (XmlPar->HasAttribute())
                {
                TDpsAttribute attrib;
		        XmlPar->GetAttribute(attrib);
		        iRepParam.iContent.Append(attrib);            
                }
            else
                {
                iRepParam.iContent.Append(0);
                }    
            }
        TUint32 value;
        TLex8 converter;
        TLex8 parser(aArgs[0].iContent);
        parser.SkipSpace();
        while (!parser.Eos())
            {
            parser.Mark();
            parser.SkipCharacters();
            if (KFullWordWidth == parser.TokenLength())
                {
                TPtrC8 token = parser.MarkedToken();
                converter.Assign(token);
                parser.SkipSpace();  
                }
            else
                {
                IF_DEBUG(Print(_L("***Wrong argument")));
                return KErrArgument;
                }
            TInt error = converter.Val(value, EHex);
            if (error != KErrNone)
                {
                IF_DEBUG(Print(_L("convert error %d"), error));
                return error;
                }
            if (EDpsArgPaperTypes == aArgs[0].iElement)
                {
                TDpsPaperType paperType;
                TInt major = value >> KShiftLength;
                paperType.iMajor = (TDpsPaperTypeMajor)major;
                paperType.iMinor = (TDpsPaperTypeMinor)(value & KDpsMinorMask);
                iRepParam.iPaperType.Append(paperType);
                }
            else
                {
                // remove the extra zeros
                value = value >> KShiftLength; 
                iRepParam.iContent.Append(value);
                IF_DEBUG(Print(_L("the value is %x"), value));
                }
            
            }
        iRepParam.iCap = aArgs[0].iElement;
           
        }
    
    IF_DEBUG(Print(_L("<<<TDpsGetCapability::FillRepArgs")));
    return KErrNone;    
    }

// ---------------------------------------------------------------------------
//  Not all parameters are checked. If the invalid parameter can pass the 
//  the complaint test, it should not be checked. Instead, the client will see
//  it as unknown error. Otherwise, there might be unnecessary code and the
//  client cannot be informed. 
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsConfigPrintService::FillRepArgs(const TDpsArgArray& aArgs, 
                                                  CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsConfigPrintService::FillRepArgs")));    
    TInt error = KErrNone;
    TLex8 converter, parser;
    const TInt count = aArgs.Count();
    
    for (TInt i = 0; i < count; i++)
        {
        switch (aArgs[i].iElement)
            {
            case EDpsArgPrintServiceAvailable: 
                converter.Assign(aArgs[i].iContent);
                TInt64 result;
                error = converter.Val(result, EHex);
                if (error != KErrNone)
                    {
                    IF_DEBUG(Print(_L("*** convert error")));
                    return error;
                    }
                // removes the low four bytes zeros. 
                IF_DEBUG(Print(_L("--Printer available is %x"), result));   
                result = result >> KShiftLength;    
                iRepParam.iPrintAvailable = result;
            break;
        
            case EDpsArgDpsVersions:
                TDpsVersion version;
            
                parser.Assign(aArgs[i].iContent);
                while (!parser.Eos())
                    {
                    parser.Mark();
                    parser.SkipCharacters();
                    if (parser.TokenLength())
                        {
                        TPtrC8 token = parser.MarkedToken();
                        converter.Assign(token);
                        parser.SkipSpace();
                        }
                    else
                        {
                        IF_DEBUG(Print(_L("***wrong !!!")));
                        return KErrArgument;
                        }
                    error = aTrader->ConvertVersion(converter, version);
                    if (error != KErrNone)
                        {
                        return error;
                        }
                    iRepParam.iDpsVersions.Append(version);            
                    }
            break;
        
            case EDpsArgVendorName:
                iRepParam.iVendorName.Copy(aArgs[i].iContent);
                IF_DEBUG(Print(_L
                                ("vendor name is %S"), &iRepParam.iVendorName));
            break;
        
            case EDpsArgVendorSpecificVersion:
                parser.Assign(aArgs[i].iContent);
                aTrader->ConvertVersion(parser, version);
                iRepParam.iVendorVersion = version;
                IF_DEBUG(Print(_L
                        ("vendor version is %x"), iRepParam.iVendorVersion));
            break;
       
            case EDpsArgProductName:
                iRepParam.iProductName.Copy(aArgs[i].iContent);
                IF_DEBUG(Print(_L
                        ("product name is %S"), &iRepParam.iProductName));
            break;
        
            case EDpsArgSerialNo:
                iRepParam.iSerialNo.Copy(aArgs[i].iContent);
                IF_DEBUG(Print(_L("SerialNo is %S"), &iRepParam.iSerialNo));
            break;
        
            default:
                IF_DEBUG(Print(_L("--Unknown param!!")));
                return KErrArgument;
            }
        }
        
    IF_DEBUG(Print(_L("<<<TDpsConfigPrintService::FillRepArgs")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
//  Not all parameters are checked. If the invalid parameter can pass the 
//  the complaint test, it should not be checked. Instead, the client will see
//  it as unknown error. Otherwise, there might be unnecessary code and the
//  client cannot be informed. 
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TDpsGetJobStatus::FillRepArgs(const TDpsArgArray& aArgs, 
                                            CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsGetJobStatus::FillRepArgs")));
    TInt value, error, per;
    TBuf<KMaxArgLen> fileName;
    TLex8 converter;    
    TInt count = aArgs.Count();
    TBool reply = aTrader->IsReply();
    if (!reply)
        {
        aTrader->Engine()->Event()->iEvent = EDpsEvtNotifyJobStatus;
        }
    else
        {
        aTrader->Engine()->Event()->iEvent = EDpsEvtEmpty;
        }
    for (TInt i = 0; i < count; i++)
        {
        switch (aArgs[i].iElement)
            {
            case EDpsArgPrtPID:
                converter.Assign(aArgs[i].iContent);
                error = converter.Val(value);
                if (error != KErrNone)
                    {
                    return error;
                    }
                if (reply)
                    {
                    iRepParam.iPrtPID = value;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iJobEvent.iRepParam.iPrtPID = 
                        value;
                    }
            break;
                
            case EDpsArgFilePath:
                if (reply)
                    {
                    iRepParam.iFilePath.Copy(aArgs[i].iContent);
                    }
                else
                    {
                    aTrader->Engine()->Event()->iJobEvent.iRepParam.iFilePath. 
                    Copy(aArgs[i].iContent);
                    }
            break;
                
            case EDpsArgCopyID:
                converter.Assign(aArgs[i].iContent);
                error = converter.Val(value);
                if (error != KErrNone)
                    {
                     return error;
                    }
                if (reply)
                    {
                    iRepParam.iCopyID = value;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iJobEvent.iRepParam.iCopyID = 
                        value;
                    }    
            break;
                
            case EDpsArgProgress:
                per = aTrader->ParsePercentage(aArgs[i].iContent);
                if (per < KErrNone)
                    {
                    return per;
                    }
                
                if (reply)
                    {
                    iRepParam.iProgress = per;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iJobEvent.iRepParam.iProgress = 
                        per;
                    }
            break;
               
            case EDpsArgImagesPrinted:
                converter.Assign(aArgs[i].iContent);
                error = converter.Val(value);
                if (error != KErrNone)
                    {
                     return error;
                    }
                if (reply)
                    {
                    iRepParam.iImagesPrinted = value;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iJobEvent.iRepParam.
                        iImagesPrinted = value;
                    }                                                                  
            break;
            
            default:
                IF_DEBUG(Print(_L("***wrong param!!!")));
                return KErrArgument;                
            }
        }
    IF_DEBUG(Print(_L("<<<TDpsGetJobStatus::FillRepArgs")));    
    return KErrNone;    
    }

// ---------------------------------------------------------------------------
//  Not all parameters are checked. If the invalid parameter can pass the 
//  the complaint test, it should not be checked. Instead, the client will see
//  it as unknown error. Otherwise, there might be unnecessary code and the
//  client cannot be informed. 
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt TDpsGetPrinterStatus::FillRepArgs(const TDpsArgArray& aArgs, 
                                                CDpsTransaction* aTrader)
    {
    IF_DEBUG(Print(_L(">>>TDpsGetPrinterStatus::FillRepArgs")));
    // if UI has not pass the event pointer, we do need to fill in it
    if (!aTrader->Engine()->Event())
        {
        return KErrNone;
        }
    TLex8 converter;
    TDpsPrinterStatusRep param;
    const TInt count = aArgs.Count();
    TUint32 value, temp;
    TBool reply = aTrader->IsReply();
    if (!reply)
        {
        aTrader->Engine()->Event()->iEvent = EDpsEvtNotifyDeviceStatus;
        }
    else
        {
        aTrader->Engine()->Event()->iEvent = EDpsEvtEmpty;
        }
    for (TInt i = 0; i < count; i++)
        {
        converter.Assign(aArgs[i].iContent);
        TInt error = converter.Val(value, EHex);
        if (error != KErrNone)
            {
            return error;
            }
        IF_DEBUG(Print(_L("--the value is %x"), value));
        
        switch (aArgs[i].iElement)
            {
            case EDpsArgDpsPrintServiceStatus:
                value = value >> KShiftLength;
                if (EDpsPrintServiceStatusPrinting == value ||
                    EDpsPrintServiceStatusIdle == value ||
                    EDpsPrintServiceStatusPaused == value)
                    {
                    if (reply)
                        {
                        iRepParam.iPrintStatus = 
                            (TDpsPrintServiceStatus)(value);
                        }
                    else
                        {
                        aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                            iPrintStatus = (TDpsPrintServiceStatus)(value);
                        }
                    }
                else
                    {
                    return KErrArgument;
                    }
                    
            break;
            
            case EDpsArgJobEndReason:
                value = value >> KShiftLength;
                if (EDpsJobStatusNotEnded == value ||
                    EDpsJobStatusEndedOk == value ||
                    EDpsJobStatusEndedAbortImmediately == value ||
                    EDpsJobStatusEndedAbortCompleteCurrent == value ||
                    EDpsJobStatusEndedOther == value)
                    {
                    if (reply)
                        {
                        iRepParam.iJobStatus = (TDpsJobStatus)(value);    
                        }
                    else
                        {
                        aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                            iJobStatus = (TDpsJobStatus)(value);    
                        }    
                    }
                else
                    {
                    return KErrArgument;
                    }
                
            break;
            
            case EDpsArgErrorStatus:
                value = value >> KShiftLength;
                if (EDpsErrorStatusOk == value ||
                    EDpsErrorStatusWarning == value ||
                    EDpsErrorStatusFatal == value)
                    {
                    if (reply)
                        {
                        iRepParam.iErrorStatus = (TDpsErrorStatus)(value);    
                        }
                    else
                        {
                        aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                            iErrorStatus = (TDpsErrorStatus)(value);
                        }    
                    }
                else
                    {
                    return KErrArgument;
                    }
                
            break;
            
            case EDpsArgErrorReason:
                temp = value >> KShiftLength;
                if (EDpsJobErrorNone == temp ||
                    EDpsJobErrorPaper == temp ||
                    EDpsJobErrorInk == temp ||
                    EDpsJobErrorHardware == temp ||
                    EDpsJobErrorFile == temp)
                    {
                    param.iJobEndReason.iMajor = 
                        (TDpsJobEndReasonMajor)(temp);
                    IF_DEBUG(Print(_L("the end reason is %x"), temp));
                    value = value & KDpsMinorMask;
                    switch (param.iJobEndReason.iMajor)
                        {
                        case EDpsJobErrorPaper:
                            param.iJobEndReason.iPaperMinor = 
                                (TDpsJobEndReasonPaper)(value);
                        break;
        
                        case EDpsJobErrorInk:
                            param.iJobEndReason.iInkMinor = 
                                (TDpsJobEndReasonInk)(value);
                        break;
        
                        case EDpsJobErrorHardware:
                            param.iJobEndReason.iHardMinor = 
                                (TDpsJobEndReasonHard)(value);
                        break;
        
                        case EDpsJobErrorFile:
                            param.iJobEndReason.iFileMinor = 
                                (TDpsJobEndReasonFile)(value);
                        break;
        
                        default:
                            IF_DEBUG(Print(_L("no minor error")));
                        break; 
                        }
                    if (reply)
                        {
                        iRepParam.iJobEndReason = param.iJobEndReason;
                        }
                    else
                        {
                        aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                            iJobEndReason = param.iJobEndReason;
                        }
                    }
                else
                    {
                    return KErrArgument;
                    }
                
            break;
            
            case EDpsArgDisconnectEnable:
                value = value >> KShiftLength;
                if (EDpsDisconnectEnableFalse == value)
                    {
                    param.iDisconnectEnable = EFalse;    
                    }
                else if (EDpsDisconnectEnableTrue == value)
                    {
                    param.iDisconnectEnable = ETrue;
                    }
                else
                    {
                    return KErrArgument;
                    }
                if (reply)
                    {
                    iRepParam.iDisconnectEnable = param.iDisconnectEnable; 
                    }
                else
                    {
                    aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                        iDisconnectEnable = param.iDisconnectEnable; 
                    }    
            break;
            
            case EDpsArgCapabilityChanged:
                value = value >> KShiftLength;
                if (EDpsCapabilityChangedFalse == value)
                    {
                    param.iCapabilityChange = EFalse;    
                    }
                else if (EDpsCapabilityChangedTrue == value)
                    {
                    param.iCapabilityChange = ETrue;
                    }
                else
                    {
                    return KErrArgument;
                    }
                if (reply)
                    {
                    iRepParam.iCapabilityChange = param.iCapabilityChange;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                        iCapabilityChange = param.iCapabilityChange;
                    }
            break;
            
            case EDpsArgNewJobOk:
                value = value >> KShiftLength;
                if (EDpsNewJobOkTrue == value)
                    {
                    param.iNewJobOk = ETrue;    
                    }
                else if (EDpsNewJobOkFalse == value)
                    {
                    param.iNewJobOk = EFalse;   
                    }
                else
                    {
                    return KErrArgument;
                    }
                if (reply)
                    {
                    iRepParam.iNewJobOk = param.iNewJobOk;
                    }
                else
                    {
                    aTrader->Engine()->Event()->iPrinterEvent.iRepParam.
                        iNewJobOk = param.iNewJobOk;
                    }    
            break;
            
            default:
                IF_DEBUG(Print(_L("***wrong param")));
                return KErrArgument;
            }
        }
 
    IF_DEBUG(Print(_L("<<<TDpsGetPrinterStatus::FillRepArgs")));    
    return KErrNone;    
	}
