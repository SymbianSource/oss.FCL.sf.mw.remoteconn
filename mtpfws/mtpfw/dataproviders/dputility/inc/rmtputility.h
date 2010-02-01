// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef RMTPUTILITY_H_
#define RMTPUTILITY_H_

#include <e32std.h> 
#include <e32base.h>
#include <badesca.h>
#include <mtp/mtpprotocolconstants.h>
#include "rmtpframework.h"
#include "mtpdebug.h"

class MMTPDataProviderFramework;
class CMTPObjectMetaData;
class CMTPExtensionMapping;

class RMTPUtility
	{
public:
	RMTPUtility();
	
    void OpenL(MMTPDataProviderFramework& aFramework);
    void Close();
    
public:
	IMPORT_C TBool TTime2MTPTimeStr(const TTime& aTime, TDes& aRet ) const;
	IMPORT_C TBool MTPTimeStr2TTime(const TDesC& aTimeString, TTime& aRet) const;
	IMPORT_C void RenameObjectL( TUint aObjectHandle, const TDesC& aNewName );
	IMPORT_C HBufC* ContainerMimeType( const TDesC& aFullPath );
	IMPORT_C TMTPFormatCode FormatFromFilename( const TDesC& aFullFileName );
	IMPORT_C void FormatExtensionMapping();
	IMPORT_C TMTPFormatCode GetFormatByExtension(const TDesC& aExtension);
	IMPORT_C TUint32 GetDpId(const TDesC& aExtension,const TDesC& aMIMEType);
    IMPORT_C TUint GetEnumerationFlag(const TDesC& aExtension);	
	
private:
	void RenameAllChildrenL(TUint32 aStorageId, TUint32 aParentHandle, const TDesC& aNewFolderName, const TDesC& aOldFolderName);
	TBool GetYear(const TDesC& aTimeString, TInt& aYear) const;
	TBool GetMonth(const TDesC& aTimeString, TMonth& aMonth) const;
	TBool GetDay(const TDesC& aTimeString, TInt& aDay) const;
	TBool GetHour(const TDesC& aTimeString, TInt& aHour) const;
	TBool GetMinute(const TDesC& aTimeString, TInt& aMinute) const;
	TBool GetSecond(const TDesC& aTimeString, TInt& aSecond) const;
	TBool GetTenthSecond(const TDesC& aTimeString, TInt& aTenthSecond) const;
	TBool GetTimeZone(const TDesC& aTimeString, TBool& aPositiveTimeZone, TInt& aTimeZoneInHour, TInt& aTimeZoneInMinute) const;
	HBufC* OdfMimeTypeL( const TDesC& aFullFileName );
	void AppendFormatExtensionMapping(const CDesCArray& aFormatExtensionMapping,TUint32 aDpId);
	
	
private:
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
	MMTPDataProviderFramework*  iFramework;
	RMTPFramework				iSingleton;
    RPointerArray<CMTPExtensionMapping> iFormatMappings;
    
	};

#endif /* RMTPUTILITY_H_ */
