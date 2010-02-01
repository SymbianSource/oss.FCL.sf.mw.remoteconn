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
	CPTPIPDataContainer* self = new (ELeave) CPTPIPDataContainer();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
 Destructor.
 */
EXPORT_C CPTPIPDataContainer::~CPTPIPDataContainer()
	{
	iChunkHeader.Close();
	}

/**
 Constructor.
 */
CPTPIPDataContainer::CPTPIPDataContainer( ) :
	CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), iChunkHeader(
			KFlatChunkSize, *this),
			iElementInfo(iElementMetaData, ENumElements),iIsNextHeader(EFalse)
	{
	
	}

/**
 Second phase constructor.
 */
void CPTPIPDataContainer::ConstructL( )
	{
	iChunkHeader.OpenL ( );
	ChunkAppendL (iChunkHeader );
	}

/**
 Provides the bulk container payload.
 @return The bulk container payload.
 */
EXPORT_C MMTPType* CPTPIPDataContainer::Payload() const
	{
	return iPayload;
	}

/**
 Sets the bulk container payload.
 @param aPayload The new bulk container payload.
 */
EXPORT_C void CPTPIPDataContainer::SetPayloadL(MMTPType* aPayload)
	{
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
	}

EXPORT_C TUint CPTPIPDataContainer::Type() const
	{
	return EPTPIPTypeDataContainer;
	}

const CMTPTypeCompoundBase::TElementInfo& CPTPIPDataContainer::ElementInfo(
		TInt aElementId ) const
	{
	__ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
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
	        
	        return err;
	        }

/**
  Over-ridden implementation of CommitChunkL() derived from CMTPTypeCompoundBase.
  This increments the number of chunks read only after, the first header is read
  @param aChunk The data that is currently given by Initiator
 **/
EXPORT_C MMTPType* CPTPIPDataContainer::CommitChunkL(TPtr8& aChunk)
	    {       
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

	    return res;
	    }
 
