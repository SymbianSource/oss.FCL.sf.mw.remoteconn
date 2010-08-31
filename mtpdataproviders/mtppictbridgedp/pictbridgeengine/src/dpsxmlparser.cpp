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


#include <e32debug.h>
#include "dpsxmlparser.h"
#include "dpsconst.h"
#include "pictbridge.h"
#include "dpsxmlstring.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#   define PRINT_DES(t)\
 {TBuf<KMaxArgLen> _buf; _buf.Copy(t);RDebug::Print(_L("---%S"), &_buf);}
#else
#	define IF_DEBUG(t)
#   define PRINT_DES(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsXmlParser* CDpsXmlParser::NewL(CDpsEngine* aEngine)
	{
    IF_DEBUG(Print(_L("CDpsXmlParser::NewL")));
    CDpsXmlParser* self = new (ELeave) CDpsXmlParser(aEngine);
    return self;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsXmlParser::~CDpsXmlParser()
    {
    IF_DEBUG(Print(_L(">>>~CDpsXmlParser")));
    iDpsArgs.Close();
    __IF_DEBUG(Print(_L("<<<~CDpsXmlParser")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsXmlParser::CDpsXmlParser(CDpsEngine* aEngine) : iEngine(aEngine)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::Ctor")));
	Reset();
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::Ctor")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnStartDocumentL(
		const RDocumentParameters& /*aDocParam*/, TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnStartDocumentL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("---, error code is %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnStartDocumentL")));
	}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnEndDocumentL(TInt aErrorCode)
	{
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnEndDocumentL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("---, error code is %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnEndDocumentL")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnStartElementL(const RTagInfo& aElement, 
									const RAttributeArray& aAttributes, 
									TInt aErrCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnStartElementL")));
    
    if (aErrCode != KErrNone)
        {
        IF_DEBUG(Print(_L("---, error code is %d"), aErrCode));
        User::Leave(aErrCode);
        }
    if (aAttributes.Count() > 1)
        {
        IF_DEBUG(Print(_L("---cannot have more than one attribute!")));
        User::Leave(KErrArgument);
        }
    	
    // Gets the name of the tag
    const TDesC8& name = aElement.LocalName().DesC();
    
    IF_DEBUG(Print(_L("---Start"))); PRINT_DES(name);
	
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
                IF_DEBUG(Print(_L("---found the operation")));
                PRINT_DES(name);
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
            IF_DEBUG(Print(_L("---the element is")));
            PRINT_DES(name);
            iElement = EDpsCapability;
            }
        else
            {
            for (TInt i = 0; i < EDpsArgMax; i++)
                {
                if (!name.Compare(iEngine->DpsParameters()->iDpsArgStrings[i]))
                    {
                    IF_DEBUG(Print(_L("---the argument is ")));
                    PRINT_DES(name);	
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
        IF_DEBUG(Print(_L("--- non-PB element! %S"), &name));
        User::Leave(KErrNotSupported);
        }
    if (aAttributes.Count() == 1)
        {
        ParseAttributesL(aAttributes, name);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnStartElementL")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	     
void CDpsXmlParser::ParseAttributesL(const RAttributeArray& aAttributes, 
                                     const TDesC8& tag)
    {
    // element
    const TDesC8& name = aAttributes[0].Attribute().LocalName().DesC();
    // only "layouts" and "paperTypes" have attributes
    if (tag.Compare(KDpsXmlPaperTypes) && tag.Compare(KDpsXmlLayouts))
        {
        // error
        IF_DEBUG(Print(_L("--- this tag have no attribute")));
        PRINT_DES(name);
        User::Leave(KErrArgument);
        }
    // the element of the attributes must be "paperSize"	
    if (name.Compare(KDpsXmlPaperSize))
        {
        // error
        IF_DEBUG(Print(_L("--- wrong attribute")))
        PRINT_DES(name);
        User::Leave(KErrArgument);
        }
    // value
    HBufC8* value = aAttributes[0].Value().DesC().AllocLC();
    TUint32 result;    
    TLex8 converter(*value);
    TInt error = converter.Val(result, EHex);
    if (error != KErrNone)
        {
        IF_DEBUG(Print(_L("--- convert error %d"), error));
        User::Leave(error);
        }
    iAttrib = result >> KShiftLength; 
    CleanupStack::PopAndDestroy(value);
    IF_DEBUG(Print(_L("--- attribte value %x"), result));
     
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnEndElementL(const RTagInfo& aElement, 
								  TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnEndElementL")));
    
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("--- error code is %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
        
    // Get the name of the tag
    const TDesC8& name = aElement.LocalName().DesC();
    PRINT_DES(name);
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnEndElementL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
	{
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnContentL content is")));
    PRINT_DES(aBytes);
    
    if (aErrorCode != KErrNone)
   	    {
        IF_DEBUG(Print(_L("--- error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }	
    if (aBytes[0] >= KSOH && aBytes[0] <= KSpace)
        {
        IF_DEBUG(Print(_L("the unprintable char %d"), aBytes[0]));
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
            IF_DEBUG(Print(_L("--- convert error %d"), error));
            User::Leave(error);
            }
        IF_DEBUG(Print(_L("--- result %x"), value));
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
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnContentL")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnStartPrefixMappingL(const RString& /*aPrefix*/, 
										  const RString& /*aUri*/, 
										  TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnStartPrefixMappingL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("--- error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnStartPrefixMappingL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnEndPrefixMappingL(const RString& /*aPrefix*/, 
									    TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnEndPrefixMappingL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("--- error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnEndPrefixMappingL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsXmlParser::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, 
                                           TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnIgnorableWhiteSpaceL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("---error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnIgnorableWhiteSpaceL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnSkippedEntityL(const RString& /*aName*/, 
								     TInt aErrorCode)
	{
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnSkippedEntityL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("--- error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnSkippedEntityL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnProcessingInstructionL(const TDesC8& /*aTarget*/, 
											 const TDesC8& /*aData*/, 
											 TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnProcessingInstructionL")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("--- error code %d"), aErrorCode));
        User::Leave(aErrorCode);
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnProcessingInstructionL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
void CDpsXmlParser::OnError(TInt aErrorCode)
    {
    IF_DEBUG(Print(_L(">>>CDpsXmlParser::OnError()")));
    if (aErrorCode != KErrNone)
        {
        IF_DEBUG(Print(_L("---error code %d"), aErrorCode));
        }
    IF_DEBUG(Print(_L("<<<CDpsXmlParser::OnError()")));	
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

