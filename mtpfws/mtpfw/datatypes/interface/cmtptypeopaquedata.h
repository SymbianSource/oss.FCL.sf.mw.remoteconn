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
#ifndef CMTPTYPEOPAQUEDATA_H_
#define CMTPTYPEOPAQUEDATA_H_

#include <e32base.h>
#include <mtp/mmtptype.h>

class CMTPTypeOpaqueData : public CBase, public MMTPType
    {
public:
    IMPORT_C static CMTPTypeOpaqueData* NewL();
    IMPORT_C static CMTPTypeOpaqueData* NewLC();
    IMPORT_C static CMTPTypeOpaqueData* NewL(const TDesC8 &aDes);
    IMPORT_C static CMTPTypeOpaqueData* NewLC(const TDesC8 &aDes);
    IMPORT_C ~CMTPTypeOpaqueData();
    
public: //MMTPType
    IMPORT_C virtual TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TBool CommitRequired() const;
    IMPORT_C virtual MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C virtual TUint64 Size() const;
    IMPORT_C virtual TUint Type() const;
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk, TUint aDataLength);

public:
    IMPORT_C TInt Read(TPtrC8 &aDes) const;
    IMPORT_C TInt Write(const TPtrC8 &aDes);
    
private:
    void ConstructL();
    void ConstructL(const TDesC8 &aDes);
    CMTPTypeOpaqueData();
    TInt CreateBuffer( const TInt aMaxSize );

    
private:
    RBuf8    iBuffer;
    TPtr8    iPtrBuffer;
    };


#endif /* CMTPTYPEOPAQUEDATA_H_ */
