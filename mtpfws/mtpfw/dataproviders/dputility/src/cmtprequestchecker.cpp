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

#include <f32file.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>

#include "cmtprequestchecker.h"
#include "rmtpdpsingletons.h"
#include "cmtpfsexclusionmgr.h"

static const TInt KMTPRequestCheckerHandleGranularity = 2;
__FLOG_STMT(_LIT8(KComponent,"MTPRequestChecker");)

/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection object
@return a pointer to the created request checker object
*/   
EXPORT_C CMTPRequestChecker* CMTPRequestChecker::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPRequestChecker* self = new (ELeave) CMTPRequestChecker(aFramework, aConnection);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPRequestChecker::~CMTPRequestChecker()
	{
	iHandles.Close();
	iObjectArray.ResetAndDestroy();
	__FLOG_CLOSE;
	}

/**
Verfiy the request.  It check the request header first (session id and transaction code), and check for special values, last
it iterates through the verification elements for checking individul parameters in the request

@param aRequest	The request object to verify
@param aCount	The number of verification elements
@param aElementInfo	The array of verification elements
@return reponse code to return to the initiator
*/	
EXPORT_C TMTPResponseCode CMTPRequestChecker::VerifyRequestL(const TMTPTypeRequest& aRequest, TInt aCount, const TMTPRequestElementInfo* aElementInfo)
	{
   	TMTPResponseCode result = EMTPRespCodeOK;
   	iHandles.Close();
   	iObjectArray.ResetAndDestroy();
    
	result = CheckRequestHeader(aRequest);
	
	for (TInt i = 0; i < aCount && EMTPRespCodeOK == result; i++)
		{
		TUint32 parameter = aRequest.Uint32(aElementInfo[i].iElementIndex);
		if (!IsSpecialValue(parameter, aElementInfo[i]))
			{
			switch (aElementInfo[i].iElementType)
				{
				case EMTPElementTypeSessionID:
					result = VerifySessionId(parameter, aElementInfo[i]);
					break;
				case EMTPElementTypeObjectHandle:
					result = VerifyObjectHandleL(parameter, aElementInfo[i]);
					break;
				case EMTPElementTypeStorageId:
					result = VerifyStorageIdL(parameter, aElementInfo[i]);
					break;
				case EMTPElementTypeFormatCode:
					result = VerifyFormatCode(parameter, aElementInfo[i]);
					break;				
				default:
					User::Invariant();  // Should never run
					break;				
				}			
			}
	    }
		
    return result;     
	
	}

/**
Return the object info for the handle.  This is to remove extra expensive DMBS retrieval operations.

@param aHandle	the handle of the object requested
@return an object info for the handle
*/
EXPORT_C CMTPObjectMetaData* CMTPRequestChecker::GetObjectInfo(TUint32 aHandle) const
	{
	CMTPObjectMetaData* result = NULL;
	TInt count = iHandles.Count();
	for(TInt i = 0; i < count; i++)
		{
		if (iHandles[i] == aHandle)
			{
			result = iObjectArray[i];
			break;
			}
		}
	return result;
	}

/**
Check the request header portion (session Id and transaction code)
@param aRequest	the request object to check
@return repsonse code to return to initiator
*/
TMTPResponseCode CMTPRequestChecker::CheckRequestHeader(const TMTPTypeRequest& aRequest) const
	{
    TMTPResponseCode ret = EMTPRespCodeOK; 
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
  	TUint32 sessionId = aRequest.Uint32(TMTPTypeRequest::ERequestSessionID);
	TUint32 transactionCode = aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID);
	
	if (operationCode == EMTPOpCodeCloseSession || operationCode == EMTPOpCodeResetDevice)
		{
		if (sessionId != 0)
			{
			ret = EMTPRespCodeInvalidParameter;
			}
		}
	else
		{		
	    // requests that are valid when there's no opened session.
	    if (sessionId == 0)
	        {
	        switch (operationCode)
	            {            	
	            case EMTPOpCodeGetDeviceInfo:
	            case EMTPOpCodeOpenSession: 
	                {
	                // Transaction id must be 0 when called out side an active session.
	                if (transactionCode != 0)
	                    {
	                    ret = EMTPRespCodeInvalidTransactionID;
	                    }

	                }
	                break;
	            
	            default:
	                ret = EMTPRespCodeSessionNotOpen;
	                break;
	            }
	        }
	    else if (!iConnection.SessionWithMTPIdExists(sessionId))
	        { 
	        ret = EMTPRespCodeSessionNotOpen;
	        }
		}
    return ret;	
	}

