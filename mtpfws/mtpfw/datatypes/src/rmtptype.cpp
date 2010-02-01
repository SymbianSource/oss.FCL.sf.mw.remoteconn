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
*/

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtptypescomplex.h>
#include <mtp/mtptypessimple.h>
#include <mtp/rmtptype.h>
#include <mtp/tmtptypedatapair.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypeserviceprop.h>
#include <mtp/cmtptypeserviceformat.h>
#include <mtp/cmtptypeservicemethod.h>
#include <mtp/cmtptypeserviceevent.h>
#include <mtp/cmtptypeservicepropdesclist.h>
#include <mtp/cmtptypeservicecapabilitylist.h>

#include "mtpdatatypespanic.h"


/**
Constructor.
*/
EXPORT_C RMTPType::RMTPType() :
    iData(NULL)
    {

    }

/**
Destructor
*/
EXPORT_C RMTPType::~RMTPType()
    {
    Close();
    }
    
/**
Pushes a cleanup item for the managed data type onto the cleanup stack. The effect 
of which is to cause Close() to be called on the managed data type when 
CleanupStack::PopAndDestroy() is called at some later time.

@code
...
RMTPType x;
...
x.OpenL(EMTPTypeAUINT32);
x.CleanupClosePushL();
...
CleanupStack::PopAndDestroy();
...
@endcode
@see RMTPType::Close()
*/
EXPORT_C void RMTPType::CleanupClosePushL()
    {
    ::CleanupClosePushL(*this);
    }

/**
Releases the storage assigned to the data type.
@panic MTPDataTypes 5, if the type's data type identifier datacode is 
not supported.
*/
EXPORT_C void RMTPType::Close()
    {
    if (iData)
        {
        switch (iData->Type())
            {
        case EMTPTypeINT8:
            delete static_cast<TMTPTypeInt8*> (iData);
            break;

        case EMTPTypeUINT8:
            delete static_cast<TMTPTypeUint8*> (iData);
            break;
            
        case EMTPTypeINT16:
            delete static_cast<TMTPTypeInt16*> (iData);
            break;
            
        case EMTPTypeUINT16:
            delete static_cast<TMTPTypeUint16*> (iData);
            break;
            
        case EMTPTypeINT32:
            delete static_cast<TMTPTypeInt32*> (iData);
            break;
            
        case EMTPTypeUINT32:
            delete static_cast<TMTPTypeUint32*> (iData);
            break;
            
        case EMTPTypeINT64:
            delete static_cast<TMTPTypeInt64*> (iData);
            break;
            
        case EMTPTypeUINT64:
            delete static_cast<TMTPTypeUint64*> (iData);
            break;
            
        case EMTPTypeINT128:
            delete static_cast<TMTPTypeInt128*> (iData);
            break;
            
        case EMTPTypeUINT128:
            delete static_cast<TMTPTypeUint128*> (iData);
            break;
            
        case EMTPTypeAINT8:
        case EMTPTypeAUINT8:
        case EMTPTypeAINT16:
        case EMTPTypeAUINT16:
        case EMTPTypeAINT32:
        case EMTPTypeAUINT32:
        case EMTPTypeAINT64:
        case EMTPTypeAUINT64:
        case EMTPTypeAINT128:
        case EMTPTypeAUINT128:
            delete static_cast<CMTPTypeArray*> (iData);
            break;
            
        case EMTPTypeString:
            delete static_cast<CMTPTypeString*> (iData);
            break;
            
        default: 
            Destroy(iData);
            break;
            }
            
        iData = NULL;
        }
    }

/**
Allocates storage for the data type. By default all MTP simple (signed and 
unsigned integer), array (signed and unsigned integer), and string types are 
supported. Support for any other data types must be provided by a derived 
subclass which implements suitable CreateL and Destroy methods. This method 
will release any pre-existing allocated storage.
@param aDataType The type's data type identifier datacode.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 5, if the type's data type identifier datacode is 
not supported.
@see CreateL
@see Destroy
*/
EXPORT_C void RMTPType::OpenL(TUint aDataType)
    {
    if (iData)
        {
        Close();
        }
        
    iData = AllocL(aDataType);
    if (!iData)
        {
        // Type is not supported. Pass to the derived class (if any).
        iData = CreateL(aDataType);
        }
    }
    
