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

#include "cmtpextensionmapping.h"

CMTPExtensionMapping* CMTPExtensionMapping::NewL(const TDesC& aExtension, TMTPFormatCode aFormatCode)
	{
	CMTPExtensionMapping* self = new (ELeave) CMTPExtensionMapping(aFormatCode);
	CleanupStack::PushL(self);
	self->ConstructL(aExtension,KNullDesC);
	CleanupStack::Pop(self);
	return self;
	}

CMTPExtensionMapping* CMTPExtensionMapping::NewL(const TDesC& aExtension, TMTPFormatCode aFormatCode,const TDesC& aMIMEType)
    {
    CMTPExtensionMapping* self = new (ELeave) CMTPExtensionMapping(aFormatCode);
    CleanupStack::PushL(self);
    self->ConstructL(aExtension,aMIMEType);
    CleanupStack::Pop(self);
    return self;
    }
	
CMTPExtensionMapping::CMTPExtensionMapping(TMTPFormatCode aFormatCode) :
	iFormatCode(aFormatCode)
	{
	}
	
void CMTPExtensionMapping::ConstructL(const TDesC& aExtension,const TDesC& aMIMEType)
    {
    iExtension = aExtension.AllocL();
    iMIMEType = aMIMEType.AllocL();
    }
	
CMTPExtensionMapping::~CMTPExtensionMapping()
	{
	delete iExtension;
	delete iMIMEType;
	}
	
const TDesC& CMTPExtensionMapping::Extension() const
	{
	return *iExtension;
	}

const TDesC& CMTPExtensionMapping::MIMEType() const
    {
    return *iMIMEType;
    }
	
TMTPFormatCode CMTPExtensionMapping::FormatCode() const
	{
	return iFormatCode;
	}

TUint16 CMTPExtensionMapping::SubFormatCode() const
    {
    return iSubFormatCode;
    }

TUint32 CMTPExtensionMapping::DpId() const
    {
    return iDpId;
    }

TUint CMTPExtensionMapping::EnumerationFlag() const
    {
    return iNeedFileDp;
    }

void CMTPExtensionMapping::SetExtensionL(const TDesC& aExtension)
	{
	delete iExtension;
	iExtension = NULL;
	iExtension = aExtension.AllocL();	
	}

void CMTPExtensionMapping::SetMIMETypeL(const TDesC& aMIMEType)
    {
    delete iMIMEType;
    iMIMEType = NULL;
    
    iMIMEType = aMIMEType.AllocL();   
    }
	
void CMTPExtensionMapping::SetDpId(const TUint32 aDpId)
    {
    iDpId = aDpId;
    }

void CMTPExtensionMapping::SetFormatCode(const TMTPFormatCode aFormatCode)
    {
    iFormatCode = aFormatCode;
    }
void CMTPExtensionMapping::SetSubFormatCode(TUint16  aSubFormatCode)
    {
    iSubFormatCode = aSubFormatCode;
    }

void CMTPExtensionMapping::SetEnumerationFlag(const TUint aNeedFileDp)
    {
    iNeedFileDp = aNeedFileDp;
    }

TInt CMTPExtensionMapping::Compare(const CMTPExtensionMapping& aFirst, const CMTPExtensionMapping& aSecond)
	{
	return (aFirst.Extension().CompareF(aSecond.Extension()));
	}

TInt CMTPExtensionMapping::ComparewithMIME(const CMTPExtensionMapping& aFirst, const CMTPExtensionMapping& aSecond)
    {
     TInt i = Compare(aFirst,aSecond);
     if ( 0==i )
         {
         return aFirst.MIMEType().CompareF(aSecond.MIMEType());
         }
     return i;
    }
