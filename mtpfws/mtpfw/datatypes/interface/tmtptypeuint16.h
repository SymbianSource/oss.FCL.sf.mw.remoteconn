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

#ifndef TMTPTYPEUINT16_H
#define TMTPTYPEUINT16_H

#include <e32std.h>
#include <mtp/tmtptypeuintbase.h>

/**
Defines the MTP unsigned 16-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeUint16 : public TMTPTypeUintBase
    {

public:

	IMPORT_C TMTPTypeUint16();	
	IMPORT_C TMTPTypeUint16(TUint16 aData);

    IMPORT_C void Set(TUint16 aValue);
    IMPORT_C TUint16 Value() const;
    };

#endif // TMTPTYPEUINT16_H
