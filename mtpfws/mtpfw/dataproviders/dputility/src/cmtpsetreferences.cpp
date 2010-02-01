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


#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpsetreferences.h"
#include "mtpdppanic.h"

/**
Verification data for the SetReferences request
*/
const TMTPRequestElementInfo KMTPSetReferencesPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 0, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin    The data provider plugin
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/    
EXPORT_C MMTPRequestProcessor* CMTPSetReferences::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPSetReferences* self = new (ELeave) CMTPSetReferences(aFramework, aConnection);
    return self;
    }

/**
Destructor
*/    
EXPORT_C CMTPSetReferences::~CMTPSetReferences()
    {
    delete iReferences;
    }

/**
Standard c++ constructor
*/    
CMTPSetReferences::CMTPSetReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPSetReferencesPolicy)/sizeof(TMTPRequestElementInfo), KMTPSetReferencesPolicy)
    {
    
    }

/**
SetReferences request handler
start receiving reference data from the initiator
*/
void CMTPSetReferences::ServiceL()
    {
    delete iReferences;
    iReferences = NULL;
    iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
    ReceiveDataL(*iReferences);
    }

/**
Apply the references to the specified object
@return EFalse
*/    
TBool CMTPSetReferences::DoHandleResponsePhaseL()
    {
    if(!VerifyReferenceHandlesL())
        {
        SendResponseL(EMTPRespCodeInvalidObjectReference);
        }
    else
        {
        MMTPReferenceMgr& referenceMgr = iFramework.ReferenceMgr();
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        referenceMgr.SetReferencesL(TMTPTypeUint32(objectHandle), *iReferences);
        SendResponseL(EMTPRespCodeOK);
        }
    return EFalse;    
    }

TBool CMTPSetReferences::HasDataphase() const
    {
    return ETrue;
    }

/**
Verify if the references are valid handles to objects
@return ETrue if all the references are good, otherwise, EFalse
*/    
TBool CMTPSetReferences::VerifyReferenceHandlesL() const
    {
    __ASSERT_DEBUG(iReferences, User::Invariant());
    TBool result = ETrue;
    TInt count = iReferences->NumElements();
    CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
    MMTPObjectMgr& objectMgr = iFramework.ObjectMgr();
    for(TInt i = 0; i < count; i++)
        {
        TMTPTypeUint32 handle;
        iReferences->ElementL(i, handle);
        if(!objectMgr.ObjectL(handle, *object))
            {
            result = EFalse;
            break;
            }
        }
    CleanupStack::PopAndDestroy(object);
    return result;    
    }
