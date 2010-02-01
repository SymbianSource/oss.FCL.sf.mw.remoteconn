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
 @released
*/

#ifndef TMTPTYPEUINT32_H
#define TMTPTYPEUINT32_H

#include <e32std.h>
#include <mtp/tmtptypeuintbase.h>

/**
Defines the MTP signed 32-bit integer data type.
@publishedPartner
@released
*/
class TMTPTypeUint32 : public TMTPTypeUintBase
    {

public:

	IMPORT_C TMTPTypeUint32();	
	IMPORT_C TMTPTypeUint32(TUint32 aData);	

    IMPORT_C void Set(TUint32 aValue);
    IMPORT_C TUint32 Value() const;
    };

#endif // TMTPTYPEUINT32_H
