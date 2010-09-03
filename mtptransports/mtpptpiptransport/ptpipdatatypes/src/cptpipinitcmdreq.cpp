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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipinitcmdreqTraces.h"
#endif
    




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
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_NEWL_ENTRY );
    CPTPIPInitCmdRequest* self = new (ELeave) CPTPIPInitCmdRequest(); 
    CleanupStack::PushL(self); 
    self->ConstructL();   
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_NEWL_EXIT );
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
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_CPTPIPINITCMDREQUEST_ENTRY );
    
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_CPTPIPINITCMDREQUEST_EXIT );
    }

/**
Destructor.
*/
 EXPORT_C CPTPIPInitCmdRequest::~CPTPIPInitCmdRequest()
    {
    OstTraceFunctionEntry0( DUP1_CPTPIPINITCMDREQUEST_CPTPIPINITCMDREQUEST_ENTRY );
    iChunkHeader.Close();
    iBuffer.Close();
    OstTraceFunctionExit0( DUP1_CPTPIPINITCMDREQUEST_CPTPIPINITCMDREQUEST_EXIT );
    }
    /**
Second phase constructor.
*/   
 void CPTPIPInitCmdRequest::ConstructL()
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_CONSTRUCTL_ENTRY );
    iChunkHeader.OpenL();
    ChunkAppendL(iChunkHeader);
    ChunkAppendL(iVersion);
   iChunkCount = EIdNumChunks;
    
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_CONSTRUCTL_EXIT );
    }
    
    
 EXPORT_C TUint CPTPIPInitCmdRequest::Type() const
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_TYPE_ENTRY );
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_TYPE_EXIT );
    return EPTPIPTypeInitCmdRequest;
    } 
const CMTPTypeCompoundBase::TElementInfo& CPTPIPInitCmdRequest::ElementInfo(TInt aElementId) const
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_ELEMENTINFO_ENTRY );
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_ELEMENTINFO_EXIT );
    return iElementInfo[aElementId];
    }
EXPORT_C TInt CPTPIPInitCmdRequest::FirstWriteChunk(TPtr8& aChunk)
{
	OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_FIRSTWRITECHUNK_ENTRY );
	iChunkCount = EIdFlatChunk;
    OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_FIRSTWRITECHUNK_EXIT );
	return CMTPTypeCompoundBase::FirstWriteChunk(aChunk);
	
}
EXPORT_C TInt CPTPIPInitCmdRequest::NextWriteChunk(TPtr8& aChunk)
{
	OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_NEXTWRITECHUNK_ENTRY );
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
	OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_NEXTWRITECHUNK_EXIT );
	return ret;
}

EXPORT_C MMTPType* CPTPIPInitCmdRequest::CommitChunkL(TPtr8& aChunk)
{
    OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_COMMITCHUNKL_ENTRY );

	if(iChunkCount != EIdVersionChunk)
		{
		return CMTPTypeCompoundBase::CommitChunkL(aChunk);
			
		}
	OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_COMMITCHUNKL_EXIT );
	return NULL;	
}

EXPORT_C TDes16& CPTPIPInitCmdRequest::HostFriendlyName()
{
OstTraceFunctionEntry0( CPTPIPINITCMDREQUEST_HOSTFRIENDLYNAME_ENTRY );
OstTraceFunctionExit0( CPTPIPINITCMDREQUEST_HOSTFRIENDLYNAME_EXIT );
return iBuffer;
}