EXPORT_C MMTPType& RMTPType::Data()
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return *iData;
    }
    
EXPORT_C TInt RMTPType::FirstReadChunk(TPtrC8& aChunk) const
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->FirstReadChunk(aChunk);
    }

EXPORT_C TInt RMTPType::NextReadChunk(TPtrC8& aChunk) const
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->NextReadChunk(aChunk);
    }

EXPORT_C TInt RMTPType::FirstWriteChunk(TPtr8& aChunk)
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->FirstWriteChunk(aChunk);
    }
    
EXPORT_C TInt RMTPType::NextWriteChunk(TPtr8& aChunk)
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->NextWriteChunk(aChunk);
    }

EXPORT_C TUint64 RMTPType::Size() const
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->Size();
    }

EXPORT_C TUint RMTPType::Type() const
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->Type();
    }

EXPORT_C TBool RMTPType::CommitRequired() const
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->CommitRequired();
    }

EXPORT_C MMTPType* RMTPType::CommitChunkL(TPtr8& aChunk)
    {
    __ASSERT_ALWAYS(iData, Panic(EMTPTypeBadStorage));
    return iData->CommitChunkL(aChunk);
    }
    
EXPORT_C TInt RMTPType::Validate() const
    {
    TInt ret(KMTPDataTypeInvalid);
    if (iData)
        {
        ret = iData->Validate();
        }
    return ret;
    }
    
/**
Allocates storage for non-default data types. Non-default data types can be 
supported by a derived class which overides both this and the Destroy methods.
@param aDataType The type's data type identifier datacode.
@return A pointer to the allocated data type storage. Ownership IS transferred.
@leave KMTPDataTypeInvalid, if the type's data type identifier datacode is 
not supported.
@leave One of the system wide error codes, if a processing failure occurs.
@see Destroy
*/
EXPORT_C MMTPType* RMTPType::CreateL(TUint aDataType)
    {
    MMTPType* type(AllocL(aDataType));
    if (!type)
        {
        User::Leave(KMTPDataTypeInvalid);   
        }

    return type;  
    }

/**
Releases the storage assigned to the data type. Non-default data types can be 
supported by a derived class which overides both this and the CreateL methods.
@panic MTPDataTypes 5, if the type's data type identifier datacode is 
not supported.
@see CreateL
*/
EXPORT_C void RMTPType::Destroy(MMTPType* /*aType*/)
    {
    Panic(EMTPTypeNotSupported);
    }
    
MMTPType* RMTPType::AllocL(const TUint aDataType)
    {
    MMTPType* type(0);
    switch (aDataType)
        {
    case EMTPTypeINT8:
        type = new(ELeave) TMTPTypeInt8();
        break;
        
    case EMTPTypeUINT8:
        type = new(ELeave) TMTPTypeUint8();
        break;
        
    case EMTPTypeINT16:
        type = new(ELeave) TMTPTypeInt16();
        break;
        
    case EMTPTypeUINT16:
        type = new(ELeave) TMTPTypeUint16();
        break;
        
    case EMTPTypeINT32:
        type = new(ELeave) TMTPTypeInt32();
        break;
        
    case EMTPTypeUINT32:
        type = new(ELeave) TMTPTypeUint32();
        break;
        
    case EMTPTypeINT64:
        type = new(ELeave) TMTPTypeInt64();
        break;
        
    case EMTPTypeUINT64:
        type = new(ELeave) TMTPTypeUint64();
        break;
        
    case EMTPTypeINT128:
        type = new(ELeave) TMTPTypeInt128();
        break;
        
    case EMTPTypeUINT128:
        type = new(ELeave) TMTPTypeUint128();
        break;
        
    case EMTPTypeAINT8:
    case EMTPTypeAUINT8:
    case EMTPTypeAINT16:
    case EMTPTypeAUINT16:
    case EMTPTypeAINT32:
    case EMTPTypeAUINT32:
    case EMTPTypeAINT64:
    case EMTPTypeAUINT64:
    case EMTPTypeAINT128:
    case EMTPTypeAUINT128:
        type = CMTPTypeArray::NewL(aDataType);
        break;
        
    case EMTPTypeString:
        type = CMTPTypeString::NewL();
        break;
    case EMTPTypeDataPair:
        {
        type = new(ELeave) TMTPTypeDataPair();
        }
        break;
    case EMTPTypeObjectPropDescDataset:
    	type = CMTPTypeObjectPropDesc::NewL();
    	break;
    case EMTPTypeFormatCapabilityDataset:
    	type = CMTPTypeFormatCapability::NewL();
    	break;
    case EMTPTypeServicePropDesc:
    	type = CMTPTypeServicePropDesc::NewL();
    	break;
    case EMTPTypeServicePropertyElementDataset:
    	type = CMTPTypeServicePropertyElement::NewL();
    	break;
    case EMTPTypeServiceFormatElementDataset:
    	type = CMTPTypeServiceFormatElement::NewL();
    	break;
    case EMTPTypeServiceMethodElementDataset:
    	type = CMTPTypeServiceMethodElement::NewL();
    	break;
    case EMTPTypeServiceEventElementDataset:
    	type = CMTPTypeServiceEventElement::NewL();
    	break; 
    default: 
        break;
        }
        
    return type;
    }

