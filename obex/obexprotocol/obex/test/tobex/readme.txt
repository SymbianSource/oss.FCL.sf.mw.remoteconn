/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/

TObex - Readme.txt


TObex provides provision for the existance of a Win32 emulator USB transport plugin, 
together with a Symbian Win32 USB driver. 
These would allow TObex to use USB when running on the emulator.
However NEITHER a Win32 emulator USB transport plugin NOR a Symbian Win32 USB driver 
are currently available. 
An attempt to load USB when running TObex on the emulator will result in a 
KErrNotFound error being posted.