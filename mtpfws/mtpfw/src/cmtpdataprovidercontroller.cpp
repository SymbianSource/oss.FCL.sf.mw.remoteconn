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

#include <barsc.h> 
#include <barsread.h> 
#include <e32property.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/rmtpclient.h>

#include "cmtpdataprovider.h"
#include "cmtpdataproviderconfig.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpobjectmgr.h"
#include "mtpframeworkconst.h"
#include "cmtpframeworkconfig.h"
#include "cmtpstoragemgr.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdataprovidercontrollerTraces.h"
#endif



// Class constants.
_LIT(KMTPDpResourceDirectory, "z:\\resource\\mtp\\");
_LIT(KMTPDpDummyResourcefile, "z:\\resource\\mtp\\dummydp.rsc");

static const TUint KOpaqueDataLength(64);

/**
CMTPDataProviderController panics
*/
_LIT(KMTPPanicCategory, "CMTPDataProviderController");
enum TMTPPanicReasons
    {
    EMTPPanicStorageEnumeration = 0,
    EMTPPanicFrameworkEnumeration = 1,
    EMTPPanicDataProviderStorageEnumeration = 2,
    EMTPPanicDataProviderEnumeration = 3
    };
    
LOCAL_C void Panic(TInt aReason)
    {
    User::Panic(KMTPPanicCategory, aReason);
    }

/**
CMTPDataProviderController factory method. 
@return A pointer to a new CMTPDataProviderController instance. Ownership IS 
transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPDataProviderController* CMTPDataProviderController::NewL()
    {
    CMTPDataProviderController* self = new (ELeave) CMTPDataProviderController();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
CMTPDataProviderController::~CMTPDataProviderController()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_CMTPDATAPROVIDERCONTROLLER_DES_ENTRY );
    
    Cancel();
    UnloadDataProviders();
    iDataProviderIds.Close();
    iEnumeratingDps.Close();
    iEnumeratingStorages.Close();
    iSingletons.Close();
    CloseRegistrySessionAndEntryL();
    delete iOpenSessionWaiter;
    delete iPendingRequestTimer;
    
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_CMTPDATAPROVIDERCONTROLLER_DES_EXIT );
    }
    
/**
Loads the set of available data providers and initiates the data provider
enumeration sequence.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPDataProviderController::LoadDataProvidersL()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL_ENTRY );

    // Retrieve the ECOM data provider implementations list
    RImplInfoPtrArray   implementations;
    TCleanupItem        cleanup(ImplementationsCleanup, reinterpret_cast<TAny*>(&implementations));
    CleanupStack::PushL(cleanup);
    REComSession::ListImplementationsL(KMTPDataProviderPluginInterfaceUid, implementations);
    implementations.Sort(TLinearOrder<CImplementationInformation>(ImplementationsLinearOrderUid));
    
    // Retrieve the data provider registration resource file list.
    CDir* registrations;
    LEAVEIFERROR(iSingletons.Fs().GetDir(KMTPDpResourceDirectory, KEntryAttNormal, ESortByName, registrations),
            OstTrace0( TRACE_ERROR, DUP5_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, "get MTP Dp resource directory error!" ));
            
    CleanupStack::PushL(registrations);

    CreateRegistrySessionAndEntryL();
    
    // Load the registered data providers. 
    const TUint KCount(registrations->Count());
    TInt index = 0; 
    for (TInt i = 0; i < KCount; ++i)
        {
        TUint uid = 0;
        if(Uid((*registrations)[i].iName, uid) != KErrNone)
        	{
        	OstTraceExt1( TRACE_WARNING, CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, 
        	        "LoadDataProvidersL - Fail to get UID = %S", (*registrations)[i].iName);       	
        	continue;
        	}
        index = implementations.FindInOrder(TUid::Uid(uid), ImplementationsLinearOrderUid);
        if (KErrNotFound == index)
        	{
        	continue;
        	}
		if( uid != KMTPImplementationUidDeviceDp && uid != KMTPImplementationUidProxyDp && uid != KMTPImplementationUidFileDp )
			{
	        //get the dpid for the uid from dpidstore table
	        TBool tFlag;
		    iNextDpId = iSingletons.ObjectMgr().DPIDL(uid, tFlag);
		    if(tFlag == EFalse)
		    	{
		    	iSingletons.ObjectMgr().InsertDPIDObjectL(iNextDpId,uid);
		    	}	       	
			}
		else
			{
			switch (uid)
				{
				case KMTPImplementationUidDeviceDp : 
					iNextDpId = KMTPDeviceDPID;
					break;

				case KMTPImplementationUidFileDp   : 
					iNextDpId = KMTPFileDPID;
					break;

				case KMTPImplementationUidProxyDp  : 
					iNextDpId = KMTPProxyDPID;
					break;				
				}
			}
		LoadROMDataProvidersL((*registrations)[i].iName, implementations);
		delete implementations[index];
		implementations.Remove(index);
        }

    //Load installed DPs on non-ROM drives.
    for (index = 0; index < implementations.Count(); ++index)
        {
        TRAPD(err, LoadInstalledDataProvidersL(implementations[index]));
        if (KErrNone != err)
            {
            OstTrace1(TRACE_ERROR, DUP1_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, 
                    "Load installed data provider[0x%x] failed.", implementations[index]->ImplementationUid().iUid);
            }
        }

    CleanupStack::PopAndDestroy(registrations);
    CleanupStack::PopAndDestroy(&implementations);    

    // Verify that the framework data providers are loaded.
    LEAVEIFERROR(DpId(KMTPImplementationUidDeviceDp),
            OstTrace0( TRACE_ERROR, DUP2_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, "Device Dp not loaded!" ));           
    LEAVEIFERROR(DpId(KMTPImplementationUidProxyDp),
            OstTrace0( TRACE_ERROR, DUP3_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, "Proxy Dp not loaded!" ));       
    LEAVEIFERROR(DpId(KMTPImplementationUidFileDp),
            OstTrace0( TRACE_ERROR, DUP4_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL, "file Dp not loaded!" ));
            
	// Sort the data provider set on enumeration phase order.
	iDataProviders.Sort(TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderEnumerationPhase));
	// Add the DP IDs into DP ID array, except for device DP, File DP and proxy DP
	for (TUint index=0; index < iDataProviders.Count(); index++)
	  {
	  if ((iDataProviders[index]->DataProviderId() != iDpIdDeviceDp)
	  	  && (iDataProviders[index]->DataProviderId() != iDpIdProxyDp))
	    {
	    iDataProviderIds.AppendL(iDataProviders[index]->DataProviderId());
	    }
	  }
	
    // Ensure that the data provider set is ordered on DataProvider Id.
    iDataProviders.Sort(TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderDPId));
    
    // Start enumerating.
    iEnumeratingStorages.AppendL(KMTPStorageAll);
    iEnumerationState = EEnumerationStarting;
    Schedule();
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERSL_EXIT );
    }
    
/**
Unloads all active data providers.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPDataProviderController::UnloadDataProviders()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_UNLOADDATAPROVIDERS_ENTRY );
    TRAP_IGNORE(iSingletons.ObjectMgr().ObjectStore().CleanL());
    iDataProviders.ResetAndDestroy();
    iDataProviderIds.Reset();
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_UNLOADDATAPROVIDERS_EXIT );
    }
    
/**
Issues the specified notification to all loaded data providers.
@param aNotification The notification type identifier.
@param aParams The notification type specific parameter block
@leave One of the system wide error code if a processing failure occurs
in the data provider.
*/
EXPORT_C void CMTPDataProviderController::NotifyDataProvidersL(TMTPNotification aNotification, const TAny* aParams)
    {
    NotifyDataProvidersL(KMTPDataProviderAll, aNotification, aParams);
    }

