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
* Description:
*
*/

// This file defines the API for dunutilstest.dll

#ifndef DUNUTILSTEST_H
#define DUNUTILSTEST_H

//  Include Files

#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>

#include "DunNoteHandler.h"

//  Constants

//  Class Definitions


NONSHARABLE_CLASS(Cdunutilstest) : public CScriptBase
    {
public:
    // new functions
    
    static Cdunutilstest* NewL(CTestModuleIf& aTestModuleIf);

    virtual ~Cdunutilstest();

public: // Functions from base classes

    /**
    * From CScriptBase Runs a script line.
    * @since ?Series60_version
    * @param aItem Script line containing method name and parameters
    * @return Symbian OS error code
    */
    virtual TInt RunMethodL( CStifItemParser& aItem );

private:
    // new functions
    Cdunutilstest(CTestModuleIf& aTestModuleIf);
    void ConstructL();
    
    virtual TInt LaunchDialogL( CStifItemParser& aItem );

private:
    // data
    CDunNoteHandler* mDunandler;
    };

#endif  // DUNUTILSTEST_H

