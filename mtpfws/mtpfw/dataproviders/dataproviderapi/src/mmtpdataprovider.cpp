// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
*/

#include <mtp/mmtpdataprovider.h>


EXPORT_C void MMTPDataProvider::SupportedL(TMTPSupportCategory /*aCategory*/, CDesCArray& /*aStrings*/) const
    {
    
    }

EXPORT_C TAny* MMTPDataProvider::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    return NULL;
    }

EXPORT_C void MMTPDataProvider::StartObjectEnumerationL(TUint32 /*aStorageId*/)
	{
	__DEBUG_ONLY(_LIT(KMTPPanicCategory, "MMTPDataProvider"));
	__DEBUG_ONLY( User::Panic( KMTPPanicCategory, KErrNotSupported ));
	}

EXPORT_C void MMTPDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
	{
	StartObjectEnumerationL(aStorageId);
	}
