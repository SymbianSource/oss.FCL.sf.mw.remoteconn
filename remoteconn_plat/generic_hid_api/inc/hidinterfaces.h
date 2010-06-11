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



#ifndef C_HIDDRIVER_H
#define C_HIDDRIVER_H

#include <ecom/ecom.h>


class CReportRoot;
class MDriverAccess;
class MTransportLayer;
class CHidDriver;
// ----------------------------------------------------------------------

// Return codes from the Generic HID to Transport layers
//
const TInt KErrHidBase = -8000;

const TInt KErrHidNoDriver = KErrHidBase;           /*!< No driver could be found to handle the request */
const TInt KErrHidSuspended = KErrHidBase - 1;      /*!< Command not handled - driver is suspended */
const TInt KErrHidUnrecognised = KErrHidBase - 2;   /*!< Driver could not understand the data */
const TInt KErrHidUnexpected = KErrHidBase - 3;     /*!< Unsolicited data from the device */
const TInt KErrHidBadChannel = KErrHidBase - 4;     /*!< Channel was not Int or Ctrl type */
const TInt KErrHidPartialSupported = KErrHidBase - 5;     /*!< Partil supported hid device */

// Errors returned by the report translator / generator:
//
const TInt KErrUsageNotFound = KErrHidBase - 10;    //!< Usage page/ID wasn't found in the field
const TInt KErrValueOutOfRange = KErrHidBase - 11;  //!< Value specified is outside the logical range
const TInt KErrNoSpaceInArray = KErrHidBase - 12;   //!< Array field already contains the maximum number of values
const TInt KErrBadControlIndex = KErrHidBase - 13;  //!< Control index exceeds the number of controls in the field

// ACK codes returned by Set/Get requests to the devices
//
const TInt KErrCommandAckBase = -8100;

const TInt KErrAckInvalidReportID = KErrCommandAckBase;         /*!< Invalid report ID */
const TInt KErrAckInvalidParameter = KErrCommandAckBase - 1;    /*!< Invalid or out of range param */
const TInt KErrAckUnknown = KErrCommandAckBase - 2;             /*!< Command failed, but the device can't determine why */
const TInt KErrAckFatal = KErrCommandAckBase - 3;               /*!< The device is in an unrecoverable state and must be restarted */

// Constant for plugin interface:
const TUid KHidDriverPluginInterfaceUid        = { 0x10201d26 };


class THidEvent
    {
    public:
    /**
     * Usage page
     */
    TInt iUsagePage;
    
    /**
     * Keycode
     */
    TInt iKeyCode;
    };

class CHidInputDataHandlingReg: public CBase
    {
       
public:
    /**
     * Two-phased constructor.     
     */
    static CHidInputDataHandlingReg* NewL();

    /**
     * Destructor
     */
     virtual ~CHidInputDataHandlingReg(); 

public:        
        
    /**
     * Check if key event is not allready processed
     *
     * @since S60 v5.0
     * @param aUsagePage a usage page to be checked
     * @param aUsage a usage code to be checked
     * @return true if event is not allready handled
     */
    IMPORT_C TBool  AllowedToHandleEvent(TInt aUsagePage, TInt aUsage);
    
    
    /**
     * Add handled event
     *
     * @since S60 v5.0
     * @param aUsagePage usage page to be handled
     * @param aUsage Usage to be handled
     * @return None.
     */ 
    IMPORT_C void  AddHandledEvent( TInt aUsagePage, TInt aUsage);
    
    /**
     * Reset Array 
     */
    void  Reset();
    
private:
    
    CHidInputDataHandlingReg();
    void ConstructL();

private: 
    
    /**
     * Event array
     * Own.
     */
    CArrayFixFlat<THidEvent>* iEventArray;
    
    };

/**
 *
 *  Transport layer interface class
 *  Interface allowing the generic HID layer to pass data to a Transport layer
 *  All commands are asynchronous.  The results are returned via the
 *
 *  @since S60 v5.0
 */
