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

#include <mtp/mtpprotocolconstants.h>
#include "ptpipdatatypes.h"
#include "tptpiprequestpayload.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "tptpiprequestpayloadTraces.h"
#endif


// Dataset element metadata.
const TPTPIPTypeRequestPayload::TElementInfo
		TPTPIPTypeRequestPayload::iElementMetaData[ENumElements] =
			{
				{EMTPTypeUINT32, 0, KMTPTypeUINT32Size	}, // EDataPhase
				{EMTPTypeUINT16, 4, KMTPTypeUINT16Size	}, // EOpCode
				{EMTPTypeUINT32, 6, KMTPTypeUINT32Size	}, // ETransactionId
				{EMTPTypeUINT32, 10, KMTPTypeUINT32Size	}, // EParameter1
				{EMTPTypeUINT32, 14, KMTPTypeUINT32Size	}, // EParameter2
				{EMTPTypeUINT32, 18, KMTPTypeUINT32Size	}, // EParameter3
				{EMTPTypeUINT32, 22, KMTPTypeUINT32Size	}, // EParameter4
				{EMTPTypeUINT32, 26, KMTPTypeUINT32Size	}  // EParameter5
			};

/**
 Constructor.
 */
EXPORT_C TPTPIPTypeRequestPayload::TPTPIPTypeRequestPayload( ) :
iElementInfo(iElementMetaData, ENumElements),
iBuffer(KSize),
//iNumParameters(0)
iMaxSetParam(0)
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_TPTPIPTYPEREQUESTPAYLOAD_ENTRY );
	SetBuffer (iBuffer );
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_TPTPIPTYPEREQUESTPAYLOAD_EXIT );
	}

/**
 Provides the number of parameters in the block which are set.
 @return the number of set parameters.
 */
EXPORT_C TUint TPTPIPTypeRequestPayload::NumParameters() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_NUMPARAMETERS_ENTRY );
	//return iNumParameters;
	//return (iMaxSetParam - EParameter1 + 1);
	return (iMaxSetParam > 0 )? (iMaxSetParam - EParameter1 + 1) : 0;
	}

/**
 Copies up to five parameter values into the parameter block dataset from the specified dataset. 
 The paramters to be copied should be non null paramter or valid null paramter.

 @param aFrom The parameter values source dataset. This dataset should define three or five contiguous 32-bit parameter values starting at the specified offset.
 @param aParamStartOffset The element ID of the first in the set of contiguous 
 source dataset parameter values.
 @param aParamEndOffset The element ID of the last in the set of contiguous 
 source dataset parameter values.
 @param aIsNullParamValid a boolean value to check if a null parameter is valid. A value of ETrue means a null parameter is valid; EFalse means invalid.
 @param aNumOfNullParam the number of valid null parameters to be copied.
 */
EXPORT_C void TPTPIPTypeRequestPayload::CopyIn( const TMTPTypeFlatBase& aFrom,
		TUint aParamStartOffset, TUint aParamEndOffset,
		TBool aIsNullParamValid, TUint aNumOfNullParam )

	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_COPYIN_ENTRY );
	__ASSERT_DEBUG((aParamEndOffset >= aParamStartOffset && (aParamEndOffset - aParamStartOffset + 1) <= ENumElements), User::Invariant());
	TUint32 parameter = 0;
	TUint numberOfNullParam = 0;

	for (TUint t(EParameter1), s(aParamStartOffset); s <= aParamEndOffset; t++, s++)
		{
		parameter = aFrom.Uint32(s);
		if (parameter != KMTPNotSpecified32)
			{
			SetUint32(t, parameter);
			}
		else if(aIsNullParamValid && (numberOfNullParam < aNumOfNullParam))
			{
			SetUint32(t, parameter);
			numberOfNullParam++;
			}
		}

	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_COPYIN_EXIT );
	}

/**
 Copies the non-null parameter values from the parameter block dataset into the specified dataset. 
 Note: the number of parameters to be copied out depends on two conditions:
 1. The number of parameters contained in this parameter block dataset.
 2. The number of parameters to be copied out to the parameter values sink dataset, 
 which is (aParamEndOffset - aParamStartOffset + 1) by the caller.

 @param aTo The parameter values sink dataset. This dataset should define three or five contiguous 32-bit parameter values starting at the specified offset.
 @param aParamStartOffset the element ID of the first in the set of contiguous sink dataset parameter values.
 @param aParamEndOffset the element ID of the last in the set of contiguous sink dataset parameter values.
 */
