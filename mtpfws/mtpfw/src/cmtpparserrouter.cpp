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
#include <mtp/mtpprotocolconstants.h>

#include "cmtpconnection.h"
#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpframeworkconfig.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"
#include "cmtpsession.h"
#include "cmtpstoragemgr.h"
#include "tmtptypeobjecthandle.h"
#include "cmtpservicemgr.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ParserRouter");)

/**
Provides the byte size of the specified array.
*/
#define _ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


/**
CMTPParserRouter panic codes.
*/


enum TMTPPanicReasons
    {
    EMTPPanicRoutingConflict = 0
    };
    
/**
Produces a "CMTPParserRouter" category panic.
@param aReason The panic code.
*/
LOCAL_C void Panic(TInt __DEBUG_ONLY(aReason))
    {
	__DEBUG_ONLY(_LIT(KMTPPanicCategory, "CMTPParserRouter"));
    __DEBUG_ONLY(User::Panic(KMTPPanicCategory, aReason));
    }
    
/**
Constructor.
*/
EXPORT_C CMTPParserRouter::TRoutingParameters::TRoutingParameters(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) :
    iConnection(aConnection),
    iRequest(aRequest),
    iParameters(iParameterData, ENumTypes)
    {
    Reset();
    }
        
    
/**
Copy constructor.
*/
CMTPParserRouter::TRoutingParameters::TRoutingParameters(const TRoutingParameters& aParams) :
    iConnection(aParams.iConnection),
    iRequest(aParams.iRequest),
    iParameters(iParameterData, ENumTypes)
    {
    iParameters.Copy(aParams.iParameters.Begin(), aParams.iParameters.Count());
    }

/**
Provides the handle of the MTP connection on associated with the operation.
@return The MTP connection handle.
*/    
EXPORT_C MMTPConnection& CMTPParserRouter::TRoutingParameters::Connection() const
    {
    return iConnection;
    }
    
/**
Provides the value of the specified parameter.
@param aId The parameter identifier.
@return The parameter value.
*/
EXPORT_C TUint CMTPParserRouter::TRoutingParameters::Param(CMTPParserRouter::TRoutingParameters::TParameterType aId) const
    {
    return iParameters[aId];
    }
    
/**
Resets all parameter values to zero.
*/
EXPORT_C void CMTPParserRouter::TRoutingParameters::Reset()
    {
    iParameters.Reset();
    }
    
/**
Provides the operation dataset associated with the operation.
@return The operation dataset.
*/
EXPORT_C const TMTPTypeRequest& CMTPParserRouter::TRoutingParameters::Request() const
    {
    return iRequest;
    }
    
/**
Sets the value of the specified parameter.
@param aId The parameter identifier.
@param aVal The new parameter value.
*/
EXPORT_C void CMTPParserRouter::TRoutingParameters::SetParam(CMTPParserRouter::TRoutingParameters::TParameterType aId, TUint aVal)
    {
    iParameters[aId] = aVal;
    }

/**
CMTPParserRouter factory method.
@return A pointer to a new CMTPDataProvider instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
 */
CMTPParserRouter* CMTPParserRouter::NewL()
    {
    CMTPParserRouter* self = new (ELeave) CMTPParserRouter();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/
CMTPParserRouter::~CMTPParserRouter()
    {
    __FLOG(_L8("~CMTPParserRouter, Entry"));
    iMaps.ResetAndDestroy();
    iSingletons.Close();
    __FLOG(_L8("~CMTPParserRouter, Exit"));
    __FLOG_CLOSE;
    }

/**
Configures the parser/router. This processing primarily involves retrieving 
each data provider's set of supported category codes and using them to build
up the operation parameter lookup routing sub-type tables.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPParserRouter::ConfigureL()
    {
    __FLOG(_L8("ConfigureL, Entry"));    
    const TUint KMapIds[] = 
        {
        ESubTypeDevicePropCode,
        ESubTypeObjectPropCode,
        ESubTypeOperationCode,
        ESubTypeStorageType,
        ESubTypeFormatCodeFormatSubcode,
        ESubTypeFormatCodeOperationCode,
        ESubTypeStorageTypeOperationCode,
        ESubTypeFormatCodeFormatSubcodeStorageType,
        ESubTypeServiceIDOperationCode
        };
        
    iMaps.ResetAndDestroy();
    for (TUint i(0); (i < _ARRAY_SIZE(KMapIds)); i++)
        {
        const TUint KSubType(KMapIds[i]);
        CMap* map(CMap::NewLC(KSubType));
        iMaps.AppendL(map);
        CleanupStack::Pop(map);
        
        RArray<TUint> p1Codes;
        RArray<TUint> p2Codes;
        RArray<TUint> p3Codes;
        CleanupClosePushL(p1Codes);
        CleanupClosePushL(p2Codes);
        CleanupClosePushL(p3Codes);
        
        GetMapParameterIdsL(KSubType, p1Codes, p2Codes, p3Codes);
        
        const TUint KParams(KSubType & ESubTypeParamsMask);
        switch (KParams)
            {
        case ESubTypeParams1:
            Configure1ParameterMapL(KSubType, p1Codes);
            break;
            
        case ESubTypeParams2:
            Configure2ParameterMapL(KSubType, p1Codes, p2Codes);
            break;
            
        case ESubTypeParams3:
            Configure3ParameterMapL(KSubType, p1Codes, p2Codes, p3Codes);
            break;

        default:
            __DEBUG_ONLY(User::Invariant());
            break;
            }
            
        CleanupStack::PopAndDestroy(&p3Codes);
        CleanupStack::PopAndDestroy(&p2Codes);
        CleanupStack::PopAndDestroy(&p1Codes);
        }
        
    __FLOG_STMT(FLOGMapsL());
    __FLOG(_L8("ConfigureL, Exit"));
    }
    
/**
Indicates if the specified MTP operation code is supported by any of the set 
of loaded data providers.
@param aOperation The MTP operation code.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
EXPORT_C TBool CMTPParserRouter::OperationSupportedL(TUint16 aOperation) const
    {
    __FLOG(_L8("OperationSupported, Entry"));
    RArray<TUint> from;
    RArray<TUint> to;
    CleanupClosePushL(from);
    CleanupClosePushL(to);
    
    from.AppendL(aOperation);
    iMaps[Index(ESubTypeOperationCode)]->GetToL(from, to);
    TBool ret(to.Count() > 0);

    CleanupStack::PopAndDestroy(&to);
    CleanupStack::PopAndDestroy(&from);
    __FLOG(_L8("OperationSupported, Exit"));
    return (ret);
    }

/**
Parses the operation dataset supplied in the specified routing parameters 
object which encapsulates all information required to route the operation. 
The parsing process involves:

    1.  Extracting all relevant routing information from the received operation 
        dataset. Note that not all parameter data is extracted, only that which 
        is required to route the operation.
    2.  Coarse grain validating the parsed data. Specifically this involves 
        validating that any MTP StorageID or Object Handle parameter data refers 
        to valid entities that exist on the device.
    3.    Extracting additional meta-data related to specific data objects and/or 
        storages referred to in the operation dataset and which is required to 
        route the operation.
        
@param aParams The routing parameters object. On entry this contains the 
operation dataset to be parsed and the handle of the MTP connection on which it
was received. On exit this contains all information required to route the 
operation. 
@leave One of the system wide error codes if a processing failure occurs,
*/
EXPORT_C void CMTPParserRouter::ParseOperationRequestL(TRoutingParameters& aParams) const
    {
    __FLOG(_L8("ParseOperationRequestL, Entry"));
    const TUint16 KOpCode(aParams.Request().Uint16(TMTPTypeRequest::ERequestOperationCode));
    __FLOG_VA((_L8("Operation Code = 0x%04X"), KOpCode));
    switch (KOpCode)
        {
    case EMTPOpCodeGetStorageInfo:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamStorageId, aParams);
        break;
      
    case EMTPOpCodeGetObjectInfo:
    case EMTPOpCodeGetObject:
    case EMTPOpCodeGetThumb:
    case EMTPOpCodeSetObjectProtection:
    case EMTPOpCodeMoveObject:
    case EMTPOpCodeCopyObject:        
    case EMTPOpCodeGetPartialObject:
    case EMTPOpCodeGetObjectReferences:
    case EMTPOpCodeSetObjectReferences:
    case EMTPOpCodeUpdateObjectPropList :
    case EMTPOpCodeDeleteObjectPropList :
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamObjectHandle, aParams);
        break; 
        
    case EMTPOpCodeGetObjectPropValue:
    case EMTPOpCodeSetObjectPropValue:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamObjectHandle, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter2, TRoutingParameters::EParamObjectPropCode, aParams);
        break; 

    case EMTPOpCodeDeleteObject:              
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamObjectHandle, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter2, TRoutingParameters::EParamFormatCode, aParams); 
        break;
        
    case EMTPOpCodeSendObjectInfo:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamStorageId, aParams);
        break;

    case EMTPOpCodeInitiateCapture: 
    case EMTPOpCodeInitiateOpenCapture:
        {   
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamStorageId, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter2, TRoutingParameters::EParamFormatCode, aParams); 
        TUint format(aParams.Request().Uint32(TMTPTypeRequest::ERequestParameter2));
        if (format == KMTPNotSpecified32)
            {
            iSingletons.FrameworkConfig().GetValueL(MMTPFrameworkConfig::EDefaultObjectFormatCode, format);
            if (format == KMTPNotSpecified32)
                {
                format = EMTPFormatCodeUndefined;
                }
            aParams.SetParam(TRoutingParameters::EParamFormatCode, format);
            }
        }
        break; 
 
    case EMTPOpCodeSendObjectPropList:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamStorageId, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter3, TRoutingParameters::EParamFormatCode, aParams); 
        break;  
        
    case EMTPOpCodeGetDevicePropDesc:
    case EMTPOpCodeGetDevicePropValue:
    case EMTPOpCodeSetDevicePropValue:
    case EMTPOpCodeResetDevicePropValue:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamDevicePropCode, aParams);
        break;
        
    case EMTPOpCodeGetObjectPropsSupported:
    case EMTPOpCodeGetInterdependentPropDesc:
    case EMTPOpCodeGetFormatCapabilities:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamFormatCode, aParams); 
        break;

    case EMTPOpCodeGetObjectPropDesc:
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamObjectPropCode, aParams); 
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter2, TRoutingParameters::EParamFormatCode, aParams);
        break;

    case EMTPOpCodeGetObjectPropList:              
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter2, TRoutingParameters::EParamFormatCode, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamObjectHandle, aParams);
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter3, TRoutingParameters::EParamObjectPropCode, aParams);
        break;
        
    case EMTPOpCodeGetServiceInfo :
    case EMTPOpCodeGetServiceCapabilities :
    case EMTPOpCodeGetServicePropDesc :
    case EMTPOpCodeGetServicePropList :
    case EMTPOpCodeSetServicePropList:
    case EMTPOpCodeDeleteServicePropList :
        {
        ParseOperationRequestParameterL(TMTPTypeRequest::ERequestParameter1, TRoutingParameters::EParamServiceId, aParams);
        }
        break;
                
    case EMTPOpCodeGetDeviceInfo:
    case EMTPOpCodeOpenSession:
    case EMTPOpCodeCloseSession:
    case EMTPOpCodeGetStorageIDs:
    case EMTPOpCodeGetNumObjects:
    case EMTPOpCodeGetObjectHandles:
    case EMTPOpCodeSendObject:
    case EMTPOpCodeFormatStore:
    case EMTPOpCodeResetDevice:
    case EMTPOpCodeSelfTest:
    case EMTPOpCodePowerDown:
    case EMTPOpCodeSetObjectPropList:
    case EMTPOpCodeSkip:
    case EMTPOpCodeGetServiceIDs:
        default:
        break;
        }
    __FLOG(_L8("ParseOperationRequestL, Exit"));
    }

