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


#ifndef CMTPPICTBRIDGEENUMERATOR_H
#define CMTPPICTBRIDGEENUMERATOR_H

#include <e32base.h>
#include <f32file.h>
#include <d32dbms.h>

#include <comms-infras/commsdebugutility.h>
#include "mtppictbridgedpconst.h"
#include "rmtpframework.h"

class MMTPDataProviderFramework;
class MMTPObjectMgr;
class CMTPObjectMetaData;
class MMTPPictBridgeEnumeratorCallback;

/** 
Defines file enumerator.  Enumerates all files/directories under a specified path or storage

*/
class CMTPPictBridgeEnumerator : public CBase
    {
public:
    static CMTPPictBridgeEnumerator* NewL(MMTPDataProviderFramework& aFramework, MMTPPictBridgeEnumeratorCallback& aCallback);
    ~CMTPPictBridgeEnumerator();

public:
    void EnumerateObjectsL(TUint32 aStorageId);
    void EnumerateStoragesL();
    TUint32 DeviceDiscoveryHandle() const;
    
private:
    CMTPPictBridgeEnumerator(MMTPDataProviderFramework& aFramework, MMTPPictBridgeEnumeratorCallback& aCallback);
    void ConstructL();

private: 
    // Owned
    MMTPDataProviderFramework&              iFramework;
    MMTPPictBridgeEnumeratorCallback&       iCallback;
    TUint32                                 iDpsDiscoveryHandle;
    RMTPFramework       iSingletons;
    };

#endif // CMTPPICTBRIDGEENUMERATOR_H
