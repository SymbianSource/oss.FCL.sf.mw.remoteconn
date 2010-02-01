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


#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpimagedpgetobjectpropssupported.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"GetObjectPropsSupported");)

/**
Verification data for the GetObjectPropSupported request
*/
const TMTPRequestElementInfo KMTPGetObjectPropSupportedPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeFormatCode, EMTPElementAttrNone, 1, EMTPFormatCodeUndefined, 0},
    };
 
/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
MMTPRequestProcessor* CMTPImageDpGetObjectPropsSupported::NewL(
                                            MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection,CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpGetObjectPropsSupported* self = new (ELeave) CMTPImageDpGetObjectPropsSupported(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/	
CMTPImageDpGetObjectPropsSupported::~CMTPImageDpGetObjectPropsSupported()
    {
    delete iObjectPropsSupported;
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/		
CMTPImageDpGetObjectPropsSupported::CMTPImageDpGetObjectPropsSupported(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPropSupportedPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPropSupportedPolicy)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }
    
/**
GetObjectPropSupported request handler
*/
void CMTPImageDpGetObjectPropsSupported::ServiceL()
    {
    TInt count = sizeof(KMTPImageDpSupportedProperties) / sizeof(TUint16);
    for(TInt i = 0; i < count; i++)
        {
        iObjectPropsSupported->AppendUintL(KMTPImageDpSupportedProperties[i]);
        }
    SendDataL(*iObjectPropsSupported);	
    }

/**
Second-phase construction
*/		
void CMTPImageDpGetObjectPropsSupported::ConstructL()
    {
    iObjectPropsSupported = CMTPTypeArray::NewL(EMTPTypeAUINT16);	
    }
