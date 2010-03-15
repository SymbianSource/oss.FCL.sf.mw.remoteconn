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

#ifndef CMTPDELETEOBJECT_H
#define CMTPDELETEOBJECT_H

#include <badesca.h>
#include <f32file.h>

#include "rmtpframework.h"
#include "cmtprequestprocessor.h"
#include "mtpdebug.h"

class MMTPObjectMgr;

/** 
Defines data provider GetObject request processor.
@internalComponent
*/
class CMTPDeleteObject : public CMTPRequestProcessor, public MFileManObserver
    {
public:

    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPDeleteObject();    

private: // From CMTPRequestProcessor

    TMTPResponseCode CheckRequestL();
    void ServiceL();
 
private:    
    void ConstructL();
    CMTPDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ProcessFinalPhaseL();
	TBool IsFolderObject(const CMTPObjectMetaData& aObject);
	TBool IsStoreReadOnlyL(TUint32 aObjectHandle);
	void DeleteFolderOrFileL(CMTPObjectMetaData* aMeta);
    void DeleteFolderL(CMTPObjectMetaData* aMeta);
    void DeleteFileL(CMTPObjectMetaData* aMeta);    
	
private:    

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
	
	/**
    True if any deletion succeeded
	*/
    TBool					iSuccessDeletion;
    TBool 					iObjectWritePotected;
    RMTPFramework           iSingletons;
	};
	
#endif

