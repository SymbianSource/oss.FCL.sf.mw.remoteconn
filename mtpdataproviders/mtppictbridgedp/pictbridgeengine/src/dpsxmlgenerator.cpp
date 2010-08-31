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
* Description:  This class creates the dps xml script. 
*
*/


#include <e32debug.h>
#include "dpsxmlgenerator.h"
#include "pictbridge.h"
#include "dpsconst.h"
#include "dpsxmlstring.h"
#include <e32debug.h>

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

const TInt KCodeSize = 4;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator* CDpsXmlGenerator::NewL(CDpsEngine* aEngine)
    {
    IF_DEBUG(Print(_L("CDpsXmlGenerator::NewL")));
    CDpsXmlGenerator* self= new (ELeave) CDpsXmlGenerator(aEngine);
    return self;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator::~CDpsXmlGenerator()
    {
    IF_DEBUG(Print(_L("~CDpsXmlGenerator")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator::CDpsXmlGenerator(CDpsEngine* aEngine) : 
    iEngine(aEngine)
    {        
    IF_DEBUG(Print(_L("CDpsXmlGenerator::Ctor")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::CreateResultScriptL(TDpsEvent aEvent,
		                     			  RWriteStream& aScript,
	           		         			  const TDpsResult& aResult) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::CreateResultScript")));    
    StartDocumentL(aScript);       
    StartResultL(aScript, aResult);       
    if (aEvent != EDpsEvtEmpty)
        {
        CreateEventL(aScript, aEvent);          
        }    
    EndResultL(aScript);    
    EndDocumentL(aScript);    
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::CreateResultScript")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::CreateReplyScriptL(TDpsOperation aOperation, 
                                         RWriteStream& aScript,
                                         const TDpsResult& aResult,
                                         const TDpsArg& aArg) const
    {
    StartDocumentL(aScript);
    StartResultL(aScript, aResult);
    StartOperationL(aOperation, aScript);
    CreateArgL(aScript, aArg);
    EndOperationL(aOperation, aScript);
    EndResultL(aScript);
    EndDocumentL(aScript);
    }                                            
                                            
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//		           		         		
void CDpsXmlGenerator::StartDocumentL(RWriteStream& aScript) const
	{
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::StatrDocument")));
    aScript.WriteL(KDpsXmlHeader);
    aScript.WriteL(KDpsXmlNS);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::StartDocument")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::EndDocumentL(RWriteStream& aScript) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::EndDocument")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXml);
    aScript.WriteL(KDpsXmlBraceClose);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::EndDocument")));		
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::StartInputL(RWriteStream& aScript) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::StartInput")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlInput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::StartInput")));	
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsXmlGenerator::EndInputL(RWriteStream& aScript) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::EndInput")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlInput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::EndInput")));		
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsXmlGenerator::StartResultL(RWriteStream& aScript, 
								   const TDpsResult& aResult) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::StartResult")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlOutput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlResult);
    aScript.WriteL(KDpsXmlBraceClose);
    TBuf8<KCodeSize> major;
    major.NumUC(aResult.iMajorCode, EHex);
    aScript.WriteL(major);
    major.Zero();
    major.NumFixedWidthUC(aResult.iMinorCode, EHex, KCodeSize);
    aScript.WriteL(major);    
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlResult);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::StartResult")));		
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::EndResultL(RWriteStream& aScript) const
	{
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::EndResult")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlOutput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::EndResult")));		
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::StartOperationL(TDpsOperation aOperation, 
                                       RWriteStream& aScript, 
									   TBool aEnd) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::StartOperation %d"), aOperation));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsOperationStrings[aOperation - 1]);
    if (aEnd)
        {
        aScript.WriteL(KDpsXmlSlash);
        }
    aScript.WriteL(KDpsXmlBraceClose);	
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::StartOperation")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//			
void CDpsXmlGenerator::EndOperationL(TDpsOperation aOperation, 
                                     RWriteStream& aScript) const 
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::EndOperation")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    // there is an empty op in the op enum so we must reduce one
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsOperationStrings[aOperation - 1]);
    aScript.WriteL(KDpsXmlBraceClose);	
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::EndOperation")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::CreateEventL(RWriteStream& aScript, TDpsEvent aEvent) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::CreatEvent")));
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsEventStrings[aEvent - 1]);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::CreatEvent")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::CreateArgL(RWriteStream& aScript, 
								  const TDpsArg& aArgument, 
					   			  TDpsAttribute aAttribute) const           
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::CreatArg")));
    aScript.WriteL(KDpsXmlBraceOpen); //<
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsArgStrings[aArgument.iElement]);	
    if (aAttribute != 0)
        {
        aScript.WriteL(KDpsXmlSpace); // space
        aScript.WriteL(KDpsXmlPaperSize); // paperSize
        aScript.WriteL(KDpsXmlEqual); // =
        aScript.WriteL(KDpsXmlQuote); // "
        // patch the lower four bytes zero
        aAttribute = aAttribute << KShiftLength;
        TBuf8<KFullWordWidth> string;
        string.AppendNumUC(aAttribute, EHex);
        aScript.WriteL(string); // 12345678
        aScript.WriteL(KDpsXmlQuote); // "
        }
    if (!aArgument.iContent.Compare(KNullDesC8))
        {
        aScript.WriteL(KDpsXmlSlash); // /
        aScript.WriteL(KDpsXmlBraceClose); // >
        }
    else
        {
        aScript.WriteL(KDpsXmlBraceClose); // >
        aScript.WriteL(aArgument.iContent); // 123 345 567
        aScript.WriteL(KDpsXmlBraceOpen); // <
        aScript.WriteL(KDpsXmlSlash); // /
        aScript.WriteL(iEngine->DpsParameters()->
            iDpsArgStrings[aArgument.iElement]);
        aScript.WriteL(KDpsXmlBraceClose); // >
        }
    aScript.WriteL(KDpsXmlSpace);	
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::CreatArg")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::StartElementL(TDpsElement aElement, 
                                     RWriteStream& aScript) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::StartElement")));
    aScript.WriteL(KDpsXmlBraceOpen); //<
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsElementStrings[aElement - 1]);	
    aScript.WriteL(KDpsXmlBraceClose); // >
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::StartElement")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::EndElementL(TDpsElement aElement, 
                                   RWriteStream& aScript) const
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlGenerator::EndElement")));
    aScript.WriteL(KDpsXmlBraceOpen); //<
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsElementStrings[aElement - 1]);	
    aScript.WriteL(KDpsXmlBraceClose); // >
    aScript.WriteL(KDpsXmlSpace);
    IF_DEBUG(Print(_L("<<<CDpsXmlGenerator::EndElement")));
    }
