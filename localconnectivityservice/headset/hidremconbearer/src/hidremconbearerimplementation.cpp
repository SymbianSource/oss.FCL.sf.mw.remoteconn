/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bearer plugin API Implementation file
 *
*/

#include <e32def.h>
#include <remcon/remconbearerobserver.h>
#include <remcon/remconconverterplugin.h>
#include <remconaddress.h>
#include <remconcoreapi.h>
#include <RemConExtensionApi.h>
#include <PSVariables.h>   // Property values
#include <coreapplicationuisdomainpskeys.h>
#include "hidremconbearerinternalpskeys.h"
#include "hidremconbearerscancodes.h"
#include "hidremconbearerimplementation.h"
#include "hidremconbearerobserver.h"
#include "debug.h"
                                  
_LIT8(KFormatString,"%c");
_LIT8(KVoiceFormatString,"%d");
_LIT_SECURITY_POLICY_C1(KHidRemconBearerReadPolicy, ECapability_None);
_LIT_SECURITY_POLICY_C1(KHidRemconBearerWritePolicy, ECapability_None);

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CHidRemconBearer::NewL()
// Description: Factory function.
// return: Ownership of a new CHidRemconBearer.
// ---------------------------------------------------------------------------
//
CHidRemconBearer* CHidRemconBearer::NewL( TBearerParams& aParams )
    {
    CHidRemconBearer* self = new ( ELeave ) CHidRemconBearer( aParams );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::~CHidRemconBearer()
// Destructor.
// ---------------------------------------------------------------------------
//
CHidRemconBearer::~CHidRemconBearer()
    {
    delete iMediaKeyObserver;
    delete iAccessoryVolKeyObserver;
    delete iMuteKeyObserver;
    delete iHookKeyObserver;
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::CHidRemconBearer()
// construction.
// ---------------------------------------------------------------------------
//
CHidRemconBearer::CHidRemconBearer( TBearerParams& aParams ) :
    CRemConBearerPlugin( aParams )
    {
    //Pass
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::ConstructL()
// 2nd-phase construction.
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::ConstructL()
    {
        TRACE_FUNC_ENTRY
    // throw an indication up to RemCon.
    TRemConAddress addr;
    addr.BearerUid() = Uid();
    TInt err = Observer().ConnectIndicate( addr );

    //Define the P&S for Volume keys and Media Keys    
    RProperty::Define( KPSUidHidEventNotifier, KHidControlKeyEvent,
            RProperty::EInt, KHidRemconBearerReadPolicy,
            KHidRemconBearerWritePolicy );
    // Define P&S Key for Volume keys from Accessory device
    RProperty::Define( KPSUidHidEventNotifier, KHidAccessoryVolumeEvent,
            RProperty::EInt, KHidRemconBearerReadPolicy,
            KHidRemconBearerWritePolicy );
    // Define P&S Key for Mute key from Accessory device
    RProperty::Define( KPSUidHidEventNotifier, KHidMuteKeyEvent,
            RProperty::EInt, KHidRemconBearerReadPolicy,
            KHidRemconBearerWritePolicy );

    // Define P&S Key for phone keys from Accessory device
    RProperty::Define( KPSUidHidEventNotifier, KHidHookKeyEvent,
            RProperty::EInt, KHidRemconBearerReadPolicy,
            KHidRemconBearerWritePolicy );

    // Start Active object for listening key events from P&S

    iMediaKeyObserver = CHidRemconBearerObserver::NewL( *this, EMediaKeys );
    iAccessoryVolKeyObserver = CHidRemconBearerObserver::NewL( *this,
            EAccessoryVolumeKeys );

    iMuteKeyObserver = CHidRemconBearerObserver::NewL( *this, EMuteKey );

    iHookKeyObserver = CHidRemconBearerObserver::NewL( *this, EHookKeys );
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::ConnectRequest()
// Rem Con server send connect request to Bearer Plugin
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::ConnectRequest( const TRemConAddress& /*aAddr*/)
    {
    TRACE_FUNC
    // Pass
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::DisconnectRequest()
// Rem Con server send disconnect request to Bearer Plugin
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::DisconnectRequest( const TRemConAddress& /*aAddr*/)
    {
    TRACE_FUNC
    // Pass
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::SendResponse()
// 
// ---------------------------------------------------------------------------
//
TInt CHidRemconBearer::SendResponse( TUid /* aInterfaceUid */,
        TUint /*aOperationId*/, 
        TUint /*aTransactionId*/, // we don't care about this transaction ID
        RBuf8& aData, 
        const TRemConAddress& /*aAddr*/)
    {
    TRACE_FUNC
    aData.Close();
    return KErrNone;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::SendCommand()
// 
// ---------------------------------------------------------------------------
//
TInt CHidRemconBearer::SendCommand( TUid /* aInterfaceUid */,
        TUint /* aOperationId */, TUint /* aTransactionId */,
        RBuf8& /* aData */, const TRemConAddress& /*aAddr*/)
    {
    TRACE_FUNC
    return KErrNone;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::GetResponse()
// 
// ---------------------------------------------------------------------------
//
TInt CHidRemconBearer::GetResponse( TUid& /* aInterfaceUid */,
        TUint& /* aTransactionId*/, TUint& /* aOperationId */,
        RBuf8& /* aData */, TRemConAddress& /*aAddr */)
    {
    TRACE_FUNC
    return KErrNone;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::GetCommand()
// 
// ---------------------------------------------------------------------------
//
TInt CHidRemconBearer::GetCommand( TUid& aInterfaceUid,
        TUint& aTransactionId, TUint& aOperationId, RBuf8& aData,
        TRemConAddress& aAddr )
    {
        TRACE_FUNC
    aInterfaceUid = iInterfaceUid;
    aOperationId = iOperationId;
    // Pass ownership of this to RemCon.
    TRAPD(retTrap, aData.CreateL(iData));
    if ( retTrap != KErrNone )
        {
        return retTrap;
        }
    aAddr.BearerUid() = Uid();
    aAddr.Addr() = KNullDesC8();
    aTransactionId = iTransactionId;
    return KErrNone;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::SetSVKOperationIdAndData()
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::SetSVKOperationIdAndData( TInt aEnumValue )
    {
        TRACE_FUNC
    // 1. Interface ID is required when Getcommand is called
    iInterfaceUid = TUid::Uid( KRemConCoreApiUid ); // from Remconcoreapi.h
    switch ( aEnumValue )
        {
        case KPSVolumeUpPressed:
            iOperationId = ERemConCoreApiVolumeUp;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case KPSVolumeUpReleased:
            iOperationId = ERemConCoreApiVolumeUp;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case KPSVolumeUpClicked:
            iOperationId = ERemConCoreApiVolumeUp;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case KPSVolumeDownPressed:
            iOperationId = ERemConCoreApiVolumeDown;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case KPSVolumeDownReleased:
            iOperationId = ERemConCoreApiVolumeDown;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case KPSVolumeDownClicked:
            iOperationId = ERemConCoreApiVolumeDown;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case KPSVolumeNoKey: // This should never execute
            break;
        default:
            iOperationId = aEnumValue;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        }
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::SetMuteKeyOperationIdAndData
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::SetMuteKeyOperationIdAndData( TInt aEnumValue )
    {
        TRACE_INFO((_L("[HID]\tCHidRemconBearer::SetMuteKeyOperation \
                IdAndData( %d )"), aEnumValue));
    // 1. Interface ID is required when Getcommand is called
    iInterfaceUid = TUid::Uid( KRemConCoreApiUid ); // from Remconcoreapi.h
    switch ( aEnumValue )
        {
        case KPSMutePressed:
            iOperationId = ERemConCoreApiMute;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case KPSMuteReleased:
            iOperationId = ERemConCoreApiMute;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case KPSMuteClicked:
            iOperationId = ERemConCoreApiMute;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        default:
            TRACE_INFO(_L("CHidRemconBearer::SetMuteKeyOperationIdAndData : \
                    Not supported"))
            ;
        }
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::SetMediaKeyOperationIdAndData()
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::SetMediaKeyOperationIdAndData( TInt aEnumValue )
    {
        TRACE_FUNC
    // 1. Interface ID is required when Getcommand is called
    iInterfaceUid = TUid::Uid( KRemConCoreApiUid ); // from Remconcoreapi.h
    switch ( aEnumValue )
        {
        case EPlayReleased:
            iOperationId = ERemConCoreApiPausePlayFunction;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case EPlayPressed:
            iOperationId = ERemConCoreApiPausePlayFunction;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case EPlayClicked:
            iOperationId = ERemConCoreApiPausePlayFunction;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case EStopReleased:
            iOperationId = ERemConCoreApiStop;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case EStopPressed:
            iOperationId = ERemConCoreApiStop;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case EStopClicked:
            iOperationId = ERemConCoreApiStop;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case EForwardReleased:
            iOperationId = ERemConCoreApiForward;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case EForwardPressed:
            iOperationId = ERemConCoreApiForward;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case EForwardClicked:
            iOperationId = ERemConCoreApiForward;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case ERewindReleased:
            iOperationId = ERemConCoreApiRewind;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case ERewindPressed:
            iOperationId = ERemConCoreApiRewind;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case ERewindClicked:
            iOperationId = ERemConCoreApiRewind;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case EFastForwardReleased:
            iOperationId = ERemConCoreApiFastForward;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case EFastForwardPressed:
            iOperationId = ERemConCoreApiFastForward;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case EFastForwardClicked:
            iOperationId = ERemConCoreApiFastForward;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case EBackwardReleased:
            iOperationId = ERemConCoreApiBackward;
            iData.Format( KFormatString, ERemConCoreApiButtonRelease );
            break;
        case EBackwardPressed:
            iOperationId = ERemConCoreApiBackward;
            iData.Format( KFormatString, ERemConCoreApiButtonPress );
            break;
        case EBackwardClicked:
            iOperationId = ERemConCoreApiBackward;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
            // This should never execute
        default:
            iOperationId = aEnumValue;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;

        }
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::ReceivedKeyEvent()
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::SetPhoneKeyOperationIdAndData( TInt aEnumValue )
    {
    TRACE_FUNC_ENTRY
    iInterfaceUid = TUid::Uid( KRemConExtCallHandlingApiUid );
    switch ( aEnumValue )
        {
        case KPSAnsweClicked:
            TRACE_INFO((_L("[HID]\tCHidRemconBearer::SetPhoneKey \
                    OperationIdAndData Answer")));
            iOperationId = ERemConExtAnswerCall;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case KPSHangUpClicked:
            iOperationId = ERemConExtEndCall;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case KPSRedial:
            iOperationId = ERemConExtLastNumberRedial;
            iData.Format( KFormatString, ERemConCoreApiButtonClick );
            break;
        case KPSVoiceDial:
            iOperationId = ERemConExtVoiceDial;
            iData.Format( KVoiceFormatString, ERemConCoreApiButtonRelease );
            break;
        default:
            TRACE_INFO(_L("CHidRemconBearer::SetPhoneKeyOperationIdAndData \
                    : Not supported"));        }
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::ReceivedKeyEvent()
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::ReceivedKeyEvent( TInt aEnumValue, TInt aKeyType )
    {
    TRACE_INFO((_L("[HID]\tCHidRemconBearer::ReceivedKeyEvent: value %d, \
            type %d )"), aEnumValue, aKeyType));

    /* To keep the background light on */
    User::ResetInactivityTime();

    // 2. Transaction ID
    iTransactionId = Observer().NewTransactionId();

    // 3. Addr 
    TRemConAddress addr;
    addr.BearerUid() = Uid();
    addr.Addr() = KNullDesC8();

    // 4 & 5. Operation ID and Data
    HandleKeyOperation( aEnumValue, aKeyType );

    // Msgtype  is ERemConCommand
    TInt error = Observer().NewCommand( addr );
    TRACE_INFO((_L("[HID]\tCHidRemconBearer::ReceivedKeyEvent: error %d )"), 
            error));
    //Start the listener once again     
    RestartKeyObeserver( aKeyType );

    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::RestartKeyObeserver
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::RestartKeyObeserver( TInt aKeyType )
    {
    TRACE_FUNC_ENTRY
    switch ( aKeyType )
        {
        case EMediaKeys:
            iMediaKeyObserver->Start();
            break;
        case EAccessoryVolumeKeys:
            iAccessoryVolKeyObserver->Start();
            break;
        case EMuteKey:
            iMuteKeyObserver->Start();
            break;
        case EHookKeys:
            iHookKeyObserver->Start();
            break;
        default:
            TRACE_INFO(_L("CHidRemconBearer::RestartKeyObeserver : Not supported"))
            ;
        }
        TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::RestartObexserver
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::HandleKeyOperation( TInt aEnumValue, TInt aKeyType )
    {
    TRACE_FUNC_ENTRY
    switch ( aKeyType )
        {
        case EAccessoryVolumeKeys:
            SetSVKOperationIdAndData( aEnumValue );
            break;
        case EMuteKey:
            SetMuteKeyOperationIdAndData( aEnumValue );
            break;
        case EMediaKeys:
            SetMediaKeyOperationIdAndData( aEnumValue );
            break;
        case EHookKeys:
            SetPhoneKeyOperationIdAndData( aEnumValue );
            break;
        default:
            TRACE_INFO(_L("CHidRemconBearer::HandleKeyOperation : Not \
                    supported"))
            ;
        }
    TRACE_FUNC_EXIT
    }
// ---------------------------------------------------------------------------
// CHidRemconBearer::SecurityPolicy()
// ---------------------------------------------------------------------------
//
TSecurityPolicy CHidRemconBearer::SecurityPolicy() const
    {
    return TSecurityPolicy( ECapabilityLocalServices );
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::ClientStatus()
// ---------------------------------------------------------------------------
//
void CHidRemconBearer::ClientStatus( TBool /*aControllerPresent*/, TBool /*aTargetPresent*/)
    {
        //Pass
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// CHidRemconBearer::GetInterface()
// ---------------------------------------------------------------------------
//
TAny* CHidRemconBearer::GetInterface( TUid aUid )
    {
    TRACE_FUNC
    TAny* ret = NULL;
    if ( aUid == TUid::Uid( KRemConBearerInterface1 ) )
        {
        ret = reinterpret_cast<TAny*> ( 
                        static_cast<MRemConBearerInterface*> ( this ) );
        }
    return ret;
    }
