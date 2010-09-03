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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsxmlgeneratorTraces.h"
#endif


const TInt KCodeSize = 4;

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator* CDpsXmlGenerator::NewL(CDpsEngine* aEngine)
    {
    CDpsXmlGenerator* self= new (ELeave) CDpsXmlGenerator(aEngine);
    return self;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator::~CDpsXmlGenerator()
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_CDPSXMLGENERATOR_DES_ENTRY );
    OstTraceFunctionExit0( CDPSXMLGENERATOR_CDPSXMLGENERATOR_DES_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlGenerator::CDpsXmlGenerator(CDpsEngine* aEngine) : 
    iEngine(aEngine)
    {        
    OstTraceFunctionEntry0( DUP1_CDPSXMLGENERATOR_CDPSXMLGENERATOR_CONS_ENTRY );
    OstTraceFunctionExit0( DUP1_CDPSXMLGENERATOR_CDPSXMLGENERATOR_CONS_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::CreateResultScriptL(TDpsEvent aEvent,
		                     			  RWriteStream& aScript,
	           		         			  const TDpsResult& aResult) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_CREATERESULTSCRIPTL_ENTRY ); 
    StartDocumentL(aScript);       
    StartResultL(aScript, aResult);       
    if (aEvent != EDpsEvtEmpty)
        {
        CreateEventL(aScript, aEvent);          
        }    
    EndResultL(aScript);    
    EndDocumentL(aScript);    
    OstTraceFunctionExit0( CDPSXMLGENERATOR_CREATERESULTSCRIPTL_EXIT );
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
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_STARTDOCUMENTL_ENTRY );
    aScript.WriteL(KDpsXmlHeader);
    aScript.WriteL(KDpsXmlNS);
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_STARTDOCUMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::EndDocumentL(RWriteStream& aScript) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_ENDDOCUMENTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXml);
    aScript.WriteL(KDpsXmlBraceClose);	
    OstTraceFunctionExit0( CDPSXMLGENERATOR_ENDDOCUMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::StartInputL(RWriteStream& aScript) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_STARTINPUTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlInput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_STARTINPUTL_EXIT );
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsXmlGenerator::EndInputL(RWriteStream& aScript) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_ENDINPUTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlInput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);	
    OstTraceFunctionExit0( CDPSXMLGENERATOR_ENDINPUTL_EXIT );
    }
  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsXmlGenerator::StartResultL(RWriteStream& aScript, 
								   const TDpsResult& aResult) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_STARTRESULTL_ENTRY );
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
    OstTraceFunctionExit0( CDPSXMLGENERATOR_STARTRESULTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::EndResultL(RWriteStream& aScript) const
	{
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_ENDRESULTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlOutput);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);	
    OstTraceFunctionExit0( CDPSXMLGENERATOR_ENDRESULTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::StartOperationL(TDpsOperation aOperation, 
                                       RWriteStream& aScript, 
									   TBool aEnd) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_STARTOPERATIONL_ENTRY );
    OstTrace1( TRACE_NORMAL, CDPSXMLGENERATOR_STARTOPERATIONL, "Operation %d", aOperation );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsOperationStrings[aOperation - 1]);
    if (aEnd)
        {
        aScript.WriteL(KDpsXmlSlash);
        }
    aScript.WriteL(KDpsXmlBraceClose);	
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_STARTOPERATIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//			
void CDpsXmlGenerator::EndOperationL(TDpsOperation aOperation, 
                                     RWriteStream& aScript) const 
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_ENDOPERATIONL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(KDpsXmlSlash);
    // there is an empty op in the op enum so we must reduce one
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsOperationStrings[aOperation - 1]);
    aScript.WriteL(KDpsXmlBraceClose);	
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_ENDOPERATIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::CreateEventL(RWriteStream& aScript, TDpsEvent aEvent) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_CREATEEVENTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsEventStrings[aEvent - 1]);
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(KDpsXmlBraceClose);
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_CREATEEVENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlGenerator::CreateArgL(RWriteStream& aScript, 
								  const TDpsArg& aArgument, 
					   			  TDpsAttribute aAttribute) const           
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_CREATEARGL_ENTRY );
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
    OstTraceFunctionExit0( CDPSXMLGENERATOR_CREATEARGL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::StartElementL(TDpsElement aElement, 
                                     RWriteStream& aScript) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_STARTELEMENTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen); //<
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsElementStrings[aElement - 1]);	
    aScript.WriteL(KDpsXmlBraceClose); // >
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_STARTELEMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlGenerator::EndElementL(TDpsElement aElement, 
                                   RWriteStream& aScript) const
    {
    OstTraceFunctionEntry0( CDPSXMLGENERATOR_ENDELEMENTL_ENTRY );
    aScript.WriteL(KDpsXmlBraceOpen); //<
    aScript.WriteL(KDpsXmlSlash);
    aScript.WriteL(iEngine->DpsParameters()->
        iDpsElementStrings[aElement - 1]);	
    aScript.WriteL(KDpsXmlBraceClose); // >
    aScript.WriteL(KDpsXmlSpace);
    OstTraceFunctionExit0( CDPSXMLGENERATOR_ENDELEMENTL_EXIT );
    }