class MTransportLayer
    {
public:

    /**
     * Request for the country code of the given device.
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return country code.
     */
    virtual TUint CountryCodeL(TInt aConnID) = 0;

    /**
     * Request for the vendor identifier for the given device.
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return vendor id.
     */
    virtual TUint VendorIdL(TInt aConnID) = 0;

    /**
     * Request for the product identifier for the given device.
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return product id.
     */
    virtual TUint ProductIdL(TInt aConnID) = 0;

    /**
     * Request for the current device report protocol. The protocol will be
     * received as a control report via the CHidTransport::DataIn function
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aInterface The interface we want to get the protocol from
     * @return None.
     */
    virtual void GetProtocolL(TInt aConnID,  TUint16 aInterface) = 0;

    /**
     * Request to put the device in the specified protocol
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aValue The requested protocol for the device (boot or report)
     * @param aInterface The interface we want to set the protocol for
     * @return None.
     */
    virtual void SetProtocolL(TInt aConnID, TUint16 aValue,
        TUint16 aInterface) = 0;

    /**
     * Request for a device report. The data will be received as a control report
     * via the CHidTransport::DataIn function
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aReportType The type of report (input/output/feature) requested
     * @param aReportID The specific report required
     * @param aInterface The interface we want the report from
     * @param aLength The expected length of the report buffer
     * @return None.
     */
    virtual void GetReportL(TInt aConnID, TUint8 aReportType,TUint8 aReportID,
        TUint16 aInterface, TUint16 aLength) = 0;

    /**
     * Request to send a report to a device. The response will be reported via the
     * CHidTransport::CommandResult function
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aReportType The type of report (input/output/feature) requested
     * @param aReportID The specific report required to set
     * @param aInterface The interface we want to send the report to
     * @param aReport The report payload to be sent to the device
     * @return None.
     */
    virtual void SetReportL(TInt aConnID, TUint8 aReportType,TUint8 aReportID,
        TUint16 aInterface, const TDesC8& aReport) = 0;

    /**
     * Request to send data to a device. There are no responses to this report from device.
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aReportID The specific report required
     * @param aInterface The interface we want to send the report to
     * @param aReport The report payload to be sent to the device
     * @return None.
     */
    virtual void DataOutL(TInt aConnID, TUint8 aReportID,
        TUint16 aInterface, const TDesC8& aReport) = 0;

    /**
     * Request for the current idle rate of a report from the device. The response
     * will be recevied via the CHidTransport::DataIn function
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aReportID The specific report to be queried
     * @param aInterface The interface we want to query for idle rate
     * @return None.
     */
    virtual void GetIdleL(TInt aConnID, TUint8 aReportID,
        TUint16 aInterface) = 0;

    /**
     * Request to set the current idle rate for a report on the device.
     * The response will be received via the CHidTransport::CommandResult function
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @param aReportID The specific report to be queried
     * @param aDuration The time period between idle reports (4ms per bit. An
     *        interval of 0 disables idle reports so that Interrupt reports are only ever
     *        received when the reported data has changed
     * @param aReportID The specific report to be queried
     * @param aInterface The interface we want to query for idle rate
     * @return None.
     */
    virtual void SetIdleL(TInt aConnID,  TUint8 aDuration, TUint8 aReportID,
        TUint16 aInterface) = 0;
    };

/**
 *
 *  Driver access interface
 *  Interface allowing drivers to request data from and send data to devices via
 *  the Generic HID layer.
 *
 *  @since S60 v5.0
 */
