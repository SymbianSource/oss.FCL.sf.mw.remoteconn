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

#include "cmtpstoragemgr.h"
#include "cmtpdeleteobject.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdeleteobjectTraces.h"
#endif



// Class constants.

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
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CMTPDeleteObject::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPDELETEOBJECT_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
	OstTraceFunctionExit0( CMTPDELETEOBJECT_CONSTRUCTL_EXIT );
	}

/**
Destructor
*/    
EXPORT_C CMTPDeleteObject::~CMTPDeleteObject()
    {
    OstTraceFunctionEntry0( CMTPDELETEOBJECT_CMTPDELETEOBJECT_DES_ENTRY );
    iSingletons.Close();
    OstTraceFunctionExit0( CMTPDELETEOBJECT_CMTPDELETEOBJECT_DES_EXIT );
    }

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/ 

 
TMTPResponseCode CMTPDeleteObject::CheckRequestL()
	{
    OstTraceFunctionEntry0( CMTPDELETEOBJECT_CHECKREQUESTL_ENTRY );
	TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
	const TUint32 KObjectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	if ((EMTPRespCodeOK == result) && (IsStoreReadOnlyL(KObjectHandle)) )
		{
		result = EMTPRespCodeStoreReadOnly;
		}
		
	OstTraceFunctionExit0( CMTPDELETEOBJECT_CHECKREQUESTL_EXIT );
	return result;	
	} 
	
void CMTPDeleteObject::DeleteFolderOrFileL(CMTPObjectMetaData* aMeta)
    {
    __ASSERT_DEBUG(aMeta, Panic(EMTPDpObjectNull));
    if (IsFolderObject(*aMeta))
        {
        OstTrace0( TRACE_NORMAL, CMTPDELETEOBJECT_DELETEFOLDERORFILEL, "Delete the folder itself which is empty " );
        DeleteFolderL(aMeta);
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPDELETEOBJECT_DELETEFOLDERORFILEL, "Going to delete a file." );        
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
    OstTraceFunctionEntry0( CMTPDELETEOBJECT_SERVICEL_ENTRY );
	const TUint32 KHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
	iObjectWritePotected = EFalse;
	iSuccessDeletion = EFalse;
	
	CMTPObjectMetaData* meta = NULL;
	meta = iRequestChecker->GetObjectInfo(KHandle);
	__ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
	OstTrace1( TRACE_NORMAL, CMTPDELETEOBJECT_SERVICEL, 
	        "meta->Uint(CMTPObjectMetaData::EDataProviderId) is %d",  meta->Uint(CMTPObjectMetaData::EDataProviderId));
	OstTrace1( TRACE_NORMAL, DUP1_CMTPDELETEOBJECT_SERVICEL, "iFramework.DataProviderId() is %d", iFramework.DataProviderId() );
		
	if ( meta != NULL && meta->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId())
	    {
	    DeleteFolderOrFileL(meta);
	    }
	else
	    {
	    SendResponseL(EMTPRespCodeInvalidObjectHandle);
	    }

    OstTraceFunctionExit0( CMTPDELETEOBJECT_SERVICEL_EXIT );
    }

/**
Signal to the initiator that the deletion operation has finished with or without error
*/
void CMTPDeleteObject::ProcessFinalPhaseL()
	{
    OstTraceFunctionEntry0( CMTPDELETEOBJECT_PROCESSFINALPHASEL_ENTRY );
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
	OstTraceFunctionExit0( CMTPDELETEOBJECT_PROCESSFINALPHASEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPDELETEOBJECT_ISSTOREREADONLYL_ENTRY );
	TBool result(EFalse);
	CMTPObjectMetaData *info(CMTPObjectMetaData::NewLC());
    if (iFramework.ObjectMgr().ObjectL(aObjectHandle, *info))
        {
		result = !iSingletons.StorageMgr().IsReadWriteStorage(info->Uint(CMTPObjectMetaData::EStorageId));
        }
	CleanupStack::PopAndDestroy(info);
	OstTraceFunctionExit0( CMTPDELETEOBJECT_ISSTOREREADONLYL_EXIT );
	return result;	
	}












