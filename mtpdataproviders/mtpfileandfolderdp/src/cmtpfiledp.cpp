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

#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/mtpprotocolconstants.h>
#include <ecom/ecom.h>

#include "cmtpfiledp.h"
#include "cmtpfiledpexclusionmgr.h"
#include "cmtpfsenumerator.h"
#include "cmtpfiledpconfigmgr.h"
#include "cmtprequestprocessor.h"
#include "mtpfiledpconst.h"
#include "mtpfiledppanic.h"
#include "mtpfiledpprocessor.h"

// Class constants
static const TInt KArrayGranularity = 3;
static const TInt KActiveEnumeration = 0;
__FLOG_STMT(_LIT8(KComponent,"CMTPFileDataProvider");)

/**
File data provider factory method.
@return A pointer to a file data provider object. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPFileDataProvider::NewL(TAny* aParams)
    {
    CMTPFileDataProvider* self = new (ELeave) CMTPFileDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPFileDataProvider::~CMTPFileDataProvider()
    {
    __FLOG(_L8("~CMTPFileDataProvider - Entry"));
    iPendingEnumerations.Close();
    TUint count(iActiveProcessors.Count());
    while (count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();
    iDpSingletons.Close();
	iFileDPSingletons.Close();
    delete iFileEnumerator;
    delete iExclusionMgr;
    __FLOG(_L8("~CMTPFileDataProvider - Exit"));
    __FLOG_CLOSE; 
    }

void CMTPFileDataProvider::Cancel()
    {
    __FLOG(_L8("Cancel - Entry"));
    iFileEnumerator->Cancel();
    __FLOG(_L8("Cancel - Exit"));
    }
        
void CMTPFileDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8("ProcessEventL - Entry"));
    TInt idx(LocateRequestProcessorL(aEvent, aConnection));
    
    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }
    __FLOG(_L8("ProcessEventL - Exit"));
    }
     
void CMTPFileDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    __FLOG(_L8("ProcessNotificationL - Entry"));
    switch (aNotification)
        {
    case EMTPSessionClosed:
        SessionClosedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
        
    case EMTPSessionOpened:
        SessionOpenedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
        
    default:
        // Ignore all other notifications.
        break;
        }
    __FLOG(_L8("ProcessNotificationL - Exit"));
    }
        
void CMTPFileDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    __FLOG(_L8("ProcessRequestPhaseL - Entry"));
    TInt idx(LocateRequestProcessorL(aRequest, aConnection));
    __ASSERT_DEBUG((idx != KErrNotFound), Panic(EMTPFileDpNoMatchingProcessor));
    MMTPRequestProcessor* processor(iActiveProcessors[idx]);
    iActiveProcessor = idx;
    iActiveProcessorRemoved = EFalse;
    TBool result(processor->HandleRequestL(aRequest, aPhase));
    if (iActiveProcessorRemoved)
	    {
	    processor->Release(); // destroy the processor
	    }
    else if (result)
	    {
	    processor->Release();    	
	    iActiveProcessors.Remove(idx);
	    }
    iActiveProcessor = -1;

    __FLOG(_L8("ProcessRequestPhaseL - Exit"));
    }
    
void CMTPFileDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    __FLOG(_L8("StartObjectEnumerationL - Entry"));

    iExclusionMgr->AppendFormatExclusionListL();
    iDpSingletons.MTPUtility().FormatExtensionMapping();
    iPendingEnumerations.AppendL(aStorageId);
    if (iPendingEnumerations.Count() == 1)
        {
        iFileEnumerator->StartL(iPendingEnumerations[KActiveEnumeration]);
        }
    __FLOG(_L8("StartObjectEnumerationL - Exit"));
    }
    
void CMTPFileDataProvider::StartStorageEnumerationL()
    {
    __FLOG(_L8("StartStorageEnumerationL - Entry"));
    Framework().StorageEnumerationCompleteL();
    __FLOG(_L8("StartStorageEnumerationL - Exit"));
    }
    
void CMTPFileDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    __FLOG(_L8("Supported - Entry"));
    switch (aCategory) 
        {        
    case EEvents:
        break;
        
    case EObjectCaptureFormats:
    case EObjectPlaybackFormats:
        aArray.Append(EMTPFormatCodeUndefined);
        
        /**
         * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
         * but it does not really own the format 0x3002.
         */
        if( PictbridgeDpExistL() )
        	{
        	aArray.Append(EMTPFormatCodeScript);
        	}
        break;
        
    case EObjectProperties:
        {
        TInt count(sizeof(KMTPFileDpSupportedProperties) / sizeof(KMTPFileDpSupportedProperties[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPFileDpSupportedProperties[i]);
            }   
        }
        break; 
        
    case EOperations:
        {
        TInt count(sizeof(KMTPFileDpSupportedOperations) / sizeof(KMTPFileDpSupportedOperations[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPFileDpSupportedOperations[i]);
            }   
        }
        break;  
        
    case EStorageSystemTypes:
        aArray.Append(CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
        break; 
        
    default:
    
        // Unrecognised category, leave aArray unmodified.
        break;
        }
    __FLOG(_L8("Supported - Exit"));
    }    
       
#ifdef __FLOG_ACTIVE  
void CMTPFileDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError)
#else
void CMTPFileDataProvider::NotifyEnumerationCompleteL(TUint32 /*aStorageId*/, TInt /*aError*/)
#endif // __FLOG_ACTIVE
    {
    __FLOG(_L8("HandleEnumerationCompletedL - Entry"));
    __FLOG_VA((_L8("Enumeration of storage 0x%08X completed with error status %d"), aStorageId, aError));
    __ASSERT_DEBUG((aStorageId == iPendingEnumerations[KActiveEnumeration]), User::Invariant());
    
    Framework().ObjectEnumerationCompleteL(iPendingEnumerations[KActiveEnumeration]);
    iPendingEnumerations.Remove(KActiveEnumeration);
    if (iPendingEnumerations.Count())
        {
        iFileEnumerator->StartL(iPendingEnumerations[KActiveEnumeration]);
        }
    __FLOG(_L8("HandleEnumerationCompletedL - Exit"));
    }
    
