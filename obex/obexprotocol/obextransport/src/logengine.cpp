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
 @internalTechnology
*/


#include <e32base.h>
#include <comms-infras/commsdebugutility.h>
#include "logger.h"


#ifdef __FLOG_ACTIVE
_LIT8(KSubsystem, "obex");
_LIT8(KLogCmpt, "logengine");
#endif


NONSHARABLE_CLASS(TLogData)
	{
	public:
#ifdef __FLOG_ACTIVE
		TLogData();
		
		void SetLogTags(const TDesC8& aCmpt);

		TInt iAccessCount;

		RFileLogger iLogEngine;
		TBuf8<KMaxTagLength> iCurrentComponent;
#endif
	};


#ifdef __FLOG_ACTIVE
TLogData::TLogData()
	: iAccessCount(0), iCurrentComponent(KNullDesC8)
	{}

void TLogData::SetLogTags(const TDesC8& aCmpt)
	{
	if (aCmpt != iCurrentComponent)
		{
		iLogEngine.SetLogTags(KSubsystem, aCmpt.Left(KMaxTagLength));
		iCurrentComponent = aCmpt.Left(KMaxTagLength);
		}
	}
#endif

#define GETLOG TLogData* __logger = static_cast<TLogData*>(Dll::Tls());



EXPORT_C /*static*/ TInt CObexLog::Connect()
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (!__logger)
		{
		CObexLog::Write(KLogCmpt, _L8("Opening new logger connection"));
		__logger = new TLogData();
		if (!__logger)
			{
			CObexLog::Write(KLogCmpt, _L8("Opening logger connection failed, no memory"));
			return KErrNoMemory;
			}
		
		__logger->iLogEngine.Connect();
		Dll::SetTls(__logger);
		}
	
	__logger->iAccessCount++;
	CObexLog::WriteFormat(KLogCmpt, _L8("Opening -- %d instances now open"), __logger->iAccessCount);
		
	return KErrNone;
#else
	return KErrNotSupported;
#endif
	}


EXPORT_C /*static*/ void CObexLog::Close()
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		TInt& count = __logger->iAccessCount;
		
		if (count)
			{
			count--;
			CObexLog::WriteFormat(KLogCmpt, _L8("Closing -- %d instance(s) left open"), count);
			if (!count)
				{
				__logger->iLogEngine.Close();
				delete __logger;
				Dll::SetTls(NULL);
				CObexLog::Write(KLogCmpt, _L8("Fully closed and deleted, now flogging statically."));
				}
			}
		else
			{
			CObexLog::Write(KLogCmpt, _L8("Not closing -- not opened"));
			}
		}
#endif
	}



