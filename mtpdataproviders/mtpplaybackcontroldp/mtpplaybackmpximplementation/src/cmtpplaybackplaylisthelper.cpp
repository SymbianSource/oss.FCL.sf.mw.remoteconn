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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackplaylisthelperTraces.h"
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::NewL
// ---------------------------------------------------------------------------
//
CMTPPlaybackPlaylistHelper* CMTPPlaybackPlaylistHelper::NewL( CMTPPlaybackControlImpl& aControlImpl )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_NEWL_ENTRY );
    
    CMTPPlaybackPlaylistHelper* self = new ( ELeave ) 
                CMTPPlaybackPlaylistHelper( aControlImpl );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_NEWL_EXIT );
    return self;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper
// ---------------------------------------------------------------------------
//
CMTPPlaybackPlaylistHelper::~CMTPPlaybackPlaylistHelper()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_CMTPPLAYBACKPLAYLISTHELPER_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_CMTPPLAYBACKPLAYLISTHELPER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL
//// Get Playlist via aMedia
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL( const TMTPPbDataSuid& aPlayObject )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_GETPLAYLISTFROMCOLLECTIONL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_GETPLAYLISTFROMCOLLECTIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL
// Get Playlist via index
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::GetPlayListFromCollectionL( TInt aIndex )
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_GETPLAYLISTFROMCOLLECTIONL_ENTRY );
    
    iSongIndex = aIndex;
    
    UpdatePathAndOpenL();
    
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_GETPLAYLISTFROMCOLLECTIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::GetMTPPBSuidFromCollectionL
// ---------------------------------------------------------------------------
//
TMTPPbDataSuid CMTPPlaybackPlaylistHelper::GetMTPPBSuidFromCollectionL( 
        const CMPXCollectionPlaylist& aPlaylist )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_GETMTPPBSUIDFROMCOLLECTIONL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_GETMTPPBSUIDFROMCOLLECTIONL_EXIT );
    return dataSuid;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPbCategory
// ---------------------------------------------------------------------------
//
TMTPPbCategory CMTPPlaybackPlaylistHelper::MTPPbCategory() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_MTPPBCATEGORY_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_MTPPBCATEGORY_EXIT );
    return iPlayCategory;
    }
// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPbSuid
// ---------------------------------------------------------------------------
//
TFileName CMTPPlaybackPlaylistHelper::MTPPbSuid() const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_MTPPBSUID_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_MTPPBSUID_EXIT );
    return TFileName( *iPlayObject );
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// Handle completion of a asynchronous command
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::HandleCollectionMessage( CMPXMessage* aMsg, TInt aErr )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_HANDLECOLLECTIONMESSAGE_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPLAYBACKPLAYLISTHELPER_HANDLECOLLECTIONMESSAGE, "CMTPPlaybackPlaylistHelper::HandleCollectionMessage( %d )", aErr );
    
    if (( KErrNone == aErr ) && aMsg )
        {
        TRAP( aErr, DoHandleCollectionMessageL( *aMsg ));
        }
    
    if ( KErrNone != aErr )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aErr );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_HANDLECOLLECTIONMESSAGE_EXIT );
    }
// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// ---------------------------------------------------------------------------  
//
void CMTPPlaybackPlaylistHelper::HandleOpenL( const CMPXMedia& aEntries, 
        TInt /*aIndex*/, TBool /*aComplete*/, TInt aError )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL_ENTRY );
    OstTrace1( TRACE_FLOW, CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL, "CMTPPlaybackPlaylistHelper::HandleOpenL( %d )", aError );
    
    if ( KErrNone == aError )
        {
        TRAP( aError, DoHandleOpenL( aEntries ));
        }
    
    if ( KErrNone != aError )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aError );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL_EXIT );
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionObserver
// ---------------------------------------------------------------------------  
//
void CMTPPlaybackPlaylistHelper::HandleOpenL( const CMPXCollectionPlaylist& aPlaylist,
        TInt aError )
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL_ENTRY );
    OstTrace1( TRACE_FLOW, DUP1_CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL, "CMTPPlaybackPlaylistHelper::HandleOpenL( aPlaylist, aError = %d )", aError );
    
    if ( KErrNone == aError )
        {
        TRAP( aError, MTPPlaybackControlImpl().GetPlaylistFromCollectionCompleteL( aPlaylist ));
        }
    
    if ( KErrNone != aError )
        {
        TInt error = MTPPlaybackControlImpl().MapError( aError );
        MTPPlaybackControlImpl().DoHandleError( error );
        }
    
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_HANDLEOPENL_EXIT );
    }

