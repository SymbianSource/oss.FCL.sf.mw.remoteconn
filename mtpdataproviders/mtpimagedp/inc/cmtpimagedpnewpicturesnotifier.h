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

#ifndef CMTPIMAGEDPNEWPICTURENOTIFIER_H
#define CMTPIMAGEDPNEWPICTURENOTIFIER_H

#include <e32base.h>

class CMTPImageDpNewPicturesNotifier : public CTimer
    {
public:

    static CMTPImageDpNewPicturesNotifier* NewL(); 
    virtual ~CMTPImageDpNewPicturesNotifier();
    
    void SetNewPictures(TInt aValue);
    void IncreaseCount(TInt aValue);
    void DecreaseCount(TInt aValue);
    
private: // From CTimer

    void RunL();
    
private:

    CMTPImageDpNewPicturesNotifier();
    void ConstructL();
    
private:
    TInt   iNewPictures;
    };

#endif // CMTPIMAGEDPNEWPICTURENOTIFIER_H
