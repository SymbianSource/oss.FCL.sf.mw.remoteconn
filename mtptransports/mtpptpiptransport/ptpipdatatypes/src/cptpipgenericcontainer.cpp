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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipgenericcontainerTraces.h"
#endif


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
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_NEWL_ENTRY );
	CPTPIPGenericContainer* self = new (ELeave) CPTPIPGenericContainer();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_NEWL_EXIT );
	return self;
	}

/**
 Destructor.
 */
EXPORT_C CPTPIPGenericContainer::~CPTPIPGenericContainer()
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_CPTPIPGENERICCONTAINER_ENTRY );
	iChunkHeader.Close();
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_CPTPIPGENERICCONTAINER_EXIT );
	}

/**
 Constructor.
 */
CPTPIPGenericContainer::CPTPIPGenericContainer( ) :
	CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), iChunkHeader(
			KFlatChunkSize, *this),
			iElementInfo(iElementMetaData, ENumElements)
	{
OstTraceFunctionEntry0( DUP1_CPTPIPGENERICCONTAINER_CPTPIPGENERICCONTAINER_ENTRY );

	OstTraceFunctionExit0( DUP1_CPTPIPGENERICCONTAINER_CPTPIPGENERICCONTAINER_EXIT );
	}

/**
 Second phase constructor.
 */
void CPTPIPGenericContainer::ConstructL( )
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_CONSTRUCTL_ENTRY );
	iChunkHeader.OpenL ( );
	ChunkAppendL (iChunkHeader );
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_CONSTRUCTL_EXIT );
	}
/**
 Provides the container payload.
 @return The container payload.
 */
EXPORT_C MMTPType* CPTPIPGenericContainer::Payload() const
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_PAYLOAD_ENTRY );
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_PAYLOAD_EXIT );
	return iPayload;
	}

/**
 Sets the container payload.
 @param aPayload The new container payload.
 */
EXPORT_C void CPTPIPGenericContainer::SetPayloadL(MMTPType* aPayload)
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_SETPAYLOADL_ENTRY );
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
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_SETPAYLOADL_EXIT );
	}

EXPORT_C TUint CPTPIPGenericContainer::Type() const
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_TYPE_ENTRY );
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_TYPE_EXIT );
	return EPTPIPTypeGenericContainer;
	}

const CMTPTypeCompoundBase::TElementInfo& CPTPIPGenericContainer::ElementInfo(
		TInt aElementId ) const
	{
	OstTraceFunctionEntry0( CPTPIPGENERICCONTAINER_ELEMENTINFO_ENTRY );
	__ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
	OstTraceFunctionExit0( CPTPIPGENERICCONTAINER_ELEMENTINFO_EXIT );
	return iElementInfo[aElementId];
	}