EXPORT_C void CMTPDataProviderController::NotifyDataProvidersL(TUint aDPId, TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_NOTIFYDATAPROVIDERSL_ENTRY);
    // Schedule any long running operations.
    switch (aNotification)
        {
    case EMTPStorageAdded:
        {
        // Queue a storage enumeration operation.
        __ASSERT_DEBUG(aParams, User::Invariant());
        const TMTPNotificationParamsStorageChange* params(static_cast<const TMTPNotificationParamsStorageChange*>(aParams));
        iEnumeratingStorages.AppendL(params->iStorageId);
        
        // Only schedule the operation start if there is not one currently underway.
        if (iEnumerationState == EEnumeratedFulllyCompleted) 
            {
            iNextDpId           = iDpIdDeviceDp;
            iEnumerationState   = EEnumeratingFrameworkObjects;
            Schedule();
            }
        }
        break;
	case EMTPStorageRemoved:
		{
		// Dequeue an unhandled storage enumeration operations if existed.
        // If not existed, just ignore the remove event, since the logical storageId already removed from StorageMgr
        // by the caller, i.e. CMTPStorageWatcher. 
        __ASSERT_DEBUG(aParams, User::Invariant());
        const TMTPNotificationParamsStorageChange* params(static_cast<const TMTPNotificationParamsStorageChange*>(aParams));
        //Start checking from the second event, since iEnumeratingStorages[0] is the event being handled by DPs.
        TUint32 storageId = params->iStorageId;
        for(TInt i=1; i<iEnumeratingStorages.Count(); i++)
            {
            if(storageId==iEnumeratingStorages[i])
                {
                iEnumeratingStorages.Remove(i);
                OstTrace1(TRACE_ERROR, CMTPDATAPROVIDERCONTROLLER_NOTIFYDATAPROVIDERSL, 
                        "Unhandle memory card add event removed, storageId: %d", storageId);
                }
            }
        }
        break;
    case EMTPObjectAdded:
        break;
    default:
        break;        
        }
        
    // Issue the notification.    
    const TUint KLoadedDps(iDataProviders.Count());
    if(aDPId == KMTPDataProviderAll)
        {
        for (TUint i(0); i < KLoadedDps; ++i)
            {
            CMTPDataProvider *dp = iDataProviders[i];
            if ((dp->DataProviderId() != iDpIdDeviceDp) &&
                (dp->DataProviderId() != iDpIdProxyDp))
                {
                dp->Plugin().ProcessNotificationL(aNotification, aParams);
                }
            
            //DeviceDP need handle the SessionClose Notification
            if ((dp->DataProviderId() == iDpIdDeviceDp) &&
                ( EMTPSessionClosed == aNotification))
                {
                dp->Plugin().ProcessNotificationL(aNotification, aParams);
                }            
            }
        }
    else
        {
        for (TUint i(0); i < KLoadedDps; ++i)
            {
            CMTPDataProvider *dp = iDataProviders[i];
            if ( dp->DataProviderId() == aDPId )
                {
                dp->Plugin().ProcessNotificationL(aNotification, aParams);
                break;
                }
            }
        }    
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_NOTIFYDATAPROVIDERSL_EXIT);
    }

/**
Provides the number of active data providers.
@return the number of active data providers.
*/
EXPORT_C TUint CMTPDataProviderController::Count()
    {
    return iDataProviders.Count();
    }

/**
Provides a reference to the data provider with the specified identifier.
@param aId The data provider identifier.
@return The data provider reference.
*/
EXPORT_C CMTPDataProvider& CMTPDataProviderController::DataProviderL(TUint aId)
    {
    return DataProviderByIndexL(iDataProviders.FindInOrder(aId, CMTPDataProvider::LinearOrderDPId));
    }  

/**
Provides a reference to the data provider with the specified index.
@param aIndex The data provider index.
@return The data provider reference.
*/
EXPORT_C CMTPDataProvider& CMTPDataProviderController::DataProviderByIndexL(TUint aIndex)
    {
    __ASSERT_DEBUG((aIndex < iDataProviders.Count()), User::Invariant());
    return *(iDataProviders[aIndex]);
    }  

/**
 * Determine whether a data provider with the specified data provider id has been loaded
 * @param aId the id of the data provider to be checked
 * @return true if the data provider has been loaded, otherwise false
 */
