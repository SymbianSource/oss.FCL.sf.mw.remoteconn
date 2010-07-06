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

#ifndef CMTPPLAYBACKPLAYLISTHELPER_H_
#define CMTPPLAYBACKPLAYLISTHELPER_H_

#include <mpxcollectionobserver.h>
#include <mpxcollectionuihelperobserver.h>

#include "mtpdebug.h"
#include "cmtpplaybackparam.h"

class MMPXCollectionUiHelper;
class MMPXCollectionHelper;
class MMPXCollectionUtility;

class TMTPPbDataSuid;
class CMTPPlaybackControlImpl;

NONSHARABLE_CLASS( CMTPPlaybackPlaylistHelper ) : public CBase,
                                                  public MMPXCollectionObserver
    {
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CMTPPlaybackPlaylistHelper* NewL( CMTPPlaybackControlImpl& aControlImpl );

    /**
     * Destructor.
     */
    virtual ~CMTPPlaybackPlaylistHelper();
    
public:
    
    /**
     * Get playlist from collection server
     * @param aPlayObject, refer to TMTPPbDataSuid
     */
    void GetPlayListFromCollectionL( const TMTPPbDataSuid& aPlayObject );
    
    /**
     * Get playlist from collection server
     * @param aIndex, song index
     */
    void GetPlayListFromCollectionL( TInt aIndex );
    
    /**
     * Get suid from collection server
     * @param aPlaylist, current collection playlist
     */
    TMTPPbDataSuid GetMTPPBSuidFromCollectionL( 
            const CMPXCollectionPlaylist& aPlaylist );
    
    /**
     * Get category
     */
    TMTPPbCategory MTPPbCategory() const;
    
    /**
     * Get suid
     */
    TFileName MTPPbSuid() const;
    
private:
    
    //from base class MMPXCollectionObserver  
    /** 
     *  Handle collection message
     * 
     *  @param aMsg collection message, ownership not transferred. 
     *         Please check aMsg is not NULL before using it. 
     *         If aErr is not KErrNone, plugin might still call back with more
     *         info in the aMsg.
     *  @param aErr system error code.
     */
    void HandleCollectionMessage( CMPXMessage* aMsg, TInt aErr );
    
    /**
     *  Handles the collection entries being opened. Typically called
     *  when client has Open()'d a folder.
     *
     *  @param aEntries collection entries opened
     *  @param aIndex focused entry
     *  @param aComplete ETrue no more entries. EFalse more entries
     *                   expected
     *  @param aError error code   
     */
    void HandleOpenL( const CMPXMedia& aEntries, TInt aIndex, TBool aComplete,
              TInt aError );
    
    /**
     *  Handles the item being opened. Typically called
     *  when client has Open()'d an item. Client typically responds by
     *  'playing' the item via the playlist.
     *
     *  @param aPlaylist collection playlist
     *  @param aError error code
     */
    void HandleOpenL( const CMPXCollectionPlaylist& aPlaylist, TInt aError );
    
    /**
     * From MMPXCollectionMediaObserver 
     * It handles MediaL event.
     *
     * @param aMedia object containing collection's informations
     * @param aError error code
     */
    void HandleCollectionMediaL( const CMPXMedia& aMedia, TInt aError );

private:
    
    /**
     * C++ default constructor.
     */
    CMTPPlaybackPlaylistHelper( CMTPPlaybackControlImpl& aControlImpl );
    
    /**
     * By default Symbian 2nd phase constructor is private.
     */
    void ConstructL();
    
private:
    
    /**
     * Handle collection message
     */
    void DoHandleCollectionMessageL( const CMPXMessage& aMsg );
    
    /**
     * Handle OpenL
     */
    void DoHandleOpenL( const CMPXMedia& aEntries );
    
    /**
     * Open the root path of playlist
     */
    void OpenMusicPlayListPathL();
    
    /**
     * Open the root path of album
     */
    void OpenMusicAblumPathL();
    
    /**
     * Reset play source
     */
    void ResetPlaySource();
    
    /**
    *Update path index according to iPlayObject
    */
    void UpdatePlaylistPathIndexL( const CMPXMedia& aEntries );
    
    /**
    *Got the collectionHelperL
    */
    MMPXCollectionHelper* CollectionHelperL();
    
    /**
     * Map uri to ItemId
     */
    const TMPXItemId UriToItemIdL();
    
    /**
     * Map uri to ItemId
     */
    const TFileName ItemIdToUriL( const TMPXItemId& aId );
    
    /**
     * Find Album Songs
     */
    CMPXMedia* FindAlbumSongsL( const TMPXItemId& aAlbumId );
    
    /*
     * UpdatePathAndOpenL
     */
    void UpdateAlbumPathAndOpenL();
    
    /*
     * UpdatePathAndOpenL
     */
    void UpdatePathAndOpenL();
    
    /**
     *  Return instance of CMTPPlaybackControlImpl.
     *
     */
    CMTPPlaybackControlImpl& MTPPlaybackControlImpl();
    
private:
    
    MMPXCollectionUiHelper*              iCollectionUiHelper;//Owned
    MMPXCollectionHelper*                iCollectionHelper;//Owned
    MMPXCollectionUtility*               iCollectionUtil;//Owned
    HBufC*                               iPlayObject;  //Owned
    
    CMTPPlaybackControlImpl&             iMTPPlaybackControl;// Not owned
    
    TMTPPbCategory                       iPlayCategory;
    TInt                                 iSongIndex;
    TInt                                 iPathIndex;

    /**
    FLOGGER debug trace member variable.
     */
    __FLOG_DECLARATION_MEMBER;
    };

#endif /* CMTPPLAYBACKPLAYLISTHELPER_H_ */