// ---------------------------------------------------------------------------
// From MMPXCollectionMediaObserver
// ---------------------------------------------------------------------------
void CMTPPlaybackPlaylistHelper::HandleCollectionMediaL( const CMPXMedia& /*aMedia*/, TInt /*aError*/ )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_HANDLECOLLECTIONMEDIAL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_HANDLECOLLECTIONMEDIAL_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_CMTPPLAYBACKPLAYLISTHELPER_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_CMTPPLAYBACKPLAYLISTHELPER_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::ConstructL
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_CONSTRUCTL_ENTRY );
    
    iCollectionUiHelper = CMPXCollectionHelperFactory::NewCollectionUiHelperL();
    iCollectionUtil = MMPXCollectionUtility::NewL( this, KMcModeDefault );
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_CONSTRUCTL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::DoHandleCollectionMessage
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::DoHandleCollectionMessageL( const CMPXMessage& aMsg )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_DOHANDLECOLLECTIONMESSAGEL_ENTRY );
    
    TMPXMessageId id( aMsg.ValueTObjectL<TMPXMessageId>( KMPXMessageGeneralId ) );

    if ( KMPXMessageGeneral == id )
        {
        TInt event( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralEvent ) );
        TInt type( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralType ) );
        TInt data( aMsg.ValueTObjectL<TInt>( KMPXMessageGeneralData ) );
        
        OstTraceExt2( TRACE_FLOW, CMTPPLAYBACKPLAYLISTHELPER_DOHANDLECOLLECTIONMESSAGEL, "Event code is 0x%X, type code is 0x%X", static_cast<TUint32>( event ), type );
        OstTrace1( TRACE_FLOW, DUP1_CMTPPLAYBACKPLAYLISTHELPER_DOHANDLECOLLECTIONMESSAGEL, "Data code is 0x%X", data );
        
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
            OstTrace0( TRACE_NORMAL, DUP2_CMTPPLAYBACKPLAYLISTHELPER_DOHANDLECOLLECTIONMESSAGEL, "Ignore this event" );
            }
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_DOHANDLECOLLECTIONMESSAGEL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::DoHandleOpenL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::DoHandleOpenL( const CMPXMedia& aEntries )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_DOHANDLEOPENL_ENTRY );
    
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_DOHANDLEOPENL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::OpenMusicPlayListPathL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_OPENMUSICPLAYLISTPATHL_ENTRY );
    
    CMPXCollectionPath* path = iCollectionUiHelper->MusicPlaylistPathL();
    CleanupStack::PushL( path );
    iCollectionUtil->Collection().OpenL( *path );
    CleanupStack::PopAndDestroy( path );
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_OPENMUSICPLAYLISTPATHL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::OpenMusicAblumPathL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_OPENMUSICABLUMPATHL_ENTRY );
    
    CMPXCollectionPath* path = iCollectionUiHelper->MusicMenuPathL();
    CleanupStack::PushL( path );
    path->AppendL(KMPXCollectionArtistAlbum);
    iCollectionUtil->Collection().OpenL( *path );
    CleanupStack::PopAndDestroy( path );
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_OPENMUSICABLUMPATHL_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::ResetPlaySource
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::ResetPlaySource()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_RESETPLAYSOURCE_ENTRY );
    
    iPathIndex = -1;
    iSongIndex = 0;
    delete iPlayObject;
    iPlayObject = NULL;
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_RESETPLAYSOURCE_EXIT );
    }

// ----------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL
// ----------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdatePlaylistPathIndexL( const CMPXMedia& aEntries )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEPLAYLISTPATHINDEXL_ENTRY );
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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEPLAYLISTPATHINDEXL_EXIT );
    }

// ---------------------------------------------------------------------------
// return instance of CollectionHelper.
// ---------------------------------------------------------------------------
//
MMPXCollectionHelper* CMTPPlaybackPlaylistHelper::CollectionHelperL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_COLLECTIONHELPERL_ENTRY );
    
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
        TInt ret = RFs::CharToDrive( driveChar, driveNumber );
        LEAVEIFERROR( ret, 
                OstTrace0( TRACE_ERROR, CMTPPLAYBACKPLAYLISTHELPER_COLLECTIONHELPERL, "Can't get driveNumber" ));
            
        // get root path
        TBuf<KStorageRootMaxLength> storeRoot;
        ret = PathInfo::GetRootPath( storeRoot, driveNumber );
        LEAVEIFERROR( ret, 
                        OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKPLAYLISTHELPER_COLLECTIONHELPERL, "Can't get storeRoot" ));
            
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
            LEAVEIFERROR( KErrGeneral, 
                    OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKPLAYLISTHELPER_COLLECTIONHELPERL, "MPX database error" ));
            }
        CleanupStack::PopAndDestroy( foundMedia ); // - foundMedia
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_COLLECTIONHELPERL_EXIT );
    return iCollectionHelper;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UriToItemIdL
