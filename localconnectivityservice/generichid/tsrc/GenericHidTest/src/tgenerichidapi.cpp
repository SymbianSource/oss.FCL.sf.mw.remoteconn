/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  HID field search
*
*/

#include <e32std.h>

#include "tGenericHIDAPI.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CtGenericHIDAPI* CtGenericHIDAPI::NewLC()
    {    
    CtGenericHIDAPI* self = new (ELeave) CtGenericHIDAPI();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CtGenericHIDAPI* CtGenericHIDAPI::NewL()
    {
    CtGenericHIDAPI* self = NewLC();
    CleanupStack::Pop();
    return self;    
    }
// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CtGenericHIDAPI::CtGenericHIDAPI()
    {    
    }
// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//    
CtGenericHIDAPI::~CtGenericHIDAPI()
    {
    delete iHidDriver;
    iFieldList.Close();
    delete iGenHID;
    }
// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::ConstructL()
    {
    iGenHID = CGenericHid::NewL( this );
    iDriverAcces=iGenHID;

    iHidDriver = CHidDriver::NewL(TUid::Uid(0xE000B187),iGenHID);
    }
// ---------------------------------------------------------------------------
// ConnectL()
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::ConnectL(TInt aConnectionId, const TDesC8& aDescriptor)
    {
    TInt16 ret = iGenHID->ConnectedL( aConnectionId, aDescriptor );
    ret = iGenHID->DriverActive( aConnectionId , CHidTransport::EActive);
    
    //todo retval
    }

// ---------------------------------------------------------------------------
// DisconnectL()
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::DisconnectL(TInt aConnectionId )
    {    
    TInt16 ret = iGenHID->Disconnected( aConnectionId );
    //todo retval
    }

// ---------------------------------------------------------------------------
// DataInL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::DataInL(TInt aConnectionId, const TDesC8& aDescriptor)
    {
    TInt16 ret = iGenHID->DataIn(aConnectionId, CHidTransport::EHidChannelInt, aDescriptor);
    return ret;
    }

// ---------------------------------------------------------------------------
// RunCountryCodeL()
// ---------------------------------------------------------------------------
//    
TInt CtGenericHIDAPI::RunCountryCodeL()
    {
    TInt16 countryCode = iGenHID->CountryCodeL(0);
    if(countryCode != 0)
    	return KErrNotFound;
    
    return KErrNone;
    }
 
// ---------------------------------------------------------------------------
// RunVendorIdL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunVendorIdL()
    {
    TUint vendorId = iGenHID->VendorIdL(0);
    if(vendorId != 0x0421)
    	return KErrNotFound;
    return KErrNone;
    }
        
// ---------------------------------------------------------------------------
// RunProductIdL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::RunProductIdL()
    {
    TUint productId = iGenHID->ProductIdL(0);
    if(productId != 0x0083)
    	return KErrNotFound;
    
    return KErrNone;
    }
 
// ---------------------------------------------------------------------------
// RunSetProtocol()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunSetProtocolL()
    {
    iGenHID->SetProtocolL(0,0,MDriverAccess::EReport,iHidDriver);
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// RunGetProtocoL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunGetProtocoL()
    {
    iGenHID->GetProtocolL(0,0);    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// RunGetReportL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunGetReportL()
    {
    iGenHID->GetReportL(0, 0 ,0 , 0);    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// RunSetReportL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunSetReportL(const TDesC8& aDescriptor)
    {
    iGenHID->SetReportL(0,0, MDriverAccess::EOutput, aDescriptor,0, iHidDriver);
    return KErrNone;
    }
                
// ---------------------------------------------------------------------------
// GetDataOutL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::RunDataOutL(const TDesC8& aDescriptor)
    {
    iGenHID->DataOutL( 0, 0,  aDescriptor, 0);    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// GetGetIdleL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::GetGetIdleL()
    {
    iGenHID->GetIdleL(0, 0, 0);
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// RunSetIdleL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::RunSetIdleL()
    {
    iGenHID->SetIdleL(0, 10, 0, 0, iHidDriver);
    return KErrNone;
    }
        
// ---------------------------------------------------------------------------
// GetCommandResultL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::GetCommandResultL()
    {
    iGenHID->CommandResult(0, 1);
    return KErrNone;
    }
        
// ---------------------------------------------------------------------------
// GetReportDescriptorL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::GetReportDescriptorL()
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    if(tmp == NULL)
    	return KErrNotFound;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CreateReportGeneratorL
// ---------------------------------------------------------------------------
//    
TInt CtGenericHIDAPI::CreateReportGeneratorL()   
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    
    if(tmp == NULL)
    	return KErrNotFound;
    
    iReportGenerator = CReportGenerator::NewL(tmp, 0, CField::EInput);
    
    if(iReportGenerator == NULL)
    	return KErrNotFound;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// DeleteReportGeneratorL()
// ---------------------------------------------------------------------------
//        
TInt CtGenericHIDAPI::DeleteReportGeneratorL()   
    {    
    delete iReportGenerator;
    iReportGenerator = NULL;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// ReportGeneratorReport()
// ---------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportGeneratorReport()
    {
    iReportGenerator->Report();    
    //todo check return value???
    return KErrNone;
    }    

// ---------------------------------------------------------------------------
// ReportGeneratorReport()
// ---------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportGeneratorSetFieldL( TBool aIsArray, TBool aOutOfRange, TBool aFieldNull, TBool aFieldCountZero, TBool aExistingUsageId, TBool aBadIndex )
    {
    CField* field = CreateCFieldL();
    
    TInt usageId = EConsumerUsageVolumeInc;//233;//0xE9
    TInt indexOffset = 0;
    
    if(aBadIndex != EFalse)
    	{
    	indexOffset = 1000;
    	}
    
    if(aExistingUsageId == EFalse)
    	{
    	usageId = EConsumerUsagePlayPause;//205
    	field->SetUsageRange(0,255);
    	}
    
    if(aIsArray == EFalse)
    	{
    	field->SetAttributes(KFieldAttributeVariable);
    	if(field->IsArray() != EFalse)
    		{
    		return KErrGeneral;
    		}
    	}
    else 
    {
		field->SetAttributes(!KFieldAttributeVariable);
		if(field->IsArray() == EFalse)
    	{
    	return KErrGeneral;
    	}
    }
    
    field->SetLogicalMax(10);
    if(aOutOfRange != EFalse)
    	{
    	field->SetLogicalMin(5);
    	}
    
    if(aFieldCountZero != EFalse)
    	{
    	field->SetCount(0);
    	}
    
    const CField* constField;
    if(aFieldNull == EFalse)
    	{
    	constField = (const CField*)field;
    	}
    else
    	{
    	constField = NULL;
    	}
    TInt16 ret = iReportGenerator->SetField(constField, usageId, 1, indexOffset);
    return ret;
    }    
            
// -----------------------------------------------------------------------------
// CtGenericHid::CreateReportTranslatorL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::CreateReportTranslatorL( const TDesC8& aDescriptor, TBool aIsArray, TBool aFieldNull, TBool aFieldCountZero)
    {
    CField* field = CreateCFieldL();
    
    
    if(aIsArray == EFalse)
    	{
    	field->SetAttributes(KFieldAttributeVariable);
    	if(field->IsArray() != EFalse)
    		{
    		return KErrGeneral;
    		}
    	}
    else 
    {
		field->SetAttributes(!KFieldAttributeVariable);
		if(field->IsArray() == EFalse)
    	{
    	return KErrGeneral;
    	}
    }
    
   /* field->SetLogicalMax(10);
    if(aOutOfRange != EFalse)
    	{
    	field->SetLogicalMin(5);
    	}*/
    
    if(aFieldCountZero != EFalse)
    	{
    	field->SetCount(0);
    	}
    
    const CField* constField;
    if(aFieldNull == EFalse)
    	{
    	constField = (const CField*)field;
    	}
    else
    	{
    	constField = NULL;
    	}
    
    
    
    
    //const CField* field = CreateConstCFieldL();
    iTranslator = new (ELeave) TReportTranslator(aDescriptor, constField);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CreateReportTranslator_FieldNullL
// -----------------------------------------------------------------------------
//   
/*TInt CtGenericHIDAPI::CreateReportTranslator_FieldNullL( const TDesC8& aDescriptor  )
    {
    const CField* field = NULL;
    iTranslator = new (ELeave) TReportTranslator(aDescriptor, field);
    return KErrNone;
    }*/

// -----------------------------------------------------------------------------
// CtGenericHid::DeleteReportTranslatorL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::DeleteReportTranslatorL(  )
    {
    delete iTranslator;    
    iTranslator =NULL;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorGetValueL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorGetValueL()
    {
    //todo if
    TInt value = 0;
    TInt res = iTranslator->GetValue(value,EConsumerUsageVolumeInc);

    return res;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorValueL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorValueL()
    {
    TInt value = iTranslator->ValueL(EConsumerUsageVolumeInc); 

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorGetUsageIdL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorGetUsageIdL( TBool aBadControlIndex )
    {
    TInt id;
    TInt index = 0;
    
    if(aBadControlIndex != EFalse)
    	{
    	index = -1;//bad index
    	}
    //todo if
    TInt ret = iTranslator->GetUsageId(id,index);

    return ret;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorUsageIdL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorUsageIdL( )
    {
    TInt value = iTranslator->UsageIdL(0);  

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorRawValueL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorRawValueL(   )
    {
    TInt value = iTranslator->RawValueL(0);  

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportTranslatorCountL
// -----------------------------------------------------------------------------
//   
TInt CtGenericHIDAPI::ReportTranslatorCountL( )
    {
    TInt count =iTranslator->Count();  
    //todo if, retval
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectiontypeL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectiontypeL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TUint32 type=tmp->Type();
    //todo check type?
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsPhysicalL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsPhysicalL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TBool result=tmp->IsPhysical();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsLogicalL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsLogicalL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TBool result=tmp->IsLogical();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsReportL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsReportL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TBool result=tmp->IsReport();   

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsNamedArrayL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsNamedArrayL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TBool result =tmp->IsNamedArray();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsUsageSwitchL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsUsageSwitchL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TBool resul = tmp->IsUsageSwitch();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsUsageModifierL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsUsageModifierL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->IsUsageModifier();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionIsApplicationL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionIsApplicationL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->IsApplication();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionUsagePageL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionUsagePageL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->UsagePage();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionUsageL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionUsageL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->Usage();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionCollectionCountL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionCollectionCountL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->CollectionCount();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionFieldCountL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionFieldCountL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt result = tmp->FieldCount();

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionCollectionByIndexL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionCollectionByIndexL( TBool aPresent )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt index;
    
    index = (aPresent != EFalse) ? 0 : -1;

    const CCollection* result = tmp->CollectionByIndex(index);
    
    if(aPresent != EFalse && result == NULL)
    	return KErrGeneral;
    
    if(aPresent == EFalse && result != NULL)
    	return KErrGeneral;

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::CollectionFieldByIndexL
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::CollectionFieldByIndexL( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    const CField* result = tmp->FieldByIndex(-1);
    
    if(result != NULL)
    	return KErrGeneral;
    //todo if

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CtGenericHid::ReportrootReportSizeBytes
// -----------------------------------------------------------------------------
// 
TInt CtGenericHIDAPI::ReportrootReportSizeBytes( )
    {
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);
    User::LeaveIfNull(tmp);
    TInt16 size = tmp->ReportSizeBytes(0,CField::EInput);  

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldLogicalMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldLogicalMaxL( )//todo remove
    {
    CField* field = CreateCFieldL();
    const TInt max = 5;
    field->SetLogicalMax(max);
    TInt ret = field->LogicalMax();
    if(ret != max)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldPhysicalMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldPhysicalMinL( )//todo remove
    {
    CField* field = CreateCFieldL();
    const TInt min = 0;
    field->SetPhysicalMin(min);
    TInt ret = field->PhysicalMin();
    if(ret != min)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldPhysicalMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldPhysicalMaxL( )//todo remove    
    {    
    CField* field = CreateCFieldL();
    const TInt max = 5;
    field->SetPhysicalMax(max);
    TInt ret=field->PhysicalMax();
    if(ret != max)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUnitL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUnitL( )
    {
    CField* field = CreateCFieldL();
    TInt unit = 5;
    field->SetUnit(unit);
    TInt ret = field->Unit();
    if(ret != unit)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUnitExponentL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUnitExponentL( )
    {    
    CField* field = CreateCFieldL();
    TInt unitExponent = 5;
    field->SetUnitExponent(unitExponent);
    TInt ret = field->UnitExponent();
    if(ret != unitExponent)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldCountL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldCountL( )
    {    
    CField* field = CreateCFieldL();
    TInt count = 5;
    field->SetCount(count);
    TInt ret = field->Count();
    if(ret != count)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSizeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSizeL( )
    {    
    CField* field = CreateCFieldL();
    TInt size = 5;
    field->SetSize(size);
    TInt ret = field->Size();
    if(ret != size)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldOffsetL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldOffsetL(TBool setReport )
    {    
    CField* field = CreateCFieldL();
    
    if(setReport != EFalse)
    	{
    	field->SetReportId(5);
    	}
    
    TInt offset = 5;
    field->SetOffset(offset);
    TInt ret = field->Offset();
    
    if(setReport != EFalse)
    	{
    	offset += KSizeOfByte;
    	}
    
    if(ret != offset)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldReportIdL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldReportIdL( )
    {    
    CField* field = CreateCFieldL();
    TInt reportId = 5;
    field->SetReportId(reportId);
    TInt ret = field->ReportId();
    if(ret != reportId)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsInReportL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsInReportL( )
    {    
    CField* field = CreateCFieldL();
    TInt reportId = 5;
    field->SetReportId(reportId);
    TBool ret = field->IsInReport(reportId);
    if(!ret)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldDesignatorIndexL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldDesignatorIndexL( )
    {    
    CField* field = CreateCFieldL();
    TInt designatorIndex = 5;
    field->SetDesignatorIndex(designatorIndex);
    TInt ret = field->DesignatorIndex();
    if(ret != designatorIndex)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldStringIndexL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldStringIndexL( )
    {    
    CField* field = CreateCFieldL();
    TInt stringIndex = 5;
    field->SetStringIndex(stringIndex);
    TInt ret = field->StringIndex();
    if(ret != stringIndex)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUsagePageL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUsagePageL( )
    {    
    CField* field = CreateCFieldL();
    TInt usagePage = 5;
    field->SetUsagePage(usagePage);
    TInt ret = field->UsagePage();
    if(ret != usagePage)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldHasUsageL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldHasUsageL( )
    {    
    CField* field = CreateCFieldL();
    TInt usage = 233;
    field->AddUsageL(usage);
    TBool ret = field->HasUsage(usage);
    if(!ret)
    	return KErrGeneral;
    //todo check EFalse
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUsageArrayL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUsageArrayL( )
    {    
    CField* field = CreateCFieldL();
    TInt usage = 233;
    field->AddUsageL(usage);
    TArray<TInt> usageArray = field->UsageArray();
    
    if(usageArray[0] != usage)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUsageL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUsageL( )
    {    
    CField* field = CreateCFieldL();
    TInt usage = 233;
    field->AddUsageL(usage);
    TInt ret = field->Usage(0);
    
    if(ret != usage)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldUsageCountL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldUsageCountL( )
    {    
    CField* field = CreateCFieldL();
    TInt firstCount = field->UsageCount();
    TInt usage = 233;
    field->AddUsageL(usage);
    TInt secondCount = field->UsageCount();
    
    if(secondCount != firstCount+1)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldClearUsageListL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldClearUsageListL( )
    {    
    CField* field = CreateCFieldL();
    TInt usage = 233;
    field->AddUsageL(usage);
    TInt ret = field->UsageCount();
    
    if(ret == 0)
    	return KErrGeneral;
    
    field->ClearUsageList();
    ret = field->UsageCount();
    if(ret != 0)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldLastUsageL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldLastUsageL( TBool empty )
    {    
    CField* field = CreateCFieldL();
    //todo if - without add
    TInt usage = 233;
    
    if(empty != EFalse)
    	{
    	field->ClearUsageList();
    	usage = 0;
    	}
    else
    	{
    	field->AddUsageL(usage);
    	}
    
    TInt ret = field->LastUsage();
    
    if(ret != usage)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldAttributesL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldAttributesL( )
    {
    CField* field = CreateCFieldL();
    TUint32 attributes = KFieldAttributeConstant;
    field->SetAttributes(attributes);
    TUint32 ret = field->Attributes();
    if(ret != attributes)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldTypeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldTypeL( )
    {
    CField* field = CreateCFieldL();
    CField::TType type = CField::EInput;
    field->SetType(type);
    CField::TType ret = field->Type();
    if(ret != type)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsVariableL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsVariableL( )
    {
    CField* field = CreateCFieldL();
    TUint32 attributes = KFieldAttributeVariable;
    field->SetAttributes(attributes);
    TBool ret = field->IsVariable();
    if(!ret)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsArrayL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsArrayL( )
    {
    CField* field = CreateCFieldL();
    TUint32 attributes = KFieldAttributeVariable;
    field->SetAttributes(attributes);
    TBool ret = field->IsArray();
    if(ret)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsConstantL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsConstantL(  )
    {    
    CField* field = CreateCFieldL();
    TUint32 attributes = KFieldAttributeConstant;
    field->SetAttributes(attributes);
    TBool ret = field->IsConstant();
    if(!ret)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsDataL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsDataL(  )
    {    
    CField* field = CreateCFieldL();
    TUint32 attributes = KFieldAttributeConstant;
    field->SetAttributes(attributes);
    TBool ret = field->IsData();
    if(ret)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetLogicalMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetLogicalMinL()
    {
    CField* field = CreateCFieldL();
    
    TInt value = 0;
    field->SetLogicalMin(value);
    
    TInt ret = field->LogicalMin();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetLogicalMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetLogicalMaxL( )
    {    
    CField* field = CreateCFieldL();
    TInt value = 10;
    field->SetLogicalMax(value);
    
    TInt ret = field->LogicalMax();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetPhysicalMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetPhysicalMinL( )
    {
    CField* field = CreateCFieldL();
    TInt value = 0;
    field->SetPhysicalMin(value);
    TInt ret = field->PhysicalMin();
    if(ret != value)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetPhysicalMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetPhysicalMaxL( ) 
    {
    CField* field = CreateCFieldL();
    TInt value = 2;
    field->SetPhysicalMax(value);
    TInt ret = field->PhysicalMax();
    if(ret != value)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetDesignatorMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetDesignatorMinL( )
    {
    CField* field = CreateCFieldL();
    TInt value = 0;
    field->SetDesignatorMin(value);
    TInt ret = field->DesignatorMin();
    if(ret != value)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetDesignatorMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetDesignatorMaxL(  )
    {
    CField* field = CreateCFieldL();
    TInt value = 2;
    field->SetDesignatorMax(value);
    TInt ret = field->DesignatorMax();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetUsageMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetUsageMinL( )
    {
    CField* field = CreateCFieldL();
    TInt value = 0;
    field->SetUsageMin(value);
    TInt ret = field->UsageMin();
    if(ret != value)
    	return KErrGeneral;
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetUsageMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetUsageMaxL(  )
    {
    CField* field = CreateCFieldL();
    TInt value = 2;
    field->SetUsageMax(value);
    TInt ret = field->UsageMax();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetStringMinL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetStringMinL( )
    {    
    CField* field = CreateCFieldL();
    TInt value = 0;
    field->SetStringMin(value);
    TInt ret = field->StringMin();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }
// ---------------------------------------------------------------------------
// FieldSetStringMaxL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetStringMaxL( )
    {
    CField* field = CreateCFieldL();
    TInt value = 2;
    field->SetStringMax(value);
    TInt ret = field->StringMax();
    if(ret != value)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetLogicalRangeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetLogicalRangeL( )
    {
    CField* field = CreateCFieldL();
    TInt min = 0;
    TInt max = 5;
    field->SetLogicalRange(min,max);
    TInt retMin = field->LogicalMin();
    TInt retMax = field->LogicalMax();
    if(min != retMin || max != retMax)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetUsageRangeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetUsageRangeL( )
    {
    CField* field = CreateCFieldL();
    TInt min = 0;
    TInt max = 5;
    field->SetUsageRange(min,max);
    TInt retMin = field->UsageMin();
    TInt retMax = field->UsageMax();
    if(min != retMin || max != retMax)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetPhysicalRangeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetPhysicalRangeL( )
    {
    CField* field = CreateCFieldL();
    TInt min = 0;
    TInt max = 5;
    field->SetPhysicalRange(min,max);
    TInt retMin = field->PhysicalMin();
    TInt retMax = field->PhysicalMax();
    if(min != retMin || max != retMax)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetStringRangeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetStringRangeL( )
    {
    CField* field = CreateCFieldL();
    TInt min = 0;
    TInt max = 5;
    field->SetStringRange(min,max);
    TInt retMin = field->StringMin();
    TInt retMax = field->StringMax();
    if(min != retMin || max != retMax)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldSetDesignatorRangeL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldSetDesignatorRangeL( )
    {
    CField* field = CreateCFieldL();
    TInt min = 0;
    TInt max = 5;
    field->SetDesignatorRange(min,max);
    TInt retMin = field->DesignatorMin();
    TInt retMax = field->DesignatorMax();
    if(min != retMin || max != retMax)
    	return KErrGeneral;

    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsInputL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsInputL( )
    {
    CField* field = CreateCFieldL();
    CField::TType type = CField::EInput;
    field->SetType(type);
    TBool ret = field->IsInput();
    if(!ret)
    	return KErrGeneral;
    //todo check EFalse
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsOutputL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsOutputL( )
    {
    CField* field = CreateCFieldL();
    CField::TType type = CField::EOutput;
    field->SetType(type);
    TBool ret = field->IsOutput();
    if(!ret)
    	return KErrGeneral;
    //todo check EFalse
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// FieldIsFeatureL()
// ---------------------------------------------------------------------------
//
TInt CtGenericHIDAPI::FieldIsFeatureL( )
    {    
    CField* field = CreateCFieldL();
    CField::TType type = CField::EFeature;
    field->SetType(type);
    TBool ret = field->IsFeature();
    if(!ret)
    	return KErrGeneral;
    //todo check EFalse
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CountryCodeL()
// ---------------------------------------------------------------------------
//
TUint CtGenericHIDAPI::CountryCodeL(TInt /*aConnID*/)
    {    
    return 0;
    }

// ---------------------------------------------------------------------------
// VendorIdL
// ---------------------------------------------------------------------------
//
TUint CtGenericHIDAPI::VendorIdL(TInt /*aConnID*/)
    {
    return 0x0421;
    }

// ---------------------------------------------------------------------------
// ProductIdL
// ---------------------------------------------------------------------------
//
TUint CtGenericHIDAPI::ProductIdL(TInt /*aConnID*/)
    {    
    return 0x0083;
    }

// ---------------------------------------------------------------------------
// GetProtocolL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::GetProtocolL(TInt /*aConnID*/,  TUint16 /*aInterface*/)
    {
    }

// ---------------------------------------------------------------------------
// SetProtocolL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::SetProtocolL(TInt /*aConnID*/, TUint16 /*aValue*/,
        TUint16 /*aInterface*/)
    {    
    }

// ---------------------------------------------------------------------------
// GetReportL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::GetReportL(TInt /*aConnID*/, TUint8 /*aReportType*/,
        TUint8 /*aReportID*/, TUint16 /*aInterface*/, TUint16 /*aLength*/)
    {    
    }

// ---------------------------------------------------------------------------
// SetReportL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::SetReportL(TInt /*aConnID*/, TUint8 /*aReportType*/,TUint8 /*aReportID*/,
    TUint16 /*aInterface*/, const TDesC8& /*aReport*/)
    {    
    }

// ---------------------------------------------------------------------------
// DataOutL()
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::DataOutL(TInt /*aConnID*/, TUint8 /*aReportID*/,
        TUint16 /*aInterface*/, const TDesC8& /*aReport*/)
    {    
    }

// ---------------------------------------------------------------------------
// GetIdleL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::GetIdleL(TInt /*aConnID*/, TUint8 /*aReportID*/,
        TUint16 /*aInterface*/)
    {    
    }

// ---------------------------------------------------------------------------
// SetIdleL
// ---------------------------------------------------------------------------
//
void CtGenericHIDAPI::SetIdleL(TInt /*aConnID*/,  TUint8 /*aDuration*/, TUint8 /*aReportID*/,
        TUint16 /*aInterface*/)
    {    
    }

const CField* CtGenericHIDAPI::CreateConstCFieldL()
	{
    CReportRoot* tmp=iGenHID->ReportDescriptor(0);  
    User::LeaveIfNull(tmp);
    const CCollection* collection = tmp->CollectionByIndex(0);
    if(collection == NULL)
    	User::Leave(KErrGeneral);
    const CField* field=collection->FieldByIndex(0);
	return field;
	}

CField* CtGenericHIDAPI::CreateCFieldL()
	{
	return (CField*)CreateConstCFieldL();
	}
