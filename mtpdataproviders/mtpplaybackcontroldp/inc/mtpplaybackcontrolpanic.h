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
 @internalComponent
*/

#ifndef MTPPLAYBACKCONTROLPANIC_H
#define MTPPLAYBACKCONTROLPANIC_H

#include <e32std.h>

enum TMTPPlaybackControlPanic
    {
    EMTPPBReserved                          = 0,
    EMTPPBCollectionErrMsg                  = 1,
    EMTPPBCallbackInvalid                   = 2,
    EMTPPBCollectionErrCall                 = 3,
    EMTPPBDataTypeErr                       = 4,
    EMTPPBArgumentErr                       = 5,
    EMTPPBDataNullErr                       = 6,
    };

inline void Panic( TMTPPlaybackControlPanic aCode )
    {
    User::Panic( _L("MTPPlaybackControl"), aCode );
    };

#endif //MTPPLAYBACKCONTROLPANIC_H
