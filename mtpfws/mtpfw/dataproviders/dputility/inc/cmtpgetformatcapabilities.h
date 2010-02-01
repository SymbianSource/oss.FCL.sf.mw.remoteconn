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
#ifndef CMTPGETFORMATCAPABILITLIST_H_
#define CMTPGETFORMATCAPABILITLIST_H_

#include <mtp/cmtptypeformatcapabilitylist.h>

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"

class CMTPTypeObjectPropDesc;
/** 
Implements the file data provider CMTPGetFormatCapabilities request processor.
@internalComponent
*/
class CMTPGetFormatCapabilities : public CMTPRequestProcessor
    {
public:
    
    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPGetFormatCapabilities();    
    

private: // From CMTPRequestProcessor

    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:

    CMTPGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    void BuildFormatAssociationL();
    void BuildFormatAsUndefinedL( TUint aFormatCode );
    CMTPTypeObjectPropDesc* ServiceProtectionStatusL();
    CMTPTypeObjectPropDesc* ServiceAssociationTypeL();
    CMTPTypeObjectPropDesc* ServiceNonConsumableL();
    
private: // Owned

    //[SP-Format-0x3002]
    //Make the same behavior betwen 0x3000 and 0x3002.
	//it is used to judge whether FileDP supports 0x3002 or not.
	RMTPFramework       						 iSingletons;
	
    TUint                                        iFormatCode;
    /* 
     * CMTPTypeCapabilityList dataset.
     */
    CMTPTypeFormatCapabilityList*                iCapabilityList;
    };


#endif /* CMTPGETFORMATCAPABILITLIST_H_ */
