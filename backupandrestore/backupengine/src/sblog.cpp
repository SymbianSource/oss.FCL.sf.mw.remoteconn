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
// Implementation of logging functions
// 
//

/**
 @file
*/

#include "sblog.h"

namespace conn
	{
	
	namespace securebackuplog 
		{
        #if (defined(SBE_LOGGING_DEBUG_ONLY) && defined(_DEBUG)) || defined(SBE_LOGGING_DEBUG_AND_RELEASE)

        /** The flogger directory
    	@internalComponent */    
    	_LIT(KLogDirectory,"connect");

    	/** The flogger file
    	 @internalComponent */
        _LIT(KLogFile, "securebackup.txt");

        /** The maximum length of text that can be logged
    	 @internalComponent */
    	const TInt KMaxLogData = 0x200;
    
        void __LogRaw( TDes& aData )
        /** Performs the logging operation based upon SBEngine.mmh macro configuration
		@param aData The data to be logged
        */
            {
		    #if defined(SBE_LOGGING_METHOD_FLOGGER)
	        	RFileLogger::Write(KLogDirectory, KLogFile, EFileLoggingModeAppend, aData);
	    	#endif    

            #if defined(SBE_LOGGING_METHOD_RDEBUG) || defined(SBE_LOGGING_METHOD_UI)
            
            /** The logging component name
    		 @internalComponent */
        	_LIT(KLogComponentName, "[SBE] ");
        	
                aData.Insert( 0, KLogComponentName );

                #if defined( SBE_LOGGING_METHOD_UI )
                    User::InfoPrint( aData );
                #endif
            	#if defined( SBE_LOGGING_METHOD_RDEBUG )
                	RDebug::Print( _L("%S"), &aData );
            	#endif
            #endif
            }




		void __Log( TRefByValue<const TDesC> aFmt, ... )	
	 	/** Logs a message to FLOGGER and to the UI depending on
	 	controlling macros.
	 	
	 	Note that FLOG macros are probably disabled in release builds, 
	 	so we might need to use something else for logging to files

		@internalComponent
		@param aFmt The formatting codes
        */
			{
			VA_LIST list;
		    VA_START(list,aFmt);
		    
		    TBuf< KMaxLogData > buf;
		    buf.FormatList(aFmt,list); 
		    
            __LogRaw( buf );
			}


        void __DebugDump( const TDesC& aFormat, const TUint8* aAddress, TInt aLength )
        /** Logs binary data as ASCII (hex encoded). Useful for debugging data transfer
        @param aFormat The format specifier, must always include a string format identifer, i.e. <code>%S</code>
        @param aAddress The starting memory address containing data that is to be logged
        @param aLength The amount of data (in bytes) to log, starting at <code>aAddress</code>
        */
            {
        	_LIT( KEndOfAddressText, ": ");
            _LIT( KDoubleSpace, "  " );
            _LIT( KSingleSpace, " " );

            TInt len = aLength;
            const TInt maxLen = aLength;
            const TUint8* pDataAddr = aAddress;

            TBuf<KMaxLogData> formatBuffer;
        	TBuf<81> out;
        	TBuf<20> ascii;
        	TInt offset = 0;
        	const TUint8* a = pDataAddr;
            //
        	while(len>0)
        		{
        		out.Zero();
        		ascii.Zero();
        		out.AppendNumFixedWidth((TUint) a, EHex, 8);
        		out.Append( KEndOfAddressText );

                TUint b;
        		for (b=0; b<16; b++)
        			{
                    TUint8 c = ' ';
                    if	((pDataAddr + offset + b) < pDataAddr + maxLen)
        	            {
        	            c = *(pDataAddr + offset + b);
        				out.AppendNumFixedWidth(c, EHex, 2);
        	            }
                    else
        	            {
        				out.Append( KDoubleSpace );
        	            }

                    out.Append( KSingleSpace );

                    if (c<0x20 || c>=0x7f || c=='%')
        				c=0x2e;

                    ascii.Append(TChar(c));
        			}
        		
                out.Append(ascii);
                out.ZeroTerminate();

                formatBuffer.Format( aFormat, &out );
                __LogRaw( formatBuffer );

                a += 16;
        		offset += 16;
        		len -= 16;
                }
            }
			
        #endif
		}//securebackuplog
	}
