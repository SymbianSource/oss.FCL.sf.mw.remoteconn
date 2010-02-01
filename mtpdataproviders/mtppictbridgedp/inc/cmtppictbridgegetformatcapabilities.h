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


#ifndef CMTPPICTBRIDGEGETFORMATCAPABILITIES_H_
#define CMTPPICTBRIDGEGETFORMATCAPABILITIES_H_

#include <mtp/cmtptypeformatcapabilitylist.h>
#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"
#include "mtpdpconst.h"

/** 
Implements the file data provider CMTPGetFormatCapabilities request processor.
@internalComponent
*/
class CMTPPictBridgeDpGetFormatCapabilities : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);    
    ~CMTPPictBridgeDpGetFormatCapabilities();    

private: // From CMTPRequestProcessor
    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:
    CMTPPictBridgeDpGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();
    void BuildFormatScriptL();
    CMTPTypeObjectPropDesc* ServiceProtectionStatusL();
    CMTPTypeObjectPropDesc* ServiceNonConsumableL();
    TUint16 GetPropertyGroupNumber(const TUint16 aPropCode) const;
    
private: // Owned
    CMTPPictBridgeDataProvider&   iPictBridgeDP;
    TUint                         iFormatCode;
    /* 
     * CMTPTypeCapabilityList dataset.
     */
    CMTPTypeFormatCapabilityList* iCapabilityList;
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    };

#endif /* CMTPPICTBRIDGEGETFORMATCAPABILITIES_H_ */

