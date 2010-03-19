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
#include <mtp/tmtptyperequest.h>

#include "cmtpdevrequestunknown.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"

/**
Factory method.
@param aFramework The data provider framework
@param aConnection The connection on which the request was received.
@return A pointer to the created request processor object.
@leave One of the system wide error codes, if a processing error occurs.
*/ 
MMTPRequestProcessor* CMTPDevRequestUnknown::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPDevRequestUnknown* self = new (ELeave) CMTPDevRequestUnknown(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPDevRequestUnknown::~CMTPDevRequestUnknown()
    {
    iSingletons.Close(); 
    delete iDataSet;
    iDataSet = NULL;
    }

/**
Implements the default MTP request handler.
@leave One of the system wide error codes, if a processing error occurs.
*/    
void CMTPDevRequestUnknown::ServiceL()    
    {
    const TUint KOpCode(iRequest->Uint16(TMTPTypeRequest::ERequestOperationCode));
    TBool NoDataPhase(ETrue);
    TMTPResponseCode rsp(EMTPRespCodeOperationNotSupported);
    if (iSingletons.Router().OperationSupportedL(KOpCode))
    	{
    	switch (KOpCode)
    	    {
	    case EMTPOpCodeGetInterdependentPropDesc:
	        {
            const TUint32 KFormatcode = iRequest->Uint32(TMTPTypeRequest::ERequestParameter1);
            
            if ((KFormatcode == KMTPFormatsAll) || (KFormatcode == KMTPNotSpecified32))
                {
                rsp = EMTPRespCodeInvalidCodeFormat;
                }
            else
                {
            ServiceInterdependentPropDescL();
            NoDataPhase = EFalse;
                }
            }
            break;

	        
	    case EMTPOpCodeTerminateOpenCapture:
	        {
	        if (iSingletons.Router().RouteRequestRegisteredL(*iRequest, iConnection))
	            {
    	        rsp = EMTPRespCodeInvalidTransactionID;
	            }
            else
                {
    	        rsp = EMTPRespCodeCaptureAlreadyTerminated;
                }
	        }
	        break;
	        
	    case EMTPOpCodeGetPartialObject:
	    case EMTPOpCodeGetObjectReferences:
	    case EMTPOpCodeSetObjectReferences:
	    case EMTPOpCodeSetObjectProtection:
            {
            CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
            if (!iSingletons.ObjectMgr().ObjectL(iRequest->Uint32(TMTPTypeRequest::ERequestParameter1), *object))
                {
    	        rsp = EMTPRespCodeInvalidObjectHandle;
                }
            else if ((KOpCode == EMTPOpCodeSetObjectReferences) &&
                        (object->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation) &&
                        (object->Uint(CMTPObjectMetaData::EFormatSubCode) == EMTPAssociationTypeGenericFolder))
                {
    	        rsp = EMTPRespCodeAccessDenied;
                }
            else
                {
    	        rsp = EMTPRespCodeOperationNotSupported;
                }
            CleanupStack::PopAndDestroy(object);
            }
	        break;
	        
        case EMTPOpCodeGetServiceCapabilities:
	    case EMTPOpCodeGetServicePropDesc:
        case EMTPOpCodeGetServicePropList:
        case EMTPOpCodeSetServicePropList:    
	    case EMTPOpCodeDeleteServicePropList:
	        {
	         rsp = EMTPRespCodeInvalidServiceID;
	        }
	        break;
	    case EMTPOpCodeDeleteObjectPropList:
	    case EMTPOpCodeUpdateObjectPropList:
	        {
	        rsp = EMTPRespCodeInvalidObjectHandle;
	        }
	        break;
	        
	    default:
	        rsp = EMTPRespCodeGeneralError;
	        break;
    	    }
        }
    if ( NoDataPhase )
       {
       SendResponseL(rsp);
       }

    }  


/**
+Send empty interdependentpropdesc data when the request is route to device dp
+*/
void CMTPDevRequestUnknown::ServiceInterdependentPropDescL()
    {
    delete iDataSet;
    iDataSet = NULL;

    iDataSet = CMTPTypeInterdependentPropDesc::NewL();
    SendDataL(*iDataSet);
    }

    
/**
Constructor.
@param aFramework The data provider framework
@param aConnection The connection on which the request was received.
*/    
CMTPDevRequestUnknown::CMTPDevRequestUnknown(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestUnknown(aFramework, aConnection)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPDevRequestUnknown::ConstructL()
    {
    iSingletons.OpenL();
    }
