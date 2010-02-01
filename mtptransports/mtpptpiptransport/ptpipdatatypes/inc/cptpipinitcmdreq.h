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

#ifndef CPTPIPINITCMDREQ_H_
#define CPTPIPINITCMDREQ_H_
#include <mtp/cmtptypecompoundbase.h>


#include <mtp/cmtptypestring.h>
class CPTPIPInitCmdRequest : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP USB generic bulk container dataset element identifiers.
    */
    enum TElements
        {
		ELength,
		EPktType,
		EInitiatorGUID,

		EVersion,
        
        /**
        The number of elements.
        */
        ENumElements        
 		}; 
public:
    
     IMPORT_C static CPTPIPInitCmdRequest* NewL();
     IMPORT_C ~CPTPIPInitCmdRequest();
    

     
public: // From MMTPType

     IMPORT_C TUint Type() const;
     IMPORT_C TDes16& HostFriendlyName();
     IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
	 IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
	 IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);	
	 
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

   

private:

    CPTPIPInitCmdRequest();        
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
   //     EIdNameChunk,
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

    CMTPTypeString*                                    iString; 
    TMTPTypeUint32									iVersion;
    TInt 									iChunkCount;
    RBuf16   iBuffer;
    };

#endif /*CPTPIPINITCMDREQ_H_*/
