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

#include <bautils.h>

#include <mtp/cmtptypearray.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpdeleteobject.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeleteObject");)

/**
Verification data for the DeleteObject request
*/
const TMTPRequestElementInfo KMTPDeleteObjectPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrWrite, 1, KMTPHandleAll, 0},
    };
    
    
/**
Standard c++ constructor
*/    
CMTPDeleteObject::CMTPDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPDeleteObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPDeleteObjectPolicy)
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPDeleteObject - Entry"));
    __FLOG(_L8("CMTPDeleteObject - Exit"));
    }


/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
EXPORT_C MMTPRequestProcessor* CMTPDeleteObject::NewL(MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection)
    {
    CMTPDeleteObject* self = new (ELeave) CMTPDeleteObject(aFramework, aConnection);
    return self;
    }

/**
Destructor
*/    
EXPORT_C CMTPDeleteObject::~CMTPDeleteObject()
    {
    __FLOG(_L8("~CMTPDeleteObject - Entry"));
    __FLOG(_L8("~CMTPDeleteObject - Exit"));
    __FLOG_CLOSE;
    }

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/ 

 
TMTPResponseCode CMTPDeleteObject::CheckRequestL()
	{
    __FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode result(EMTPRespCodeOK);
	if (IsStoreReadOnlyL(Request().Uint32(TMTPTypeRequest::ERequestParameter1)))
		{
		result = EMTPRespCodeStoreReadOnly;
		}
	else
		{
		result = CMTPRequestProcessor::CheckRequestL();
		}		
    __FLOG(_L8("CheckRequestL - Exit"));
	return result;	
	} 
	
void CMTPDeleteObject::DeleteFolderOrFileL(CMTPObjectMetaData* aMeta)
    {
    __ASSERT_DEBUG(aMeta, Panic(EMTPDpObjectNull));
    if (IsFolderObject(*aMeta))
        {
        __FLOG( _L8("Delete the folder itself which is empty ") );
        DeleteFolderL(aMeta);
        }
    else
        {
        __FLOG(_L8("Going to delete a file.")); 
        DeleteFileL(aMeta);
        }
    ProcessFinalPhaseL();
    }

void CMTPDeleteObject::DeleteFolderL(CMTPObjectMetaData* aMeta)
    {
    TParsePtrC fileNameParser(aMeta->DesC(CMTPObjectMetaData::ESuid));    
    TInt err = KErrNone;    
    if ( fileNameParser.IsWild() )
        {
        err = KErrBadName;
        }
    else
        {
        err = iFramework.Fs().RmDir(aMeta->DesC(CMTPObjectMetaData::ESuid));        
        }
    
    if( KErrNone == err )
        {
        iFramework.ObjectMgr().RemoveObjectL(aMeta->Uint(CMTPObjectMetaData::EHandle));
        iSuccessDeletion = ETrue;
        }
    
    }

void CMTPDeleteObject::DeleteFileL(CMTPObjectMetaData* aMeta)
    {
    TParsePtrC fileNameParser(aMeta->DesC(CMTPObjectMetaData::ESuid));
    TInt err = KErrNone;
    if ( !fileNameParser.NamePresent() )
        {
        err = KErrBadName;
        }
    else if ( fileNameParser.IsWild() )
        {
        err = KErrBadName;
        }
    else
        {
        err = iFramework.Fs().Delete(aMeta->DesC(CMTPObjectMetaData::ESuid));
        }
    
    if( KErrNone == err )
        {
        iFramework.ObjectMgr().RemoveObjectL(aMeta->Uint(CMTPObjectMetaData::EHandle));
        iSuccessDeletion = ETrue;
        }
    else if(KErrAccessDenied == err)
        {
        err = KErrBadHandle;
        iObjectWritePotected = ETrue;
        }
    }

/**
DeleteObject request handler
*/    
void CMTPDeleteObject::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry")); 
	const TUint32 KHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
	iObjectWritePotected = EFalse;
	iSuccessDeletion = EFalse;
	
	CMTPObjectMetaData* meta = NULL;
	meta = iRequestChecker->GetObjectInfo(KHandle);
	__ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
	__FLOG_VA((_L8("meta->Uint(CMTPObjectMetaData::EDataProviderId) is %d"), meta->Uint(CMTPObjectMetaData::EDataProviderId))); 
	__FLOG_VA((_L8("iFramework.DataProviderId() is %d"), iFramework.DataProviderId())); 
	
	if ( meta != NULL && meta->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId())
	    {
	    DeleteFolderOrFileL(meta);
	    }
	else
	    {
	    SendResponseL(EMTPRespCodeInvalidObjectHandle);
	    }
	
    __FLOG(_L8("ServiceL - Exit")); 
    }

/**
Signal to the initiator that the deletion operation has finished with or without error
*/
void CMTPDeleteObject::ProcessFinalPhaseL()
	{
    __FLOG(_L8("ProcessFinalPhaseL - Entry"));
	TMTPResponseCode rsp = EMTPRespCodeOK;
	if ( iObjectWritePotected )
	    {
	    rsp = EMTPRespCodeObjectWriteProtected;
	    }
	else if ( !iSuccessDeletion )
	    {
	    rsp = EMTPRespCodeAccessDenied;
	    }
	SendResponseL(rsp);
    __FLOG(_L8("ProcessFinalPhaseL - Exit"));	
	}
	
/**
Indicates if the specified object is a generic folder association.
@param aObject The object meta-data.
@return ETrue if the specified object is a generic folder association, 
otherwise EFalse.
*/
TBool CMTPDeleteObject::IsFolderObject(const CMTPObjectMetaData& aObject)
    {
    return ((aObject.Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation) &&
            (aObject.Uint(CMTPObjectMetaData::EFormatSubCode) == EMTPAssociationTypeGenericFolder));
    }

/**
Check whether the store on which the object resides is read only.
@return ETrue if the store is read only, EFalse if read-write
*/
TBool CMTPDeleteObject::IsStoreReadOnlyL(TUint32 aObjectHandle)
	{
    __FLOG(_L8("IsStoreReadOnlyL - Entry"));
	TBool result(EFalse);
	CMTPObjectMetaData *info(CMTPObjectMetaData::NewLC());
    if (iFramework.ObjectMgr().ObjectL(aObjectHandle, *info))
        {
    	TInt drive(iFramework.StorageMgr().DriveNumber(info->Uint(CMTPObjectMetaData::EStorageId)));
    	User::LeaveIfError(drive);
    	TVolumeInfo volumeInfo;
    	User::LeaveIfError(iFramework.Fs().Volume(volumeInfo, drive));			
    	if (volumeInfo.iDrive.iMediaAtt == KMediaAttWriteProtected) 
    		{
    		result = ETrue;
    		}
        }
	CleanupStack::PopAndDestroy(info);
    __FLOG(_L8("IsStoreReadOnlyL - Exit"));
	return result;	
	}












