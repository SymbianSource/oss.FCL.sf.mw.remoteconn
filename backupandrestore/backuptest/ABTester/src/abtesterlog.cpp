// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Logging utilities
// 
//

/**
 @file
*/
#include "abtesterlog.h"

namespace conn
	{
	
	namespace ABTesterLog 
		{
		#if (defined(ABT_LOGGING_DEBUG_ONLY) && defined(_DEBUG)) || defined(ABT_LOGGING_DEBUG_AND_RELEASE)
		
        /** The flogger subsystem name 
		@internalComponent*/    
		_LIT(KLogDirectory,"connect");

		/** The component name 
		@internalComponent*/
        _LIT(KLogComponentName, "ABTestClient.txt");

        /** The maximum length of text that can be logged 
		@internalComponent*/                
		const TInt KMaxLogText = 0x100;
			
		
		void __Log(TRefByValue<const TDesC> aFmt,...)	
	 	/**	Logs a message to FLOGGER and to the UI depending on
	 	controlling macros.
	 	
	 	Note that FLOG macros are probably disabled in release builds, 
	 	so we might need to use something else for logging to files
		@internalComponent
	 	@param aFmt Buffer for logging
	 	*/
			{
			VA_LIST list;
		    VA_START(list,aFmt);
		    
		    TBuf<KMaxLogText> buf;
		    buf.FormatList(aFmt,list);   
		    
		#if defined(ABT_LOGGING_METHOD_FLOGGER)   
			RFileLogger::Write(KLogDirectory,KLogComponentName, EFileLoggingModeAppend,buf);
		#endif    
		    
		#if defined(ABT_LOGGING_METHOD_RDEBUG) || defined(ABT_LOGGING_METHOD_UI)
            
        	
            aData.Insert( 0, KLogComponentName );

            #if defined( ABT_LOGGING_METHOD_UI )
                User::InfoPrint( buf );
            #endif
        	#if defined( SBE_LOGGING_METHOD_RDEBUG )
            	RDebug::Print( buf );
        	#endif
    	#endif
			}
		}
		#endif
	}
	
