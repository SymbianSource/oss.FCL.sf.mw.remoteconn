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

#ifndef CMTPTYPESERVICEPROPDESCLIST_H_

#define CMTPTYPESERVICEPROPDESCLIST_H_



#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypelist.h>

class CMTPTypeServicePropDesc;

/** 
Defines the MTP Service Property Description list dataset. 
@see CMTPTypeObjectPropDesc
@publishedPartner
@released
*/ 
class CMTPTypeServicePropDescList : public CMTPTypeList
    {
public:
    
    IMPORT_C static CMTPTypeServicePropDescList* NewL();
    IMPORT_C static CMTPTypeServicePropDescList* NewLC();

    IMPORT_C virtual ~CMTPTypeServicePropDescList();  
    IMPORT_C void AppendL(CMTPTypeServicePropDesc* aServicePropDesc);
    IMPORT_C CMTPTypeServicePropDesc& ElementL(TUint aIndex) const; 
    
private:
    CMTPTypeServicePropDescList();
    void ConstructL();   

    };


/** 
Defines the MTP Service Property Description dataset. The dataset is a 
component element of the CMTPTypeServicePropDescList.
@see CMTPTypeObjectPropDesc
@publishedPartner
@released
*/ 
class CMTPTypeServicePropDesc : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP ObjectPropDesc Service Property Description dataset element  identifiers.
    */
    enum TMTPObjectPropDescElement
        {
        /**
        Service Property code.
        */
        EServicePropertyCode,
        
        /**
        Data-type code
        */
        EDataType1,
        
        /**
        Get/Set.
        */
        EGetSet1,
        
        /**
        Form Flag .
        */
        EFormFlag,
        
        /**
        FORM element. This element depends EFormFlag and is absent if EFormFlag 
        is ENone.
        */
        EForm,         

        /**
        The number of dataset elements.
        */
        ENumElements,
        };
    
    /**
    MTP ServicePropDesc dataset Get/Set element values.
    */
    enum TMTPServicePropDescGetSet
        {

        /** 
        Get.
        */
        EReadOnly       = 0x00,

        /** 
        Get/Set.
        */
        EReadWrite      = 0x01
        };
    
public:
    IMPORT_C static CMTPTypeServicePropDesc* NewL();
    IMPORT_C static CMTPTypeServicePropDesc* NewLC();
    IMPORT_C static CMTPTypeServicePropDesc* NewL( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm );
    IMPORT_C static CMTPTypeServicePropDesc* NewLC( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm);
    
    IMPORT_C virtual ~CMTPTypeServicePropDesc();  
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
    
private:

    CMTPTypeServicePropDesc();
    void ConstructL( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm );
    
    TBool HasFormField(TUint8 aFormFlag) const;

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
    Streaming the EForm data chunk.
    */
    EFormChunk           
    };
  
    /**
    Data chunk identifiers.
    */
    enum TChunkIds
       {
       /**
       The first flat data chunk ID.
       */
       EIdFlat1Chunk,
       
       /**
       The EForm element data chunk ID.
       */
       EIdFormChunk,
       
       /**
       The number of data chunks.
       */
       EIdNumChunks
       };

private:

    /**
    The data chunk count of ObjectPropDesc datasets with an EForm of other than 
    ENone.
    */
    static const TUint                              KNumChunksWithForm;
    
    /**
    The data chunk count of ObjectPropDesc datasets with an EForm of ENone.
    */
    static const TUint                              KNumChunksWithoutForm;

    /*
    The size in bytes of the first flat data chunk.
    */
    static const TUint                              KFlat1ChunkSize;
    
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The write data stream completion state. This will be set to EFlat2Chunk for 
    ObjectPropDesc datasets with an EForm of ENone, otherwise this will be 
    EFormChunk.
    */
    TUint                                           iWriteSequenceCompletionState;
    TUint 											iWriteSequenceState;
    
    
    /**
    The flat data chunk comprising elements EPKeyNamespace and EPKeyID.
    */    
    RMTPTypeCompoundFlatChunk                       iChunkFlat1;
    
    /**
    The EForm element data chunk.
    */
    CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm      iChunkForm;
    };




#endif /* CMTPTYPESERVICEPROPDESCLIST_H_ */
