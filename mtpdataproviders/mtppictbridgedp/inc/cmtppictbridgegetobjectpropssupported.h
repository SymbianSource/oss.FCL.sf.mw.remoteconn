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
 @internalComponent
 */

#ifndef CMTPPICTBRIDGEGETOBJECTPROPSSUPPORTED_H_
#define CMTPPICTBRIDGEGETOBJECTPROPSSUPPORTED_H_

#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"

class CMTPTypeArray;

/** 
Defines pictbridge data provider GetObjectPropsSupported request processor

@internalTechnology
*/
class CMTPPictBridgeGetObjectPropsSupported : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider );
    ~CMTPPictBridgeGetObjectPropsSupported();
    
private:
    CMTPPictBridgeGetObjectPropsSupported( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider );
    void ConstructL();
    
private:    //from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();
    
private:
    CMTPPictBridgeDataProvider& iPictBridgeDP;
    CMTPTypeArray*          iObjectPropsSupported;
    };

#endif /* CMTPPICTBRIDGEGETOBJECTPROPSSUPPORTED_H_ */
