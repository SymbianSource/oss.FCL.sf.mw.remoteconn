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

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DataProviderSingletons");)

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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    iFramework = &aFramework;
    iSingletons = &CSingletons::OpenL(aFramework);
    __FLOG(_L8("OpenL - Exit"));
    }
    
/**
Closes the singletons reference.
*/
EXPORT_C void RMTPDpSingletons::Close()
    {
    __FLOG(_L8("Close - Entry"));
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    __FLOG(_L8("Close - Exit"));
    __FLOG_CLOSE;
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Entry"));
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Exit"));
    return *self;
    }
    
void RMTPDpSingletons::CSingletons::Close()
    {
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        __FLOG(_L8("CSingletons::Close - Entry"));
        self->Dec();
        if (self->AccessCount() == 0)
            {
            __FLOG(_L8("CSingletons::Close - Exit"));
            delete self;
            Dll::SetTls(NULL);
            }
        else
            {
            __FLOG(_L8("CSingletons::Close - Exit"));
            }
        }
    }
    
RMTPDpSingletons::CSingletons::~CSingletons()
    {
    __FLOG(_L8("CSingletons::~CSingletons - Entry"));
    iExclusionList.Close();
    iMTPUtility.Close();
    __FLOG(_L8("CSingletons::~CSingletons - Exit"));
    __FLOG_CLOSE;
    }
    
void RMTPDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CSingletons::ConstructL - Entry"));
    
    iMTPUtility.OpenL(aFramework);
    
    __FLOG(_L8("CSingletons::ConstructL - Exit"));
    }

EXPORT_C RMTPUtility& RMTPDpSingletons::MTPUtility() const
	{
	return iSingletons->iMTPUtility;
	}