EXPORT_C void TPTPIPTypeRequestPayload::CopyOut( TMTPTypeFlatBase& aTo,
		TUint aParamStartOffset, TUint aParamEndOffset )
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_COPYOUT_ENTRY );
	__ASSERT_DEBUG((aParamEndOffset >= aParamStartOffset && (aParamEndOffset - aParamStartOffset + 1) <= ENumElements), User::Invariant());
	for (TUint s(EParameter1), t(aParamStartOffset); (s <= iMaxSetParam && t <= aParamEndOffset); s++, t++)
		{
		aTo.SetUint32((t), Uint32(s));
		}
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_COPYOUT_EXIT );
	}

/**
 Resets the dataset.
 */
EXPORT_C void TPTPIPTypeRequestPayload::Reset( )
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_RESET_ENTRY );
	TMTPTypeFlatBase::Reset();
	iMaxSetParam = 0;
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_RESET_EXIT );
	}

/**
 Setter for the op code.
 */
EXPORT_C void TPTPIPTypeRequestPayload::SetUint16(TInt aElementId, TUint16 aData)
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_SETUINT16_ENTRY );
	__ASSERT_DEBUG((aElementId == EOpCode), User::Invariant());
	TMTPTypeFlatBase::SetUint16(aElementId, aData);
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_SETUINT16_EXIT );
	}

/**
 Getter for the op code. 
 */
EXPORT_C TUint16 TPTPIPTypeRequestPayload::Uint16(TInt aElementId) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_UINT16_ENTRY );
	__ASSERT_DEBUG((aElementId == EOpCode), User::Invariant());
	return TMTPTypeFlatBase::Uint16(aElementId);
	}

/**
 Setter for the parameters, data phase and tran id
 */
EXPORT_C void TPTPIPTypeRequestPayload::SetUint32(TInt aElementId, TUint32 aData )
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_SETUINT32_ENTRY );
	__ASSERT_DEBUG((aElementId != EOpCode), User::Invariant());
	if (aElementId >= EParameter1)
		{
		if (aElementId > iMaxSetParam)
			iMaxSetParam = aElementId;
		/**
		// Recalculate iNumParameters.
		TInt num((aElementId - EParameter1) + 1);

		if (num > iNumParameters)
			{
			iNumParameters = num;
			}
			*/
		}

	//  Set the element value.
	TMTPTypeFlatBase::SetUint32(aElementId, aData);
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_SETUINT32_EXIT );
	}

/**
 Getter for the parameters, data phase and tran id. 
 */
EXPORT_C TUint32 TPTPIPTypeRequestPayload::Uint32(TInt aElementId) const
	{
OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_UINT32_ENTRY );
//	__ASSERT_DEBUG((aElementId < iMaxSetParam ), User::Invariant());
//	__ASSERT_DEBUG((aElementId != EOpCode), User::Invariant());
	return TMTPTypeFlatBase::Uint32(aElementId);
	}

/**
 todo
 */
EXPORT_C TInt TPTPIPTypeRequestPayload::FirstReadChunk(TPtrC8& aChunk) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_FIRSTREADCHUNK_ENTRY );
	TInt ret(TMTPTypeFlatBase::FirstReadChunk(aChunk));
	TUint64 size(Size());

	if (size < aChunk.Size())
		{
		// Truncate the buffer at the last set parameter.
		aChunk.Set(aChunk.Left(size));
		}

	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_FIRSTREADCHUNK_EXIT );
	return ret;
	}

EXPORT_C TUint64 TPTPIPTypeRequestPayload::Size() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_SIZE_ENTRY );
	return (NumParameters() * KMTPTypeUINT32Size + KHeaderElementsSize);
	}

EXPORT_C TUint TPTPIPTypeRequestPayload::Type() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_TYPE_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_TYPE_EXIT );
	return EPTPIPTypeRequestPayload;
	}

EXPORT_C TBool TPTPIPTypeRequestPayload::CommitRequired() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_COMMITREQUIRED_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_COMMITREQUIRED_EXIT );
	return ETrue;
	}

/**
Reset the value of the max set param. 
 */
EXPORT_C MMTPType* TPTPIPTypeRequestPayload::CommitChunkL(TPtr8& /*aChunk*/)
	{
OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_COMMITCHUNKL_ENTRY );
//	iNumParameters = 0;

	// Recalculate iNumParameters.
	for (TUint i(EParameter1); (i <= EParameter5); i++)
		{
		if (TMTPTypeFlatBase::Uint32(i) != KMTPNotSpecified32)
			{
			//iNumParameters = (i - EParameter1 + 1 );
			iMaxSetParam = i;
			}
		}
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_COMMITCHUNKL_EXIT );
	return NULL;
	}

EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPTypeRequestPayload::ElementInfo(TInt aElementId) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPEREQUESTPAYLOAD_ELEMENTINFO_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPEREQUESTPAYLOAD_ELEMENTINFO_EXIT );
	return iElementInfo[aElementId];
	}