/**
Routes an MTP operation using the specified routing parameters. By default 
only operation parameter routing is performed.
@param aParams The routing parameters.
@param aTargets One exit, the set of data provider targets to which the 
operation should be dispatched.
@leave One of the system wide error codes if a processing failure occurs,
*/
EXPORT_C void CMTPParserRouter::RouteOperationRequestL(const TRoutingParameters& aParams, RArray<TUint>& aTargets) const
    {
    __FLOG(_L8("RouteOperationRequestL, Entry"));
    aTargets.Reset();
    
    // By default ETypeOperationParameter routing is always enabled.
    if (!(aParams.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeOperationParameter))
        {
        const_cast<TRoutingParameters&>(aParams).SetParam(TRoutingParameters::EFlagRoutingTypes, ETypeOperationParameter);
        }
    
    // Get the routing and validation sub-types.
    RArray<TUint> routing;
    CleanupClosePushL(routing);
    RArray<TUint> validation;
    CleanupClosePushL(validation);
    RArray<TRoutingParameters> params;
    CleanupClosePushL(params);
    params.AppendL(TRoutingParameters(aParams));
    TRoutingParameters& param1(params[0]);
    GetRoutingSubTypesL(params, routing, validation);
    
    // Execute the routing sub-types.
    const TUint KCountParams(params.Count());
    for (TUint p(0); ((p < KCountParams) && (aTargets.Count() == 0)); p++)
        {
        const TRoutingParameters& KParam(params[p]);
        const TUint KCountRouting(routing.Count());
        for (TUint r(0); (r < KCountRouting); r++)
            {
            const TUint KRouting(routing[r]);
            if ((KRouting & ESubTypeParamsMask) == ESubTypeParams0)
                {
                RouteOperationRequest0ParametersL(KRouting, KParam, aTargets);
                }
            else
                {
                RouteOperationRequestNParametersL(KRouting, KParam, aTargets);
                }
            }
        }
     
    // Execute the validation sub-types.
    ValidateTargetsL(param1, validation, aTargets);
    const TUint KCountTargets(aTargets.Count());
    if ((KCountTargets == 0) && (param1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeFramework))
        {
        SelectTargetL(iSingletons.DpController().DeviceDpId(), aTargets);
        }
    else if (KCountTargets > 1)
        {
        if (param1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeFlagSingleTarget)
            {
            TUint target(KCountTargets);
            while (target-- > 1)
                {
                aTargets.Remove(target);
                }
            }
        else if (param1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeFramework)
            {
            aTargets.Reset();
            SelectTargetL(iSingletons.DpController().ProxyDpId(), aTargets);
            }
        }    
    CleanupStack::PopAndDestroy(&params);
    CleanupStack::PopAndDestroy(&validation);
    CleanupStack::PopAndDestroy(&routing);
    __FLOG(_L8("RouteOperationRequestL, Exit"));
    }

/**
Indicates if a routing request with the specified MTP operation code is 
registered on the specified session.
@param aRequest The MTP operation requesty dataset specifying MTP operation 
code and session.
@param aConnection The handle of the MTP connection on which the operation
request is expected to be received.
@return ETrue if a routing request with the specified MTP operation code is 
registered on the session, otherwise EFalse.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TBool CMTPParserRouter::RouteRequestRegisteredL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
    {
    __FLOG(_L8("RouteRequestRegistered, Entry"));
    TBool ret(EFalse);
    const TUint32 KSessionId(aRequest.Uint32(TMTPTypeRequest::ERequestSessionID));
    if ((KSessionId != KMTPSessionAll) && (aConnection.SessionWithMTPIdExists(KSessionId)))
        {
        CMTPSession& session(static_cast<CMTPSession&>(aConnection.SessionWithMTPIdL(KSessionId)));
        ret = session.RouteRequestRegistered(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
        }
    __FLOG(_L8("RouteRequestRegistered, Exit"));
    return ret;
    }
   
/**
Routes and dispatches the specified MTP event dataset. The only valid event 
that is accepted is CancelTransaction, all other event types are discarded. 
Events are routed to the same target to which the currently active operation 
was dispatched. If there is no active transaction in progress then the event 
will be discarded.
@param aEvent The MTP event dataset.
@param aConnection The MTP connection on which the event was received.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPParserRouter::ProcessEventL(const TMTPTypeEvent& aEvent, CMTPConnection& aConnection) const
    {
    __FLOG(_L8("ProcessEventL, Entry"));
    if ((aEvent.Uint16(TMTPTypeEvent::EEventCode) == EMTPEventCodeCancelTransaction) &&
        (aConnection.SessionWithMTPIdExists(aEvent.Uint32(TMTPTypeEvent::EEventSessionID))))
        {
        CMTPSession& session(static_cast<CMTPSession&>(aConnection.SessionWithMTPIdL(aEvent.Uint32(TMTPTypeEvent::EEventSessionID))));
        if (session.TransactionPhase() != EIdlePhase)
            {    
            iSingletons.DpController().DataProviderL(RoutingTargetL(session.ActiveRequestL(), aConnection)).ExecuteEventL(aEvent, aConnection);
            }
        }
    __FLOG(_L8("ProcessEventL, Exit"));
    }
   
/**
Routes and dispatches the specified MTP operation (request) dataset.
@param aRequest The MTP operation (request) dataset.
@param aConnection The MTP connection on which the event was received.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPParserRouter::ProcessRequestL(const TMTPTypeRequest& aRequest, CMTPConnection& aConnection) const
    {
    __FLOG(_L8("ProcessRequestL, Entry")); 
    iSingletons.DpController().DataProviderL(RoutingTargetL(aRequest, aConnection)).ExecuteRequestL(aRequest, aConnection);
    __FLOG(_L8("ProcessRequestL, Exit"));
    }

/**
Registers the calling data provider to receive one or more occurrences of 
the specified request dataset that are received on the specified connection.
@param aRequest The operation request dataset being registered.
@param aConnection The handle of the MTP connection on which the operation
request is expected to be received.
@param aId The data provider identifier.
@leave One of the system wide error codes, if a processing failure occurs.
@see MMTPDataProviderFramework::RouteRequestRegisterL
*/
void CMTPParserRouter::RouteRequestRegisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TInt aId)
    {
    __FLOG(_L8("RouteRequestRegisterL, Entry"));
    const TUint32 KSessionId(aRequest.Uint32(TMTPTypeRequest::ERequestSessionID));
    if (KSessionId == KMTPSessionAll)
        {
        // Register the request on all sessions.
        const TUint KNumSessions(aConnection.SessionCount());
        TUint count(0);
        TUint sessionId(1);
        while (count < KNumSessions)
            {
            // Insert the correct session id into a copy of the request and register this copy with the session
            if (aConnection.SessionWithMTPIdExists(sessionId))
                {
                // Session exists
                count++;
                TMTPTypeRequest req(aRequest);
                req.SetUint32(TMTPTypeRequest::ERequestSessionID, sessionId);
                CMTPSession& session(static_cast<CMTPSession&>(aConnection.SessionWithMTPIdL(sessionId)));
                session.RouteRequestRegisterL(req, aId);
                }
            sessionId++;
            }
        }
    else
        {
        CMTPSession& session(static_cast<CMTPSession&>(aConnection.SessionWithMTPIdL(KSessionId)));
        session.RouteRequestRegisterL(aRequest, aId);
        }
    __FLOG(_L8("RouteRequestRegisterL, Exit"));
    }

/**
Cancels a pending RouteRequestRegisterL registration.
@param aRequest The registered operation request dataset.
@param aConnection The handle of the MTP connection for which the operation
request was registered.
@leave One of the system wide error codes, if a general processing error
occurs.
@see MMTPDataProviderFramework::RouteRequestUnregisterL
*/
void CMTPParserRouter::RouteRequestUnregisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("RouteRequestUnregisterL, Entry"));
    CMTPSession& session(static_cast<CMTPSession&>(aConnection.SessionWithMTPIdL(aRequest.Uint32(TMTPTypeRequest::ERequestSessionID))));
    session.RouteRequestUnregister(aRequest);
    __FLOG(_L8("RouteRequestUnregisterL, Exit"));
    }

