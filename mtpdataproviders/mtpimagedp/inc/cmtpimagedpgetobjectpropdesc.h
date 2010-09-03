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

#ifndef CMTPIMAGEDPGETOBJECTPROPDESC_H
#define CMTPIMAGEDPGETOBJECTPROPDESC_H

#include "cmtprequestprocessor.h"

class CMTPImageDpObjectPropertyMgr;
class CMTPTypeObjectPropDesc;
class CMTPImageDataProvider;

/** 
Defines file data provider GetObjectPropDesc request processor

*/
class CMTPImageDpGetObjectPropDesc : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
    ~CMTPImageDpGetObjectPropDesc();	
    
private:	
    CMTPImageDpGetObjectPropDesc(
                    MMTPDataProviderFramework& aFramework,
                    MMTPConnection& aConnection);
    void ConstructL();

private:	//from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();	

private:
    //helper
    void ServiceStorageIdL();
    void ServiceObjectFormatL();
    void ServiceProtectionStatusL();
    void ServiceObjectSizeL();
    void ServiceFileNameL();
    void ServiceDateCreatedL();
    void ServiceDateModifiedL();
    void ServiceParentObjectL();
    void ServicePuidL();
    void ServiceNameL();
    void ServiceWidthL();
    void ServiceHeightL();
    void ServiceImageBitDepthL();
    void ServiceRepresentativeSampleFormatL();
    void ServiceRepresentativeSampleSizeL();
    void ServiceRepresentativeSampleHeightL();
    void ServiceRepresentativeSampleWidthL();
    void ServiceRepresentativeSampleDataL();
    void ServiceNonConsumableL();
    void ServiceHiddenL();
    TUint16 GetPropertyGroupNumber(const TUint16 aPropCode) const;
    
private:
    CMTPTypeObjectPropDesc*	iObjectProperty;
    };
    
#endif //CMTPIMAGEDPGETOBJECTPROPDESC_H

