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

#include "rmtpdpsingletons.h"
#include "cmtpfsentrycache.h"

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rmtpdpsingletonsTraces.h"
#endif



/**
Constructor.
*/
EXPORT_C RMTPDpSingletons::RMTPDpSingletons() :
    iSingletons(NULL)
    {
    }

/**
Opens the singletons reference.
*/
EXPORT_C void RMTPDpSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( RMTPDPSINGLETONS_OPENL_ENTRY );
    iFramework = &aFramework;
    iSingletons = &CSingletons::OpenL(aFramework);
    OstTraceFunctionExit0( RMTPDPSINGLETONS_OPENL_EXIT );
    }
    
/**
Closes the singletons reference.
*/
EXPORT_C void RMTPDpSingletons::Close()
    {
    OstTraceFunctionEntry0( RMTPDPSINGLETONS_CLOSE_ENTRY );
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    OstTraceFunctionExit0( RMTPDPSINGLETONS_CLOSE_EXIT );
    }


/**
This method finds the specific data provider's file system exclusion manager based on the 
DP ID and returns it.
@return the calling data provider's file system exclusion manager
*/	
EXPORT_C CMTPFSExclusionMgr& RMTPDpSingletons::ExclusionMgrL() const
	{
	TExclusionMgrEntry entry = { 0, iFramework->DataProviderId() };
	TInt index = iSingletons->iExclusionList.FindInOrderL(entry, TLinearOrder<TExclusionMgrEntry>(TExclusionMgrEntry::Compare));
	return *(iSingletons->iExclusionList[index].iExclusionMgr);
	}

/**
Inserts the calling data provider's file system exclusion manager to an ordered list 
based on the the DP ID.
@param aExclusionMgr a reference to a data provider's file system exclusion manager.
*/	
EXPORT_C void RMTPDpSingletons::SetExclusionMgrL(CMTPFSExclusionMgr& aExclusionMgr)
	{
	TExclusionMgrEntry entry = { &aExclusionMgr, iFramework->DataProviderId() };
	iSingletons->iExclusionList.InsertInOrderL(entry, TLinearOrder<TExclusionMgrEntry>(TExclusionMgrEntry::Compare));
	}

TInt RMTPDpSingletons::TExclusionMgrEntry::Compare(const TExclusionMgrEntry& aFirst, const TExclusionMgrEntry& aSecond)
	{
	return (aFirst.iDpId - aSecond.iDpId);
	}
 
RMTPDpSingletons::CSingletons* RMTPDpSingletons::CSingletons::NewL(MMTPDataProviderFramework& aFramework)
    {
    CSingletons* self(new(ELeave) CSingletons());
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    return self;
    }

RMTPDpSingletons::CSingletons& RMTPDpSingletons::CSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( RMTPDPSIGNGLETONS_CSINGLETONS_OPENL_ENTRY );
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (!self)
        {
        self = CSingletons::NewL(aFramework);
        Dll::SetTls(reinterpret_cast<TAny*>(self));
        }
    else
        {        
        self->Inc();
        }
    OstTraceFunctionExit0( RMTPDPSIGNGLETONS_CSINGLETONS_OPENL_EXIT );
    return *self;
    }
    
void RMTPDpSingletons::CSingletons::Close()
    {
    OstTraceFunctionEntry0( CSINGLETONS_CLOSE_ENTRY );    
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        self->Dec();
        if (self->AccessCount() == 0)
            {
            delete self;
            Dll::SetTls(NULL);
            OstTraceFunctionExit0( CSINGLETONS_CLOSE_EXIT );
            }
        else
            {
            OstTraceFunctionExit0( DUP1_CSINGLETONS_CLOSE_EXIT );
            }
        }
    }
    
RMTPDpSingletons::CSingletons::~CSingletons()
    {
    OstTraceFunctionEntry0( CSINGLETONS_CSINGLETONS_DES_ENTRY );
    iExclusionList.Close();
    iMTPUtility.Close();
    delete iCopyingBigFileCache;
    delete iMovingBigFileCache;
    OstTraceFunctionExit0( CSINGLETONS_CSINGLETONS_DES_EXIT );
    }
    
void RMTPDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( CSINGLETONS_CONSTRUCTL_ENTRY );
    
    iMTPUtility.OpenL(aFramework);
    iCopyingBigFileCache = CMTPFSEntryCache::NewL();
    iMovingBigFileCache = CMTPFSEntryCache::NewL();

    OstTraceFunctionExit0( CSINGLETONS_CONSTRUCTL_EXIT );
    }

EXPORT_C RMTPUtility& RMTPDpSingletons::MTPUtility() const
	{
	return iSingletons->iMTPUtility;
	}

EXPORT_C CMTPFSEntryCache& RMTPDpSingletons::CopyingBigFileCache() const
  {
  return *(iSingletons->iCopyingBigFileCache);
  }

EXPORT_C CMTPFSEntryCache& RMTPDpSingletons::MovingBigFileCache() const
  {
  return *(iSingletons->iMovingBigFileCache);
  }

