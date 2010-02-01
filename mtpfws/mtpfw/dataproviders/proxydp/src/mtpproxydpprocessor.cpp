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

#include <mtp/tmtptyperequest.h>

#include "cmtpcopyobject.h"
#include "cmtpmoveobject.h"
#include "cmtpdeleteobject.h"
#include "cmtpgetobjectproplist.h"
#include "cmtpgetobjectpropssupported.h"
#include "cmtprequestprocessor.h"
#include "cmtprequestunknown.h"
#include "cmtpsendobjectinfo.h"
#include "cmtpsendobjectproplist.h"
#include "cmtpsetobjectproplist.h"
#include "mtpproxydpprocessor.h"
#include "cmtpgetformatcapabilities.h"
#include "cmtpdeleteobjectproplist.h"
/**
proxy data provider mapping table from request ID to factory method of the request processor
*/
static const TMTPRequestProcessorEntry KMTPRequestProcessorTable[] = 
    {
        {EMTPOpCodeCopyObject,              CMTPCopyObject::NewL},
        {EMTPOpCodeMoveObject,              CMTPMoveObject::NewL},
        {EMTPOpCodeDeleteObject,            CMTPDeleteObject::NewL},
        {EMTPOpCodeGetObjectPropList,       CMTPGetObjectPropList::NewL},
        {EMTPOpCodeGetObjectPropsSupported, CMTPGetObjectPropsSupported::NewL},
        {EMTPOpCodeSendObjectInfo,          CMTPSendObjectInfo::NewL},
        {EMTPOpCodeSendObjectPropList,      CMTPSendObjectPropList::NewL},
        {EMTPOpCodeSetObjectPropList,       CMTPSetObjectPropList::NewL},
        {EMTPOpCodeGetFormatCapabilities,   CMTPGetFormatCapabilities::NewL},
        {EMTPOpCodeDeleteObjectPropList,    CMTPDeleteObjectPropList::NewL}
   
    };

/**
Creates request processor based on the request and connection
*/    
MMTPRequestProcessor* MTPProxyDpProcessor::CreateL(MMTPDataProviderFramework& aFramework, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    TMTPRequestProcessorCreateFunc factory(NULL); 
    TUint16 op(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
    const TInt count(sizeof(KMTPRequestProcessorTable) / sizeof(TMTPRequestProcessorEntry));
    for (TInt i(0); ((!factory) && (i < count)); i++)
        {
        if (KMTPRequestProcessorTable[i].iOperationCode == op)
            {
            factory = KMTPRequestProcessorTable[i].iCreateFunc;
            }
        }

    if (!factory)    
        {
        factory = CMTPRequestUnknown::NewL;
        }
                
    return (*factory)(aFramework, aConnection);
    }
