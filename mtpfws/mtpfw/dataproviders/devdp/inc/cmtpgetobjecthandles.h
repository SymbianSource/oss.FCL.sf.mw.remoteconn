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

#ifndef CMTPGETOBJECTHANLES_H
#define CMTPGETOBJECTHANLES_H

#include "cmtpgetnumobjects.h"

class CMTPTypeArray;

/** 
Implements device data provider GetObjectHandles request processor.
@internalComponent
*/
class CMTPGetObjectHandles : public CMTPGetNumObjects
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetObjectHandles();    
    
private:    

    CMTPGetObjectHandles(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor

    void ServiceL();
        
private:

    CMTPTypeArray* iHandles;
    };
    
#endif // CMTPGETOBJECTHANLES_H

