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

#ifndef CMTPGETPARTIALOBJECT_H
#define CMTPGETPARTIALOBJECT_H

#include "cmtprequestprocessor.h"

class CMTPTypeFile;

/** 
Defines file data provider GetObject request processor
@internalComponent
*/
class CMTPGetPartialObject : public CMTPRequestProcessor
    {
public:

	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
	IMPORT_C ~CMTPGetPartialObject();

private: // From CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();
    TMTPResponseCode CheckRequestL();
    
private: 
   
    CMTPGetPartialObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

        
private: // Owned

    CMTPTypeFile*       iFileObject;
    CMTPObjectMetaData* iObjectInfo;    
    TUint               iOffset;
    TUint               iLength;

    };
    
#endif // CMTPGETPARTIALOBJECT_H

