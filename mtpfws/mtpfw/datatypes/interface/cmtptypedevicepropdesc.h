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
 @internalComponent
*/

#ifndef CMTPTYPEDEVICEPROPDESC_H
#define CMTPTYPEDEVICEPROPDESC_H

#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>

/** 
Defines the MTP DevicePropDesc dataset. The DevicePropDesc dataset describes 
an MTP device property, and is generated in response to a GetDevicePropDesc 
operation request.
@internalComponent
*/ 
class CMTPTypeDevicePropDesc : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP DevicePropDesc dataset element identifiers.
    */
    enum TMTPDevicePropDescElement
        {
        /**
        Property Code (read-only).
        */
        EDevicePropertyCode,
        
        /**
        Datatype (read-only).
        */
        EDatatype,
        
        /**
        Get/Set.
        */
        EGetSet,
        
        /**
        Factory Default Value.
        */
        EFactoryDefaultValue,
        
        /**
        Factory Default Value.
        */
        ECurrentValue,
        
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
    MTP DevicePropDesc dataset Get/Set element values.
    */
    enum TMTPDevicePropDescGetSet
        {
        /** 
        Get.
        */
        EReadOnly   = 0x00,

        /** 
        Get/Set.
        */
        EReadWrite  = 0x01,
        };

    /**
    MTP DevicePropDesc dataset FormFlag element values.
    */
    enum TMTPDevicePropDescFormFlag
        {
        /** 
        None.
        */
        ENone               = 0x00,

        /** 
        Range form.
        */
        ERangeForm          = 0x01,

        /** 
        Enumeration form.
        */
        EEnumerationForm    = 0x02,
        };

    /**
    Defines the MTP DevicePropDesc dataset meta data.
    */
    struct TPropertyInfo
        {
        /**
        The property's MTP data type identifier datacode.
        */
        TUint   iDataType;
        
        /**
        The FormFlag identifier.
        */
        TUint8  iFormFlag;
        
        /**
        The property's Get/Set flag.
        */
        TUint8  iGetSet;
        };

public:
    
    IMPORT_C static CMTPTypeDevicePropDesc* NewL(TUint16 aPropertyCode);
    IMPORT_C static CMTPTypeDevicePropDesc* NewLC(TUint16 aPropertyCode);
    
    IMPORT_C static CMTPTypeDevicePropDesc* NewL(TUint16 aPropertyCode, const MMTPType& aForm);
    IMPORT_C static CMTPTypeDevicePropDesc* NewLC(TUint16 aPropertyCode, const MMTPType& aForm);
    
    IMPORT_C static CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewL(TUint16 aPropertyCode, TUint8 aGetSet, TUint8 aFormFlag, const MMTPType* aForm);
    IMPORT_C static CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC(TUint16 aPropertyCode, TUint8 aGetSet, TUint8 aFormFlag, const MMTPType* aForm);
    
    IMPORT_C static CMTPTypeDevicePropDesc* NewL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm);
    IMPORT_C static CMTPTypeDevicePropDesc* NewLC(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm);
    
    IMPORT_C virtual ~CMTPTypeDevicePropDesc();   
     
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
    Defines the MTP DevicePropDesc FORM dataset. The FORM dataset is a 
    component element of the DevicePropDesc dataset.
    @internalComponent
    */ 
    class RMTPTypeDevicePropDescForm : public RMTPType
        {
    public:

        RMTPTypeDevicePropDescForm();
        
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

    static CMTPTypeDevicePropDesc* NewLC();

    CMTPTypeDevicePropDesc();
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
        EWriteIdle,
        
        /**
        Streaming the first flat data chunk.
        */
        EWriteFlat1Chunk,
        
        /**
        Streaming the EFactoryDefaultValue data chunk..
        */
        EWriteDefaultValueChunk,
        
        /**
        Streaming the ECurrentValue data chunk..
        */
        EWriteCurrentValueChunk,
        
        /**
        Streaming the second flat data chunk.
        */
        EWriteFormFlagChunk,
        
        /**
        Streaming the EForm data chunk.
        */
        EWriteFormChunk 
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
        The EFactoryDefaultValue element data chunk ID.
        */
        EIdDefaultValueChunk,
        
        /**
        The ECurrentValue element data chunk ID.
        */
        EIdCurrentValueChunk,
        
        /**
        The second flat data chunk ID.
        */
        EIdFormFlagChunk,
        
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
    DevicePropDesc datasets with an EForm of ENone, otherwise this will be 
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
    The data chunk count of DevicePropDesc datasets with an EForm of other than 
    ENone.
    */
    static const TUint                              KNumChunksWithForm;
    
    /**
    The data chunk count of DevicePropDesc datasets with an EForm of ENone.
    */
    static const TUint                              KNumChunksWithoutForm;
    
    /**
    The flat data chunk comprising elements EPropertyCode to EGetSet.
    */    
    RMTPTypeCompoundFlatChunk                       iChunkFlat1;
    
    /**
    The EFactoryDefaultValue element data chunk.
    */
    RMTPType                                        iChunkDefaultValue;
    
    /**
    The ECurrentValue element data chunk.
    */
    RMTPType                                        iChunkCurrentValue;
    
    /**
    The EFormFlag data chunk.
    */
    TMTPTypeUint8                                   iChunkFormFlag;
    
    /**
    The EForm element data chunk.
    */
    RMTPTypeDevicePropDescForm                      iChunkForm;
    };

/** 
Defines the MTP DevicePropDesc Enumeration FORM dataset. The Enumeration FORM 
dataset is a component element of the DevicePropDesc dataset.
@see CMTPTypeDevicePropDesc
@internalComponent
*/ 
class CMTPTypeDevicePropDescEnumerationForm : public CMTPTypeCompoundBase
    {
        
public:
    
    IMPORT_C static CMTPTypeDevicePropDescEnumerationForm* NewL(TUint aDataType);
    IMPORT_C static CMTPTypeDevicePropDescEnumerationForm* NewLC(TUint aDataType);
    
    IMPORT_C virtual ~CMTPTypeDevicePropDescEnumerationForm();
    
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

    CMTPTypeDevicePropDescEnumerationForm(TUint aDataType);
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
Defines the MTP DevicePropDesc Range FORM dataset. The Range FORM dataset is a 
component element of the DevicePropDesc dataset.
@see CMTPTypeDevicePropDesc
@internalComponent
*/ 
class CMTPTypeDevicePropDescRangeForm : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP DevicePropDesc Range FORM element dataset identifiers.
    */
    enum TMTPDevicePropDescElement
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
    
    IMPORT_C static CMTPTypeDevicePropDescRangeForm* NewL(TUint aDataType);
    IMPORT_C static CMTPTypeDevicePropDescRangeForm* NewLC(TUint aDataType);
    
    IMPORT_C virtual ~CMTPTypeDevicePropDescRangeForm();  
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    
private:

    CMTPTypeDevicePropDescRangeForm(TUint aDataType);
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

#endif // CMTPTYPEDEVICEPROPDESC_H
