/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class defines the dps constant strings. 
*
*/


#ifndef DPSXMLSTRING_H
#define DPSXMLSTRING_H

#include <e32base.h>

typedef RArray<TPtrC8> TDpsStrings;

/**
* The class for the dps string
*/
class TDpsXmlString
    {
    public:
        /**
        *
        */
        static TDpsXmlString* NewL();
        
        /**
        *
        */
        ~TDpsXmlString();
        
    private:
        /**
        *
        */
        void ConstructL();
        
        /**
        *
        */
        TDpsXmlString();    
        
    public:    
        TDpsStrings iDpsOperationStrings;
        TDpsStrings iDpsElementStrings;
        TDpsStrings iDpsEventStrings;
        TDpsStrings iDpsArgStrings;
    };

#endif