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
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID) );
    
    //EMTPObjectPropCodeObjectFormat
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat) );
    
    //EMTPObjectPropCodeProtectionStatus
    frmCap->AppendL( ServiceProtectionStatusL() );
    
    //EMTPObjectPropCodeAssociationType
    frmCap->AppendL( ServiceAssociationTypeL() );
    
    //EMTPObjectPropCodeAssociationDesc
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeAssociationDesc) );
    
    //EMTPObjectPropCodeObjectSize
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize) );
    
    //EMTPObjectPropCodeObjectFileName
    _LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName );   
    frmCap->AppendL(CMTPTypeObjectPropDesc::NewL( EMTPObjectPropCodeObjectFileName,
            CMTPTypeObjectPropDesc::ERegularExpressionForm, form));
    CleanupStack::PopAndDestroy(form );       
    
    //EMTPObjectPropCodeDateModified
    frmCap->AppendL(  CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified) );
    
    //EMTPObjectPropCodeParentObject
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject) );
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier) );
    
    //EMTPObjectPropCodeName
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName)); 
    
    //EMTPObjectPropCodeNonConsumable
    frmCap->AppendL(ServiceNonConsumableL() );
    
    iCapabilityList->AppendL(frmCap);
    CleanupStack::Pop(frmCap);
    CleanupStack::Pop(interDesc);
    }


void CMTPGetFormatCapabilities::BuildFormatAsUndefinedL( TUint aFormatCode )
    {
    CMTPTypeInterdependentPropDesc*  interDesc = CMTPTypeInterdependentPropDesc::NewLC();
    CMTPTypeFormatCapability* frmCap = CMTPTypeFormatCapability::NewLC( aFormatCode ,interDesc );
    
    //EMTPObjectPropCodeStorageID
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeStorageID) );
    
    //EMTPObjectPropCodeObjectFormat
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectFormat) );
    
    //EMTPObjectPropCodeProtectionStatus
    frmCap->AppendL( ServiceProtectionStatusL() );
    
    //EMTPObjectPropCodeObjectSize
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeObjectSize) );
    
    //EMTPObjectPropCodeObjectFileName
    _LIT(KMtpObjDescObjFileName, "[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 7}\\.[[a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~][a-zA-Z!#\\$%&'\\(\\)\\-0-9@\\^_\\`\\{\\}\\~ ]{0, 2}]?");
    CMTPTypeString* form = CMTPTypeString::NewLC( KMtpObjDescObjFileName );   
    frmCap->AppendL(CMTPTypeObjectPropDesc::NewL( EMTPObjectPropCodeObjectFileName,
            CMTPTypeObjectPropDesc::ERegularExpressionForm, form));
    CleanupStack::PopAndDestroy(form );     
    
    //EMTPObjectPropCodeDateModified
    frmCap->AppendL(  CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeDateModified) );
    
    //EMTPObjectPropCodeParentObject
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeParentObject) );
    
    //EMTPObjectPropCodePersistentUniqueObjectIdentifier
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodePersistentUniqueObjectIdentifier) );
    
    //EMTPObjectPropCodeName
    frmCap->AppendL( CMTPTypeObjectPropDesc::NewL(EMTPObjectPropCodeName)); 
    
    //EMTPObjectPropCodeNonConsumable
    frmCap->AppendL(ServiceNonConsumableL() );
    
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


   


