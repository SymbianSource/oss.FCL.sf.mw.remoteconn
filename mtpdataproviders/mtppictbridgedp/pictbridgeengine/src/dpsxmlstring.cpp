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
* Description:  This class implements the dps constant strings. 
*
*/


#include "dpsxmlstring.h"
#include "dpsdefs.h"
#include "dpsconst.h"
#include <e32debug.h>

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsXmlString* TDpsXmlString::NewL()
    {
    TDpsXmlString* self = new (ELeave) TDpsXmlString();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsXmlString::TDpsXmlString() 
    {
    iDpsOperationStrings = TDpsStrings(EDpsOpMax - 1);
    iDpsElementStrings = TDpsStrings(KDpsEleMax - 1);
    iDpsEventStrings = TDpsStrings(KDpsEvtMax - 1);
    iDpsArgStrings = TDpsStrings(EDpsArgMax);
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsXmlString::ConstructL()
    {
    // define Print Service Operations
    // ref: DPS spec page 14
    for (TInt i = 1; i < EDpsOpMax; i++)
        {
        switch (i)
            {
            case EDpsOpConfigPrintService:
                iDpsOperationStrings.AppendL(_L8("configurePrintService"));           
            break;
            
            case EDpsOpGetCapability:
                iDpsOperationStrings.AppendL(_L8("getCapability"));           
            break;
            
            case EDpsOpGetJobStatus:
                iDpsOperationStrings.AppendL(_L8("getJobStatus"));           
            break;
            
            case EDpsOpGetDeviceStatus:
                iDpsOperationStrings.AppendL(_L8("getDeviceStatus"));           
            break;
            
            case EDpsOpStartJob:
                iDpsOperationStrings.AppendL(_L8("startJob"));           
            break;
            
            case EDpsOpAbortJob:
                iDpsOperationStrings.AppendL(_L8("abortJob"));           
            break;
            
    // define Storage Service Operation
    // ref: DPS spec page 34
            case EDpsOpContinueJob:
                iDpsOperationStrings.AppendL(_L8("continueJob"));           
            break;
            
            case EDpsOpGetFileID:
                iDpsOperationStrings.AppendL(_L8("getFileID"));           
            break;
            
            case EDpsOpGetFileInfo:
                iDpsOperationStrings.AppendL(_L8("getFileInfo"));           
            break;
            
            case EDpsOpGetFile:
                iDpsOperationStrings.AppendL(_L8("getFile"));           
            break;
            
            case EDpsOpGetPartialFile:
                iDpsOperationStrings.AppendL(_L8("getPartialFile"));           
            break;
            
            case EDpsOpGetFileList:
                iDpsOperationStrings.AppendL(_L8("getFileList"));           
            break;
            
            case EDpsOpGetThumb:
                iDpsOperationStrings.AppendL(_L8("getThumb"));           
            break;
            
            default:
                IF_DEBUG(Print(_L("wrong in OperationString")));
                User::Leave(KErrArgument);
            break;                        
            }
        }
        
    // define the special element which includes other elements
    for (TInt i = 1; i < KDpsEleMax; i++)
        {
        switch(i)
            {
            case EDpsCapability:
                iDpsElementStrings.AppendL(_L8("capability")); 
            break;
            
            case EDpsJobConfig:
                iDpsElementStrings.AppendL(_L8("jobConfig")); 
            break;
            
            case EDpsPrintInfo:
                iDpsElementStrings.AppendL(_L8("printInfo")); 
            break;
                
            default:
                IF_DEBUG(Print(_L("wrong in ElementString")));
                User::Leave(KErrArgument);
            break;    
            }
        }
        
    // define Print Service Event Notification
    // ref: DPS spec page 14
    for (TInt i = 1; i < KDpsEvtMax; i++)
        {
        switch(i)
            {
            case EDpsEvtNotifyJobStatus:
                iDpsEventStrings.AppendL(_L8("notifyJobStatus")); 
            break;
            
            case EDpsEvtNotifyDeviceStatus:
                iDpsEventStrings.AppendL(_L8("notifyDeviceStatus")); 
            break;
               
            default:
                IF_DEBUG(Print(_L("wrong in Event String")));
                User::Leave(KErrArgument);
            break;    
            }
        }
     
    for (TInt i = 0; i < EDpsArgMax; i++)
        {
        switch (i)
            {
            case EDpsArgDpsVersions:
                iDpsArgStrings.AppendL(_L8("dpsVersions")); 
            break;
        
            case EDpsArgVendorName:
                iDpsArgStrings.AppendL(_L8("vendorName")); 
            break;
        
            case EDpsArgVendorSpecificVersion:
                iDpsArgStrings.AppendL(_L8("vendorSpecificVersion")); 
            break;
        
            case EDpsArgProductName:
                iDpsArgStrings.AppendL(_L8("productName")); 
            break;
        
            case EDpsArgSerialNo:
                iDpsArgStrings.AppendL(_L8("serialNo")); 
            break;
        
            case EDpsArgPrintServiceAvailable:
                iDpsArgStrings.AppendL(_L8("printServiceAvailable")); 
            break;
        
            case EDpsArgQualities:
                iDpsArgStrings.AppendL(_L8("qualities")); 
            break;
        
            case EDpsArgPaperSizes:
                iDpsArgStrings.AppendL(_L8("paperSizes")); 
            break;
        
            case EDpsArgPaperTypes:
                iDpsArgStrings.AppendL(_L8("paperTypes")); 
            break;
        
            case EDpsArgFileTypes:
                iDpsArgStrings.AppendL(_L8("fileTypes")); 
            break;
        
            case EDpsArgDatePrints:
                iDpsArgStrings.AppendL(_L8("datePrints")); 
            break;
        
            case EDpsArgFileNamePrints:
                iDpsArgStrings.AppendL(_L8("fileNamePrints")); 
            break;
        
            case EDpsArgImageOptimizes:
                iDpsArgStrings.AppendL(_L8("imageOptimizes")); 
            break;
        
            case EDpsArgLayouts:
                iDpsArgStrings.AppendL(_L8("layouts")); 
            break;
        
            case EDpsArgFixedSizes:
                iDpsArgStrings.AppendL(_L8("fixedSizes")); 
            break;
        
            case EDpsArgChroppings:
                iDpsArgStrings.AppendL(_L8("chroppings")); 
            break;
        
            case EDpsArgPrtPID:
                iDpsArgStrings.AppendL(_L8("prtPID")); 
            break;
        
            case EDpsArgFilePath:
                iDpsArgStrings.AppendL(_L8("filePath")); 
            break;
        
            case EDpsArgCopyID:
                iDpsArgStrings.AppendL(_L8("copyID")); 
            break;
        
            case EDpsArgProgress:
                iDpsArgStrings.AppendL(_L8("progress")); 
            break;
        
            case EDpsArgImagePrinted:
                iDpsArgStrings.AppendL(_L8("imagePrinted")); 
            break;
        
            case EDpsArgDpsPrintServiceStatus:
                iDpsArgStrings.AppendL(_L8("dpsPrintServiceStatus")); 
            break;
        
            case EDpsArgJobEndReason:
                iDpsArgStrings.AppendL(_L8("jobEndReason")); 
            break;
        
            case EDpsArgErrorStatus:
                iDpsArgStrings.AppendL(_L8("errorStatus")); 
            break;
        
            case EDpsArgErrorReason:
                iDpsArgStrings.AppendL(_L8("errorReason")); 
            break;
        
            case EDpsArgDisconnectEnable:
                iDpsArgStrings.AppendL(_L8("disconnectEnable")); 
            break;
        
            case EDpsArgCapabilityChanged:
                iDpsArgStrings.AppendL(_L8("capabilityChanged")); 
            break;
        
            case EDpsArgNewJobOk:
                iDpsArgStrings.AppendL(_L8("newJobOK")); 
            break;
        
            case EDpsArgQuality:
                iDpsArgStrings.AppendL(_L8("quality")); 
            break;
        
            case EDpsArgPaperSize:
                iDpsArgStrings.AppendL(_L8("paperSize")); 
            break;
        
            case EDpsArgPaperType:
                iDpsArgStrings.AppendL(_L8("paperType")); 
            break;
        
            case EDpsArgFileType:
                iDpsArgStrings.AppendL(_L8("fileType")); 
            break;
        
            case EDpsArgDatePrint:
                iDpsArgStrings.AppendL(_L8("datePrint")); 
            break;
        
            case EDpsArgFileNamePrint:
                iDpsArgStrings.AppendL(_L8("fileNamePrint")); 
            break;
        
            case EDpsArgImageOptimize:
                iDpsArgStrings.AppendL(_L8("imageOptimize")); 
            break;
        
            case EDpsArgLayout:
                iDpsArgStrings.AppendL(_L8("layout")); 
            break;
        
            case EDpsArgFixedSize:
                iDpsArgStrings.AppendL(_L8("fixedSize")); 
            break;
        
            case EDpsArgCropping:
                iDpsArgStrings.AppendL(_L8("cropping")); 
            break;
        
            case EDpsArgCroppingArea:
                iDpsArgStrings.AppendL(_L8("croppingArea")); 
            break;
        
            case EDpsArgFileID:
                iDpsArgStrings.AppendL(_L8("fileID")); 
            break;
        
            case EDpsArgFileName:
                iDpsArgStrings.AppendL(_L8("fileName")); 
            break;
        
            case EDpsArgDate:
                iDpsArgStrings.AppendL(_L8("date")); 
            break;
        
            case EDpsArgCopies:
                iDpsArgStrings.AppendL(_L8("copies")); 
            break;
        
            case EDpsArgAbortStyle:
                iDpsArgStrings.AppendL(_L8("abortStyle")); 
            break;
        
            case EDpsArgImagesPrinted:
                iDpsArgStrings.AppendL(_L8("imagesPrinted")); 
            break;
        
            case EDpsArgBasePathID:
                iDpsArgStrings.AppendL(_L8("basePathID")); 
            break;
        
            case EDpsArgFileSize:
                iDpsArgStrings.AppendL(_L8("fileSize")); 
            break;
        
            case EDpsArgThumbFormat:
                iDpsArgStrings.AppendL(_L8("thumbFormat")); 
            break;
        
            case EDpsArgThumbSize:
                iDpsArgStrings.AppendL(_L8("thumbSize")); 
            break;
 
            case EDpsArgBytesRead:
                iDpsArgStrings.AppendL(_L8("bytesRead")); 
            break;
        
            case EDpsArgOffset:
                iDpsArgStrings.AppendL(_L8("offset")); 
            break;
        
            case EDpsArgMaxSize:
                iDpsArgStrings.AppendL(_L8("maxSize")); 
            break;
        
            case EDpsArgParentFileID:
                iDpsArgStrings.AppendL(_L8("parentFileID")); 
            break;
        
            case EDpsArgMaxNumIDs:
                iDpsArgStrings.AppendL(_L8("maxNumIDs")); 
            break;
        
            case EDpsArgFileIDs:
                iDpsArgStrings.AppendL(_L8("fileIDs")); 
            break;
        
            case EDpsArgNumIDs:
                iDpsArgStrings.AppendL(_L8("numIDs")); 
            break;
        
            default:
                IF_DEBUG(Print(_L("wrong in ArgString")));
                User::Leave(KErrArgument);
            break;
            }
        }
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsXmlString::~TDpsXmlString()
    {
    iDpsArgStrings.Close();
    iDpsElementStrings.Close();
    iDpsEventStrings.Close();
    iDpsOperationStrings.Close();
    }