/**
Constructor.
@param aFrom The map source parameter.
*/
CMTPParserRouter::TMap::TMap(TUint aFrom) :
    iFrom(aFrom),
    iSubType(0),
    iTo(0)
    {

    }

/**
Constructor.
@param aFrom The map source parameter.
@param aTo The map target.
@param aSubType The map routing sub-type code (@see CMTPParserRouter::TRoutingSubType).
*/
CMTPParserRouter::TMap::TMap(TUint aFrom, TUint aTo, TUint aSubType) :
    iFrom(aFrom),
    iSubType(aSubType),
    iTo(aTo)
    {

    }

/**
CMTPParserRouter::CMap factory method.
@param aSubType The map routing sub-type code (@see CMTPParserRouter::TRoutingSubType).
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
CMTPParserRouter::CMap* CMTPParserRouter::CMap::NewLC(TUint aSubType)
    {
    return NewLC(0, aSubType);
    }

/**
Destructor.
*/
CMTPParserRouter::CMap::~CMap()
    {
    if (Params(iSubType) == ESubTypeParams1)
        {
        iToNodes.Reset();
        }
    else
        {
        iToBranches.ResetAndDestroy();
        }
    __FLOG_CLOSE;
    }
    
/**
Provides the map source parameter.
@return The map source parameter.
*/
TUint CMTPParserRouter::CMap::From() const
    {
    return iFrom;
    }
    
/**
Initialises a map source parameter set array.
@param aFrom On exit, the initialised map source parameter set array.
@leave One of the system wide error codes, if a general processing error 
occurs.

*/
void CMTPParserRouter::CMap::InitParamsL(RArray<TUint>& aFrom) const
    {
    aFrom.Reset();
    TUint KCount(ParamsCount(iSubType));
    for (TUint i(0); (i < KCount); i++)
        {
        aFrom.AppendL(0);
        }
    }

/**
Inserts an entry into the map table with the specified source and target 
parameters.
@param aFrom The map source parameter set array.
@param aFrom The map target parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::InsertL(const RArray<TUint>& aFrom, TUint aTo)
    {
    const TUint KFrom(Param(aFrom));
    if (Params(iSubType) == ESubTypeParams1)
        {
        // Node.
        __FLOG_STMT(FLOGMapEntryL(aFrom, aTo));
        const TUint KSubType(CMTPParserRouter::SubType(Index(iSubType), Flags(iSubType), (ParamsCount(iSubType) - 1)));
        const TMap KNode(KFrom, aTo, KSubType);
        NodeInsertL(KNode);
        }
    else
        {
        // Branch.
        TInt idx(BranchFind(KFrom));
        if (idx == KErrNotFound)
            {
            idx = BranchInsertL(KFrom);
            __ASSERT_DEBUG((idx != KErrNotFound), User::Invariant());
            }
        iToBranches[idx]->InsertL(aFrom, aTo);
        }
    }

/**
Provides the set of targets which map from the specified source parameters.
@param aFrom The map source parameters.
@param aTo The map target parameter set.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::GetToL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const
    {
    __FLOG(_L8("CMap::GetToL - entry"));
    const TUint KFrom(Param(aFrom));
    if (KFrom == KMTPNotSpecified32)
        {
        // Null (zero) parameter acts as a wildcard.
        SelectTargetAllL(aFrom, aTo);
        }
    else if (Flags(iSubType) & ESubTypeFlagEnableDuplicates)
        {
        // Select 0 .. n matching targets.
        SelectTargetMatchingL(aFrom, aTo);
        }
    else
        {
        // Select 0 .. 1 matching targets.
        SelectTargetSingleL(aFrom, aTo);
        }
	__FLOG(_L8("CMap::GetToL - Exit"));        
    }
    
/**
Provides the map subtype code.
@return The map subtype code.
*/
TUint CMTPParserRouter::CMap::SubType() const
    {
    return iSubType;
    }

#ifdef __FLOG_ACTIVE
/**
Logs the map table entries (source and target) which match the specified source 
parameters.
@param aFrom The map source parameters.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/  
void CMTPParserRouter::CMap::FLOGMapL(RArray<TUint>& aFrom) const
    {
    if (Params(iSubType) == ESubTypeParams1)
        {
        // Node.
        const TUint KCount(iToNodes.Count());
        for (TUint i(0); (i < KCount); i++)
            {
            aFrom[ParamIdx(aFrom)] = iToNodes[i].iFrom;
            FLOGMapEntryL(aFrom, iToNodes[i].iTo);
            }
        }
    else 
        {
        // Branch.
        const TUint KCount(iToBranches.Count());
        for (TUint i(0); (i < KCount); i++)
            {
            const CMap& KBranch(*iToBranches[i]);
            aFrom[ParamIdx(aFrom)] = KBranch.iFrom;
            KBranch.FLOGMapL(aFrom);
            }
        }
    }
    
/**
Logs the specified source and target map table entry parameters.
@param aFrom The map source parameters.
@param aTo The map target parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::FLOGMapEntryL(const RArray<TUint>& aFrom, TUint aTo) const
    {
    __ASSERT_DEBUG((aFrom.Count() >= ParamsCount(iSubType)), User::Invariant());
    RBuf log;
    log.CleanupClosePushL();
    const TUint KParamsCount(aFrom.Count());
    const TUint KWidthFrom(8);
    const TUint KWidthTo(2);
    const TUint KLength((KParamsCount * 11) + 7);
    log.CreateL(KLength);
    for (TUint i(0); (i < KParamsCount); i++)
        {
        log.Append(_L("0x"));
        log.AppendNumFixedWidthUC(aFrom[i], EHex, KWidthFrom);
        log.Append(_L(" "));
        }
    log.Append(_L("-> 0x"));
    log.AppendNumFixedWidthUC(aTo, EHex, KWidthTo);
    __FLOG(log);
    CleanupStack::PopAndDestroy(&log);
    }
#endif

/**
CMTPParserRouter::CMap factory method.
@param aFrom The map source parameter.
@param aTo The map target parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
CMTPParserRouter::CMap* CMTPParserRouter::CMap::NewLC(TUint aFrom, TUint aSubType)
    {
    CMap* self(new(ELeave) CMap(aFrom, aSubType));
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }
    
/**
Constructor.
@param aFrom The map source parameter.
@param aTo The map target parameter.
*/
CMTPParserRouter::CMap::CMap(TUint aFrom, TUint aSubType) :
    iFrom(aFrom),
    iSubType(aSubType)
    {

    }
    
/**
Second-phase constructor.
*/
void CMTPParserRouter::CMap::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }
    
/**
Locates the map branch table index of the first map table entry matching the 
specified source parameter, using a binary search algorithm.
@param aFrom The map source parameter,
@return The map node table index of the first matching entry, or KErrNotFound 
if no matching entry is found.
*/
TInt CMTPParserRouter::CMap::BranchFind(TUint aFrom) const
    {
    return (iToBranches.FindInOrder(aFrom, ((Flags(iSubType) & ESubTypeFlagOrderDescending) ? BranchOrderFromKeyDescending : BranchOrderFromKeyAscending)));
    }
    
/**
Inserts a new map branch table with the specified source parameter.
@param aFrom The map source paramete.
@return The map branch table index of the new entry.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
TUint CMTPParserRouter::CMap::BranchInsertL(TUint aFrom)
    {
    CMap* branch(CMap::NewLC(aFrom, CMTPParserRouter::SubType(Index(iSubType), Flags(iSubType), (ParamsCount(iSubType) - 1))));
    TLinearOrder<CMap> KOrder((iSubType & ESubTypeFlagOrderDescending) ? BranchOrderFromDescending : BranchOrderFromAscending);
    iToBranches.InsertInOrderL(branch, KOrder);
    CleanupStack::Pop(branch);
    return iToBranches.FindInOrder(branch, KOrder);
    }
    
/**
Locates the map node table index of the first map table entry matching the 
specified source parameter, using a binary search algorithm.
@param aFrom The source parameter,
@return The map node table index of the first matching entry, or KErrNotFound 
if no matching entry is found.
*/
TInt CMTPParserRouter::CMap::NodeFind(TUint aFrom) const
    {
    return (iToNodes.FindInOrder(aFrom, ((Flags(iSubType) & ESubTypeFlagOrderDescending) ? NodeOrderFromKeyDescending : NodeOrderFromKeyAscending)));
    }
    
/**
Locates the map node table index of the  map table entry matching the specified
node, using a binary search algorithm.
@param aFrom The map node table entry.
@return The map node table index, or KErrNotFound if no matching entry is 
found.
*/
TInt CMTPParserRouter::CMap::NodeFind(const TMap& aNode) const
    {
    return iToNodes.FindInOrder(aNode, ((Flags(iSubType) & ESubTypeFlagOrderDescending) ? NodeOrderFromToDescending : NodeOrderFromToAscending));
    }
    
/**
Inserts the specified map node into the map node table.
@param aFrom The map node table entry.
@return The map node table index of the new entry.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
TUint CMTPParserRouter::CMap::NodeInsertL(const TMap& aMap)
    {   
    TLinearOrder<TMap> KOrder((iSubType & ESubTypeFlagOrderDescending) ? NodeOrderFromToDescending : NodeOrderFromToAscending);
    if (Flags(iSubType) & ESubTypeFlagEnableDuplicates)
        {
        // Duplicates allowed, but discard completely duplicated routes.
        if (NodeFind(aMap) == KErrNotFound) 
            {
            iToNodes.InsertInOrderL(aMap, KOrder);
            }
        }
    else
        {
        TInt err(iToNodes.InsertInOrder(aMap, KOrder));
        if (err == KErrAlreadyExists)
            {
            Panic(EMTPPanicRoutingConflict);
            }
        else
            {
            User::LeaveIfError(err);
            }
        }
    const TInt KIdx(NodeFind(aMap));
    __ASSERT_DEBUG((KIdx != KErrNotFound), User::Invariant());
    return KIdx;
    }
    
/**
Provides the source parameter value from the specified source parameter set 
appropriate to the parameter level of the map.
@param aFrom The map source parameter set.
@return The parameter value.
*/
TUint CMTPParserRouter::CMap::Param(const RArray<TUint>& aFrom) const
    {
    return (aFrom[ParamIdx(aFrom)]);
    }
    