/**
Standard C++ constructor
*/
CMTPFileDataProvider::CMTPFileDataProvider(TAny* aParams) :
    CMTPDataProviderPlugin(aParams),
    iActiveProcessors(KArrayGranularity),
    iPendingEnumerations(KArrayGranularity),
    iActiveProcessor(-1)
    {

    }

/**
Second-phase constructor.
*/    
void CMTPFileDataProvider::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);  
    __FLOG(_L8("ConstructL - Entry"));
  	iDpSingletons.OpenL(Framework());
  	iFileDPSingletons.OpenL(Framework());
  	
  	iExclusionMgr = CMTPFileDpExclusionMgr::NewL(Framework());
  	iDpSingletons.SetExclusionMgrL(*iExclusionMgr);
  	
  	TUint processLimit = iFileDPSingletons.FrameworkConfig().UintValueL(CMTPFileDpConfigMgr::EEnumerationIterationLength);
    iFileEnumerator = CMTPFSEnumerator::NewL(Framework(), iDpSingletons.ExclusionMgrL(), *this, processLimit);
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
Find or create a request processor that can process the specified request.
@param aRequest    The request to be processed
@param aConnection The connection from which the request comes
@return the idx of the found/created request processor
*/    
TInt CMTPFileDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("LocateRequestProcessorL - Entry"));  
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i(0); (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            idx = i;
            break;
            }
        }
        
    if (idx == KErrNotFound)
        {
        MMTPRequestProcessor* processor = MTPFileDpProcessor::CreateL(Framework(), aRequest, aConnection);
        CleanupReleasePushL(*processor);
        iActiveProcessors.AppendL(processor);
        CleanupStack::Pop();
        idx = count;
        }
        
    __FLOG(_L8("LocateRequestProcessorL - Exit"));
    return idx;
    }

/**
Find or create a request processor that can process the specified event.
@param aEvent    The event to be processed
@param aConnection The connection from which the request comes
@return the idx of the found request processor, KErrNotFound if not found
*/    
TInt CMTPFileDataProvider::LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8("LocateRequestProcessorL - Entry"));
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i(0); (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aEvent, aConnection))
            {
            idx = i;
            break;
            }
        }    
    __FLOG(_L8("LocateRequestProcessorL - Exit"));
    return idx;    
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPFileDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    __FLOG(_L8("SessionClosedL - Entry"));
    TInt count = iActiveProcessors.Count();
    while(count--)
        {
        MMTPRequestProcessor* processor = iActiveProcessors[count];
        TUint32 sessionId = processor->SessionId();
        if((sessionId == aSession.iMTPId) && (processor->Connection().ConnectionId() == aSession.iConnection.ConnectionId()))
            {
            iActiveProcessors.Remove(count);
            if (count == iActiveProcessor)
        		{
        		iActiveProcessorRemoved = ETrue;
        		}
        	else
	       		{
        		processor->Release();
	       		}
            }
        } 
    __FLOG(_L8("SessionClosedL - Exit"));   
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef __FLOG_ACTIVE
void CMTPFileDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPFileDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    __FLOG(_L8("SessionOpenedL - Entry"));
    __FLOG_VA((_L8("SessionID = %d"), aSession.iMTPId));
    __FLOG(_L8("SessionOpenedL - Exit"));
    }

/**
 * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
 * but it does not really own the format 0x3002.
 * 
 * Check whether the Pictbridgedp exists or not.
 */
TBool CMTPFileDataProvider::PictbridgeDpExistL() const
	{
    __FLOG(_L8("PictbridgeDpExistL - Entry"));
    
	RImplInfoPtrArray   implementations;
	TCleanupItem        cleanup(ImplementationsCleanup, reinterpret_cast<TAny*>(&implementations));
	CleanupStack::PushL(cleanup);
	REComSession::ListImplementationsL(KMTPDataProviderPluginInterfaceUid, implementations);
	
	TBool ret = EFalse;
	const TUint KUidPictBridge = 0x2001fe3c;
	TInt count = implementations.Count();
	while(--count)
		{
		if(implementations[count]->ImplementationUid().iUid == KUidPictBridge)
			{
			ret = ETrue;
			break;
			}
		}
    CleanupStack::PopAndDestroy(&implementations);
    
    __FLOG_VA((_L8("return value ret = %d"), ret));
    __FLOG(_L8("PictbridgeDpExistL - Exit"));
    return ret;
	}

/**
 * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
 * but it does not really own the format 0x3002.
 * 
 * Cleanup function
 */
void CMTPFileDataProvider::ImplementationsCleanup(TAny* aData)
    {
    reinterpret_cast<RImplInfoPtrArray*>(aData)->ResetAndDestroy();
    }
