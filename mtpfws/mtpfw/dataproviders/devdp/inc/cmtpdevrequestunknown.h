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

#ifndef CMTPDEVREQUESTUNKNOWN_H
#define CMTPDEVREQUESTUNKNOWN_H

#include <mtp/cmtptypeinterdependentpropdesc.h>
#include "cmtprequestunknown.h"
#include "rmtpframework.h"

/** 
Implements the device data provider default request processor. MTP requests 
which cannot be dispatched to a suitable data provider are routed to the 
device data provider.
@internalComponent
*/
class CMTPDevRequestUnknown : public CMTPRequestUnknown
    {
public:
    
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPDevRequestUnknown();

private: // From CMTPRequestProcessor

    void ServiceL();
    void ServiceInterdependentPropDescL();

private:    
    
    CMTPDevRequestUnknown(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
	
private:
	
	CMTPTypeInterdependentPropDesc* iDataSet;
    RMTPFramework iSingletons;
    };
    
#endif // CMTPDEVREQUESTUNKNOWN_H

