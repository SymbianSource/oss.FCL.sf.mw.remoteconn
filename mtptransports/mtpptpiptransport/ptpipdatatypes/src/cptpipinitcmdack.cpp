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

#include "cptpipinitcmdack.h"
#include "ptpipdatatypes.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipinitcmdackTraces.h"
#endif
   
// Dataset constants
const TUint CPTPIPInitCmdAck::KFlatChunkSize(28);  
const CMTPTypeCompoundBase::TElementInfo CPTPIPInitCmdAck::iElementMetaData[CPTPIPInitCmdAck::ENumElements] = 
    {
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,	0,					KMTPTypeUINT32Size}},   // ELength
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,	4,					KMTPTypeUINT32Size}},   // EType
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,	8,					KMTPTypeUINT32Size}},   // EConNumber
		{EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT128,	12,					KMTPTypeUINT128Size}},   // GUID
	//	{EIdNameChunk,      EMTPTypeString,     {EMTPTypeString,    KMTPNotApplicable,	KMTPNotApplicable}},   // friendly name
        {EIdVersionChunk,	EMTPTypeUINT32,     {EMTPTypeUINT32,    KMTPNotApplicable,	KMTPNotApplicable}}   // version
        
    };

EXPORT_C  CPTPIPInitCmdAck* CPTPIPInitCmdAck::NewL()
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDACK_NEWL_ENTRY );
    CPTPIPInitCmdAck* self = new (ELeave) CPTPIPInitCmdAck(); 
    CleanupStack::PushL(self); 
    self->ConstructL();   
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CPTPIPINITCMDACK_NEWL_EXIT );
    return self; 
    }
/**
Constructor.
*/
EXPORT_C CPTPIPInitCmdAck::CPTPIPInitCmdAck() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iChunkHeader(KFlatChunkSize, *this),
    iElementInfo(iElementMetaData, ENumElements)
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDACK_CPTPIPINITCMDACK_ENTRY );
    
    OstTraceFunctionExit0( CPTPIPINITCMDACK_CPTPIPINITCMDACK_EXIT );
    }

/**
Destructor.
*/
 EXPORT_C CPTPIPInitCmdAck::~CPTPIPInitCmdAck()
    {
    OstTraceFunctionEntry0( DUP1_CPTPIPINITCMDACK_CPTPIPINITCMDACK_ENTRY );
    iChunkHeader.Close();
    iBuffer.Close();
    OstTraceFunctionExit0( DUP1_CPTPIPINITCMDACK_CPTPIPINITCMDACK_EXIT );
    }
    /**
Second phase constructor.
*/   
 void CPTPIPInitCmdAck::ConstructL()
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDACK_CONSTRUCTL_ENTRY );
    iChunkHeader.OpenL();
    ChunkAppendL(iChunkHeader);
    ChunkAppendL(iVersion);
   iChunkCount = EIdNumChunks;    
    OstTraceFunctionExit0( CPTPIPINITCMDACK_CONSTRUCTL_EXIT );
    }
    
    
 EXPORT_C TUint CPTPIPInitCmdAck::Type() const
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDACK_TYPE_ENTRY );
    OstTraceFunctionExit0( CPTPIPINITCMDACK_TYPE_EXIT );
    return EPTPIPTypeInitCmdAck;
    } 
const CMTPTypeCompoundBase::TElementInfo& CPTPIPInitCmdAck::ElementInfo(TInt aElementId) const
    {
    OstTraceFunctionEntry0( CPTPIPINITCMDACK_ELEMENTINFO_ENTRY );
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    OstTraceFunctionExit0( CPTPIPINITCMDACK_ELEMENTINFO_EXIT );
    return iElementInfo[aElementId];
    }
    
 EXPORT_C TInt CPTPIPInitCmdAck::FirstReadChunk(TPtrC8& aChunk) const
{
	OstTraceFunctionEntry0( CPTPIPINITCMDACK_FIRSTREADCHUNK_ENTRY );
	iChunkCount = EIdFlatChunk;
	OstTraceFunctionExit0( CPTPIPINITCMDACK_FIRSTREADCHUNK_EXIT );
	return CMTPTypeCompoundBase::FirstReadChunk(aChunk);
	
}   		
EXPORT_C TInt CPTPIPInitCmdAck::NextReadChunk(TPtrC8& aChunk) const
{
	OstTraceFunctionEntry0( CPTPIPINITCMDACK_NEXTREADCHUNK_ENTRY );
	TInt ret = KErrNone;
	if(iChunkCount == EIdFlatChunk)
		{	
		aChunk.Set((const TUint8*)&iBuffer[0],iBuffer.Size());		
		}
	else
		{
		ret = CMTPTypeCompoundBase::NextReadChunk(aChunk);	
		}
	iChunkCount++;
	OstTraceFunctionExit0( CPTPIPINITCMDACK_NEXTREADCHUNK_EXIT );
	return ret;
}



EXPORT_C  void CPTPIPInitCmdAck::SetDeviceFriendlyName(TDesC16& aName)
{
	OstTraceFunctionEntry0( CPTPIPINITCMDACK_SETDEVICEFRIENDLYNAME_ENTRY );
	if(KErrNone == iBuffer.Create(aName,aName.Length()+KMTPNullCharLen))
		{
		iBuffer.Append(KMTPNullChar);
		}
}    

EXPORT_C TUint64 CPTPIPInitCmdAck::Size() const
{
	OstTraceFunctionEntry0( CPTPIPINITCMDACK_SIZE_ENTRY );
	TUint64 size = CMTPTypeCompoundBase::Size();
	size += iBuffer.Size();
	OstTraceFunctionExit0( CPTPIPINITCMDACK_SIZE_EXIT );
	return size;
}

