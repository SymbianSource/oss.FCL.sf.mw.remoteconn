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

#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mtpdataproviderconfig.hrh>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperesponse.h> 

#include "cmtpsession.h"
#include "cmtpconnection.h"
#include "cmtpconnectionmgr.h"
#include "cmtpdataprovider.h"
#include "cmtpdataproviderconfig.h"
#include "cmtpframeworkconfig.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"
#include "cmtpreferencemgr.h"
#include "cmtpstoragemgr.h"
#include "mmtptransactionproxy.h"
#include "rmtpframework.h"
#include "cdummydp.h"
#include "cmtpdatacodegenerator.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DataProvider");)

const TInt KWaitForEnumeration = 1000000 * 3;

#define KMTPDummyDataProvider             0x0000

/**
CMTPDataProvider factory method. 
@param aId The data provider identifier.
@param aUid The data provider implementation UID.
@param aConfig The data provider configurability parameter data. Ownership IS 
transfered.
@return A pointer to a new CMTPDataProvider instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPDataProvider* CMTPDataProvider::NewL(TUint aId, TUid aUid, CMTPDataProviderConfig* aConfig)
    {
    CMTPDataProvider* self = CMTPDataProvider::NewLC(aId, aUid, aConfig);
    CleanupStack::Pop(self);
    return self;
    }

/**
CMTPDataProvider factory method. A pointer to the CMTPDataProvider instance is
placed on the cleanup stack.
@param aUid The data provider implementation UID.
@param aConfig The data provider configurability parameter data. Ownership IS 
transfered.
@return A pointer to a new CMTPDataProvider instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/ 
CMTPDataProvider* CMTPDataProvider::NewLC(TUint aId, TUid aUid, CMTPDataProviderConfig* aConfig)
    {
    CMTPDataProvider* self = new (ELeave) CMTPDataProvider(aId, aUid, aConfig);
    CleanupStack::PushL(self);
    self->ConstructL();    
    return self;
    }
    
/**
Destructor.
*/
CMTPDataProvider::~CMTPDataProvider()
    {
    __FLOG_VA((_L8("~CMTPDataProvider - Entry, data provider %d "), iId));
    Cancel();
    iSupported.ResetAndDestroy();
    delete iImplementation;
    iSingletons.Close();

    // Only delete passed objects when fully constructed.
    if (iConstructed)
        {
        delete iConfig;
        }
	
	iTimer.Close();  
    __FLOG_VA((_L8("~CMTPDataProvider - Exit, data provider %d "), iId));
    __FLOG_CLOSE;
    }
    
