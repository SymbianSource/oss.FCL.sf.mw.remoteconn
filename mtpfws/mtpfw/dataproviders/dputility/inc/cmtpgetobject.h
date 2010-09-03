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

#ifndef CMTPGETOBJECT_H
#define CMTPGETOBJECT_H

#include "cmtprequestprocessor.h"

class CMTPTypeFile;

/** 
Defines file data provider GetObject request processor
@internalComponent
*/
class CMTPGetObject : public CMTPRequestProcessor
    {
public:

	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
	IMPORT_C ~CMTPGetObject();    

private: // From CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();
    
private: 
   
    CMTPGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

    void BuildFileObjectL(const TDesC& aFileName);
        
private: // Owned

    CMTPTypeFile*       iFileObject;
    TMTPResponseCode    iError;
    };
    
#endif // CMTPGETOBJECT_H

