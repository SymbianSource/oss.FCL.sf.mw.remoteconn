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
 @internalTechnology
*/

#ifndef CMTPPARSERROUTER_H
#define CMTPPARSERROUTER_H

#include <e32cmn.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpdataprovidercontroller.h"
#include "rmtpframework.h"
#include "../traces/OstTraceDefinitions.h"

class CMTPConnection;
class CMTPDataProvider;
class MMTPConnection;
class TMTPTypeEvent;

/**
Implements the MTP framework parser/router singleton. 

The MTP framework parser/router singleton is responsible for directing received
MTP operation request (and event) datasets to the appropriate data provider(s) 
for processing, and performs the following major functions: 
    
    1.  Parses received operation request and event datasets to extract and 
        coarse grain validate routing data (@see ParseOperationRequestL).
    2.  Executes one or more routing algorithms to select the set of data 
        provider targets able to service the operation or event 
        (@see RouteOperationRequestL).
    3.  (Optionally) dispatches the request or event (@see ProcessRequestL).
    
There are two basic scenarios which the parser/router must handle, and a 
different set of APIs is provided for each. These scenarios are:

    1.  The initial routing of operations as they are first received. This 
        routing is initiated by the MTP framework using the @see ProcessRequestL 
        API. This API executes all three of the functions identified above.
    2.  The secondary routing of those operations which cannot be dispatched 
        directly. Either because they require the combined processing 
        capabilities of multiple data providers to produce an aggregated 
        response, or; because they cannot be routed using data available during
        the operation request phase and can only be routed to their final 
        target using data available during the operation data phase. This 
        secondary routing is performed by the MTP framework's proxy data 
        provider using the @see ParseOperationRequestL and 
        @see RouteOperationRequestL APIs. These APIs respectively execute the 
        first two functions identified above, which are separated in order to 
        allow additional information to be added to the routing data by the 
        proxy data provider. The third function identified above is not executed 
        in this scenario, since a very different dispatching mechanism must be 
        used by the proxy data provider. 

The parser/router implements a number of different routing algorithm types, 
some of which are further decomposed into sub-types to achieve a finer level of 
routing granularity and control. In combination these algorithms support the 
following general routing mechanisms:
    
    1.  Device - A subset of device oriented MTP operations are always 
        dispatched to the device data provider.
    2.  Proxy - A subset of MTP operations cannot be dispatched directly. 
        Either they require the combined processing capabilities of multiple 
        data providers to produce an aggregated response, or cannot be routed 
        using data available from the operation dataset alone. These operations
        are dispatched to the proxy data provider for additional processing. 
    3.  Object Handle - MTP object handles generated by the MTP framework are 
        encoded with a data provider identifier to efficiently route those MTP 
        operations which supply an object handle as a parameter.
    4.  StorageID - MTP storage identifiers generated by the MTP framework are 
        encoded with a data provider identifier to efficiently route those MTP 
        operations which supply a storage identifier as a parameter.
    5.  Operation Parameter - On being loaded each data provider supplies a set 
        of configuration data (MTP operation codes, event codes, object 
        property codes, device property codes, and storage system types) which 
        collectively define the set of MTP operation requests that it wishes to 
        receive.
    6.  Request Registration - Data providers may optionally register MTP 
        operation request datasets with the framework which, when subsequently 
        matched, will be routed to them. This mechanism is used to route those 
        MTP operation types which are paired to complete over two transaction 
        cycles (e.g. SendObjectInfo/SendObject). It may also be used to route 
        vendor extension operation requests.

The following routing algorithm types are implemeted:

    1.  Operation Parameter Routing

        Operation parameter routing is the primary routing mechanism and by 
        default is enabled for both initial and secondary proxy routing 
        scenarios. Operation parameter routing involves selecting one or more 
        data provider targets using routing data obtained from or related to 
        the supplied operation dataset parameters using one of the following 
        sub-types:
        
        o   Parameter lookup routing sub-types, which match operation dataset 
            parameter data with the set of supported capabilities registered 
            by each data provider on being loaded, or;
        o   Parameter decode routing sub-types, which extract data provider 
            target identifiers encoded into the parameter data itself.

        The parameter lookup routing sub-types are table driven and utilise 
        binary search techniques to minimise processing overhead. The 
        following parameter lookup routing sub-types (tables) are implemented,
        each characterised by the combination of parameters they require to map
        to a routing target:
        
        o   DevicePropCode
        o   ObjectPropCode
        o   OperationCode
        o   StorageType
        o   FormatCode + FormatSubcode
        o   FormatCode + OperationCode
        o   StorageType + OperationCode
        o   FormatCode + FormatSubcode + StorageType

        These routing tables may be further characterised by modifier flags 
        which e.g. allow or disallow duplicate entries, or specify ascending 
        or descending sort orders which in turn can manipulate the order in 
        which proxy targets are processed.

        The parameter decode routing sub-types extract routing target 
        identifiers directly from the operation dataset parameter data itself. 
        The following parameter decode routing sub-types are implemented:

        o   Object Handle
        o   StorageID
        
    2.  Framework Routing

        By default, framework routing is enabled during initial operation 
        routing only and not during secondary proxy routing. Framework routing 
        directs a sub-set of MTP operations to one or other of the framework 
        data providers. Using one of the following routing sub-types: 
        
        o   Device DP. A fixed sub-set of MTP operations are always dispatched 
            to the device data provider.
        o   Proxy DP. A fixed sub-set of MTP operations are always dispatched 
            to the proxy data provider. In addition, any operations which yield
            multiple routing targets will also be routed to the proxy data 
            provider.
            
    3.  Request Registration Routing

        By default, request registration routing is enabled during initial 
        operation routing only and not during secondary proxy routing. Data 
        providers may optionally register MTP operation request datasets with 
        the framework which, when subsequently matched, will be routed to them.
        This mechanism is used to process those MTP operation types which are 
        paired to complete over two transaction cycles (e.g. 
        SendObjectInfo/SendObject). It may also be used to route vendor 
        extension operation requests.

        Request registration routing does not implement any routing sub-types.

@internalTechnology
 
*/
class CMTPParserRouter : public CBase
    {
public:
        
    /**
    Defines the MTP operation routing parameter data, which is an output of the 
    parser/routers @see ParseOperationRequestL API and an input to the 
    @see RouteOperationRequestL API. 
    */
    class TRoutingParameters
        {
    public: 
    
        /**
        The routing parameter type identifiers. Note that not all parameter
        types may be defined, only those deemed necessary to route the 
        associated operation dataset.
        */
        enum TParameterType
            {
            /**
            The DevicePropCode parameter.
            */
            EParamDevicePropCode,
            
            /**
            The object FormatCode parameter.
            */
            EParamFormatCode,
            
            /**
            The object format sub-code parameter. This parameter is undefined 
            unless @see EFormatCode contains a value of 
            @see EMTPFormatCodeAssociation, in which case this parameter should
            the specify the MTP association type code.
            */
            EParamFormatSubCode,
            
            /**
            The ObjectHandle parameter.
            */
            EParamObjectHandle,
            
            /**
            The ObjectPropCode parameter.
            */
            EParamObjectPropCode,
            
            /**
            The StorageId parameter.
            */
            EParamStorageId,
            
            /**
            The storage system type parameter.
            */
            EParamStorageSystemType,
            
            /**
            The invalid dataset flag. When set, this flag indicates that an 
            operation dataset validation check failed.
            */
            EFlagInvalid,
            
            /**
            The routing type codes. This flag is intended for internal use 
            only.
            */
            EFlagRoutingTypes,
            
            /**
            The ServiceID parameter.
            */
            EParamServiceId,
            
            /**
            */
            ENumTypes
            };
        
    public: 
    
        IMPORT_C TRoutingParameters(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
        TRoutingParameters(const TRoutingParameters& aParams);
    
        IMPORT_C MMTPConnection& Connection() const;
        IMPORT_C TUint Param(TParameterType aId) const;
        IMPORT_C void Reset();
        IMPORT_C const TMTPTypeRequest& Request() const; 
        IMPORT_C void SetParam(TParameterType aId, TUint aVal);
        
    private: // Not owned
    
        /**
        The handle of the MTP connection on which the operation is being
        processed.
        */
        MMTPConnection&        			iConnection;
        
        /**
        The operation dataset.
        */
        const TMTPTypeRequest&			iRequest; 
        
    private: // Owned
    
        TUint                           iParameterData[ENumTypes];
        TFixedArray<TUint, ENumTypes>	iParameters;
        };

public:

    static CMTPParserRouter* NewL(); 
    virtual ~CMTPParserRouter();
    
    IMPORT_C void ConfigureL();
    IMPORT_C TBool OperationSupportedL(TUint16 aOperation) const;
    IMPORT_C void ParseOperationRequestL(TRoutingParameters& aParams) const;
    IMPORT_C void RouteOperationRequestL(const TRoutingParameters& aParams, RArray<TUint>& aTargets) const;
    IMPORT_C TBool RouteRequestRegisteredL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
    
    void ProcessEventL(const TMTPTypeEvent& aEvent, CMTPConnection& aConnection) const;
    void ProcessRequestL(const TMTPTypeRequest& aRequest, CMTPConnection& aConnection) const;
    void RouteRequestRegisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TInt aId);
    void RouteRequestUnregisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);