EXPORT_C TBool CMTPDataProviderController::IsDataProviderLoaded(TUint aId) const
	{
	TInt index = iDataProviders.FindInOrder(aId, CMTPDataProvider::LinearOrderDPId);
	if (index >= 0 && index < iDataProviders.Count())
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

/**
Provides the identifier of the device data provider.
@return TInt The device data provider identifier.
*/
EXPORT_C TInt CMTPDataProviderController::DeviceDpId()
    {
    return iDpIdDeviceDp;
    }
    
/**
Provides the identifier of the data provider with the specified implementation 
UID.
@param aUid The implementation UID.
@return TInt The proxy data provider identifier.
*/  
EXPORT_C TInt CMTPDataProviderController::DpId(TUint aUid)
    {
    TInt dpId = KErrNotFound;
    for (TUint index=0; index < iDataProviders.Count(); index++)
      {
      if (iDataProviders[index]->ImplementationUid().iUid == aUid)
        {
        dpId = iDataProviders[index]->DataProviderId();
        break;
        }
      }
    
    return dpId;
    }

/**
Provides the identifier of the proxy data provider.
@return TInt The proxy data provider identifier.
*/  
EXPORT_C TInt CMTPDataProviderController::ProxyDpId()
    {
    return iDpIdProxyDp;
    }

EXPORT_C TInt CMTPDataProviderController::FileDpId()
    {
    return iDpIdFileDp;
    }

/**
Wait for the enumeration complete.
*/ 
EXPORT_C void CMTPDataProviderController::WaitForEnumerationComplete()
{
	if((EnumerateState() < CMTPDataProviderController::EEnumeratingPhaseOneDone) && ( !iOpenSessionWaiter->IsStarted()))
		{
		iOpenSessionWaiter->Start();
		}
}
TBool CMTPDataProviderController::FreeEnumerationWaiter()
	{
	if(iOpenSessionWaiter->IsStarted())
		{
		iOpenSessionWaiter->AsyncStop();
		return ETrue;
		}
	return EFalse;
	}
/**
Data provider enumeration state change notification callback.
@param aDp The notifying data provider.
*/    
void CMTPDataProviderController::EnumerationStateChangedL(const CMTPDataProvider& aDp)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_ENUMERATIONSTATECHANGEDL_ENTRY);
    OstTraceDefExt2(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPDATAPROVIDERCONTROLLER_ENUMERATIONSTATECHANGEDL, 
            "iEnumerationState: 0x%x DpId: %d", iEnumerationState, aDp.DataProviderId());
    switch (iEnumerationState)
        {        
    case EEnumeratingFrameworkStorages:
    	switch (aDp.ImplementationUid().iUid)
            {
        case KMTPImplementationUidDeviceDp:
            iNextDpId = iDpIdProxyDp;
            break;

            
        case KMTPImplementationUidProxyDp:
            iNextDpId = iDpIdFileDp;
            break;
            
        case KMTPImplementationUidFileDp:
            iEnumerationState   = EEnumeratingDataProviderStorages;
            iDpIdArrayIndex     = 0;
            break;
            }
        Schedule();
        break;    
        
    case EEnumeratingDataProviderStorages:
        // Data provider storage enumerations execute sequentially.
        if (++iDpIdArrayIndex >= iDataProviderIds.Count())
            {
            iNextDpId           = iDpIdDeviceDp;
            iEnumerationState   = EEnumeratingFrameworkObjects;
            }
        Schedule();
        break;
          
    case EEnumeratingFrameworkObjects:
        switch (aDp.ImplementationUid().iUid)
            {
        case KMTPImplementationUidDeviceDp:
            iSingletons.ObjectMgr().RemoveNonPersistentObjectsL(aDp.DataProviderId());
            iNextDpId = iDpIdProxyDp;
            Schedule();
            break;
            
        case KMTPImplementationUidProxyDp:
            iEnumerationState   = EEnumeratingDataProviderObjects;
            iEnumerationPhase   = DataProviderL(iDataProviderIds[0]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase);
            iDpIdArrayIndex     = 0;                
            Schedule();
            break;
            }
        break;
        
    case EEnumeratingDataProviderObjects:
    case EEnumeratingSubDirFiles:
        if(KMTPImplementationUidFileDp == aDp.ImplementationUid().iUid && NeedEnumeratingPhase2())
            {
            OstTrace0(TRACE_NORMAL, DUP1_CMTPDATAPROVIDERCONTROLLER_ENUMERATIONSTATECHANGEDL,
                    "File DP first level enum complete");
            Cancel();

            //clean the root level snapshot
            TRAP_IGNORE(iSingletons.ObjectMgr().ObjectStore().CleanDBSnapshotL(ETrue));

            //Schedule again to scan subdir
            iEnumerationState   = EEnumeratingSubDirFiles;

            }
        else
            {
            iEnumeratingDps.Remove(iEnumeratingDps.FindInOrderL(aDp.DataProviderId()));
            // Remove any non-persistent objects that are still marked.
            iSingletons.ObjectMgr().RemoveNonPersistentObjectsL(aDp.DataProviderId());
    
            if ((iEnumeratingDps.Count() == 0) && iDpIdArrayIndex >= iDataProviderIds.Count())
                {
                Cancel();
                iEnumerationState   = EEnumeratingCleanDBSnapshot;
                TRAP_IGNORE(iSingletons.ObjectMgr().ObjectStore().CleanDBSnapshotL(EFalse));
                }
            else
                {
                if ((iEnumeratingDps.Count() == 0) && (iEnumerationPhase != DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase)))
                    {
                    // Enter next enumeration phase
                    iEnumerationPhase = DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase);
                    } 
                Schedule();
                }
            }
       
        break;
    case EEnumeratedFulllyCompleted:
    case EUnenumerated:
    case EEnumerationStarting:
    case EEnumeratingPhaseOneDone:
    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
    
    OstTraceExt3(TRACE_NORMAL, DUP2_CMTPDATAPROVIDERCONTROLLER_ENUMERATIONSTATECHANGEDL, 
                "iEnumerationState: 0x%x, DpId: %d, UID=0x%x", iEnumerationState, aDp.DataProviderId(), (TUint)aDp.ImplementationUid().iUid);
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_ENUMERATIONSTATECHANGEDL_EXIT);
    }

