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
 @InternalTechnology
*/

#ifndef _CMTPTYPECOMPLEXBUFFER_H
#define _CMTPTYPECOMPLEXBUFFER_H

#include <e32base.h>
#include <mtp/mmtptype.h>
    
/**
Defines the MTP Buffer data type. 
@InternalTechnology
@released 
*/
class CMTPTypeComplexBuffer : public CBase, public MMTPType
    {
public:

    IMPORT_C static CMTPTypeComplexBuffer* NewL();
    IMPORT_C static CMTPTypeComplexBuffer* NewL(const TDesC8& aData);
    IMPORT_C static CMTPTypeComplexBuffer* NewLC();
    IMPORT_C static CMTPTypeComplexBuffer* NewLC(const TDesC8& aData);
	IMPORT_C ~CMTPTypeComplexBuffer();
    
	
	IMPORT_C const TDesC8& Buffer();
	IMPORT_C void SetBuffer(const TDesC8& aData );
	
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    
private:

    CMTPTypeComplexBuffer();
	void ConstructL(const TDesC8& aData);	

private:

    /**
    The read and write data stream states.
    */
    enum TReadWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Data stream is in progress.
        */
        EInProgress            
        };

    /**
    The intermediate data buffer. Note that this is declared mutable to allow 
    state updates while processing a read data stream.
    */
	mutable RBuf8   iBuffer;

       
    /**
    The read data stream state variable. Note that this is declared mutable 
    to allow state updates while processing a read data stream.
    */
	mutable TUint   iReadSequenceState;
    
    /**
    The write data stream state variable.
    */
	TUint           iWriteSequenceState;
    };
    
#endif // _CMTPTypeBuffer_H

