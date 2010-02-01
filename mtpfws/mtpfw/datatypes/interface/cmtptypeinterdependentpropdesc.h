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

#ifndef CMTPTYPEINTERDEPENDENTPROPDESC_H
#define CMTPTYPEINTERDEPENDENTPROPDESC_H

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeInterdependentProperties;
class CMTPTypeObjectPropDesc;

/** 
Defines the MTP InterdependentPropDesc dataset. The InterdependentPropDesc 
implements an array of MTP ObjectPropDesc arrays, each describing a permissable
collection of ranges. Each array of MTP ObjectPropDesc datasets gives one 
possible definition for the interdependent properties that are contained in 
that array; properties not found in that array are constrained only by the 
usual ObjectPropDesc datasets. This dataset is used in the 
GetInterdependentPropDesc request.
@publishedPartner
@released 
*/ 
class CMTPTypeInterdependentPropDesc : public CMTPTypeCompoundBase
    {
public:
    
    IMPORT_C static CMTPTypeInterdependentPropDesc* NewL();
    IMPORT_C static CMTPTypeInterdependentPropDesc* NewLC();

    IMPORT_C virtual ~CMTPTypeInterdependentPropDesc();  
    
    IMPORT_C void AppendL(CMTPTypeInterdependentProperties* aInterdependentProperties);
    
    IMPORT_C TUint32 NumberOfInterdependencies() const;
    IMPORT_C CMTPTypeInterdependentProperties& InterdependentProperties(TUint aIndex) const;  
     
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

    CMTPTypeInterdependentPropDesc();
    void ConstructL();
    
    void AppendElementChunkL(CMTPTypeInterdependentProperties* aElement);
    TInt UpdateWriteSequenceErr(TInt aErr);

private: // Owned

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
    The NumberOfInterdependencies element data chunk ID.
    */
    static const TUint                              KNumberOfInterdependenciesChunk;
    
    /**
    The InterdependentProperties array starting element data chunk ID.
    */
    static const TUint                              KInterdependentPropertiesChunks;

    /**
    The write data stream error state. 
    */
    TInt                                            iWriteSequenceErr;
    
    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;
    
    /**
    The NumberOfInterdependencies element metadata content.
    */
    CMTPTypeCompoundBase::TElementInfo              iInfoNumberOfInterdependencies;
    
    /**
    The InterdependentProperties elements array metadata content. Note that this is 
    declared mutable to allow state updates while processing a read data stream.
    */
    mutable CMTPTypeCompoundBase::TElementInfo      iInfoInterdependentProperties;
    
    /**
    The NumberOfInterdependencies data chunk.
    */
    TMTPTypeUint32                                  iChunkNumberOfInterdependencies;
    
    /**
    The InterdependentProperties elements array data chunks.
    */
    RPointerArray<CMTPTypeInterdependentProperties> iChunksInterdependentProperties;
    };

/**
Defines the MTP InterdependentPropDesc array element dataset. The 
InterdependentPropDesc array element dataset is itself an array of MTP 
ObjectPropDesc datasets that specifies one possible definition for the 
interdependent properties that are contained in that array.
*/
class CMTPTypeInterdependentProperties : public CMTPTypeCompoundBase
    {
public:
    
    IMPORT_C static CMTPTypeInterdependentProperties* NewL();
    IMPORT_C static CMTPTypeInterdependentProperties* NewLC();

    IMPORT_C virtual ~CMTPTypeInterdependentProperties();  
    
    IMPORT_C void AppendL(CMTPTypeObjectPropDesc* aObjectPropDesc);
    
    IMPORT_C TUint32 NumberOfPropDescs() const;
    IMPORT_C CMTPTypeObjectPropDesc& ObjectPropDesc(TUint aIndex) const;  
     
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

    CMTPTypeInterdependentProperties();
    void ConstructL();
    
    void AppendElementChunkL(CMTPTypeObjectPropDesc* aElement);
    TInt UpdateWriteSequenceErr(TInt aErr);

private: // Owned

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
    The NumberOfPropDescs element data chunk ID.
    */
    static const TUint                          KNumberOfPropDescsChunk;
    
    /**
    The ObjectPropDesc array starting element data chuck ID.
    */
    static const TUint                          KObjectPropDescChunks;

    /**
    The write data stream error state. 
    */
    TInt                                        iWriteSequenceErr;
    
    /**
    The write data stream state variable. 
    */
    TUint                                       iWriteSequenceState;
    
    /**
    The NumberOfPropDescs element metadata content.
    */
    CMTPTypeCompoundBase::TElementInfo          iInfoNumberOfPropDescs;
    
    /**
    The ObjectPropDesc array elements metadata content. Note that this is 
    declared mutable to allow state updates while processing a read data stream.
    */
    mutable CMTPTypeCompoundBase::TElementInfo  iInfoObjectPropDesc;
    
    /**
    The NumberOfPropDescs element data chunk.
    */
    TMTPTypeUint32                              iChunkNumberOfPropDescs;
    
    /**
    The ObjectPropDesc elements array data chunks.
    */
    RPointerArray<CMTPTypeObjectPropDesc>       iChunksObjectPropDesc;
    };

#endif // CMTPTYPEINTERDEPENDENTPROPDESC_H
