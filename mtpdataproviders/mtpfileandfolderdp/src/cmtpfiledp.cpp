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
#include "cmtpdataprovidercontroller.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpfiledpTraces.h"
#endif


// Class constants
static const TInt KArrayGranularity = 3;
static const TInt KActiveEnumeration = 0;

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
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_CMTPFILEDATAPROVIDER_DES_ENTRY );
    iPendingEnumerations.Close();
    TUint count(iActiveProcessors.Count());
    while (count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();
    iDpSingletons.Close();
	iFileDPSingletons.Close();
	iSingletons.Close();
    delete iFileEnumerator;
    delete iExclusionMgr;
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_CMTPFILEDATAPROVIDER_DES_EXIT );
    }

void CMTPFileDataProvider::Cancel()
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_CANCEL_ENTRY );
    iFileEnumerator->Cancel();
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_CANCEL_EXIT );
    }
        
void CMTPFileDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_PROCESSEVENTL_ENTRY );
    TInt idx(LocateRequestProcessorL(aEvent, aConnection));
    
    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_PROCESSEVENTL_EXIT );
    }
     
void CMTPFileDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_PROCESSNOTIFICATIONL_ENTRY );
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
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_PROCESSNOTIFICATIONL_EXIT );
    }
        
void CMTPFileDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_PROCESSREQUESTPHASEL_ENTRY );
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

    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_PROCESSREQUESTPHASEL_EXIT );
    }
    
void CMTPFileDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_STARTOBJECTENUMERATIONL_ENTRY );
    iPendingEnumerations.AppendL(aStorageId);
    CMTPDataProviderController& dpController(iSingletons.DpController());
    //must read this NeedEnumeratingPhase2 before this function return
    TBool bScanAll = dpController.NeedEnumeratingPhase2();
    iFileEnumerator->StartL(iPendingEnumerations[KActiveEnumeration], bScanAll);
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_STARTOBJECTENUMERATIONL_EXIT );
    }
    
void CMTPFileDataProvider::StartStorageEnumerationL()
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_STARTSTORAGEENUMERATIONL_ENTRY );
    iExclusionMgr->AppendFormatExclusionListL();
    iDpSingletons.MTPUtility().FormatExtensionMapping();
    Framework().StorageEnumerationCompleteL();
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_STARTSTORAGEENUMERATIONL_EXIT );
    }
    
void CMTPFileDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_SUPPORTED_ENTRY );
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
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_SUPPORTED_EXIT );
    }    

#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPFileDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError)
#else
void CMTPFileDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /*aError*/)
#endif       
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPFILEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL, 
            "Enumeration of storage 0x%08X completed with error status %d", aStorageId, (TInt32)aError);
    aStorageId = aStorageId;
    __ASSERT_DEBUG((aStorageId == iPendingEnumerations[KActiveEnumeration]), User::Invariant());
    
    Framework().ObjectEnumerationCompleteL(iPendingEnumerations[KActiveEnumeration]);
    iPendingEnumerations.Remove(KActiveEnumeration);
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_CONSTRUCTL_ENTRY );
  	iDpSingletons.OpenL(Framework());
  	iFileDPSingletons.OpenL(Framework());
  	iSingletons.OpenL();
  	
  	iExclusionMgr = CMTPFileDpExclusionMgr::NewL(Framework());
  	iDpSingletons.SetExclusionMgrL(*iExclusionMgr);
  	
  	TUint processLimit = iFileDPSingletons.FrameworkConfig().UintValueL(CMTPFileDpConfigMgr::EEnumerationIterationLength);
    iFileEnumerator = CMTPFSEnumerator::NewL(Framework(), iDpSingletons.ExclusionMgrL(), *this, processLimit);
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_CONSTRUCTL_EXIT );
    }

/**
Find or create a request processor that can process the specified request.
@param aRequest    The request to be processed
@param aConnection The connection from which the request comes
@return the idx of the found/created request processor
*/    
TInt CMTPFileDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY ); 
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

    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPFILEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );
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
    OstTraceFunctionExit0( DUP1_CMTPFILEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
    return idx;    
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPFileDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
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
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPFileDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPFileDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_SESSIONOPENEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPFILEDATAPROVIDER_SESSIONOPENEDL, "SessionID = %d", aSession.iMTPId );
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_SESSIONOPENEDL_EXIT );
    }

/**
 * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
 * but it does not really own the format 0x3002.
 * 
 * Check whether the Pictbridgedp exists or not.
 */
TBool CMTPFileDataProvider::PictbridgeDpExistL() const
	{
    OstTraceFunctionEntry0( CMTPFILEDATAPROVIDER_PICTBRIDGEDPEXISTL_ENTRY );
    
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

    OstTrace1( TRACE_NORMAL, CMTPFILEDATAPROVIDER_PICTBRIDGEDPEXISTL, "return value ret = %d", ret );
    OstTraceFunctionExit0( CMTPFILEDATAPROVIDER_PICTBRIDGEDPEXISTL_EXIT );
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
