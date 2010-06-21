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

#include <centralrepository.h>

#include "cmtpframeworkconfig.h"

/**
CMTPFrameworkConfig factory method. 
@return A pointer to a new CMTPFrameworkConfig instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPFrameworkConfig* CMTPFrameworkConfig::NewL()
    {
    CMTPFrameworkConfig* self = new (ELeave) CMTPFrameworkConfig();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
CMTPFrameworkConfig::~CMTPFrameworkConfig()
    {
    //Save the AbnormalDown state to EFalse
    const TInt KNormalShutDownValue = 0;
    iRepository->Set(EAbnormalDown, KNormalShutDownValue);
    
    delete iRepository;
    }

EXPORT_C void CMTPFrameworkConfig::GetValueL(TParameter aParam, TDes& aValue) const
    {
    TInt err(iRepository->Get(aParam, aValue));
    if (KErrNotFound == err)
        {
        aValue = KNullDesC;
        }
    else if (KErrNone != err)
        {
        User::Leave(err);
        }
    }

EXPORT_C HBufC* CMTPFrameworkConfig::ValueL(TParameter aParam) const
    {
    TInt length;
    HBufC* buf = HBufC::NewLC(0);
    TPtr ptr(buf->Des());
    TInt err = iRepository->Get(aParam, ptr, length);    

    // We want to get the length here so ignore the error if KErrOverflow
    // Sometimes, the return value is KErrNone
    if (KErrOverflow == err  || KErrNone == err)
        {
        // Now reallocate the buffer to length
        if((length > 0)&&(length <255))
            {
        buf = buf->ReAllocL(length);
        CleanupStack::Pop();
        CleanupStack::PushL(buf);
        
        // Get the value
        ptr.Set(buf->Des());
        User::LeaveIfError(iRepository->Get(aParam, ptr));
            }
        }
    else if (KErrNotFound != err)
        {
        User::Leave(err);
        }
    CleanupStack::Pop(buf);
    return buf;
    }

EXPORT_C void CMTPFrameworkConfig::GetValueL(TParameter aParam, TUint& aValue) const
    {    
    // Use a temporary to avoid the compiler warning
    TInt value(0);
    TInt err(iRepository->Get(aParam, value));
    if ((KErrNone != err ) &&
        (KErrNotFound != err))
        {
        User::Leave(err);
        }
    aValue = static_cast<TUint>(value);
    }

EXPORT_C void CMTPFrameworkConfig::GetValueL(TParameter aParam, TBool& aValue) const
    {
    TInt value(0);
    if(EAbnormalDown == aParam)
    	{
    	value = iAbnormalDownValue;
    	}
    else
    	{
        TInt err(iRepository->Get(aParam, value));
        if ((KErrNone != err ) &&
            (KErrNotFound != err))
            {
            User::Leave(err);
            };
    	}
    aValue = (value != 0);
    }
   
EXPORT_C void CMTPFrameworkConfig::GetValueL(TParameter aParam, RArray<TUint>& aArray) const
    {
    aArray.Reset();
    if (CMTPFrameworkConfig::EExcludedStorageDrives != aParam)
        {
        User::Leave(KErrArgument);
        }
        
    // Array settings key mask. All array settings keys must be unique that are
    // unique in the most significant 2 bytes of the mask
    static const TUint32 KMTPRepositoryArrayMask = 0xFFFF0000;
    RArray<TUint32> keys;
    CleanupClosePushL(keys);
    
    aArray.Reset();            
    // Retrieve the keys for all array elements
    TInt err(iRepository->FindL(aParam, KMTPRepositoryArrayMask, keys));
    if (KErrNone == err)
        {
        // Iterate the keys, retrieve the values and append them to the destination array
        TInt count = keys.Count();
        for (TInt index = 0; index < count; index++)
            {        
            TInt value;
            User::LeaveIfError(iRepository->Get(keys[index], value));    
            aArray.AppendL(static_cast<TUint>(value));
            }                 
        }
    else if (KErrNotFound != err)
        {
        User::Leave(err);    
        }
    
    CleanupStack::PopAndDestroy(&keys);    
    }
    
/**
Constructor
*/
CMTPFrameworkConfig::CMTPFrameworkConfig()
    {
    }

/**
Second phase constructor.
*/
void CMTPFrameworkConfig::ConstructL()
    {
    const TUint32 KUidMTPRepositoryValue(0x10282FCC);
    const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPRepository);
    
    const TInt KStartupInitValue = 1;
    iAbnormalDownValue = 0;
    TInt err(iRepository->Get(EAbnormalDown, iAbnormalDownValue));
    if ((KErrNone != err ) && (KErrNotFound != err))
		{
		User::Leave(err);
		}
    
    //Save the AbnormalDown state to ETrue
    //if connect the phone to PC while backup, this will leave.
    TRAP_IGNORE(iRepository->Set(EAbnormalDown, KStartupInitValue ));
    }