void CMTPDataProvider::ExecuteEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {   
    __FLOG_VA((_L8("ExecuteEventL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
	
    if (iTimerActive && aEvent.Uint16(TMTPTypeEvent::EEventCode) == EMTPEventCodeCancelTransaction)
	    {
	    Cancel();
	    }
    
    // Pass this event notification directly to the plugin...
    // In reality we will only ever see one event canceltransaction.
    iImplementation->ProcessEventL(aEvent, aConnection);
    __FLOG_VA((_L8("ExecuteEventL - Exit, data provider %d "), iId));
    }
        
void CMTPDataProvider::ExecuteRequestL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("ExecuteRequestL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
               
    iCurrentRequest = &aRequest;
    iCurrentConnection = &iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId());
    // Schedule data provider to process this request.  
    Schedule(); 
    
    __FLOG_VA((_L8("ExecuteRequestL - Exit, data provider %d "), iId));
    }
    
EXPORT_C void CMTPDataProvider::ExecuteProxyRequestL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, MMTPTransactionProxy& aProxy)
    {
    __FLOG_VA((_L8("ExecuteProxyRequestL - Entry, data provider %d "), iId));
    iProxy = &aProxy;
    iCurrentRequest    = &aRequest;
    iCurrentConnection = static_cast<CMTPConnection*>(&aConnection);
    Schedule();
    __FLOG_VA((_L8("ExecuteProxyRequestL - Exit, data provider %d "), iId));
    }
    
void CMTPDataProvider::EnumerateObjectsL(TUint32 aStorageId)
    {
    __FLOG_VA((_L8("EnumerateObjectsL - Entry, data provider %d "), iId));
    iEnumerationState = ((iEnumerationState & ~EObjectsEnumerationState) | EObjectsEnumerating);
    TBool abnormaldown = EFalse;
    iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::EAbnormalDown, abnormaldown);
    iImplementation->StartObjectEnumerationL(aStorageId, abnormaldown);
    __FLOG_VA((_L8("EnumerateObjectsL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::EnumerateStoragesL()
    {
    __FLOG_VA((_L8("EnumerateStoragesL - Entry, data provider %d "), iId));
    iEnumerationState = ((iEnumerationState & ~EStoragesEnumerationState) | EStoragesEnumerating);
    iImplementation->StartStorageEnumerationL();
    __FLOG_VA((_L8("EnumerateStoragesL - Exit, data provider %d "), iId));
    }

/**
Provides the data provider's enumeration state.
@return The data provider's enumeration state.
*/
EXPORT_C TUint CMTPDataProvider::EnumerationState() const
	{
    return iEnumerationState;	
	}
    
/**
Provides the data provider's implementation UID
@return The data provider's implementation UID.
*/
EXPORT_C TUid CMTPDataProvider::ImplementationUid() const 
    {
    return iImplementationUid;
    }
    
/**
Provides the handle of the bound data provider plug-in.
@return The data provider plug-in.
*/
EXPORT_C CMTPDataProviderPlugin& CMTPDataProvider::Plugin() const
    {
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    return *iImplementation;
    }
    
EXPORT_C TBool CMTPDataProvider::Supported(TMTPSupportCategory aCategory, TUint aCode) const
    {
    return iSupported[aCategory]->Supported(aCode);
    }
    
EXPORT_C const RArray<TUint>& CMTPDataProvider::SupportedCodes(TMTPSupportCategory aCategory) const
    {
    return iSupported[aCategory]->Codes();
    }

/**
Sets the 
*/    
void CMTPDataProvider::SetDataProviderId(TUint aId)
    {
    __FLOG_VA((_L8("~CMTPDataProvider - Entry, data provider %d "), iId));
    iId = aId;
    __FLOG_VA((_L8("~CMTPDataProvider - Entry, data provider %d "), iId));
    }
	
/**
Implements a linear order relation for @see CMTPDataProvider 
objects based on relative @see CMTPDataProvider::ImplementationUid.
@param aUid The implementation UID object to match.
@param aObject The object instance to match.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/     
TInt CMTPDataProvider::LinearOrderUid(const TUid* aUid, const CMTPDataProvider& aObject)
    {
    return (aUid->iUid - aObject.ImplementationUid().iUid);
    }
    
/**
Implements a @see TLinearOrder for @see CMTPDataProvider objects based on 
relative @see CMTPDataProvider::ImplementationUid.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/   
TInt CMTPDataProvider::LinearOrderUid(const CMTPDataProvider& aL, const CMTPDataProvider& aR)
    {
    return (aL.iImplementationUid.iUid - aR.iImplementationUid.iUid);
    }

/**
Implements a @see TLinearOrder for @see CMTPDataProvider objects based on 
relative @see CMTPDataProvider::DataProviderId.
@param aDPId The Dataprovider ID to match.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/   
TInt CMTPDataProvider::LinearOrderDPId(const TUint* aDPId, const CMTPDataProvider& aObject)
    {
    return (*aDPId - aObject.DataProviderId());
    }

/**
Implements a @see TLinearOrder for @see CMTPDataProvider objects based on 
relative @see CMTPDataProvider::DataProviderId.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/   
TInt CMTPDataProvider::LinearOrderDPId(const CMTPDataProvider& aL, const CMTPDataProvider& aR)
    {
    return (aL.DataProviderId() - aR.DataProviderId());
    }

/**
Implements a @see TLinearOrder for @see CMTPDataProvider objects based on 
relative enumeration phase
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/   
TInt CMTPDataProvider::LinearOrderEnumerationPhase(const CMTPDataProvider& aL, const CMTPDataProvider& aR)
    {    
    return (aL.DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase)
    	      - aR.DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase));
    }

    
TUint CMTPDataProvider::DataProviderId() const
    {
    return iId;
    }    
    
TMTPOperationalMode CMTPDataProvider::Mode() const
    {
    return (iSingletons.DpController().Mode());    
    }
    
void CMTPDataProvider::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("ReceiveDataL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    __ASSERT_DEBUG(!IsActive(), User::Invariant());
    
    if (iProxy)
        {
        // Pass to proxy plugin for processing
        iProxy->ProxyReceiveDataL(aData, aRequest, aConnection, iStatus);
        SetActive();
        
        // Running under proxy mode so manually set the next transaction phase to be processed by the plugin
        iProxyTransactionPhase = EResponsePhase;
        }
    else
        {
        // Inform the connection that we wish to receive data from the connection
        CMTPConnection& connection(iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId()));
        connection.ReceiveDataL(aData, aRequest, iStatus);
        SetActive();
        // Async call so wait for object to be activated...
        }
    __FLOG_VA((_L8("ReceiveDataL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("SendDataL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    __ASSERT_DEBUG(!IsActive(), User::Invariant());
    
    if (iProxy)
        {
        // Pass to proxy plugin for processing
        iProxy->ProxySendDataL(aData,aRequest, aConnection, iStatus);
        SetActive();
        
        // Running under proxy mode so manually set the next transaction phase to be processed by the plugin
        iProxyTransactionPhase = EResponsePhase;
        }
    else
        {
        // Pass request to connection...
        CMTPConnection& connection(iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId()));
        connection.SendDataL(aData, aRequest, iStatus);
        SetActive();
        // Async call so wait for object to be activated...
        }
    __FLOG_VA((_L8("SendDataL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::SendEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("SendEventL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    
    CMTPConnection& connection(iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId()));
    connection.SendEventL(aEvent);
    __FLOG_VA((_L8("SendEventL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::SendEventL(const TMTPTypeEvent& aEvent)
    {
    __FLOG_VA((_L8("SendEventL - Entry, data provider %d "), iId));
    if (aEvent.Uint32(TMTPTypeEvent::EEventSessionID) != KMTPSessionAll)
        {
        User::Leave(KErrArgument);            
        }
    
    TUint numConnections(iSingletons.ConnectionMgr().ConnectionCount());
    
    for (TUint i(0); (i < numConnections); i++)
        {
        iSingletons.ConnectionMgr()[i].SendEventL(aEvent);       
        }
    __FLOG_VA((_L8("SendEventL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("SendResponseL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    __ASSERT_DEBUG(!IsActive(), User::Invariant());
    
    if (iProxy)
        {
        // Pass to proxy plugin for processing
        iProxy->ProxySendResponseL(aResponse, aRequest, aConnection, iStatus);
        SetActive();
        
        // Running under proxy mode so manually set the next transaction phase to be processed by the plugin
        iProxyTransactionPhase = ECompletingPhase;
        }
    else
        {
        CMTPConnection& connection(iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId()));
        connection.SendResponseL(aResponse, aRequest, iStatus);
        SetActive();
        }
    __FLOG_VA((_L8("SendResponseL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::TransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("TransactionCompleteL - Entry, data provider %d "), iId));
    __ASSERT_DEBUG(iImplementation, User::Invariant());
    
    if (iProxy)
        {
        // Pass to proxy plugin for processing
        iProxy->ProxyTransactionCompleteL(aRequest, aConnection);
        // Running under proxy mode so manually set the next transaction phase to be processed by the plugin
        iProxyTransactionPhase = ERequestPhase;
        }
    else
        {
        CMTPConnection& connection(iSingletons.ConnectionMgr().ConnectionL(aConnection.ConnectionId()));
        connection.TransactionCompleteL(aRequest);            
        }
    
    // Clear pointers
    iProxy = NULL;
    iCurrentRequest = NULL;
    iCurrentConnection = NULL;
    
    __FLOG_VA((_L8("TransactionCompleteL - Exit, data provider %d "), iId));
    } 
    
void CMTPDataProvider::RouteRequestRegisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("RouteRequestRegisterL - Entry, data provider %d "), iId));
    iSingletons.Router().RouteRequestRegisterL(aRequest, aConnection, iId);
    __FLOG_VA((_L8("RouteRequestRegisterL - Exit, data provider %d "), iId));
    }

void CMTPDataProvider::RouteRequestUnregisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG_VA((_L8("RouteRequestUnregister - Entry, data provider %d "), iId));
    iSingletons.Router().RouteRequestUnregisterL(aRequest, aConnection);
    __FLOG_VA((_L8("RouteRequestUnregister - Exit, data provider %d "), iId));
    }

#ifdef __FLOG_ACTIVE  	
void CMTPDataProvider::ObjectEnumerationCompleteL(TUint32 aStorageId)
#else
void CMTPDataProvider::ObjectEnumerationCompleteL(TUint32 /*aStorageId*/)
#endif // __FLOG_ACTIVE
    {
    __FLOG_VA((_L8("ObjectEnumerationCompleteL - Entry, data provider %d "), iId));
    __FLOG_VA((_L8("StorageId = 0x%08X "), aStorageId));
    iEnumerationState = ((iEnumerationState & ~EObjectsEnumerationState) | EObjectsEnumerated);
    iSingletons.DpController().EnumerationStateChangedL(*this);
    __FLOG_VA((_L8("ObjectEnumerationCompleteL - Exit, data provider %d "), iId));
    }
    
void CMTPDataProvider::StorageEnumerationCompleteL()
    {
    __FLOG_VA((_L8("StorageEnumerationCompleteL - Entry, data provider %d "), iId));
    iEnumerationState = ((iEnumerationState & ~EStoragesEnumerationState) | EStoragesEnumerated);
    iSingletons.DpController().EnumerationStateChangedL(*this);
    __FLOG_VA((_L8("StorageEnumerationCompleteL - Exit, data provider %d "), iId));
    }

const MMTPDataProviderConfig& CMTPDataProvider::DataProviderConfig() const
	{
	return *iConfig;
	}
	
const MMTPFrameworkConfig& CMTPDataProvider::FrameworkConfig() const
	{
    return iSingletons.FrameworkConfig();
	}

MMTPObjectMgr& CMTPDataProvider::ObjectMgr() const
    {    
    return iSingletons.ObjectMgr();
    }
    
MMTPReferenceMgr& CMTPDataProvider::ReferenceMgr() const
    {
    return iSingletons.ReferenceMgr();
    }
    
MMTPStorageMgr& CMTPDataProvider::StorageMgr() const
    {    
    return iSingletons.StorageMgr();
    } 
    
RFs& CMTPDataProvider::Fs() const
    {
    return iSingletons.Fs();
    }

MMTPDataCodeGenerator& CMTPDataProvider::DataCodeGenerator() const
    {
    return iSingletons.DataCodeGenerator();
    }

void CMTPDataProvider::NotifyFrameworkL( TMTPNotificationToFramework aNotification, const TAny* aParams )
    {
    __FLOG(_L8("NotifyFrameworkL - Entry"));
    
    __ASSERT_DEBUG( aParams, User::Invariant());
    
    switch ( aNotification )
        {
    case EMTPAddFolder:
        {
        TUint deviceDpId = iSingletons.DpController().DeviceDpId();
        iSingletons.DpController().NotifyDataProvidersL( deviceDpId, EMTPObjectAdded, aParams );
        }
        break;
    default:
        __FLOG(_L8("Ignore other notification"));
        break;
        }
    
    __FLOG(_L8("NotifyFrameworkL - Exit"));
    }

void CMTPDataProvider::DoCancel()
    {
    __FLOG_VA((_L8("DoCancel - Entry, data provider %d "), iId));
    __FLOG_VA((_L8("DoCancel - Exit, data provider %d "), iId)); 
	
    if (iTimerActive)
	    {
	    iTimer.Cancel();
	    iTimerActive = EFalse;
	    }
    else
	    {
    	TRequestStatus* status = &iStatus;
	    User::RequestComplete(status, KErrCancel);
	    }  
    }
    
void CMTPDataProvider::RunL()
    {  
    __FLOG_VA((_L8("RunL - Entry, data provider %d "), iId));
    __MTP_HEAP_FLOG
    __ASSERT_DEBUG(iCurrentConnection, User::Invariant());
     
	
    iTimerActive = EFalse; 

	if (iProxy)
        {
        iCurrentTransactionPhase = iProxyTransactionPhase;
        }
    else
        {
        iCurrentTransactionPhase = iCurrentConnection->TransactionPhaseL(iCurrentRequest->Uint32(TMTPTypeRequest::ERequestSessionID));        
        }
    __FLOG_VA((_L8("Current transaction phase = 0x%08X"), iCurrentTransactionPhase));
    
    TInt status(iStatus.Int());
    if ((status != KErrNone) &&
    	(status != KErrAbort) &&
    	(status != KErrCancel))
	    {
	    // Framework or transport error has occured, force error recovery.
        iErrorRecovery = iStatus.Int();
	    }
    else if (status == KErrCancel)
        {
        iImplementation->Cancel();
        }
    else if (status == KErrAbort)
        {
        if (iCurrentRequest != NULL)
            {
            TMTPTypeEvent event;
            event.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeCancelTransaction);
            event.SetUint32(TMTPTypeEvent::EEventSessionID, iCurrentRequest->Uint32(TMTPTypeRequest::ERequestSessionID) );
            event.SetUint32(TMTPTypeEvent::EEventTransactionID, iCurrentRequest->Uint32(TMTPTypeRequest::ERequestTransactionID) );
            
            iImplementation->ProcessEventL(event ,*iCurrentConnection);
            }
        }

    
    if (iErrorRecovery != KErrNone)
        {
        __FLOG(_L8("Error recovery in progress"));
        switch (iCurrentTransactionPhase)
            {
        case ERequestPhase:
        case EDataIToRPhase:
        case EDataRToIPhase:
        case EResponsePhase:
            SendErrorResponseL(iErrorRecovery);
            break;
            
        case ECompletingPhase:
            TRAPD(err, iImplementation->ProcessRequestPhaseL(iCurrentTransactionPhase, *iCurrentRequest, *iCurrentConnection));
            __FLOG_VA((_L8("iImplementation->ProcessRequestPhaseL error %d"), err));
            if (err != KErrNone)
                {
                TransactionCompleteL(*iCurrentRequest, *iCurrentConnection);   
                }
            iErrorRecovery = KErrNone;
            break;
            
        default:
            break;                
            }
        }
    
		else if (iSingletons.DpController().EnumerateState() < CMTPDataProviderController::EEnumeratingPhaseOneDone)
    	{
        __FLOG(_L8("DP Enumeration is not complete"));

        TUint16 opCode = iCurrentRequest->Uint16(TMTPTypeRequest::ERequestOperationCode);

        switch(opCode)
        {
        case EMTPOpCodeOpenSession:
        case EMTPOpCodeCloseSession:
        case EMTPOpCodeGetDeviceInfo:
        case EMTPOpCodeGetDevicePropDesc:
        case EMTPOpCodeGetObjectPropsSupported:
        case EMTPOpCodeGetObjectPropDesc:
        case EMTPOpCodeVendorExtextensionEnd:
            iImplementation->ProcessRequestPhaseL(iCurrentTransactionPhase, *iCurrentRequest, *iCurrentConnection);
    	break;

        case EMTPOpCodeGetStorageIDs:
        case EMTPOpCodeGetStorageInfo:
        	if(iSingletons.DpController().EnumerateState() > CMTPDataProviderController::EEnumeratingDataProviderStorages)
        	{
        		iImplementation->ProcessRequestPhaseL(iCurrentTransactionPhase, *iCurrentRequest, *iCurrentConnection);
        	}
        	else
        	{
        	    //If storages are not enumerated wait for 3 secs and check it again.
    			iTimer.After(iStatus, TTimeIntervalMicroSeconds32(KWaitForEnumeration));
    			SetActive();
    			iTimerActive = ETrue;
        	}
            break;
        	    	
        default:

	    switch (iCurrentTransactionPhase)
		    {
		case ERequestPhase:
		   	// Wait till the objects are enumerated. 
		    // checks for every 3 secs till the objects are eumrated.
		    // Won't send any device busy response.
		    // Windows wont repond if it gets device busy response after it has send Openseesion.
			iTimer.After(iStatus, TTimeIntervalMicroSeconds32(KWaitForEnumeration));
			SetActive();
			iTimerActive = ETrue;
			break;
		case EResponsePhase:
			iImplementation->ProcessRequestPhaseL(iCurrentTransactionPhase, *iCurrentRequest, *iCurrentConnection);
			break; 		   
	  case ECompletingPhase:
	   	TransactionCompleteL(*iCurrentRequest, *iCurrentConnection);   
		  break;
	  default:
		  break;
		    }
	    }
		}
    else
        {
        // Pass to the bound plugin for processing.
        iImplementation->ProcessRequestPhaseL(iCurrentTransactionPhase, *iCurrentRequest, *iCurrentConnection);
        
        //Make ActiveRequest processor and CancelRequest processing to occur synchronously
        if( iCurrentTransactionPhase == ERequestPhase )
        	{
        	CMTPSession& session(static_cast<CMTPSession&>(iCurrentConnection->SessionWithMTPIdL(iCurrentRequest->Uint32(TMTPTypeRequest::ERequestSessionID))));
        	MMTPConnectionProtocol& connection(static_cast<MMTPConnectionProtocol&>(*iCurrentConnection)); 
        	
        	// Pass transaction to session to check against any pending events
           	if ( session.CheckPendingEvent(*iCurrentRequest) )
            	{
                //Current request matches a pending event, pass event to connection layer event processing
                connection.ReceivedEventL(session.PendingEvent());
                }
        	}
        
        }
    
    __MTP_HEAP_FLOG
    __FLOG_VA((_L8("RunL - Exit, data provider %d "), iId));
    }
     
TInt CMTPDataProvider::RunError(TInt aError)
	{
    __FLOG_VA((_L8("RunError - Entry, data provider %d "), iId));
    __FLOG_VA((_L8("Error = %d"), aError));
    
    /* 
    CMTPDataProvider or iImplementation error, save the error state and 
    re-schedule.
    */
    iErrorRecovery = aError;
    Schedule();
    
    __FLOG_VA((_L8("RunError - Exit, data provider %d "), iId));
	return KErrNone;
	}

/**
Constructor.
@param aId The data provider identifier.
@param aUid The data provider implementation UID.
@param aConfig The data provider configurability parameter data. Ownership IS 
transfered.
*/
CMTPDataProvider::CMTPDataProvider(TUint aId, TUid aUid, CMTPDataProviderConfig* aConfig) : 
	CActive(EPriorityStandard),
	iConfig(aConfig),
	iId(aId),
	iImplementationUid(aUid),
	iProxyTransactionPhase(ERequestPhase)
    {
    CActiveScheduler::Add(this);
    }
        
/**
Second phase constructor.
@leave One of the system wide error codes if a processing failure occurs.
*/
void CMTPDataProvider::ConstructL()
	{
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG_VA((_L8("ConstructL - Entry, data provider %d "), iId));
    
    iSingletons.OpenL();
    TUint tDPType = iConfig->UintValue(MMTPDataProviderConfig::EDataProviderType);   
    const TUint KExcludeCategoryStart = EVendorExtensionSets;
    const TUint KExcludeCategoryEnd = EFormatExtensionSets;
    if( tDPType == KMTPDummyDataProvider)
    	{
		iImplementation = static_cast<CMTPDataProviderPlugin*>(CDummyDp::NewL(static_cast<MMTPDataProviderFramework*>(this)));
		//create your dummy classCMTPDataProviderPlugin
		for (TUint i(EAssociationTypes); (i < ENumCategories); i++)
            {
            CSupportedCodes* codes = CSupportedCodes::NewLC(static_cast<TMTPSupportCategory>(i), Plugin());
            
            if((i >= KExcludeCategoryStart) && (i <= KExcludeCategoryEnd) && (codes->Codes().Count() >0))
            	{
            	User::Leave(KErrNotSupported);
            	}
            
            iSupported.AppendL(codes);
            CleanupStack::Pop(codes);
            }
		iConstructed = ETrue;	
		}
	else   
    	{
    	iImplementation = CMTPDataProviderPlugin::NewL(iImplementationUid, static_cast<MMTPDataProviderFramework*>(this));
	
    	for (TUint i(EAssociationTypes); (i < ENumCategories); i++)
        	{
        	CSupportedCodes* codes = CSupportedCodes::NewLC(static_cast<TMTPSupportCategory>(i), Plugin());
        	
        	if((i >= KExcludeCategoryStart) && (i <= KExcludeCategoryEnd) && (codes->Codes().Count() >0))
            	{
            	User::Leave(KErrNotSupported);
            	}
        	iSupported.AppendL(codes);
        	CleanupStack::Pop(codes);
        	}
				
    	User::LeaveIfError(iTimer.CreateLocal());

		// Only assume ownership of passed objects on successful construction.
		iConstructed = ETrue;
	
		__FLOG_VA((_L8("Data provider %d iImplementationUid 0x08%X "), iId, iImplementationUid));
    	__FLOG_VA((_L8("ConstructL - Exit, data provider %d "), iId));	
    	}
	 
	}

/**
Schedules the next request phase or event.
*/
void CMTPDataProvider::Schedule()
    {
    __FLOG_VA((_L8("Schedule - Entry, data provider %d "), iId));
    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    SetActive();
    User::RequestComplete(status, KErrNone);
    __FLOG_VA((_L8("Schedule - Exit, data provider %d "), iId));
    }

/**
Formats and sends an MTP response dataset from the specified error code.
@param aError The error code.
@leave One of the system wide error codes, if a processing error occurs.
*/
void CMTPDataProvider::SendErrorResponseL(TInt aError)
	{
    __FLOG_VA((_L8("SendResponseL - Entry, data provider %d "), iId));
	__ASSERT_DEBUG(iCurrentRequest != NULL, User::Invariant());
	
	TMTPResponseCode code;
	switch (aError)
	    {
    case KErrOverflow:
    case KMTPDataTypeInvalid:
        code = EMTPRespCodeInvalidDataset;
        break;
        
    default:
        code = EMTPRespCodeGeneralError;
        break;
	    }
	    
    __FLOG_VA((_L8("Sending response code  0x%04X"), code));
	iResponse.SetUint16(TMTPTypeResponse::EResponseCode, code);		    
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, iCurrentRequest->Uint32(TMTPTypeResponse::EResponseSessionID));	
	iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, iCurrentRequest->Uint32(TMTPTypeResponse::EResponseTransactionID));
	SendResponseL(iResponse, *iCurrentRequest, *iCurrentConnection);
	
    __FLOG_VA((_L8("SendResponseL - Exit, data provider %d "), iId));	
	}
    
CMTPDataProvider::CSupportedCodes* CMTPDataProvider::CSupportedCodes::NewLC(TMTPSupportCategory aCategory, MMTPDataProvider& aDp)
    {
    CSupportedCodes* self = new(ELeave) CSupportedCodes();
    CleanupStack::PushL(self);
    self->ConstructL(aCategory, aDp);
    return self;
    }
    
CMTPDataProvider::CSupportedCodes::~CSupportedCodes()
    {
    iCodes.Close();
    }
    
const RArray<TUint>& CMTPDataProvider::CSupportedCodes::Codes() const
    {
    return iCodes;
    }
    
TBool CMTPDataProvider::CSupportedCodes::Supported(TUint aCode) const
    {
    return (iCodes.FindInOrder(aCode) != KErrNotFound);
    }
    
CMTPDataProvider::CSupportedCodes::CSupportedCodes() :
    iCodes()
    {
    
    }
    
void CMTPDataProvider::CSupportedCodes::ConstructL(TMTPSupportCategory aCategory, MMTPDataProvider& aDp)
    {
    aDp.Supported(aCategory, iCodes);
    iCodes.Sort();
    }


