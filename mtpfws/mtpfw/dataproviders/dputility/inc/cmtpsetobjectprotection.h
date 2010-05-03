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

#ifndef __CMTPSETOBJECTPROTECTION_H__
#define __CMTPSETOBJECTPROTECTION_H__

#include <e32std.h> 
#include <mtp/cmtpobjectmetadata.h>
#include "cmtprequestprocessor.h"
#include "rmtpframework.h"
#include "rmtpdpsingletons.h"

class CMTPTypeString;
class TMTPTypeUint8;


/** 
Defines file data provider SetObjectPropValue request processor

@internalTechnology
*/
class CMTPSetObjectProtection : public CMTPRequestProcessor
    {
public:
    IMPORT_C static MMTPRequestProcessor* NewL(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection);   
    IMPORT_C ~CMTPSetObjectProtection(); 
    
private:    
    CMTPSetObjectProtection(
                    MMTPDataProviderFramework& aFramework,
                    MMTPConnection& aConnection);
    void ConstructL();

private:    //from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();

private:
    RFs&                    iRfs;
    CMTPObjectMetaData*     iObjMeta;
    RMTPFramework           iSingletons;
    };
    
#endif

