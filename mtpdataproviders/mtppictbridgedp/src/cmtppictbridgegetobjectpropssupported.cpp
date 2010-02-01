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
#include <mtp/cmtptypearray.h>
#include "cmtppictbridgegetobjectpropssupported.h"
#include "mtppictbridgedpconst.h"

/**
Verification data for the GetObjectPropSupported request
*/
const TMTPRequestElementInfo KMTPGetObjectPropSupportedPolicy[] = 
    {
            {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeFormatCode, EMTPElementAttrNone, 1, EMTPFormatCodeUndefined, 0},
    };

MMTPRequestProcessor* CMTPPictBridgeGetObjectPropsSupported::NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider )
    {
    CMTPPictBridgeGetObjectPropsSupported* self = new (ELeave) CMTPPictBridgeGetObjectPropsSupported( aFramework, aConnection, aDataProvider );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/  
CMTPPictBridgeGetObjectPropsSupported::~CMTPPictBridgeGetObjectPropsSupported()
    {
    delete iObjectPropsSupported;
    }

/**
Standard c++ constructor
*/
CMTPPictBridgeGetObjectPropsSupported::CMTPPictBridgeGetObjectPropsSupported( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider )
:CMTPRequestProcessor( aFramework, aConnection, sizeof(KMTPGetObjectPropSupportedPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPropSupportedPolicy ),
iPictBridgeDP( aDataProvider )
    {
    
    }

/**
check format code
*/
TMTPResponseCode CMTPPictBridgeGetObjectPropsSupported::CheckRequestL()
    {
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    if ( EMTPRespCodeOK == responseCode )
        {
        TUint32 formatCode = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
        if( formatCode != EMTPFormatCodeScript )
            {
            responseCode = EMTPRespCodeInvalidObjectFormatCode;
            }
        }
    
    return responseCode;
    }

/**
CMTPPictBridgeGetObjectPropsSupported request handler
*/
void CMTPPictBridgeGetObjectPropsSupported::ServiceL()
    {
    TInt count = sizeof(KMTPPictBridgeDpSupportedProperties) / sizeof(TUint16);
    for( TInt i = 0; i < count; i++ )
        {
        iObjectPropsSupported->AppendUintL(KMTPPictBridgeDpSupportedProperties[i]);
        }
    SendDataL(*iObjectPropsSupported);
    }

/**
Second-phase construction
*/
void CMTPPictBridgeGetObjectPropsSupported::ConstructL()
    {
    iObjectPropsSupported = CMTPTypeArray::NewL(EMTPTypeAUINT16);
    }
