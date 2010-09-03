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

#ifndef CMTPIMAGEDPGETTHUMB_H
#define CMTPIMAGEDPGETTHUMB_H

#include "cmtprequestprocessor.h"


class CMTPImageDpThumbnailCreator;
class CMTPImageDataProvider;
class CMTPTypeOpaqueData;

/** 
Defines file data provider GetObject request processor
@internalComponent
*/

class CMTPImageDpGetThumb : public CMTPRequestProcessor
    {
public:

	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
	~CMTPImageDpGetThumb();    

private: // From CMTPRequestProcessor

    void ServiceL();
    TMTPResponseCode CheckRequestL();
    TBool DoHandleCompletingPhaseL();
private: 
   
    CMTPImageDpGetThumb(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
    void ConstructL();
        
private: // Owned
    CMTPImageDataProvider& imgDp;
    CMTPTypeOpaqueData*    iThumb;    
    CMTPObjectMetaData*    iObjectMeta;
    };
    
#endif // CMTPIMAGEDPGETTHUMB_H

