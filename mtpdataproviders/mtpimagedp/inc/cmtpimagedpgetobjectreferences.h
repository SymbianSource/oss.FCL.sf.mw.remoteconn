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

#ifndef CMTPIMAGEDPGETOBJECTREFERENCES_H
#define CMTPIMAGEDPGETOBJECTREFERENCES_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;
class CMTPImageDataProvider;

/** 
Defines generic file system object GetObjectReferences request processor.
@internalTechnology
*/
class CMTPImageDpGetObjectReferences : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider); 
    ~CMTPImageDpGetObjectReferences();  
    
private:    
    CMTPImageDpGetObjectReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
private: // From CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();    
    
private:
    
    CMTPTypeArray*          iReferences;
    };
    
#endif // CMTPIMAGEDPGETOBJECTREFERENCES_H

