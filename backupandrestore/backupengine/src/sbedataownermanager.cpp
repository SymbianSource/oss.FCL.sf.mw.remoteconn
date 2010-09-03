// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of CDataOwnerManager
// 
//

/**
 @file
*/
#include "sbpackagedatatransfer.h"
#include "sbedataowner.h"
#include "sbedataownermanager.h"
#include "sbebufferhandler.h"
#include "abserver.h"

#include "sbecompressionandencryption.h"

#include <e32svr.h>
#include <arc4.h>
#include <babackup.h>
#include <ezcompressor.h>
#include <ezdecompressor.h>
#include <swi/sisregistrypackage.h>
#include <swi/sisregistryentry.h>
#include <swi/swispubsubdefs.h>
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbedataownermanagerTraces.h"
#endif

namespace conn
	{
	const TInt KSID = 0x10202D56;
	_LIT_SECURITY_POLICY_S0(KWritePolicy, KSID);
	_LIT_SECURITY_POLICY_PASS(KReadPolicy);
	_LIT_SECURE_ID(KDummyId,0x00000000);
	
	CDataOwnerContainer* CDataOwnerContainer::NewL(TSecureId aSecureId, CDataOwnerManager* apDataOwnerManager)
	/** Symbian OS static constructor
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERCONTAINER_NEWL_ENTRY );
		CDataOwnerContainer* self = new(ELeave) CDataOwnerContainer(aSecureId);
		CleanupStack::PushL(self);
		self->ConstructL(apDataOwnerManager);			
		CleanupStack::Pop(self);
		
		OstTraceFunctionExit0( CDATAOWNERCONTAINER_NEWL_EXIT );
		return self;
		}
		
	CDataOwnerContainer::CDataOwnerContainer(TSecureId aSecureId) :
		iSecureId(aSecureId)
	/** Standard C++ constructor
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERCONTAINER_CDATAOWNERCONTAINER_CONS_ENTRY );
		OstTraceFunctionExit0( CDATAOWNERCONTAINER_CDATAOWNERCONTAINER_CONS_EXIT );
		}
		
	CDataOwnerContainer::~CDataOwnerContainer()
		{
		OstTraceFunctionEntry0( CDATAOWNERCONTAINER_CDATAOWNERCONTAINER_DES_ENTRY );
		delete ipDataOwner;
		OstTraceFunctionExit0( CDATAOWNERCONTAINER_CDATAOWNERCONTAINER_DES_EXIT );
		}
		
	void CDataOwnerContainer::ConstructL(CDataOwnerManager* apDataOwnerManager)
	/* Symbian second phase constructor
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERCONTAINER_CONSTRUCTL_ENTRY );
		ipDataOwner = CDataOwner::NewL(iSecureId, apDataOwnerManager);
		OstTraceFunctionExit0( CDATAOWNERCONTAINER_CONSTRUCTL_EXIT );
		}
		
	TSecureId CDataOwnerContainer::SecureId() const
	/** Secure Id accessor
	
	@return the secure id for the data owner container
	*/
		{
		return iSecureId;
		}
		
	CDataOwner& CDataOwnerContainer::DataOwner() const
	/** Data owner accessor
	
	@return the data owner
	*/
		{
		return (*ipDataOwner);
		}

	/**
	Method will be used for Sort on RPointerArray
	
	@param aFirst CDataOwnerContainer& data owner container to compare
	@param aSecond CDataOwnerContainer& data owner container to compare
	
	@see RArray::Sort()
	*/
	TInt CDataOwnerContainer::Compare(const CDataOwnerContainer& aFirst, const CDataOwnerContainer& aSecond)
		{
		OstTraceFunctionEntry0( CDATAOWNERCONTAINER_COMPARE_ENTRY );
		if (aFirst.SecureId() < aSecond.SecureId())
			{
			OstTraceFunctionExit0( CDATAOWNERCONTAINER_COMPARE_EXIT );
			return -1;
			}
 		else if (aFirst.SecureId() > aSecond.SecureId())
 			{
 			OstTraceFunctionExit0( DUP1_CDATAOWNERCONTAINER_COMPARE_EXIT );
 			return 1;
 			}
 		else 
 			{
 			OstTraceFunctionExit0( DUP2_CDATAOWNERCONTAINER_COMPARE_EXIT );
 			return 0;
 			}
		}
		
	/**
	Method will be used for Find on RPointerArray
	
	@param aFirst CDataOwnerContainer& data owner container to match
	@param aSecond CDataOwnerContainer& data owner container to match
	
	@see RArray::Find()
	*/
	TBool CDataOwnerContainer::Match(const CDataOwnerContainer& aFirst, const CDataOwnerContainer& aSecond)
		{
		return (aFirst.SecureId() == aSecond.SecureId());
		}

	CDataOwnerManager* CDataOwnerManager::NewLC()
	/** Symbian OS static constructor	
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_NEWLC_ENTRY );
		CDataOwnerManager* self	= new(ELeave) CDataOwnerManager();
		CleanupStack::PushL(self);
		self->ConstructL();
		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_NEWLC_EXIT );
		return self;
		}
		
	
	CDataOwnerManager::CDataOwnerManager() 
	/**
	Standard C++
	*/
	: ipABServer(NULL), iBufferFileReader(NULL), iDecompressor(NULL), 
	  iResetAfterRestore(EFalse), iJavaDOM(NULL), iSIDListForPartial(NULL),
	  iConfig(NULL), iBaBackupSession(NULL)
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_CDATAOWNERMANAGER_CONS_ENTRY );
		OstTraceFunctionExit0( CDATAOWNERMANAGER_CDATAOWNERMANAGER_CONS_EXIT );
		}
		
	void CDataOwnerManager::ConstructL()
	/** Symbian OS second phase contrutor
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_CONSTRUCTL_ENTRY );
		TInt err = iFs.Connect();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP4_CDATAOWNERMANAGER_CONSTRUCTL, "Leave: %d", err));
		err = RProperty::Define(TUid::Uid(KUidSystemCategoryValue), 
						  KUidBackupRestoreKey, 
						  RProperty::EInt, KReadPolicy, KWritePolicy, 0);
		if ((err != KErrNone) && (err != KErrAlreadyExists))
			{
		    OstTrace1(TRACE_ERROR, DUP3_CDATAOWNERMANAGER_CONSTRUCTL, "Leave: %d", err);
			User::Leave(err);
			}
			
		// Load a reference plugin with implementation Uid 0x2000D926
		const TUid aImplementationUid = { 0x2000D926 };
		iJavaDOM = CJavaManagerInterface::NewL( aImplementationUid );
		
		iDecompressor = CSBEDecompressAndEncrypt::NewL();
		iConfig = CSBEConfig::NewL(iFs);
		TRAP(err, iConfig->ParseL());
		if (err != KErrNone)
			{
		    OstTrace1(TRACE_NORMAL, CDATAOWNERMANAGER_CONSTRUCTL, "Error trying to parse sbeconfig.xml : %d", err);
		    OstTrace0(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_CONSTRUCTL, "Using Default Settings !");
			iConfig->SetDefault();
			}
		else
			{
		    OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_CONSTRUCTL, "sbeconfig.xml parsed sucessfully");
			}
		iBaBackupSession = CBaBackupSessionWrapper::NewL();
		iParserProxy = CSBEParserProxy::NewL(iFs);
		OstTraceFunctionExit0( CDATAOWNERMANAGER_CONSTRUCTL_EXIT );
		}


	CDataOwnerManager::~CDataOwnerManager()
	/** C++ Destructor
		
	Destructor functions are the inverse of constructor functions. They are called 
	when objects are destroyed (deallocated).

	The destructor is commonly used to "clean up" when an object is no longer necessary.
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_CDATAOWNERMANAGER_DES_ENTRY );
		if(iJavaDOM)
			{
			delete iJavaDOM;
			}
		
		delete iParserProxy;
		delete iBufferFileReader;
		delete iDecompressor;
		iDataOwners.ResetAndDestroy();
		iDataOwners.Close();
		iPackageDataOwners.ResetAndDestroy();
		iPackageDataOwners.Close();
		iFs.Close();
		if (iSIDListForPartial != NULL)
			{
			iSIDListForPartial->Close();
			delete iSIDListForPartial;
			}
		delete iConfig;
		delete iBaBackupSession;
		OstTraceFunctionExit0( CDATAOWNERMANAGER_CDATAOWNERMANAGER_DES_EXIT );
		}

	void CDataOwnerManager::AllSystemFilesRestoredL()
	/**
	Handle send from the client informing us that the reg files have now all been supplied and 
	we can parse/start active data owners etc.
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL_ENTRY );
		if(iBURType == EBURRestoreFull || iBURType == EBURRestorePartial)
			{
		    OstTrace0(TRACE_NORMAL, CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "called, parse reg files & start active DO's");
			// Build the list of dataOwners

			TInt err;
			TRAP(err, FindDataOwnersL());
			if (err != KErrNone)
				{
			    OstTrace1(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Error while finding data owners: %d", err);
				}

			TInt doCount = iDataOwners.Count();
			
			// Loop throught the list
			for (TInt x = 0; x < doCount; x++)
				{
				CDataOwnerContainer* pContainer = iDataOwners[x];
				CDataOwner& dataOwner = pContainer->DataOwner();
				TRAP_IGNORE(dataOwner.ParseFilesL());
				}

			doCount = iDataOwners.Count();
			
			// update partial state of active data owners
			TRAP(err, UpdateDataOwnersPartialStateL());
			if (err != KErrNone)
				{
			    OstTrace1(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Error while updating state: %d", err);
				}
				
			for (TInt index = 0; index < doCount; index++)
				{
				// Start data owning process if necessary for active data owners
				TRAP(err, iDataOwners[index]->DataOwner().StartProcessIfNecessaryL());
				if (err != KErrNone)
					{
				    OstTrace1(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Error while starting process if necessary: %d", err);
					}
				
				// Set up the internal state of the data owners now that all reg files and proxies are back
				TRAP(err, iDataOwners[index]->DataOwner().BuildDriveStateArrayL());
				if (err != KErrNone)
					{
				    OstTrace1(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Error while building drive array: %d", err);
					}
				}
			
			if(iJavaDOM)
				{				
				TRAP_IGNORE(iJavaDOM->AllSystemFilesRestored());
				}
			else
				{
			    OstTrace0(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
				}
			
			// now deal with special case packages
			CDesCArray* files = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
			CleanupStack::PushL(files);
			TRAP_IGNORE(FindRegistrationFilesL(KImportDir, *files));
			const TInt count = files->Count();
			for (TInt x = 0; x < count; x++)
				{
				// Strip the SID
				TSecureId sid;
				const TDesC& fileName = (*files)[x];
				TRAPD(err, StripSecureIdL(fileName, sid));
			
				if (err == KErrNone) // If there was an error then ignore it as it is probally not a dir
					{	
					CPackageDataTransfer* pDataTransfer = NULL;
					TRAPD(err, pDataTransfer = FindPackageDataContainerL(sid));
					if (err == KErrNone)
						{
						TRAP(err, pDataTransfer->SetRegistrationFileL(fileName));
						if (err == KErrNone)
							{
							TRAP(err, pDataTransfer->ParseL());
							}
						OstTraceExt2(TRACE_NORMAL, DUP6_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "found reg file: %S for Package: 0x%08x", fileName, sid.iId);
						}
					if (err == KErrNoMemory)
						{
					    OstTrace0(TRACE_ERROR, DUP8_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "Leave: KErrNoMemory");
						User::Leave(KErrNoMemory);
						}
					}
				} // for
				
			CleanupStack::PopAndDestroy(files);
			
			} // end if
		else
			{
		    OstTrace0(TRACE_NORMAL, DUP7_CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL, "*Error: called when device is not in Restore mode !");
			}
		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_ALLSYSTEMFILESRESTOREDL_EXIT );
		}

		
	void CDataOwnerManager::SetBURModeL(const TDriveList& aDriveList, TBURPartType aBURType, 
						 		   	   TBackupIncType aBackupIncType)
	/** Sets the publish & subscribe backup flag
	
	@param aDriveList a list of drives involved in the backup
	@param aBURType the type
	@param aBackupIncType the incremental type
	@leave KErrInUse a Software install is in progress, plus system wide errors
	*/						 		  
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_SETBURMODEL_ENTRY );
		OstTraceExt2(TRACE_NORMAL, CDATAOWNERMANAGER_SETBURMODEL, "Request new BURType (0x%08x), IncType (0x%08x)", aBURType, aBackupIncType);
		OstTraceExt2(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_SETBURMODEL, "CDataOwnerManager::SetBURModeL() - Previous BURType (0x%08x), IncType (0x%08x)", iBURType, iIncType);
		// Ensure that the device can't transition directly from backup to restore mode. It must
		// go through a normal state first. Allow the state to be set to the same.
		switch(aBURType)
			{
			case EBURNormal:
			// allow to set Normal mode in any case. no need to do anything if previous mode was Normal
				if (iBURType == EBURNormal)
					{
					OstTraceFunctionExit0( CDATAOWNERMANAGER_SETBURMODEL_EXIT );
					return;
					}
				break;
			case EBURUnset:
			// don't do anything if previous modes were Normal or Unset
				if (iBURType == EBURNormal || iBURType == EBURUnset)
					{
					OstTraceFunctionExit0( DUP1_CDATAOWNERMANAGER_SETBURMODEL_EXIT );
					return;
					}
				break;
			case EBURBackupFull:
			case EBURBackupPartial:
			case EBURRestoreFull:
			case EBURRestorePartial:
			// don't allow mode change unless , device was put into normal mode before
				if (iBURType != EBURNormal && iBURType != EBURUnset)
					{
				    OstTraceExt2(TRACE_ERROR, DUP2_CDATAOWNERMANAGER_SETBURMODEL, "*Error: BUR type has not transitioned between modes correctly, %d became %d", iBURType, aBURType);
					User::Leave(KErrCorrupt);
					}
				break;
			} // switch
			
		TBURPartType previousBURType = iBURType;	 
		
		// Need to reset the list of data owners, and old style babackup
		if ((aBURType == EBURNormal) || (aBURType == EBURUnset))
			{
		    OstTrace1(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_SETBURMODEL, "Going Normal/Unset/NoBackup (%d)", aBURType);
			// If we've transitioned from a Restore to a Normal mode, we need to send a RestoreComplete
			if (previousBURType == EBURRestoreFull || previousBURType == EBURRestorePartial) 
				{
			    OstTrace0(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_SETBURMODEL, "Calling RestoreCompleteL on all active data owners");
				TInt restoreCompleteCount = iDataOwners.Count();

				for (TInt index = 0; index < restoreCompleteCount; index++)
					{
					// Start data owning process if necessary for active data owners
					TRAP_IGNORE(iDataOwners[index]->DataOwner().RestoreCompleteL());
					}
					
				// Do we need to reset the device
				if (iResetAfterRestore)
					{
					//
					// Currently there is no Symbian wide way to reset a device. The below call
					// was intended to perform this function but has not been implemented. 
					//
					// When/If a Symbian way to reset a device comes into existance, or a licencee
					// has specific reset calls then this is the location to add such code.
					//
					//UserSvr::ResetMachine(EStartupWarmReset);
					} // if
				}

			iResetAfterRestore = EFalse;
			iDataOwners.ResetAndDestroy();
			iPackageDataOwners.ResetAndDestroy();
			OstTrace0(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_SETBURMODEL, "Restart All Non-System Applications");
			iBaBackupSession->NotifyBackupOperationL(TBackupOperationAttributes(MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd));
			iBaBackupSession->RestartAll();
			}
		else
			{
			// Check that SWInstall are not doing anything.
			TInt value;
			TInt regErr = RProperty::Get(KUidSystemCategory, Swi::KUidSoftwareInstallKey, value);
			if (regErr == KErrNone && value != Swi::ESwisNone)
				{
			    OstTrace0(TRACE_ERROR, DUP6_CDATAOWNERMANAGER_SETBURMODEL, "*Error: Leave software Install in progress.");
				User::Leave(KErrInUse);
				} // if
			else if (regErr != KErrNotFound && regErr != KErrNone)
				{
			    OstTrace0(TRACE_ERROR, DUP7_CDATAOWNERMANAGER_SETBURMODEL, "*Error: Leave could not get KUidSoftwareInsallKey");
				User::Leave(regErr);
				} // else	
			
			// Clobber files that are locked open
			TRequestStatus status;
			OstTrace0(TRACE_NORMAL, DUP8_CDATAOWNERMANAGER_SETBURMODEL, "Calling CloseAll()");
			if(aBURType == EBURBackupFull || aBURType == EBURBackupPartial)
 				{
 				TBackupOperationAttributes atts(MBackupObserver::EReleaseLockReadOnly, MBackupOperationObserver::EStart);
 				iBaBackupSession->NotifyBackupOperationL(atts);		 		 		 
 				iBaBackupSession->CloseAll(MBackupObserver::EReleaseLockReadOnly, status);
 				}
 			else
 				{
 				TBackupOperationAttributes atts(MBackupObserver::EReleaseLockNoAccess, MBackupOperationObserver::EStart);
 				iBaBackupSession->NotifyBackupOperationL(atts);		 		 		 
 				iBaBackupSession->CloseAll(MBackupObserver::EReleaseLockNoAccess, status);
 				}
 		 	User::WaitForRequest(status);
 		 	
 		 	OstTrace0(TRACE_NORMAL, DUP9_CDATAOWNERMANAGER_SETBURMODEL, "CloseAll() returned");
			
			// update partial state for active data owners
			if (aBURType == EBURBackupPartial)
				{
				UpdateDataOwnersPartialStateL();
				}
				
			if (aBURType == EBURBackupPartial || aBURType == EBURBackupFull)
				{
				TInt doCount = iDataOwners.Count();
				for (TInt index = 0; index < doCount; index++)
					{
					// Start data owning process if necessary for active data owners
					TRAPD(err, iDataOwners[index]->DataOwner().StartProcessIfNecessaryL());
					if (err != KErrNone)
						{
					    OstTraceExt2(TRACE_NORMAL, DUP10_CDATAOWNERMANAGER_SETBURMODEL, "Data owner (or proxy) with SID 0x%08x errored (%d) whilst starting", iDataOwners[index]->SecureId().iId, static_cast<TInt32>(err));
						}
					}
				}
			}
		
		TInt setError = RProperty::Set(TUid::Uid(KUidSystemCategoryValue), KUidBackupRestoreKey, aBURType | aBackupIncType);
		OstTraceExt3(TRACE_NORMAL, DUP11_CDATAOWNERMANAGER_SETBURMODEL, "Setting P&S flag to BURType (0x%08x), IncType (0x%08x), err: %d", static_cast<TUint>(aBURType), static_cast<TUint>(aBackupIncType), setError);
		LEAVEIFERROR(setError, OstTrace1(TRACE_ERROR, DUP13_CDATAOWNERMANAGER_SETBURMODEL, "Leave: %d", setError));
		
		// This configurable delay allows extra time to close all non-system apps.
		TUint closeDelay = iConfig->AppCloseDelay();
	 	if((closeDelay>0) && (aBURType == EBURBackupFull || aBURType == EBURBackupPartial || 
	 	                      aBURType == EBURRestoreFull || aBURType == EBURRestorePartial))
			{
			User::After(closeDelay);
			}
		
		iDriveList = aDriveList;
		iBURType = aBURType;
		iIncType = aBackupIncType;
		
		//When we set back to normal mode, invalidate all current available
		//CABSessions,since they could not be used in sequent backup/restore event
		if (aBURType == EBURUnset || aBURType == EBURNormal) 
			{
		    OstTrace1(TRACE_NORMAL, DUP12_CDATAOWNERMANAGER_SETBURMODEL, "Invalidate all ABSessions after set Setting P&S flag to 0x%08x", aBURType);
			ipABServer->InvalidateABSessions();
			}
		OstTraceFunctionExit0( DUP2_CDATAOWNERMANAGER_SETBURMODEL_EXIT );
		}

	
	void CDataOwnerManager::GetDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners)
	/** Gets the Information about data owners
	
	@param aDataOwners on return the list of data owners
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETDATAOWNERSL_ENTRY );
		if (iBURType != EBURNormal && iBURType != EBURUnset)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETDATAOWNERSL, "*Error: ListOfDataOnwers called when device isn't in Normal/Unset mode");
			User::Leave(KErrAccessDenied);
			}
			
		// Build the list of dataOwners
		FindDataOwnersL();
		Swi::RSisRegistrySession registrySession;
		TInt err = registrySession.Connect();
		LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, DUP9_CDATAOWNERMANAGER_GETDATAOWNERSL, "Leave: %d", err));
		CleanupClosePushL(registrySession);
		
		err = KErrNone;
		TUint count = iDataOwners.Count();
		// Loop throught the list
		while(count--)
			{
			CDataOwnerContainer* pContainer = iDataOwners[count];
			CDataOwner& dataOwner = pContainer->DataOwner();
			TSecureId secureId = pContainer->SecureId();
			
			// forwards declarations
			TDriveList driveList;
			driveList.SetMax();
			
			TCommonBURSettings commonSettings = ENoOptions;
			
			// parse registration files
			TRAP(err, dataOwner.ParseFilesL());
			if (err != KErrNone)
				{
			    OstTraceExt2(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_GETDATAOWNERSL, "ParseFilesL() - Error in sid: 0x%08x (%d)", secureId.iId, static_cast<TInt32>(err));
				} // if
			else
				{
				// Reset the state for these data owners
				TRAP(err, dataOwner.BuildDriveStateArrayL());
				if (err != KErrNone)
					{
				    OstTraceExt2(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_GETDATAOWNERSL, "BuildDriveStateArrayL() - Error in sid: 0x%08x (%d)", secureId.iId, static_cast<TInt32>(err));
					}//if
				else 
					{
					// Get drive list (this is needed to update drive list for packages)
					TRAP(err, dataOwner.GetDriveListL(driveList));
					if (err != KErrNone)
						{
					    OstTraceExt2(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_GETDATAOWNERSL, "GetDriveListL() - Error in sid: 0x%08x (%d)", secureId.iId, static_cast<TInt32>(err));
						}//if		
					else
						{
						TRAP(err, commonSettings = dataOwner.CommonSettingsL());
						if (err != KErrNone)
							{
						    OstTraceExt2(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_GETDATAOWNERSL, "CommonSettingsL() - Error in sid: 0x%08x (%d)", secureId.iId, static_cast<TInt32>(err));
							}//if		
						}//else
					}
				}//else
			
			CSBGenericDataType* pId = NULL;
			if (err == KErrNone)
				{
				// check if the sid is part of the package
				if ((commonSettings & EHasSystemFiles) == EHasSystemFiles)
					{
					Swi::CSisRegistryPackage* pRegistryPackage = NULL;
					TRAPD(error, pRegistryPackage = registrySession.SidToPackageL(secureId));
					if ((error == KErrNone))
						{
						TUid packageUid = pRegistryPackage->Uid();
						OstTraceExt2(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_GETDATAOWNERSL, "Found package for secure id 0x%08x, package id 0x%08x", secureId.iId, packageUid.iUid);
						
						CleanupStack::PushL(pRegistryPackage);
						pId = CSBPackageId::NewL(packageUid, secureId, pRegistryPackage->Name());
						CleanupStack::PopAndDestroy(pRegistryPackage);
						CleanupStack::PushL(pId);
						
						// finds or adds package to the internal array
						CPackageDataTransfer* pak = FindPackageDataContainerL(packageUid);
						//renews the drive list
						TRAP(err, pak->GetDriveListL(driveList));
						if( err == KErrNotSupported)
							{
						    OstTrace0(TRACE_NORMAL, DUP6_CDATAOWNERMANAGER_GETDATAOWNERSL, "Error KErrNotSupported");
							err = KErrNone;
							}
						} // if
					else
						{
					    OstTraceExt2(TRACE_NORMAL, DUP7_CDATAOWNERMANAGER_GETDATAOWNERSL, "Error(%d) retrieving package data for sid 0x%08x", static_cast<TInt32>(error), secureId.iId);
						} // else
					} // if
				} // if
	
			if (pId == NULL) // not a package or error happend
				{
				pId = CSBSecureId::NewL(secureId);
				CleanupStack::PushL(pId);
				}	
				
			//
			// dont create it if not required
			
			CDataOwnerInfo* pDataOwnerInfo = NULL;			
			if (err != KErrNone)
				{
				// There has been an error create a blank data owner so the PC will now
				pDataOwnerInfo = CDataOwnerInfo::NewL(pId, ENoOptions, ENoPassiveOptions, 
													  ENoActiveOptions, driveList);
				}
			else
				{
				if (commonSettings & EPassiveBUR || commonSettings & EHasSystemFiles || dataOwner.ActiveInformation().iActiveType != EProxyImpOnly || dataOwner.PassiveSettingsL() & EHasPublicFiles)
					{
					pDataOwnerInfo = CDataOwnerInfo::NewL(pId, commonSettings, dataOwner.PassiveSettingsL(),
														dataOwner.ActiveSettingsL(), driveList);
					}
				} // else
			
			if (pDataOwnerInfo != NULL)
				{
				// ownership was trasferred to pDataOwnerInfo
				CleanupStack::Pop(pId);
				CleanupStack::PushL(pDataOwnerInfo);	
				aDataOwners.AppendL(pDataOwnerInfo);
				CleanupStack::Pop(pDataOwnerInfo);
				}
			else
				{
				CleanupStack::PopAndDestroy(pId);
				}
			} // for
			
		// find special case packages which don't have private directories.
		TRAP_IGNORE(FindImportPackagesL(registrySession, aDataOwners));
		
		CleanupStack::PopAndDestroy(&registrySession);
		
		if(iJavaDOM)
			{
			iJavaDOM->GetDataOwnersL(aDataOwners);
			}
		else
			{
		    OstTrace0(TRACE_NORMAL, DUP8_CDATAOWNERMANAGER_GETDATAOWNERSL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
			}
		OstTraceFunctionExit0( CDATAOWNERMANAGER_GETDATAOWNERSL_EXIT );
		}		
		
	CDataOwner& CDataOwnerManager::DataOwnerL(TSecureId aSID)
	/**
	Called by the ABServer when creating a session in order for the session to query the DataOwner
	
	@param aSID The SID of the active data owner
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_DATAOWNERL_ENTRY );
		CDataOwnerContainer* pDOContainer = NULL;
		
		pDOContainer = FindL(aSID);

		if (!pDOContainer)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_DATAOWNERL, "Leave: KErrNotFound");
			User::Leave(KErrNotFound);
			}

		CDataOwner& dataOwner = pDOContainer->DataOwner();
		OstTraceFunctionExit0( CDATAOWNERMANAGER_DATAOWNERL_EXIT );
		return dataOwner;
		}

	void CDataOwnerManager::GetExpectedDataSizeL(CSBGenericTransferType* apGenericTransferType, TUint& aSize)
	/** Gets the expected data size of a backup for the given information

	@param apGenericTransferType the generic transfer type
	@param aSize on return the expected data size of the backup
	@post deletes the apGenericTransferType
	@leave KErrNotSupported unsupported transfer type
	@leave KErrNotFound object relating to apGenericTransferType not found
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL_ENTRY );
		if (iBURType != EBURBackupPartial && iBURType != EBURBackupFull)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "*Error: GetExpectedDataSizeL called when device is not in Backup mode !");
			User::Leave(KErrAccessDenied);
			}
			
		switch (apGenericTransferType->DerivedTypeL())
			{
			case ESIDTransferDerivedType:
				{
				OstTrace0(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "ESIDTransferDerivedType");
				CSBSIDTransferType* pSIDTransferType = CSBSIDTransferType::NewL(apGenericTransferType);
				CleanupStack::PushL(pSIDTransferType);
				
				DataOwnerL(pSIDTransferType->SecureIdL()).GetExpectedDataSizeL(pSIDTransferType->DataTypeL(), pSIDTransferType->DriveNumberL(), aSize);
				CleanupStack::PopAndDestroy(pSIDTransferType);
				break;
				};
			case EPackageTransferDerivedType:
				{
				OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "EPackageTransferDerivedType");

				// This code should be changed.  Ideally, the GetExpectedDataSizeL method should be virtual, rendering 
				// this switch statement unnecessary.  When java support is added this will become even more important.
				//
				// For the moment, to avoid re-structuring the data owner class, 
				// we are using completely separate classes to handle packages and non packages
				
				CSBPackageTransferType* pPackageTransferType = CSBPackageTransferType::NewL(apGenericTransferType);
				CleanupStack::PushL(pPackageTransferType);
				//
				const TUid packageId = pPackageTransferType->PackageIdL();
				const TPackageDataType dataType = pPackageTransferType->DataTypeL();
				const TDriveNumber driveNumber = pPackageTransferType->DriveNumberL();
				//
				OstTraceExt3(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "package id: 0x%08x, dataType: %d, drive: %c ", packageId.iUid, static_cast<TInt32>(dataType), static_cast<TInt8>(driveNumber + 'A'));
				CPackageDataTransfer* pDataTransfer = FindPackageDataContainerL(pPackageTransferType->PackageIdL());
				pDataTransfer->GetExpectedDataSizeL(dataType, driveNumber, aSize);
				CleanupStack::PopAndDestroy(pPackageTransferType);
				break;
				}

			case EJavaTransferDerivedType:
				{
				OstTrace0(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "EJavaTransferDerivedType");

				// Call the Java DOM to calculate and return the expected size of the data specified in 
				// apGenericTransferType
				if(iJavaDOM)
					{
					iJavaDOM->GetExpectedDataSizeL(apGenericTransferType, aSize);
					}
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
					}

				break;
				}

			default:
				{
				OstTrace0(TRACE_ERROR, DUP6_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "unsupported transfer type");
				User::Leave(KErrNotSupported);
				}
			} // switch
		OstTrace1(TRACE_NORMAL, DUP7_CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL, "size is: %d", aSize);
		OstTraceFunctionExit0( CDATAOWNERMANAGER_GETEXPECTEDDATASIZEL_EXIT );
		}


	void CDataOwnerManager::GetPublicFileListL(CSBGenericDataType* aGenericDataType, 
											   TDriveNumber aDriveNumber,
											   RFileArray& aFiles)
	/** Retreives the public file list for the given secure id

	@param aSID 			the Secure Id of the data owner you are requesting information for
	@param aDriveNumber		the drive number to obtain the public files for
	@param aFiles			on return the list of public files
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETPUBLICFILELISTL_ENTRY );
		if (iBURType != EBURBackupPartial && iBURType != EBURBackupFull)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETPUBLICFILELISTL, "*Error: GetPublicFileListL called when device is not in Backup mode !");
			User::Leave(KErrAccessDenied);
			}
		
		if (!(iDriveList[aDriveNumber]))
			{
		    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_GETPUBLICFILELISTL, "The drive in the argument is not in the list of the drives for backup/restore");
			User::Leave(KErrArgument);
			}
		
		switch (aGenericDataType->DerivedTypeL())
			{
			case EPackageDerivedType:
				{
				CSBPackageId* pUID = CSBPackageId::NewL(aGenericDataType);
				CleanupStack::PushL(pUID);
				
				FindPackageDataContainerL(pUID->PackageIdL())->GetPublicFileListL(aDriveNumber, aFiles);
				CleanupStack::PopAndDestroy(pUID);
				break;
				}
			case ESIDDerivedType:
				{
				CSBSecureId* pSID = CSBSecureId::NewL(aGenericDataType);
				CleanupStack::PushL(pSID);
				
				DataOwnerL(pSID->SecureIdL()).GetPublicFileListL(aDriveNumber, aFiles);
				CleanupStack::PopAndDestroy(pSID);
				break;
				}

			case EJavaDerivedType:
				{
				if(iJavaDOM)
					{
					iJavaDOM->GetPublicFileListL(aGenericDataType, aDriveNumber, aFiles);
					}
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_GETPUBLICFILELISTL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
					}
				break;
				}

			default:
				{
				OstTrace0(TRACE_ERROR, DUP3_CDATAOWNERMANAGER_GETPUBLICFILELISTL, "Leave: KErrNotSupported");
				User::Leave(KErrNotSupported);
				}
			}
		OstTraceFunctionExit0( CDATAOWNERMANAGER_GETPUBLICFILELISTL_EXIT );
		}

		
    void CDataOwnerManager::GetRawPublicFileListL(CSBGenericDataType* aGenericDataType, TDriveNumber aDriveNumber, 
    						   					  RRestoreFileFilterArray& aFileFilter)
	/** Retrieves the raw list as described in the XML files
	
	@param aSID the secure id
	@param aDriveNumber the drive number
	@param aFileFilter on return an array of TRestoreFileFilter
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL_ENTRY );
		if (iBURType != EBURBackupPartial && iBURType != EBURBackupFull)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL, "*Error: GetRawPublicFileListL called when device is not in Backup mode !");
			User::Leave(KErrAccessDenied);
			}
			
		if (!(iDriveList[aDriveNumber]))
			{
		    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL, "The drive in the argument is not in the list of the drives for backup/restore");
			User::Leave(KErrArgument);
			}
		
		switch (aGenericDataType->DerivedTypeL())
			{
			case EPackageDerivedType:
				{
				CSBPackageId* pUID = CSBPackageId::NewL(aGenericDataType);
				CleanupStack::PushL(pUID);
				
				FindPackageDataContainerL(pUID->PackageIdL())->GetRawPublicFileListL(aDriveNumber, aFileFilter);
				CleanupStack::PopAndDestroy(pUID);
				break;
				}
			case ESIDDerivedType:
				{
				CSBSecureId* pSID = CSBSecureId::NewL(aGenericDataType);
				CleanupStack::PushL(pSID);
				
				DataOwnerL(pSID->SecureIdL()).GetRawPublicFileListL(aDriveNumber, aFileFilter);

				CleanupStack::PopAndDestroy(pSID);
				break;
				}

			case EJavaDerivedType:
				{
				if(iJavaDOM)
					{
					iJavaDOM->GetRawPublicFileListL(aGenericDataType, aDriveNumber, aFileFilter);
					}
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
					}
				break;
				}

			default:
				{
				OstTrace0(TRACE_ERROR, DUP3_CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL, "Leave: KErrNotSupported");
				User::Leave(KErrNotSupported);
				}
			}
		OstTraceFunctionExit0( CDATAOWNERMANAGER_GETRAWPUBLICFILELISTL_EXIT );
		}
    	
    	
    void CDataOwnerManager::GetXMLPublicFileListL(TSecureId /*aSID*/, TDriveNumber /*aDriveNumber*/, 
    											  HBufC*& /*aBuffer*/)
	/** Gets the XML public file list
	
	@param aSID the secure id
	@param aDriveNumber the drive number
	@param aBuffer the buffer to write the data too
	
	*/
    	{
    	OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETXMLPUBLICFILELISTL_ENTRY );
    	if (iBURType != EBURBackupPartial && iBURType != EBURBackupFull)
			{
    	    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETXMLPUBLICFILELISTL, "*Error: GetXMLPublicFileListL called when device is not in Backup mode !");
			User::Leave(KErrAccessDenied);
			}
		else 
			{
			//will need to check if the drive exists in our list
		    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_GETXMLPUBLICFILELISTL, "*Error: GetXMLPublicFileListL Not Yet Implemented");
			User::Leave(KErrNotSupported);
			}
    	OstTraceFunctionExit0( CDATAOWNERMANAGER_GETXMLPUBLICFILELISTL_EXIT );
    	}
    	
	void CDataOwnerManager::SetSIDListForPartialBURL(TDesC8& aFlatArrayPtr)
	/**
	Sets the list of Active SID's participating in a backup or restore
	
	@param aFlatArrayPtr Flat Array Pointer
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_SETSIDLISTFORPARTIALBURL_ENTRY );
		if (iBURType != EBURNormal && iBURType != EBURUnset)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_SETSIDLISTFORPARTIALBURL, "*Error: called when device isn't in Normal/Unset mode");
			User::Leave(KErrAccessDenied);
			}
		
		if (iSIDListForPartial != NULL)
			{
			iSIDListForPartial->Close();
			delete iSIDListForPartial;
			iSIDListForPartial = NULL;
			}
			
		iSIDListForPartial = RSIDArray::InternaliseL(aFlatArrayPtr);
		OstTraceFunctionExit0( CDATAOWNERMANAGER_SETSIDLISTFORPARTIALBURL_EXIT );
		}
	
	void CDataOwnerManager::UpdateDataOwnersPartialStateL()
	/**
	Specifies the list of SID's that are to be backed up in a partial backup
	
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_UPDATEDATAOWNERSPARTIALSTATEL_ENTRY );
		if (iSIDListForPartial != NULL)
			{
			TUint count = iSIDListForPartial->Count();
			
			while(count--)
				{
				// Find the data owner responsible for this SID and set it's partial backup flag to ETrue
				DataOwnerL((*iSIDListForPartial)[count]).SetBackedUpAsPartial(ETrue);
				} // for
			} // if
		OstTraceFunctionExit0( CDATAOWNERMANAGER_UPDATEDATAOWNERSPARTIALSTATEL_EXIT );
		}
	
	void CDataOwnerManager::SIDStatusL(RSIDStatusArray& aSIDStatus)
	/**
	Returns the ready statuses of selected Data Owner's
	
	@param aSIDStatus Array of SID's and their associated statuses. The statuses will be populated upon return
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_SIDSTATUSL_ENTRY );
		if (iBURType == EBURNormal || iBURType == EBURUnset)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_SIDSTATUSL, "*Error: called when device is in Normal/Unset mode");
			User::Leave(KErrAccessDenied);
			}
			
		TUint count = aSIDStatus.Count();
		CDataOwnerContainer* pDOContainer = NULL;
		
		while(count--)
			{
			pDOContainer = FindL(aSIDStatus[count].iSID);
			if (!pDOContainer)
				{
				aSIDStatus[count].iStatus = EDataOwnerNotFound;
				}
			else
				{
				// Assign the status of the data owner to the array element
				aSIDStatus[count].iStatus = pDOContainer->DataOwner().ReadyState();
				}
			}
		OstTraceFunctionExit0( CDATAOWNERMANAGER_SIDSTATUSL_EXIT );
		}
		
	void CDataOwnerManager::AllSnapshotsSuppliedL()
	/**
	All the snapshots have been supplied
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_ALLSNAPSHOTSSUPPLIEDL_ENTRY );		
		if (iBURType == EBURBackupPartial || iBURType == EBURBackupFull)
			{
			TUint count = iDataOwners.Count();
			while(count--)
				{
				CDataOwner* dataOwner = &iDataOwners[count]->DataOwner();
				if (dataOwner->PartialAffectsMe() && dataOwner->ActiveInformation().iActiveDataOwner && (dataOwner->ActiveInformation().iActiveType != EProxyImpOnly))
					{
					TSecureId id = dataOwner->SecureId();
					const TUint KActiveStateMaxRetries = 4;
					const TUint KActiveStateDelay = 500000;
					TUint retries = 0;
					do
						{
						TRAPD(err, ipABServer->AllSnapshotsSuppliedL(id));
						if (err == KErrNotFound)
							{
							retries++;
							User::After(KActiveStateDelay);
							}
						else
							{
							break;
							}
						} while (retries < KActiveStateMaxRetries);
					} // if
				} // while
			} //if
		else 
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_ALLSNAPSHOTSSUPPLIEDL, "*Error: can only be called in Backup mode");
			User::Leave(KErrAccessDenied);
			} // else		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_ALLSNAPSHOTSSUPPLIEDL_EXIT );
		}

	void CDataOwnerManager::GetNextPublicFileL(CSBGenericDataType* aGenericDataType,
											   TBool aReset,
	                           				   TDriveNumber aDriveNumber,
	                           				   TEntry& aEntry)
	/** Retreives the next public file associated with the given secure id

	@param aGenericDataType the generic data type
	@param aReset set true to start reading from the beginning of the list
	@param aDriveNumber the drive to retrieve the public files for
	@param aEntry on return the next entry in the list, an empty entry indicates the end of the list has been reached
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_GETNEXTPUBLICFILEL_ENTRY );		
		if (iBURType != EBURBackupPartial && iBURType != EBURBackupFull)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_GETNEXTPUBLICFILEL, "*Error: GetPublicFileListL called when device is not in Backup mode !");
			User::Leave(KErrAccessDenied);
			}
		
		if (!(iDriveList[aDriveNumber]))
			{
		    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_GETNEXTPUBLICFILEL, "The drive in the argument is not in the list of the drives for backup/restore");
			User::Leave(KErrArgument);
			}
		
		if (aGenericDataType->DerivedTypeL() == ESIDDerivedType)
			{
			CSBSecureId* pSID = CSBSecureId::NewL(aGenericDataType);
			CleanupStack::PushL(pSID);

			DataOwnerL(pSID->SecureIdL()).GetNextPublicFileL(aReset, aDriveNumber, aEntry);
			CleanupStack::PopAndDestroy(pSID);
			}
		else
			{
		    OstTrace0(TRACE_ERROR, DUP2_CDATAOWNERMANAGER_GETNEXTPUBLICFILEL, "Leave: KErrNotSupported");
			User::Leave(KErrNotSupported);
			}		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_GETNEXTPUBLICFILEL_EXIT );
		}


    void CDataOwnerManager::SupplyDataL(CSBGenericTransferType* apGenericTransferType, TDesC8& aBuffer, 
    				 					TBool aLastSection)
    /** Supply data
    
    @param apGenericTransferType the generic transfertype. NOTE: This function will delete this.
    @param aBuffer the buffer to supply data from.
    @param aLastSection is this the last section.
    @leave KErrNotSupported Unknown transfer type
    @leave KErrNotFound Unknown object
    */
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_SUPPLYDATAL_ENTRY );
		OstTrace1(TRACE_NORMAL, CDATAOWNERMANAGER_SUPPLYDATAL, "about to decompress %d bytes of data", aBuffer.Length());

        if (iBURType == EBURNormal || iBURType == EBURUnset)
			{
            OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_SUPPLYDATAL, "*Error: called not when device in Normal/Unset mode");
			User::Leave(KErrAccessDenied);
			}
			
		// The buffer that we are given is compressed. We need to uncompress this into 
		// 64K chunks and then pass through the code
		iDecompressor->SetGenericTransferTypeL(apGenericTransferType);
		iDecompressor->SetBuffer(aBuffer);
		TBool moreData = ETrue;
		
		// Packages some times need to stop before the data has all been uncompressed.
		TBool packageContinue = ETrue;
		
		HBufC8* uncompressedData = NULL;
		while (moreData && packageContinue)
			{
			uncompressedData = NULL;
			if (!iDecompressor->NextLC(uncompressedData, moreData))
				{
			    OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_SUPPLYDATAL, "iDecompressor->NextLC returned EFalse");
				if (uncompressedData != NULL)
					{
				    OstTrace0(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_SUPPLYDATAL, "uncompressedData not NULL so cleaning up");
					CleanupStack::PopAndDestroy(uncompressedData);
					}
				break;
				}
				
			if (uncompressedData == NULL)
				{
			    OstTrace0(TRACE_ERROR, DUP4_CDATAOWNERMANAGER_SUPPLYDATAL, "uncompressedData is NULL after NextLC, corrupt data");
				User::Leave(KErrCorrupt);
				}
			
			TPtr8 dataPtr(uncompressedData->Des());
			OstTrace1(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_SUPPLYDATAL, "decompressed data length: %d", dataPtr.Length());
			
			// Check aLastSection
			TBool lastSection = aLastSection && !moreData;
			
			switch(apGenericTransferType->DerivedTypeL())
				{
			case ESIDTransferDerivedType:
				{
				OstTrace0(TRACE_NORMAL, DUP6_CDATAOWNERMANAGER_SUPPLYDATAL, "ESIDTransferDerivedType");
				CSBSIDTransferType* pSIDTransferType = CSBSIDTransferType::NewL(apGenericTransferType);
				CleanupStack::PushL(pSIDTransferType);
				
				// Is this the data for registration files? These are now not supported
				if (pSIDTransferType->DataTypeL() == ERegistrationData)
					{
				    OstTrace0(TRACE_ERROR, DUP12_CDATAOWNERMANAGER_SUPPLYDATAL, "Leave: KErrNotSupported");
					User::Leave(KErrNotSupported);
					} // if
				else
					{
					// Does this dataowner require a reboot?
					const TSecureId sid = pSIDTransferType->SecureIdL();
					const TDriveNumber driveNumber = pSIDTransferType->DriveNumberL();
					CDataOwner& dataOwner = DataOwnerL(sid);
					OstTraceExt2(TRACE_NORMAL, DUP7_CDATAOWNERMANAGER_SUPPLYDATAL, "trying to restore data for SID: 0x%08x, drive: %c", sid.iId, static_cast<TInt8>('A' + driveNumber));

					if ((dataOwner.CommonSettingsL() & ERequiresReboot) == ERequiresReboot)
						{
					    OstTrace1(TRACE_NORMAL, DUP8_CDATAOWNERMANAGER_SUPPLYDATAL, "data owner 0x%08x requires a REBOOT!", sid.iId);
						iResetAfterRestore = ETrue;
						}
					
					dataOwner.SupplyDataL(driveNumber, pSIDTransferType->DataTypeL(), dataPtr, lastSection);
					} // else
			
				CleanupStack::PopAndDestroy(pSIDTransferType);
				break;
				}
			case EPackageTransferDerivedType:
				{
				OstTrace0(TRACE_NORMAL, DUP9_CDATAOWNERMANAGER_SUPPLYDATAL, "EPackageTransferDerivedType");
				// Ideally, we would use the same CDataOwner class, or a class derived
				// from it to handle the package backup/restore, however to do this would 
				// require a re-design.
				CSBPackageTransferType *pPackageTransferType = CSBPackageTransferType::NewL(apGenericTransferType);
				CleanupStack::PushL(pPackageTransferType);
					
				TUid packageId = pPackageTransferType->PackageIdL();
					
				CPackageDataTransfer* pDataTransfer = FindPackageDataContainerL(packageId);
				pDataTransfer->SupplyDataL(pPackageTransferType->DriveNumberL(), 
										   pPackageTransferType->DataTypeL(),
										   dataPtr, lastSection);
				
				CleanupStack::PopAndDestroy(pPackageTransferType);
				break;
				}

		    case EJavaTransferDerivedType:
			    { 
			    OstTrace0(TRACE_NORMAL, DUP10_CDATAOWNERMANAGER_SUPPLYDATAL, "EJavaTransferDerivedType");
			   if(iJavaDOM)
					{
				    iJavaDOM->SupplyDataL(apGenericTransferType, dataPtr, lastSection);
					}
				else
					{
				    OstTrace0(TRACE_NORMAL, DUP11_CDATAOWNERMANAGER_SUPPLYDATAL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
					}
			    break;
			    }					

            default:
				{
				OstTrace0(TRACE_ERROR, DUP13_CDATAOWNERMANAGER_SUPPLYDATAL, "Leave: KErrNotSupported");
				User::Leave(KErrNotSupported);
				}
				} // switch

			// Cleanup
			CleanupStack::PopAndDestroy(uncompressedData);
			} // while		
    	OstTraceFunctionExit0( CDATAOWNERMANAGER_SUPPLYDATAL_EXIT );
    	}


    void CDataOwnerManager::RequestDataL(CSBGenericTransferType* apGenericTransferType,
    	    		  					 TPtr8& aBuffer, TBool& aLastSection)
	/** Request data
	
    @param apGenericTransferType the generic transfertype. NOTE: This function will delete this.
    @param aBuffer the buffer to write data to.
    @param aLastSection is this the last section.
    @leave KErrNotSupported Unknown transfer type
    @leave KErrNotFound Unknown object
	*/
    	{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_REQUESTDATAL_ENTRY );
		OstTraceExt2(TRACE_NORMAL, CDATAOWNERMANAGER_REQUESTDATAL, "aBuffer.Ptr(): 0x%08x, aBuffer.Length(): %d", reinterpret_cast<TInt32>(aBuffer.Ptr()), static_cast<TInt32>(aBuffer.Length()));
    	if (iBURType == EBURNormal || iBURType == EBURUnset)
			{
    	    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_REQUESTDATAL, "*Error: called when device is in Normal/Unset mode");
			User::Leave(KErrAccessDenied);
			}
			
		// Reserve space to perform inline compression later
		CSBECompressAndEncrypt* pCE = CSBECompressAndEncrypt::NewLC(apGenericTransferType, aBuffer);
		
		switch(apGenericTransferType->DerivedTypeL())
			{
		case ESIDTransferDerivedType:
			{
			CSBSIDTransferType* pSIDTransferType = CSBSIDTransferType::NewL(apGenericTransferType);
			CleanupStack::PushL(pSIDTransferType);
			TSecureId sid(pSIDTransferType->SecureIdL());
			DataOwnerL(sid).RequestDataL(pSIDTransferType->DriveNumberL(), 
												 pSIDTransferType->DataTypeL(),
												 aBuffer, aLastSection);
												
			CleanupStack::PopAndDestroy(pSIDTransferType);
			break;
			}
		case EPackageTransferDerivedType:
			{
			CSBPackageTransferType *pPackageTransferType = CSBPackageTransferType::NewL(apGenericTransferType);
			CleanupStack::PushL(pPackageTransferType);
			
			CPackageDataTransfer *packageDataTransfer = 
				FindPackageDataContainerL(pPackageTransferType->PackageIdL());
			
			packageDataTransfer->RequestDataL(pPackageTransferType->DriveNumberL(),
											  pPackageTransferType->DataTypeL(),
											  aBuffer, aLastSection);
			
			CleanupStack::PopAndDestroy(pPackageTransferType);
					
			break;	 
			}

		case EJavaTransferDerivedType:
			{
			if(iJavaDOM)
				{
				iJavaDOM->RequestDataL(apGenericTransferType, aBuffer, aLastSection);
				}
			else
				{
			    OstTrace0(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_REQUESTDATAL, "Java Backup-Restore Plug-In not loaded, java files won't be backed or restored");
				}
			break;
			}

		default:
			{
			OstTrace0(TRACE_ERROR, DUP5_CDATAOWNERMANAGER_REQUESTDATAL, "Leave: KErrNotSupported");
			User::Leave(KErrNotSupported);
			}
			} // switch
			
		// Compress the data block
		if (aBuffer.Size() > 0) // Dont compress no data
			{
		    OstTrace1(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_REQUESTDATAL, "got %d bytes of uncompressed data, about to pack it...", aBuffer.Length());
			pCE->PackL(aBuffer);
			}
		else
			{
		    OstTrace1(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_REQUESTDATAL, "got %d bytes of uncompressed data, free reserved space...", aBuffer.Length());
			pCE->FreeReservedSpace(aBuffer);
			}
		
		
		CleanupStack::PopAndDestroy(pCE);		
    	OstTraceFunctionExit0( CDATAOWNERMANAGER_REQUESTDATAL_EXIT );
    	}
	                     
	// Accessors
	void CDataOwnerManager::SetActiveBackupServer(CABServer* aABServer)
		{
		ipABServer = aABServer;		
		}
		
	RFs& CDataOwnerManager::GetRFs()
		{
		return iFs;
		}

	// PRIVATE //
	/** Searches for registration files in Import Directories
	
	@param aDataOwners appended list of data owners infos
	
	*/
	void CDataOwnerManager::FindImportPackagesL(Swi::RSisRegistrySession& aRegistry, RPointerArray<CDataOwnerInfo>& aDataOwners)
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_FINDIMPORTPACKAGESL_ENTRY );		
		CDesCArray* files = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
		CleanupStack::PushL(files);
		FindRegistrationFilesL(KImportDir, *files);
		const TInt count = files->Count();
		
		RSisRegistryEntry entry;
		CleanupClosePushL(entry);
		
		for (TInt x = 0; x < count; x++)
			{
			// Strip the SID
			TSecureId sid;
			const TDesC& fileName = (*files)[x];
			TRAPD(err, StripSecureIdL(fileName, sid));
			
			if (err == KErrNone) // If there was an error then ignore it as it is probally not a dir
				{	
				err = entry.Open(aRegistry, sid);
				if (err == KErrNone)
					{
				    OstTraceExt2(TRACE_NORMAL, CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "found reg file: %S for Package: 0x%08x", fileName, sid.iId);
					CPackageDataTransfer* pDataTransfer = FindPackageDataContainerL(sid);
					
					TRAP(err, pDataTransfer->SetRegistrationFileL(fileName));
					if (err == KErrNone)
						{
						TRAP(err, pDataTransfer->ParseL());	
						}
					if (err == KErrNoMemory)
						{
					    OstTrace0(TRACE_ERROR, DUP8_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "Leave: KErrNoMemory");
						User::Leave(KErrNoMemory);
						}
					// create generic data type
					HBufC* pkgName = entry.PackageNameL();
					CleanupStack::PushL(pkgName);
					CSBGenericDataType* pId = CSBPackageId::NewL(sid, KDummyId, *pkgName);
					CleanupStack::PopAndDestroy(pkgName);
					CleanupStack::PushL(pId);
					
					TDriveList driveList;
					TRAP(err, pDataTransfer->GetDriveListL(driveList));
					if (err != KErrNone)
					    {//Non-removable, ignore this data owner
						CleanupStack::PopAndDestroy(pId);
						entry.Close();
						continue;
					    }
					// create a data owner info
					CDataOwnerInfo* pDataOwnerInfo = CDataOwnerInfo::NewL(pId, pDataTransfer->CommonSettingsL(),
														  pDataTransfer->PassiveSettingsL(), pDataTransfer->ActiveSettingsL(),
														  driveList);
					CleanupStack::PushL(pDataOwnerInfo);
					aDataOwners.AppendL(pDataOwnerInfo);
					CleanupStack::Pop(pDataOwnerInfo);
					
					CleanupStack::Pop(pId);
					
					/*
					 * Check wheahter Package has public files or not. If yes then add the SID dataowner to process public files.					 * 
					 */
					TPassiveBURSettings passiveBURSettings = ENoPassiveOptions;
					TRAPD( passiveErr, passiveBURSettings = pDataTransfer->PassiveSettingsL());
					
					if ( passiveErr == KErrNone && EHasPublicFiles & passiveBURSettings )
						{
						CDataOwnerContainer* pDataOwner = FindL(sid);
						if (pDataOwner == NULL)  // If it does not exist we need to create it
							{
						    OstTrace1(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "Package has the public files for SID: 0x%08x", sid.iId);
	
							pDataOwner = CDataOwnerContainer::NewL(sid, this);
							CleanupStack::PushL(pDataOwner);
							
							iDataOwners.InsertInOrder(pDataOwner, CDataOwnerContainer::Compare);
							CleanupStack::Pop(pDataOwner);										
							} // if
						else
							{
						    OstTrace1(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "SID already exists in the list SID: 0x%08x", sid.iId);
							}
	
						TRAP_IGNORE(pDataOwner->DataOwner().AddRegistrationFilesL(fileName));					
						
						// forwards declarations
						TDriveList driveList;
						driveList.SetMax();
						
												
						// parse registration files
						TRAP(err, pDataOwner->DataOwner().ParseFilesL());
						if (err != KErrNone)
							{
						    OstTraceExt2(TRACE_NORMAL, DUP3_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "ParseFilesL() - Error in sid: 0x%08x (%d)", sid.iId, static_cast<TInt32>(err));
							} // if
						else
							{
							// Reset the state for these data owners
							TRAP(err, pDataOwner->DataOwner().BuildDriveStateArrayL());
							if (err != KErrNone)
								{
							    OstTraceExt2(TRACE_NORMAL, DUP4_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "BuildDriveStateArrayL() - Error in sid: 0x%08x (%d)", sid.iId, static_cast<TInt32>(err));
								}//if
							else 
								{
								// Get drive list (this is needed to update drive list for packages)
								TRAP(err, pDataOwner->DataOwner().GetDriveListL(driveList));
								if (err != KErrNone)
									{
								    OstTraceExt2(TRACE_NORMAL, DUP5_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "GetDriveListL() - Error in sid: 0x%08x (%d)", sid.iId, static_cast<TInt32>(err));
									}//if		
								else
									{
									TRAP(err, pDataOwner->DataOwner().CommonSettingsL());
									if (err != KErrNone)
										{
									    OstTraceExt2(TRACE_NORMAL, DUP6_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "CommonSettingsL() - Error in sid: 0x%08x (%d)", sid.iId, static_cast<TInt32>(err));
										}//if		
									}//else
								}
							}//else
							
						} // if EHasPublicFiles & passiveBURSettings
						
					} //if
				entry.Close();
				} // if
				
			if (err != KErrNone)
				{
			    OstTraceExt1(TRACE_NORMAL, DUP7_CDATAOWNERMANAGER_FINDIMPORTPACKAGESL, "cannot get Package UID for reg file: %S", fileName);
				
				CSBGenericDataType* pId = CSBPackageId::NewL(sid, KDummyId, KNullDesC);
				CleanupStack::PushL(pId);
				TDriveList driveList;
				driveList.SetLength(driveList.MaxLength());
				driveList.FillZ();
				CDataOwnerInfo* pDataOwnerInfo = CDataOwnerInfo::NewL(pId, ENoOptions, ENoPassiveOptions, 
													  ENoActiveOptions, driveList);
				CleanupStack::PushL(pDataOwnerInfo);
				aDataOwners.AppendL(pDataOwnerInfo);
				CleanupStack::Pop(pDataOwnerInfo);
					
				CleanupStack::Pop(pId);
				} // if
				
				
			} // for x
			
			
		CleanupStack::PopAndDestroy(&entry);
		
		CleanupStack::PopAndDestroy(files);		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_FINDIMPORTPACKAGESL_EXIT );
		}
	
	void CDataOwnerManager::FindRegistrationFilesL(const TDesC& aPath, CDesCArray& aFiles)
	/** Searches the device for registration files

	@param aFiles on return a list of registration files on the device
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_FINDREGISTRATIONFILESL_ENTRY );		
		
		// Find private directorys
		TFindFile findDir(iFs);
		CDir* pDir = NULL;
		TInt errD = findDir.FindWildByDir(KStar, aPath, pDir);
		while (errD == KErrNone)
			{
			CleanupStack::PushL(pDir);
			
			// Loop through the directorys and look for registration files
			TUint dirCount = pDir->Count();
			while(dirCount--)
				{
				const TEntry& dirEntry = (*pDir)[dirCount];
				if (dirEntry.IsDir())
					{
					// Full path of dir
					TParse path;
					path.Set(dirEntry.iName, &findDir.File(), NULL);
					
					// See if there are any backup registration files
					TFindFile findFile(iFs);
					CDir* pFile = NULL;
					TFileName fullPath(path.FullName());
					fullPath.Append(KPathDelimiter);
					TInt errF = findFile.FindWildByPath(KBackupRegistrationFile, &fullPath, pFile);
					if (errF == KErrNone) // Where registration files found?
						{
						CleanupStack::PushL(pFile);
						// Add to the list of registration files
						TUint fileCount = pFile->Count();
						while(fileCount--)
							{
							path.Set((*pFile)[fileCount].iName, &findFile.File(), NULL);
							OstTraceExt1(TRACE_NORMAL, CDATAOWNERMANAGER_FINDREGISTRATIONFILESL, "found file: %S", path.FullName());
							aFiles.AppendL(path.FullName());
							} // for y
						
						// Cleanup
						CleanupStack::PopAndDestroy(pFile);
						} // if
					} // if
				} // for x
			
			// Cleanup
			CleanupStack::PopAndDestroy(pDir);
						
			// Check next drive
			errD = findDir.FindWild(pDir);
			} // while
		
		OstTrace1(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_FINDREGISTRATIONFILESL, "total files %d", aFiles.Count());
		OstTraceFunctionExit0( CDATAOWNERMANAGER_FINDREGISTRATIONFILESL_EXIT );
		}

	CDataOwnerContainer* CDataOwnerManager::FindL(TSecureId aSID)
	/** Finds a data owner in the array given a secure id

	@param aSID 			the Secure Id of the data owner you want
	@leave KErrNotFound		no such secure id
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_FINDL_ENTRY );
		CDataOwnerContainer* tempCont = CDataOwnerContainer::NewL(aSID, this);
		TInt res = iDataOwners.Find(tempCont, CDataOwnerContainer::Match);
		delete tempCont;
		
		if (res == KErrNotFound)
			{
			OstTraceFunctionExit0( CDATAOWNERMANAGER_FINDL_EXIT );
			return NULL;
			}
		else
			{
			OstTraceFunctionExit0( DUP1_CDATAOWNERMANAGER_FINDL_EXIT );
			return iDataOwners[res];
			}
		}
		

	CPackageDataTransfer* CDataOwnerManager::FindPackageDataContainerL(TUid aPid)
	/** Finds the package data container
	
	@param aPid The process id
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_FINDPACKAGEDATACONTAINERL_ENTRY );
		CPackageDataTransfer* pPackageTransfer = CPackageDataTransfer::NewL(aPid, this);
		CleanupStack::PushL(pPackageTransfer);
		TInt res = iPackageDataOwners.Find(pPackageTransfer, CPackageDataTransfer::Match);
		if (res == KErrNotFound)
			{
		    TInt err = iPackageDataOwners.InsertInOrder(pPackageTransfer, CPackageDataTransfer::Compare);
			LEAVEIFERROR(err, OstTrace1(TRACE_ERROR, CDATAOWNERMANAGER_FINDPACKAGEDATACONTAINERL, "Leave: %d", err));
			CleanupStack::Pop(pPackageTransfer);
			OstTraceFunctionExit0( CDATAOWNERMANAGER_FINDPACKAGEDATACONTAINERL_EXIT );
			return pPackageTransfer;
			}
		else
			{
			CleanupStack::PopAndDestroy(pPackageTransfer);
			OstTraceFunctionExit0( DUP1_CDATAOWNERMANAGER_FINDPACKAGEDATACONTAINERL_EXIT );
			return iPackageDataOwners[res];
			}
		}

	void CDataOwnerManager::StripSecureIdL(const TDesC& aStrip, TSecureId& aSecureId)
	/** Strips a Secure Id from a text string

	Looks for a "//private//" directory in the string and strips the SID after it.
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_STRIPSECUREIDL_ENTRY );
		
		TInt start = aStrip.FindF(KImportDir);
		if (start == KErrNotFound)
			{
			start = aStrip.FindF(KPrivate);
			if (start == KErrNotFound)
				{
			    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_STRIPSECUREIDL, "Leave: KErrArgument");
				User::Leave(KErrArgument);
				}
			start += KPrivate().Length();
			}
		else
			{
			start += KImportDir().Length();
			}
			
		// Find the end of the SID
		TInt end = (aStrip.Right(aStrip.Length() - start)).FindF(KBackSlash);
		end += start;
		
		// Create the secure Id
		TLex sIdLex(aStrip.Mid(start, end - start + 1));
		// If we cant do the convert then ignore as it is a directory that is not a SID.
		if (sIdLex.Val(aSecureId.iId, EHex) != KErrNone)
			{
		    OstTrace0(TRACE_ERROR, DUP1_CDATAOWNERMANAGER_STRIPSECUREIDL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		OstTraceFunctionExit0( CDATAOWNERMANAGER_STRIPSECUREIDL_EXIT );
		}

	
	void CDataOwnerManager::FindDataOwnersL()
	/** Gets a list of data owners

	This list only contains the Secure ID's for applications that have a backup 
	registration files.
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_FINDDATAOWNERSL_ENTRY );		
		// Clear out any current list
		iDataOwners.ResetAndDestroy();
		
		// Find all registration files on the device
		CDesCArray* registrationFiles = new(ELeave) CDesCArrayFlat(KDesCArrayGranularity);
		CleanupStack::PushL(registrationFiles);
		FindRegistrationFilesL(KPrivate, *registrationFiles);
		OstTrace0(TRACE_NORMAL, CDATAOWNERMANAGER_FINDDATAOWNERSL, " ");
		
		// Add registration files to iDataOwners
		const TInt count = registrationFiles->Count();
		for (TInt x = 0; x < count; x++)
			{
			// Strip the SID
			TSecureId sid;
			const TDesC& fileName = (*registrationFiles)[x];
			TRAPD(err, StripSecureIdL(fileName, sid));
			if (err == KErrNone) // If there was an error then ignore it as it is probally not a dir
				{
				CDataOwnerContainer* pDataOwner = FindL(sid);
				if (pDataOwner == NULL)  // If it does not exist we need to create it
					{
				    OstTraceExt2(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_FINDDATAOWNERSL, "found reg file: %S for sid: 0x%08x", fileName, sid.iId);

					pDataOwner = CDataOwnerContainer::NewL(sid, this);
					CleanupStack::PushL(pDataOwner);
					
					iDataOwners.InsertInOrder(pDataOwner, CDataOwnerContainer::Compare);
					CleanupStack::Pop(pDataOwner);										
					} // if
				else
					{
				    OstTraceExt2(TRACE_NORMAL, DUP2_CDATAOWNERMANAGER_FINDDATAOWNERSL, "found reg file: %S for existing sid: 0x%08x", fileName, sid.iId);
					}

				pDataOwner->DataOwner().AddRegistrationFilesL(fileName);
				} // if
			} // for x
		
		CleanupStack::PopAndDestroy(registrationFiles);		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_FINDDATAOWNERSL_EXIT );
		}
		
	CSBEConfig& CDataOwnerManager::Config()
	/** Getter for the Secure Backup configuration
	@return Reference to the CSBEConfig instance
	*/
		{
		return *iConfig;
		}
		
	TBool CDataOwnerManager::IsSetForPartialL(TSecureId aSecureId) const
	/** Queries whether a SID is included in a partial operation
	@return TBool specifying whether a SID is included or not
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERMANAGER_ISSETFORPARTIALL_ENTRY );
		TBool found = EFalse;
		
		if (iSIDListForPartial != NULL)
			{
			TInt result = iSIDListForPartial->Find(aSecureId);
			if (result != KErrNotFound)
				{
				found = ETrue;
				}
			}
		else
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERMANAGER_ISSETFORPARTIALL, "SID list not created yet so leaving!");
			User::Leave(KErrNotFound);
			}
		
		OstTraceExt2(TRACE_NORMAL, DUP1_CDATAOWNERMANAGER_ISSETFORPARTIALL, "SID: 0x%08x, found: %d", aSecureId.iId, static_cast<TInt32>(found));	
		
		OstTraceFunctionExit0( CDATAOWNERMANAGER_ISSETFORPARTIALL_EXIT );
		return found;
		}
	} // namespace conn
