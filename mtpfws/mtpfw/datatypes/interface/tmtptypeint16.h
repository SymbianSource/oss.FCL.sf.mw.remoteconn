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

#ifndef TMTPTYPEINT16_H
#define TMTPTYPEINT16_H

#include <e32std.h>
#include <mtp/tmtptypeintbase.h>

/**
Defines the MTP signed 16-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeInt16 : public TMTPTypeIntBase
    {

public:

	IMPORT_C TMTPTypeInt16();	
	IMPORT_C TMTPTypeInt16(TInt16 aData);

    IMPORT_C void Set(TInt16 aValue);
    IMPORT_C TInt16 Value() const;
    };

#endif // TMTPTYPEINT16_H
