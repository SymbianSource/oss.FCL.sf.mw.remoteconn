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


#ifndef BTSETTINGS_H
#define BTSETTINGS_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <btengsettings.h>
// CLASS DECLARATION

/**
 *  CBtSettings
 */
NONSHARABLE_CLASS( CBtSettings ) : public CBase,
                                   public MBTEngSettingsObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CBtSettings();

	/**
	 * Two-phased constructor.
	 */
	static CBtSettings* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CBtSettings* NewLC();
	
    TInt GetPowerState( TBTPowerStateValue& aState );
    TInt SetPowerState( TBTPowerStateValue aState );
    TInt GetVisibilityMode( TBTVisibilityMode& aMode );
    TInt SetVisibilityMode( TBTVisibilityMode aMode, TInt aTime = 0 );
    TInt GetLocalName( TDes& aName );
    TInt SetLocalName( const TDes& aName );


private:

	// From MBTEngSettingsObserver
    void PowerStateChanged( TBTPowerStateValue aState );
    void VisibilityModeChanged( TBTVisibilityMode aState );
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CBtSettings();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

	
private: //Data
	
	CBTEngSettings* iSettings; // Own.
	CActiveSchedulerWait* iWaiter; //Own.
	TInt iError;
	
	};

#endif // BTSETTINGS_H