void CMTPDataProviderController::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_DOCANCEL_ENTRY);
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_DOCANCEL_EXIT);
    }
    


/*
 *Buid Db SnapShot for storage
 */
void CMTPDataProviderController::EstablishDBSnapshotL(TUint32 aStorageId)
    {
    if(KMTPStorageAll == aStorageId)
        {
        iSingletons.ObjectMgr().ObjectStore().EstablishDBSnapshotL(aStorageId);
        }
    else 
        {
        const CMTPStorageMetaData& storage(iSingletons.StorageMgr().StorageL(aStorageId));
        if(storage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
            {
            const RArray<TUint>& logicalIds(storage.UintArray(CMTPStorageMetaData::EStorageLogicalIds));
            const TUint KCountLogicalIds(logicalIds.Count());
            for (TUint i(0); (i < KCountLogicalIds); i++)
                {
                OstTrace1(TRACE_NORMAL, CMTPDATAPROVIDERCONTROLLER_ESTABLISHDBSNAPSHOTL,
                        "Establish snapshot for storage: 0x%x", logicalIds[i]);
                iSingletons.ObjectMgr().ObjectStore().EstablishDBSnapshotL(logicalIds[i]);
                }   
            }
        }
    }


void CMTPDataProviderController::RunL()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_RUNL_ENTRY );
    OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPDATAPROVIDERCONTROLLER_RUNL, 
            "iEnumerationState: 0x%x", iEnumerationState);
    switch (iEnumerationState)
        {
    case EEnumerationStarting:
        iEnumerationState   = EEnumeratingFrameworkStorages;
        iNextDpId           = iDpIdDeviceDp;
        // Fall through to issue the StartStorageEnumerationL signal.
        
    case EEnumeratingFrameworkStorages:
        // Enumerate storages sequentially.
    	DataProviderL(iNextDpId).EnumerateStoragesL();
        break;
        
    case EEnumeratingDataProviderStorages:
        // Enumerate storages sequentially.
            
        // In case there was no DPs other than devdp and proxydp.
        if (iDpIdArrayIndex < iDataProviderIds.Count())
            {
            DataProviderL(iDataProviderIds[iDpIdArrayIndex]).EnumerateStoragesL();
            }
        else
            {
            iNextDpId           = iDpIdDeviceDp;
            iEnumerationState   = EEnumeratingFrameworkObjects;
	
            Schedule();
            }
        break;
        
    case EEnumeratingFrameworkObjects:
        {
        TUint32 storageId = iEnumeratingStorages[0];
        if( ( KMTPStorageAll != storageId ) && (!iSingletons.StorageMgr().ValidStorageId(storageId)))
            {
            iNextDpId = 0;
            //Specified storage not existed, not necessary to start enumeration on this stroage.
            //Do this check will save enumeration time, i.e. avoid unnecessary enumeration, since memory card removed before enumeration starts.
            iEnumeratingStorages.Remove(0);
            if (iEnumeratingStorages.Count() == 0)
                {
                iEnumerationState = EEnumeratedFulllyCompleted;
                }
            else
                {
                //deal next storage
                iNextDpId = iDpIdDeviceDp;
                Schedule();
                }
            }
        else
            {
            // Enumerate framework data providers sequentially.
            if(iNextDpId == iDpIdDeviceDp)
                {
                EstablishDBSnapshotL(storageId);
                }
            EnumerateDataProviderObjectsL(iNextDpId);          
            }
        }
        break;
        
    case EEnumeratingDataProviderObjects:
    case EEnumeratingSubDirFiles:
        {
        
        if(EEnumeratingSubDirFiles == iEnumerationState &&  NeedEnumeratingPhase2())
            {
            TUint32 storageId = iEnumeratingStorages[0];
            EnumerateDataProviderObjectsL(iDpIdFileDp);
            SetNeedEnumeratingPhase2(EFalse);
            
            if(iOpenSessionWaiter->IsStarted())
                {
                iOpenSessionWaiter->AsyncStop();
                }            
            ExecutePendingRequestL(); 
            }
        
        TUint currentDp = 0;
        
        // Enumerate non-framework data providers concurrently.
        const TUint KLoadedDps(iDataProviderIds.Count());
        while ((iEnumeratingDps.Count() < KMTPMaxEnumeratingDataProviders) && (iDpIdArrayIndex < KLoadedDps)
               && (iEnumerationPhase == DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase)))
            {
            currentDp = iDataProviderIds[iDpIdArrayIndex++];
            OstTrace1(TRACE_NORMAL, DUP1_CMTPDATAPROVIDERCONTROLLER_RUNL, 
                    "Enumerating dpid %d", currentDp);
            iEnumeratingDps.InsertInOrderL(currentDp);
            EnumerateDataProviderObjectsL(currentDp);
            }
        
        OstTraceExt2(TRACE_NORMAL, DUP2_CMTPDATAPROVIDERCONTROLLER_RUNL, 
                "iDpIdArrayIndex = %d, KLoadedDps = %d", iDpIdArrayIndex, KLoadedDps);
        }
        break;
        
    case EEnumeratingCleanDBSnapshot:
        {
        iEnumeratingStorages.Remove(0);        
        iSingletons.ObjectMgr().ObjectStore().ObjectsEnumComplete();
        if(iOpenSessionWaiter->IsStarted())
            {
            iOpenSessionWaiter->AsyncStop();
            }
        ExecutePendingRequestL();   
        
        if (iEnumeratingStorages.Count() > 0)
            {
            iNextDpId           = iDpIdDeviceDp;
            iEnumerationState   = EEnumeratingFrameworkObjects;
            Schedule();
            }
        else
            {
            iEnumerationState   = EEnumeratedFulllyCompleted; //Really finished
            }
        }
        break;
        
    case EEnumeratedFulllyCompleted:
    case EUnenumerated:
    case EEnumeratingPhaseOneDone:
    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_RUNL_EXIT );
    }


