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

#ifndef CMTPTYPEDEVICEINFO_H
#define CMTPTYPEDEVICEINFO_H

#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeArray;
class CMTPTypeString;

//

/** 
Defines the MTP DeviceInfo dataset. The DeviceInfo dataset provides a 
description of the MTP device and is generated in response to a GetDeviceInfo
operation request.
@internalComponent
*/ 
class CMTPTypeDeviceInfo : public CMTPTypeCompoundBase
    {
public:

    /**
    MTP DeviceInfo dataset element identifiers.
    */
    enum TMTPDeviceInfoElement
        {
        /**
        Standard Version.
        */
        EStandardVersion,
        
        /**
        MTP Vendor Extension ID.
        */
        EMTPVendorExtensionID,
        
        /**
        MTP Version.
        */
        EMTPVersion,
        
        /**
        MTP Extensions.
        */
        EMTPExtensions, 
        
        /**
        Functional Mode.
        */
        EFunctionalMode,
        
        /**
        Operations Supported.
        */
        EOperationsSupported,
        
        /**
        Events Supported.
        */
        EEventsSupported,
        
        /**
        Device Properties Supported.
        */
        EDevicePropertiesSupported,
        
        /**
        Capture Formats.
        */
        ECaptureFormats,
        
        /**
        Playback Formats.
        */
        EPlaybackFormats,
        
        /**
        Manufacturer.
        */
        EManufacturer,
        
        /**
        Model.
        */
        EModel,  
        
        /**
        Device Version.
        */
        EDeviceVersion,
        
        /**
        Serial Number.
        */
        ESerialNumber,
        
        /**
        The number of dataset elements.
        */
        ENumElements,
        };

public:
    
    static CMTPTypeDeviceInfo* NewL();
    static CMTPTypeDeviceInfo* NewLC();
    
    virtual ~CMTPTypeDeviceInfo();   
     
public: // From MMTPType

    TUint Type() const;
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;

private:

    CMTPTypeDeviceInfo();
    void ConstructL();

    MMTPType* NewArrayChunkL(const TElementInfo& aElementInfo);
    MMTPType* NewFlatChunkL(const TElementInfo& aElementInfo);
    MMTPType* NewStringChunkL(const TElementInfo& aElementInfo);
    MMTPType* NewUInt16Chunk(const TElementInfo& aElementInfo);

private:
        
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
        EIdMTPExtensionsChunk,
        
        /**
        The EFunctionalMode element data chunk ID.
        */
        EIdFunctionalModeChunk,
        
        /**
        The EOperationsSupported element data chunk ID.
        */
        EIdOperationsSupportedChunk,
        
        /**
        The EEventsSupported element data chunk ID.
        */
        EIdEventsSupportedChunk,
        
        /**
        The EDevicePropertiesSupported element data chunk ID.
        */
        EIdDevicePropertiesSupportedChunk,
        
        /**
        The ECaptureFormats element data chunk ID.
        */
        EIdCaptureFormatsChunk,
        
        /**
        The EPlaybackFormats element data chunk ID.
        */
        EIdPlaybackFormatsChunk,
        
        /**
        The EPlaybackFormats element data chunk ID.
        */
        EIdManufacturerChunk,
        
        /**
        The EModel element data chunk ID.
        */
        EIdModelChunk,
        
        /**
        The EDeviceVersion element data chunk ID.
        */
        EIdDeviceVersionChunk,
        
        /**
        The ESerialNumber element data chunk ID.
        */
        EIdSerialNumberChunk,
        
        /**
        The number of data chunks.
        */
        EIdNumChunks
        };
    
    /**
    The dataset element metadata table content.
    */
    static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
    /**
    The flat data chunk comprising elements EStandardVersion to EMTPVersion.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat;
    
    /**
    The EFunctionalMode data chunk.
    */
    TMTPTypeUint16                                  iChunkFunctionalMode;
    
    /**
    The MTP array type data chunks (EOperationsSupported, EEventsSupported, 
    EDevicePropertiesSupported, ECaptureFormats, and EPlaybackFormats). 
    */
    RPointerArray<CMTPTypeArray>                    iArrayChunks;
    
    /**
    The MTP string type data chunks (EManufacturer, EModel, EDeviceVersion, and 
    ESerialNumber).
    */
    RPointerArray<CMTPTypeString>                   iStringChunks;
    };
    
#endif // CMTPTYPEDEVICEINFO_H
