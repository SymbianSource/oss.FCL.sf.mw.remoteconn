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

#ifndef CMTPIMAGEDPGETOBJECTPROPVALUE_H
#define CMTPIMAGEDPGETOBJECTPROPVALUE_H

#include "cmtprequestprocessor.h"

class CMTPImageDpObjectPropertyMgr;
class TMTPTypeUint8;
class TMTPTypeUint16;
class TMTPTypeUint32;
class TMTPTypeUint64;
class TMTPTypeUint128;
class CMTPTypeString;
class CMTPTypeArray;
class CMTPImageDataProvider;


class CMTPImageDpGetObjectPropValue : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);	
    ~CMTPImageDpGetObjectPropValue();	
    
private:	
    CMTPImageDpGetObjectPropValue(
                    MMTPDataProviderFramework& aFramework,
                    MMTPConnection& aConnection,
                    CMTPImageDataProvider& aDataProvider);
    void ConstructL();

private:	//from CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
    virtual void ServiceL();

private:
    void ServiceStorageIdL();
    void ServiceObjectFormatL();
    void ServiceProtectionStatusL();
    void ServiceObjectSizeL();
    void ServiceFileNameL();
    void ServiceDateCreatedL();
    void ServiceDateModifiedL();
    void ServiceParentObjectL();
    void ServicePuidL();
    void ServiceNameL();
    void ServiceWidthL();
    void ServiceHeightL();
    void ServiceImageBitDepthL();
    void ServiceRepresentativeSampleFormatL();
    void ServiceRepresentativeSampleSizeL();
    void ServiceRepresentativeSampleHeightL();
    void ServiceRepresentativeSampleWidthL();
    void ServiceRepresentativeSampleDataL();
    void ServiceNonConsumableL();

private:
    CMTPImageDpObjectPropertyMgr&	iObjectPropertyMgr;
    TMTPTypeUint8			iMTPTypeUint8;
    TMTPTypeUint16			iMTPTypeUint16;
    TMTPTypeUint32			iMTPTypeUint32;
    TMTPTypeUint64			iMTPTypeUint64;
    TMTPTypeUint128			iMTPTypeUint128;
    CMTPTypeString*			iMTPTypeString;
    CMTPTypeArray*          iMTPTypeArray;
    CMTPObjectMetaData*     iObjectMeta;
    
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;    
    };
    
#endif  //CMTPIMAGEDPGETOBJECTPROPVALUE_H

