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


#include "dpsxmlparser.h"
#include "dpsconst.h"
#include "pictbridge.h"
#include "dpsxmlstring.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsxmlparserTraces.h"
#endif


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlParser* CDpsXmlParser::NewL(CDpsEngine* aEngine)
	{
    CDpsXmlParser* self = new (ELeave) CDpsXmlParser(aEngine);
    return self;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsXmlParser::~CDpsXmlParser()
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_CDPSXMLPARSER_DES_ENTRY );
    iDpsArgs.Close();
    OstTraceFunctionExit0( CDPSXMLPARSER_CDPSXMLPARSER_DES_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsXmlParser::CDpsXmlParser(CDpsEngine* aEngine) : iEngine(aEngine)
    {
    OstTraceFunctionEntry0( DUP1_CDPSXMLPARSER_CDPSXMLPARSER_CONS_ENTRY );
	Reset();
	OstTraceFunctionExit0( DUP1_CDPSXMLPARSER_CDPSXMLPARSER_CONS_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnStartDocumentL(
		const RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONSTARTDOCUMENTL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONSTARTDOCUMENTL, "---, error code is %d", aErrorCode );
        User::Leave(aErrorCode);
        }
	OstTraceFunctionExit0( CDPSXMLPARSER_ONSTARTDOCUMENTL_EXIT );
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnEndDocumentL(TInt aErrorCode)
	{
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONENDDOCUMENTL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONENDDOCUMENTL, "---, error code is %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    
    OstTraceFunctionExit0( CDPSXMLPARSER_ONENDDOCUMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnStartElementL(const RTagInfo& aElement, 
									const RAttributeArray& aAttributes, 
									TInt aErrCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONSTARTELEMENTL_ENTRY );
    
    if (aErrCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONSTARTELEMENTL, "---, error code is %d", aErrCode );
        User::Leave(aErrCode);
        }
    if (aAttributes.Count() > 1)
        {
        OstTrace0( TRACE_ERROR, DUP1_CDPSXMLPARSER_ONSTARTELEMENTL, "---cannot have more than one attribute!" );
        User::Leave(KErrArgument);
        }
    	
    // Gets the name of the tag
    const TDesC8& name = aElement.LocalName().DesC();
    
    OstTraceExt1( TRACE_NORMAL, DUP2_CDPSXMLPARSER_ONSTARTELEMENTL, "---Start---%s", name );
    //Checks the element
    // this is the first layer <dps>
    if (!name.Compare(KDpsXml))
        {
        iAction = EDpsXmlStart;
        }
    // this is the second layer	<input|output>
    else if (!name.Compare(KDpsXmlOutput))
        {
        iAction = EDpsXmlOutput;
        }
    else if (!name.Compare(KDpsXmlInput))
        {
        // operation and event cannot be distiguised here
        iAction = EDpsXmlInput;
        }
    // this is the third layer <operation|event|result>	
    else if (iAction == EDpsXmlOutput && !name.Compare(KDpsXmlResult))
        {
        // output must be result
        iAction = EDpsXmlResult;
        }		
    else if (iAction == EDpsXmlInput &&
        (!name.Compare(iEngine->DpsParameters()->iDpsEventStrings[0]) ||
        !name.Compare(iEngine->DpsParameters()->iDpsEventStrings[1])))
        {
        if (!name.Compare(iEngine->DpsParameters()->iDpsEventStrings[0]))
            {
            iDpsEvent = EDpsEvtNotifyJobStatus;
            }
        else
            {
            iDpsEvent = EDpsEvtNotifyDeviceStatus;	
            }
        iAction = EDpsXmlEvent;	
        }
    else if (iAction == EDpsXmlInput || iAction == EDpsXmlResult)
        {
        iAction = EDpsXmlOperation;
        for (TInt i= 0; i < EDpsOpMax-1; i++) //i=0 as AppendL() method adds item on first free position in array
            {
            if (!name.Compare(iEngine->DpsParameters()->iDpsOperationStrings[i]))
                {
                OstTraceExt1( TRACE_NORMAL, DUP3_CDPSXMLPARSER_ONSTARTELEMENTL, "---found the operation---%s", name );
                iDpsOperation = (TDpsOperation)(i+1);// i+1 to by pass first operation EDpsOpEmpty in enum which is not added to array
                break;
                }
            }
        }
		
    // below is the fourth layer, e.g. the argument list/element. there
    // could be many arugments, but only one element	
    else if (iAction == EDpsXmlOperation || iAction == EDpsXmlEvent)
        {
        // only element in output is capability
        if (!name.Compare(iEngine->DpsParameters()->iDpsElementStrings[0]))
            {
            OstTraceExt1( TRACE_NORMAL, DUP4_CDPSXMLPARSER_ONSTARTELEMENTL, "---the element is %s", name );
            iElement = EDpsCapability;
            }
        else
            {
            for (TInt i = 0; i < EDpsArgMax; i++)
                {
                if (!name.Compare(iEngine->DpsParameters()->iDpsArgStrings[i]))
                    {
                    OstTraceExt1( TRACE_NORMAL, DUP5_CDPSXMLPARSER_ONSTARTELEMENTL, "---the argument is %s", name );
                    TDpsArg arg;
                    arg.iElement = (TDpsArgument)i;
                    arg.iContent.Copy(_L(""));
                    iDpsArgs.Append(arg);
                    break;
                    }
                }
            }
		
        }
    else
        {
        // something wrong
        OstTraceExt1( TRACE_ERROR, DUP6_CDPSXMLPARSER_ONSTARTELEMENTL, "--- non-PB element! %S", name );
        User::Leave(KErrNotSupported);
        }
    if (aAttributes.Count() == 1)
        {
        ParseAttributesL(aAttributes, name);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONSTARTELEMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	     
void CDpsXmlParser::ParseAttributesL(const RAttributeArray& aAttributes, 
                                     const TDesC8& tag)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_PARSEATTRIBUTESL_ENTRY );
    // element
    const TDesC8& name = aAttributes[0].Attribute().LocalName().DesC();
    // only "layouts" and "paperTypes" have attributes
    if (tag.Compare(KDpsXmlPaperTypes) && tag.Compare(KDpsXmlLayouts))
        {
        // error
        OstTraceExt1( TRACE_ERROR, CDPSXMLPARSER_PARSEATTRIBUTESL, "--- this tag %s have no attribute", name );
        User::Leave(KErrArgument);
        }
    // the element of the attributes must be "paperSize"	
    if (name.Compare(KDpsXmlPaperSize))
        {
        // error
        OstTraceExt1( TRACE_ERROR, DUP1_CDPSXMLPARSER_PARSEATTRIBUTESL, "--- wrong attribute--- %s", name );
        User::Leave(KErrArgument);
        }
    // value
    HBufC8* value = aAttributes[0].Value().DesC().AllocLC();
    TUint32 result;    
    TLex8 converter(*value);
    TInt error = converter.Val(result, EHex);
    if (error != KErrNone)
        {
        OstTrace1( TRACE_ERROR, DUP2_CDPSXMLPARSER_PARSEATTRIBUTESL, "--- convert error %d", error );
        User::Leave(error);
        }
    iAttrib = result >> KShiftLength; 
    CleanupStack::PopAndDestroy(value);
    OstTrace1( TRACE_NORMAL, DUP3_CDPSXMLPARSER_PARSEATTRIBUTESL, "--- attribte value %x", result );
  
    OstTraceFunctionExit0( CDPSXMLPARSER_PARSEATTRIBUTESL_EXIT );
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnEndElementL(const RTagInfo& aElement, 
								  TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONENDELEMENTL_ENTRY );
    
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONENDELEMENTL, "--- error code is %d", aErrorCode );
        User::Leave(aErrorCode);
        }
        
    // Get the name of the tag
    const TDesC8& name = aElement.LocalName().DesC();
    OstTraceExt1( TRACE_NORMAL, DUP1_CDPSXMLPARSER_ONENDELEMENTL, "%s", name );    
    OstTraceFunctionExit0( CDPSXMLPARSER_ONENDELEMENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
	{
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONCONTENTL_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CDPSXMLPARSER_ONCONTENTL, "content is %s", aBytes );
    
    if (aErrorCode != KErrNone)
   	    {
        OstTrace1( TRACE_ERROR, DUP1_CDPSXMLPARSER_ONCONTENTL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }	
    if (aBytes[0] >= KSOH && aBytes[0] <= KSpace)
        {
        OstTrace1( TRACE_NORMAl, DUP2_CDPSXMLPARSER_ONCONTENTL, "the unprintable char %d", aBytes[0] );
        OstTraceFunctionExit0( CDPSXMLPARSER_ONCONTENTL_EXIT );
        return; 
        }
    // parses the result	
    if (iAction == EDpsXmlResult)
        {
        TUint32 value;
        TLex8 converter(aBytes);
        TInt error = converter.Val(value, EHex);
        if (error != KErrNone)
            {
            OstTrace1( TRACE_ERROR, DUP3_CDPSXMLPARSER_ONCONTENTL, "--- convert error %d", error );
            User::Leave(error);
            }
        OstTrace1( TRACE_NORMAL, DUP4_CDPSXMLPARSER_ONCONTENTL, "--- result %x", value);
        
        // we have got the result
        iDpsResult.iMajorCode = 
            static_cast<TDpsResultMajorCode>(value >> KShiftLength);
        iDpsResult.iMinorCode = 
            static_cast<TDpsResultMinorCode>(value & KDpsMinorMask);
        }
    // gets the argument	
    else if (iAction == EDpsXmlOperation || iAction == EDpsXmlEvent)
        {
        iDpsArgs[iDpsArgs.Count() - 1].iContent.Copy(aBytes);
        }
    OstTraceFunctionExit0( DUP1_CDPSXMLPARSER_ONCONTENTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnStartPrefixMappingL(const RString& /*aPrefix*/, 
										  const RString& /*aUri*/, 
										  TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONSTARTPREFIXMAPPINGL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONSTARTPREFIXMAPPINGL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONSTARTPREFIXMAPPINGL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnEndPrefixMappingL(const RString& /*aPrefix*/, 
									    TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONENDPREFIXMAPPINGL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONENDPREFIXMAPPINGL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONENDPREFIXMAPPINGL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, 
                                           TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONIGNORABLEWHITESPACEL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONIGNORABLEWHITESPACEL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONIGNORABLEWHITESPACEL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnSkippedEntityL(const RString& /*aName*/, 
								     TInt aErrorCode)
	{
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONSKIPPEDENTITYL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONSKIPPEDENTITYL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONSKIPPEDENTITYL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnProcessingInstructionL(const TDesC8& /*aTarget*/, 
											 const TDesC8& /*aData*/, 
											 TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONPROCESSINGINSTRUCTIONL_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_ERROR, CDPSXMLPARSER_ONPROCESSINGINSTRUCTIONL, "--- error code %d", aErrorCode );
        User::Leave(aErrorCode);
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONPROCESSINGINSTRUCTIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnError(TInt aErrorCode)
    {
    OstTraceFunctionEntry0( CDPSXMLPARSER_ONERROR_ENTRY );
    if (aErrorCode != KErrNone)
        {
        OstTrace1( TRACE_NORMAL, CDPSXMLPARSER_ONERROR, "---error code %d", aErrorCode );
        }
    OstTraceFunctionExit0( CDPSXMLPARSER_ONERROR_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	

TAny* CDpsXmlParser::GetExtendedInterface(const TInt32 aUid)
    {
    return aUid == MDpsExtensionHandler::EInterfaceUid ?
    static_cast<MDpsExtensionHandler*>(this) : 0;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//  
void CDpsXmlParser::Reset()
    {
    iDpsArgs.Reset();
    iAction = EDpsXmlEmpty;
    iDpsEvent = EDpsEvtEmpty; 
    iDpsOperation = EDpsOpEmpty;
    iElement = EDpsEleEmpty; 
    iAttrib = 0;
    }