void RMTPType::Destroy(const TUint aDataType, MMTPType* aData)
    {
    switch ( aDataType )
        {
    case EMTPTypeINT8:
        delete static_cast<TMTPTypeInt8*> (aData);
        break;

    case EMTPTypeUINT8:
        delete static_cast<TMTPTypeUint8*> (aData);
        break;
        
    case EMTPTypeINT16:
        delete static_cast<TMTPTypeInt16*> (aData);
        break;
        
    case EMTPTypeUINT16:
        delete static_cast<TMTPTypeUint16*> (aData);
        break;
        
    case EMTPTypeINT32:
        delete static_cast<TMTPTypeInt32*> (aData);
        break;
        
    case EMTPTypeUINT32:
        delete static_cast<TMTPTypeUint32*> (aData);
        break;
        
    case EMTPTypeINT64:
        delete static_cast<TMTPTypeInt64*> (aData);
        break;
        
    case EMTPTypeUINT64:
        delete static_cast<TMTPTypeUint64*> (aData);
        break;
        
    case EMTPTypeINT128:
        delete static_cast<TMTPTypeInt128*> (aData);
        break;
        
    case EMTPTypeUINT128:
        delete static_cast<TMTPTypeUint128*> (aData);
        break;
        
    case EMTPTypeAINT8:
    case EMTPTypeAUINT8:
    case EMTPTypeAINT16:
    case EMTPTypeAUINT16:
    case EMTPTypeAINT32:
    case EMTPTypeAUINT32:
    case EMTPTypeAINT64:
    case EMTPTypeAUINT64:
    case EMTPTypeAINT128:
    case EMTPTypeAUINT128:
        delete static_cast<CMTPTypeArray*> (aData);
        break;
    case EMTPTypeString:
        delete static_cast<CMTPTypeString*> (aData);
        break;
    case EMTPTypeDataPair:
        delete static_cast<TMTPTypeDataPair*>(aData);
        break;
    case EMTPTypeObjectPropDescDataset:
        delete static_cast<CMTPTypeObjectPropDesc*> (aData);
        break;
    case EMTPTypeFormatCapabilityDataset:
        delete static_cast<CMTPTypeFormatCapability*> (aData);
        break;
    case EMTPTypeServicePropDesc:
        delete static_cast<CMTPTypeServicePropDesc*> (aData);
        break;
    case EMTPTypeServicePropertyElementDataset:
        delete static_cast<CMTPTypeServicePropertyElement*> (aData);
        break;
    case EMTPTypeServiceFormatElementDataset:
        delete static_cast<CMTPTypeServiceFormatElement*> (aData);
        break;
    case EMTPTypeServiceMethodElementDataset:
        delete static_cast<CMTPTypeServiceMethodElement*> (aData);
        break;
    case EMTPTypeServiceEventElementDataset:
        delete static_cast<CMTPTypeServiceEventElement*> (aData);
        break;     
            
    default: 
        Panic(EMTPTypeNotSupported);
        break;
        }

    }
