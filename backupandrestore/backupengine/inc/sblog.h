/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
* Logging utilities
* 
*
*/



/**
 @file
*/

#ifndef __SBLOG_H__
#define __SBLOG_H__

#include <flogger.h>
#include <e32debug.h>

/**
@defgroup Log Log

This module implements logging utilities. Information is currently
logged via flogger, but this can be easily changed as logging is
hidden by global macros.

A nested namespace, log, contains the class and global function that
performs the logging operations: TLog, OpenLog(), CloseLog() and TLog
operator() - which accepts a variable number of arguments. There is
macro for each of these functions.

OpenLog() and CloseLog() must be called only once and this is done by
MainL().
*/ 

namespace conn
	{
    
	/** LOG CONTROL MACROS */

    /** @{ */
      
    #if (defined(SBE_LOGGING_DEBUG_ONLY) && defined(_DEBUG)) || defined(SBE_LOGGING_DEBUG_AND_RELEASE)

        #define SBE_LOGGING_ENABLED

        namespace securebackuplog
        /**
        @ingroup Log
            This namespace hides the internal of logging from the rest of the system.
        */
            {
            void __LogRaw( TDes& aData );
		    void __Log(TRefByValue<const TDesC> aFmt,...);	
		    void __DebugDump( const TDesC& aFormat, const TUint8* aAddress, TInt aLength );
	        }//securebackuplog

        /** Logs a message */
        #define __LOG(TXT)                              { _LIT(__KText,TXT); securebackuplog::__Log(__KText); }

        /** Logs a message plus an additional value. The text must
         contain an appropriate printf alike indication, e.g. %d if the additional
        value is an integer. */
        #define __LOG1(TXT, A)                          { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A); }
        #define __LOG2(TXT, A, B)                       { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A, B); }
        #define __LOG3(TXT, A, B, C )                   { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A, B, C); }
        #define __LOG4(TXT, A, B, C, D )                { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A, B, C, D); }
        #define __LOG5(TXT, A, B, C, D, E )             { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A, B, C, D, E); }
        #define __LOG6(TXT, A, B, C, D, E, F )          { _LIT(__KText,TXT); securebackuplog::__Log(__KText, A, B, C, D, E, F); }
        
        /** Logs data as ascii text (hex encoded) */
        #define __LOGDATA(TXT, DATAPOINTER, LEN)        { _LIT(__KText,TXT); securebackuplog::__DebugDump(__KText, DATAPOINTER, LEN); }
    
    #else

    	#define __LOG(TXT)
        #define __LOG1(TXT, A)
        #define __LOG2(TXT, A, B)
        #define __LOG3(TXT, A, B, C )
        #define __LOG4(TXT, A, B, C, D )
        #define __LOG5(TXT, A, B, C, D, E )
        #define __LOG6(TXT, A, B, C, D, E, F )
        #define __LOGDATA(TXT, DATAPOINTER, LEN)
    
    #endif

    /** @} */
	
	}//conn
		
#endif //__SBLOG_H__
