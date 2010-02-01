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

#ifndef CMTPIMAGEDPGETPARTIALOBJECT_H
#define CMTPIMAGEDPGETPARTIALOBJECT_H

#include "cmtprequestprocessor.h"

class MMTPObjectMgr;
class RFs;
class CMTPTypePartialFile;
class CMTPImageDataProvider;
class CMTPImageDpObjectPropertyMgr;
/** 
Defines file data provider GetObject request processor

@internalTechnology
*/
class CMTPImageDpGetPartialObject : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
    ~CMTPImageDpGetPartialObject();	
    
private:	
    CMTPImageDpGetPartialObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider);
    void ConstructL();

private:	//from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();
    virtual TBool DoHandleResponsePhaseL();

private:
    //helper
    TBool VerifyParametersL();
    void BuildPartialDataL();
    
private:
    /**
    FLOGGER debug trace member variable.
     */
    __FLOG_DECLARATION_MEMBER_MUTABLE;

    RFs&                  iFs;
    TUint32               iOffset;
    TUint32               iMaxLength;
    TUint32               iPartialDataLength;
    CMTPTypeFile*         iFileObject;
    CMTPObjectMetaData*   iObjectMeta;
    };
    
#endif  //CMTPIMAGEDPGETPARTIALOBJECT_H
