// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include "ptpipdatatypes.h"
#include "cptpipdatacontainer.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipdatacontainerTraces.h"
#endif


// Dataset constants
const TUint CPTPIPDataContainer::KFlatChunkSize(12);

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CPTPIPDataContainer::iElementMetaData[CPTPIPDataContainer::ENumElements] = 
    {
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EContainerLength
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    4,                  KMTPTypeUINT32Size}},   // EContainerType
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    8,                  KMTPTypeUINT32Size}},   // ETransactionId
		{EIdPayloadChunk,   EMTPTypeUndefined,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}}     // EPayload
    };

/**
 Generic Container's factory method. 
 This is used to create an empty MTP PTPIP data container dataset type. 
 @return  Ownership IS transfered.
 @leave One of the system wide error codes, if unsuccessful.
 */
EXPORT_C CPTPIPDataContainer* CPTPIPDataContainer::NewL()
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_NEWL_ENTRY );
	CPTPIPDataContainer* self = new (ELeave) CPTPIPDataContainer();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_NEWL_EXIT );
	return self;
	}

/**
 Destructor.
 */
EXPORT_C CPTPIPDataContainer::~CPTPIPDataContainer()
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_CPTPIPDATACONTAINER_ENTRY );
	iChunkHeader.Close();
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_CPTPIPDATACONTAINER_EXIT );
	}

/**
 Constructor.
 */
CPTPIPDataContainer::CPTPIPDataContainer( ) :
	CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), iChunkHeader(
			KFlatChunkSize, *this),
			iElementInfo(iElementMetaData, ENumElements),iIsNextHeader(EFalse)
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPDATACONTAINER_CPTPIPDATACONTAINER_ENTRY );
	
	OstTraceFunctionExit0( DUP1_CPTPIPDATACONTAINER_CPTPIPDATACONTAINER_EXIT );
	}

/**
 Second phase constructor.
 */
void CPTPIPDataContainer::ConstructL( )
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_CONSTRUCTL_ENTRY );
	iChunkHeader.OpenL ( );
	ChunkAppendL (iChunkHeader );
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_CONSTRUCTL_EXIT );
	}

/**
 Provides the bulk container payload.
 @return The bulk container payload.
 */
EXPORT_C MMTPType* CPTPIPDataContainer::Payload() const
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_PAYLOAD_ENTRY );
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_PAYLOAD_EXIT );
	return iPayload;
	}

/**
 Sets the bulk container payload.
 @param aPayload The new bulk container payload.
 */
EXPORT_C void CPTPIPDataContainer::SetPayloadL(MMTPType* aPayload)
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_SETPAYLOADL_ENTRY );
	if (iPayload)
		{
		// Remove the existing payload from the super class.
		ChunkRemove(iElementMetaData[EPayload].iChunkId);
		}

	if (aPayload)
		{
		// Pass the payload to the super class for management.
		ChunkAppendL(*aPayload);
		}
	iPayload = aPayload;
	iIsNextHeader = EFalse; 
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_SETPAYLOADL_EXIT );
	}

EXPORT_C TUint CPTPIPDataContainer::Type() const
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_TYPE_ENTRY );
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_TYPE_EXIT );
	return EPTPIPTypeDataContainer;
	}

const CMTPTypeCompoundBase::TElementInfo& CPTPIPDataContainer::ElementInfo(
		TInt aElementId ) const
	{
	OstTraceFunctionEntry0( CPTPIPDATACONTAINER_ELEMENTINFO_ENTRY );
	__ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
	OstTraceFunctionExit0( CPTPIPDATACONTAINER_ELEMENTINFO_EXIT );
	return iElementInfo[aElementId];
	}
	
/**
  Over-ridden implementation of FirstWriteChunk() derived from CMTPTypeCompoundBase.
  This sets the Write Sequence-State to EInProgressNext, forcing the headers, from the second 
  PTP/IP packet onwards,to be ignored
  @param aChunk The data that is currently given by Initiator
 **/
EXPORT_C TInt CPTPIPDataContainer::FirstWriteChunk(TPtr8& aChunk)
	        {
	        OstTraceFunctionEntry0( CPTPIPDATACONTAINER_FIRSTWRITECHUNK_ENTRY );
	        TInt err(KErrNone);	        
	        
	        aChunk.Set(NULL, 0, 0);
	        
	        
	        if (iChunks.Count() == 0)
	            {
	            err = KErrNotFound;            
	            }
	        else
	            {
	            iWriteChunk = 0;
	            
	            TInt iWriteErr = iChunks[iWriteChunk]->FirstWriteChunk(aChunk);
	            
	            if ((iIsNextHeader) && (iWriteErr == KMTPChunkSequenceCompletion))
	            	iWriteErr = KErrNone;
	            
		        switch (iWriteErr)
		            {
		        case KMTPChunkSequenceCompletion:
		            if ((iWriteChunk + 1) < iChunks.Count()) 
		                {
		                iWriteSequenceState = EInProgressFirst;
		                err = KErrNone;
		                }
		            else
		                {
		                iWriteSequenceState = EIdle;                 
		                }
		            break;
		            
		        case KErrNone:
		            iWriteSequenceState = EInProgressNext;
		            break;
		            
		        default:
		            break;                      
	           } 
	            } 
	        
	        iIsNextHeader = ETrue;
	        
	        OstTraceFunctionExit0( CPTPIPDATACONTAINER_FIRSTWRITECHUNK_EXIT );
	        return err;
	        }

/**
  Over-ridden implementation of CommitChunkL() derived from CMTPTypeCompoundBase.
  This increments the number of chunks read only after, the first header is read
  @param aChunk The data that is currently given by Initiator
 **/
EXPORT_C MMTPType* CPTPIPDataContainer::CommitChunkL(TPtr8& aChunk)
	    {       
	    OstTraceFunctionEntry0( CPTPIPDATACONTAINER_COMMITCHUNKL_ENTRY );
	    MMTPType *chunk(iChunks[iWriteChunk]);
	    MMTPType* res = NULL;
	    if (chunk->CommitRequired())
	        {
	        res = chunk->CommitChunkL(aChunk);
	        }
	        
	    
	    if (iWriteChunk == 0)
	        {
	        iWriteChunk++;            
	        }

	    OstTraceFunctionExit0( CPTPIPDATACONTAINER_COMMITCHUNKL_EXIT );
	    return res;
	    }
 
