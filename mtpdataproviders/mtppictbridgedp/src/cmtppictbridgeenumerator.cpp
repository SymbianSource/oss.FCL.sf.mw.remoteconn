// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <bautils.h>
#include <s32file.h>
#include <e32std.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptypeevent.h>
#include "cmtppictbridgeenumerator.h"
#include "mmtppictbridgeenumeratorcallback.h"
#include "ptpdef.h"
#include "cmtpdataprovidercontroller.h"
#include <mtp/cmtptypefile.h>
#include <pathinfo.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgeenumeratorTraces.h"
#endif


//==================================================================
// 
//==================================================================  
CMTPPictBridgeEnumerator* CMTPPictBridgeEnumerator::NewL(MMTPDataProviderFramework& aFramework, MMTPPictBridgeEnumeratorCallback& aCallback)
    {
    CMTPPictBridgeEnumerator* self = new (ELeave) CMTPPictBridgeEnumerator(aFramework, aCallback);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

//==================================================================
// 
//==================================================================  
CMTPPictBridgeEnumerator::CMTPPictBridgeEnumerator(MMTPDataProviderFramework& aFramework, MMTPPictBridgeEnumeratorCallback& aCallback)
    :iFramework(aFramework), iCallback(aCallback)
    {
    }

//==================================================================
// 
//==================================================================  
void CMTPPictBridgeEnumerator::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEENUMERATOR_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
    OstTraceFunctionExit0( CMTPPICTBRIDGEENUMERATOR_CONSTRUCTL_EXIT );
    }

/**
destructor
*/    
CMTPPictBridgeEnumerator::~CMTPPictBridgeEnumerator()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEENUMERATOR_CMTPPICTBRIDGEENUMERATOR_DES_ENTRY );
    // we keep the persistent handle
    iSingletons.Close();
    OstTraceFunctionExit0( CMTPPICTBRIDGEENUMERATOR_CMTPPICTBRIDGEENUMERATOR_DES_EXIT );
    }

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
//
void CMTPPictBridgeEnumerator::EnumerateStoragesL()
    {
    iCallback.NotifyStorageEnumerationCompleteL();
    }

// --------------------------------------------------------------------------
// "handle of the file DDISCVRY.DPS"
// --------------------------------------------------------------------------
TUint32 CMTPPictBridgeEnumerator::DeviceDiscoveryHandle() const
    {
    return iDpsDiscoveryHandle;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
void CMTPPictBridgeEnumerator::EnumerateObjectsL(TUint32 aStorageId)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL_ENTRY );
    const TUint storageId(iFramework.StorageMgr().DefaultStorageId());

    if ((aStorageId==KMTPStorageAll) || (aStorageId==storageId))
        {
        MMTPObjectMgr& objectMgr=iFramework.ObjectMgr();

        //delete the files which maybe impact printing
        TFileName        fullPath;        
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostDiscovery);
		OstTraceExt1( TRACE_NORMAL, CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL, "full path is %S ", fullPath );
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostRequest);
	    OstTraceExt1( TRACE_NORMAL, DUP1_CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL, "full path is %S ", fullPath );
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostResponse);
        OstTraceExt1( TRACE_NORMAL, DUP2_CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL, "full path is %S ", fullPath );		
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
        // enumerate device discovery file (create if not exist)

        RFile rf;
        CleanupClosePushL(rf);
        fullPath = PathInfo::PhoneMemoryRootPath();
        fullPath.Append(KDeviceDiscovery);
        OstTraceExt1( TRACE_NORMAL, DUP3_CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL, "full path is %S ", fullPath );                
        iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
        iFramework.Fs().Delete(fullPath);
        
        rf.Create(iFramework.Fs(), fullPath, EFileWrite);
        TTime time;
        time.HomeTime();
        rf.SetModified(time);
        CleanupStack::PopAndDestroy(&rf);
        
        CMTPObjectMetaData* objectP = CMTPObjectMetaData::NewLC(iSingletons.DpController().FileDpId(), EMTPFormatCodeScript, storageId, fullPath);

        objectP->SetUint(CMTPObjectMetaData::EParentHandle, KMTPHandleNoParent);
	    objectMgr.InsertObjectL(*objectP);
	    iDpsDiscoveryHandle = objectP->Uint( CMTPObjectMetaData::EHandle );
	    OstTrace1( TRACE_NORMAL, DUP4_CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL, 
	            "added discovery file iDpsDiscoveryHandle is 0x%08X", iDpsDiscoveryHandle );

        CleanupStack::PopAndDestroy(objectP);
        }
		iCallback.NotifyEnumerationCompleteL(aStorageId, KErrNone);

    OstTraceFunctionExit0( CMTPPICTBRIDGEENUMERATOR_ENUMERATEOBJECTSL_EXIT );
    }

