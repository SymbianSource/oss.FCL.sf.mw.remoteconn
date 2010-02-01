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
 @internalTechnology
*/

#ifndef CMTPCOMMONREQUESTPROCESSOR_H
#define CMTPCOMMONREQUESTPROCESSOR_H

#include <mtp/cmtptypetrivialdata.h>
#include "cmtprequestprocessor.h"


/** 
Defines generic file system object CMTPCommonRequestProcessor request processor.
@internalTechnology
*/
class CMTPCommonRequestProcessor : public CMTPRequestProcessor
	{
public:

    template<TMTPResponseCode RESPCODE, TBool HASITORDATA>
	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, TMTPResponseCode aResponseCode, TBool aHasIToRDataPhase);	
	IMPORT_C ~CMTPCommonRequestProcessor();	
	
private:
	
    CMTPCommonRequestProcessor(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, TMTPResponseCode aResponseCode, TBool aHasIToRDataPhase);
    void ConstructL();

private: // From CMTPRequestProcessor
    virtual void ServiceL();
    virtual TBool DoHandleResponsePhaseL();
    virtual TBool HasDataphase() const; 
    
private:
    TMTPResponseCode                iResponseCode;
    TBool                           iHasIToRDataPhase;	
    CMTPTypeTrivialData*            iIToRData;
	};


template<TMTPResponseCode RESPCODE, TBool HASITORDATA>
    MMTPRequestProcessor* CMTPCommonRequestProcessor::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    return CMTPCommonRequestProcessor::NewL(aFramework, aConnection, RESPCODE, HASITORDATA);
    }
    
	
#endif // CMTPCOMMONREQUESTPROCESSOR_H

