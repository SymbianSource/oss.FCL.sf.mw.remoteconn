/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Special AT command handler
*
*/

#ifndef C_CDUNATSPECIALCMDHANDLER_H
#define C_CDUNATSPECIALCMDHANDLER_H

#include <e32base.h>
#include <badesca.h>

const TInt KInputBufLength = (512 + 1);  // Set this the same as in KDunInputBufLength

/**
 *  Class for special AT command handler
 *
 *  @lib dunatext.lib
 *  @since TB9.2
 */
NONSHARABLE_CLASS( CDunAtSpecialCmdHandler ) : public CBase
    {

public:

    /**
     * Two-phased constructor.
     * @param None
     * @return Instance of self
     */
	static CDunAtSpecialCmdHandler* NewL();

    /**
    * Destructor.
    */
    ~CDunAtSpecialCmdHandler();

public:

    /**
     * Checks if the command has to be treated special way.
     * For example in case of MAC, it sends command AT&FE0Q0V1&C1&D2+IFC=3,1.
     * meaning there is no delimiters in the command.
     * In case of MAC we try to search AT&F (sub command) string from the
     * beginning of the command.
     * Search is done string basis.
     *
     * @since TB9.2
     * @param aCharacter Character to add
     * @return ETrue if data is ready for comparison, EFalse otherwise
     */
    TBool IsCompleteSubCommand( TChar aCharacter );

    /**
     * Resets the buffer used for comparisons
     *
     * @since TB9.2
     * @return None
     */
    void ResetComparisonBuffer();

private:

    CDunAtSpecialCmdHandler();

    void ConstructL();

    /**
     * Defines when comparison is excecuted, checks if the data lengths are
     * equal.
     *
     * @since TB9.2
     * @return ETrue if data is ready for comparison, EFalse otherwise
     */
    TBool IsDataReadyForComparison( TInt aLength );

    /**
     * Defines minimum length of the special commands.
     *
     * @since TB9.2
     * @return Minimum length of the special commands
     */
    TInt MinimumLength();

private:  // data

    /**
     * Buffer for temporary AT command input
     */
    TBuf8<KInputBufLength> iBuffer;

    /**
     * Special commands for parsing
     */
    CDesC8Array *iSpecialCmds;

    };

#endif  // C_CDUNATSPECIALCMDHANDLER_H
