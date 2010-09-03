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

#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpproxydp.h"
#include "cmtprequestprocessor.h"
#include "mtpproxydppanic.h"
#include "mtpproxydpprocessor.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpproxydpTraces.h"
#endif
 

static const TInt KMTPProxyDpSessionGranularity = 3;	

/**
Standard NewL method
@param aParams	The pointer to MMTPDataProviderFramework
*/	
TAny* CMTPProxyDataProvider::NewL(TAny* aParams)
	{
	CMTPProxyDataProvider* self = new (ELeave) CMTPProxyDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
	}
	
	
void CMTPProxyDataProvider::ConstructL()
	{  
	OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_CONSTRUCTL_ENTRY );
	iDpSingletons.OpenL(Framework());
	iSingletons.OpenL(Framework());
	OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_CONSTRUCTL_EXIT );
	}

/**
Destructor
*/	
CMTPProxyDataProvider::~CMTPProxyDataProvider()
	{
	OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_CMTPPROXYDATAPROVIDER_DES_ENTRY );
	TInt count = iActiveProcessors.Count();
	while(count--)
		{
		iActiveProcessors[count]->Release();
		}
	iActiveProcessors.Close();	
	iDpSingletons.Close();
	iSingletons.Close();
	OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_CMTPPROXYDATAPROVIDER_DES_EXIT );
	}
    
void CMTPProxyDataProvider::Cancel()
    {
    
    }

/**
Process the event from initiator
@param aEvent	The event to be processed
@param aConnection	The connection from which the event comes
*/	
void CMTPProxyDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
	{
	TInt index = LocateRequestProcessorL(aEvent, aConnection);
	if(index != KErrNotFound)
		{
		iActiveProcessors[index]->HandleEventL(aEvent);
		}
	}
     
void CMTPProxyDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
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
    }

/**
Process the request from initiator.  The method first searches if there is any matching active processor;
if not, it creates a new processor.  It deletes the processor once it finishes.
@param aPhase	The request transaction phase
@param aRequest	The request to be processed
@param aConnection	The connection from which the request comes
*/	
void CMTPProxyDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
	{	
	TInt index = LocateRequestProcessorL(aRequest, aConnection);
	__ASSERT_DEBUG(index != KErrNotFound, Panic(EMTPNoMatchingProcessor));
	MMTPRequestProcessor* processor = iActiveProcessors[index];
	iActiveProcessor = index;
	iActiveProcessorRemoved = EFalse;
	TBool result = processor->HandleRequestL(aRequest, aPhase);
	if (iActiveProcessorRemoved)
		    {
		    processor->Release(); // destroy the processor
		    }
	else if (result)
		    {    
		    processor->Release();    	
		    iActiveProcessors.Remove(index);
		    }
	iActiveProcessor = -1;

	}
	
void CMTPProxyDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    // The proxy data provider does not manage any data objects.
    Framework().ObjectEnumerationCompleteL(aStorageId);
    }
    
void CMTPProxyDataProvider::StartStorageEnumerationL()
    {
    Framework().StorageEnumerationCompleteL();
    }

/**
Append all the operations that are supported by the proxy data provider to
the operations array.
@param aCategory category of operations of interest
@param aArray array to hold the operations
*/
void CMTPProxyDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& /*aArray*/) const
    {
    OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_SUPPORTED_ENTRY );
    switch (aCategory) 
        {        
    case EEvents:
        break;
        
    case EObjectCaptureFormats:
    case EObjectPlaybackFormats:

        break;
        
    case EObjectProperties:
        break; 
        
    case EOperations:
        break;  
        
    case EStorageSystemTypes:
        break; 
        
    default:
    
        // Unrecognised category, leave aArray unmodified.
        break;
        }

    OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_SUPPORTED_EXIT );
    }
    

void CMTPProxyDataProvider::SupportedL(TMTPSupportCategory /*aCategory*/, CDesCArray& /*aStrings*/) const
	{
	OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_SUPPORTEDL_ENTRY );
	OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_SUPPORTEDL_EXIT );
	}
  
    

/**
Standard c++ constructor
*/
CMTPProxyDataProvider::CMTPProxyDataProvider(TAny* aParams)
	:CMTPDataProviderPlugin(aParams),
	iActiveProcessors(KMTPProxyDpSessionGranularity),
	iActiveProcessor(-1)
	{
	}

/**
Find or create a request processor that can process the request
@param aRequest	The request to be processed
@param aConnection The connection from which the request comes
@return the index of the found/created request processor
*/
TInt CMTPProxyDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    TInt index = KErrNotFound;
    TInt count = iActiveProcessors.Count();
    for(TInt i = 0; i < count; i++)
        {
        if(iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            index = i;
            break;
            }
        }
    if(index == KErrNotFound)
        {
        MMTPRequestProcessor* processor = MTPProxyDpProcessor::CreateL(Framework(), aRequest, aConnection);
        __ASSERT_DEBUG(processor, Panic(EMTPNoMatchingProcessor));
        CleanupReleasePushL(*processor);
        iActiveProcessors.AppendL(processor);
        CleanupStack::Pop();
        index = count;
        }
    return index;

    }

/**
Find a request processor that can process the event
@param aEvent	The event to be processed
@param aConnection The connection from which the request comes
@return the index of the found request processor, KErrNotFound if not found
*/	
TInt CMTPProxyDataProvider::LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    TInt index = KErrNotFound;
    TInt count = iActiveProcessors.Count();
    for(TInt i = 0; i < count; i++)
        {
        if(iActiveProcessors[i]->Match(aEvent, aConnection))
            {
            index = i;
            break;
            }
        }
    return index;
    }
       
void CMTPProxyDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
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
    OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPProxyDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#else
void CMTPProxyDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPPROXYDATAPROVIDER_SESSIONOPENEDL_ENTRY );
    OstTraceFunctionExit0( CMTPPROXYDATAPROVIDER_SESSIONOPENEDL_EXIT );
    }

