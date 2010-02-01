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
#ifndef CMTPTYPETRIVIALDATA_H_
#define CMTPTYPETRIVIALDATA_H_

#include <e32base.h>
#include <mtp/mmtptype.h>

class CMTPTypeTrivialData : public CBase, public MMTPType
    {
public:
    IMPORT_C static CMTPTypeTrivialData* NewL();
    IMPORT_C static CMTPTypeTrivialData* NewLC();
    IMPORT_C ~CMTPTypeTrivialData();
    
public: //MMTPType
    IMPORT_C virtual TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk, TUint aDataLength);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk, TUint aDataLength);
    
    IMPORT_C virtual TUint64 Size() const;
    IMPORT_C virtual TUint Type() const;
    
private:
    void ConstructL();
    CMTPTypeTrivialData();
    
private:
    RBuf8    iBuffer;
    };


#endif /* CMTPTYPETRIVIALDATA_H_ */
