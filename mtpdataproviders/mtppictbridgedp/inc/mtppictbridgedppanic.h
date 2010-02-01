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

/**
 @file
 @internalComponent
 */


#ifndef MTPPICTBRIDGEDPPANIC_H
#define MTPPICTBRIDGEDPPANIC_H

#include <e32std.h>

/**
define the file data provider panic code
@internalTechnology

*/
enum TMTPPictBridgeDpPanic
    {
    EMTPPictBridgeDpNoMatchingProcessor = 0,
    EMTPPictBridgeDpObjectNull = 1,
    EMTPPictBridgeDpFormatNotSupported = 2,
    EMTPPictBridgeDpSendObjectStateInvalid = 3,
    EMTPPictBridgeDpRequestCheckNull = 4,
    EMTPPictBridgeDpHandlesNull = 5,
    EMTPPictBridgeDpUnsupportedProperty = 6,
    EMTPPictBridgeDpObjectPropertyNull = 7,
    EMTPPictBridgeDpCannotRenameDir = 8,
    };

_LIT(KMTPPictBridgeDpPanic, "MTP PictBridge Data Provider");

inline void Panic(TMTPPictBridgeDpPanic aCode)
    {
    User::Panic(KMTPPictBridgeDpPanic, aCode);
    };
#endif // MTPPICTBRIDGEPANIC_H

