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


#ifndef CMTPGETSERVICEIDS_H_
#define CMTPGETSERVICEIDS_H_


#include "cmtprequestprocessor.h"
#include "f32file.h"
#include "rmtpframework.h"

class CMTPTypeArray;

/** 
Implements the device data provider's GetServiceIds request processor
@internalComponent
*/
class CMTPGetServiceIds : public CMTPRequestProcessor
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetServiceIds();    
    
private:    

	CMTPGetServiceIds(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor

    void ServiceL();

private:

    void BuildServiceIdsL();
    
private: // Owned

    RMTPFramework   iFrameworkSingletons;
    CMTPTypeArray*  iServiceIds;
    };

#endif /*CMTPGETSERVICEIDS_H_*/
