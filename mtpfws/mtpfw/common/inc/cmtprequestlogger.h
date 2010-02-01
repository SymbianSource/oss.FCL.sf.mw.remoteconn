// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalComponent
*/

#ifndef CMTPREQUESTLOGGER_H
#define CMTPREQUESTLOGGER_H

#include <e32base.h>

//#include "rmtpframework.h"
#include "mtpdebug.h"

#include <gmxmlcomposer.h>
#include <mtp/mtpdataproviderapitypes.h>

class TMTPTypeRequest;
class TMTPTypeResponse;
class CMTPXmlLogger;
	
/** 
@internalComponent
*/
class CMTPRequestLogger : public CBase, public MMDXMLComposerObserver 
    {
public:

	IMPORT_C ~CMTPRequestLogger();
	IMPORT_C static CMTPRequestLogger* NewL();
	IMPORT_C void LogRequestL(const TMTPTypeRequest& aRequest);
	IMPORT_C void LogResponseL(const TMTPTypeResponse& aResponse);
	IMPORT_C void WriteDataPhaseL(const MMTPType& aData, TMTPTransactionPhase aDataPhase);
	IMPORT_C TInt CreateOutputDir(TDes& aTestCaseFn);
    IMPORT_C void WriteTestCaseL(TDesC& aTestCaseFn, const TMTPTypeResponse& aResponse);
    
    public: // from MMDXMLComposerObserver
	IMPORT_C void ComposeFileCompleteL();
    
private:    
    CMTPRequestLogger();
    void ConstructL();
    
private:
    // We store all of the test case data upto the response phase as the XML parser requires an
    // 2nd level active scheduler to be constructed to work in this context.
    TMTPTypeRequest iCurrentRequest;
    TBuf<KMaxFileName> iCurrentRToIDataPhaseFN;
    TBuf<KMaxFileName> iCurrentIToRDataPhaseFN;
    
    TInt iCurrentTestCaseId;
    TBuf<KMaxFileName> iCurrentXMLTestCase;
    TBuf<KMaxFileName> iCurrentTestOutputDir;
    RFs iFs;
    RFile iCurrentTestOutput;
    CActiveScheduler* s;
    CMTPXmlLogger* iXMLLogger;
    //CActiveSchedulerWait iWait;
    CActiveScheduler* iScheduler;
    CActiveScheduler* iCurrent;
    TInt iCurrentSessionID;
    __FLOG_DECLARATION_MEMBER;
    };

#endif
