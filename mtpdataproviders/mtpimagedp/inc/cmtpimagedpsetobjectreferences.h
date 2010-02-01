// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPIMAGEDPSETOBJECTREFERENCES_H
#define CMTPIMAGEDPSETOBJECTREFERENCES_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;
class CMTPImageDataProvider;

/** 
Defines generic file system object GetObjectReferences request processor.
@internalTechnology
*/
class CMTPImageDpSetObjectReferences : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider); 
    ~CMTPImageDpSetObjectReferences();  
    
private:    
    CMTPImageDpSetObjectReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
private: // From CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();   
    TBool HasDataphase() const;
    TBool DoHandleResponsePhaseL();
    
private:
    TBool VerifyReferenceHandlesL() const;
    
private:
    /**
    FLOGGER debug trace member variable.
    **/
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMTPTypeArray*          iReferences;
    };
    
#endif // CMTPIMAGEDPSETOBJECTREFERENCES_H

