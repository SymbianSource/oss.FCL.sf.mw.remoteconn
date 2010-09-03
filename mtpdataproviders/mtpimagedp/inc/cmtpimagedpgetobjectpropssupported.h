// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPIMAGEDPGETOBJECTPROPSSUPPORTED_H
#define CMTPIMAGEDPGETOBJECTPROPSSUPPORTED_H

#include "cmtprequestprocessor.h"

class CMTPTypeArray;
class CMTPImageDataProvider;

/** 
Defines file data provider GetObjectPropsSupported request processor

@internalTechnology
*/
class CMTPImageDpGetObjectPropsSupported : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
    ~CMTPImageDpGetObjectPropsSupported();	
    
private:	
    CMTPImageDpGetObjectPropsSupported(
                    MMTPDataProviderFramework& aFramework,
                    MMTPConnection& aConnection);
    void ConstructL();

private:	//from CMTPRequestProcessor
    virtual void ServiceL();
    
private:
    CMTPTypeArray*			iObjectPropsSupported;
    };
    
#endif  //CMTPIMAGEDPGETOBJECTPROPSSUPPORTED_H

