// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef TMTPTYPEGUID_H
#define TMTPTYPEGUID_H

#include <e32std.h>
#include <e32base.h>
#include <mtp/mmtptype.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypeuint128.h>


/**
Defines the MTP Guid data type.
@publishedPartner
@released 
*/
class TMTPTypeGuid : public TMTPTypeUint128
    {

public:

	IMPORT_C TMTPTypeGuid();	
	IMPORT_C TMTPTypeGuid(const TDesC& aData);
	IMPORT_C TMTPTypeGuid(const TUint64 aData1,const TUint64 aData2);
    IMPORT_C void Set(const TUint64 aData1,const TUint64 aData2);
    IMPORT_C void SetL(const TDesC& aData);
    IMPORT_C TInt ToString( TDes& aRetDes ) const;
    IMPORT_C static TBool IsGuidFormat(const TDesC& aData);
    
private:
    /*
     * The number of Uint16 elements in GUID
     */
    static const TUint                               KMTPGUIDUint16Num = 2;
    
    /*
     * The number of Uint8 elements in GUID
     */
    static const TUint                               KMTPGUIDUint8Num = 8;
    
    /*
     * GUID struct
     */
    typedef struct T_GUID
        {
        TUint32 iUint32;
        TUint16 iUint16[KMTPGUIDUint16Num];
        TUint8  iByte[KMTPGUIDUint8Num];
        } TGUID;
    
private:
    TInt StrToGUID(const TDesC& aData, TGUID& aGUID) const;
    };

#endif // TMTPTYPEGUID_H
