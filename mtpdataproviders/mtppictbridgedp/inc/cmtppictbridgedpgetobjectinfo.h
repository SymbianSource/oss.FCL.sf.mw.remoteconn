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

#ifndef CMTPPICTBRIDGEDPGETOBJECTINFO_H
#define CMTPPICTBRIDGEDPGETOBJECTINFO_H

#include <f32file.h>

#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"

class CMTPTypeObjectInfo;

class CMTPPictBridgeDpGetObjectInfo : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);   
    ~CMTPPictBridgeDpGetObjectInfo();   
    
private:
    CMTPPictBridgeDpGetObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);   
    void ConstructL();

private: // From CMTPRequestProcessor
    void ServiceL();

private:
    void BuildObjectInfoL();
    void SetFileSizeDateL(const TDesC& aFileName, TBool aDiscoveryFile);    

private:
    CMTPPictBridgeDataProvider& iPictBridgeDP;
    CMTPTypeObjectInfo* iObjectInfoToBuildP; // owned
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    };

#endif CMTPPICTBRIDGEDPGETOBJECTINFO_H

