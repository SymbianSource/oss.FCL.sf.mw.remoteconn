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

#ifndef TMTPTYPEUINT8_H
#define TMTPTYPEUINT8_H

#include <e32std.h>
#include <mtp/tmtptypeuintbase.h>

/**
Defines the MTP unsigned 8-bit integer data type.
@publishedPartner
@released
*/
class TMTPTypeUint8 : public TMTPTypeUintBase
    {

public:

	IMPORT_C TMTPTypeUint8();	
	IMPORT_C TMTPTypeUint8(TUint8 aData);

    IMPORT_C void Set(TUint8 aValue);
    IMPORT_C TUint8 Value() const;
    };

#endif // TMTPTYPEUINT8_H
