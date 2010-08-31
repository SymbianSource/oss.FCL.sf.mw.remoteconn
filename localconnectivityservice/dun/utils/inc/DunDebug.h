/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Common debug functionality for this component
*
*/


#ifndef DUN_DEBUG_H
#define DUN_DEBUG_H

_LIT( KComponentName, "[DUN] " );

//#define FILE_LOGGING

#ifdef _DEBUG

#ifdef FILE_LOGGING

#include <e32std.h>
#include <f32file.h>
#include <flogger.h>

#define FTRACE(a)    {a;}

_LIT( KLogFile,"dun.txt" );
_LIT( KLogDir, "dun" );
_LIT( KLogDirFullName,"c:\\logs\\dun\\" );

// Declare the FPrint function
//
inline void FPrint( const TRefByValue<const TDesC> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list );
    }

inline void FPrintRaw( const TDesC8& /*aDes*/ )
    {
    }

#else

#include <e32svr.h>

#define FTRACE(a)    {a;}

/**
Overflow handler for VA_LIST parsing into a fixed size buffer to
be printed out using RDebug.
*/
NONSHARABLE_CLASS( TRDebugOverflowHander ) : public TDesOverflow
{
    /**
    The function that will get called when the buffer is over-flowed.
    In this case we just ignore the overflow, so the buffer will
    effectively be truncated.
    */
    virtual void Overflow( TDes16& /*aDes*/ ) { return; }
};

/** The maximum length of a log line (in characters) output using RDebug. */
const TUint KRDebugMaxLineLen = 0x100;

// Declare the FPrint function
//

inline void FPrint( const TRefByValue<const TDesC> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    TRDebugOverflowHander overflow;
    TBuf<KRDebugMaxLineLen> buf;
    TRefByValue<const TDesC> fmt = aFmt;
    buf.Copy( KComponentName );
    buf.AppendFormatList( fmt, list, &overflow );
    RDebug::Print( buf );
    }

inline void FPrintRaw( const TDesC8& aDes )
    {
    RDebug::RawPrint( aDes );
    }

#endif // FILE_LOGGING

// ===========================================================================
#else // // No loggings --> Reduced binary size
// ===========================================================================
#define FTRACE( a )

#endif // _DEBUG

#endif  // DUN_DEBUG_H

// End of File
