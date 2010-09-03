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
@internalComponent
*/
#ifndef CMTPIMAGEDPGETFORMATCAPABILITIES_H_
#define CMTPIMAGEDPGETFORMATCAPABILITIES_H_

#include <mtp/cmtptypeformatcapabilitylist.h>
#include "cmtprequestprocessor.h"


class CMTPTypeObjectPropDesc;
class CMTPImageDataProvider;

/** 
Implements the file data provider CMTPGetFormatCapabilities request processor.
@internalComponent
*/
class CMTPImageDpGetFormatCapabilities : public CMTPRequestProcessor
    {
public:
    
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);    
    ~CMTPImageDpGetFormatCapabilities();    
    

private: // From CMTPRequestProcessor

    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:

    CMTPImageDpGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    void ServiceOneFormatCapabilitiesL(TUint aFormatCode);
    CMTPTypeObjectPropDesc* ServiceProtectionStatusL();
    CMTPTypeObjectPropDesc* ServiceNonConsumableL();
	CMTPTypeObjectPropDesc* ServiceHiddenL();
    
    TUint16 GetPropertyGroupNumber(const TUint16 aPropCode) const;
    
private: // Owned
    
    TUint                                        iFormatCode;
    /* 
     * CMTPTypeCapabilityList dataset.
     */
    CMTPTypeFormatCapabilityList*                iCapabilityList;
    };


#endif /* CMTPIMAGEDPGETFORMATCAPABILITIES_H_*/
