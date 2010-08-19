/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Debug utility for SyncServer
*
*/


#ifndef _SECON_DEBUG_H
#define _SECON_DEBUG_H

#ifdef _DEBUG
    
    // File logging
    #define __FLOGGING__
    
    #include <e32svr.h>
    #ifdef __FLOGGING__
        #include <f32file.h>
        #include <flogger.h>
    #endif
    
    NONSHARABLE_CLASS(TOverflowTruncate16) : public TDes16Overflow
        {
    public:
        void Overflow(TDes16& /*aDes*/) {}
        };
    
    NONSHARABLE_CLASS(TOverflowTruncate8) : public TDes8Overflow
        {
    public:
        void Overflow(TDes8& /*aDes*/) {}
        };
    
    _LIT( KLogDir, "SECON" );
    _LIT( KLogFile, "SeconDebug.txt" );
    
    _LIT(KTracePrefix16, "[SeConSync] ");
    _LIT8(KTracePrefix8, "[SeConSync] ");
    _LIT8(KFuncEntryFormat8, "%S : Begin");
    _LIT8(KFuncExitFormat8, "%S : End");
    _LIT8(KFuncFormat8, "><%S");
    
    const TInt KMaxLogLineLength = 512;
    
    // old function loggin macros
    #define FLOG(a) {FPrint(a);}
    #define FLOG_1(a,b) {FPrint(a,b);}
    #define FLOG_2(a,b,c) {FPrint(a,b,c);}
    #define FTRACE(a) {a;}
    #define LOGGER_ENTERFN( name )      {TRACE_FUNC_ENTRY;}
    #define LOGGER_LEAVEFN( name )      {TRACE_FUNC_EXIT;}
    
    #define LOGGER_WRITE( text )                    {_LIT( KTemp, text ); FPrint( KTemp );}
    #define LOGGER_WRITE_1( text,par1 )             {_LIT( KTemp, text ); FPrint( KTemp, par1 );}
    #define LOGGER_WRITE8_1( text,par1 )            {_LIT8( KTemp, text ); FPrint( KTemp, par1 );}
    #define LOGGER_WRITE_2( text,par1,par2 )        {_LIT( KTemp, text ); FPrint( KTemp, par1, par2 );}
    #define LOGGER_WRITE_3( text,par1,par2,par3 )   {_LIT( KTemp, text ); FPrint( KTemp, par1, par2, par3 );}
    
    // New function logging macros
    #define TRACE_FUNC_ENTRY {TPtrC8 ptr8((TUint8*)__PRETTY_FUNCTION__); FPrint(KFuncEntryFormat8, &ptr8);}
    #define TRACE_FUNC_EXIT {TPtrC8 ptr8((TUint8*)__PRETTY_FUNCTION__); FPrint(KFuncExitFormat8, &ptr8);}
    #define TRACE_FUNC {TPtrC8 ptr8((TUint8*)__PRETTY_FUNCTION__); FPrint(KFuncFormat8, &ptr8);}
    
    // Declare the FPrint function
    inline void FPrint( TRefByValue<const TDesC16> aFmt, ...)
        {
        VA_LIST list;
        VA_START(list,aFmt);
    #ifdef __FLOGGING__
        RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list );
    #endif
        TBuf16<KMaxLogLineLength> theFinalString;
        theFinalString.Append(KTracePrefix16);
        TOverflowTruncate16 overflow;
        theFinalString.AppendFormatList(aFmt,list,&overflow);
        RDebug::Print(theFinalString);
        }
    
    // Declare the FPrint function
    inline void FPrint(TRefByValue<const TDesC8> aFmt, ...)
        {
        VA_LIST list;
        VA_START(list, aFmt);
    #ifdef __FLOGGING__
        RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    #endif
        TOverflowTruncate8 overflow;
        TBuf8<KMaxLogLineLength> buf8;
        buf8.Append(KTracePrefix8);
        buf8.AppendFormatList(aFmt, list, &overflow);
        TBuf16<KMaxLogLineLength> buf16(buf8.Length());
        buf16.Copy(buf8);
        TRefByValue<const TDesC> tmpFmt(_L("%S"));
        RDebug::Print(tmpFmt, &buf16);
        }
#else
    
    // No loggings --> reduced code size
    #define FLOG(a)
    #define FLOG_1(a,b)
    #define FLOG_2(a,b,c)
    #define FTRACE(a)
    #define LOGGER_ENTERFN( name )
    #define LOGGER_LEAVEFN( name )
    #define LOGGER_WRITE( text )
    #define LOGGER_WRITE_1( text, par1 )
    #define LOGGER_WRITE8_1( text, par1 )
    #define LOGGER_WRITE_2( text, par1, par2 )
    #define LOGGER_WRITE_3( text, par1, par2, par3 )
    #define TRACE_FUNC_ENTRY
    #define TRACE_FUNC_EXIT
    #define TRACE_FUNC

#endif //_DEBUG

#endif // SECON_DEBUG_H

// End of file

