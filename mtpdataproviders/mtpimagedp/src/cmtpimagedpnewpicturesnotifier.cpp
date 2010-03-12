// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalTechnology
*/

#include <mtp/mtpprotocolconstants.h>

#include "cmtpimagedp.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedpnewpicturesnotifier.h"

CMTPImageDpNewPicturesNotifier* CMTPImageDpNewPicturesNotifier::NewL()
    {
    CMTPImageDpNewPicturesNotifier* self = new (ELeave) CMTPImageDpNewPicturesNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor.
*/
CMTPImageDpNewPicturesNotifier::~CMTPImageDpNewPicturesNotifier()
    {
    Cancel();
    }

void CMTPImageDpNewPicturesNotifier::SetNewPictures(TInt aValue)
    {
    iNewPictures = aValue;
    }

void CMTPImageDpNewPicturesNotifier::IncreaseCount(TInt aValue)
    {
    Cancel();//cancel the latest timer if we have
    iNewPictures += aValue;       
    RProperty::Set(TUid::Uid(KMTPServerUID), KMTPNewPicKey, iNewPictures);
    }

void CMTPImageDpNewPicturesNotifier::DecreaseCount(TInt aValue)
    {
    iNewPictures -= aValue;
    
    Cancel();//cancel the latest timer if we have
    if (iNewPictures > 0)
        {
        After(KImageDpNotifyDelay);
        }
    else
        {
        /**
         * if the new pictures is equal to zero, then directly set RProperty' value
         */
        iNewPictures = 0;
        RProperty::Set(TUid::Uid(KMTPServerUID), KMTPNewPicKey, iNewPictures);
        }    
    }

void CMTPImageDpNewPicturesNotifier::RunL()
    {
    RProperty::Set(TUid::Uid(KMTPServerUID), KMTPNewPicKey, iNewPictures);
    }
    
/** 
Constructor
*/
CMTPImageDpNewPicturesNotifier::CMTPImageDpNewPicturesNotifier() : 
    CTimer(EPriorityNormal)    
    {
    
    }    

/**
Second phase constructor.
*/    
void CMTPImageDpNewPicturesNotifier::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    }

