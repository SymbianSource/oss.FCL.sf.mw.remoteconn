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

#ifndef MTPPICTBRIDGEGETOBJECTPROPDESC_H_
#define MTPPICTBRIDGEGETOBJECTPROPDESC_H_

#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"

class CMTPTypeObjectPropDesc;

/** 
Defines pictbridge data provider GetObjectPropDesc request processor
@internalTechnology
*/
class CMTPPictBridgeGetObjectPropDesc : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider );
    ~CMTPPictBridgeGetObjectPropDesc();
    
private:
    CMTPPictBridgeGetObjectPropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();

private:    //from CMTPRequestProcessor
    void ServiceL();    

private:
    //helper
    void ServiceStorageIdL();
    void ServiceObjectFormatL();
    void ServiceProtectionStatusL();
    void ServiceObjectSizeL();
    void ServiceFileNameL();
    void ServiceDateModifiedL();
    void ServiceParentObjectL();
    void ServicePuidL();
    void ServiceNameL();
    void ServiceNonConsumableL();
    TUint16 GetPropertyGroupNumber(const TUint16 aPropCode) const;
    
private:
    CMTPTypeObjectPropDesc* iObjectProperty;
    CMTPPictBridgeDataProvider& iPictBridgeDP;
    };
    
#endif /* MTPPICTBRIDGEGETOBJECTPROPDESC_H_ */