/**
Provides the source parameter set index of the source parameter corresponding to the 
parameter level of the map.
@param aFrom The map source parameter set.
@return The parameter set index.
*/
TUint CMTPParserRouter::CMap::ParamIdx(const RArray<TUint>& aFrom) const
    {
    return (aFrom.Count() - ParamsCount(iSubType));
    }

/**
Selects all map targets at the parameter level of the map.
@param aFrom The map source parameter set.
@param aTo The matching target parameters.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::SelectTargetAllL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const
    {
    if (Params(iSubType) == ESubTypeParams1)
        {
        // Node.
        const TUint KCount(iToNodes.Count());
        for (TUint idx(0); (idx < KCount); idx++)
            {
            SelectTargetL(iToNodes[idx].iTo, aTo);
            }
        }
    else
        {
        // Branch.
        const TUint KCount(iToBranches.Count());
        for (TUint idx(0); (idx < KCount); idx++)
            {
            iToBranches[idx]->GetToL(aFrom, aTo);
            }
        }
    }

/**
Selects all map targets which match the specified source parameters.
@param aFrom The map source parameter set.
@param aTo The matching target parameters.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::SelectTargetMatchingL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const
    {
    __FLOG(_L8("CMap::SelectTargetMatchingL - entry"));
    const TUint KFrom(Param(aFrom));
    TInt idx(KErrNotFound);
    if (Params(iSubType) == ESubTypeParams1)
        {
        idx = iToNodes.SpecificFindInOrder(TMap(KFrom), ((iSubType & ESubTypeFlagOrderDescending) ? NodeOrderFromDescending : NodeOrderFromAscending), EArrayFindMode_First);                
        if (idx != KErrNotFound)
            {
            const TUint KCount(iToNodes.Count());
            while ((idx < KCount) && (iToNodes[idx].iFrom == KFrom))
                {
                SelectTargetL(iToNodes[idx++].iTo, aTo);
                }
            }
        }
    else
        {
        CMap* from(CMap::NewLC(KFrom, iSubType));
        idx = iToBranches.SpecificFindInOrder(from, ((iSubType & ESubTypeFlagOrderDescending) ? BranchOrderFromDescending : BranchOrderFromAscending), EArrayFindMode_First);
        CleanupStack::PopAndDestroy(from);
        const TUint KCount(iToBranches.Count());
        while ((idx < KCount) && (iToBranches[idx]->From() == KFrom))
            {
            iToBranches[idx++]->GetToL(aFrom, aTo);
            }
        }
	__FLOG(_L8("CMap::SelectTargetMatchingL - exit"));        
    }

/**
Selects the first map target which matches the specified source parameters.
@param aFrom The map source parameter set.
@param aTo The matching target parameters.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::CMap::SelectTargetSingleL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const
    {
    const TUint KFrom(Param(aFrom));
    TInt idx(KErrNotFound);
    if (Params(iSubType) == ESubTypeParams1)
        {
        idx = NodeFind(KFrom);
        if (idx != KErrNotFound)
            {
            SelectTargetL(iToNodes[idx].iTo, aTo);
            }
        }
    else
        {
        idx = BranchFind(KFrom);
        if (idx != KErrNotFound)
            {
            iToBranches[idx]->GetToL(aFrom, aTo);
            }
        }
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::CMap 
branch map objects based on ascending map source parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::BranchOrderFromAscending(const CMap& aL, const CMap& aR)
    {
    return (aL.iFrom - aR.iFrom);
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::CMap 
branch map objects based on descending map source parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A positive value, if the first 
object is less than the second, or; A negative value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::BranchOrderFromDescending(const CMap& aL, const CMap& aR)
    {
    return (aR.iFrom - aL.iFrom);
    }
    
/**
Implements a map source parameter key identity relation for 
@see CMTPParserRouter::CMap branch map objects based on ascending key order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::BranchOrderFromKeyAscending(const TUint* aL, const CMap& aR)
    {
    return (*aL - aR.iFrom);
    }
    
/**
Implements a map source parameter key identity relation for 
@see CMTPParserRouter::CMap branch map objects based on descending key order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A positive value, if the first 
object is less than the second, or; A negative value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::BranchOrderFromKeyDescending(const TUint* aL, const CMap& aR)
    {
    return (aR.iFrom - *aL);
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::TMap 
node map objects based on ascending map source parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromAscending(const TMap& aL, const TMap& aR)
    {
    return (aL.iFrom - aR.iFrom);
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::TMap 
node map objects based on descending map source parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A positive value, if the first 
object is less than the second, or; A negative value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromDescending(const TMap& aL, const TMap& aR)
    {
    return (aR.iFrom - aL.iFrom);
    }
    
/**
Implements a map source parameter key identity relation for 
@see CMTPParserRouter::TMap node map objects based on ascending key order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromKeyAscending(const TUint* aL, const TMap& aR)
    {
    return (*aL - aR.iFrom);
    }
    
/**
Implements a map source parameter key identity relation for 
@see CMTPParserRouter::TMap node map objects based on descending key order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A positive value, if the first 
object is less than the second, or; A negative value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromKeyDescending(const TUint* aL, const TMap& aR)
    {
    return (aR.iFrom - *aL);
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::CMap 
branch map objects based on ascending map source and target parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromToAscending(const TMap& aL, const TMap& aR)
    {
    TInt ret(0);
    if (aL.iFrom != aR.iFrom)
        {
        ret = (aL.iFrom - aR.iFrom);
        }
    else
        {
        ret = (aL.iTo - aR.iTo);
        }
    return ret;
    }
    
/**
Implements an @see TLinearOrder relation for @see CMTPParserRouter::CMap 
branch map objects based on descending map source and target parameter order.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A positive value, if the first 
object is less than the second, or; A negative value, if the first object is 
greater than the second.
*/
TInt CMTPParserRouter::CMap::NodeOrderFromToDescending(const TMap& aL, const TMap& aR)
    {
    TInt ret(0);
    if (aL.iFrom != aR.iFrom)
        {
        ret = aR.iFrom - aL.iFrom;
        }
    else
        {
        ret = aR.iTo - aL.iTo;
        }
    return ret;
    }

/**
Constructor.
*/
CMTPParserRouter::CMTPParserRouter()
    {
    
    }
    
/**
Second-phase constructor.
*/
void CMTPParserRouter::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL, Entry"));
    iSingletons.OpenL();
    __FLOG(_L8("ConstructL, Exit"));
    }
    
/**
Provides the set of @see TMTPSupportCategory codes which comprise each of the 
lookup parameters for the specified routing sub-type (routing map).
@param aSubType The routing sub-type identifier.
@param aP1Codes On exit, the set of @see TMTPSupportCategory codes which 
comprise the first lookup parameter. This set will be empty if the routing
sub-type implements fewer than one parameter.
@param aP2Codes On exit, the set of @see TMTPSupportCategory codes which 
comprise the second lookup parameter. This set will be empty if the routing
sub-type implements fewer than two parameters.
@param aP2Codes On exit, the set of @see TMTPSupportCategory codes which 
comprise the third lookup parameter. This set will be empty if the routing
sub-type implements fewer than three parameters.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetMapParameterIdsL(TUint aSubType, RArray<TUint>& aP1Codes, RArray<TUint>& aP2Codes, RArray<TUint>& aP3Codes)
    {
    aP1Codes.Reset();
    aP2Codes.Reset();
    aP3Codes.Reset();
    
    switch (aSubType)
        {
    case ESubTypeDevicePropCode:
        aP1Codes.AppendL(EDeviceProperties);
        break;
        
    case ESubTypeObjectPropCode:
        aP1Codes.AppendL(EObjectProperties);
        break;
        
    case ESubTypeOperationCode:
        aP1Codes.AppendL(EOperations);
        break;
        
    case ESubTypeStorageType:
        aP1Codes.AppendL(EStorageSystemTypes);
        break;
        
    case ESubTypeFormatCodeFormatSubcode:
        aP1Codes.AppendL(EObjectCaptureFormats);
        aP1Codes.AppendL(EObjectPlaybackFormats);
        aP2Codes.AppendL(EAssociationTypes);
        break;
        
    case ESubTypeFormatCodeOperationCode:
        aP1Codes.AppendL(EObjectCaptureFormats);
        aP1Codes.AppendL(EObjectPlaybackFormats);
        aP2Codes.AppendL(EOperations);
        break;
        
    case ESubTypeStorageTypeOperationCode:
        aP1Codes.AppendL(EStorageSystemTypes);
        aP2Codes.AppendL(EOperations);
        break;
        
    case ESubTypeFormatCodeFormatSubcodeStorageType:
        aP1Codes.AppendL(EObjectCaptureFormats);
        aP1Codes.AppendL(EObjectPlaybackFormats);
        aP2Codes.AppendL(EAssociationTypes);
        aP3Codes.AppendL(EStorageSystemTypes);
        break;
        
    case ESubTypeServiceIDOperationCode:
        aP1Codes.AppendL(EServiceIDs);
        break;   
        
    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
    }

/**
Selects the specified data provider target identifier by appending it to the 
set of selected targets. Each target identifier may only appear once in the 
set of selected targets. A selected target which is already a member of the 
selected set will be replaced to ensure that targets are dispatched in order
of most recent selection.
@param aTarget The data provider target identifier.
@param aTargets The set of selected targets.
@leave One of the system wide error codes, if a general processing error 
occurs.

*/    
void CMTPParserRouter::SelectTargetL(TUint aTarget, RArray<TUint>& aTargets)
    {
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("SelectTargetL, Entry"));
    TInt idx(aTargets.Find(aTarget));
    if (idx != KErrNotFound)
        {
        aTargets.Remove(idx);
        }
    aTargets.AppendL(aTarget);
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("SelectTargetL, Exit"));
    }

