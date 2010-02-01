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
#include "cptpipgenericcontainer.h"

// Dataset constants
const TUint CPTPIPGenericContainer::KFlatChunkSize(8);

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CPTPIPGenericContainer::iElementMetaData[CPTPIPGenericContainer::ENumElements] = 
    {
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EContainerLength
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    4,                  KMTPTypeUINT32Size}},   // EContainerType
		{EIdPayloadChunk,   EMTPTypeUndefined,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}}     // EPayload
    };

/**
 Generic Container's factory method. 
 This is used to create an empty MTP PTPIP generic container dataset type. 
 @return  Ownership IS transfered.
 @leave One of the system wide error codes, if unsuccessful.
 */
EXPORT_C CPTPIPGenericContainer* CPTPIPGenericContainer::NewL()
	{
	CPTPIPGenericContainer* self = new (ELeave) CPTPIPGenericContainer();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
 Destructor.
 */
EXPORT_C CPTPIPGenericContainer::~CPTPIPGenericContainer()
	{
	iChunkHeader.Close();
	}

/**
 Constructor.
 */
CPTPIPGenericContainer::CPTPIPGenericContainer( ) :
	CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), iChunkHeader(
			KFlatChunkSize, *this),
			iElementInfo(iElementMetaData, ENumElements)
	{

	}

/**
 Second phase constructor.
 */
void CPTPIPGenericContainer::ConstructL( )
	{
	iChunkHeader.OpenL ( );
	ChunkAppendL (iChunkHeader );
	}
/**
 Provides the container payload.
 @return The container payload.
 */
EXPORT_C MMTPType* CPTPIPGenericContainer::Payload() const
	{
	return iPayload;
	}

/**
 Sets the container payload.
 @param aPayload The new container payload.
 */
EXPORT_C void CPTPIPGenericContainer::SetPayloadL(MMTPType* aPayload)
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
	}

EXPORT_C TUint CPTPIPGenericContainer::Type() const
	{
	return EPTPIPTypeGenericContainer;
	}

const CMTPTypeCompoundBase::TElementInfo& CPTPIPGenericContainer::ElementInfo(
		TInt aElementId ) const
	{
	__ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
	return iElementInfo[aElementId];
	}

