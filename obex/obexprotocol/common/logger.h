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
// All rights reserved.
// 
//

/**
 @file
 @internalTechnology
*/


#ifndef LOGGER_H
#define LOGGER_H

#include <e32base.h>

// Control function entry and exit logging using a compile-time switch.
//#define __LOG_FUNCTIONS__

// Dump contents of packets using a compile-time switch.
//#define __LOG_PACKET_DUMP__

class TFunctionLogger;

#ifndef __COMMSDEBUGUTILITY_H__		// comms-infras/commsdebugutility.h not included
#ifdef _DEBUG						// If this is a debug build...
// Set flogging active.
#define __FLOG_ACTIVE
#endif
#endif

#ifdef __FLOG_ACTIVE
#define IF_FLOGGING(a) a
#else
#define IF_FLOGGING(a)
#endif

_LIT8(KDefaultLogFile, "obex");

#ifdef __FLOG_ACTIVE
#define LEAVEIFERRORL(a)				VerboseLeaveIfErrorL(KLogComponent, __FILE__, __LINE__, a)
#define PANIC(CAT, CODE) 				VerbosePanic(KLogComponent, __FILE__, __LINE__, CODE, (TText8*)#CODE, CAT)
#define FLOG(a)							CObexLog::Write(KDefaultLogFile, a);

#define FTRACE(a)						{a;}
#define LOG(text)						CObexLog::Write(KLogComponent, text);
#define LOG1(text, a)					CObexLog::WriteFormat(KLogComponent, text, a);
#define LOG2(text, a, b)				CObexLog::WriteFormat(KLogComponent, text, a, b);
#define LOG3(text, a, b, c)				CObexLog::WriteFormat(KLogComponent, text, a, b, c);
#define LOGHEXDESC(desc)				CObexLog::HexDump(KLogComponent, 0, 0, desc.Ptr() , desc.Length());
#define LOGHEXRAW(data, len)			CObexLog::HexDump(KLogComponent, 0, 0, data, len);
#else
#define LEAVEIFERRORL(a)				static_cast<void>(User::LeaveIfError(a))
#define PANIC(CAT, CODE) 				User::Panic(CAT, CODE)
#define FLOG(a)
#define FTRACE(a)
#define LOG(text)
#define LOG1(text, a)
#define LOG2(text, a, b)
#define LOG3(text, a, b, c)
#define LOGHEXDESC(desc)
#define LOGHEXRAW(data, len)
#endif // __FLOG_ACTIVE

#define FORCED_LOG_FUNC					TFunctionLogger __instrument(KLogComponent, TPtrC8((TUint8*)__PRETTY_FUNCTION__), (TAny*)this);
#define FORCED_LOG_STATIC_FUNC_ENTRY	TFunctionLogger __instrument(KLogComponent, TPtrC8((TUint8*)__PRETTY_FUNCTION__), (TAny*)NULL);

#if ( defined __FLOG_ACTIVE && defined __LOG_FUNCTIONS__ )
#define LOG_LINE						CObexLog::Write(KLogComponent, KNullDesC8());
#define LOG_FUNC						FORCED_LOG_FUNC
#define LOG_STATIC_FUNC_ENTRY			FORCED_LOG_STATIC_FUNC_ENTRY
#else
#define LOG_LINE
#define LOG_FUNC
#define LOG_STATIC_FUNC_ENTRY
#endif



NONSHARABLE_CLASS(CObexLog) : public CBase
	{
public:
	IMPORT_C static TInt Connect();
	IMPORT_C static void Close();
	
	IMPORT_C static void Write(const TDesC8& aCmpt, const TDesC8& aText);
	IMPORT_C static void WriteFormat(const TDesC8& aCmpt, TRefByValue<const TDesC8> aFmt, ...);
	IMPORT_C static void WriteFormat(const TDesC8& aCmpt, TRefByValue<const TDesC8> aFmt, VA_LIST& aList);
	IMPORT_C static void Write(const TDesC8& aCmpt, const TDesC16& aText);
	IMPORT_C static void WriteFormat(const TDesC8& aCmpt, TRefByValue<const TDesC16> aFmt, ...);
	IMPORT_C static void WriteFormat(const TDesC8& aCmpt, TRefByValue<const TDesC16> aFmt, VA_LIST& aList);
	IMPORT_C static void HexDump(const TDesC8& aCmpt, const TText* aHeader, const TText* aMargin, const TUint8* aPtr, TInt aLen);
	};


#ifndef NO_FPRINT
inline void FPrint(const TRefByValue<const TDesC> IF_FLOGGING(aFmt), ...)
	{
#ifdef __FLOG_ACTIVE
	VA_LIST list;
	VA_START(list,aFmt);
	CObexLog::WriteFormat(KDefaultLogFile, aFmt, list);
#endif
	}
#endif


#ifndef NO_FHEX_PTR
inline void FHex(const TUint8* IF_FLOGGING(aPtr), TInt IF_FLOGGING(aLen))
	{
#ifdef __FLOG_ACTIVE
	CObexLog::HexDump(KDefaultLogFile, 0, 0, aPtr, aLen);
#endif
	}
#endif


#ifndef NO_FHEX_DESC
inline void FHex(const TDesC8& IF_FLOGGING(aDes))
	{
#ifdef __FLOG_ACTIVE
	FHex(aDes.Ptr(), aDes.Length());
#endif
	}
#endif


IMPORT_C void VerboseLeaveIfErrorL(const TDesC8& aCpt, 
						  char* aFile, 
						  TInt aLine, 
						  TInt aReason);
						  
IMPORT_C void VerbosePanic(const TDesC8& aCpt, 
				  char* aFile, 
				  TInt aLine, 
				  TInt aPanicCode, 
				  TText8* aPanicName,
				  const TDesC& aPanicCategory);
				  
NONSHARABLE_CLASS(TFunctionLogger)
	{
public:
	IMPORT_C TFunctionLogger(const TDesC8& aCpt, const TDesC8& aString, TAny* aThis);
	IMPORT_C ~TFunctionLogger();
	
private:
	TPtrC8 iCpt;
	TPtrC8 iString;
	};

#endif	// LOGGER_H
