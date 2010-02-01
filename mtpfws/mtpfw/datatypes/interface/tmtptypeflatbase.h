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

#ifndef TMTPTYPEFLATBASE_H
#define TMTPTYPEFLATBASE_H

#include <e32std.h>
#include <mtp/mmtptype.h>
#include <mtp/mtptypessimple.h>



/**
Defines the MTP flat data types base class. Flat data types are used to
represent MTP datasets comprising a single contiguous sequence of data
elements, each of a known type and length. This class implements generic
flat data type storage management capabilities and is intended for derivation
by concrete classes which implement specific MTP datasets.
@publishedPartner
@released
*/
class TMTPTypeFlatBase : public MMTPType
    {
public:

    IMPORT_C TMTPTypeFlatBase();

    IMPORT_C virtual void Reset();

    IMPORT_C virtual void GetL(TInt aElementId, MMTPType& aElement) const;
    IMPORT_C virtual void SetL(TInt aElementId, const MMTPType& aElement);

    IMPORT_C virtual void SetInt8(TInt aElementId, TInt8 aData);
    IMPORT_C virtual void SetInt16(TInt aElementId, TInt16 aData);
    IMPORT_C virtual void SetInt32(TInt aElementId, TInt32 aData);
    IMPORT_C virtual void SetInt64(TInt aElementId, TInt64 aData);

    IMPORT_C virtual void SetUint8(TInt aElementId, TUint8 aData);
    IMPORT_C virtual void SetUint16(TInt aElementId, TUint16 aData);
    IMPORT_C virtual void SetUint32(TInt aElementId, TUint32 aData);
    IMPORT_C virtual void SetUint64(TInt aElementId, TUint64 aData);

    IMPORT_C virtual TInt8 Int8(TInt aElementId) const;
    IMPORT_C virtual TInt16 Int16(TInt aElementId) const;
    IMPORT_C virtual TInt32 Int32(TInt aElementId) const;
    IMPORT_C virtual TInt64 Int64(TInt aElementId) const;

    IMPORT_C virtual TUint8 Uint8(TInt aElementId) const;
    IMPORT_C virtual TUint16 Uint16(TInt aElementId) const;
    IMPORT_C virtual TUint32 Uint32(TInt aElementId) const;
    IMPORT_C virtual TUint64 Uint64(TInt aElementId) const;

	inline TPtr8& DataBuffer()
    {
    return iBuffer;
    }



public: // From MMTPType

    IMPORT_C virtual TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TUint64 Size() const;
    IMPORT_C virtual TUint Type() const;

public:

    /**
    Defines the MTP flat data types element meta data.
    */
    struct TElementInfo
        {
        /**
        The element's MTP type identifier.
        */
        TInt iType;

        /**
        The offset (in bytes) of the element data from the start of the managed
        dataset buffer.
        */
        TInt iOffset;

        /**
        The size (in bytes) of the element data.
        */
        TInt iSize;
        };

protected:

    IMPORT_C virtual void SetBuffer(const TDes8& aBuffer);

	/**
	Provides the meta data for the specified element.
	@param aElementId The identifier of the requested element.
	@return The requested element's meta data.
	*/
	virtual const TElementInfo& ElementInfo(TInt aElementId) const = 0;

	//not owned
	/**
	The managed MTP dataset buffer.
	*/
	TPtr8   iBuffer;
    };

#endif //TMTPTYPEFLATBASE_H
