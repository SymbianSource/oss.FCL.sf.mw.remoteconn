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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPPictBridgeEnumerator::ConstructL"));
    iSingletons.OpenL();
    __FLOG(_L8("<< CMTPPictBridgeEnumerator::ConstructL"));
    }

/**
destructor
*/    
CMTPPictBridgeEnumerator::~CMTPPictBridgeEnumerator()
    {
    __FLOG(_L8(">> CMTPPictBridgeEnumerator::~CMTPPictBridgeEnumerator"));
    // we keep the persistent handle
    iSingletons.Close();
    __FLOG(_L8("<< CMTPPictBridgeEnumerator::~CMTPPictBridgeEnumerator"));
	__FLOG_CLOSE;
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
    __FLOG(_L8(">> CMTPPictBridgeEnumerator::EnumerateObjectsL"));
    const TUint storageId(iFramework.StorageMgr().DefaultStorageId());

    if ((aStorageId==KMTPStorageAll) || (aStorageId==storageId))
        {
        MMTPObjectMgr& objectMgr=iFramework.ObjectMgr();

        //delete the files which maybe impact printing
        TFileName        fullPath;        
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostDiscovery);
		__FLOG_VA((_L16("full path is %S "), &fullPath));
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostRequest);
		__FLOG_VA((_L16("full path is %S "), &fullPath));
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
		fullPath = PathInfo::PhoneMemoryRootPath();
		fullPath.Append(KHostResponse);
		__FLOG_VA((_L16("full path is %S "), &fullPath));
		iFramework.Fs().SetAtt(fullPath, KEntryAttNormal, KEntryAttReadOnly);
		iFramework.Fs().Delete(fullPath);
		
        // enumerate device discovery file (create if not exist)

        RFile rf;
        CleanupClosePushL(rf);
        fullPath = PathInfo::PhoneMemoryRootPath();
        fullPath.Append(KDeviceDiscovery);
        __FLOG_VA((_L16("full path is %S "), &fullPath));
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
	    __FLOG_VA((_L8("added discovery file iDpsDiscoveryHandle is 0x%08X"), iDpsDiscoveryHandle));

        CleanupStack::PopAndDestroy(objectP);
        }
		iCallback.NotifyEnumerationCompleteL(aStorageId, KErrNone);

    __FLOG(_L8("<< CMTPPictBridgeEnumerator::EnumerateObjectsL"));
    }