TInt CMTPDataProviderController::RunError(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_RUNERROR_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPDATAPROVIDERCONTROLLER_RUNERROR, "Error = %d", aError);
    
    // If a RunL error happens, there's no point in trying to continue.
    switch (iEnumerationState)
        {
    case EEnumerationStarting:
    case EEnumeratingFrameworkStorages:
        Panic(EMTPPanicStorageEnumeration);
        break;
        
    case EEnumeratingFrameworkObjects:
        Panic(EMTPPanicFrameworkEnumeration);
        break;
        
    case EEnumeratingDataProviderStorages:
        Panic(EMTPPanicDataProviderStorageEnumeration);
        break;
        
    case EEnumeratingDataProviderObjects:
        Panic(EMTPPanicDataProviderEnumeration);
        break;
        
    case EUnenumerated:
    case EEnumeratingPhaseOneDone:
    default:
        User::Invariant();
        break;
        }

    // This code is never reached
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_RUNERROR_EXIT );
    return KErrNone;
    }

/**
Constructor.
*/
CMTPDataProviderController::CMTPDataProviderController() :
    CActive(EPriorityNormal)
    {
    CActiveScheduler::Add(this);
    }

/**
Second-phase constructor.
@leave One of the system wide error codes if a processing failure occurs.
*/
void CMTPDataProviderController::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
	TInt tMTPMode;
	TInt err = RProperty::Get(KUidSystemCategory, KUidMTPModeKeyValue, tMTPMode);
	if(err != KErrNone)
		{
		tMTPMode = KMTPModeMTP;		
		}
	else
		{
		if(tMTPMode != KMTPModeMTP && tMTPMode != KMTPModePTP && tMTPMode != KMTPModePictBridge)
		tMTPMode = KMTPModeMTP;
		}
	iMode = (TMTPOperationalMode)tMTPMode;
	//CreateRegistrySessionAndEntryL();
    
	SetNeedEnumeratingPhase2(EFalse);
	
    iOpenSessionWaiter = new(ELeave) CActiveSchedulerWait();
    
    iPendingRequestTimer = CMTPPendingReqestTimer::NewL(this);
    
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_CONSTRUCTL_EXIT );
    }
    
/**
Creates a data provider configurability parameter data instance on the cleanup 
stack.
@param aResourceFilename The data provider configuration data resource filename.
@return A pointer to the data provider configurability parameter data instance, 
which is also placed on the cleanup stack. Ownership is transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
CMTPDataProviderConfig* CMTPDataProviderController::CreateConfigLC(const TDesC& aResourceFilename)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_CREATECONFIGLC_ENTRY );

    // Open the configuration data resource file
    RResourceFile file;
    CleanupClosePushL(file);
    file.OpenL(iSingletons.Fs(), aResourceFilename);    
    
    // Create the resource reader.
    const TInt KDefaultResourceId(1);
    HBufC8* buffer(file.AllocReadLC(KDefaultResourceId));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    // Load the data provider configurability parameter data.
    CMTPDataProviderConfig* config(CMTPDataProviderConfig::NewL(reader, aResourceFilename));
    CleanupStack::PopAndDestroy(buffer);
    CleanupStack::PopAndDestroy(&file);
    CleanupStack::PushL(config);
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_CREATECONFIGLC_EXIT );
    return config;
    }

/**
Check the  necessity of objects enumeration as given the data provider 
@param dp CMTPDataProvider reference
*/
TBool CMTPDataProviderController::IsObjectsEnumerationNeededL(CMTPDataProvider& dp)
{
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_ISOBJECTSENUMERATIONNEEDEDL_ENTRY );

	CMTPStorageMgr& storages = iSingletons.StorageMgr();
	TUint32 aStorageId = iEnumeratingStorages[0];
	
    TBool doEnumeration = true;
	
	if (aStorageId != KMTPStorageAll && storages.PhysicalStorageId(aStorageId))
		{
		TBool isFSBased(false);
		RArray<TUint> storageTypes = dp.SupportedCodes( EStorageSystemTypes );
                // check whether the storage type of the dp is file system based.
		for (TInt i = 0; i < storageTypes.Count() && !isFSBased; i++)
			{
			   isFSBased= (storageTypes[i] == CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
			}
		// As given physical storage id, only fs based dp need to do the enumeration.
		if (!isFSBased)
			{
			  doEnumeration = false;
			}
	    }
	OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_ISOBJECTSENUMERATIONNEEDEDL_EXIT );
	return doEnumeration;
}

/**
Requests that the given data provider enumerate its objects.
@param aId data provider ID
*/
void CMTPDataProviderController::EnumerateDataProviderObjectsL(TUint aId)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_ENUMERATEDATAPROVIDEROBJECTSL_ENTRY );
    OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPDATAPROVIDERCONTROLLER_ENUMERATEDATAPROVIDEROBJECTSL, 
            "Data provider with DpId %d to enumerate its objects", aId );
    CMTPDataProvider& dp(DataProviderL(aId));

    if (IsObjectsEnumerationNeededL(dp))
        {   
        TBool abnormaldown = ETrue;
        iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::EAbnormalDown , abnormaldown);
        if ( (!abnormaldown) && (dp.DataProviderConfig().BoolValue(MMTPDataProviderConfig::EObjectEnumerationPersistent)))
            {       
            // Initialize persistent objects store.
            iSingletons.ObjectMgr().RestorePersistentObjectsL(aId);
            }
        else
            {
            // Mark all non-persistent objects.    
            iSingletons.ObjectMgr().MarkNonPersistentObjectsL(aId,iEnumeratingStorages[0]);
            }
        if ((KMTPStorageAll == iEnumeratingStorages[0]) || (iSingletons.StorageMgr().ValidStorageId(iEnumeratingStorages[0])))
            {
            //Only initiate the data provider enumeration sequence for valid storage or all storage
            dp.EnumerateObjectsL(iEnumeratingStorages[0]);
            }
        else
            {
            EnumerationStateChangedL(dp);
            }
       
        }
    else 
        {
        //The DP does not need enumeration this time, so just change the state to go on.
        EnumerationStateChangedL(dp);
        }

    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_ENUMERATEDATAPROVIDEROBJECTSL_EXIT );
    }
    