private: // Owned

    /**
    Defines the routing type codes. These codes are partitioned into 
    two sub-fields as follows:
    
        1.  Bits  0-15 - routing type identifier.
        2.	Bits 16-23 - unused.
        3.  Bits 24-31 - routing type modifier flags.
    */
    enum TRoutingType
        {
        /**
        The routing type modifier flags mask.
        */
        ETypeFlagMask            	= 0xFF000000,
        
        /**
        The single target flag. When set this flag will result in at most one
        eligible routing target being selected. If multiple eligible targets 
        exists then the first eligible target will be selected.
        */
        ETypeFlagSingleTarget       = 0x01000000,
        
        /**
        The routing type identifier mask.
        */
        ETypeMask                   = 0x0000FFFF,
        
        /**
        The framework routing type identifier.
        */
        ETypeFramework              = 0x00000001,
        
        /**
        The operation parameter routing type identifier.
        */
        ETypeOperationParameter		= 0x00000002,
        
        /**
        The request registration routing type identifier.
        */
        ETypeRequestRegistration	= 0x00000004,
        };
    
    /**
    Defines the routing sub-type codes. These codes are partitioned into 
    two sub-fields as follows:
    
        1.  Bits  0-15 - routing sub-type (map) index.
        2.  Bits 16-23 - routing sub-type modifier flags.
        3.  Bits 24-31 - routing sub-type parameter count type.
    */
    enum TRoutingSubType
        {
        /**
        The routing sub-type identifier (table index) mask.
        */
        ESubTypeIndexMask                        	= 0x0000FFFF,
        
        /**
        The routing sub-type modifier bit flags mask.
        */
        ESubTypeFlagMask                            = 0x00FF0000,
        
        /**
        The null flag bits.
        */
        ESubTypeFlagNone                            = 0x00000000,
        
        /**
        The duplicates enabled flag bits. When set this flag indicates that 
        duplicate routing table entries are permitted.
        */
        ESubTypeFlagEnableDuplicates                = 0x00010000,
        
        /**
        The order descending flag bits. When set this flag indicates that 
        routing table entries are to be arranged in descending rather than 
        ascending order.
        */
        ESubTypeFlagOrderDescending               	= 0x00020000,
        
        /**
        The routing sub-type parameter count mask.
        */
        ESubTypeParamsMask                       	= 0xFF000000,
        
        /**
        The zero parameter count code.
        */
        ESubTypeParams0                           	= 0x00000000,
        
        /**
        The one parameter count code.
        */
        ESubTypeParams1                          	= 0x01000000,
        
        /**
        The two parameter count code.
        */
        ESubTypeParams2                           	= 0x02000000,
        
        /**
        The three parameter count code.
        */
        ESubTypeParams3                           	= 0x03000000,
        
        /**
        The DevicePropCode operation parameter lookup routing sub-type.
        */
        ESubTypeDevicePropCode                  	= (0x00000000 | ESubTypeParams1 | ESubTypeFlagNone),
        
        /**
        The ObjectPropCode operation parameter lookup routing sub-type.
        */
        ESubTypeObjectPropCode                   	= (0x00000001 | ESubTypeParams1 | ESubTypeFlagEnableDuplicates),
        
        /**
        The OperationCode operation parameter lookup routing sub-type.
        */
        ESubTypeOperationCode                    	= (0x00000002 | ESubTypeParams1 | ESubTypeFlagEnableDuplicates),
        
        /**
        The StorageType operation parameter lookup routing sub-type.
        */
        ESubTypeStorageType                     	= (0x00000003 | ESubTypeParams1 | ESubTypeFlagEnableDuplicates),
        
        /**
        The FormatCode + FormatSubcode operation parameter lookup routing 
        sub-type.
        */
        ESubTypeFormatCodeFormatSubcode          	= (0x00000004 | ESubTypeParams2 | (ESubTypeFlagEnableDuplicates | ESubTypeFlagOrderDescending)),
        
        /**
        The FormatCode + OperationCode operation parameter lookup routing 
        sub-type.
        */
        ESubTypeFormatCodeOperationCode          	= (0x00000005 | ESubTypeParams2 | ESubTypeFlagEnableDuplicates),
        
        /**
        The StorageType + OperationCode operation parameter lookup routing 
        sub-type.
        */
        ESubTypeStorageTypeOperationCode            = (0x00000006 | ESubTypeParams2 | ESubTypeFlagEnableDuplicates),
        
        /**
        The FormatCode + FormatSubcode + StorageType operation parameter 
        lookup routing sub-type.
        */
        ESubTypeFormatCodeFormatSubcodeStorageType	= (0x00000007 | ESubTypeParams3 | ESubTypeFlagNone),
        
        /**
        The ServiceID operation parameter lookup routing sub-type.
        */
        ESubTypeServiceIDOperationCode              = (0x00000008 | ESubTypeParams1 | ESubTypeFlagNone),
        
        /**
        The device DP framework routing sub-type.
        */
        ESubTypeDpDevice                            = (0x00000009 | ESubTypeParams0 | ESubTypeFlagNone),
        
        /**
        The proxy DP framework routing sub-type.
        */
        ESubTypeDpProxy                         	= (0x0000000A | ESubTypeParams0 | ESubTypeFlagNone),
        
        /**
        The object owner operation parameter decode routing sub-type.
        */
        ESubTypeOwnerObject                     	= (0x0000000B | ESubTypeParams0 | ESubTypeFlagNone),
        
        /**
        The storage owner operation parameter decode routing sub-type.
        */
        ESubTypeOwnerStorage                        = (0x0000000C | ESubTypeParams0 | ESubTypeFlagNone),
        
        /**
        The default request registration routing sub-type.
        */
        ESubTypeRequestRegistration             	= (0x0000000D | ESubTypeParams0 | ESubTypeFlagNone),
        
        };
        
    /**
    Defines the routing parameter IDs.
    */
    enum TRoutingParams
        {
        /**
        The first parameter.
        */
        EParam1 = 0,
        
        /**
        The second parameter.
        */
        EParam2 = 1
        
        /**
        The third parameter.
        */,
        EParam3 = 2,
        };

    /**
    Implements a simple single parameter map table entry which associates a 
    single routing parameter value and target data provider identifier pair. 
    */
    class TMap
        {
    public:
    
        TMap(TUint aFrom);
        TMap(TUint aFrom, TUint aTo, TUint aSubType);
    
    public:
    
        /**
        The routing parameter.
        */
        TUint   iFrom;
        
        /**
        The routing table sub-type code.
        */
        TUint   iSubType;
    
        /**
        The routing target data provider identifier.
        */
        TUint	iTo;
        };

    /**
    Implements a generic, multi-parameter routing table data structure. This 
    table implements owns EITHER:

        1.  A container of @see TMap node objects if implementing a single 
            parameter table or the final level (n) of a multi-parameter table,
            OR;
        2.  A container of @see CMap branch objects if implementing an 
            intermediate level (1 .. n-1) of a multi-parameter table.
    */
    class CMap : public CBase
        {            
    public:
    
        static CMap* NewLC(TUint aSubType);
        ~CMap();
        
        TUint From() const;
        void InitParamsL(RArray<TUint>& aFrom) const;
        void InsertL(const RArray<TUint>& aFrom, TUint aTo);
        void GetToL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const;
        TUint SubType() const;
        
#ifdef OST_TRACE_COMPILER_IN_USE
        void OSTMapL(RArray<TUint>& aFrom) const;
        void OSTMapEntryL(const RArray<TUint>& aFrom, TUint aTo) const;
#endif
        
    private:
    
        static CMap* NewLC(TUint aFrom, TUint aSubType);
        CMap(TUint aFrom, TUint aSubType);
        void ConstructL();
        
        TInt BranchFind(TUint aFrom) const;
        TUint BranchInsertL(TUint aFrom);
        TInt NodeFind(TUint aFrom) const;
        TInt NodeFind(const TMap& aNode) const;
        TUint NodeInsertL(const TMap& aMap);
        TUint Param(const RArray<TUint>& aFrom) const;
        TUint ParamIdx(const RArray<TUint>& aFrom) const;
        void SelectTargetAllL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const;
        void SelectTargetMatchingL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const;
        void SelectTargetSingleL(const RArray<TUint>& aFrom, RArray<TUint>& aTo) const;
        
        static TInt BranchOrderFromAscending(const CMap& aL, const CMap& aR);
        static TInt BranchOrderFromDescending(const CMap& aL, const CMap& aR);
        static TInt BranchOrderFromKeyAscending(const TUint* aL, const CMap& aR);
        static TInt BranchOrderFromKeyDescending(const TUint* aL, const CMap& aR);
        static TInt NodeOrderFromAscending(const TMap& aL, const TMap& aR);
        static TInt NodeOrderFromDescending(const TMap& aL, const TMap& aR);
        static TInt NodeOrderFromKeyAscending(const TUint* aL, const TMap& aR);
        static TInt NodeOrderFromKeyDescending(const TUint* aL, const TMap& aR);
        static TInt NodeOrderFromToAscending(const TMap& aL, const TMap& aR);
        static TInt NodeOrderFromToDescending(const TMap& aL, const TMap& aR);
        
    private:
 
        /**
        The routing parameter.
        */
        TUint               iFrom;
        
        /**
        The routing table sub-type code.
        */
        TUint               iSubType;
        
        /**
        The @see CMap branch object container. This container is only populated
        if implementing an intermediate level (1 .. n-1) of a multi-parameter 
        table.
        */
        RPointerArray<CMap> iToBranches;
        
        /**
        The @see TMap node object container. This container is only populated 
        if implementing a single parameter table or the final level (n) of a 
        multi-parameter table.
        */
        RArray<TMap>        iToNodes;
        };
    
