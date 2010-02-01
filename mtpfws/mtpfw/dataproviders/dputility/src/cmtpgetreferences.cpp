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

#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpgetreferences.h"

/**
Verification data for the GetReferences request
*/
const TMTPRequestElementInfo KMTPGetReferencesPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 0, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/    
EXPORT_C MMTPRequestProcessor* CMTPGetReferences::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetReferences* self = new (ELeave) CMTPGetReferences(aFramework, aConnection);
    return self;
    }

/**
Destructor
*/    
EXPORT_C CMTPGetReferences::~CMTPGetReferences()
    {
    delete iReferences;
    }

/**
Standard c++ constructor
*/    
CMTPGetReferences::CMTPGetReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetReferencesPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetReferencesPolicy)
    {
    }

/**
GetReferences request handler
*/    
void CMTPGetReferences::ServiceL()
    {
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    MMTPReferenceMgr& referenceMgr = iFramework.ReferenceMgr();
    delete iReferences;
    iReferences = NULL;
    iReferences = referenceMgr.ReferencesLC(TMTPTypeUint32(objectHandle));
    CleanupStack::Pop(iReferences);
    SendDataL(*iReferences);
    }


