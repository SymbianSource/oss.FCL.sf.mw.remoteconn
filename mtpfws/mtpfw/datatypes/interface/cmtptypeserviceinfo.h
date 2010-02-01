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

#ifndef CMTPTYPESERVICEINFO_H_
#define CMTPTYPESERVICEINFO_H_


#include <e32std.h>
#include <mtp/cmtptypecompoundbase.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypearray.h>

class CMTPTypeString;
class CMTPTypeServiceInfo;
class CMTPTypeServicePropertyList;
class CMTPTypeServiceFormatList;
class CMTPTypeServiceMethodList;
class CMTPTypeServiceEventList; 
class TMTPTypeGuid;

class CMTPTypeServiceInfo : public CMTPTypeCompoundBase
    {
public:
    enum TMTPServiceInfoElement
        {
        EServiceID,
        EServiceStorageID,
        EServicePGUID,
        EServiceVersion,
        EServiceGUID,
        EServiceName,
        EServiceType,
        EBaseServiceID,

        /**
        The number of dataset elements.
        */
        ENumElements
        };
    
public:
    
    IMPORT_C static CMTPTypeServiceInfo* NewL();
    IMPORT_C static CMTPTypeServiceInfo* NewLC(); 
    IMPORT_C virtual ~CMTPTypeServiceInfo();     
    
    IMPORT_C TUint32 NumberOfUsedServices() const;
    IMPORT_C void UsedServiceL( const TUint aIndex, TMTPTypeGuid& aUsedServiceGUID  ) const;  
    IMPORT_C void AppendUsedServiceL( const TMTPTypeGuid& aElement );
    
    IMPORT_C TUint32 NumberOfServiceDataBlock() const;
    IMPORT_C void SerivceDataBlockL(  const TUint aIndex, TMTPTypeGuid& aGUID ) const;  
    IMPORT_C void AppendServiceDataBlockL(const TMTPTypeGuid& aElement );    
public:
    IMPORT_C TUint Type() const;

private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TBool WriteableElementL(TInt aElementId) const;
    TBool ReadableElementL(TInt aElementId) const;
    
private:
    CMTPTypeServiceInfo();
    void ConstructL();
    TInt UpdateWriteSequenceErr(TInt aErr);
    MMTPType* NewFlatChunkL(const TElementInfo& aElementInfo);
    
public:
    IMPORT_C CMTPTypeServicePropertyList& ServicePropList();
    IMPORT_C CMTPTypeServiceFormatList& ServiceFormatList();
    IMPORT_C CMTPTypeServiceMethodList& ServiceMethodList();
    IMPORT_C CMTPTypeServiceEventList& ServiceEventList();  
    
private:

    enum TChunkIds
        {
        /**
        The flat data chunk ID.
        */
        EIdFlat1Chunk,
        /**
        The EServiceName element data chunk ID
        */
        EIdServiceNameChunk,
        
        EIdFlat2Chunk,
        
        EIdUsedServiceGUIDChunk,
        
        EIdServicePropListChunk,
        
        EIdServicePropertyListChunk,
        
        EIdServiceFormatListChunk,
        
        EIdServiceEventListChunk,
        
        EIdServiceDataBlockChunk,

        /**
        The number of data chunks.
        */
        EIdNumChunks
        };
    
    
private:
    
        /**
        The dataset element metadata table content.
        */
        static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];
           
        /**
        The dataset element metadata table.
        */
        const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
           
        /**
        The flat data chunk comprising elements .
        */
        RMTPTypeCompoundFlatChunk                       iChunkFlat1;
        
        RMTPTypeCompoundFlatChunk                       iChunkFlat2;

        /**
        The size in bytes of the first flat data chunk.
        */
        static const TUint                              KFlat1ChunkSize;
        
        /**
        The size in bytes of the second flat data chunk.
        */
        static const TUint                              KFlat2ChunkSize;
           
        /**
        The MTP string type data chunks (EServiceName).CMTPTypeString
        */ 
        CMTPTypeString*                                 iChunkString;
        
        /**
        The MTP array type data chunks .(EUsedServiceGUID) store the used services' GUIDs
        */
        CMTPTypeArray*                                   iChunkUsedServices;    
        
        CMTPTypeArray*                                   iChunkServiceDataBlock;
    
        CMTPTypeServicePropertyList*    iServicePropList;
        CMTPTypeServiceFormatList*    iServiceFormatList;
        CMTPTypeServiceMethodList*    iServiceMethodList;
        CMTPTypeServiceEventList*    iServiceEventList; 
    
    };

#endif /*CMTPTYPESERVICEINFO_H_*/

