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

#ifndef CMTPPLAYBACKCONTROLIMPL_H
#define CMTPPLAYBACKCONTROLIMPL_H

#include <mpxplaybackobserver.h>

#include "mmtpplaybackinterface.h"
#include "mtpplaybackcontrolconst.h"

class MMPXPlaybackUtility;
class CMPXCollectionPath;
class CMPXCollectionPlaylist;

class CMTPPlaybackCommandChecker;
class CMTPPlaybackPlaylistHelper;
class CMTPPlaybackResumeHelper;
class CMTPPlaybackCommand;
class CMTPPbCmdParam;


NONSHARABLE_CLASS( CMTPPlaybackControlImpl ) : public CActive,
                                               public MMPXPlaybackObserver,
                                               public MMPXPlaybackCallback,
                                               public MMTPPlaybackControl
    {
private:

    friend class CMTPPlaybackCommandChecker;
    friend class CMTPPlaybackPlaylistHelper;
    friend class CMTPPlaybackResumeHelper;
    
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CMTPPlaybackControlImpl* NewL( MMTPPlaybackObserver& aObserver );
    /**
    *  Frees resource, and destroy the object itself.
    */
    void Close();
    /**
     * Destructor.
     */
    virtual ~CMTPPlaybackControlImpl();
    
public:    
    /**
     * From MMTPPlaybackControl
     * @param aCmd, refer to CMTPPlaybackCommand
     * @param aCallback, refer to MMTPPlaybackCallback
     */
    void CommandL( CMTPPlaybackCommand& aCmd, 
            MMTPPlaybackCallback* aCallback = NULL);
    
private:
	
    /**
     * From MMPXPlaybackObserver
     * See mpxplaybackobserver.h for detailed description.
     */
    void HandlePlaybackMessage( CMPXMessage* aMessage, TInt aError );
    
    /**
     * From MMPXPlaybackCallback
     * See mpxplaybackobserver.h for detailed description.
     */
    void HandlePropertyL( TMPXPlaybackProperty aProperty, TInt aValue, TInt aError );
    
    /**
     * From MMPXPlaybackCallback
     * See mpxplaybackobserver.h for detailed description.
     */
    void HandleSubPlayerNamesL( TUid aPlayer, const MDesCArray* aSubPlayers, TBool aComplete, TInt aError );
    
    /**
     * From MMPXPlaybackCallback
     * See mpxplaybackobserver.h for detailed description.
     */
    void HandleMediaL( const CMPXMedia& aMedia, TInt aError );
    
private:// From CActive
    
    /**
     * DoCancel.
     */
    void DoCancel();
    
    /**
     * RunL.
     */
    void RunL();
    
private:
    
    /**
     * C++ default constructor.
     */
    CMTPPlaybackControlImpl( MMTPPlaybackObserver& aObserver );
    
    /**
     * By default Symbian 2nd phase constructor is private.
     */
    void ConstructL();
    
private:
    
    /**
     * Get Playlist from collection complete..
     */
    void GetPlaylistFromCollectionCompleteL( const CMPXCollectionPlaylist& aPlaylist );
    
    /**
     * Deactive other player so that we can play
     */
    void DeActiveOtherPlayerL();
    
    /**
     * Check playback command and cache
     */
    void CheckPlaybackCmdAndCacheL( CMTPPlaybackCommand& aCmd );
    
    /**
     * UpdateCommandArrary
     */
    void UpdateCommandArray();
    
    /**
     * Require current media
     */
    void RequestMediaL();
    
    /**
     * Handle commandL
     */
    void DoCommandL();
    
    /**
     *  Handle playback message
     *
     *  @param aMessage playback message
     */
    void DoHandlePlaybackMessageL( const CMPXMessage& aMessage );
    
    /**
     *  Handle playback property
     *
     *  @param aProperty the property
     *  @param aValue the value of the property
     */
    void DoHandlePropertyL( TInt aProperty, TInt aValue );

    /**
     *  Handle playback state changed.
     *
     *  @param aState New Playback state
     */
    void DoHandleStateChangedL( TMPXPlaybackState aState );
    
    /**
     *  Handle request media.
     */
    void DoHandleMediaL( const CMPXMedia& aMedia );
    
    /**
     *  Handle Media changed.
     *
     */
    void DoHandleMediaChangedL();
    
    /**
     *  Handle Initialize Complete.
     *
     */
    void DoHandleInitializeCompleteL();
    
    /**
     *  Handle error.
     *
     */
    void DoHandleError( TInt aErr );
    
    /**
     * Compare two path
     * @param aPathBase the base path
     * @param aPathNew the new path
     * @param aLevel compare level
     */
    TBool IfEqual( const CMPXCollectionPath& aPathBase, const CMPXCollectionPath& aPathNew, TUint aLevel );
    
    /**
    *  Map states from TMPXPlaybackState to TMPlayerState
    *
    *  @param aState State in TMPXPlaybackState format
    *  @return State in TMPlayerState format
    */
    TMTPPlaybackState MapState( TMPXPlaybackState aState );
    
    /**
    *  Map error from TInt to the error defined in interface
    */
    TInt MapError( TInt error );
    
    /**
     *Complete a sync request
    */
    void CompleteSelf( TInt aCompletionCode );
    
    /*
     * Initiate MPX Playback Command
     */
    void InitiateMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility );
    
    /*
     * SendMPXPlaybackCommandL
     */
    void SendMPXPlaybackCommandL( TMPXPlaybackCommand aCommand, TBool aIsMTPPlaybackUtility );
    
    
    /*
     * SendPlaybackCommandCompleteL
     */
    void SendPlaybackCommandCompleteL();
    
    /*
     * SendPlaybackEventL
     */
    void SendPlaybackEventL( TMTPPlaybackEvent aEvt );
    
    /**
     * Reset playback command
     */
    void ResetPlaybackCommand();
    
private:
    
    /**
     * return current state.
     */
    TMPXPlaybackState CurrentState() const;
    
    /**
     * return previous state.
     */
    TMPXPlaybackState PreviousState() const;
    
    /**
     * return song count.
     */
    TInt32 SongCount() const;
    
    /**
     * return song index.
     */
    TInt32 SongIndex() const;
    
    /**
     * Cache mtp playback command.
     */
    void SetMTPPBCmd( TMTPPlaybackCommand aMTPPBCmd );
    
    /**
     * return mtp playback command.
     */
    TMTPPlaybackCommand MTPPBCmdHandling() const;
    
private:
    
    MMTPPlaybackObserver*        iObserver;// Not owned
    MMTPPlaybackCallback*        iCallback;// Not owned
    
    MMPXPlaybackUtility*         iPlaybackUtility;// Owned
    MMPXPlaybackUtility*         iNowActivePlaybackUtility;// Owned
    
    CMTPPlaybackCommandChecker*  iPlaybackCommandChecker;//Owned
    CMTPPlaybackPlaylistHelper*  iPlaybackPlaylistHelper;//Owned
    CMTPPlaybackResumeHelper*    iPlaybackResumeHelper;
    
    CMPXCollectionPlaylist*      iPlayList;   //Owned
    RResumeCmdArray              iPrepareCmdArray;//Owned
    RResumeCmdArray              iResumeCmdArray;//Owned
    CMTPPbCmdParam*              iCmdParam;
    
    TMTPPlaybackCommand          iMTPPBCmd;
    TMPXPlaybackState            iPreState;
    TMPXPlaybackState            iState;
    };

#endif // CMTPPLAYBACKCONTROLIMPL_H
// End of File
