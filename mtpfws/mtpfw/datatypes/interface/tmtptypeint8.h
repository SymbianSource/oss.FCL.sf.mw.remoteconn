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

#ifndef TMTPTYPEINT8_H
#define TMTPTYPEINT8_H

#include <e32std.h>
#include <mtp/tmtptypeintbase.h>

/**
Defines the MTP signed 8-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeInt8 : public TMTPTypeIntBase
    {

public:

	IMPORT_C TMTPTypeInt8();	
	IMPORT_C TMTPTypeInt8(TInt8 aData);	

    IMPORT_C void Set(TInt8 aValue);
    IMPORT_C TInt8 Value() const;
    };

#endif // TMTPTYPEINT8_H
