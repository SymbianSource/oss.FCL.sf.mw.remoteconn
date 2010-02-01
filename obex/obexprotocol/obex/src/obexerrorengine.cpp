// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file
 @internalComponent
*/

#include "obexerrorengine.h"
#include "OBEXUTIL.H"

CObexErrorEngine* CObexErrorEngine::NewL()
	{
	CObexErrorEngine* self = new (ELeave) CObexErrorEngine;
	return self;
	}

CObexErrorEngine::CObexErrorEngine()
	{
	iLastError = ENoError;
	}

/**
Function used to retreive an extended error code for the last completed client operation.

@param aErrorSet The set of errors that the resolved error will lie within.
@return An extended error code that lies within the error set specified.
*/
TUint CObexErrorEngine::LastError(TObexClientErrorResolutionSetType aErrorSet) const
	{
	TUint ret = 0;

	if (aErrorSet == EObexClientBasicErrorResolutionSet)
		{
		switch (iLastError)
			{
			case ENoError:
				{
				ret = EObexRequestAccepted;
				}
				break;

			case EAlreadyConnected:
			case EChallengeRejected:
			case EChallengeAbsent:
			case EBadConnectionId:
			case EErrorResponseFromServer:
				{
				ret = EObexRequestNotAccepted;
				}
				break;

			case EResponseTimeout:
				{
				ret = EObexRequestTimeout;
				}
				break;

			case ECannotInsertConnectInfo:
			case EPrepareConnectPacketIncorrectState:
			case EResponseWhileWriting:
			case ECannotExtractConnectInfo:
			case ECannotProcessChallenge:
			case EUnexpectedChallengeResponse:
			case EBadOpcodeInConnectPacket:
			case EAborted:
			case ECannotInitialiseObject:
			case ECannotSetConnectionId:
			case ECannotPreparePacket:
			case EMultipacketResponse:
			case ECannotExtractFinalPutHeader:
			case EPutOutOfSync:
			case EGetPrematureSuccess:
			case EGetResponseParseError:
				{
				ret = EObexRequestLocalInterruption;
				}
				break;

			case ETransportUpFailed:
			case EOpOutstandingOnTransportDown:
			case EDisconnected:
				{
				ret = EObexRequestLinkInterruption;
				}
				break;

			default:
				{
				IrOBEXUtil::Fault(EUnderlyingErrorUnrecognised);
				}
			}
		}
	else
		{
		IrOBEXUtil::Fault(EErrorSetUnrecognised);
		}

	return ret;
	}

void CObexErrorEngine::SetLastError(TObexClientUnderlyingError aError)
	{
	iLastError = aError;
	}