class MDriverAccess
    {
public:

    /**
     * The type of report requested from the device
     * Note: The enumeration values map directly to the HID equivalent values
     * (Passed as TUint16 values to the transport layer)
     */
    enum TReportType
        {
        EInput=1,     /*!< Input report */
        EOutput,      /*!< Output report */
        EFeature      /*!< Feature report */
        };

    /** Return codes from the Generic HID to the device drivers
     * Note: The enumeration values map directly to the HID equivalent values
     *(Passed as TUint8 values to the transport layer)
     */

    enum TProtocols
        {
        EBoot=0,        /*!< Boot Protocol */
        EReport=1       /*!< Report Protocol */
        };

public:

    /**
     * Retrieves the country code for the HID device
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return contry code
     */
    virtual TUint CountryCodeL(TInt aConnID) = 0;

    /**
     * Retrieves the  for the HID device
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return vendor id
     */
    virtual TUint VendorIdL(TInt aConnID) = 0;

    /**
     * Retrieves the product identifier for the HID device
     *
     * @since S60 v5.0
     * @param aConnID The device identifier
     * @return produrct id
     */
    virtual TUint ProductIdL(TInt aConnID) = 0;

    /**
     * Requests the current protocol for the device (boot or report)
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavaila
     *
     * @since S60 v5.0
     * @param aConnectionID The connection id
     * @param aInterface The current interface being used by the driver
     * @return None.
     */
    virtual void GetProtocolL(TInt aConnectionID,
        TUint16 aInterface) = 0;

    /**
     * Requests a report from the device
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The connection id
     * @param aReportId The report required from the device
     * @param aInterface The current interface being used by the driver
     * @param aLength ** NOT USED **
     * @return None
     */
    virtual void GetReportL(TInt aConnectionId, TUint8 aReportId,
        TUint16 aInterface, TUint16 aLength) = 0;

    /**
     * Requests the current Idle setting for the device
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aReportId The report required from the device
     * @param aInterface The current interface being used by the driver
     * @return None.
     */
    virtual void GetIdleL(TInt aConnectionId, TUint8 aReportId,
        TUint16 aInterface ) = 0;

    /**
     * Sets the protocol to be used for reports
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aInterface The current interface being used by the driver
     * @param aProtocol The required protocol (boot or report)
     * @parem aDriver Calling driver
     * @return None.
     */
    virtual void SetProtocolL(TInt aConnectionId,
        TUint16 aInterface, TProtocols aProtocol, CHidDriver* aDriver) = 0;

    /**
     * Sets the idle interval for interrupt data.
     * Leaves whit KErrInUse The request was not successful because the transport 
     * layer is busy with previous request and KErrNotReady The request failed 
     * because the device is currently unavailable
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aDuration The idle interval, in steps of 0.04ms intervals (where
     *                  1 = 0.04ms, 2=0.08ms). 0 will disable idle so reports
     *                   are only sent when the state of the device changes
     * @param aInterface The current interface being used by the driver
     * @param aReportId The report whose idle rate is being set
     * @param aInterface The current interface being used by the driver
     * @parem aDriver Calling driver
     * @return None.
     */
    virtual void SetIdleL(TInt aConnectionId, TUint8 aDuration,
        TUint8 aReportId, TUint16 aInterface, CHidDriver* aDriver) = 0;


    /**
     * Sends a report to the device
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aReportId The report to receive the setup report
     * @param aReportType The type of report being sent to the device (input, output
     *                     or Feature)
     * @param aPayload The report being sent to the device
     * @parem aDriver Calling driver
     * @param aInterface The current interface being used by the driver
     * @return KErrNone The request was successful and the result of the command is
     *         expected at a later time (as a CmdAck message), KErrInUse The request
     *         was not successful because the transport layer is busy with a previous
     *         request, KErrNotReady The request failed because the device is currently
     *         unavailable and KErrNoMemory The request failed because not enough memory
     *         available
     */
    virtual void SetReportL(TInt aConnectionId, TUint8 aReportId,
        TReportType aReportType, const TDesC8& aPayload,
        TUint16 aInterface, CHidDriver* aDriver) = 0;

    /**
     * Sends a report to the device (from host) using Interrupt Channel as DATA
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @param aReportId The report to receive the setup report
     * @param aPayload The report being sent to the device
     * @param aInterface The current interface being used by the driver
     * @return KErrNone The request was successful and the result of the
     *         command is expected at a later time (as a CmdAck message),
     *         KErrInUse The request was not successful because the transport layer is
     *         busy with a previous request, KErrNotReady The request failed because the
     *         device is currently unavailable and KErrNoMemory The request failed because
     *         not enough memory available
     */
    virtual void DataOutL(TInt aConnectionId, TUint8 aReportId,
        /*TReportType aReportType,*/ const TDesC8& aPayload,
        TUint16 aInterface) = 0;

    /**
     * A request for the parsed descriptor container object so the driver can
     * retrieve the report format(s)
     *
     * @since S60 v5.0
     * @param aConnectionId The device ID
     * @return A pointer to the parsed report descriptor container and
     *        NULL if no parsed report descriptor container exists
     */
    virtual CReportRoot* ReportDescriptor(TInt aConnectionId) = 0;
    };


