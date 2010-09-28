// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include <mtp/cmtptypeinterdependentpropdesc.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtptypestring.h>

#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "mtpframeworkconst.h"
#include "cmtpgetformatcapabilities.h"



EXPORT_C MMTPRequestProcessor* CMTPGetFormatCapabilities::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetFormatCapabilities* self = new (ELeave) CMTPGetFormatCapabilities(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
EXPORT_C CMTPGetFormatCapabilities::~CMTPGetFormatCapabilities()
    {
    //[SP-Format-0x3002]
    //Make the same behavior betwen 0x3000 and 0x3002.
	//iSingletons is used to judge whether FileDP supports 0x3002 or not.
    iSingletons.Close();
    
    delete iCapabilityList;
    }

void CMTPGetFormatCapabilities::ServiceL()
    {
    delete iCapabilityList;
    iCapabilityList = NULL;
    iCapabilityList = CMTPTypeFormatCapabilityList::NewL();
    iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);

    /**
     * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
     * but it does not really own the format 0x3002.
     * 
     * Make the same behavior betwen 0x3000 and 0x3002.
     */
    if((EMTPFormatCodeUndefined == iFormatCode) || (EMTPFormatCodeScript == iFormatCode))
        {
        BuildFormatAsUndefinedL(iFormatCode);
        }
    else if(EMTPFormatCodeAssociation == iFormatCode)
        {
        BuildFormatAssociationL();
        }
    else
        {
        if(iFramework.DataProviderId() == KMTPDeviceDPID)
           {
           BuildFormatAssociationL();
           }
       else if(iFramework.DataProviderId() == KMTPFileDPID)
           {
           BuildFormatAsUndefinedL(EMTPFormatCodeUndefined);
           
           //[SP-Format-0x3002]
           //Make the same behavior betwen 0x3000 and 0x3002.
           CMTPDataProvider& filedp(iSingletons.DpController().DataProviderL(KMTPFileDPID));
           if(filedp.SupportedCodes(EObjectCaptureFormats).Find(EMTPFormatCodeScript) != KErrNotFound)
        	   {
        	   BuildFormatAsUndefinedL(EMTPFormatCodeScript);
        	   }
           }
        }
    
    
    SendDataL(*iCapabilityList);    
    }
    
void CMTPGetFormatCapabilities::BuildFormatAssociationL()
    {
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC( EMTPFormatCodeAssociation ,interDesc );
    
    //EMTPObjectPropCodeStorageID
    CMTPTypeObjectPropDesc* storageId = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeStorageID);
    frmCap->AppendL(storageId);
    CleanupStack::Pop(storageId);
    
    //EMTPObjectPropCodeObjectFormat
    CMTPTypeObjectPropDesc* objFormat = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectFormat);
    frmCap->AppendL(objFormat);
    CleanupStack::Pop(objFormat);
    
    //EMTPObjectPropCodeProtectionStatus
    CMTPTypeObjectPropDesc* protectionStatus = ServiceProtectionStatusL();
    CleanupStack::PushL(protectionStatus);
    frmCap->AppendL(protectionStatus);
    CleanupStack::Pop(protectionStatus);
    
    //EMTPObjectPropCodeAssociationType
    CMTPTypeObjectPropDesc* associationType = ServiceAssociationTypeL();
    CleanupStack::PushL(associationType);
    frmCap->AppendL(associationType);
    CleanupStack::Pop(associationType);
    
    //EMTPObjectPropCodeAssociationDesc
    CMTPTypeObjectPropDesc* associationDesc = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeAssociationDesc);
    frmCap->AppendL(associationDesc);
    CleanupStack::Pop(associationDesc);
    
    //EMTPObjectPropCodeObjectSize
    CMTPTypeObjectPropDesc* objSize = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectSize);
    frmCap->AppendL(objSize);
    CleanupStack::Pop(objSize);
    
    //EMTPObjectPropCodeObjectFileName
    _LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName ); 
    CMTPTypeObjectPropDesc* objFileName = CMTPTypeObjectPropDesc::NewLC( EMTPObjectPropCodeObjectFileName,
                CMTPTypeObjectPropDesc::ERegularExpressionForm, form);
    frmCap->AppendL(objFileName);
    CleanupStack::Pop(objFileName);
    CleanupStack::PopAndDestroy(form );       
    
    //EMTPObjectPropCodeDateModified
    CMTPTypeObjectPropDesc* dateModified = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeDateModified);
    frmCap->AppendL(dateModified);
    CleanupStack::Pop(dateModified);
    
    //EMTPObjectPropCodeParentObject
    CMTPTypeObjectPropDesc* parentobj = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeParentObject);
    frmCap->AppendL(parentobj);
    CleanupStack::Pop(parentobj);
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    CMTPTypeObjectPropDesc* objectIdentifier = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    frmCap->AppendL(objectIdentifier);
    CleanupStack::Pop(objectIdentifier);
    
    //EMTPObjectPropCodeName
    CMTPTypeObjectPropDesc* name = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeName);
    frmCap->AppendL(name);
    CleanupStack::Pop(name);
    
    //EMTPObjectPropCodeNonConsumable
    CMTPTypeObjectPropDesc* nonConsumable = ServiceNonConsumableL();
    CleanupStack::PushL(nonConsumable);
    frmCap->AppendL(nonConsumable);
    CleanupStack::Pop(nonConsumable);
	
    //Hidden
	CMTPTypeObjectPropDesc* hidden = ServiceHiddenL();
    CleanupStack::PushL( hidden );
    frmCap->AppendL( hidden );
	CleanupStack::Pop( hidden ); 
    
    iCapabilityList->AppendL(frmCap);
    CleanupStack::Pop(frmCap);
    CleanupStack::Pop(interDesc);
    }