EXPORT_C /*static*/ void CObexLog::Write(const TDesC8& IF_FLOGGING(aCmpt), const TDesC8& IF_FLOGGING(aText))
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.Write(aText);
		}
	else
		{
		RFileLogger::Write(KSubsystem, aCmpt, aText);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::WriteFormat(const TDesC8& IF_FLOGGING(aCmpt), TRefByValue<const TDesC8> IF_FLOGGING(aFmt), ...)
	{
#ifdef __FLOG_ACTIVE
	VA_LIST list;
	VA_START(list, aFmt);

	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.WriteFormat(aFmt, list);
		}
	else
		{
		RFileLogger::WriteFormat(KSubsystem, aCmpt, aFmt, list);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::WriteFormat(const TDesC8& IF_FLOGGING(aCmpt), TRefByValue<const TDesC8> IF_FLOGGING(aFmt), VA_LIST& IF_FLOGGING(aList))
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.WriteFormat(aFmt, aList);
		}
	else
		{
		RFileLogger::WriteFormat(KSubsystem, aCmpt, aFmt, aList);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::Write(const TDesC8& IF_FLOGGING(aCmpt), const TDesC16& IF_FLOGGING(aText))
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.Write(aText);
		}
	else
		{
		RFileLogger::WriteFormat(KSubsystem, aCmpt, aText);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::WriteFormat(const TDesC8& IF_FLOGGING(aCmpt), TRefByValue<const TDesC16> IF_FLOGGING(aFmt), ...)
	{
#ifdef __FLOG_ACTIVE
	VA_LIST list;
	VA_START(list, aFmt);

	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.WriteFormat(aFmt, list);
		}
	else
		{
		RFileLogger::WriteFormat(KSubsystem, aCmpt, aFmt, list);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::WriteFormat(const TDesC8& IF_FLOGGING(aCmpt), TRefByValue<const TDesC16> IF_FLOGGING(aFmt), VA_LIST& IF_FLOGGING(aList))
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.WriteFormat(aFmt, aList);
		}
	else
		{
		RFileLogger::WriteFormat(KSubsystem, aCmpt, aFmt, aList);
		}
#endif
	}


EXPORT_C /*static*/ void CObexLog::HexDump(const TDesC8& IF_FLOGGING(aCmpt), const TText* IF_FLOGGING(aHeader), const TText* IF_FLOGGING(aMargin), const TUint8* IF_FLOGGING(aPtr), TInt IF_FLOGGING(aLen))
	{
#ifdef __FLOG_ACTIVE
	GETLOG;
	
	if (__logger)
		{
		__logger->SetLogTags(aCmpt);
		__logger->iLogEngine.HexDump(aHeader, aMargin, aPtr, aLen);
		}
	else
		{
		RFileLogger::HexDump(KSubsystem, aCmpt, TPtrC8(aPtr, aLen), KNullDesC8);
		}
#endif
	}







/**
Leave (if error) verbosely- log name of file and line number just before 
leaving.
@param aFile The file we're leaving from.
@param aLine The line number we're leaving from.
@param aReason The leave code.
*/
EXPORT_C void VerboseLeaveIfErrorL(const TDesC8& IF_FLOGGING(aCpt), 
						  char* IF_FLOGGING(aFile), 
						  TInt IF_FLOGGING(aLine), 
						  TInt IF_FLOGGING(aReason))
	{
#ifdef __FLOG_ACTIVE
	// only leave if an error value
	if ( aReason >= KErrNone )
		{
		return;
		}

	_LIT8(KLeavePrefix, "LEAVE: ");

	TPtrC8 fullFileName((const TUint8*)aFile);
	TPtrC8 fileName(fullFileName.Ptr()+fullFileName.LocateReverse('\\')+1);

	TBuf8<256> buf;
	buf.Append(KLeavePrefix);
	buf.AppendFormat(_L8("aReason = %d [file %S, line %d]"), aReason, &fileName, 
		aLine);
	CObexLog::Write(aCpt, buf);

	// finally
	User::Leave(aReason);
#endif
	}

/**
Panic verbosely- log name of file and line number just before panicking.
@param aFile The file that's panicking.
@param aLine The line number that's panicking.
@param aReason The panic code.
@param aPanicName The text of the panic code.
@param aPanicCategory The panic category.
*/
EXPORT_C void VerbosePanic(const TDesC8& IF_FLOGGING(aCpt), 
				  char* IF_FLOGGING(aFile), 
				  TInt IF_FLOGGING(aLine), 
				  TInt IF_FLOGGING(aPanicCode), 
				  TText8* IF_FLOGGING(aPanicName),
				  const TDesC& IF_FLOGGING(aPanicCategory))
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KPanicPrefix, "PANIC: code ");
	
	TPtrC8 fullFileName((const TUint8*)aFile);
	TPtrC8 fileName(fullFileName.Ptr()+fullFileName.LocateReverse('\\')+1);

	TBuf8<256> buf;
	buf.Append(KPanicPrefix);
	buf.AppendFormat(_L8("%d = %s [file %S, line %d]"), 
		aPanicCode, 
		aPanicName, 
		&fileName, 
		aLine);
	CObexLog::Write(aCpt, buf);

	// finally
	User::Panic(aPanicCategory, aPanicCode);
#endif
	}

#ifdef __FLOG_ACTIVE
_LIT8(KInstrumentIn, ">>%S this = [0x%08x]");
_LIT8(KInstrumentOut, "<<%S");
#endif

EXPORT_C TFunctionLogger::TFunctionLogger(const TDesC8& IF_FLOGGING(aCpt), const TDesC8& IF_FLOGGING(aString), TAny* IF_FLOGGING(aThis))
	{
#ifdef __FLOG_ACTIVE
	iCpt.Set(aCpt);
	iString.Set(aString);
	CObexLog::WriteFormat(iCpt, KInstrumentIn, &iString, aThis);
#endif
	}

EXPORT_C TFunctionLogger::~TFunctionLogger()
	{
#ifdef __FLOG_ACTIVE
	CObexLog::WriteFormat(iCpt, KInstrumentOut, &iString);
#endif
	}

