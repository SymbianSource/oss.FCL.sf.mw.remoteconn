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
 @publishedPartner
 @released
*/

#ifndef MMTPDATAPROVIDERFRAMEWORK_H
#define MMTPDATAPROVIDERFRAMEWORK_H

#include <e32cmn.h>
#include <e32def.h>
#include <mtp/mtpdataproviderconfig.hrh>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPConnection;
class MMTPDataProviderConfig;
class MMTPFrameworkConfig;
class MMTPObjectMgr;
class MMTPReferenceMgr;
class MMTPStorageMgr;
class MMTPType;
class TMTPTypeEvent;
class TMTPTypeResponse;
class TMTPTypeRequest;
class RFs;
class MMTPDataCodeGenerator;

/**
Defines the MTP data provider framework layer application programming
interface.
@publishedPartner
@released
*/
class MMTPDataProviderFramework
    {
public:

    /**
    Provides the unique identifier of the calling data provider.
    @return The data provider identifier.
    */
    virtual TUint DataProviderId() const = 0;

    /**
    Provides the current MTP operational mode.
    @return The current MTP operational mode.
    */
    virtual TMTPOperationalMode Mode() const = 0;

    /**
    Initiates a data object receive sequence in the MTP data provider framework
    layer. This method should only be invoked when processing the ERequestPhase
    of an MTP transaction (@see CMTPDataProviderPlugin::ProcessRequestPhaseL),
    and causes the MTP session transaction state to transition to the
    @see EDataIToRPhase. The data object receive sequence is completed when the 
    MTP data provider framework layer initiates the @see EResponsePhase of the 
    MTP transaction (@see CMTPDataProviderPlugin::ProcessRequestPhaseL).
    @param aData The MTP data object sink buffer.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @see CMTPDataProviderPlugin::ProcessRequestPhaseL
    @leave KErrNotReady, if invoked when the current MTP transaction phase is
    not ERequestPhase.
    */
    virtual void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Initiates a data object send sequence in the MTP data provider framework
    layer. This method should only be invoked when processing the ERequestPhase
    of an MTP transaction (@see CMTPDataProviderPlugin::ProcessRequestPhaseL),
    and causes the MTP session transaction state to transition to the
    @see EDataRToIPhase. The data object send sequence is completed when the 
    MTP data provider framework layer initiates the @see EResponsePhase of the 
    MTP transaction (@see CMTPDataProviderPlugin::ProcessRequestPhaseL).
    @param aData The MTP data object source buffer.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @see CMTPDataProviderPlugin::ProcessRequestPhaseL
    @leave KErrNotReady, if invoked when the current MTP transaction phase is
    not ERequestPhase.
    */
    virtual void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Signals the MTP data provider framework layer to send an asynchronous event
    dataset on the specified MTP connection.
    @param aEvent The MTP event dataset source buffer.
    @param aConnection The handle of the MTP connection on which the event is
    to be sent.
    @leave KErrArgument, if the event data is not valid.
    */
    virtual void SendEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection) = 0;

    /**
    Signals the MTP data provider framework layer to send an asynchronous event
    dataset on all active MTP connections.
    @param aEvent The MTP event dataset source, this should always target all
    open sessions, i.e. SessionID should be set to KMTPAllSessions.
    @leave KErrArgument, if the event data is invalid.
    */
    virtual void SendEventL(const TMTPTypeEvent& aEvent) = 0;

    /**
    Initiates an MTP response dataset send sequence in the MTP data provider
    framework layer. This method should only be invoked when processing either
    the @see ERequestPhase or @see EResponsePhase of an MTP transaction, (@see
    CMTPDataProviderPlugin::ProcessRequestPhaseL) and causes the MTP session
    transaction state to transition to the @see ECompletingPhase. The MTP 
    response dataset send sequence is completed when the MTP data provider 
    framework layer initiates the @see ECompletingPhase of the MTP transaction 
    (@see CMTPDataProviderPlugin::ProcessRequestPhaseL).
    @param aResponse The MTP aResponse dataset source buffer.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @see CMTPDataProviderPlugin::ProcessRequestPhaseL
    @leave KErrNotReady, if invoked when the current MTP transaction phase is
    not ERequestPhase or EResponsePhase.
    */
    virtual void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Signals to the MTP data provider framework layer that all processing
    related to the current transaction has been completed. This method should
    only be invoked when processing the @see ECompletingPhase of the MTP 
    transaction (@see CMTPDataProviderPlugin::ProcessRequestPhaseL), and causes 
    the MTP session transaction state to transition to the @see EIdle state.
    @param aRequest The MTP request dataset that initiated the transaction.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @see CMTPDataProviderPlugin::ProcessRequestPhaseL
    @leave KErrNotReady If invoked when the current MTP transaction phase is
    invalid.
    */
    virtual void TransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Registers the calling data provider to receive one or more occurrences of 
    the specified request dataset that are received on the specified 
    connection. This method should be used to register the calling data 
    provider to receive the following request types:
    
        1.  Follow-on requests of an MTP operation that can span multiple 
            transaction cycles. The following request types are recognised
            by the framework as follow-on requests:
            
            i)  SendObject (preceded by SendObjectInfo or SendObjectPropList). 
            ii) TerminateOpenCapture (preceded by InitiateOpenCapture).
            
        2.  MTP vendor extension requests.
        
    Note that:

        1.  The request dataset being registered must minimally specify the
            Operation Code of the expected operation and the SessionID on which 
            the operation request is expected to be received. 
            
            Follow-on request registrations MUST specify a specific SessionID. 
            Registrations of non follow-on requests may optionally specify a 
            SessionID of @see KMTPSessionAll to receive matching requests from
            any active MTP session.
           
        2.  With the exception of the TransactionID element, registered request
            datasets must exactly match all data elements of the expected request 
            dataset in order to be successfully routed.
           
        3.  Duplicate RouteRequestRegisterL registrations are not permitted. A request 
            dataset that matches that of a previous registration by this or 
            any other data provider on the same MTP session will result in the 
            previous registration being superceded.
           
        4.  Pending RouteRequestRegisterL registrations can be withdrawn at any time
            using the @see RouteRequestUnregisterL method. 
           
        5.  RouteRequestRegisterL registrations of MTP request types which ARE 
            recognised by the framework as MTP follow-on requests (SendObject, 
            TerminateOpenCapture) will result in at most one matching request 
            occurence being routed to the data provider. To receive another 
            request dataset of the same type, a new @see RouteRequestRegisterL 
            registration must be made.
            
        6   RouteRequestRegisterL registrations of MTP request types which ARE
            NOT recognised by the framework as MTP follow-on requests will 
            result in all matching requests which are subsequently received 
            being routed to the data provider. This will continue until such 
            time as the RouteRequestRegisterL registration is withdrawn using 
            the @see RouteRequestUnregisterL method.
            
        7.  RouteRequestRegisterL registrations request datasets which specify 
            an SessionID value of @see KMTPSessionAll, will result in matching 
            requests which are subsequently received on any active MTP session
            being routed to the data provider.

    @param aRequest The operation request dataset being registered.
    @param aConnection The handle of the MTP connection on which the operation
    request is expected to be received.
    @leave KErrArgument, if the request dataset does meet the minimal
    registration requirements specified above.
    @leave One of the system wide error codes, if a general processing error
    occurs.
    @see RouteRequestUnregisterL
    */
    virtual void RouteRequestRegisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Cancels a pending RouteRequestRegisterL registration.
    @param aRequest The registered operation request dataset.
    @param aConnection The handle of the MTP connection for which the operation
    request was registered.
    @leave One of the system wide error codes, if a general processing error
    occurs.
    @see RouteRequestRegisterL
    */
    virtual void RouteRequestUnregisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;

    /**
    Signals the completion of the data provider's object store enumeration
    sequence that was previously initiated by a StartObjectEnumerationL signal
    made to the MTP data provider interface.
    @param aStorageId The MTP StorageID of the enumerated storage. This should 
    match the value specified in the preceding StartObjectEnumerationL.
    @see MMTPDataProvider::StartObjectEnumerationL
    */
    virtual void ObjectEnumerationCompleteL(TUint32 aStorageId) = 0;

    /**
    Signals the completion of the data provider's storage enumeration sequence
    that was previously initiated by a StartStorageEnumerationL signal
    made to the MTP data provider interface.
    @see MMTPDataProvider::StartStorageEnumerationL
    */
    virtual void StorageEnumerationCompleteL() = 0;

    /**
    Provides a handle to the configurability data specified in the data 
    provider's configuration file. 
    @return Handle to the data provider's configurability data.
    */
    virtual const MMTPDataProviderConfig& DataProviderConfig() const = 0;

    /**
    Provides a handle to the data provider framework configurability parameter 
    data. 
    @return Handle to the data provider framework configurability data.
    */
    virtual const MMTPFrameworkConfig& FrameworkConfig() const = 0;

    /**
    Provides a handle to the MTP object manager, which manages the assignment
    of MTP object handles and their mapping to actual data objects on behalf
    of the data provider.
    @return Handle to the MTP data provider framework object manager.
    */
    virtual MMTPObjectMgr& ObjectMgr() const = 0;

    /**
    Provides a handle to the MTP object reference manager, to which data
    providers can delegate the handling of the MTP persistent, abstract
    object referencing mechanism.
    @return Handle to the MTP data provider framework object reference manager.
    */
    virtual MMTPReferenceMgr& ReferenceMgr() const = 0;

    /**
    Provides a handle to the MTP storage manager, which manages the assignment 
    of MTP storage identifiers on behalf of the data provider.
    @return Handle to the MTP data provider framework storage manager.
    */
    virtual MMTPStorageMgr& StorageMgr() const = 0;
    
    /**
    Provides a handle to the MTP data provider framework RFs session.
    @return Handle to the MTP data provider framework RFs session.
    */
    virtual RFs& Fs() const = 0;
    
    /**
    Provides a handle to the MTP datacode generator, which generate the datacode of service properties , formats and methods etc.
    @return Handle to the MTP datacode generator.
    */
    virtual MMTPDataCodeGenerator& DataCodeGenerator() const = 0;
    
    /**
    Issues the specified notification to framework.
    @param aNotification The notification type identifier.
    @param aParams The notification type specific parameter block
    @leave One of the system wide error code if a processing failure occurs
    in the framework.
    */
    virtual void NotifyFrameworkL( TMTPNotificationToFramework aNotification, const TAny* aParams ) = 0;
   
    };

#endif // MMTPDATAPROVIDERFRAMEWORK_H
