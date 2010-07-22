/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Manages note showing in UI
*
*/


#ifndef C_CDUNNOTEHANDLER_H
#define C_CDUNNOTEHANDLER_H

#include <e32base.h>
#include <hbdevicemessageboxsymbian.h>
#include "DunTransporter.h"

/**
 *  Class for managing note showing in UI
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunNoteHandler ) : public CBase,
                                       public MHbDeviceMessageBoxObserver
    {

public:

    /**
     * Two-phased constructor.
     * @return Instance of self
     */
	static CDunNoteHandler* NewL();

    /**
    * Destructor.
    */
    virtual ~CDunNoteHandler();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Issues request to start showing UI note
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops showing UI note
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunNoteHandler();

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Issues request to start showing UI note
     *
     * @since S60 3.2
     * @return None
     */
    void DoIssueRequestL();

// from base class MHbDeviceMessageBoxObserver

	// TODO: ADD DESCRIPTION HERE!
    void MessageBoxClosed( const CHbDeviceMessageBoxSymbian* aMessageBox,
                           CHbDeviceMessageBoxSymbian::TButtonId aButton );

private:  // data

    /**
     * Note to show
     */
    CHbDeviceMessageBoxSymbian* iNote;

    /**
     * Current state of note showing: active or inactive
     */
    TDunState iNoteState;

    };

#endif  // C_CDUNNOTEHANDLER_H