/**
Loads the dataprovider on ROM drives depending upon the mode and activates the specified ECOM data provider.
@param aResourceFilename The data provider registration and configuration data 
resource filename.
@param aImplementations The ECOM data provider implementations list (ordered by 
implementation UID).
@return ETrue if data provider is successfully loaded, EFalse otherwise.
@leave One of the system wide error codes, if a processing failure occurs. 
*/
TBool CMTPDataProviderController::LoadROMDataProvidersL(const TDesC& aResourceFilename, const RImplInfoPtrArray& aImplementations)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_LOADROMDATAPROVIDERSL_ENTRY );
    // Retrieve the implementation UID
    TUint uid(0);
    LEAVEIFERROR(Uid(aResourceFilename, uid),
            OstTraceExt1( TRACE_ERROR, CMTPDATAPROVIDERCONTROLLER_LOADROMDATAPROVIDERSL, "can't get uid from resource file %S", aResourceFilename));
            
    TBool success(EFalse);

    // Check for a corresponding plug-in implementation.
    TInt index = aImplementations.FindInOrder(TUid::Uid(uid), ImplementationsLinearOrderUid);
    if (index >= 0)
        {       	
        // Construct the configuration data resource file full path name.
        RBuf filename;
        CleanupClosePushL(filename);  
        filename.CreateL(KMTPDpResourceDirectory.BufferSize + aResourceFilename.Length());
        filename.Append(KMTPDpResourceDirectory);
        filename.Append(aResourceFilename); 
        if(iStubFound)
            {
            RPointerArray<HBufC> files;
            CleanupClosePushL(files);
            iSisEntry.FilesL(files);
            for (TInt i = 0; i< files.Count(); ++i)
                {
                TPtrC resourceFileName = files[i]->Des();
                TPtrC fileName = resourceFileName.Mid(resourceFileName.LocateReverse('\\') + 1);
                if(fileName.MatchF(aResourceFilename) != KErrNotFound)
                    {
                    TDriveName drive = aImplementations[index]->Drive().Name();
                    //replace "z:" with "c:" or "d:" or ...
                    filename.Replace(0,2,drive);
                    break;
                    }
                }
            CleanupStack::Pop(&files);
            files.ResetAndDestroy();  	        	
            }
        success = LoadDataProviderL(filename);
        CleanupStack::PopAndDestroy(&filename);
        }    	
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_LOADROMDATAPROVIDERSL_EXIT );
    return success;
    }

/**
Load all data providers installed on non-ROM drives depending upon the mode and activates 
the specified ECOM data provider.
@param aImplementations The installed ECOM data provider implementations list (ordered by 
implementation UID).
@leave One of the system wide error codes, if a processing failure occurs. 
*/
void CMTPDataProviderController::LoadInstalledDataProvidersL(const CImplementationInformation* aImplementations)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_LOADINSTALLEDDATAPROVIDERSL_ENTRY );
    TUint uid = aImplementations->ImplementationUid().iUid;
    TBool tFlag(EFalse);
    iNextDpId = iSingletons.ObjectMgr().DPIDL(uid, tFlag);
    if(!tFlag)
        {
        iSingletons.ObjectMgr().InsertDPIDObjectL(iNextDpId, uid);
        }   

    HBufC8 *OpaqData = HBufC8::NewLC(KOpaqueDataLength);
    *OpaqData = aImplementations->OpaqueData();
    TBuf16<KOpaqueDataLength> pkgIDstr;
    pkgIDstr.Copy(*OpaqData);
    CleanupStack::PopAndDestroy(OpaqData);
    pkgIDstr.Trim();
    _LIT(prefix, "0x");
    TInt searchindex = pkgIDstr.FindC(prefix);
    if(KErrNotFound != searchindex)
        {
        //Skip "0x".
        pkgIDstr = pkgIDstr.Mid(searchindex + 2);
        }
    if (0 == pkgIDstr.Length())
        {
        OstTrace0( TRACE_ERROR, CMTPDATAPROVIDERCONTROLLER_LOADINSTALLEDDATAPROVIDERSL, "pkgIdstr is empty!" );
        User::Leave(KErrArgument);
        }
    
    TUint aUid(0);
    LEAVEIFERROR(Uid(pkgIDstr, aUid),
            OstTraceExt1( TRACE_ERROR, DUP1_CMTPDATAPROVIDERCONTROLLER_LOADINSTALLEDDATAPROVIDERSL, "can't get uid from string %S",pkgIDstr ));
            
    
    iSingletons.ObjectMgr().InsertPkgIDObjectL(iNextDpId, aUid);
    TDriveName drive = aImplementations->Drive().Name();
    RBuf resourcefilename;
    CleanupClosePushL(resourcefilename);  
    resourcefilename.CreateL(KMaxFileName);
    resourcefilename.Copy(KMTPDpResourceDirectory);
    //Replace "z:"(at 0 position) with "c:" or "d:" or ...
    resourcefilename.Replace(0,2,drive);

    RBuf rscfile;
    CleanupClosePushL(rscfile);
    rscfile.CreateL(KMaxFileName);
    rscfile.NumUC(uid,EHex);
    _LIT(postfix, ".rsc");
    rscfile.Append(postfix);
    resourcefilename.Append(rscfile);
    CleanupStack::PopAndDestroy(&rscfile);
   
    LoadDataProviderL(resourcefilename);

    CleanupStack::PopAndDestroy(&resourcefilename);
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_LOADINSTALLEDDATAPROVIDERSL_EXIT );
    }

