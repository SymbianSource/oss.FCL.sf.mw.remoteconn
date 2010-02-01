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

#ifndef TMTPTYPEUINTBASE_H
#define TMTPTYPEUINTBASE_H

#include <e32std.h>
#include <mtp/mmtptype.h>

/**
Defines the MTP unsigned integer data type base class for 8 to 32-bit integers.
This class is intended for derivation.
@publishedPartner
@released
*/
class TMTPTypeUintBase : public MMTPType
    {
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    
protected:

	IMPORT_C TMTPTypeUintBase(TUint aData, TUint aSize, TInt aType);
	
protected:

    /**
    The data buffer.
    */
	TUint   iData;
	
private:

	/**
	The type's size in bytes.
	*/
	TUint64 iSize;
	
	/**
	The type's MTP datatype code.
	*/
	TUint   iType;
    };

#endif // TMTPTYPEUINTBASE_H