/**
Configures (loads) the specified one lookup parameter routing sub-type map. The
map is constructed by interrogating each data provider in turn and building a 
set of map table entries which resolve each supported @see TMTPSupportCategory 
code to its data provider (target) identifier.
@param aSubType The routing sub-type identifier.
@param aP1Codes The set of @see TMTPSupportCategory codes which comprise the 
first lookup parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::Configure1ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes)
    {
    CMap& map(*iMaps[aSubType & ESubTypeIndexMask]);
    RArray<TUint> from;
    CleanupClosePushL(from);
    map.InitParamsL(from);
    
    const TUint KCount(iSingletons.DpController().Count());
    for (TUint d(0); (d < KCount); d++)
        {
        CMTPDataProvider& dp(iSingletons.DpController().DataProviderByIndexL(d));
        __FLOG(_L8(""));
        __FLOG_VA((_L8("Creating DP %02d Table 0x%08X Entries"), dp.DataProviderId(), aSubType));
        __FLOG(_L8("---------------------------------------")); 
        
        RArray<TUint> p1s;
        CleanupClosePushL(p1s);
        GetConfigParametersL(dp, aP1Codes, p1s);
        const TUint KCountP1s(p1s.Count());
        for (TUint p1(0); (p1 < KCountP1s); p1++)
            {
            const TUint KP1(p1s[p1]);
            from[EParam1] = KP1;
            map.InsertL(from, dp.DataProviderId());
            }
        CleanupStack::PopAndDestroy(&p1s);
        __FLOG(_L8(""));
        }
    CleanupStack::PopAndDestroy(&from);
    }

/**
Configures (loads) the specified two lookup parameter routing sub-type map. The
map is constructed by interrogating each data provider in turn and building a 
set of map table entries which resolve each combination of supported 
@see TMTPSupportCategory codes to its data provider (target) identifier.
@param aSubType The routing sub-type identifier.
@param aP1Codes The set of @see TMTPSupportCategory codes which comprise the 
first lookup parameter.
@param aP2Codes The set of @see TMTPSupportCategory codes which comprise the 
second lookup parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::Configure2ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes, const RArray<TUint>& aP2Codes)
    {
    CMap& map(*iMaps[aSubType & ESubTypeIndexMask]);
    RArray<TUint> from;
    CleanupClosePushL(from);
    map.InitParamsL(from);
    
    const TUint KCountDps(iSingletons.DpController().Count());
    for (TUint d(0); (d < KCountDps); d++)
        {
        CMTPDataProvider& dp(iSingletons.DpController().DataProviderByIndexL(d));
        __FLOG(_L8(""));
        __FLOG_VA((_L8("Creating DP %02d Table 0x%08X Entries"), dp.DataProviderId(), aSubType));
        __FLOG(_L8("---------------------------------------"));
        
        RArray<TUint> p1s;
        CleanupClosePushL(p1s);
        GetConfigParametersL(dp, aP1Codes, p1s);
        const TUint KCountP1s(p1s.Count());
        for (TUint p1(0); (p1 < KCountP1s); p1++)
            {
            const TUint KP1(p1s[p1]);
            from[EParam1] = KP1;
            if ((aSubType == ESubTypeFormatCodeFormatSubcode) && 
                (KP1 != EMTPFormatCodeAssociation))
                {
                from[EParam2] = EMTPAssociationTypeUndefined;
                map.InsertL(from, dp.DataProviderId());
                }
            else
                {
                RArray<TUint> p2s;
                CleanupClosePushL(p2s);
                GetConfigParametersL(dp, aP2Codes, p2s);
                const TUint KCountP2s(p2s.Count());
                for (TUint p2(0); (p2 < KCountP2s); p2++)
                    {
                    const TUint KP2(p2s[p2]);
                    from[EParam2] = KP2;
                    map.InsertL(from, dp.DataProviderId());
                    }
                CleanupStack::PopAndDestroy(&p2s);
                }
            }
        CleanupStack::PopAndDestroy(&p1s);
        __FLOG(_L8(""));
        } 
    CleanupStack::PopAndDestroy(&from);
    }

/**
Configures (loads) the specified three lookup parameter routing sub-type map. 
The map is constructed by interrogating each data provider in turn and building
a set of map table entries which resolve each combination of supported 
@see TMTPSupportCategory codes to its data provider (target) identifier.
@param aSubType The routing sub-type identifier.
@param aP1Codes The set of @see TMTPSupportCategory codes which comprise the 
first lookup parameter.
@param aP2Codes The set of @see TMTPSupportCategory codes which comprise the 
second lookup parameter.
@param aP3Codes The set of @see TMTPSupportCategory codes which comprise the 
third lookup parameter.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::Configure3ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes, const RArray<TUint>& aP2Codes, const RArray<TUint>& aP3Codes)
    {
    CMap& map(*iMaps[aSubType & ESubTypeIndexMask]);
    RArray<TUint> from;
    CleanupClosePushL(from);
    map.InitParamsL(from);
    
    const TUint KCount(iSingletons.DpController().Count());
    for (TUint d(0); (d < KCount); d++)
        {
        CMTPDataProvider& dp(iSingletons.DpController().DataProviderByIndexL(d));
        __FLOG(_L8(""));
        __FLOG_VA((_L8("Creating DP %02d Table 0x%08X Entries"), dp.DataProviderId(), aSubType));
        __FLOG(_L8("---------------------------------------"));
        
        RArray<TUint> p1s;
        CleanupClosePushL(p1s);
        GetConfigParametersL(dp, aP1Codes, p1s);
        const TUint KCountP1s(p1s.Count());
        for (TUint p1(0); (p1 < KCountP1s); p1++)
            {
            RArray<TUint> p3s;
            CleanupClosePushL(p3s);
            const TUint KP1(p1s[p1]);
            from[EParam1] = KP1;
            if ((aSubType == ESubTypeFormatCodeFormatSubcodeStorageType) && 
                (KP1 != EMTPFormatCodeAssociation))
                {
                from[EParam2] = EMTPAssociationTypeUndefined;
                GetConfigParametersL(dp, aP3Codes, p3s);
                const TUint KCountP3s(p3s.Count());
                for (TUint p3(0); (p3 < KCountP3s); p3++)
                    {
                    const TUint KP3(p3s[p3]);
                    from[EParam3] = KP3;
                    map.InsertL(from, dp.DataProviderId());
                    }
                }
            else
                {
                RArray<TUint> p2s;
                CleanupClosePushL(p2s);
                GetConfigParametersL(dp, aP2Codes, p2s);
                const TUint KCountP2s(p2s.Count());
                for (TUint p2(0); (p2 < KCountP2s); p2++)
                    {
                    const TUint KP2(p2s[p2]);
                    from[EParam2] = KP2;
                    GetConfigParametersL(dp, aP3Codes, p3s);
                    const TUint KCountP3s(p3s.Count());
                    for (TUint p3(0); (p3 < KCountP3s); p3++)
                        {
                        const TUint KP3(p3s[p3]);
                        from[EParam3] = KP3;
                        map.InsertL(from, dp.DataProviderId());
                        }
                    }
                CleanupStack::PopAndDestroy(&p2s);
                }
            CleanupStack::PopAndDestroy(&p3s);
            }
        CleanupStack::PopAndDestroy(&p1s);
        __FLOG(_L8(""));
        }
    CleanupStack::PopAndDestroy(&from);
    }

/**
Obtains the set of supported @see TMTPSupportCategory code parameter values 
supported by the specified data provider.
@param aDp The data provider.
@param aCodes The set of @see TMTPSupportCategory codes which comprise the 
parameter.
@param aParams On exit, the set of supported parameter values.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetConfigParametersL(const CMTPDataProvider& aDp, const RArray<TUint>& aCodes, RArray<TUint>& aParams) const
    {
    aParams.Reset();
    const TUint KCountCodes(aCodes.Count());
    for (TUint c(0); (c < KCountCodes); c++)
        {
        const RArray<TUint>& KParams(aDp.SupportedCodes(static_cast<TMTPSupportCategory>(aCodes[c])));
        const TUint KCountParams(KParams.Count());
        for (TUint p(0); (p < KCountParams); p++)
            {
            if(( EServiceIDs == aCodes[c] )&&( iSingletons.ServiceMgr().IsSupportedService( KParams[p] )) )
                {
                __FLOG_1(_L8("GetConfigParametersL, abstract service id = %d"), KParams[p]);
                continue;
                }
            TInt err(aParams.InsertInOrder(KParams[p]));
            if ((err != KErrNone) && (err != KErrAlreadyExists))
                {
                User::Leave(err);
                }
            }
        }
    }

/**
Provides the set of operation parameter routing and validation sub-types to be 
executed against each of the specified operation routing parameter data.
@param aParams The set of operation routing parameter data.
@param aRoutingSubTypes On exit, the set of routing sub-types to be executed.
@param aValidationSubTypes On exit, the set of validation sub-types to be executed. 
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetRoutingSubTypesL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesL, Entry"));
    __ASSERT_DEBUG((aParams.Count() > 0), User::Invariant());
    aRoutingSubTypes.Reset();
    aValidationSubTypes.Reset();
    
    TRoutingParameters& params1(aParams[0]);
    if (params1.Param(TRoutingParameters::EFlagInvalid))
        {
        SelectSubTypeRoutingL(ESubTypeDpDevice, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else
        {
        const TUint KOpCode(params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode));
        const TUint KRoutingTypes(params1.Param(TRoutingParameters::EFlagRoutingTypes));
        switch (KOpCode)
            {
        case EMTPOpCodeGetDeviceInfo:
        case EMTPOpCodeOpenSession:
        case EMTPOpCodeCloseSession:
        case EMTPOpCodeGetStorageIDs:
        case EMTPOpCodeGetNumObjects:
        case EMTPOpCodeGetObjectHandles:
        case EMTPOpCodeFormatStore:
        case EMTPOpCodeResetDevice:
        case EMTPOpCodeSelfTest:
        case EMTPOpCodePowerDown:
            if (KRoutingTypes & ETypeFramework)
                {
                SelectSubTypeRoutingL(ESubTypeDpDevice, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;
            
        case EMTPOpCodeDeleteObject:
            GetRoutingSubTypesDeleteRequestL(aParams, aRoutingSubTypes, aValidationSubTypes);
            break;
            
        case EMTPOpCodeGetObjectPropList:
            GetRoutingSubTypesGetObjectPropListRequestL(aParams, aRoutingSubTypes, aValidationSubTypes);
            break;

        case EMTPOpCodeSetObjectPropList:
            if (KRoutingTypes & ETypeFramework)
                {
                SelectSubTypeRoutingL(ESubTypeDpProxy, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            else if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
                SelectSubTypeValidationL(ESubTypeObjectPropCode, aValidationSubTypes);
                }
            break;
            
        case EMTPOpCodeSendObjectInfo:
            if (KRoutingTypes & ETypeFramework)
                {
                SelectSubTypeRoutingL(ESubTypeDpProxy, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            else if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeFormatCodeFormatSubcodeStorageType, aRoutingSubTypes, aValidationSubTypes, aParams); 
                }
            break;
            
        case EMTPOpCodeGetDevicePropDesc:
        case EMTPOpCodeGetDevicePropValue:
        case EMTPOpCodeSetDevicePropValue:
        case EMTPOpCodeResetDevicePropValue:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeDevicePropCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;

        case EMTPOpCodeGetObjectPropsSupported:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeFormatCodeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;

        case EMTPOpCodeSkip:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                params1.SetParam(TRoutingParameters::EFlagRoutingTypes, (KRoutingTypes | ETypeFlagSingleTarget));
                }
            break;

        case EMTPOpCodeGetObjectPropDesc:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeFormatCodeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                SelectSubTypeValidationL(ESubTypeObjectPropCode, aValidationSubTypes);
                params1.SetParam(TRoutingParameters::EFlagRoutingTypes, (KRoutingTypes | ETypeFlagSingleTarget));
                }
            break;

        case EMTPOpCodeGetInterdependentPropDesc:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeFormatCodeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;
            
        case EMTPOpCodeInitiateCapture:
        case EMTPOpCodeInitiateOpenCapture:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeFormatCodeFormatSubcodeStorageType, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;
            
        case EMTPOpCodeSendObjectPropList:
            GetRoutingSubTypesSendObjectPropListRequestL(aParams, aRoutingSubTypes, aValidationSubTypes);
            break;
            
        case EMTPOpCodeMoveObject:    
        case EMTPOpCodeCopyObject:
        	GetRoutingSubTypesCopyMoveRequestL(aParams, aRoutingSubTypes, aValidationSubTypes);
        	break;
        case EMTPOpCodeGetObjectInfo:
        case EMTPOpCodeGetObject:
        case EMTPOpCodeGetThumb:
        case EMTPOpCodeGetPartialObject:
        case EMTPOpCodeGetObjectReferences:
        case EMTPOpCodeSetObjectReferences:
        case EMTPOpCodeUpdateObjectPropList :
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;
            
        case EMTPOpCodeGetObjectPropValue:
        case EMTPOpCodeSetObjectPropValue:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
                SelectSubTypeValidationL(ESubTypeObjectPropCode, aValidationSubTypes);
                }
            break;
            
        case EMTPOpCodeGetStorageInfo:
            if (KRoutingTypes & ETypeOperationParameter)
                {
                SelectSubTypeRoutingL(ESubTypeOwnerStorage, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;

        case EMTPOpCodeSendObject:
        case EMTPOpCodeTerminateOpenCapture:
            if (KRoutingTypes & ETypeRequestRegistration)
                {
                SelectSubTypeRoutingL(ESubTypeRequestRegistration, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            break;

        case EMTPOpCodeGetServiceInfo:     
        case EMTPOpCodeGetServiceCapabilities:
        case EMTPOpCodeGetServicePropDesc:
        case EMTPOpCodeGetServicePropList:
        case EMTPOpCodeSetServicePropList:
        case EMTPOpCodeDeleteServicePropList:
            {
            if ( iSingletons.ServiceMgr().IsSupportedService( params1.Param(TRoutingParameters::EParamServiceId) ) )
                {
                SelectSubTypeRoutingL(ESubTypeDpDevice, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            else
                {
                SelectSubTypeRoutingL(ESubTypeServiceIDOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            }
            break;
        case EMTPOpCodeDeleteObjectPropList :
            {
            GetRoutingSubTypesDeleteObjectPropListL( aParams, aRoutingSubTypes, aValidationSubTypes );
            }
            break;
        case EMTPOpCodeGetFormatCapabilities:
            {
            GetRoutingSubTypesGetFormatCapabilitiesL(aParams, aRoutingSubTypes, aValidationSubTypes);
            }
            break;
        case EMTPOpCodeSetObjectProtection:
            SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
            break;
        default:
            if (KRoutingTypes & ETypeRequestRegistration)
                {
                SelectSubTypeRoutingL(ESubTypeRequestRegistration, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            if (KRoutingTypes & ETypeFramework)
                {
                SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                params1.SetParam(TRoutingParameters::EFlagRoutingTypes, (KRoutingTypes | ETypeFlagSingleTarget));
                }
            break;
            }
        }
    __FLOG(_L8("GetRoutingSubTypesL, Exit"));
    }

/**
Provides the set of operation parameter routing and validation sub-types to be 
executed against each of the specified MTP DeleteObject operation routing 
parameter data.
@param aParams The set of operation routing parameter data.
@param aRoutingSubTypes On exit, the set of routing sub-types to be executed.
@param aValidationSubTypes On exit, the set of validation sub-types to be executed. 
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetRoutingSubTypesDeleteRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesDeleteRequestL, Entry"));
    TRoutingParameters& params1(aParams[0]);
    __ASSERT_DEBUG((params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode) == EMTPOpCodeDeleteObject), User::Invariant());
    
    const TUint KObjectHandle(params1.Param(TRoutingParameters::EParamObjectHandle));
    const TUint KObjectFormatCode(params1.Param(TRoutingParameters::EParamFormatCode));
    if (KObjectHandle == KMTPHandleAll)
        {
        if (KObjectFormatCode == KMTPFormatsAll || KObjectFormatCode == EMTPFormatCodeAssociation)
            {
            /* 
            Deleting all objects of all formats. Force the format to 
            Association to ensure that objects are deleted in the correct
            order.
            */
            if (KObjectFormatCode == KMTPFormatsAll)
            	{
            	params1.SetParam(TRoutingParameters::EParamFormatCode, EMTPFormatCodeAssociation);
            	}
                SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
            }
        else if ( KObjectFormatCode == EMTPFormatCodeUndefined )
            {
            SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
            }
        SelectSubTypeRoutingL(ESubTypeFormatCodeFormatSubcode, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else if (KObjectHandle != KMTPHandleNone)
        {
        if (KObjectFormatCode == EMTPFormatCodeAssociation)
            {
            SelectSubTypeRoutingL(ESubTypeStorageTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
            }
        SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    __FLOG(_L8("GetRoutingSubTypesDeleteRequestL, Exit"));
    }

/**
Provides the set of operation parameter routing and validation sub-types to be 
executed against each of the specified MTP CopyObject and MoveObject operation routing 
parameter data.
@param aParams The set of operation routing parameter data.
@param aRoutingSubTypes On exit, the set of routing sub-types to be executed.
@param aValidationSubTypes On exit, the set of validation sub-types to be executed. 
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetRoutingSubTypesCopyMoveRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
	{
	__FLOG(_L8("GetRoutingSubTypesCopyMoveRequestL, Entry"));
	const TUint KObjectFormatCode(aParams[0].Param(TRoutingParameters::EParamFormatCode));
	if (KObjectFormatCode == EMTPFormatCodeAssociation)
        {
        SelectSubTypeRoutingL(ESubTypeStorageTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else
    	{
    	SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);    
    	}
	__FLOG(_L8("GetRoutingSubTypesCopyMoveRequestL, Exit"));    
	}
/**
Provides the set of operation parameter routing and validation sub-types to be 
executed against each of the specified MTP GetObjectPropList operation routing 
parameter data.
@param aParams The set of operation routing parameter data.
@param aRoutingSubTypes On exit, the set of routing sub-types to be executed.
@param aValidationSubTypes On exit, the set of validation sub-types to be executed. 
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetRoutingSubTypesGetObjectPropListRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesGetObjectPropListRequestL, Entry"));
    TRoutingParameters& params1(aParams[0]);
    __ASSERT_DEBUG((params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode) == EMTPOpCodeGetObjectPropList), User::Invariant());
    if (params1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeOperationParameter)
        {
        const TUint KObjectHandle(params1.Param(TRoutingParameters::EParamObjectHandle));
        const TUint KObjectFormatCode(params1.Param(TRoutingParameters::EParamFormatCode));
        const TUint KObjectFormatSubCode(params1.Param(TRoutingParameters::EParamFormatSubCode));
        const TUint KObjectPropCode(params1.Param(TRoutingParameters::EParamObjectPropCode));
        const TUint KDepth(params1.Request().Uint32(TMTPTypeRequest::ERequestParameter5));
        if ( KDepth==0 )
            {
            if ((KObjectHandle == KMTPHandleAll) || 
                (KObjectHandle == KMTPHandleAllRootLevel))
                {
                SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                }
            else if (KObjectHandle != KMTPHandleNone)
                {
                 if( (KObjectFormatCode == EMTPFormatCodeAssociation) && (KObjectFormatSubCode == EMTPAssociationTypeGenericFolder) )
                     {
                         if ( params1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeFramework )
                         {
                         SelectSubTypeRoutingL(ESubTypeDpProxy, aRoutingSubTypes, aValidationSubTypes, aParams);
                         }
                         else
                         {
                         SelectSubTypeRoutingL(ESubTypeStorageTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
                         SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
                         }
                     }
                 else
                     {
                     SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
                     }
                 } 
            }
        else
            {
            SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
            }
        
        }
    __FLOG(_L8("GetRoutingSubTypesGetObjectPropListRequestL, Exit"));
    }

/**
Provides the set of operation parameter routing and validation sub-types to be 
executed against each of the specified MTP SendObjectPropList operation routing 
parameter data.
@param aParams The set of operation routing parameter data.
@param aRoutingSubTypes On exit, the set of routing sub-types to be executed.
@param aValidationSubTypes On exit, the set of validation sub-types to be executed. 
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::GetRoutingSubTypesSendObjectPropListRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesSendObjectPropListRequestL, Entry"));
    TRoutingParameters& params1(aParams[0]);
    __ASSERT_DEBUG((params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode) == EMTPOpCodeSendObjectPropList), User::Invariant());
    const TUint KRoutingTypes(params1.Param(TRoutingParameters::EFlagRoutingTypes));
    if ((KRoutingTypes & ETypeFramework) &&
        (params1.Param(TRoutingParameters::EParamFormatCode) == EMTPFormatCodeAssociation))
        {
        SelectSubTypeRoutingL(ESubTypeDpProxy, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else if (KRoutingTypes & ETypeOperationParameter)
        {
        SelectSubTypeRoutingL(ESubTypeFormatCodeFormatSubcodeStorageType, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    __FLOG(_L8("GetRoutingSubTypesSendObjectPropListRequestL, Exit"));
    }

void CMTPParserRouter::GetRoutingSubTypesDeleteObjectPropListL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesDeleteObjectPropListL, Entry"));
    TRoutingParameters& params1(aParams[0]);
    
    __ASSERT_DEBUG((params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode) == EMTPOpCodeDeleteObjectPropList), User::Invariant());
    
    const TUint KObjectHandle(params1.Param(TRoutingParameters::EParamObjectHandle));
    if ((KObjectHandle == KMTPHandleAll) || (KObjectHandle == KMTPHandleNone))
        {
        SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else 
        {
        SelectSubTypeRoutingL(ESubTypeOwnerObject, aRoutingSubTypes, aValidationSubTypes, aParams);
        }

    __FLOG(_L8("GetRoutingSubTypesDeleteObjectPropListL, Exit"));
    }

void CMTPParserRouter::GetRoutingSubTypesGetFormatCapabilitiesL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("GetRoutingSubTypesGetFormatCapabilities, Entry"));
    TRoutingParameters& params1(aParams[0]);
    
    __ASSERT_DEBUG((params1.Request().Uint16(TMTPTypeRequest::ERequestOperationCode) == EMTPOpCodeGetFormatCapabilities), User::Invariant());
    
    if( params1.Param(TRoutingParameters::EParamFormatCode) == KMTPFormatsAll)
        {
        SelectSubTypeRoutingL(ESubTypeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    else if ( params1.Param(TRoutingParameters::EFlagRoutingTypes) & ETypeOperationParameter)
        {
        SelectSubTypeRoutingL(ESubTypeFormatCodeOperationCode, aRoutingSubTypes, aValidationSubTypes, aParams);
        }
    
    __FLOG(_L8("GetRoutingSubTypesGetFormatCapabilities, Exit"));
    }

/**
Parses the specified MTP operation request dataset to extract the specified 
parameter value together with any applicable meta-data. The parameter value 
will be extracted only if not null (0x00000000).  
@param aParam The operation request dataset parameter identifier.
@param aType The operation request dataset parameter type.
@param aParams The operation routing parameter data, updated on exit with the 
parameter value together with any associated meta-data.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/    
void CMTPParserRouter::ParseOperationRequestParameterL(TMTPTypeRequest::TElements aParam, TRoutingParameters::TParameterType aType, TRoutingParameters& aParams) const
    {
    __FLOG(_L8("ParseOperationRequestParameterL, Entry"));
    const TUint32 KParam(aParams.Request().Uint32(aParam));    
    __FLOG_VA((_L8("Parameter %d = 0x%08X"), (aParam - TMTPTypeRequest::ERequestParameter1 + 1), KParam));
    
    // Parse out the parameter value if a non-null value is present.
    if (KParam != KMTPNotSpecified32)
        {
        aParams.SetParam(aType, KParam);
        }
    
    // Extract any applicable meta-data.
    switch (aType)
        {
    case TRoutingParameters::EParamStorageId:
        {
        CMTPStorageMgr& storages(iSingletons.StorageMgr());
        if (KParam == KMTPStorageDefault) 
            {
            aParams.SetParam(TRoutingParameters::EParamStorageSystemType, storages.StorageL(storages.DefaultStorageId()).Uint(CMTPStorageMetaData::EStorageSystemType));
            }
        else if (storages.ValidStorageId(KParam))
            {
            aParams.SetParam(TRoutingParameters::EParamStorageSystemType, storages.StorageL(KParam).Uint(CMTPStorageMetaData::EStorageSystemType));
            }
        else
            {
            aParams.SetParam(TRoutingParameters::EFlagInvalid, ETrue);
            }
        }
        break;
        
    case TRoutingParameters::EParamObjectHandle:
        if ((KParam != KMTPHandleAll) && (KParam != KMTPHandleAllRootLevel))
            {
            CMTPObjectMetaData* obj(CMTPObjectMetaData::NewLC());
            if (!iSingletons.ObjectMgr().ObjectL(aParams.Param(CMTPParserRouter::TRoutingParameters::EParamObjectHandle), *obj))
                {
                // Object does not exist.
                aParams.SetParam(TRoutingParameters::EFlagInvalid, ETrue);
                }
            else
                {
                aParams.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatCode, obj->Uint(CMTPObjectMetaData::EFormatCode));
                aParams.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, obj->Uint(CMTPObjectMetaData::EFormatSubCode));
                aParams.SetParam(CMTPParserRouter::TRoutingParameters::EParamStorageSystemType, iSingletons.StorageMgr().StorageL(obj->Uint(CMTPObjectMetaData::EStorageId)).Uint(CMTPStorageMetaData::EStorageSystemType));            
                }
            CleanupStack::PopAndDestroy(obj);
            }
        break;
        
    default:
        break;
        }    
    __FLOG(_L8("ParseOperationRequestParameterL, Exit"));
    }

/**
Resolves set of zero or more routing targets using the specified lookup 
parameter based routing sub-type.
@param aRoutingSubType The routing sub-type.
@param aParams The operation routing parameter data.
@param aTargets On exit, the seto of resolved routing targets.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::RouteOperationRequestNParametersL(TUint aRoutingSubType, const TRoutingParameters& aParams, RArray<TUint>& aTargets) const
    {
    __FLOG(_L8("RouteOperationRequestNParametersL, Entry"));
    __FLOG_VA((_L8("Routing Sub-type = 0x%08X"), aRoutingSubType));
    
    // Build the set of map source parameter values.
    RArray<TUint> from;
    CleanupClosePushL(from);
    switch (aRoutingSubType)
        {
    case ESubTypeDevicePropCode:
        from.AppendL(aParams.Param(TRoutingParameters::EParamDevicePropCode));
        break;
        
    case ESubTypeObjectPropCode:
        from.AppendL(aParams.Param(TRoutingParameters::EParamObjectPropCode));
        break;
        
    case ESubTypeOperationCode:
        from.AppendL(aParams.Request().Uint16(TMTPTypeRequest::ERequestOperationCode));
        break;
        
    case ESubTypeStorageType:
        from.AppendL(aParams.Param(TRoutingParameters::EParamStorageSystemType));
        break;
        
    case ESubTypeFormatCodeFormatSubcode:
        from.AppendL(aParams.Param(TRoutingParameters::EParamFormatCode));
        from.AppendL(aParams.Param(TRoutingParameters::EParamFormatSubCode));
        break;
        
    case ESubTypeFormatCodeOperationCode:
        from.AppendL(aParams.Param(TRoutingParameters::EParamFormatCode));
        from.AppendL(aParams.Request().Uint16(TMTPTypeRequest::ERequestOperationCode));
        break;
        
    case ESubTypeStorageTypeOperationCode:
        from.AppendL(aParams.Param(TRoutingParameters::EParamStorageSystemType));
        from.AppendL(aParams.Request().Uint16(TMTPTypeRequest::ERequestOperationCode));
        break;
        
    case ESubTypeFormatCodeFormatSubcodeStorageType:
        from.AppendL(aParams.Param(TRoutingParameters::EParamFormatCode));
        from.AppendL(aParams.Param(TRoutingParameters::EParamFormatSubCode));
        from.AppendL(aParams.Param(TRoutingParameters::EParamStorageSystemType));
        break;
    case ESubTypeServiceIDOperationCode :
        {
        from.AppendL(aParams.Param(TRoutingParameters::EParamServiceId));
        }
        break;   
        
    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
    
    // Resolve the map target parameter set.
    iMaps[Index(aRoutingSubType)]->GetToL(from, aTargets);
    CleanupStack::PopAndDestroy(&from);
    __FLOG(_L8("RouteOperationRequestNParametersL, Exit"));
    }

/**
Resolves set of zero or more routing targets using the specified parameterless 
routing sub-type.
@param aRoutingSubType The routing sub-type.
@param aParams The operation routing parameter data.
@param aTargets On exit, the seto of resolved routing targets.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::RouteOperationRequest0ParametersL(TUint aRoutingSubType, const TRoutingParameters& aParams, RArray<TUint>& aTargets) const
    {
    __FLOG(_L8("RouteOperationRequest0ParametersL, Entry"));
    __FLOG_VA((_L8("Routing Sub-type = 0x%08X"), aRoutingSubType));
    TInt id(KErrNotFound);
    switch (aRoutingSubType)
        {
    case ESubTypeDpDevice:
        id = iSingletons.DpController().DeviceDpId();
        break;
        
    case ESubTypeDpProxy:
        id = iSingletons.DpController().ProxyDpId();
        break;
        
    case ESubTypeOwnerObject:
        id = iSingletons.ObjectMgr().ObjectOwnerId(aParams.Param(TRoutingParameters::EParamObjectHandle));
        if ( EMTPOpCodeSetObjectProtection == aParams.Request().Uint16(TMTPTypeRequest::ERequestOperationCode))
            {
            if ( (EMTPFormatCodeScript!=aParams.Param(TRoutingParameters::EParamFormatCode)) && (EMTPFormatCodeEXIFJPEG!=aParams.Param(TRoutingParameters::EParamFormatCode)) )
                {
                id = iSingletons.DpController().FileDpId();
                }
            }
        break;
        
    case ESubTypeOwnerStorage:
        {
        CMTPStorageMgr& storages(iSingletons.StorageMgr());
        const TUint KStorageId(aParams.Param(TRoutingParameters::EParamStorageId));
        if (storages.LogicalStorageId(KStorageId))
            {
            id = storages.LogicalStorageOwner(KStorageId);
            }
        else
            {
            id = storages.PhysicalStorageOwner(KStorageId);
            }
        }
        break;
        
    case ESubTypeRequestRegistration:
        {
        CMTPSession& session(static_cast<CMTPSession&>(aParams.Connection().SessionWithMTPIdL(aParams.Request().Uint32(TMTPTypeRequest::ERequestSessionID))));
        id = session.RouteRequest(aParams.Request());
        }
        break;

    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
        
    if (id != KErrNotFound && iSingletons.DpController().IsDataProviderLoaded(id))
        {
        SelectTargetL(id, aTargets);
        }
    __FLOG(_L8("RouteOperationRequest0ParametersL, Exit"));
    }

/**
Provides a single suitable routing target for the specified request. A target 
is selected such that:

    1.	Any request that resolves to multiple targets will always be directed 
     	to the proxy data provider.
    2.	Any request that cannot be resolved to at least one target will always 
        be directed to the device data provider.

@param aRequest The operation request dataset of the MTP operation to be 
routed.
@param aConnection The MTP connection on which the operation request is being 
processed.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
TUint CMTPParserRouter::RoutingTargetL(const TMTPTypeRequest& aRequest, CMTPConnection& aConnection) const
    {
    __FLOG(_L8("RoutingTargetL, Entry"));    
    // Parse the operation request dataset.
    TRoutingParameters params(aRequest, static_cast<MMTPConnection&>(aConnection));
    ParseOperationRequestL(params);
      
    // Route the operation request.
    RArray<TUint> targets;
    CleanupClosePushL(targets);
    params.SetParam(TRoutingParameters::EFlagRoutingTypes, (ETypeFramework | ETypeOperationParameter | ETypeRequestRegistration));
    RouteOperationRequestL(params, targets);
    
    // Dispatch the operation request.
    TUint target(0);
    if (targets.Count() > 1)
        {
        target = iSingletons.DpController().ProxyDpId();
        }
    else
        {
        target = targets[0];
        }
    CleanupStack::PopAndDestroy(&targets);
    __FLOG(_L8("RoutingTargetL, Exit"));
    return target;
    }
    
/**
Selects the specified routing sub-type together with any applicable validation 
sub-types.
@param aSubType The selected routing sub-type.
@param aRoutingSubTypes The set of selected routing sub-types, updated on exit.
@param aValidationSubTypes The set of selected validation sub-types, 
potentially updated on exit.
@param aParams The set of operation routing parameter data.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::SelectSubTypeRoutingL(TRoutingSubType aSubType, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes, RArray<TRoutingParameters>& aParams) const
    {
    __FLOG(_L8("SelectSubTypeRoutingL, Entry"));
    __ASSERT_DEBUG((aRoutingSubTypes.Find(aSubType) == KErrNotFound), User::Invariant());
    aRoutingSubTypes.AppendL(aSubType);    
    switch (aSubType)
        {
    case ESubTypeDevicePropCode:
    case ESubTypeObjectPropCode:
    case ESubTypeStorageType:
    case ESubTypeOwnerObject:
    case ESubTypeOwnerStorage:
        SelectSubTypeValidationL(ESubTypeOperationCode, aValidationSubTypes);
        break;
        
    case ESubTypeFormatCodeFormatSubcode:
    case ESubTypeFormatCodeOperationCode:
    case ESubTypeFormatCodeFormatSubcodeStorageType:
        {
        TRoutingParameters params2(aParams[0]);
        params2.SetParam(TRoutingParameters::EParamFormatCode, EMTPFormatCodeUndefined);
        aParams.AppendL(params2);
        SelectSubTypeValidationL(ESubTypeOperationCode, aValidationSubTypes);
        }
        break;
        
    case ESubTypeRequestRegistration:
    default:
        break;
        }
    __FLOG(_L8("SelectSubTypeRoutingL, Exit"));
    }
    
/**
Selects the specified validation sub-type.
@param aSubType The selected validation sub-type.
@param aValidationSubTypes The set of selected validation sub-types, updated 
on exit.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/
void CMTPParserRouter::SelectSubTypeValidationL(TRoutingSubType aSubType, RArray<TUint>& aValidationSubTypes) const
    {
    __FLOG(_L8("SelectSubTypeValidationL, Entry"));
    TInt err(aValidationSubTypes.InsertInOrder(aSubType));
    if ((err != KErrNone) &&
        (err != KErrAlreadyExists))
        {
        User::Leave(err);
        }
    __FLOG(_L8("SelectSubTypeValidationL, Exit"));
    }

/**
Validates the specified set of routing targets.
@param aParams The operation routing parameter data.
@param aValidationSubTypes The set of validation sub-types. 
@param aTargets The set of data provider targets to be validated. Invalid 
targets are removed from this set.
*/    
void CMTPParserRouter::ValidateTargetsL(const TRoutingParameters& aParams, const RArray<TUint>& aValidationSubTypes, RArray<TUint>& aTargets) const
    {
    __FLOG(_L8("ValidateTargetsL, Entry"));
    const TUint KValidationsCount(aValidationSubTypes.Count());
    for (TUint v(0); (v < KValidationsCount); v++)
        {
        RArray<TUint> valid;
        CleanupClosePushL(valid);
        RouteOperationRequestNParametersL(aValidationSubTypes[v], aParams, valid);
        valid.Sort();
        TUint target(aTargets.Count());
        while (target--)
            {
            if (valid.FindInOrder(aTargets[target]) == KErrNotFound)
                {
                aTargets.Remove(target);
                }
            }
        CleanupStack::PopAndDestroy(&valid);
        }
    __FLOG(_L8("ValidateTargetsL, Exit"));
    }
    
/**
Provides the routing sub-type modifier flags of the specified routing sub-type.
@param aSubType The routing sub-type identifier.
@return The routing sub-type modifier flags.
*/
TUint CMTPParserRouter::Flags(TUint aSubType)
    {
    return (aSubType & ESubTypeFlagMask);
    }
    
/**
Provides the routing sub-type (map) index of the specified routing sub-type.
@param aSubType The routing sub-type identifier.
@return The routing sub-type (map) index.
*/
TUint CMTPParserRouter::Index(TUint aSubType)
    {
    return (aSubType & ESubTypeIndexMask);
    }
    
/**
Provides the routing sub-type parameter count type of the specified routing 
sub-type.
@param aSubType The routing sub-type identifier.
@return The routing sub-type parameter count type.
*/
TUint CMTPParserRouter::Params(TUint aSubType)
    {
    return (aSubType & ESubTypeParamsMask);
    }
    
/**
Provides the routing sub-type parameter count of the specified routing 
sub-type.
@param aSubType The routing sub-type identifier.
@return The routing sub-type parameter count.
*/
TUint CMTPParserRouter::ParamsCount(TUint aSubType)
    {
    return (Params(aSubType) >> 24);
    }

/**
Encodes a routing sub-type identifier using the specified sub-field values.
@param aIndex The routing sub-type (map) index.
@param aFlags The routing sub-type modifier flags.
@param aParamsCount The routing sub-type parameter count.
*/
TUint CMTPParserRouter::SubType(TUint aIndex, TUint aFlags, TUint aParamsCount)
    {
    return ((aParamsCount << 24) | aFlags | aIndex);
    }
  
#ifdef __FLOG_ACTIVE
/**
Logs the map table entries of all map tables.
@leave One of the system wide error codes, if a general processing error 
occurs.
*/  
void CMTPParserRouter::FLOGMapsL() const
    {
    __FLOG(_L8("FLOGMapsL, Entry"));
    const TUint KCount(iMaps.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        const CMap& KMap (*iMaps[i]);
        __FLOG(_L8(""));
        __FLOG_VA((_L8("Table 0x%08X"), KMap.SubType()));
        __FLOG(_L8("----------------"));
        RArray<TUint> from;
        CleanupClosePushL(from);
        KMap.InitParamsL(from);
        KMap.FLOGMapL(from);
        CleanupStack::PopAndDestroy(&from);
        __FLOG(_L8(""));
        }
    __FLOG(_L8("FLOGMapsL, Exit"));
    }
#endif
