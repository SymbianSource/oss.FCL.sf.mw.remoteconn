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

#ifndef CPTPIPINITCMDACK_H_
#define CPTPIPINITCMDACK_H_



#include <mtp/cmtptypecompoundbase.h>


class CPTPIPInitCmdAck : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP USB generic bulk container dataset element identifiers.
    */
    enum TElements
        {
		ELength,
		EPktType,
		EConNumber,
		EResponderGUID,
		EVersion,
        
        /**
        The number of elements.
        */
        ENumElements        
 		}; 
public:
    
    IMPORT_C static CPTPIPInitCmdAck* NewL();
    IMPORT_C ~CPTPIPInitCmdAck();
	IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;   		
	IMPORT_C TInt NextReadChunk(TPtrC8& aChunk)  const;
	IMPORT_C void SetDeviceFriendlyName(TDesC16& aName); 
	IMPORT_C TUint64 Size() const;
     
public: // From MMTPType

	IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
private:

    CPTPIPInitCmdAck();        
    void ConstructL();   
    
private: // Owned
        
    /**
    Data chunk identifiers.
    */
    enum TChunkIds
        {
        /**
        The flat data chunk ID.
        */
        EIdFlatChunk,
        
        /**
        The EMTPExtensions element data chunk ID.
        */
     //   EIdNameChunk,
        /**
        
        */
        EIdVersionChunk,        
        /**
        The number of data chunks.
        */
        EIdNumChunks
        };
    
    /**
    The flat data chunk comprising the header elements EContainerLength to 
    ETransactionId.
    */
    RMTPTypeCompoundFlatChunk                       iChunkHeader;
    
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The size in bytes of the header data chunk.
    */
    static const TUint                              KFlatChunkSize;
    
private: // Not owned
	TMTPTypeUint32									iVersion;
    mutable TInt 									iChunkCount;
    RBuf16   iBuffer;		           
    };




#endif /*CPTPIPINITCMDACK_H_*/