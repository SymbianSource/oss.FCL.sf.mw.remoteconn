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

#include <mtp/tmtptyperequest.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypeobjectproplist.h>

#include "cmtpsetobjectproplist.h"
#include "cmtpdataprovidercontroller.h"


EXPORT_C MMTPRequestProcessor* CMTPSetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPSetObjectPropList* self = new (ELeave) CMTPSetObjectPropList(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
EXPORT_C CMTPSetObjectPropList::~CMTPSetObjectPropList()
	{
	iDpSingletons.Close();
	iSingleton.Close();		
	delete iPropertyList;
	delete iObjMeta;
	}
	
CMTPSetObjectPropList::CMTPSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)		
	{
	
	}
	
void CMTPSetObjectPropList::ConstructL()
	{
	iDpSingletons.OpenL(iFramework);
	iSingleton.OpenL();		
	iPropertyList = CMTPTypeObjectPropList::NewL();
	iObjMeta = CMTPObjectMetaData::NewL();
	}

void CMTPSetObjectPropList::ServiceL()
	{
	ReceiveDataL(*iPropertyList);
	}

TBool CMTPSetObjectPropList::DoHandleResponsePhaseL()
	{
    MMTPObjectMgr& objects(iFramework.ObjectMgr());
	TUint32 parameter(0);
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    const TUint count(iPropertyList->NumberOfElements());
    iPropertyList->ResetCursor();
    for (TUint i(0); ((i < count) && (responseCode == EMTPRespCodeOK)); i++)
    	{
    	CMTPTypeObjectPropListElement& element=iPropertyList->GetNextElementL(); 
    	TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);
    	TUint16 propertyCode = element.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode);
    	TUint16 dataType = element.Uint16L(CMTPTypeObjectPropListElement::EDatatype);
    	
		if (objects.ObjectOwnerId(handle) == iFramework.DataProviderId())
		    {
		    // Object is owned by the FileDp
    		responseCode = CheckPropCode(propertyCode, dataType);	
        	if (responseCode == EMTPRespCodeOK)
        		{
    			responseCode = CheckObjectHandleL(handle);
        		}
        					
     		if (responseCode == EMTPRespCodeOK)
     			{
     			switch(propertyCode)
     				{
    			case EMTPObjectPropCodeObjectFileName:
    				{
    				const TDesC& newName =  element.StringL(CMTPTypeObjectPropListElement::EValue);
					TRAPD(err, iDpSingletons.MTPUtility().RenameObjectL(handle, newName));
					if(KErrNotFound == err)
						{
						responseCode = EMTPRespCodeInvalidObjectHandle;
						}
					else if( KErrAlreadyExists == err)
						{
						responseCode = EMTPRespCodeInvalidObjectPropValue;
						}
					else if(KErrNone != err)
						{
						responseCode = EMTPRespCodeAccessDenied;
						}
    				}
    				break;
    			case EMTPObjectPropCodeName:
    				{
    				const TDesC& newName =  element.StringL(CMTPTypeObjectPropListElement::EValue);
					iObjMeta->SetDesCL( CMTPObjectMetaData::EName, newName);
					iFramework.ObjectMgr().ModifyObjectL(*iObjMeta);

    				break;
    				}
    									
    			case EMTPObjectPropCodeDateModified:
    				{
    				TTime modifiedTime;
					if( iDpSingletons.MTPUtility().MTPTimeStr2TTime(element.StringL(CMTPTypeObjectPropListElement::EValue), modifiedTime) )
						{
						iSingleton.Fs().SetModified( iObjMeta->DesC(CMTPObjectMetaData::ESuid), modifiedTime );
						}
					else
						{
						responseCode = EMTPRespCodeInvalidObjectPropValue;
						}
    				}			
    				break;
    				
    			case EMTPObjectPropCodeNonConsumable:
    				iObjMeta->SetUint( CMTPObjectMetaData::ENonConsumable, element.Uint8L(CMTPTypeObjectPropListElement::EValue));
					iFramework.ObjectMgr().ModifyObjectL(*iObjMeta);
    				break;
    				
    			case EMTPObjectPropCodeAssociationType:
    			    if (EModeMTP != iFramework.Mode())
    			    	{
    			    	responseCode = EMTPRespCodeOK;	
    			    	}
    			    else if( element.Uint16L(CMTPTypeObjectPropListElement::EValue) == EMTPAssociationTypeGenericFolder )
						{
						responseCode = EMTPRespCodeOK;			
						}
					else
						{
						responseCode = EMTPRespCodeInvalidObjectPropValue;
						}
					break;
					
				case EMTPObjectPropCodeAssociationDesc:
					if((element.Uint32L(CMTPTypeObjectPropListElement::EValue)) == 0)
						{
						responseCode = EMTPRespCodeOK;
						}
					else
						{
						responseCode = EMTPRespCodeInvalidObjectPropValue;
						}
					break;
    			default:
    				responseCode = EMTPRespCodeObjectPropNotSupported;
    				break;
     				}
     			}
     			
     		if (responseCode != EMTPRespCodeOK)
     			{
     			// Return the index of the failed property in the response.
     			parameter = i;
     			}
		    }
    	}
    
	SendResponseL(responseCode, 1, &parameter);
	return EFalse;
	}

TBool CMTPSetObjectPropList::HasDataphase() const
	{
	return ETrue;
	}

TMTPResponseCode CMTPSetObjectPropList::CheckPropCode(TUint16 aPropertyCode, TUint16 aDataType) const
	{
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	switch(aPropertyCode)
		{
		case EMTPObjectPropCodeStorageID:
		case EMTPObjectPropCodeObjectFormat:
		case EMTPObjectPropCodeObjectSize:		
		case EMTPObjectPropCodeParentObject:
		case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
		case EMTPObjectPropCodeProtectionStatus:
			responseCode = 	EMTPRespCodeAccessDenied;
			break;
							
		case EMTPObjectPropCodeObjectFileName:	
		case EMTPObjectPropCodeName:
		case EMTPObjectPropCodeDateModified:
			if (aDataType != EMTPTypeString)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
		case EMTPObjectPropCodeNonConsumable:
			if (aDataType != EMTPTypeUINT8)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
		case EMTPObjectPropCodeAssociationType:
			if (aDataType != EMTPTypeUINT16)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
		case EMTPObjectPropCodeAssociationDesc:
			if (aDataType != EMTPTypeUINT32)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
		default:
			responseCode = EMTPRespCodeInvalidObjectPropCode;
		}
	return responseCode;
	}
	
TMTPResponseCode CMTPSetObjectPropList::CheckObjectHandleL(TUint32 aHandle) const
	{
    TMTPResponseCode responseCode = EMTPRespCodeOK; 

    // Obj handle doesn't exist
	if (!(iFramework.ObjectMgr().ObjectL(aHandle, *iObjMeta)) )
		{
		responseCode = EMTPRespCodeInvalidObjectHandle;
		}	
	return responseCode;	
	}







