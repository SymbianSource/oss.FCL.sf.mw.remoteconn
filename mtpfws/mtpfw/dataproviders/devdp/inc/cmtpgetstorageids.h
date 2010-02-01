// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPGETSTORAGEIDS_H
#define CMTPGETSTORAGEIDS_H

#include "cmtprequestprocessor.h"
#include "f32file.h"
#include "rmtpframework.h"

class CMTPTypeArray;

/** 
Implements the device data provider's GetStorageIds request processor
@internalComponent
*/
class CMTPGetStorageIds : public CMTPRequestProcessor
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetStorageIds();    
    
private:    

    CMTPGetStorageIds(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor

    void ServiceL();

private:

    void BuildStorageIdsL();
    
private: // Owned

    RMTPFramework   iFrameworkSingletons;
    CMTPTypeArray*  iStorageIds;
    };
    
#endif // CMTPGETSTORAGEIDS_H

