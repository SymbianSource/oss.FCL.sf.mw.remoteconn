/*
* Copyright (c) 2002 - 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class which is resposible for handling communication with
*                Bt settings API
*
*/


#include "BtSettings.h"

CBtSettings::CBtSettings()
	{
	// No implementation required
	}

CBtSettings::~CBtSettings()
	{
	delete iSettings;
	if( iWaiter->IsStarted() )
		{
		iWaiter->AsyncStop();
		}
	delete iWaiter;
	}

CBtSettings* CBtSettings::NewLC()
	{
	CBtSettings* self = new ( ELeave ) CBtSettings();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CBtSettings* CBtSettings::NewL()
	{
	CBtSettings* self = CBtSettings::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CBtSettings::ConstructL()
	{
	iWaiter = new ( ELeave ) CActiveSchedulerWait();
	iSettings = CBTEngSettings::NewL( this );
	iError = KErrNone;
	}

void CBtSettings::PowerStateChanged( TBTPowerStateValue /*aState*/ )
	{
	if( iWaiter->IsStarted() )
		{
		iWaiter->AsyncStop();
		}
	}

void CBtSettings::VisibilityModeChanged( TBTVisibilityMode /*aState*/ )
	{
	if( iWaiter->IsStarted() )
		{
		iWaiter->AsyncStop();
		}
	}

TInt CBtSettings::GetPowerState( TBTPowerStateValue& aState )
	{
	iError = iSettings->GetPowerState( aState );
	return iError;
	}

TInt CBtSettings::SetPowerState( TBTPowerStateValue aState )
	{
    TBTPowerStateValue state;
    
    iError = iSettings->GetPowerState( state );
	if( iError )
		{
		return iError;
		}
	else if( state != aState )
			{
			if( state == EBTPowerOff )
				{
				iError = iSettings->SetPowerState( EBTPowerOn );
				}
			else
				{
				iError = iSettings->SetPowerState( EBTPowerOff );
			    }
			
			if( iError )
				{
				return iError;
				}
			else
				{
		        if ( !iWaiter->IsStarted() )
		            {
		            iWaiter->Start();
		            }
				}
			}
	return iError;
	}

TInt CBtSettings::GetVisibilityMode( TBTVisibilityMode& aMode )
	{
	return KErrNone;
	}

TInt SetVisibilityMode( TBTVisibilityMode aMode, TInt aTime )
	{
	return KErrNone;
	}

TInt GetLocalName( TDes& aName )
	{
	return KErrNone;
	}

TInt SetLocalName( const TDes& aName )
	{
	return KErrNone;
	}

// End of file