// ---------------------------------------------------------------------------
//
const TMPXItemId CMTPPlaybackPlaylistHelper::UriToItemIdL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_URITOITEMIDL_ENTRY );

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
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_URITOITEMIDL_EXIT );
    return itemId;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::ItemIdToUriL.
// ---------------------------------------------------------------------------
//
const TFileName CMTPPlaybackPlaylistHelper::ItemIdToUriL( const TMPXItemId& aId )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_ITEMIDTOURIL_ENTRY );
    
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
        LEAVEIFERROR( KErrNotSupported, 
                OstTrace0( TRACE_ERROR, CMTPPLAYBACKPLAYLISTHELPER_ITEMIDTOURIL, "ItemId convert to Uri error" ));
        }
    else if ( *foundMedia->Value<TInt>( KMPXMediaArrayCount ) != 1 )
        {
        LEAVEIFERROR( KErrNotSupported, 
                OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKPLAYLISTHELPER_ITEMIDTOURIL, "ItemId convert to Uri error" ));
        }
    
    const CMPXMediaArray* tracksArray = foundMedia->Value<CMPXMediaArray> ( KMPXMediaArrayContents );
    CMPXMedia* item = tracksArray->AtL(0);
    
    if ( item->IsSupported( KMPXMediaGeneralUri ))
        {
        itemUri = item->ValueText(KMPXMediaGeneralUri);
        }
    
    CleanupStack::PopAndDestroy( foundMedia ); // - foundMedia
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_ITEMIDTOURIL_EXIT );
    return itemUri;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::FindAlbumSongsL
// ---------------------------------------------------------------------------
//
CMPXMedia* CMTPPlaybackPlaylistHelper::FindAlbumSongsL( const TMPXItemId& aAlbumId )
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_FINDALBUMSONGSL_ENTRY );
    
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
        LEAVEIFERROR( KErrNotSupported, 
                OstTrace0( TRACE_ERROR, CMTPPLAYBACKPLAYLISTHELPER_FINDALBUMSONGSL, "Find album song error" ));
        }
    TInt foundItemCount = *foundMedia->Value<TInt>( KMPXMediaArrayCount );
    if ( foundItemCount == 0 )
        {
        LEAVEIFERROR( KErrNotFound, 
                OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKPLAYLISTHELPER_FINDALBUMSONGSL, "Can't find album song" ));
        }
    if ( !foundMedia->IsSupported( KMPXMediaArrayContents ) )
        {
        LEAVEIFERROR( KErrNotSupported, 
                OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKPLAYLISTHELPER_FINDALBUMSONGSL, "Find album song error" ));
        }
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_FINDALBUMSONGSL_EXIT );
    return foundMedia;
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL.
// aParam: const CMPXMedia& aAlbums
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdateAlbumPathAndOpenL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEALBUMPATHANDOPENL_ENTRY );
    
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
        OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEALBUMPATHANDOPENL_EXIT );
        return;
        }
    
    ids.AppendL(id);
    cpath->AppendL( ids.Array() ); // top level items
    cpath->Set( 0 );
    ids.Reset();
    
    CMPXMedia* songs = FindAlbumSongsL( id );
    CleanupStack::PushL( songs );
    const CMPXMediaArray* tracksArray = songs->Value<CMPXMediaArray> ( KMPXMediaArrayContents );
    if ( const_cast<CMPXMediaArray*>(tracksArray) == NULL )
        {
        LEAVEIFERROR( KErrNoMemory, 
                            OstTrace0( TRACE_ERROR, CMTPPLAYBACKPLAYLISTHELPER_UPDATEALBUMPATHANDOPENL, "Find album song error" ));
        }
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
    
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPLAYLISTHELPER_UPDATEALBUMPATHANDOPENL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL.
// ---------------------------------------------------------------------------
//
void CMTPPlaybackPlaylistHelper::UpdatePathAndOpenL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEPATHANDOPENL_ENTRY );
    
    RArray<TMPXItemId> ids;
    CleanupClosePushL(ids);
    
    CMPXCollectionPath* cpath = iCollectionUtil->Collection().PathL();
    CleanupStack::PushL( cpath );
    
    cpath->Set( iSongIndex );
    
    iCollectionUtil->Collection().OpenL(*cpath);  
    CleanupStack::PopAndDestroy( cpath );
    CleanupStack::PopAndDestroy(&ids);
    
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_UPDATEPATHANDOPENL_EXIT );
    }

// ---------------------------------------------------------------------------
// CMTPPlaybackPlaylistHelper::MTPPlaybackControlImpl.
// ---------------------------------------------------------------------------
//
CMTPPlaybackControlImpl& CMTPPlaybackPlaylistHelper::MTPPlaybackControlImpl()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPLAYLISTHELPER_MTPPLAYBACKCONTROLIMPL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKPLAYLISTHELPER_MTPPLAYBACKCONTROLIMPL_EXIT );
    return iMTPPlaybackControl;
    }


