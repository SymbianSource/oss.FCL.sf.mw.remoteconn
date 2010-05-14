// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPEXTENSIONMAPPING_H
#define CMTPEXTENSIONMAPPING_H

#include <e32base.h>
#include <mtp/mtpprotocolconstants.h>

class CMTPExtensionMapping :
	public CBase
	{
public:
	static CMTPExtensionMapping* NewL(const TDesC& aExtension, TMTPFormatCode aFormatCode);
	static CMTPExtensionMapping* NewL(const TDesC& aExtension, TMTPFormatCode aFormatCode,const TDesC& aMIMEType);
	~CMTPExtensionMapping();
	
	const TDesC& Extension() const;
	TMTPFormatCode FormatCode() const;
	TUint16 SubFormatCode() const;
	const TDesC& MIMEType() const;
	TUint32 DpId() const;
	TUint EnumerationFlag() const;
	
	void SetExtensionL(const TDesC& aExtension);
	void SetMIMETypeL(const TDesC& aMIMEType);
	void SetDpId(const TUint32 aDpId);
	void SetFormatCode(const TMTPFormatCode aFormatCode);
	void SetSubFormatCode(TUint16  aSubFormatCode);
	void SetEnumerationFlag(const TUint aNeedFileDp);
	
	static TInt Compare(const CMTPExtensionMapping& aFirst, const CMTPExtensionMapping& aSecond);
	static TInt ComparewithMIME(const CMTPExtensionMapping& aFirst, const CMTPExtensionMapping& aSecond);
	
private:
	CMTPExtensionMapping(TMTPFormatCode aFormatCode);
	void ConstructL(const TDesC& aExtension,const TDesC& aMIMEType);
	
private:
	HBufC* iExtension;
	TMTPFormatCode iFormatCode;
	HBufC* iMIMEType;
	TUint32 iDpId;
	TUint   iNeedFileDp;
	TUint16  iSubFormatCode;
	};

#endif // CMTPEXTENSIONMAPPING_H
