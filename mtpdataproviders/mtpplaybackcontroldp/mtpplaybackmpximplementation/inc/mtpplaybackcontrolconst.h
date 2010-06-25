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

#ifndef MTPPLAYBACKCONTROLCONST_H_
#define MTPPLAYBACKCONTROLCONST_H_

#include <mpxplaybackutility.h>

/**
Used by test db existed
*/
const TInt KMTPPlaybackInvalidSongID = 0x1FFFFFFF;
const TInt KStorageRootMaxLength = 10;

/**
The volume step of mtp playback
*/
const TUint32 KMTPPlaybackVolumeStep = 1;

/**
The playlist or album level of collection path 
*/
const TInt KMTPPlaybackPlaylistAblumLevel = 2;

/**
The album id of collection path 
*/
const TInt KMPXCollectionArtistAlbum = 3;

/**
iMPXCommand is the mpx command
iMPXExpectState is the expected state
*/
struct TMPXComandElement
    {
    TMPXPlaybackCommand   iMPXCommand;
    TMPXPlaybackState     iMPXExpectState;
    };  

typedef RArray<TMPXComandElement> RResumeCmdArray;

/**
The uid of playback control dp
*/
const TUid  KMTPPlaybackControlDpUid = { 0x2002EA9A };

#endif /* MTPPLAYBACKCONTROLCONST_H_ */
