/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Defines the LCD Bearer Plugin identifier.
*
*/


#ifndef T_LOCODBEARER_H
#define T_LOCODBEARER_H

#include <e32base.h>

/**  bearer value in LC */
enum TLocodBearer
    {
    ELocodBearerBT =  0x0001,
	ELocodBearerIR =  0x0010,
	ELocodBearerUSB = 0x0100,
    };

#endif // T_LOCODBEARER_H
