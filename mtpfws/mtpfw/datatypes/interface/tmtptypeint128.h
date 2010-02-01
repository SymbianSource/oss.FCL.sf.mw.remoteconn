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

#ifndef TMTPTYPEINT128_H
#define TMTPTYPEINT128_H

#include <e32std.h>
#include <mtp/mmtptype.h>
#include <mtp/mtpdatatypeconstants.h>

/**
Defines the MTP signed 128-bit integer data type.
@publishedPartner
@released 
*/
class TMTPTypeInt128 : public MMTPType
    {

public:

	IMPORT_C TMTPTypeInt128();	
	IMPORT_C TMTPTypeInt128(const TPtrC8& aData);

    IMPORT_C void Set(TInt64 aUpperValue, TUint64 aLowerValue);
    IMPORT_C TUint64 LowerValue() const;
    IMPORT_C TInt64 UpperValue() const;
    
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
	
private:
   
    /**
    Least significant 64-bit buffer offset.
    */
    static const TInt           KMTPTypeInt128OffsetLS = 0;
   
    /**
    Most significant 64-bit buffer offset.
    */
    static const TInt           KMTPTypeInt128OffsetMS = 8;
    
    /**
    The data buffer.
    */
	TBuf8<KMTPTypeINT128Size>   iData;
    };

#endif // TMTPTYPEINT128_H
