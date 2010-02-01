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

#ifndef TMTPTYPEUINT128_H
#define TMTPTYPEUINT128_H

#include <e32std.h>
#include <mtp/mmtptype.h>
#include <mtp/mtpdatatypeconstants.h>

/**
Defines the MTP unsigned 128-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeUint128 : public MMTPType
    {

public:

	IMPORT_C TMTPTypeUint128();	
	IMPORT_C TMTPTypeUint128(const TPtrC8& aData);
	IMPORT_C TMTPTypeUint128(const TUint64 aUpperValue, const TUint64 aLowerValue);

    IMPORT_C void Set(TUint64 aUpperValue, TUint64 aLowerValue);
    IMPORT_C TUint64 LowerValue() const;
    IMPORT_C TUint64 UpperValue() const;
    
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    
    IMPORT_C TBool Equal(const TMTPTypeUint128& aR) const;
    IMPORT_C TInt Compare(const TMTPTypeUint128& aR) const;
    IMPORT_C TInt Compare(const TUint64 aRUpper, const TUint64 aRLower) const;

private:
   
    /**
    Least significant 64-bit buffer offset.
    */
    static const TInt           KMTPTypeUint128OffsetLS = 0;
   
    /**
    Most significant 64-bit buffer offset.
    */
    static const TInt           KMTPTypeUint128OffsetMS = 8;

protected:
    /**
    The data buffer.
    */
	TBuf8<KMTPTypeUINT128Size>  iData;
    };

#endif // TMTPTYPEUINT128_H