void CMTPGetFormatCapabilities::BuildFormatAsUndefinedL( TUint aFormatCode )
    {
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC( aFormatCode ,interDesc );
    
    //EMTPObjectPropCodeStorageID
    CMTPTypeObjectPropDesc* storageID = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeStorageID);
    frmCap->AppendL(storageID);
    CleanupStack::Pop(storageID);
    
    //EMTPObjectPropCodeObjectFormat
    CMTPTypeObjectPropDesc* objectFormat = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectFormat);
    frmCap->AppendL(objectFormat);
    CleanupStack::Pop(objectFormat);
    
    //EMTPObjectPropCodeProtectionStatus
    CMTPTypeObjectPropDesc* protectionStatus = ServiceProtectionStatusL();
    CleanupStack::PushL(protectionStatus);
    frmCap->AppendL(protectionStatus);
    CleanupStack::Pop(protectionStatus);
    
    //EMTPObjectPropCodeObjectSize
    CMTPTypeObjectPropDesc* objSize = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeObjectSize);
    frmCap->AppendL(objSize);
    CleanupStack::Pop(objSize);
    
    //EMTPObjectPropCodeObjectFileName
    _LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName );
    CMTPTypeObjectPropDesc* objFileName = CMTPTypeObjectPropDesc::NewLC( EMTPObjectPropCodeObjectFileName,
                CMTPTypeObjectPropDesc::ERegularExpressionForm, form);
    frmCap->AppendL(objFileName);
    CleanupStack::Pop(objFileName);
    CleanupStack::PopAndDestroy(form );     
    
    //EMTPObjectPropCodeDateModified
    CMTPTypeObjectPropDesc* dateModified = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeDateModified);
    frmCap->AppendL(dateModified);
    CleanupStack::Pop(dateModified);
    
    //EMTPObjectPropCodeParentObject
    CMTPTypeObjectPropDesc* parentObject = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeParentObject);
    frmCap->AppendL(parentObject);
    CleanupStack::Pop(parentObject);
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    CMTPTypeObjectPropDesc* objectIdentifier = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodePersistentUniqueObjectIdentifier);
    frmCap->AppendL(objectIdentifier);
    CleanupStack::Pop(objectIdentifier);
    
    //EMTPObjectPropCodeName
    CMTPTypeObjectPropDesc* name = CMTPTypeObjectPropDesc::NewLC(EMTPObjectPropCodeName);
    frmCap->AppendL(name);
    CleanupStack::Pop(name);
    
    //EMTPObjectPropCodeNonConsumable
    CMTPTypeObjectPropDesc* nonConsumable = ServiceNonConsumableL();
    CleanupStack::PushL(nonConsumable);
    frmCap->AppendL(nonConsumable);
    CleanupStack::Pop(nonConsumable);
    //Hidden
	CMTPTypeObjectPropDesc* hidden = ServiceHiddenL();
    CleanupStack::PushL( hidden );
    frmCap->AppendL( hidden );
	CleanupStack::Pop( hidden ); 
    
    iCapabilityList->AppendL(frmCap);
    CleanupStack::Pop(frmCap);
    CleanupStack::Pop(interDesc);
    }
    