/**
Load data providers
@param aImplementations The installed ECOM data provider implementations list (ordered by 
implementation UID).
@return ETrue if data provider is successfully loaded, EFalse otherwise.
@leave One of the system wide error codes, if a processing failure occurs. 
*/
TBool CMTPDataProviderController::LoadDataProviderL(const TDesC& aResourceFilename)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERL_ENTRY );
    // Load the configurability parameter data.
    CMTPDataProviderConfig* config(CreateConfigLC(aResourceFilename));
    
    
    TBool success(EFalse);
    TBool supported(ETrue);
    TUint aUid(0);
    if ( Uid(aResourceFilename,aUid) != KErrNone )
       	{
        OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERL_EXIT );
        return success;	
       	}
    TUint uid(aUid);
    if ((uid != KMTPImplementationUidDeviceDp) && (uid != KMTPImplementationUidProxyDp) && (uid != KMTPImplementationUidFileDp))
        {
        supported = EFalse;
        RArray<TUint> supportedModeArray;			
        config->GetArrayValue(MMTPDataProviderConfig::ESupportedModes, supportedModeArray);
        TInt i=0;
        while (i < supportedModeArray.Count())
            {
            if(iMode == supportedModeArray[i])
                {
                supported = ETrue;
                break;
                }
            i++;
            }
        supportedModeArray.Close();
        if(!supported)
            {
            //Update the Database table last IsDploaded filed with Efalse;
            /* create dummy  DP which is just new of Dervied DataPRoviderClass
            update the DataBase  so that DPIP which this was get then set handle Store DB */
            iSingletons.ObjectMgr().MarkDPLoadedL(iNextDpId,EFalse);
            RBuf dummyfilename;
            CleanupClosePushL(dummyfilename);
            dummyfilename.CreateL(KMTPDpDummyResourcefile.BufferSize);        		
            dummyfilename.Append(KMTPDpDummyResourcefile);							
            CMTPDataProviderConfig* aDummyConfig(CreateConfigLC(dummyfilename));
            CMTPDataProvider* dummyDp(NULL);		
            dummyDp = CMTPDataProvider::NewLC(iNextDpId, TUid::Uid(uid), aDummyConfig);
            iDataProviders.InsertInOrderL(dummyDp, TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderUid));			
            CleanupStack::Pop(dummyDp);          						
            CleanupStack::Pop(aDummyConfig);
            CleanupStack::PopAndDestroy(&dummyfilename);			
            }
        }
    CMTPDataProvider* dp(NULL);
    if(supported)
        {
        // Load the data provider.        	
        switch (config->UintValue(MMTPDataProviderConfig::EDataProviderType))
            {
        case EEcom:
            {
            // Configurability parameter data ownership is passed to the data provider.
            iSingletons.ObjectMgr().MarkDPLoadedL(iNextDpId, ETrue);
            dp = CMTPDataProvider::NewLC(iNextDpId, TUid::Uid(uid), config);
            }
            break;

        default:
            __DEBUG_ONLY(User::Invariant());
            break;
            }
    	}
    if (dp)
        {
        // Register the data provider.
        switch (uid)
            {
            case KMTPImplementationUidDeviceDp:
                iDpIdDeviceDp = iNextDpId;
                break;

            case KMTPImplementationUidFileDp:
                iDpIdFileDp = iNextDpId;
                break;

            case KMTPImplementationUidProxyDp:
                iDpIdProxyDp = iNextDpId;
                break;

            default:
                break;
            }
        iDataProviders.InsertInOrderL(dp,  TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderUid));
        CleanupStack::Pop(dp);
        CleanupStack::Pop(config);
        success = ETrue;
        }
    else
        {
        // No data provider was created.
        CleanupStack::PopAndDestroy(config);
        }
    OstTraceFunctionExit0( DUP1_CMTPDATAPROVIDERCONTROLLER_LOADDATAPROVIDERL_EXIT );
    return success;
    }

/**
Provides the implemetation UID associated with the specified data provider 
configuration data resource filename.
@param aResourceFilename The data provider configuration data resource
filename.
@param aUid On completion, the implemetation UID.
*/
TInt CMTPDataProviderController::Uid(const TDesC& aResourceFilename, TUint& aUid)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_UID_ENTRY );
    // Extract the implemetation UID from the filename.
    TParsePtrC parser(aResourceFilename);
    TLex lex(parser.Name());
    TInt err = lex.Val(aUid, EHex);
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_UID_EXIT );
    return err;
    }

/**
Schedules an enumeration iteration.
*/
void CMTPDataProviderController::Schedule()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_SCHEDULE_ENTRY );
    if (!IsActive())
        {
        TRequestStatus* status(&iStatus);
        *status = KRequestPending;
        SetActive();
        User::RequestComplete(status, KErrNone);
        }
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_SCHEDULE_EXIT );
    }
/**
Get the mtpkey mode.
*/    
TMTPOperationalMode CMTPDataProviderController::Mode()
	{
	return iMode;
	}
     
void CMTPDataProviderController::ImplementationsCleanup(TAny* aData)
    {
    reinterpret_cast<RImplInfoPtrArray*>(aData)->ResetAndDestroy();
    }
    
/**
Implements a linear order relation for @see CImplementationInformation 
objects based on relative @see CImplementationInformation::ImplementationUid.
@param aUid The implementation UID object to match.
@param aObject The object instance to match.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/     
TInt CMTPDataProviderController::ImplementationsLinearOrderUid(const TUid* aUid, const CImplementationInformation& aObject)
    {
    return (aUid->iUid - aObject.ImplementationUid().iUid);
    }
    
/**
Implements a @see TLinearOrder for @see CImplementationInformation objects 
based on relative @see CImplementationInformation::ImplementationUid.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/   
TInt CMTPDataProviderController::ImplementationsLinearOrderUid(const CImplementationInformation& aL, const CImplementationInformation& aR)
    {
    return (aL.ImplementationUid().iUid - aR.ImplementationUid().iUid);
    }

/**
Provides the enumeration state
@return iEnumerationState
*/
EXPORT_C TUint CMTPDataProviderController::EnumerateState()
    {
    return iEnumerationState;
    } 

