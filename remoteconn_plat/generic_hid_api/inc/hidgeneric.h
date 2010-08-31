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
* Description:  Declares main application class.
*
*/


#ifndef C_GENERICHID_H
#define C_GENERICHID_H

#include <hidinterfaces.h>

class CDriverListItem;
class TElement;
class CParser;
class CReportRoot;
class CConnectionInfo;

/**
 *
 *  Generic HID main class
 *  Generic HID layer, allowing the HID Transport layers and device drivers to
 *  pass data to the Generic HID
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class CGenericHid : public CHidTransport, public MDriverAccess
    {
public:
    virtual ~CGenericHid();

    /**
     * Constructs a generic HID layer with a reference to the owner so commands can
     * be sent to connected devices
     *
     * @since S60 v5.0
     * @param aTransportLayer The owning transport layer
     * @return a Pointer to an instantiated Generic HID layer
     */
    IMPORT_C static CGenericHid* NewL(MTransportLayer* aTransportLayer);

    /**
     * Constructs a generic HID layer with a reference to the owner so commands can
     * be sent to connected devices
     *
     * @since S60 v5.0
     * @param aTransportLayer The owning transport layer
     * @return a Pointer to an instantiated Generic HID layer
     */
    IMPORT_C static CGenericHid* NewLC(MTransportLayer* aTransportLayer);

    /**
     * Fromm class MDriverAccess
     * Retrieves the country code for the HID device
     *
     * @since S60 v5.0
     * @param aConnectionID The device identifier
     * @return  a country code
     */
    TUint CountryCodeL(TInt aConnectionID);

    /**
     * Fromm class MDriverAccess
     * Retrieves the country code for the HID device
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @return a vendor ir
     */
    TUint VendorIdL(TInt aConnectionId);

    /**
     * Fromm class MDriverAccess
     * Retrieves the product identifier for the HID device
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @return  a prodcut id
     */
    TUint ProductIdL(TInt aConnectionId);


    /**
     * Fromm class MDriverAccess
     * Sets the protocol to be used for reports. 
     * Leaves KErrInUse The request was not successful because the
     * transport layer is busy with a previous request,
     * KErrNotReady The request failed because the device
     * is currently unavailable and KErrNotFound The request
     * was unsuccessful
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @param aInterface The device interface being used by the driver
     * @param aProtocol The requested report protocol (boot or report)
     * @parem aDriver Calling driver
     * @return  None.
     */
    void SetProtocolL(TInt aConnectionId, TUint16 aInterface,
        MDriverAccess::TProtocols aProtocol, CHidDriver* aDriver);


    /**
     * Fromm class MDriverAccess
     * Requests the current protocol from the HID device.  This is an asynchronous
     * request. The protocol value will come through at a later time
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @param aInterface The hid interface
     * @return None. 
     */
    void GetProtocolL(TInt aConnectionId,  TUint16 aInterface);

    /**
     * Fromm class MDriverAccess
     * A request for a report from the device.  This is an asynchronous request.
     * The report will come through at a later time
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @param aReportId report id to be get
     * @param aInterface The device interface being used by the driver
     * @param aLength report lenght
     * @return  None.
     */
    void GetReportL(TInt aConnectionId, TUint8 aReportId,
        TUint16 aInterface, TUint16 aLength);

    /**
     * Fromm class MDriverAccess
     * A request to send a report payload to the HID device
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aReportId The report id
     * @param aReportType Type of report (input/output/feature)
     * @param aPayload The report containing the device setup packet
     * @param aInterface The device interface being used by the driver
     * @parem aDriver Calling driver
     * @return None.
     */
    void SetReportL(TInt aConnectionId,
        TUint8 aReportId, MDriverAccess::TReportType aReportType,
        const TDesC8& aPayload, TUint16 aInterface, CHidDriver* aDriver);

    /**
     * Fromm class MDriverAccess
     * A request to send a report payload to the HID device in Interrupt Channel
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aReportId report id which payload to be send
     * @param aPayload The report containing the device setup packet
     * @param aInterface The device interface being used by the driver
     * @return  None.
     */
    void DataOutL(TInt aConnectionId,  TUint8 aReportId,
                              const TDesC8& aPayload, TUint16 aInterface);

    /**
     * Fromm class MDriverAccess
     * Request for the current idle setting for the device
     * This is an asynchronous request.  The idle value will come through at a
     * later time. Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device identifier
     * @param aReportId The report ID for which we want the idle rate
     * @param aInterface The device interface being used by the driver
     * @return None.
     */
    void GetIdleL(TInt aConnectionId, TUint8 aReportId, TUint16 aInterface);

    /**
     * Fromm class MDriverAccess
     * Request to the HID device to set the specified idle rate
     * A request to send a report payload to the HID device
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aDuration The number of 0.04ms intervals to wait between reports
     * @param aReportId The report for which the idle rate is being set
     * @param aInterface The device interface being used by the driver
     * @parem aDriver Calling driver
     * @return None.
     */
    void SetIdleL(TInt aConnectionId, TUint8 aDuration,
        TUint8 aReportId, TUint16 aInterface, CHidDriver* aDriver);

    /**
     * From class CHidTransport
     * Called by the transport layers to inform the generic HID of the success of
     * the last Set... command.
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aCmdAck Status of the last Set... command
     * @return None.
     */
    void CommandResult(TInt aConnectionId, TInt aCmdAck);

    /**
     * From class MDriverAccess
     * Gives the device driver access to the results of the report
     * descriptor parsing
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @return A pointer to the parsed descriptor object.
     */
    CReportRoot* ReportDescriptor(TInt aConnectionId);

    /**
     * From class CHidTransport
     * Called by a transport layer when a device has connected and the report
     * descriptors have been obtained
     *
     * @since S60 v5.0
     * @param aConnectionId The tansport-layer enumeration for the connection
     * @param aDescriptor The report descriptor for the connected HID device
     * @return KErrNone if a driver was found, otherwise an error code
     */
    TInt ConnectedL(TInt aConnectionId, const TDesC8& aDescriptor);

    /**
     * From class CHidTransport
     * Called by a transport layer when a device has been disconnected
     *
     * @since S60 v5.0
     * @param aConnectionId The tansport-layer enumeration for the connection
     * @return KErrNone if a driver was found, otherwise an error code
     */
    TInt Disconnected(TInt aConnectionId);

    /**
     * From class CHidTransport
     * Called by the transport layer when a device has sent a report on the
     * interrupt or control channel
     *
     * @since S60 v5.0
     * @param aConnectionId The tansport-layer enumeration for the connection
     * @param aChannel Channel ID (Interrupt or Control)
     * @param aPayload The complete report payload
     * @return KErrNone if the data was handled by the driver, otherwise an error code
     */
    virtual TInt DataIn(TInt aConnectionId,
        CHidTransport::THidChannelType aChannel, const TDesC8& aPayload);

    /**
     * From class CHidTransport
     * Called by the transport layer to suspend or resume a driver
     *
     * @since S60 v5.0
     * @param aConnectionId The tansport-layer enumeration for the connection
     * @param aActive EActive to start the driver(s), ESuspend to suspend
     * @return KErrNone if the driver was successfully activated, otherwise an error code
     */
    TInt DriverActive(TInt aConnectionId,
        CHidTransport::TDriverState aActive);

protected:
    CGenericHid(MTransportLayer* aTransportLayer);
    void ConstructL();

private:

    /**
     * Remove drivers
     *
     * @since S60 v5.0
     * @return None
     */
    void RemoveDrivers();
              
    /**
     * Attempts to find a driver that is handling reports from the HID device
     *
     * @since S60 v5.0
     * @return Connection information
     */
    CConnectionInfo* SeekConnectionInfo(TInt aConnectionId);

private:

    /**
     * Instantiated driver list
     */
    TSglQue<CDriverListItem> iDriverList;

    /**
     * Parser
     * Own.
     */
    CParser* iParser;

    /**
     * Parser
     * Not own.
     */
    MTransportLayer* iTransportLayer;
    
    /**
     * Connection information
     */
    RPointerArray<CConnectionInfo> iConnectionInfo;
    
    /**
     * Input data handling registry 
     * Own.
     */
    CHidInputDataHandlingReg* iInputHandlingReg;
    
    };

#endif