CMTPTypeObjectPropDesc* CMTPGetFormatCapabilities::ServiceProtectionStatusL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    //Currently, we only support EMTPProtectionNoProtection and EMTPProtectionReadOnly
//  TUint16 values[] = {EMTPProtectionNoProtection, EMTPProtectionReadOnly, EMTPProtectionReadOnlyData, EMTPProtectionNonTransferable};
    TUint16 values[] = {EMTPProtectionNoProtection, EMTPProtectionReadOnly};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }  
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeProtectionStatus, *expectedForm);          
    CleanupStack::PopAndDestroy(expectedForm);
    return ret;
    }

CMTPTypeObjectPropDesc* CMTPGetFormatCapabilities::ServiceAssociationTypeL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    const TUint16 KMtpValues[] = {EMTPAssociationTypeUndefined, EMTPAssociationTypeGenericFolder};
    const TUint KNumMtpValues(sizeof(KMtpValues) / sizeof(KMtpValues[0])); 
    const TUint16 KPtpValues[] = {EMTPAssociationTypeUndefined, EMTPAssociationTypeGenericFolder, EMTPAssociationTypeAlbum, EMTPAssociationTypeTimeSequence, EMTPAssociationTypeHorizontalPanoramic, EMTPAssociationTypeVerticalPanoramic, EMTPAssociationType2DPanoramic,EMTPAssociationTypeAncillaryData};
    const TUint KNumPtpValues(sizeof(KPtpValues) / sizeof(KPtpValues[0]));

    TUint numValues(KNumMtpValues);
    const TUint16* values = KMtpValues;
    if (EModeMTP != iFramework.Mode())
         {
         numValues = KNumPtpValues;
         values = KPtpValues;            
         }
   
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }  
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeAssociationType, *expectedForm);       
    CleanupStack::PopAndDestroy(expectedForm);
    return ret;
    }

CMTPTypeObjectPropDesc* CMTPGetFormatCapabilities::ServiceNonConsumableL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT8);
    CleanupStack::PushL(expectedForm);
    TUint8 values[] = {0,1};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint8 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeNonConsumable, *expectedForm);     
    CleanupStack::PopAndDestroy(expectedForm);
    
    return ret;
    }

CMTPTypeObjectPropDesc* CMTPGetFormatCapabilities::ServiceHiddenL()
    {
    CMTPTypeObjectPropDescEnumerationForm* expectedForm = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT16);
    CleanupStack::PushL(expectedForm);
    TUint16 values[] = {EMTPVisible, EMTPHidden};
    TUint   numValues((sizeof(values) / sizeof(values[0])));
    for (TUint i = 0; i < numValues; i++)
        {
        TMTPTypeUint16 data(values[i]);
        expectedForm->AppendSupportedValueL(data);
        }   
    CMTPTypeObjectPropDesc* ret = CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeHidden, *expectedForm);     
    CleanupStack::PopAndDestroy(expectedForm);
    
    return ret;
    }

TMTPResponseCode CMTPGetFormatCapabilities::CheckRequestL()
    {
    TMTPResponseCode response = CMTPRequestProcessor::CheckRequestL(); 
    if( EMTPRespCodeOK != response )
        return response;
    
    TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    
    //[SP-Format-0x3002]
	//Make the same behavior betwen 0x3000 and 0x3002.
    if( (formatCode != EMTPFormatCodeUndefined) && (formatCode != EMTPFormatCodeAssociation) && (formatCode != KMTPFormatsAll) && ( EMTPFormatCodeScript != formatCode ))
        {
        return EMTPRespCodeInvalidObjectFormatCode;
        }

    if( (formatCode == EMTPFormatCodeAssociation) && (iFramework.DataProviderId() != KMTPDeviceDPID) )
        {
        return EMTPRespCodeInvalidObjectFormatCode;
        }
    
    if( (formatCode == EMTPFormatCodeUndefined) && (iFramework.DataProviderId() != KMTPFileDPID) )
        {
        return EMTPRespCodeInvalidObjectFormatCode;
        }
    
    return EMTPRespCodeOK;    
    }
    

CMTPGetFormatCapabilities::CMTPGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    }
    
void CMTPGetFormatCapabilities::ConstructL()
    {
    //[SP-Format-0x3002]
    //Make the same behavior betwen 0x3000 and 0x3002.
	//iSingletons is used to judge whether FileDP supports 0x3002 or not.
    iSingletons.OpenL();
    }


   


