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

#ifndef CMTPIMAGEDPSETOBJECTPROTECTION_H
#define CMTPIMAGEDPSETOBJECTPROTECTION_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;
class CMTPImageDataProvider;

/** 
Defines generic file system object GetObjectReferences request processor.
@internalTechnology
*/
class CMTPImageDpSetObjectProtection : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider); 
    ~CMTPImageDpSetObjectProtection();  
    
private:    
    CMTPImageDpSetObjectProtection(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
private: // From CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();   
    TBool DoHandleResponsePhaseL();
       
private:
    TMTPTypeUint16          iMTPTypeUint16;
    RFs&                    iRfs;
    CMTPObjectMetaData*     iObjMeta;
    };
    
#endif // CMTPIMAGEDPSETOBJECTPROTECTION_H

