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

#ifndef TMTPTYPEUINT64_H
#define TMTPTYPEUINT64_H

#include <e32std.h>
#include <mtp/mmtptype.h>

/**
Defines the MTP unsigned 64-bit integer data type.
@publishedPartner
@released
*/
class TMTPTypeUint64 : public MMTPType
    {

public:

	IMPORT_C TMTPTypeUint64();	
	IMPORT_C TMTPTypeUint64(TUint64 aData);

    IMPORT_C void Set(TUint64 aValue);
    IMPORT_C TUint64 Value() const;
    
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;	
	
private:

    /**
    The data buffer.
    */
	TUint64 iData;
    };

#endif // TMTPTYPEUINT64_H
