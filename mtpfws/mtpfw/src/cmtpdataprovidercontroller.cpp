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


// Class constants.
_LIT(KMTPDpResourceDirectory, "z:\\resource\\mtp\\");
_LIT(KMTPDpDummyResourcefile, "z:\\resource\\mtp\\dummydp.rsc");

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DataProviderController");)

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
    __FLOG(_L8("~CMTPDataProviderController - Entry"));
    Cancel();
    UnloadDataProviders();
    iDataProviderIds.Close();
    iEnumeratingDps.Close();
    iEnumeratingStorages.Close();
    iSingletons.Close();
    CloseRegistrySessionAndEntryL();
    delete iOpenSessionWaiter;
    __FLOG(_L8("~CMTPDataProviderController - Exit"));
    __FLOG_CLOSE;
    }
    
/**
Loads the set of available data providers and initiates the data provider
enumeration sequence.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPDataProviderController::LoadDataProvidersL()
    {
    __FLOG(_L8("LoadDataProvidersL - Entry"));
    // Retrieve the ECOM data provider implementations list
    RImplInfoPtrArray   implementations;
    TCleanupItem        cleanup(ImplementationsCleanup, reinterpret_cast<TAny*>(&implementations));
    CleanupStack::PushL(cleanup);
    REComSession::ListImplementationsL(KMTPDataProviderPluginInterfaceUid, implementations);
    implementations.Sort(TLinearOrder<CImplementationInformation>(ImplementationsLinearOrderUid));
    
    // Retrieve the data provider registration resource file list.
    CDir* registrations;
    User::LeaveIfError(iSingletons.Fs().GetDir(KMTPDpResourceDirectory, KEntryAttNormal, ESortByName, registrations));
    CleanupStack::PushL(registrations);

    // Load the registered data providers. 
    const TUint KCount(registrations->Count());
    TInt index = 0; 
    for (TInt i = 0; i < KCount; ++i)
        {
        TUint uid = 0;
        if(Uid((*registrations)[i].iName, uid) != KErrNone)
        	{
        	__FLOG_1(_L8("LoadDataProvidersL - Fail to get UID = %s"),&((*registrations)[i].iName) );
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
            __FLOG_VA((_L8("Load installed data provider[0x%x] failed."),implementations[index]->ImplementationUid().iUid));
            }
        }

    CleanupStack::PopAndDestroy(registrations);
    CleanupStack::PopAndDestroy(&implementations);    

    // Verify that the framework data providers are loaded.
    User::LeaveIfError(DpId(KMTPImplementationUidDeviceDp));
    User::LeaveIfError(DpId(KMTPImplementationUidProxyDp));
    User::LeaveIfError(DpId(KMTPImplementationUidFileDp));

	// Sort the data provider set on enumeration phase order.
	iDataProviders.Sort(TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderEnumerationPhase));
	// Add the DP IDs into DP ID array, except for device DP, File DP and proxy DP
	for (TUint index=0; index < iDataProviders.Count(); index++)
	  {
	  if ((iDataProviders[index]->DataProviderId() != iDpIdDeviceDp)
	  	  && (iDataProviders[index]->DataProviderId() != iDpIdFileDp)
	  	  && (iDataProviders[index]->DataProviderId() != iDpIdProxyDp))
	    {
	    iDataProviderIds.Append(iDataProviders[index]->DataProviderId());
	    }
	  }
	
    // Ensure that the data provider set is ordered on DataProvider Id.
    iDataProviders.Sort(TLinearOrder<CMTPDataProvider>(CMTPDataProvider::LinearOrderDPId));
    
    // Start enumerating.
    iEnumeratingStorages.AppendL(KMTPStorageAll);
    iEnumerationState = EEnumerationStarting;
    Schedule();
    __FLOG(_L8("LoadDataProvidersL - Exit"));
    }
    
/**
Unloads all active data providers.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPDataProviderController::UnloadDataProviders()
    {
    __FLOG(_L8("UnloadDataProviders - Entry"));
    TRAP_IGNORE(iSingletons.ObjectMgr().ObjectStore().CleanL());
    iDataProviders.ResetAndDestroy();
    iDataProviderIds.Reset();
    __FLOG(_L8("UnloadDataProviders - Exit"));
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
    __FLOG(_L8("NotifyDataProvidersL - Entry"));
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
                __FLOG_VA((_L8("Unhandle memory card add event removed, storageId: %d"), storageId));
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
    __FLOG(_L8("NotifyDataProvidersL - Exit"));
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
    return iDataProviders.FindInOrder(TUid::Uid(aUid), CMTPDataProvider::LinearOrderUid);
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
    __FLOG(_L8("EnumerationStateChangedL - Entry"));
    __FLOG_VA((_L8("Entry iEnumerationState: 0x%x iNextDpId: %d"), iEnumerationState, iNextDpId));
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
            //iNextDpId = iDpIdFileDp;
            if ( iDataProviderIds.Count()>0 )
                {
                iEnumerationState   = EEnumeratingDataProviderObjects;
                iEnumerationPhase   = DataProviderL(iDataProviderIds[0]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase);
                iDpIdArrayIndex     = 0;                
                }
            else
                {
                iNextDpId = iDpIdFileDp;
                }
            Schedule();
            break;
            
        case KMTPImplementationUidFileDp:
			// No other data providers
			if(NeedEnumeratingPhase2())
				{
				iEnumerationState = EEnumeratingSubDirFiles;
				if(iOpenSessionWaiter->IsStarted())
					{
					iOpenSessionWaiter->AsyncStop();
					}
				//Schedule FildDP to enumerate the files in sub-dir
				iNextDpId           = iDpIdFileDp;
				Schedule();
				}
			else
				{
				iNextDpId = 0;
				iEnumeratingStorages.Remove(0);
				if (iEnumeratingStorages.Count() == 0)
					{
					iSingletons.ObjectMgr().RemoveNonPersistentObjectsL(aDp.DataProviderId());
					iEnumerationState   = EEnumeratedFulllyCompleted;
					iSingletons.ObjectMgr().ObjectStore().CleanDBSnapshotL();
						
					Cancel();
					if(iOpenSessionWaiter->IsStarted())
						{
						iOpenSessionWaiter->AsyncStop();
						}
					}
				else
					{
					// Queued enumerations.
					iNextDpId           = iDpIdDeviceDp;
					Schedule();
					}
				}
        		
            }
        break;
        
    case EEnumeratingDataProviderObjects:
        // Enumerate non-framework data providers concurrently.
        iEnumeratingDps.Remove(iEnumeratingDps.FindInOrderL(aDp.DataProviderId()));
        // Remove any non-persistent objects that are still marked.
        iSingletons.ObjectMgr().RemoveNonPersistentObjectsL(aDp.DataProviderId());

        if ((iEnumeratingDps.Count() == 0) && iDpIdArrayIndex >= iDataProviderIds.Count())
            {
            // Enumeration complete.
            iNextDpId           = iDpIdFileDp;
            iEnumerationState   = EEnumeratingFrameworkObjects;
            			
			if ( ( iEnumeratingStorages.Count() > 1 ) &&(KErrNotFound != iEnumeratingStorages.Find(KMTPStorageAll)) )
				{
				const TUint storageid = iEnumeratingStorages[0];
				iEnumeratingStorages.Remove(0);
				if(KMTPStorageAll == storageid)
					{
					iEnumeratingStorages.Append(KMTPStorageAll);
					}
				
				if(iEnumeratingStorages[0] != KMTPStorageAll)
					{
					iNextDpId = iDpIdDeviceDp;
					}
				}
            }
        else
            {
            if ((iEnumeratingDps.Count() == 0) && (iEnumerationPhase != DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase)))
                {
                // Enter next enumeration phase
                iEnumerationPhase = DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase);
                }
            }
        Schedule();        
        break;
        
    case EEnumeratingSubDirFiles:
    	{
    	if(aDp.ImplementationUid().iUid == KMTPImplementationUidFileDp)
    		{
			iSingletons.ObjectMgr().RemoveNonPersistentObjectsL(aDp.DataProviderId());
			iNextDpId = 0;
			iEnumeratingStorages.Remove(0);
			if(iEnumeratingStorages.Count() == 0)
				{
				iSingletons.DpController().SetNeedEnumeratingPhase2(EFalse);
				iEnumerationState   = EEnumeratedFulllyCompleted;
				iSingletons.ObjectMgr().ObjectStore().CleanDBSnapshotL();
				}
			else //removable card plug in
				{
				iNextDpId           = iDpIdDeviceDp;
				iEnumerationState   = EEnumeratingFrameworkObjects;
				Schedule();
				}
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
    
    __FLOG_VA((_L8("Exit iEnumerationState: 0x%x, iNextDpId: %d, UID=0x%x"), iEnumerationState, iNextDpId, aDp.ImplementationUid().iUid));
    __FLOG(_L8("EnumerationStateChangedL - Exit"));
    }

void CMTPDataProviderController::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));
    __FLOG(_L8("DoCancel - Exit"));
    }
    
void CMTPDataProviderController::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    __FLOG_VA((_L8("iEnumerationState: 0x%x iNextDpId: %d"), iEnumerationState, iNextDpId));
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
                if(KMTPStorageAll == storageId)
                    {
                    iSingletons.ObjectMgr().ObjectStore().EstablishDBSnapshotL(storageId);
                    }
                else 
                    {
					const CMTPStorageMetaData& storage(iSingletons.StorageMgr().StorageL(storageId));
                	if(storage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
                		{
						const RArray<TUint>& logicalIds(storage.UintArray(CMTPStorageMetaData::EStorageLogicalIds));
						const TUint KCountLogicalIds(logicalIds.Count());
						for (TUint i(0); (i < KCountLogicalIds); i++)
							{
							__FLOG_VA((_L8("Establish snapshot for storage: 0x%x"), logicalIds[i]));
							iSingletons.ObjectMgr().ObjectStore().EstablishDBSnapshotL(logicalIds[i]);
							}	
                		}
                    }
                }
            	
            EnumerateDataProviderObjectsL(iNextDpId);          
            }
        	}
        break;
        
    case EEnumeratingDataProviderObjects:
        {
        TUint currentDp = 0;
        
        // Enumerate non-framework data providers concurrently.
        const TUint KLoadedDps(iDataProviderIds.Count());
        while ((iEnumeratingDps.Count() < KMTPMaxEnumeratingDataProviders) && (iDpIdArrayIndex < KLoadedDps)
               && (iEnumerationPhase == DataProviderL(iDataProviderIds[iDpIdArrayIndex]).DataProviderConfig().UintValue(MMTPDataProviderConfig::EEnumerationPhase)))
            {
            currentDp = iDataProviderIds[iDpIdArrayIndex++];
            iEnumeratingDps.InsertInOrderL(currentDp);
            EnumerateDataProviderObjectsL(currentDp);
            }
        
        __FLOG_VA((_L8("iDpIdArrayIndex = %d, KLoadedDps = %d"), iDpIdArrayIndex, KLoadedDps));
        }
        break;
        
    case EEnumeratingSubDirFiles:
    	{
    	EnumerateDataProviderObjectsL(iNextDpId); 
    	}
    	break;
    case EEnumeratedFulllyCompleted:
    case EUnenumerated:
    case EEnumeratingPhaseOneDone:
    default:
        __DEBUG_ONLY(User::Invariant());
        break;
        }
    __FLOG(_L8("RunL - Exit"));
    }

#ifdef __FLOG_ACTIVE
TInt CMTPDataProviderController::RunError(TInt aError)
#else
TInt CMTPDataProviderController::RunError(TInt /*aError*/)
#endif
    {
    __FLOG(_L8("RunError - Entry"));
    __FLOG_VA((_L8("Error = %d"), aError));
    
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
    __FLOG(_L8("RunError - Exit"));
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
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
	CreateRegistrySessionAndEntryL();
    
	SetNeedEnumeratingPhase2(EFalse);
	
    iOpenSessionWaiter = new(ELeave) CActiveSchedulerWait();
    __FLOG(_L8("ConstructL - Exit"));
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
    __FLOG(_L8("CreateConfigLC - Entry"));
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
    __FLOG(_L8("CreateConfigLC - Exit"));
    return config;
    }