/**
Check the session id in the request parameter (NOTE the session id is different from the one in the request header),
this usually only applies to the OpenSession request
@param aSessionId	Session id of the request.
@param aElementInfo ElementInfo data array to check against.
@return repsonse code to return to initiator
*/	
TMTPResponseCode CMTPRequestChecker::VerifySessionId(TUint32 aSessionId, const TMTPRequestElementInfo& /*aElementInfo*/) const
	{
    TMTPResponseCode ret = EMTPRespCodeOK; 
    
    if (aSessionId != 0)   
    	{
    	if (iConnection.SessionWithMTPIdExists(aSessionId))
			{
			ret = EMTPRespCodeSessionAlreadyOpen;			
			}
    	}
    else
    	{
    	ret = EMTPRespCodeInvalidParameter;
    	}
		
    return ret;	
	}

/**
Check the object handle in the request parameter, whether the handle is in the object store, read/write, file/dir
@param aHandle	Object handle to be checked.
@param aElementInfo Element info array to be checked against.
@return repsonse code to return to initiator
*/		
TMTPResponseCode CMTPRequestChecker::VerifyObjectHandleL(TUint32 aHandle, const TMTPRequestElementInfo& aElementInfo)
	{
	__FLOG_VA((_L8("VerifyObjectHandleL entry with handle 0x%08X"), aHandle));
    TMTPResponseCode ret = EMTPRespCodeOK; 

	CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
	TBool result(iFramework.ObjectMgr().ObjectL(aHandle, *object));
	iObjectArray.AppendL(object);
	CleanupStack::Pop(object);
	iHandles.AppendL(aHandle);
		
	// Obj handle exists
	if (result)
		{
	    TUint storageID = object->Uint(CMTPObjectMetaData::EStorageId);
		CMTPStorageMetaData* storageMetaData = (CMTPStorageMetaData *)& iFramework.StorageMgr().StorageL(storageID);
		if (storageMetaData->Uint(CMTPStorageMetaData::EStorageSystemType) != CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
			{
			return ret;
			}
		
		const TDesC& suid(object->DesC(CMTPObjectMetaData::ESuid));
		TEntry entry;
        TInt err = iFramework.Fs().Entry(suid, entry);
        
        if ( (object->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation) && 
             (object->Uint(CMTPObjectMetaData::EFormatSubCode) != EMTPAssociationTypeGenericFolder ) )
            {
            // Special association type .. not always present on the filesystem.
            return ret;
            }   
        else
            {
            User::LeaveIfError(err);
            }
		
		if (aElementInfo.iElementAttr & EMTPElementAttrWrite)
        	{
        	if (entry.IsReadOnly())
        		{
        		ret = EMTPRespCodeObjectWriteProtected;
        		}
        	}
        	
        //((EMTPRespCodeOK == ret) && (aElementInfo.iElementAttr & EMTPElementAttrFileOrDir)) is
        // covered implicitly here, EMTPRespCodeOK will be returned. It is a valid case for an object to be either a folder or file 
        // for certain operation's request parameter, for instance the first parameter of copyObject or
        // moveObject can be either a file or a directory.
        
		// Other cases.
        if ((EMTPRespCodeOK == ret) && (aElementInfo.iElementAttr & EMTPElementAttrFile))
	        	{
	        	if (entry.IsDir())
	        		{
	        		ret = EMTPRespCodeInvalidObjectHandle;
	        		}
	        	}

       	if ((EMTPRespCodeOK == ret) && (aElementInfo.iElementAttr & EMTPElementAttrDir))
	       	{
	       	if (!entry.IsDir())
	       		{
	       		ret = EMTPRespCodeInvalidParentObject;
	       		}        	
	       	}
        	       		         
		 }
	else
		{
		__FLOG(_L8("Object does not exist."));
		ret = EMTPRespCodeInvalidObjectHandle;
		}
	__FLOG_VA((_L8("VerifyObjectHandleL exit with repsonse code 0x%04X"), ret)); 
	return ret;	
	}

/**
Check the storage id parameter in the request, read/write attributes
@param aStorageId	Storage id to be checked.
@param aElementInfo Element info array to be checked against.
@return repsonse code to return to initiator
*/			
TMTPResponseCode CMTPRequestChecker::VerifyStorageIdL(TUint32 aStorageId, const TMTPRequestElementInfo& aElementInfo) const
	{
	MMTPStorageMgr& mgr(iFramework.StorageMgr());
    TMTPResponseCode ret(EMTPRespCodeOK);
    if (!mgr.ValidStorageId(aStorageId))
        {
        ret = EMTPRespCodeInvalidStorageID;
        }
    else if (!mgr.LogicalStorageId(aStorageId))
        {
        ret = EMTPRespCodeStoreNotAvailable;
        }
    else 
        {
        TInt drive(mgr.DriveNumber(aStorageId));
        // StorageIDs which are not system owned do not correspond to drives.
        if (drive != KErrNotFound)
            {
    		TDriveInfo info;
    		User::LeaveIfError(iFramework.Fs().Drive(info, drive));
    		if (info.iType == EMediaNotPresent)
    		    {
    		    /* 
    		    Race conditions between media ejection and request processing
    		    may result in a previously valid storage ID no longer being 
    		    available.
    		    */
    		    ret = EMTPRespCodeStoreNotAvailable;
    		    }
    		else if ((aElementInfo.iElementAttr & EMTPElementAttrWrite) &&
    		            ((info.iMediaAtt & KMediaAttWriteProtected) ||
    		             (info.iDriveAtt & KDriveAttRom)))
    		    {
        		ret = EMTPRespCodeStoreReadOnly;
    		    }
            }
        }
    return ret; 	
	}
	
/**
Check the format code parameter in the request,
@param aStorageId	aFormatCode to be checked.
@param aElementInfo Element info array to be checked against.
@return repsonse code to return to initiator
*/
TMTPResponseCode CMTPRequestChecker::VerifyFormatCode(TUint32 aFormatCode, const TMTPRequestElementInfo& aElementInfo) const
	{
    TMTPResponseCode ret = EMTPRespCodeInvalidObjectFormatCode; 

	if (aElementInfo.iElementAttr == EMTPElementAttrFormatEnums)
		{
		TUint32* formatArray = (TUint32*)(aElementInfo.iValue1);
		TUint32 i = 0;
		for(i = 0; i < aElementInfo.iValue2; i++)
			{
			if (aFormatCode == formatArray[i])
				{
				ret = EMTPRespCodeOK;
				break;
				}
			}
		}
	else if (aFormatCode >= EMTPFormatCodePTPStart && aFormatCode <= EMTPFormatCodeMTPEnd)
		{
		ret = EMTPRespCodeOK;
		}
		
    return ret;	
	}

/**
Check if the parameter is one of the special values
@param aParameter	The parameter value in the request
@param aElementInfo	The ElementInfo for the parameter
@return ETrue if the parameter is one of the special values, otherwise, EFalse
*/
TBool CMTPRequestChecker::IsSpecialValue(TUint32 aParameter, const TMTPRequestElementInfo& aElementInfo) const
	{
	TBool result = EFalse;
	switch(aElementInfo.iCount)
		{
		case 1:
			result = (aParameter == aElementInfo.iValue1);
			break;
		case 2:
			result = (aParameter == aElementInfo.iValue1 || aParameter == aElementInfo.iValue2);
			break;
		default:
			break;			
		}
	return result;
	}

/**
Standard c++ constructor
*/	
CMTPRequestChecker::CMTPRequestChecker(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	:iFramework(aFramework), 
	iConnection(aConnection),
	iHandles(KMTPRequestCheckerHandleGranularity),
	iObjectArray(KMTPRequestCheckerHandleGranularity)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	}



