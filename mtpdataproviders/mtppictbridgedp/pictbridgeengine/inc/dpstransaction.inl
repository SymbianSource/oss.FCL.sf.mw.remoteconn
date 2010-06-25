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
* Description:  This class creates and parses dps operations. 
*
*/

#include "dpsstatemachine.h"
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
TBool CDpsTransaction::IsReply()
	{
	return iReply;
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
CDpsXmlParser* CDpsTransaction::Parser()
    {
    return iXmlPar;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//        
CDpsEngine* CDpsTransaction::Engine()
    {
    return iOperator->DpsEngine();
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//        
CDpsXmlGenerator* CDpsTransaction::Generator()
    {
    return iXmlGen;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//        
CDpsStateMachine* CDpsTransaction::Operator()
    {
    return iOperator;
    }
          
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//            
CDpsFile* CDpsTransaction::FileHandle()
    {
    return iFile;
    }