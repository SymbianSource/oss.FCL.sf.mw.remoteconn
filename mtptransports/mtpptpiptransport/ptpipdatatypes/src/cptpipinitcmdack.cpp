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
    CPTPIPInitCmdAck* self = new (ELeave) CPTPIPInitCmdAck(); 
    CleanupStack::PushL(self); 
    self->ConstructL();   
    CleanupStack::Pop(self);
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
    
    }

/**
Destructor.
*/
 EXPORT_C CPTPIPInitCmdAck::~CPTPIPInitCmdAck()
    {
    iChunkHeader.Close();
    iBuffer.Close();
    }
    /**
Second phase constructor.
*/   
 void CPTPIPInitCmdAck::ConstructL()
    {
    iChunkHeader.OpenL();
    ChunkAppendL(iChunkHeader);
    ChunkAppendL(iVersion);
   iChunkCount = EIdNumChunks;    
    }
    
    
 EXPORT_C TUint CPTPIPInitCmdAck::Type() const
    {
    return EPTPIPTypeInitCmdAck;
    } 
const CMTPTypeCompoundBase::TElementInfo& CPTPIPInitCmdAck::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
    
 EXPORT_C TInt CPTPIPInitCmdAck::FirstReadChunk(TPtrC8& aChunk) const
{
	iChunkCount = EIdFlatChunk;
	return CMTPTypeCompoundBase::FirstReadChunk(aChunk);
	
}   		
EXPORT_C TInt CPTPIPInitCmdAck::NextReadChunk(TPtrC8& aChunk) const
{
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
	return ret;
}



EXPORT_C  void CPTPIPInitCmdAck::SetDeviceFriendlyName(TDesC16& aName)
{
	iBuffer.Create(aName,aName.Length()+KMTPNullCharLen);
	iBuffer.Append(KMTPNullChar);
}    

EXPORT_C TUint64 CPTPIPInitCmdAck::Size() const
{
	TUint64 size = CMTPTypeCompoundBase::Size();
	size += iBuffer.Size();
	return size;
}

