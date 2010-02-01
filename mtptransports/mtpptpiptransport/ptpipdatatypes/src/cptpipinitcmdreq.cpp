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
 @internalComponent
*/


#include "cptpipinitcmdreq.h"
#include "ptpipdatatypes.h"    




// Dataset constants
const TUint CPTPIPInitCmdRequest::KFlatChunkSize(24);  
const CMTPTypeCompoundBase::TElementInfo CPTPIPInitCmdRequest::iElementMetaData[CPTPIPInitCmdRequest::ENumElements] = 
    {
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,	0,					KMTPTypeUINT32Size}},   // ELength
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,	4,					KMTPTypeUINT32Size}},   // EType
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT128,	8,					KMTPTypeUINT128Size}},   // GUID
	//	{EIdNameChunk,      EMTPTypeUndefined,     {EMTPTypeUndefined,    KMTPNotApplicable,	KMTPNotApplicable}},   // friendly name
        {EIdVersionChunk,	EMTPTypeUINT32,     {EMTPTypeUINT32,    KMTPNotApplicable,	KMTPNotApplicable}}   // version
        
    };

EXPORT_C  CPTPIPInitCmdRequest* CPTPIPInitCmdRequest::NewL()
    {
    CPTPIPInitCmdRequest* self = new (ELeave) CPTPIPInitCmdRequest(); 
    CleanupStack::PushL(self); 
    self->ConstructL();   
    CleanupStack::Pop(self);
    return self; 
    }
/**
Constructor.
*/
EXPORT_C CPTPIPInitCmdRequest::CPTPIPInitCmdRequest() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iChunkHeader(KFlatChunkSize, *this),
    iElementInfo(iElementMetaData, ENumElements),iBuffer()
    {
    
    }

/**
Destructor.
*/
 EXPORT_C CPTPIPInitCmdRequest::~CPTPIPInitCmdRequest()
    {
    iChunkHeader.Close();
    iBuffer.Close();
    }
    /**
Second phase constructor.
*/   
 void CPTPIPInitCmdRequest::ConstructL()
    {
    iChunkHeader.OpenL();
    ChunkAppendL(iChunkHeader);
    ChunkAppendL(iVersion);
   iChunkCount = EIdNumChunks;
    
    }
    
    
 EXPORT_C TUint CPTPIPInitCmdRequest::Type() const
    {
    return EPTPIPTypeInitCmdRequest;
    } 
const CMTPTypeCompoundBase::TElementInfo& CPTPIPInitCmdRequest::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
EXPORT_C TInt CPTPIPInitCmdRequest::FirstWriteChunk(TPtr8& aChunk)
{
	iChunkCount = EIdFlatChunk;
	return CMTPTypeCompoundBase::FirstWriteChunk(aChunk);
	
}
EXPORT_C TInt CPTPIPInitCmdRequest::NextWriteChunk(TPtr8& aChunk)
{
	TInt ret = KErrNone;
	if(iChunkCount == EIdFlatChunk)
		{	
		
		TUint32 size(Uint32L(CPTPIPInitCmdRequest::ELength));
		size-=28;
		size/=2;
		TRAP_IGNORE(iBuffer.CreateMaxL(size));
		
		size = iBuffer.Size();
		aChunk.Set((TUint8*)&iBuffer[0],iBuffer.Size(),iBuffer.Size());
		
		}
	else
		{
		ret = CMTPTypeCompoundBase::NextWriteChunk(aChunk);	
		}
	iChunkCount++;
	return ret;
}

EXPORT_C MMTPType* CPTPIPInitCmdRequest::CommitChunkL(TPtr8& aChunk)
{

	if(iChunkCount != EIdVersionChunk)
		{
		return CMTPTypeCompoundBase::CommitChunkL(aChunk);
			
		}
	return NULL;	
}

EXPORT_C TDes16& CPTPIPInitCmdRequest::HostFriendlyName()
{
return iBuffer;
}

