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

#ifndef TMTPTYPENULL_H
#define TMTPTYPENULL_H

#include <e32std.h>
#include <mtp/mmtptype.h>

/**
Defines the MTP null (zero length) data type base class. 
@publishedPartner
@released 
*/
class TMTPTypeNull : public MMTPType
    {    
public:

    IMPORT_C TMTPTypeNull();
	IMPORT_C void SetBuffer(const TDes8& aBuffer);
    
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    
private:
	TPtr8 iNullBuffer;
    };

#endif //TMTPTYPENULL_H
