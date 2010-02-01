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

#ifndef CMTPTYPEOBJECTPROPDESC_H
#define CMTPTYPEOBJECTPROPDESC_H

#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypeinterdependentpropdesc.h>



class CMTPTypeServiceObjPropExtnForm;
class CMTPTypeServiceMethodParamExtnForm;
/** 
Defines the MTP ObjectPropDesc dataset. The ObjectPropDesc dataset describes 
an MTP data object property, and is generated in response to a 
GetObjectPropDesc operation request.
@publishedPartner
@released 
*/ 
class CMTPTypeObjectPropDesc : public CMTPTypeCompoundBase
    {
    friend class        CMTPTypeServiceObjPropExtnForm;
    friend class        CMTPTypeServiceMethodParamExtnForm;
    friend class		CMTPTypeServicePropDesc;
public:

    /**
    MTP ObjectPropDesc dataset element identifiers.
    */
    enum TMTPObjectPropDescElement
        {
        /**
        Property Code (read-only).
        */
        EPropertyCode,
        
        /**
        Datatype (read-only).
        */
        EDatatype,
        
        /**
        Get/Set.
        */
        EGetSet,
        
        /**
        Default Value.
        */
        EDefaultValue,
        
        /**
        Group Code.
        */
        EGroupCode,
        
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

    /**
    MTP ObjectPropDesc dataset Get/Set element values.
    */
    enum TMTPObjectPropDescGetSet
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

    /**
    MTP ObjectPropDesc dataset FormFlag element values.
    */
    enum TMTPObjectPropDescFormFlag
        {
        /** 
        None.
        */
        ENone                   = 0x00,

        /** 
        Range form.
        */
        ERangeForm              = 0x01,

        /** 
        Enumeration form.
        */
        EEnumerationForm        = 0x02,

        /** 
        DateTime form.
        */
        EDateTimeForm           = 0x03,

        /** 
        Fixed-length Array form.
        */
        EFixedLengthArrayForm   = 0x04,

        /** 
        Regular Expression form.
        */
        ERegularExpressionForm  = 0x05,

        /** 
        ByteArray form.
        */
        EByteArrayForm          = 0x06,

        /**
        * Service Object Property Extension Form
        */
        EServiceObjPropExtnForm = 0x81,
        
        /**
        * Service Method Parameter Extension Form
        */
        EServiceMethodParamExtnForm = 0x82,
        
        /**
        * Object ID Form
        */
        EObjectIDForm           = 0x83,
        
        /** 
        LongString form.
        */
        ELongStringForm         = 0xFF,
        };

    /**
    Defines the MTP ObjectPropDesc dataset meta data.
    */
    struct TPropertyInfo
        {
        /**
        The property's MTP data type identifier datacode.
        */
        TUint           iDataType;
        
        /**
        The FormFlag identifier.
        */
        TUint8          iFormFlag;
        
        /**
        The property's Get/Set flag.
        */
        TUint8          iGetSet;
        };

public:
    
    IMPORT_C static CMTPTypeObjectPropDesc* NewL();
    
    IMPORT_C static CMTPTypeObjectPropDesc* NewL(TUint16 aPropertyCode);
    IMPORT_C static CMTPTypeObjectPropDesc* NewLC(TUint16 aPropertyCode);
    
    IMPORT_C static CMTPTypeObjectPropDesc* NewL(TUint16 aPropertyCode, const MMTPType& aForm);
    IMPORT_C static CMTPTypeObjectPropDesc* NewLC(TUint16 aPropertyCode, const MMTPType& aForm);
    
    IMPORT_C static CMTPTypeObjectPropDesc* NewL(TUint16 aPropertyCode, TUint8 aFormFlag, const MMTPType* aForm);
    IMPORT_C static CMTPTypeObjectPropDesc* NewLC(TUint16 aPropertyCode, TUint8 aFormFlag, const MMTPType* aForm);
    
    IMPORT_C static CMTPTypeObjectPropDesc* NewL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm);
    IMPORT_C static CMTPTypeObjectPropDesc* NewLC(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm);
    
    IMPORT_C virtual ~CMTPTypeObjectPropDesc();   
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);

public:

    static void PropertyInfoL(TUint16 aPropertyCode, TPropertyInfo& aInfo);
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TBool ReadableElementL(TInt aElementId) const;
    TBool WriteableElementL(TInt aElementId) const;

private:

    /*
    Defines the MTP ObjectPropDesc FORM dataset. The FORM dataset is a 
    component element of the ObjectPropDesc dataset.
    @internalComponent
    */ 
    class RMTPTypeObjectPropDescForm : public RMTPType
        {
    public:

        RMTPTypeObjectPropDescForm();
        
        void SetMeta(TUint8 aFormFlag, TUint aDataType);
        
    private:
     
        MMTPType* CreateL(TUint aDataType);
        void Destroy(MMTPType* aType);
        
    private:
    
        /**
        The FORM dataset's MTP datatype code.
        */
        TUint   iDataType;
        
        /**
        The FORM dataset's FormFlag code.
        */
        TUint8  iFormFlag;
        };
        
private:

    friend MMTPType* CMTPTypeInterdependentProperties::CommitChunkL(TPtr8& aChunk);
    static CMTPTypeObjectPropDesc* NewLC();

    CMTPTypeObjectPropDesc();
    void ConstructL(TUint16 aPropertyCode, const MMTPType* aForm);
    void ConstructL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm);

    TBool HasFormField(TUint8 aFormFlag) const;
    TInt UpdateWriteSequenceErr(TInt aErr);

