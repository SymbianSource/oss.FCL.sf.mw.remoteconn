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

#ifndef CMTPTYPESERVICEMETHODPARAMEXTNFORM_H_
#define CMTPTYPESERVICEMETHODPARAMEXTNFORM_H_


#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypeobjectpropdesc.h>

class CMTPTypeString;

/** 
Defines the MTP ObjectPropDesc Service Method Parameter Extension FORM . The form is a 
component element of the ObjectPropDesc dataset.
@see CMTPTypeObjectPropDesc
@publishedPartner
@released
*/ 
class CMTPTypeServiceMethodParamExtnForm : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP ObjectPropDesc Service Object Property Extension FORM element dataset identifiers.
    */
    enum TMTPObjectPropDescElement
        {
        /**
        Object Property PKeyNamespace.
        */
        EPKeyNamespace,
        
        /**
        Object Property PKeyID
        */
        EPKeyID,
        
        /**
        PropertyName
        */
        EPropertyName,
        
        /**
         * Parameter Type
         */
        EParameterType,
        
        /**
         * ParameterNumber
         */
        EParameterNumber,
        
        /**
        Form Flag (read-only).
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
    
    enum TMTPServiceMethodParamType
        {
        /*
         * if reserved
         */
        EResserved,
        
        /*
         * if in
         */
        EIn,
        /*
         * if out
         */
        EOut,
        /*
         * if in/out
         */
        EInOut
        };
    
public:
    IMPORT_C static CMTPTypeServiceMethodParamExtnForm* NewL( const TUint aDataType );
    IMPORT_C static CMTPTypeServiceMethodParamExtnForm* NewLC( const TUint aDataType );
    IMPORT_C static CMTPTypeServiceMethodParamExtnForm* NewL( const TUint aDataType, const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aParamType, const TUint8 aParamNum, const TUint8 aFormFlag, const MMTPType* aForm);
    IMPORT_C static CMTPTypeServiceMethodParamExtnForm* NewLC( const TUint aDataType, const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aParamType, const TUint8 aParamNum, const TUint8 aFormFlag, const MMTPType* aForm);
    
    IMPORT_C virtual ~CMTPTypeServiceMethodParamExtnForm();  
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
    
private:

    CMTPTypeServiceMethodParamExtnForm( const TUint aDataType );
    void ConstructL( const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aParamType, const TUint8 aParamNum, const TUint8 aFormFlag, const MMTPType* aForm );
    
    TBool HasFormField(TUint8 aFormFlag) const;
    TInt UpdateWriteSequenceErr(TInt aErr);
    TBool ReadableElementL(TInt aElementId) const;
    TBool WriteableElementL(TInt aElementId) const;
private:

    /**
    The write data stream states.
    */
    enum TWriteSequenceState
      {
      /**
      Data stream is inactive.
      */
      EIdle,
      
      /**
      Streaming the first flat data chunk.
      */
      EFlat1Chunk,
      
      /**
      Streaming the EDefaultValue data chunk..
      */
      ENameChunk,
      
      /**
      Streaming the second flat data chunk.
      */
      EFlat2Chunk,
      
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
       The EPropertyName element data chunk ID.
       */
       EIdNameChunk,
       
       /**
       The second flat data chunk ID.
       */
       EIdFlat2Chunk,
       
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
    The size in bytes of the second flat data chunk.
    */
    static const TUint                              KFlat2ChunkSize;
    
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
   
    /**
    The write data stream error state. 
    */
    TUint                                           iWriteSequenceErr;
    
    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;
    
    /**
    The write data stream completion state. This will be set to EFlat2Chunk for 
    ObjectPropDesc datasets with an EForm of ENone, otherwise this will be 
    EFormChunk.
    */
    TUint                                           iWriteSequenceCompletionState;
    
    /**
    The flat data chunk comprising elements EPKeyNamespace and EPKeyID.
    */    
    RMTPTypeCompoundFlatChunk                       iChunkFlat1;
    
    /**
    The EPropertyName element data chunk.
    */
    CMTPTypeString*                                        iChunkName;
    
    /**
    The flat data chunk comprising elements EFormFlag.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat2;
    
    /**
    The EForm element data chunk.
    */
    CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm      iChunkForm;
    
    /**
    The property's MTP data type identifier datacode.
    */
    TUint                                       iDataType;
    
    /**
    The initialised flag indicating if construction is completeand that 
    read-only elements can no longer be modified.
    */
    TBool                                           iInitialised;
    };




#endif /* CMTPTYPESERVICEMETHODPARAMEXTNFORM_H_ */