/**
 *
 *  Hid transport interface
 *  Interface allowing the transport layer to pass data to the Generic HID
 *
 *  @since S60 v5.0
 */
class CHidTransport : public CBase
    {
public:
    /**
     * Defines the channel type for the DataIn function
     */
    enum THidChannelType
        {
        EHidChannelInt,     /*!< Interrupt channel */
        EHidChannelCtrl     /*!< Control channel */
        };

    /**
     *Driver state (active or suspended)
     */
    enum TDriverState
        {
        EActive,            /*!< Driver will handle interrupt data */
        ESuspend            /*!< the driver will not handle interrupt data */
        };

public:

    /**
     * Called when a device has connected
     *
     * @since S60 v5.0
     * @param aConnectionId The transport-layer enumeration for the connection
     * @param aDescriptor The report descriptor for the device
     * @return KErrNone The connection will be handled by one or more drivers,
     *         KErrHidNoDriver No driver was found to handle the connected device
     */
    virtual TInt ConnectedL(TInt aConnectionId, const TDesC8& aDescriptor) = 0;

    /**
     * Called when a device has been disconnected
     *
     * @since S60 v5.0
     * @param aConnectionId The transport-layer enumeration for the connection
     * @param aDescriptor The report descriptor for the device
     * @return KErrNone The device was recognised and its drivers were unloaded and
     *         KErrHidNoDriver No driver was found to handle the connected device
     */
    virtual TInt Disconnected(TInt aConnectionId) = 0;

    /**
     * Called when a device has been disconnected
     *
     * @since S60 v5.0
     * @param aConnectionId Transport layer connection enumeration
     * @param aChannel Channel ID (Interrupt or Control)
     * @param aPayload The complete report payload
     * @return KErrNone The payload was handled by one or more drivers,
     *         KErrHidNoDriver No driver is handling reports from the device
     *         and KErrHidSuspended The report was not handled because all the drivers
     *         handling the device are suspended
     */
    virtual TInt DataIn(TInt aConnectionId,
        THidChannelType aChannel, const TDesC8& aPayload) = 0;

    /**
     * Suspends or Resumes the driver(s) handling the connection
     *
     * @since S60 v5.0
     * @param aConnectionId Transport layer connection enumeration
     * @param aActive The requested state of the driver (active or suspended)
     * @return KErrNone The driver was successfully put into the requested state,
     *         and KErrHidAlreadyInState All the drivers were already in the
     *         requested state
     */
    virtual TInt DriverActive(TInt aConnectionId,
        CHidTransport::TDriverState aActive) = 0;

    /**
     * Called by the transport layers to inform the generic HID of the
     * success of the last Set... command.
     *
     * @since S60 v5.0
     * @param aConnectionId Transport layer connection enumeration
     * @param  aCmdAck Status of the last Set... command
     * @return None.
     */
    virtual void CommandResult(TInt aConnectionId, TInt aCmdAck) = 0;
    };

