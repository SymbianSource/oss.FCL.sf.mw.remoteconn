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

#ifndef __ABTESTERLOG_H__
#define __ABTESTERLOG_H__

#ifndef __FLOGGER_H__
#include <flogger.h>
#endif

#ifndef __E32STD_H__
#include <e32std.h>
#endif

namespace conn
	{
    /**
    @defgroup Log Log

    This module implements logging utilities. Information is currently
    logged via flogger, but this can be easily changed as logging is
    hidden by global macros.

    The log control macros allow enabling or disabling of the following
    type of logging:-

    @li UI logging (a message is displayed top right of the screen) see
    __DISPLAY_LOG_MESSAGES.

    @li flogger logging (a message is displayed in the flogger log file) see
    __DISPLAY_FLOG_MESSAGES.

    A nested namespace, SBServerLog, contains the class and global function that
    performs the logging operations: TLog, OpenLog(), CloseLog() and TLog
    operator() - which accepts a variable number of arguments. There is
    macro for each of these functions.

    OpenLog() and CloseLog() must be called only once and this is done by
    MainL().
    */ 
    
	/** LOG CONTROL MACROS */

    /** @{ */
	
	#if (defined(ABT_LOGGING_DEBUG_ONLY) && defined(_DEBUG)) || defined(ABT_LOGGING_DEBUG_AND_RELEASE)

        #define ABT_LOGGING_ENABLED
		
		namespace ABTesterLog
    	/**
    	@ingroup Log
        	This namespace hides the internal of logging from the rest of the system.
	    */
    	    {
			void __Log(TRefByValue<const TDesC> aFmt,...);	
	    	}//ABTesterLog
	    	
	    /** Logs a message */
		#define __LOG(TXT) {_LIT(__KText,TXT); ABTesterLog::__Log(__KText);}	

	    /** Logs a message plus an additional value. The text must
	     contain an appropriate printf alike indication, e.g. %d if the additional
	    value is an integer. */
	    #define __LOG1(TXT, A) {_LIT(__KText,TXT); ABTesterLog::__Log(__KText, A);}	

	    /** Logs a message plus two additional values. The text must
	     contain two appropriate printf alike indications.*/
	    #define __LOG2(TXT, A, B) {_LIT(__KText,TXT); ABTesterLog::__Log(__KText, A, B);}
	    
	    /** Logs a message plus two additional values. The text must
	     contain two appropriate printf alike indications.*/
	    #define __LOG3(TXT, A, B, C) {_LIT(__KText,TXT); ABTesterLog::__Log(__KText, A, B, C);}	

		
		/** Log Utility macro. 
		Executes the function specified under a trap and if the
		error code is not KErrNone logs the message specified before
		leaving. Note that the message must have a '%d' in it!*/
		#define __EXECLOGL(F, STR) 	TRAPD(__r, F);\
			if (__r != KErrNone) { __LOG1(STR, __r); User::Leave(__r); }
			
	#else
		
		#define __LOG(TXT)
        #define __LOG1(TXT, A)
        #define __LOG2(TXT, A, B)
        #define __LOG3(TXT, A, B, C )
        
	#endif
    /** @} */

	
	}//conn
		
#endif //__SBSSLOG_H__