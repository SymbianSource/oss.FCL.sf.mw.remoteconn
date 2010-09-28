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

#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mtptypessimple.h>

#include "cmtpobjectstore.h"
#include "cmtpreferencemgr.h"
#include "dbutility.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpreferencemgrTraces.h"
#endif



_LIT(KSQLReferenceTableName, "ReferenceStore");
_LIT(KSQLReferenceIndexName, "ReferenceIndex");
_LIT(KSQLCreateReferenceTableText,"CREATE TABLE ReferenceStore (HandleId UNSIGNED INTEGER, References LONG VARBINARY)");
_LIT(KSQLCreateReferenceIndexText,"CREATE UNIQUE INDEX ReferenceIndex on ReferenceStore (HandleId)");	


static const TInt KMTPReferenceGranularity = 5;
// Maximum value to use for granularity 
// This will result in 4KB of memory used (1024 * 4bytes) for reference SUID array
static const TInt KMTPMaxGranularity = 1024;

/**
Two phase construction
@param aObjectMgr	Reference to the object manager
@return a pointer to the reference manager instance
*/
CMTPReferenceMgr* CMTPReferenceMgr::NewL(CMTPObjectStore& aObjectStore)
	{
	CMTPReferenceMgr* self = new (ELeave) CMTPReferenceMgr(aObjectStore);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
CMTPReferenceMgr::~CMTPReferenceMgr()
	{
	iBatched.Close();
	}

/**
Provides an MTP array of the target object handles which are referenced 
by the specified source object handle. A pointer to the MTP array is 
placed on the cleanup stack.
@param aFromHandle The MTP object handle of the source object from which 
the references originate.
@return The MTP reference target object handle array. Ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
CMTPTypeArray* CMTPReferenceMgr::ReferencesLC(const TMTPTypeUint32& aFromHandle) const
	{
	return GetReferencesLC(aFromHandle.Value());
	}

/**
Provides an SUID array of the target object SUIDs which are referenced by 
the specified source object SUID. A pointer to the SUID array is 
placed on the cleanup stack.
@param aFromSuid The SUID of the source object from which the references 
originate.
@return The reference target object SUID array. Ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
CDesCArray* CMTPReferenceMgr::ReferencesLC(const TDesC& aParentSuid) const
	{
	TUint32 handle =  iObjectStore.HandleL(aParentSuid);
	return GetReferencesInDesLC(handle);
	}

/**
Removes all object reference links in which the specified SUID represents 
either the source or target reference object.
@param aSuid The object SUID.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::RemoveReferencesL(const TDesC& aSuid)
	{
	TUint32 handle =  iObjectStore.HandleL(aSuid);
	RemoveReferencesL(handle);
	}

/**
Creates an abstract reference linkage between the specified source and 
target object SUIDs.
@param aFromSuid The SUID of the source object from which the reference 
originates.
param aToSuid The SUID of the target object to which the reference is 
made.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::SetReferenceL(const TDesC& aFromSuid, const TDesC& aToSuid)
	{
	TUint32 fromHandle =  iObjectStore.HandleL(aFromSuid);
	TUint32 toHandle =  iObjectStore.HandleL(aToSuid);
	SetReferenceL(fromHandle, toHandle);
	}

/**
Replaces the abstract reference links originating from the specified 
source object handle with the specified set of target object SUIDs.
@param aFromHandle The SUID of the source object from which the references 
originate.
@param aToSuids The reference target MTP object SUID array.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::SetReferencesL(const TDesC& aParentSuid, const CDesCArray& aToSuids)
	{
	TInt count = aToSuids.Count();
	
	if (count > 0)
		{
		TUint32 fromHandle =  iObjectStore.HandleL(aParentSuid);
	
		// Limit granularity to something less or equal to KMTPMaxGranularity items
		RArray<TUint> toHandles(Min(count, KMTPMaxGranularity));
		CleanupClosePushL(toHandles);
		
		for(TInt i = 0; i < count; i++)
			{
			TUint32 toHandle =  iObjectStore.HandleL(aToSuids[i]);
			toHandles.AppendL(toHandle);
			}	
		
		SetReferencesL(fromHandle, toHandles);	
		CleanupStack::PopAndDestroy(&toHandles);
		}
	}


/**
Constructor/.
*/    
CMTPReferenceMgr::CMTPReferenceMgr(CMTPObjectStore& aObjectStore):
	iObjectStore(aObjectStore)
	{
	
	}

/**
second-phase construction
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::ConstructL()
	{
    iDatabase   = &iObjectStore.Database();

	if(!DBUtility::IsTableExistsL(*iDatabase, KSQLReferenceTableName))
		{
		CreateTableL();
		}
	if(!DBUtility::IsIndexExistsL(*iDatabase, KSQLReferenceTableName, KSQLReferenceIndexName))
		{
		CreateIndexL();
		}
	User::LeaveIfError(iBatched.Open(*iDatabase, KSQLReferenceTableName, RDbRowSet::EUpdatable));
	iBatched.SetIndex( KSQLReferenceIndexName ); 
	}

/**
Create the reference table in the database
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::CreateTableL()
	{
	LEAVEIFERROR(iDatabase->Execute(KSQLCreateReferenceTableText),
	        OstTrace0( TRACE_ERROR, CMTPREFERENCEMGR_CREATETABLEL, "TABLE ReferenceStore create failed!" ));    
	}

/**
Create the index in the reference table
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::CreateIndexL()
	{
	LEAVEIFERROR(iDatabase->Execute(KSQLCreateReferenceIndexText),
	        OstTrace0( TRACE_ERROR, CMTPREFERENCEMGR_CREATEINDEXL, "INDEX ReferenceIndex on ReferenceStore create failed!" ));
	}

/**
Get reference on the object identified by aIdentifier
@param aIdentifier	The 64bit internal object identifier
@return The array containing the handles of the references
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPTypeArray* CMTPReferenceMgr::GetReferencesLC(TUint aHandle) const
	{
	RArray<TUint> toHandles;
	CleanupClosePushL(toHandles);
	GetReferencesL(aHandle, toHandles);
	
	CMTPTypeArray* mtpReferenceArray = CMTPTypeArray::NewLC(EMTPTypeAUINT32);	
	mtpReferenceArray->AppendL(toHandles);
	CleanupStack::Pop(mtpReferenceArray);
	CleanupStack::PopAndDestroy(&toHandles);
	CleanupStack::PushL(mtpReferenceArray);
	return mtpReferenceArray;
	}

/**
Get reference on the object identified by aIdentifier
@param aIdentifier	The 64bit internal object identifier
@return The array containing the suids of the references
@leave One of the system wide error codes, if a processing failure occurs.
*/	
CDesCArray*	 CMTPReferenceMgr::GetReferencesInDesLC(TUint aHandle) const
	{
	RArray<TUint> toHandles(KMTPReferenceGranularity);
	CleanupClosePushL(toHandles);
	GetReferencesL(aHandle, toHandles);	
	const TInt count = toHandles.Count();
	CDesCArray* mtpReferenceArray = new (ELeave) CDesCArrayFlat((count==0) ? 1 : count);
	CleanupStack::PushL(mtpReferenceArray);
	
	for(TInt i = 0; i < count; i++)
		{
		const TDesC& suid = iObjectStore.ObjectSuidL(toHandles[i]);
		if(suid.Length())
			{
			mtpReferenceArray->AppendL(suid);
			}
		}
	CleanupStack::Pop(mtpReferenceArray);
	CleanupStack::PopAndDestroy(&toHandles);
	CleanupStack::PushL(mtpReferenceArray);
	return mtpReferenceArray;		
	
	}

/**
Get references on the object identified by aIdentifier
@param aIdentifier	The 64bit internal object identifier
@param aReferences	The reference array, on return, containing the 64bit internal identifiers of the references
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::GetReferencesL(TUint aHandle, RArray<TUint>& aToHandles) const
	{
	TUint32 temp = 0;
	TBool   needToUpdate = EFalse;
	aToHandles.Reset();
	if(iBatched.SeekL(aHandle))
		{
		iBatched.GetL();
	    RDbColReadStream readStream;     
        readStream.OpenLC(iBatched,2);
        TInt count = readStream.ReadInt32L();
        while(count--)
        	{
        	temp = readStream.ReadUint32L();
			if(iObjectStore.ObjectExistsL(temp))
	        	aToHandles.AppendL(temp);
			else
				needToUpdate = ETrue;
        	}
        CleanupStack::PopAndDestroy(&readStream);

		
		if(needToUpdate)
			{//Something has been deleted. Write it back to Reference list
			iBatched.UpdateL();
			RDbColWriteStream writeStream;	   
			writeStream.OpenLC(iBatched,2);
			
			count = aToHandles.Count();
			writeStream.WriteInt32L(count);
			for(TInt i = 0; i < count; i++)
				{
				writeStream.WriteUint32L(aToHandles[i]);
				}
			writeStream.CommitL(); 
			CleanupStack::PopAndDestroy(&writeStream);
			iBatched.PutL();			
			}
		
		}	
	return;

	}


/**
Remove reference on the object identified by aIdentifier
@param aIdentifier	The 64bit internal object identifier
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::RemoveReferencesL(TUint aHandle)
	{
	if(iBatched.SeekL(aHandle))
		{
		iBatched.DeleteL();
		IncTranOpsNumL();
		}
	}
/**
Set reference on the object identified by aFromIdentifier
@param aFromIdentifier	The 64bit internal object identifier of the source object
@param aToIdentifier	The 64bit internal object identifier of the referenced object
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::SetReferenceL(TUint aFromHandle, TUint aToHandle)
	{
	RArray<TUint> aToHandleArray(1);
	CleanupClosePushL(aToHandleArray);
	aToHandleArray.AppendL(aToHandle);
	SetReferencesL(aFromHandle, aToHandleArray);
	CleanupStack::PopAndDestroy(&aToHandleArray);
	}

/**
Replaces the abstract reference links originating from the specified 
source object handle with the specified set of target object handles.
@param aFromHandle The handle of the source object from which the references 
originate.
@param aToHandles The reference target object handle array.
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMTPReferenceMgr::SetReferencesL(const TMTPTypeUint32& aFromHandle, const CMTPTypeArray& aToHandles)
{
	RArray<TUint> tempArray;
	CleanupClosePushL(tempArray);
	TInt count = aToHandles.NumElements();
	for(TInt i = 0; i < count; i++)
		{
		tempArray.AppendL(aToHandles.ElementUint(i));
		}	
	SetReferencesL(aFromHandle.Value(), tempArray);
	CleanupStack::PopAndDestroy(&tempArray);
}



/**
Set references on the object identified by aFromIdentifier
@param aFromIdentifier	The 64bit internal object identifier of the source object
@param aToIdentifiers	The 64bit internal object identifiers of the referenced objects
@leave One of the system wide error codes, if a processing failure occurs.
*/   	
void CMTPReferenceMgr::SetReferencesL(TUint aHandle, const RArray<TUint>& aToHandles)
	{
	if(iBatched.SeekL(aHandle))
		{
		iBatched.UpdateL();		
		}
	else
		{
		iBatched.InsertL();		
		}
	
	iBatched.SetColL(1, aHandle);
	
	RDbColWriteStream writeStream;     
    writeStream.OpenLC(iBatched,2);
    
    const TInt count = aToHandles.Count();
    writeStream.WriteInt32L(count);
    for(TInt i = 0; i < count; i++)
        {
        writeStream.WriteUint32L(aToHandles[i]);
        }
    writeStream.CommitL(); 
    CleanupStack::PopAndDestroy(&writeStream);
	iBatched.PutL();			
	IncTranOpsNumL();
	}


void CMTPReferenceMgr::IncTranOpsNumL()
	{
	iObjectStore.IncTranOpsNumL();
	}


/**
Verify if every reference is pointing to a valid object. If the referenced object does not exist,
delete the object from the reference array.
@param aReference	The 64bit internal object identifiers of the referenced objects
@leave One of the system wide error codes, if a processing failure occurs.
*/
/*
void CMTPReferenceMgr::AdjustReferencesL(RArray<TUint32>&aHandles) const
	{
	TInt count = aHandles.Count();
	while(count--)
		{
		if(!iObjectStore.ObjectExistsL(aHandles[count]))
			{
			aHandles.Remove(count);
			}
		}
	}
	*/

