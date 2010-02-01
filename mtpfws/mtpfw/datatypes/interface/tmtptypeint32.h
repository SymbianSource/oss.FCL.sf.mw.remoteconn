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

#ifndef TMTPTYPEINT32_H
#define TMTPTYPEINT32_H

#include <e32std.h>
#include <mtp/tmtptypeintbase.h>

/**
Defines the MTP signed 32-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeInt32 : public TMTPTypeIntBase
    {

public:

	IMPORT_C TMTPTypeInt32();	
	IMPORT_C TMTPTypeInt32(TInt32 aData);	

    IMPORT_C void Set(TInt32 aValue);
    IMPORT_C TInt32 Value() const;
    };

#endif // TMTPTYPEINT32_H