void CMTPDataProviderController::CreateRegistrySessionAndEntryL()
	{
    if(iStubFound)
        {
        return;
        }
            
	LEAVEIFERROR(iSisSession.Connect(),
	        OstTrace0( TRACE_ERROR, CMTPDATAPROVIDERCONTROLLER_CREATEREGISTRYSESSIONANDENTRYL, "can't connect to iSisSession " ));
    CleanupClosePushL(iSisSession);
    TInt err = KErrNone;
    TUint stubuid;
    iSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::EPackageStubUID , stubuid);
 	TRAP_IGNORE(err=iSisEntry.Open(iSisSession, TUid::Uid(stubuid) ));
	if(err == KErrNone)
		{
		iStubFound = ETrue;	
		}
	CleanupStack::Pop();	
	}
	
void  CMTPDataProviderController::CloseRegistrySessionAndEntryL()
	{
	if (&iSisEntry != NULL)
		{
		iSisEntry.Close();	
		}
	iSisSession.Close();
	}

EXPORT_C void CMTPDataProviderController::SetNeedEnumeratingPhase2(TBool aNeed)
	{
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_SETNEEDENUMERATINGPHASE2_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPDATAPROVIDERCONTROLLER_SETNEEDENUMERATINGPHASE2, "Need = %d", aNeed);
	
	iNeedEnumeratingPhase2 = aNeed;
	
	OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_SETNEEDENUMERATINGPHASE2_EXIT );
	}

EXPORT_C TBool CMTPDataProviderController::NeedEnumeratingPhase2() const
	{
	return iNeedEnumeratingPhase2;
	}


EXPORT_C void CMTPDataProviderController::RegisterPendingRequestDP(TUint aDpUid, TUint aTimeOut)
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_REGISTERPENDINGREQUESTDP_ENTRY );
    
    __ASSERT_DEBUG((iPendingRequestDpUid == 0), User::Invariant());
    iPendingRequestDpUid = aDpUid;
    if (aTimeOut > 0)
        {
        iPendingRequestTimer->Start(aTimeOut);
        }
      
    OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_REGISTERPENDINGREQUESTDP_EXIT );
    }


EXPORT_C void CMTPDataProviderController::ExecutePendingRequestL()
    {
    OstTraceFunctionEntry0( CMTPDATAPROVIDERCONTROLLER_EXECUTEPENDINGREQUESTL_ENTRY );
    
    if (iPendingRequestDpUid == 0)
        {
        iPendingRequestTimer->Cancel();
        OstTraceFunctionExit0( CMTPDATAPROVIDERCONTROLLER_EXECUTEPENDINGREQUESTL_EXIT );
        return;
        }
    
    int i = 0;
    int count = iDataProviders.Count();
    for (i = 0; i < count; ++i)
        {
        if (iDataProviders[i]->ImplementationUid().iUid == iPendingRequestDpUid)
            {
            iDataProviders[i]->ExecutePendingRequestL();
            iPendingRequestDpUid = 0;
            break;
            }
        }
    
    iPendingRequestTimer->Cancel();   
    OstTraceFunctionExit0( DUP1_CMTPDATAPROVIDERCONTROLLER_EXECUTEPENDINGREQUESTL_EXIT );
    }

EXPORT_C TUint CMTPDataProviderController::StorageEnumerateState(TUint aStorageId)
    {
    if (iEnumeratingStorages.Count() == 0)
        return EEnumeratedFulllyCompleted;
    
    TUint ret = EUnenumerated;
    CMTPStorageMgr& storages = iSingletons.StorageMgr();
    TInt idx = iEnumeratingStorages.Find(storages.PhysicalStorageId(aStorageId));
    
    if (idx == KErrNotFound)
        {
        if (iEnumeratingStorages[0] == KMTPStorageAll)
            {
            ret = EnumerateState();
            }
        else
            {
            ret = EEnumeratedFulllyCompleted;
            }
        }
    else if (idx > 0)
        {
        ret = EUnenumerated;
        }
    else   //idx == 0
        {
        ret = EnumerateState();
        }

    return ret;
    }

CMTPDataProviderController::CMTPPendingReqestTimer* CMTPDataProviderController::CMTPPendingReqestTimer::NewL(CMTPDataProviderController* aDPController)
    {
    CMTPPendingReqestTimer* self = new (ELeave) CMTPDataProviderController::CMTPPendingReqestTimer(aDPController);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CMTPDataProviderController::CMTPPendingReqestTimer::~CMTPPendingReqestTimer()
    {
    OstTraceFunctionEntry0( CMTPPENDINGREQESTTIMER_CMTPPENDINGREQESTTIMER_ENTRY );
    
    Cancel();
    
    OstTraceFunctionExit0( CMTPPENDINGREQESTTIMER_CMTPPENDINGREQESTTIMER_EXIT );
    }
          
void CMTPDataProviderController::CMTPPendingReqestTimer::Start(TUint aTimeOut)
    {
    OstTraceFunctionEntry0( CMTPPENDINGREQESTTIMER_START_ENTRY );
    
    if (aTimeOut > 0)
        {
        const TUint KMTPPendingRequestDelay = (1000000 * aTimeOut);
        After(KMTPPendingRequestDelay);
        }

    OstTraceFunctionExit0( CMTPPENDINGREQESTTIMER_START_EXIT );
    }
        
void CMTPDataProviderController::CMTPPendingReqestTimer::RunL()
    {
    OstTraceFunctionEntry0( CMTPPENDINGREQESTTIMER_RUNL_ENTRY );

    iDPController->ExecutePendingRequestL();
    
    OstTraceFunctionExit0( CMTPPENDINGREQESTTIMER_RUNL_EXIT );
    }

CMTPDataProviderController::CMTPPendingReqestTimer::CMTPPendingReqestTimer(CMTPDataProviderController* aDPController) :
    CTimer(EPriorityNormal)
    {
    iDPController = aDPController;
    }

void CMTPDataProviderController::CMTPPendingReqestTimer::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPENDINGREQESTTIMER_CONSTRUCTL_ENTRY );

    CTimer::ConstructL();
    CActiveScheduler::Add(this);
      
    OstTraceFunctionExit0( CMTPPENDINGREQESTTIMER_CONSTRUCTL_EXIT );
    }

