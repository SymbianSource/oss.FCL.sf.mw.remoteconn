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

#ifndef CMTPTYPELIST_H_
#define CMTPTYPELIST_H_


#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/mtpdatatypeconstants.h>


class CMTPTypeList : public CMTPTypeCompoundBase
    {
public:
    IMPORT_C static CMTPTypeList* NewL(TMTPTypeIds aListType, TMTPTypeIds aElementType);
    IMPORT_C static CMTPTypeList* NewLC(TMTPTypeIds aListType, TMTPTypeIds aElementType);
    IMPORT_C virtual ~CMTPTypeList();
    IMPORT_C virtual void AppendL(const MMTPType* aElement);
    IMPORT_C virtual void Remove(const TInt aIndex);
    IMPORT_C virtual TInt ElementType() const;
    IMPORT_C virtual TUint32 NumberOfElements() const;
    IMPORT_C MMTPType& ElementL(const TInt aIndex) const;
    IMPORT_C void ResetAndDestroy();
    
public: // From CMTPTypeCompoundBase

    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TUint Type() const;
    IMPORT_C virtual TBool CommitRequired() const;
    IMPORT_C virtual MMTPType* CommitChunkL(TPtr8& aChunk); 
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk, TUint aDataLength);
    
    
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TInt ValidateChunkCount() const;

protected:
	CMTPTypeList(TInt aArrayType, TInt aElementType);
	void InitListL();
	
private:
    void ConstructL();
    TInt UpdateWriteSequenceErr(TInt aErr);
    void AppendElementChunkL(const MMTPType* aElement);
    
private:

    /**
    The write data stream states.
    */
    enum TReadWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Elements data stream is in progress.
        */
        EElementChunks,          
        };

private:

    /**
    The write data stream error state. 
    */
    TUint                                           iWriteSequenceErr;
    
    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;

    /**
    The NumberOfElements element metadata content.
    */
    CMTPTypeCompoundBase::TElementInfo              iInfoNumberOfElements;
    
    /**
    The Element array metadata content. Note that this is declared mutable 
    to allow state updates while processing a read data stream.
    */
    mutable CMTPTypeCompoundBase::TElementInfo      iInfoElement;
    
    TInt                                            iListType;
    TInt                                            iElementType;
    
    static const TUint                              KChunckNumberOfHeader;
    static const TUint                              KChunkNumberofElements;
    TMTPTypeUint32                                  iChunkNumberOfElements;    
    
    RPointerArray<MMTPType>                         iChunksElement;
    };



#endif /* CMTPTYPELIST_H_ */
