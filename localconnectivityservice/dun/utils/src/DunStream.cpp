/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Definitions needed for common stream functionality
*
*/


/*
 * TODO: When local media is of type RComm, listening on it is started with
 * RComm::NotifyDataAvailable() call. Check that USB ACM port and Irda RCOMM
 * (and any other new media in the future) behaves correctly so that when
 * RComm::ReadOneOrMore() is issued, the read is issued immediately without
 * checking for new data. If waiting for new data happens in this
 * NotifyDataAvailable/ReadOneOrMore combination, raise a defect to Symbian.
 */

#include "DunStream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunStream::~CDunStream()
    {
    FTRACE(FPrint( _L("CDunStream::~CDunStream()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunStream::~CDunStream() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunStream::ResetData()
    {
    // APIs affecting this:
    // AddConnMonCallback()
    iCallbacksR.Close();
    iCallbacksW.Close();
    // AddSkippedError()
    iOkErrorsR.Close();
    iOkErrorsW.Close();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Adds error code to consider as "no error" to either endpoint
// ---------------------------------------------------------------------------
//
TInt CDunStream::AddSkippedError( TInt aError,
                                  TDunOperationType aOperationType )
    {
    FTRACE(FPrint( _L("CDunStream::AddSkippedError()" ) ));
    RArray<TInt>* okErrors = NULL;
    if ( aOperationType == EDunOperationTypeRead )
        {
        okErrors = &iOkErrorsR;
        }
    else if ( aOperationType == EDunOperationTypeWrite )
        {
        okErrors = &iOkErrorsW;
        }
    else
        {
        FTRACE(FPrint( _L("CDunStream::AddSkippedError() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    if ( aError >= 0 )  // errors can't be >= 0
        {
        FTRACE(FPrint( _L("CDunStream::AddSkippedError() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TInt retTemp = okErrors->Find( aError );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunStream::AddSkippedError() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = okErrors->Append( aError );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunStream::AddSkippedError() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunStream::AddSkippedError() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Adds callback for line status change controlling
// The callback will be called when serious read error is detected
// ---------------------------------------------------------------------------
//
TInt CDunStream::AddConnMonCallback( MDunConnMon* aCallback,
                                     TDunOperationType aOperationType )
    {
    FTRACE(FPrint( _L("CDunStream::AddConnMonCallback()" ) ));
    RPointerArray<MDunConnMon>* callbacks = NULL;
    if ( aOperationType == EDunOperationTypeRead )
        {
        callbacks = &iCallbacksR;
        }
    else if ( aOperationType == EDunOperationTypeWrite )
        {
        callbacks = &iCallbacksW;
        }
    else
        {
        FTRACE(FPrint( _L("CDunStream::AddConnMonCallback() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunStream::AddConnMonCallback() (aCallback) not initialized!" ) ));
        return KErrGeneral;
        }
    TInt retTemp = callbacks->Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunStream::AddCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = callbacks->Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunStream::AddCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunStream::AddCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets buffering for this stream
// ---------------------------------------------------------------------------
//
TInt CDunStream::SetBuffering( TPtr8* aBufferPtr )
    {
    FTRACE(FPrint( _L("CDunStream::SetBuffering()" ) ));
    if ( !aBufferPtr )
        {
        FTRACE(FPrint( _L("CDunStream::SetBuffering() (aBufferPtr) not initialized!" ) ));
        return KErrGeneral;
        }
    if ( iBufferPtr )
        {
        FTRACE(FPrint( _L("CDunStream::SetBuffering() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    iBufferPtr = aBufferPtr;
    FTRACE(FPrint( _L("CDunStream::SetBuffering() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunStream::SetMedia( RComm* aComm, TDunMediaContext aMediaContext )
    {
    FTRACE(FPrint( _L("CDunStream::SetMedia() (RComm)" ) ));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunStream::SetMedia() (RComm) (aComm not initialized!) complete" ) ));
        return KErrGeneral;
        }
    if ( aMediaContext == EDunMediaContextNetwork )
        {
        iNetwork = aComm;
        }
    else if ( aMediaContext == EDunMediaContextLocal )
        {
        iComm = aComm;
        }
    else
        {
        FTRACE(FPrint( _L("CDunStream::SetMedia() (RComm) (ERROR) complete" )));
        return KErrGeneral;
        }
    FTRACE(FPrint( _L("CDunStream::SetMedia() (RComm) complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunStream::SetMedia( RSocket* aSocket,
                           TDunMediaContext aMediaContext )
    {
    FTRACE(FPrint( _L("CDunStream::SetMedia() (RSocket)" ) ));
    if ( !aSocket )
        {
        FTRACE(FPrint( _L("CDunStream::SetMedia() (RSocket) (aSocket not initialized!) complete" ) ));
        return KErrGeneral;
        }
    if ( aMediaContext == EDunMediaContextLocal )
        {
        iSocket = aSocket;
        }
    else
        {
        FTRACE(FPrint( _L("CDunStream::SetMedia() (RSocket) (not supported) complete" ) ));
        return KErrNotSupported;
        }
    FTRACE(FPrint( _L("CDunStream::SetMedia() (RSocket) complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Gets media context
// ---------------------------------------------------------------------------
//
TDunMediaContext CDunStream::GetMediaContext( TDunStreamType aStreamType )
    {
    FTRACE(FPrint( _L("CDunStream::GetMediaContext()" ) ));
    if ( aStreamType == EDunStreamTypeUpstream )
        {
        if ( iOperationType == EDunOperationTypeRead )
            {
            return EDunMediaContextLocal;
            }
        else if ( iOperationType == EDunOperationTypeWrite )
            {
            return EDunMediaContextNetwork;
            }
        else
            {
            return EDunMediaContextUndefined;
            }
        }
    else if ( aStreamType == EDunStreamTypeDownstream )
        {
        if ( iOperationType == EDunOperationTypeRead )
            {
            return EDunMediaContextNetwork;
            }
        else if ( iOperationType == EDunOperationTypeWrite )
            {
            return EDunMediaContextLocal;
            }
        else
            {
            return EDunMediaContextUndefined;
            }
        }
    FTRACE(FPrint( _L("CDunStream::GetMediaContext() complete" ) ));
    return EDunMediaContextUndefined;
    }

// ---------------------------------------------------------------------------
// Check whether an error code is severe error or not
// ---------------------------------------------------------------------------
//
TInt CDunStream::ProcessErrorCondition( TInt aError, TBool& aIsError )
    {
    FTRACE(FPrint( _L("CDunStream::ProcessErrorCondition() (Dir=%d)" ), iDirection));
    aIsError = EFalse;
    if ( aError != KErrNone )
        {
        aIsError = ETrue;
        RArray<TInt>* okErrors = NULL;
        if ( iOperationType == EDunOperationTypeRead )
            {
            okErrors = &iOkErrorsR;
            }
        else // iOperationType == EDunOperationTypeWrite
            {
            okErrors = &iOkErrorsW;
            }
        TInt retTemp = okErrors->Find( aError );
        if ( retTemp == KErrNotFound )
            {
            FTRACE(FPrint( _L("CDunStream::ProcessErrorCondition() (Dir=%d) (%d=ETrue) complete" ), iDirection, aError));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunStream::ProcessErrorCondition() (Dir=%d) (%d=EFalse) complete" ), iDirection, aError));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// CDunStream::CDunStream
// ---------------------------------------------------------------------------
//
CDunStream::CDunStream() : CActive( EPriorityHigh )
    {
    Initialize();
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunStream::Initialize()
    {
    // Don't initialize iUtility here (it is set through NewL)
    iBufferPtr = NULL;
    iReadLengthSocket = 0;
    iOperationType = EDunOperationTypeUndefined;
    iTransferState = EDunStateIdle;
    iDirection = EDunDirectionUndefined;
    iNetwork = NULL;
    iSocket = NULL;
    iComm = NULL;
    }