/**
Check the  necessity of objects enumeration as given the data provider 
@param dp CMTPDataProvider reference
*/
TBool CMTPDataProviderController::IsObjectsEnumerationNeededL(CMTPDataProvider& dp)
{
    __FLOG(_L8("CheckEnumerateDPObjectsL - Entry"));

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
	__FLOG(_L8("CheckEnumerateDPObjectsL - Exit"));
	return doEnumeration;
}

/**
Requests that the given data provider enumerate its objects.
@param aId data provider ID
*/
void CMTPDataProviderController::EnumerateDataProviderObjectsL(TUint aId)
    {
    __FLOG(_L8("EnumerateDataProviderObjectsL - Entry"));
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
    
        // Initiate the data provider enumeration sequence.
        dp.EnumerateObjectsL(iEnumeratingStorages[0]);
		}
	else 
		{
		//The DP does not need enumeration this time, so just change the state to go on.
		EnumerationStateChangedL(dp);
		}

    __FLOG(_L8("EnumerateDataProviderObjectsL - Exit"));
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
    __FLOG(_L8("LoadROMDataProvidersL - Entry"));
    // Retrieve the implementation UID
    TUint uid(0);
    User::LeaveIfError(Uid(aResourceFilename, uid));
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
    __FLOG(_L8("LoadROMDataProvidersL - Exit"));
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
    __FLOG(_L8("LoadInstalledDataProvidersL - Entry"));
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
        User::Leave(KErrArgument);
        }
    
    TUint aUid(0);
    User::LeaveIfError(Uid(pkgIDstr, aUid));
    
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
    __FLOG(_L8("LoadInstalledDataProvidersL - Exit"));
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
    __FLOG(_L8("LoadDataProviderL - Entry"));
    // Load the configurability parameter data.
    CMTPDataProviderConfig* config(CreateConfigLC(aResourceFilename));
    
    
    TBool success(EFalse);
    TBool supported(ETrue);
    TUint aUid(0);
    if ( Uid(aResourceFilename,aUid) != KErrNone )
       	{
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
    __FLOG(_L8("LoadDataProviderL - Exit"));
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
    __FLOG(_L8("Uid - Entry"));
    // Extract the implemetation UID from the filename.
    TParsePtrC parser(aResourceFilename);
    TLex lex(parser.Name());
    TInt err = lex.Val(aUid, EHex);
    __FLOG(_L8("Uid - Exit"));
    return err;
    }

/**
Schedules an enumeration iteration.
*/
void CMTPDataProviderController::Schedule()
    {
    __FLOG(_L8("Schedule - Entry"));
    if (!IsActive())
        {
        TRequestStatus* status(&iStatus);
        *status = KRequestPending;
        SetActive();
        User::RequestComplete(status, KErrNone);
        }
    __FLOG(_L8("Schedule - Exit"));
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
	User::LeaveIfError(iSisSession.Connect());
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
	__FLOG(_L8("SetNeedEnumeratingPhase2 - Entry"));
	__FLOG_VA((_L8("Need = %d"), aNeed)); 
	
	iNeedEnumeratingPhase2 = aNeed;
	
	__FLOG(_L8("SetNeedEnumeratingPhase2 - Exit"));
	}

EXPORT_C TBool CMTPDataProviderController::NeedEnumeratingPhase2() const
	{
	return iNeedEnumeratingPhase2;
	}

