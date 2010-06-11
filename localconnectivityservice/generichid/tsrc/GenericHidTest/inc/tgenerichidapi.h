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
* Description:  HID generic api test class
*
*/

#ifndef __TESTGEHERICHIDAPI_H
#define __TESTGEHERICHIDAPI_H


#include <e32base.h>
#include <hidinterfaces.h>
#include <hidgeneric.h>
#include "hidreportgenerator.h"
#include "hidtranslate.h"

#include "CHidDriverPlugin.h"
#include "hiduids.h"


/**
 *  HID generic api test class 
 *
 *  @lib 
 *  @since S60 v.3.2
 */
NONSHARABLE_CLASS(CtGenericHIDAPI): public CBase, public MTransportLayer
    { 
    public:
        static CtGenericHIDAPI* NewL();
        static CtGenericHIDAPI* NewLC();

    /**
     * Destructor.
     */
    virtual ~CtGenericHIDAPI();

    public:  

    /**
     * Connect
     *
     * @since S60 v.3.2
     * @param aConnectionId connection id 
     * @param aDescriptor report descriptor
     * @return None
     */
    void ConnectL(TInt aConnectionId, const TDesC8& aDescriptor);

    /**
     * Disconnect
     *
     * @since S60 v.3.2
     * @param aConnectionId a connection id 
     * @return None
     */
    void DisconnectL(TInt aConnectionId);
  
    /**
     * DataIn
     *
     * @since S60 v.3.2
     * @param aConnectionId connection id 
     * @param aDescriptor report descriptor
     * @return error code
     */
    TInt DataInL(TInt aConnectionId, const TDesC8& aDescriptor);
    
    TInt RunCountryCodeL();
        
    TInt RunVendorIdL();
        
    TInt RunProductIdL( );
        
    TInt RunSetProtocolL();
        
    TInt RunGetProtocoL( );
        
    TInt RunGetReportL( );
        
    TInt RunSetReportL(const TDesC8& aDescriptor );
                
    TInt RunDataOutL( const TDesC8& aDescriptor );
        
    TInt GetGetIdleL( );
        
    TInt RunSetIdleL( );
        
    TInt GetCommandResultL( );
        
    TInt GetReportDescriptorL();
    
    TInt CreateReportGeneratorL();
    
    TInt DeleteReportGeneratorL();
    
    TInt ReportGeneratorReport();
    
    TInt ReportGeneratorSetFieldL(  TBool aIsArray, TBool aOutOfRange, TBool aFieldNull, TBool aFieldCountZero, TBool aExistingUsageId, TBool aBadIndex  );
    
    TInt CreateReportTranslatorL( const TDesC8& aDescriptor, TBool aIsArray, TBool aFieldNull, TBool aFieldCountZero);
    
    //TInt CreateReportTranslator_FieldNullL( const TDesC8& aDescriptor  );
            
    TInt DeleteReportTranslatorL( );
    
    TInt ReportTranslatorGetValueL( );
            
    TInt ReportTranslatorValueL(  );
            
    TInt ReportTranslatorGetUsageIdL( TBool aBadControlIndex );
            
    TInt ReportTranslatorUsageIdL( );
            
    TInt ReportTranslatorRawValueL( );
            
    TInt ReportTranslatorCountL( );
    
   TInt CollectiontypeL( );
   TInt CollectionIsPhysicalL( );
   TInt CollectionIsLogicalL( );
   TInt CollectionIsReportL( );
   TInt CollectionIsNamedArrayL( );
   TInt CollectionIsUsageSwitchL( );
   TInt CollectionIsUsageModifierL( );
   
   TInt CollectionIsApplicationL( );
   TInt CollectionUsagePageL( );
   TInt CollectionUsageL( );
   TInt CollectionCollectionCountL( );
   TInt CollectionFieldCountL( );
   TInt CollectionCollectionByIndexL( TBool aPresent );
   TInt CollectionFieldByIndexL( );
   
   TInt ReportrootReportSizeBytes( );
   

   TInt FieldLogicalMaxL( );
   TInt FieldPhysicalMinL( );
   TInt FieldPhysicalMaxL( );
   TInt FieldUnitL( );
   TInt FieldUnitExponentL( );
   TInt FieldCountL( );
   TInt FieldSizeL( );
   TInt FieldOffsetL( TBool setReport );
   TInt FieldReportIdL( );
   TInt FieldIsInReportL( );
   TInt FieldUsagePageL( );
   TInt FieldDesignatorIndexL( );
   TInt FieldStringIndexL( );
   TInt FieldHasUsageL( );
   TInt FieldUsageArrayL( );
   TInt FieldUsageL( );
   TInt FieldUsageCountL( );
   TInt FieldClearUsageListL( );
   TInt FieldLastUsageL( TBool empty );
   TInt FieldAttributesL( );
   TInt FieldTypeL( );
   TInt FieldIsArrayL(  );
   TInt FieldIsVariableL( );
   TInt FieldIsConstantL(  );
   TInt FieldIsDataL(  );
   TInt FieldSetLogicalMinL();
   TInt FieldSetLogicalMaxL( );
   TInt FieldSetPhysicalMinL( );
   TInt FieldSetPhysicalMaxL( );
   TInt FieldSetDesignatorMinL( );
   TInt FieldSetDesignatorMaxL(  );
   TInt FieldSetUsageMinL( );
   TInt FieldSetUsageMaxL( );
   TInt FieldSetStringMinL( );
   TInt FieldSetStringMaxL( );
   TInt FieldSetLogicalRangeL( );
   TInt FieldSetUsageRangeL( );
   TInt FieldSetPhysicalRangeL( );
   TInt FieldSetStringRangeL( );
   TInt FieldSetDesignatorRangeL( );
   TInt FieldIsInputL( );
   TInt FieldIsOutputL( );
   TInt FieldIsFeatureL( );

    
  
private: //from MTransportLayer

    /**
     * Request for the country code of the given device.
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @return country code.
     */
    TUint CountryCodeL(TInt aConnID);

    /**
     * Request for the vendor identifier for the given device.
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @return vendor id.
     */
    TUint VendorIdL(TInt aConnID);

    /**
     * Request for the product identifier for the given device.
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @return product id.
     */
    TUint ProductIdL(TInt aConnID);

    /**
     * Request for the current device report protocol. The protocol will be
     * received as a control report via the CHidTransport::DataIn function
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aInterface The interface we want to get the protocol from
     * @return None.
     */
    void GetProtocolL(TInt aConnID,  TUint16 aInterface);

    /**
     * Request to put the device in the specified protocol
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aValue The requested protocol for the device (boot or report)
     * @param aInterface The interface we want to set the protocol for
     * @return None.
     */
    void SetProtocolL(TInt aConnID, TUint16 aValue,
            TUint16 aInterface);

    /**
     * Request for a device report. The data will be received as a control report
     * via the CHidTransport::DataIn function
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aReportType The type of report (input/output/feature) requested
     * @param aReportID The specific report required
     * @param aInterface The interface we want the report from
     * @param aLength The expected length of the report buffer
     * @return None.
     */
    void GetReportL(TInt aConnID, TUint8 aReportType,TUint8 aReportID,
            TUint16 aInterface, TUint16 aLength);

    /**
     * Request to send a report to a device. The response will be reported via the
     * CHidTransport::CommandResult function
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aReportType The type of report (input/output/feature) requested
     * @param aReportID The specific report required to set
     * @param aInterface The interface we want to send the report to
     * @param aReport The report payload to be sent to the device
     * @return None.
     */
    void SetReportL(TInt aConnID, TUint8 aReportType,TUint8 aReportID,
        TUint16 aInterface, const TDesC8& aReport);

    /**
     * Request to send data to a device. There are no responses to this report from device.
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param @param aReportID The specific report required
     * @param aInterface The interface we want to send the report to
     * @param aReport The report payload to be sent to the device   
     * @return None.
     */
    void DataOutL(TInt aConnID, TUint8 aReportID,
            TUint16 aInterface, const TDesC8& aReport);

    /**
     * Request for the current idle rate of a report from the device. The response
     * will be recevied via the CHidTransport::DataIn function
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aReportID The specific report to be queried
     * @param aInterface The interface we want to query for idle rate
     * @return None.
     */
    void GetIdleL(TInt aConnID, TUint8 aReportID,
            TUint16 aInterface);

    /**
     * Request to set the current idle rate for a report on the device.
     * The response will be received via the CHidTransport::CommandResult function
     *
     * @since S60 v3.2
     * @param aConnID The device identifier
     * @param aReportID The specific report to be queried
     * @param aDuration The time period between idle reports (4ms per bit. An
     *        interval of 0 disables idle reports so that Interrupt reports are only ever
     *        received when the reported data has changed
     * @param aReportID The specific report to be queried
     * @param aInterface The interface we want to query for idle rate
     * @return None.
     */
    void SetIdleL(TInt aConnID,  TUint8 aDuration, TUint8 aReportID,
            TUint16 aInterface);
  

private:
    CtGenericHIDAPI();    
    void ConstructL();    
    
    const CField* CreateConstCFieldL();
    CField* CreateCFieldL();
    
private:
    CGenericHid*        iGenHID;   
    MDriverAccess*      iDriverAcces;
    CReportGenerator*   iReportGenerator;
    TReportTranslator*  iTranslator;
    
    CHidDriver* iHidDriver;
    
    RPointerArray<CField> iFieldList;
  };
#endif
 

