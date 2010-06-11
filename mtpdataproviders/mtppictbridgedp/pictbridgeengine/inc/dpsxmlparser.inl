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
* Description:  This class parses the dps xml script. 
*
*/


#ifdef _DEBUG
#	define __IF_DEBUG(t) {RDebug::t;}
#else
#	define __IF_DEBUG(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::GetAttribute(TDpsAttribute& aAttrib) const
    {
    aAttrib = iAttrib;
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
TBool CDpsXmlParser::HasAttribute() const
    {
    return iAttrib != 0;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
TDpsOperation CDpsXmlParser::Operation() const
    {
    return iDpsOperation;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsEvent CDpsXmlParser::Event() const
    {
    return iDpsEvent;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::GetParameters(TDpsArgArray& aParams) const
    {
    aParams = iDpsArgs;
    }
 
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TBool CDpsXmlParser::IsEvent() const
	{
	return iDpsEvent != EDpsEvtEmpty;
	}


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::SetParameters(TDpsArgArray& aParams)
    {
    iDpsArgs = aParams;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::SetOperationResult(TDpsResult& aResult) const
    {
    aResult.iMajorCode = iDpsResult.iMajorCode;
    aResult.iMinorCode = iDpsResult.iMinorCode;
    }