private: // Owned

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
        EDefaultValueChunk,
        
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
        The EDefaultValue element data chunk ID.
        */
        EIdDefaultValueChunk,
        
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
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The initialised flag indicating if construction is completeand that 
    read-only elements can no longer be modified.
    */
    TBool                                           iInitialised;
    
    /**
    The size in bytes of the first flat data chunk.
    */
    static const TUint                              KFlat1ChunkSize;
    
    /**
    The size in bytes of the second flat data chunk.
    */
    static const TUint                              KFlat2ChunkSize;
    
    /**
    The data chunk count of ObjectPropDesc datasets with an EForm of other than 
    ENone.
    */
    static const TUint                              KNumChunksWithForm;
    
    /**
    The data chunk count of ObjectPropDesc datasets with an EForm of ENone.
    */
    static const TUint                              KNumChunksWithoutForm;
    
    /**
    The flat data chunk comprising elements EPropertyCode to EGetSet.
    */    
    RMTPTypeCompoundFlatChunk                       iChunkFlat1;
    
    /**
    The EDefaultValue element data chunk.
    */
    RMTPType                                        iChunkDefaultValue;
    
    /**
    The flat data chunk comprising elements EGroupCode to EFormFlag.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat2;
    
    /**
    The EForm element data chunk.
    */
    RMTPTypeObjectPropDescForm                      iChunkForm;
    };

/** 
Defines the MTP ObjectPropDesc Enumeration FORM dataset. The Enumeration FORM 
dataset is a component element of the ObjectPropDesc dataset.
@see CMTPTypeObjectPropDesc
@publishedPartner
@released
*/ 
class CMTPTypeObjectPropDescEnumerationForm : public CMTPTypeCompoundBase
    {
        
public:
    
    IMPORT_C static CMTPTypeObjectPropDescEnumerationForm* NewL(TUint aDataType);
    IMPORT_C static CMTPTypeObjectPropDescEnumerationForm* NewLC(TUint aDataType);
    
    IMPORT_C virtual ~CMTPTypeObjectPropDescEnumerationForm();
    
    IMPORT_C void AppendSupportedValueL(const MMTPType& aValue);
    
    IMPORT_C TUint16 NumberOfValues() const;
    IMPORT_C void SupportedValueL(TUint aIndex, MMTPType& aValue) const;
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TInt ValidateChunkCount() const;
    
private:

    CMTPTypeObjectPropDescEnumerationForm(TUint aDataType);
    void ConstructL();
    
    void AppendValueChunkL();
    
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
        Data stream is in progress.
        */
        EInProgress            
        };

    /**
    The write data stream state variable. 
    */
    TUint                                       iWriteSequenceState;
    
    /**
    The NumberOfValues element metadata content.
    */
    CMTPTypeCompoundBase::TElementInfo          iInfoNumberOfValues;
    
    /**
    The SupportedValue array elements metadata content. Note that this is 
    declared mutable to allow state updates while processing a read data stream.
    */
    mutable CMTPTypeCompoundBase::TElementInfo  iInfoValue;
    
    /**
    The NumberOfValues element data chunk ID.
    */
    static const TUint                          KNumberOfValuesChunk;
    
    /**
    The SupportedValue array elements metadata content. Note that this is 
    declared mutable to allow state updates while processing a read data stream.
    */
    static const TUint                          KValueChunks;
    
    /**
    The NumberOfPropDescs element data chunk.
    */
    TMTPTypeUint16                              iChunkNumberOfValues;
    
    /**
    The SupportedValue elements array data chunks.
    */
    RArray<RMTPType>                            iChunksValue;
    };

/** 
Defines the MTP ObjectPropDesc Range FORM dataset. The Range FORM dataset is a 
component element of the ObjectPropDesc dataset.
@see CMTPTypeObjectPropDesc
@publishedPartner
@released
*/ 
class CMTPTypeObjectPropDescRangeForm : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP ObjectPropDesc Range FORM element dataset identifiers.
    */
    enum TMTPObjectPropDescElement
        {
        /**
        Minimum Value.
        */
        EMinimumValue,
        
        /**
        Maximum Value.
        */
        EMaximumValue,
        
        /**
        Step Size.
        */
        EStepSize,

        /**
        The number of dataset elements.
        */
        ENumElements,
        };
    
public:
    
    IMPORT_C static CMTPTypeObjectPropDescRangeForm* NewL(TUint aDataType);
    IMPORT_C static CMTPTypeObjectPropDescRangeForm* NewLC(TUint aDataType);
    
    IMPORT_C virtual ~CMTPTypeObjectPropDescRangeForm();  
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
private:

    CMTPTypeObjectPropDescRangeForm(TUint aDataType);
    void ConstructL();
    
private:

    /**
    The element metadata content.
    */
    mutable CMTPTypeCompoundBase::TElementInfo  iInfoBuf;

    /**
    The property's MTP data type identifier datacode.
    */
    TUint                                       iDataType;
    
    /**
    The element data chunks.
    */
    RArray<RMTPType>                            iChunks;
    };

#endif // CMTPTYPEOBJECTPROPDESC_H
