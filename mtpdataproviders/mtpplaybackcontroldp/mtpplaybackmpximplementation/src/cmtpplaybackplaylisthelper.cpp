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

#include <mpxmediamusicdefs.h>
#include <mpxmediacontainerdefs.h>
#include <mpxmessagegeneraldefs.h>
#include <pathinfo.h>

#include <mpxcollectionhelper.h>
#include <mpxcollectionhelperfactory.h>
#include <mpxcollectionuihelper.h>
#include <mpxcollectionhelperfactory.h>

#include <mpxcollectionutility.h>
#include <mpxcollectionplaylist.h>
#include <mpxcollectionmessage.h>
#include <mpxcollectionpath.h>

#include "cmtpplaybackcontrolimpl.h"
#include "cmtpplaybackplaylisthelper.h"
#include "mtpplaybackcontrolpanic.h"

// Constants
__FLOG_STMT(_LIT8(KComponent,"PlaybackPlaylistHelper");)

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackPlaylistHelper* CMTPPlaybackPlaylistHelper::NewL( CMTPPlaybackControlImpl& aControlImpl )
    {
    CMTPPlaybackPlaylistHelper* self = new ( ELeave ) 
                CMTPPlaybackPlaylistHelper( aControlImpl );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper"));
    
    if( iCollectionUiHelper )
        {
        iCollectionUiHelper->Close();
        }
    
    if ( iCollectionHelper )
        {
        iCollectionHelper->Close();
        }
    
    if( iCollectionUtil ) 
        {
        iCollectionUtil->Close();
        }
    
    delete iPlayObject;
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper"));
    __FLOG_CLOSE;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL
//// Get Playlist via aMedia
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL( const TMTPPbDataSuid& aPlayObject )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    
    //Reset
    ResetPlaySource();
    
    iPlayCategory = aPlayObject.Category();
    iPlayObject = aPlayObject.Suid().AllocL();
    
    switch ( iPlayCategory )
        {
        case EMTPPbCatPlayList:
            {
            OpenMusicPlayListPathL();
            }
            break;
        case EMTPPbCatAlbum:
            {
            OpenMusicAblumPathL();
            }
            break;
        default:
            {
            Panic( EMTPPBCollectionErrCall );
            }
            break;
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL
// Get Playlist via index
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL( TInt aIndex )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    
    iSongIndex = aIndex;
    
    UpdatePathAndOpenL();
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetMTPPBSuidFromCollectionL
// ---------------------------------------------------------------------------
//
TMTPPbDataSuid CMTPPlaybackPlaylistHelper::GetMTPPBSuidFromCollectionL( 
        const CMPXCollectionPlaylist& aPlaylist )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    
    CMPXCollectionPath* path = iCollectionUiHelper->MusicPlaylistPathL();
    if ( path->Id() == aPlaylist.Path().Id( KMTPPlaybackPlaylistAblumLevel -1 ))
        {
        iPlayCategory = EMTPPbCatPlayList;
        }
    else
        {
        iPlayCategory = EMTPPbCatAlbum;
        }
    TFileName uri = ItemIdToUriL( aPlaylist.Path().Id( KMTPPlaybackPlaylistAblumLevel ));
    TMTPPbDataSuid dataSuid( iPlayCategory, uri );
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL"));
    return dataSuid;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPbCategory
// ---------------------------------------------------------------------------
//
TMTPPbCategory CMTPPlaybackPlaylistHelper::MTPPbCategory() const
    {
    return iPlayCategory;
    }
// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPbSuid
// ---------------------------------------------------------------------------
//
TFileName CMTPPlaybackPlaylistHelper::MTPPbSuid() const
    {
    return TFileName( *iPlayObject );
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// Handle completion of a asynchronous command
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::HandleCollectionMessage( CMPXMessage* aMsg, TInt aErr )
    {
    __FLOG_1(_L8("+CMTPPlaybackPlaylistHelper::HandleCollectionMessage( %d ) "), aErr );

    if (( KErrNone == aErr ) && aMsg )
        {
        TRAP( aErr, DoHandleCollectionMessageL( *aMsg ));
        }
    
    if ( KErrNone != aErr )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aErr );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::HandleCollectionMessage"));
    }
// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// ---------------------------------------------------------------------------  
//
void CMTPPlaybackPlaylistHelper::HandleOpenL( const CMPXMedia& aEntries, 
        TInt /*aIndex*/, TBool /*aComplete*/, TInt aError )
    {
    __FLOG_1(_L8("+CMTPPlaybackPlaylistHelper::HandleOpenL( %d )"), aError );
    
    if ( KErrNone == aError )
        {
        TRAP( aError, DoHandleOpenL( aEntries ));
        }
    
    if ( KErrNone != aError )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aError );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::HandleOpenL"));
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// ---------------------------------------------------------------------------  
//
void CMTPPlaybackPlaylistHelper::HandleOpenL( const CMPXCollectionPlaylist& aPlaylist,
        TInt aError )
    {
    __FLOG_1(_L8("+CMTPPlaybackPlaylistHelper::HandleOpenL( aPlaylist, aError = %d )"), aError );
    
    if ( KErrNone == aError )
        {
        TRAP( aError, MTPPlaybackControlImpl().GetPlaylistFromCollectionCompleteL( aPlaylist ));
        }
    
    if ( KErrNone != aError )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aError );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::HandleOpenL( aPlaylist, aError )"));
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionMediaObserver
// ---------------------------------------------------------------------------
void CMTPPlaybackPlaylistHelper::HandleCollectionMediaL( const CMPXMedia& /*aMedia*/, TInt /*aError*/ )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::HandleCollectionMediaL"));
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::HandleCollectionMediaL"));
    }

//
// CMTPPlaybackPlaylistHelper::CMTPPlaybackPlaylistHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackPlaylistHelper::CMTPPlaybackPlaylistHelper( CMTPPlaybackControlImpl& aControlImpl )
        : iCollectionUiHelper( NULL ),
          iCollectionHelper( NULL ),
          iCollectionUtil( NULL ),
          iPlayObject( NULL ),
          iMTPPlaybackControl( aControlImpl )
    {
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::ConstructL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::ConstructL"));
    
    iCollectionUiHelper = CMPXCollectionHelperFactory::NewCollectionUiHelperL();
    iCollectionUtil = MMPXCollectionUtility::NewL( this, KMcModeDefault );
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::ConstructL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::DoHandleCollectionMessage
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::DoHandleCollectionMessageL( const CMPXMessage& aMsg )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::DoHandleCollectionMessage"));
    
    TMPXMessageId id( aMsg.ValueTObjectL<TMPXMessageId>( KMPXMessageGeneralId ) );

    if ( KMPXMessageGeneral == id )
        {
        TInt event( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralEvent ) );
        TInt type( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralType ) );
        TInt data( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralData ) );
        
        __FLOG_VA((_L8("Event code is 0x%X, type code is 0x%X"), event, type ));
        __FLOG_1(_L8("Data code is 0x%X"), data );
        
        if ( event == TMPXCollectionMessage::EPathChanged &&
             type == EMcPathChangedByOpen && 
             data == EMcContainerOpened )
            {
            iCollectionUtil->Collection().OpenL();
            }
        else if ( event == TMPXCollectionMessage::EPathChanged &&
                  type == EMcPathChangedByOpen &&
                  data == EMcItemOpened )
            {
            iCollectionUtil->Collection().OpenL();
            }
        else if ( event == TMPXCollectionMessage::ECollectionChanged )
            {
            __FLOG(_L8("Ignore this event"));
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::DoHandleCollectionMessage"));
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::DoHandleOpenL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::DoHandleOpenL( const CMPXMedia& aEntries )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::DoHandleOpenL( const CMPXMedia )"));
    
    if ( EMTPPbCatAlbum == iPlayCategory )
        {
        UpdateAlbumPathAndOpenL();
        }
    else
        {
        //playlist
        if ( -1 == iPathIndex )
            {
            //first, the top path
            UpdatePlaylistPathIndexL( aEntries );
                
            if ( -1 == iPathIndex )
                {
                MTPPlaybackControlImpl().DoHandleError( KPlaybackErrParamInvalid );
                }
            else
                {
                iCollectionUtil->Collection().OpenL( iPathIndex );
                }
                }
        else
            {
            //open the first song when initObject
            iCollectionUtil->Collection().OpenL( iSongIndex );
            }
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::DoHandleOpenL( const CMPXMedia )"));
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL"));
    
    CMPXCollectionPath* path = iCollectionUiHelper->MusicPlaylistPathL();
    CleanupStack::PushL( path );
    iCollectionUtil->Collection().OpenL( *path );
    CleanupStack::PopAndDestroy( path );
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL"));
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL"));
    
    CMPXCollectionPath* path = iCollectionUiHelper->MusicMenuPathL();
    CleanupStack::PushL( path );
    path->AppendL(KMPXCollectionArtistAlbum);
    iCollectionUtil->Collection().OpenL( *path );
    CleanupStack::PopAndDestroy( path );
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL"));
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::ResetPlaySource
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::ResetPlaySource()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::ResetPlaySourceL"));
    
    iPathIndex = -1;
    iSongIndex = 0;
    delete iPlayObject;
    iPlayObject = NULL;
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::ResetPlaySourceL"));
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL( const CMPXMedia& aEntries )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL"));
    __ASSERT_DEBUG( iPlayCategory == EMTPPbCatPlayList, Panic( EMTPPBCollectionErrCall ));
    
    const CMPXMediaArray* refArray = aEntries.Value<CMPXMediaArray> ( KMPXMediaArrayContents );
    TInt count = refArray->Count();
    const TMPXItemId playlistId = UriToItemIdL();
    
    for ( TInt i=0; i<count; ++i )
        {
        CMPXMedia* container = refArray->AtL(i);
        /**
         * Try to find out the next path according to the 
         * playlist's ItemId
         */
        if ( container->IsSupported( KMPXMediaGeneralId ))
            {
            const TMPXItemId tempId = container->ValueTObjectL<TMPXItemId>(KMPXMediaGeneralId);
            if ( tempId == playlistId )
                {
                iPathIndex = i;
                break;
                }
            }
       }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL"));
    }

// ---------------------------------------------------------------------------
// return instance of CollectionHelper.
// ---------------------------------------------------------------------------
//
MMPXCollectionHelper* CMTPPlaybackPlaylistHelper::CollectionHelperL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::CollectionHelperL"));
    
    if ( iCollectionHelper == NULL )
        {
        iCollectionHelper = CMPXCollectionHelperFactory::NewCollectionCachedHelperL();
            
        // Do a search for a song ID that does not exist
        RArray<TInt> contentIDs;
        CleanupClosePushL( contentIDs ); // + contentIDs
        contentIDs.AppendL( KMPXMediaIdGeneral );
            
        CMPXMedia* searchMedia = CMPXMedia::NewL( contentIDs.Array() );
        CleanupStack::PopAndDestroy( &contentIDs ); // - contentIDs
        CleanupStack::PushL( searchMedia ); // + searchMedia
            
        searchMedia->SetTObjectValueL( KMPXMediaGeneralType, EMPXItem );
        searchMedia->SetTObjectValueL( KMPXMediaGeneralCategory, EMPXSong );
        searchMedia->SetTObjectValueL<TMPXItemId>( KMPXMediaGeneralId,
                KMTPPlaybackInvalidSongID );
            
        /*
        * store root
        */
        TChar driveChar = 'c';
        TInt driveNumber;
        User::LeaveIfError( RFs::CharToDrive( driveChar, driveNumber ) );
            
        // get root path
        TBuf<KStorageRootMaxLength> storeRoot;
        User::LeaveIfError( PathInfo::GetRootPath( storeRoot, driveNumber ) );
            
        searchMedia->SetTextValueL( KMPXMediaGeneralDrive, storeRoot );
            
        RArray<TMPXAttribute> songAttributes;
        CleanupClosePushL( songAttributes ); // + songAttributes
        songAttributes.AppendL( KMPXMediaGeneralId );
            
        CMPXMedia* foundMedia = NULL;
        TRAPD( err, foundMedia = iCollectionHelper->FindAllL(
                *searchMedia,
                songAttributes.Array() ) );
            
        CleanupStack::PopAndDestroy( &songAttributes ); // - songAttributes
        CleanupStack::PopAndDestroy( searchMedia ); // - searchMedia
            
        CleanupStack::PushL( foundMedia ); // + foundMedia
            
        if ( err != KErrNone )
            {
            iCollectionHelper->Close();
            iCollectionHelper = NULL;
            User::Leave( KErrGeneral );
            }
        CleanupStack::PopAndDestroy( foundMedia ); // - foundMedia
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::CollectionHelperL"));
    return iCollectionHelper;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UriToItemIdL
// ---------------------------------------------------------------------------
//
const TMPXItemId CMTPPlaybackPlaylistHelper::UriToItemIdL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::UriToItemIdL"));
    

    TMPXItemId itemId( KMPXInvalidItemId );
    TInt error = KErrNone;
    CMPXMedia* result = NULL;
    
    RArray<TMPXAttribute> atts; 
    CleanupClosePushL( atts );
    atts.AppendL( KMPXMediaGeneralId );
    
    if ( EMTPPbCatPlayList == iPlayCategory )
        {
        TRAP( error, result = CollectionHelperL()->GetL( *iPlayObject, atts.Array(), EMPXPlaylist ));
        }
    else
        {
        TRAP( error, result = CollectionHelperL()->GetL( *iPlayObject, atts.Array(), EMPXAbstractAlbum ));
        }
    
    if ( error != KErrNone )
        {
        CleanupStack::PopAndDestroy( &atts );
        }
    else
        {
        CleanupStack::PushL( result );
        itemId = result->ValueTObjectL<TMPXItemId>(KMPXMediaGeneralId);
        CleanupStack::PopAndDestroy( result );
        CleanupStack::PopAndDestroy( &atts );
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::UriToItemIdL"));
    return itemId;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::ItemIdToUriL.
// ---------------------------------------------------------------------------
//
const TFileName CMTPPlaybackPlaylistHelper::ItemIdToUriL( const TMPXItemId& aId )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::ItemIdToUriL"));
    
    TFileName itemUri( KNullDesC );
    
    RArray<TInt> contentIDs;
    CleanupClosePushL( contentIDs ); // + contentIDs
    contentIDs.AppendL( KMPXMediaIdGeneral );
    
    CMPXMedia* searchMedia = CMPXMedia::NewL( contentIDs.Array() );
    CleanupStack::PopAndDestroy( &contentIDs ); // - contentIDs
    CleanupStack::PushL( searchMedia ); // + searchMedia
        
    searchMedia->SetTObjectValueL( KMPXMediaGeneralType, EMPXItem );
    if ( iPlayCategory == EMTPPbCatPlayList )
        {
        searchMedia->SetTObjectValueL( KMPXMediaGeneralCategory, EMPXPlaylist );
        }
    else
        {
        searchMedia->SetTObjectValueL( KMPXMediaGeneralCategory, EMPXAbstractAlbum );
        }
    searchMedia->SetTObjectValueL<TMPXItemId>( KMPXMediaGeneralId, aId );
    
    RArray<TMPXAttribute> resultAttributes;
    CleanupClosePushL( resultAttributes ); // + resultAttributes
    resultAttributes.AppendL( KMPXMediaGeneralUri );
    
    CMPXMedia* foundMedia = CollectionHelperL()->FindAllL(
                    *searchMedia,
                    resultAttributes.Array() );
                    
    CleanupStack::PopAndDestroy( &resultAttributes ); // - resultAttributes
    CleanupStack::PopAndDestroy( searchMedia ); // - searchMedia
    
    CleanupStack::PushL( foundMedia ); // + foundMedia
    if ( !foundMedia->IsSupported( KMPXMediaArrayCount ))
        {
        User::Leave( KErrNotSupported );
        }
    else if ( *foundMedia->Value<TInt>( KMPXMediaArrayCount ) != 1 )
        {
        User::Leave( KErrNotSupported );
        }
    
    const CMPXMediaArray* tracksArray = foundMedia->Value<CMPXMediaArray> ( KMPXMediaArrayContents );
    CMPXMedia* item = tracksArray->AtL(0);
    
    if ( item->IsSupported( KMPXMediaGeneralUri ))
        {
        itemUri = item->ValueText(KMPXMediaGeneralUri);
        }
    
    CleanupStack::PopAndDestroy( foundMedia ); // - foundMedia
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::ItemIdToUriL"));
    return itemUri;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::FindAlbumSongsL
// ---------------------------------------------------------------------------
//
CMPXMedia* CMTPPlaybackPlaylistHelper::FindAlbumSongsL( const TMPXItemId& aAlbumId )
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::FindAlbumSongsL"));
    
    // Fetch the songs for the selected album
    CMPXMedia* findCriteria = CMPXMedia::NewL();
    CleanupStack::PushL( findCriteria );
    findCriteria->SetTObjectValueL<TMPXGeneralType>( KMPXMediaGeneralType, EMPXGroup );
    findCriteria->SetTObjectValueL<TMPXGeneralCategory>( KMPXMediaGeneralCategory, EMPXSong );
    findCriteria->SetTObjectValueL<TMPXItemId>( KMPXMediaGeneralId, aAlbumId );
    RArray<TMPXAttribute> attrs;
    CleanupClosePushL( attrs );
    attrs.Append( TMPXAttribute( KMPXMediaIdGeneral,
                                 EMPXMediaGeneralTitle |
                                 EMPXMediaGeneralId ) );
    attrs.Append( KMPXMediaMusicAlbumTrack );
    
    CMPXMedia* foundMedia = CollectionHelperL()->FindAllL( *findCriteria,
            attrs.Array() );
    CleanupStack::PopAndDestroy( &attrs );
    CleanupStack::PopAndDestroy( findCriteria );
    
    if ( !foundMedia->IsSupported( KMPXMediaArrayCount ) )
        {
        User::Leave( KErrNotSupported );
        }
    TInt foundItemCount = *foundMedia->Value<TInt>( KMPXMediaArrayCount );
    if ( foundItemCount == 0 )
        {
        User::Leave( KErrNotFound );
        }
    if ( !foundMedia->IsSupported( KMPXMediaArrayContents ) )
        {
        User::Leave( KErrNotSupported );
        }
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::FindAlbumSongsL"));
    return foundMedia;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL.
// aParam: const CMPXMedia& aAlbums
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdateAlbumPathAndOpenL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::UpdateAlbumPathAndOpenL"));
    
    RArray<TMPXItemId> ids;
    CleanupClosePushL(ids);
    
    CMPXCollectionPath* cpath = iCollectionUtil->Collection().PathL();
    CleanupStack::PushL( cpath );
    
    if (cpath->Levels() == 3)
        {
        // go back one level before amending path with new levels
        cpath->Back();
        }
    
    const TMPXItemId id = UriToItemIdL();
    if ( KMPXInvalidItemId == id )
        {
        MTPPlaybackControlImpl().DoHandleError( KPlaybackErrParamInvalid );
        CleanupStack::PopAndDestroy( cpath );
        CleanupStack::PopAndDestroy(&ids);
        return;
        }
    
    ids.AppendL(id);
    cpath->AppendL( ids.Array() ); // top level items
    cpath->Set( 0 );
    ids.Reset();
    
    CMPXMedia* songs = FindAlbumSongsL( id );
    CleanupStack::PushL( songs );
    const CMPXMediaArray* tracksArray = songs->Value<CMPXMediaArray> ( KMPXMediaArrayContents );
    User::LeaveIfNull(const_cast<CMPXMediaArray*>(tracksArray));
    TUint count = tracksArray->Count();
    for (TInt i=0; i<count; ++i)
        {
        CMPXMedia* song = tracksArray->AtL(i);
        const TMPXItemId id = song->ValueTObjectL<TMPXItemId>(KMPXMediaGeneralId);
        ids.AppendL(id);
        }

    cpath->AppendL(ids.Array()); // top level items
    cpath->Set( iSongIndex );
    
    iCollectionUtil->Collection().OpenL(*cpath);
    CleanupStack::PopAndDestroy( songs );    
    CleanupStack::PopAndDestroy( cpath );
    CleanupStack::PopAndDestroy(&ids);
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::UpdateAlbumPathAndOpenL"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL()
    {
    __FLOG(_L8("+CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL()"));
    
    RArray<TMPXItemId> ids;
    CleanupClosePushL(ids);
    
    CMPXCollectionPath* cpath = iCollectionUtil->Collection().PathL();
    CleanupStack::PushL( cpath );
    
    cpath->Set( iSongIndex );
    
    iCollectionUtil->Collection().OpenL(*cpath);  
    CleanupStack::PopAndDestroy( cpath );
    CleanupStack::PopAndDestroy(&ids);
    
    __FLOG(_L8("-CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL( aSong Index )"));
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPlaybackControlImpl.
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackPlaylistHelper::MTPPlaybackControlImpl()
    {
    return iMTPPlaybackControl;
    }


