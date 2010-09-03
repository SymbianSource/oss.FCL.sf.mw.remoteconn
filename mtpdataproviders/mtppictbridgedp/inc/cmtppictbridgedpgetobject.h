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


#ifndef CMTPPICTBRIDGEDPGETOBJECT_H
#define CMTPPICTBRIDGEDPGETOBJECT_H

#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"

class CMTPTypeFile;

class CMTPPictBridgeDpGetObject : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);    
    ~CMTPPictBridgeDpGetObject();    

private: // From CMTPRequestProcessor
    void ServiceL();
    TBool DoHandleResponsePhaseL();

private: 
    CMTPPictBridgeDpGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();
    void BuildFileObjectL(const TDesC& aFileName);

private: // Owned
    CMTPPictBridgeDataProvider& iPictBridgeDP;
    CMTPTypeFile*    iFileObject;  // owned
    TMTPResponseCode iError;
    };
    
#endif // CMTPPICTBRIDGEDPGETOBJECT_H

