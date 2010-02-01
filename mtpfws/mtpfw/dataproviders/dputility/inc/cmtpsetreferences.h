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
 @internalTechnology
*/

#ifndef CMTPSETREFERENCES_H
#define CMTPSETREFERENCES_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;


/** 
Defines generic file system object SetObjectReferences request processor.
@internalTechnology
*/
class CMTPSetReferences : public CMTPRequestProcessor
    {
public:

    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPSetReferences();    
    
private:    

    CMTPSetReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);

private: // From CMTPRequestProcessor

    virtual void ServiceL();
    virtual TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;
    
private:

    TBool VerifyReferenceHandlesL() const;    
    
private:

    CMTPTypeArray*  iReferences;
    };
    

#endif // CMTPSETREFERENCES_H