private:
    
    CMTPParserRouter();
    void ConstructL();
    
    static void GetMapParameterIdsL(TUint aSubType, RArray<TUint>& aP1Codes, RArray<TUint>& aP2Codes, RArray<TUint>& aP3Codes);
    static void SelectTargetL(TUint aTarget, RArray<TUint>& aTargets);
    
    void Configure1ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes);
    void Configure2ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes, const RArray<TUint>& aP2Codes);
    void Configure3ParameterMapL(TUint aSubType, const RArray<TUint>& aP1Codes, const RArray<TUint>& aP2Codes, const RArray<TUint>& aP3Codes);
    void GetConfigParametersL(const CMTPDataProvider& aDp, const RArray<TUint>& aCodes, RArray<TUint>& aParams) const;
    void GetRoutingSubTypesL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesDeleteRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesCopyMoveRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesGetObjectPropListRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesSendObjectPropListRequestL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesDeleteObjectPropListL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void GetRoutingSubTypesGetFormatCapabilitiesL(RArray<TRoutingParameters>& aParams, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes) const;
    void ParseOperationRequestParameterL(TMTPTypeRequest::TElements aParam, TRoutingParameters::TParameterType aType, TRoutingParameters& aParams) const;
    void RouteOperationRequestNParametersL(TUint aRoutingSubType, const TRoutingParameters& aParams, RArray<TUint>& aTargets) const;
    void RouteOperationRequest0ParametersL(TUint aRoutingSubType, const TRoutingParameters& aParams, RArray<TUint>& aTargets) const;
    TUint RoutingTargetL(const TMTPTypeRequest& aRequest, CMTPConnection& aConnection) const;
    void SelectSubTypeRoutingL(TRoutingSubType aSubType, RArray<TUint>& aRoutingSubTypes, RArray<TUint>& aValidationSubTypes, RArray<TRoutingParameters>& aParams) const; 
    void SelectSubTypeValidationL(TRoutingSubType aSubType, RArray<TUint>& aValidationSubTypes) const;
    void ValidateTargetsL(const TRoutingParameters& aParams, const RArray<TUint>& aValidationSubTypes, RArray<TUint>& aTargets) const;
    void ValidateOperationRequestParametersL(TRoutingParameters& aParams) const;
    
    
    static TUint Flags(TUint aSubType);
    static TUint Index(TUint aSubType);
    static TUint Params(TUint aSubType);
    static TUint ParamsCount(TUint aSubType);
    static TUint SubType(TUint aIndex, TUint aFlags, TUint aParamsCount);
    
#ifdef OST_TRACE_COMPILER_IN_USE
    void OSTMapsL() const;
#endif

private: // Owned
    
    /**
    The operation parameter routing sub-type map tables.
    */
    RPointerArray<CMap> iMaps;
    
    /**
    The framework singletons.
    */
    RMTPFramework       iSingletons;
    };

#endif // CMTPPARSERROUTER_H