/**
 *
 *  Driver plugin interface
 *  Interface allowing drivers to request data from and send data to devices via
 *  the Generic HID layer.
 *
 *  @lib generichid.lib 
 *  @since S60 v5.0
 */
class CHidDriver : public CBase
    {
public:


    /**
     * Two-phased constructor.
     * @param aImplementationUid Implementation UID of the plugin to be
     *                           created.
     * @param aHid               Driver acces interface
     */
    IMPORT_C static CHidDriver* NewL(
	    const TUid aImplementationUid,
    	MDriverAccess* aHid );

    /**
     * Destructor
     */
     IMPORT_C virtual ~CHidDriver();

    /**
     * Called by the Generic HID to see if the factory can use reports described by
     * the parsed report descriptor
     * NOTE: The current implementation supports just one driver and that driver
     * that will either handle all the reports from the device or none at all.
     * Report ID is not currently being taken into account.
     *
     * @since S60 v5.0
     * @param aReportDescriptor Parsed HID report descriptor
     * @return KErrNone The driver will handle reports from the report descriptor and
     *         KErrHidUnrecognised The driver cannot handle reports from the device
     */
    virtual TInt CanHandleReportL(CReportRoot* aReportDescriptor) = 0;

    /**
     * Called by the Generic HID layer when a device has sent a report on the
     * interrupt or control channel
     *
     * @since S60 v5.0
     * @param aChannel Channel ID (Interrupt or Control)
     * @param aPayload The complete report payload
     * @return Error if data can't be handled.
     */
    virtual TInt DataIn(CHidTransport::THidChannelType aChannel,
        const TDesC8& aPayload) = 0;

    /**
     * Called by the Generic HID layer when the handled device has been disconnected
     * interrupt or control channel
     *
     * @since S60 v5.0
     * @param aReason Disconnection code
     * @return None.
     */
    virtual void Disconnected(TInt aReason) = 0;

    /**
     * Called after a driver is sucessfully created by the Generic HID,
     * when a device is connected.
     *
     * @since S60 v5.0
     * @param aConnectionId An number used to identify the device in
     * subsequent calls from the driver to the generic HID, for example
     * when sending data to the device.
     * @return None.
     */
    virtual void InitialiseL(TInt aConnectionId) = 0;

    /**
     * Enables the driver so that it will configure the device and
     * start handling interrupt reports from the device
     *
     * @since S60 v5.0
     * @param aConnectionId A number used to identify the device in
     * subsequent calls from the driver to the Generic HID, for example
     * when sending data to the device.
     * @return None.
     */
    virtual void StartL(TInt aConnectionId) = 0;

    /**
     * Disables the driver so that it will stop handling device
     * interrupt reports
     *
     * @since S60 v5.0
     * @return None.
     */
    virtual void Stop() = 0;

    /**
     * Called by the transport layers to inform the generic HID of
     * the success of the last Set... command.
     *
     * @since S60 v5.0
     * @param  aCmdAck Status of the last Set... command
     * @return None.
     */
    virtual void CommandResult(TInt aCmdAck) = 0;
    
    /**
     * Return count of supported fields
     *
     * @since S60 v5.0     
     * @return Number of supported fields.
     */
    virtual TInt SupportedFieldCount()= 0;
    
    
    /**
     * Set input handling registy 
     *
     * @since S60 v5.0     
     * @param aHandlingReg  a Input handling registry
     * @return Number of supported fields.
     */
    virtual void SetInputHandlingReg(CHidInputDataHandlingReg* aHandlingReg) = 0;

    protected:
    /**
     * C++ constructor.
     */
    IMPORT_C CHidDriver();

    private: // Data

    /**
     * ECOM plugin instance UID.
     */
    TUid iDtor_ID_Key;
    };
    
#endif


