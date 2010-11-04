/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Managing abstracted "channels" of network side communication
*
*/


#ifndef C_CDUNTRANSPORTER_H
#define C_CDUNTRANSPORTER_H

#include <e32base.h>
#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#else
#include <c32comm.h>
#endif
#include <es_sock.h>
#include "DunNetDataport.h"

typedef TAny* TConnId;  // supported now: RSocket*, RComm*

const TInt KDunStreamTypeMask    = 0x02;  // 10
const TInt KDunOperationTypeMask = 0x01;  // 01

enum TDunOperationType
    {
    EDunOperationTypeUndefined = KErrNotFound,
    EDunOperationTypeRead      = 0x00,  // 00
    EDunOperationTypeWrite     = 0x01   // 01
    };

enum TDunMediaContext
    {
    EDunMediaContextUndefined = KErrNotFound,
    EDunMediaContextNetwork   = 0,
    EDunMediaContextLocal
    };

enum TDunMedia
    {
    EDunMediaUndefined = KErrNotFound,
    EDunMediaNetwork   = 0,
    EDunMediaRComm,
    EDunMediaRSocket
    };

enum TDunDirection
    {
    EDunDirectionUndefined = KErrNotFound,
    EDunReaderUpstream     = 0x00,  // 00
    EDunWriterUpstream     = 0x01,  // 01
    EDunReaderDownstream   = 0x02,  // 10
    EDunWriterDownstream   = 0x03   // 11
    };

enum TDunStreamType
    {
    EDunStreamTypeUndefined  = KErrNotFound,
    EDunStreamTypeUpstream   = 0x00,  // 00
    EDunStreamTypeDownstream = 0x02   // 10
    };

enum TDunState
    {
    EDunStateIdle,
    EDunStateCallListen,       // Transporter state for call state listening
    EDunStateAtCmdHandling,    // State for AT command handling
    EDunStateAtCmdPushing,     // State for AT command reply pushing
    EDunStateAtCmdEchoing,     // State for AT command character echoing (text mode)
    EDunStateUiNoting          // Transporter state for UI note showing
    };

enum TDunReasonType
    {
    EDunReasonTypeSignal,
    EDunReasonTypeRW,
    EDunReasonTypeRunL
    };

class CDunChanMan;
class MDunPluginManager;
class MDunTransporterUtility;
class CDunTransUtils;
class CDunConnWaiter;
class TDunWaiterData;
class CDunUpstream;
class CDunDownstream;
class CDunSignalCopy;
class CDunSignalNotify;
class CDunNoteHandler;

/**
 *  Class used for reporting connection error's reason and signal changes
 *  of network side
 *  Connection error can happen in read/write and RunL error cases
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunConnectionReason )
    {

public:

    /**
     * Reason type of error; either R/W or RunL
     * or signal (network side)
     * Set in all cases (signal, R/W, RunL)
     */
    TDunReasonType iReasonType;

    /**
     * Context of media where error occurred: network or local
     * Set in all error cases (signal, R/W, RunL)
     */
    TDunMediaContext iContext;

    /**
     * Signal type of network side
     * Set if signal case, 0 otherwise
     */
    TUint iSignalType;

    /**
     * Is signal high or low of network side
     * Set if signal case, 0 otherwise
     */
    TBool iSignalHigh;

    /**
     * Direction of data transfer
     * Set if R/W case, EDunDirectionUndefined otherwise
     */
    TDunDirection iDirection;

    /**
     * Error code of failure
     * Set if R/W or RunL case, KErrNone otherwise
     */
    TInt iErrorCode;

    };

/**
 *  Notification interface class to report line status
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunConnMon )
	{

public:

    /**
     * Gets called when line status changes or when any type of error is
     * detected
     *
     * @since S60 3.2
     * @param aConnId Connection ID for callback
     * @param aConnReason Reason for progress change
     * @return None
     */
    virtual void NotifyProgressChangeL(
        TConnId aConnId,
        TDunConnectionReason aConnReason ) = 0;

	};

/**
 *  Notification interface class to report service advertisement status changes
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunServAdvMon )
    {

public:

    /**
     * Gets called when advertisement status changes to start
     *
     * @since S60 5.0
     * @param aCreation ETrue if channel creation
     *                  EFalse if channel free
     * @return None
     */
    virtual void NotifyAdvertisementStart( TBool aCreation ) = 0;

    /**
     * Gets called when advertisement status changes to end
     *
     * @since S60 5.0
     * @return None
     */
    virtual void NotifyAdvertisementEnd() = 0;

    };

/**
 *  Notification interface class for buffer correction
 *  This interface makes possible to change suggested local media buffer size
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunBufferCorrection )
    {

public:

    /**
     * Notifies about request to change local media's buffer size
     *
     * @since S60 3.2
     * @param aLength Suggested buffer length that will be used if no
     *                correction done
     * @return New (corrected) buffer length
     */
    virtual TInt NotifyBufferCorrection( TInt aLength ) = 0;

    };

/**
 *  Class to store data needed for each separate "channel"
 *  One TDunChannelData data contains objects needed for one channel
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunChannelData )
    {

public:

    /**
     * RComm object of network side
     * Not own.
     */
    RComm* iNetwork;

    /**
     * RComm object of local media side
     * If this is set then iSocket is not used
     * Not own.
     */
    RComm* iComm;

    /**
     * RSocket object of local media side
     * If this is set then iComm is not used
     * Not own.
     */
    RSocket* iSocket;

    /**
     * Name for the channel
     */
    HBufC8* iChannelName;

    /**
     * Upstream read/write object
     * Reads data from local media and writes to network
     * Not own.
     */
    CDunUpstream* iUpstreamRW;

    /**
     * Downstream read/write object
     * Reads data from network and writes to local media
     * Not own.
     */
    CDunDownstream* iDownstreamRW;

    /**
     * Upstream buffer (Local -> Network)
     * Not own.
     */
    TUint8* iBufferUpstream;

    /**
     * Downstream buffer (Local <- Network)
     * Not own.
     */
    TUint8* iBufferDownstream;

    /**
     * Pointer to upstream's buffer (Local -> Network)
     * Not own.
     */
    TPtr8* iBufferUpPtr;

    /**
     * Pointer to downstream's buffer (Local <- Network)
     * Not own.
     */
    TPtr8* iBufferDownPtr;

    /**
     * Upstream signal copy
     * If this is set then iSignalNotify is not used
     * Not own.
     */
    CDunSignalCopy* iUpstreamSignalCopy;

    /**
     * Downstream signal copy
     * If this is set then iSignalNotify is not used
     * Not own.
     */
    CDunSignalCopy* iDownstreamSignalCopy;

    /**
     * Signal notifier
     * If this is set then the following are not used:
     * iUpstreamSignalCopy, iDownstreamSignalCopy
     * Not own.
     */
    CDunSignalNotify* iSignalNotify;

    /**
     * Owner's UID
     */
    TUid iOwnerUid;

    /**
     * Flag that indicates whether this channel is allocated or not
     */
    TBool iChannelInUse;

    };

/**
 *  Class to store data needed for each plugin service advertisement
 *  One TDunChannelData data contains objects needed for one plugin
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunServAdvData )
    {

public:

    /**
     * Owner's UID for which to have the monitor
     */
    TUid iOwnerUid;

    /**
     * Service advertisement monitor
     * Not own.
     */
    MDunServAdvMon* iServAdvMon;

    };

/**
 *  Class for managing abstracted "channels" of network side communication
 *  This is main class to be used by other components
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunTransporter ) : public CBase
    {

    friend class CDunTransUtils;
    friend class CDunChanMan;

public:

    /**
     * Two-phased constructor.
     * @param aPluginManager Callback to call when notification via
     *                       MDunPluginManager to be made
     * @param aNumOfMaxChannels Number of wanted maximum channels
     *                          (can be omitted with 0)
     * @return Instance of self
     */
	IMPORT_C static CDunTransporter* NewL( MDunPluginManager* aPluginManager,
	                                       TInt aNumOfMaxChannels=0 );

    /**
    * Destructor.
    */
    virtual ~CDunTransporter();

    /**
     * Number of allocated channels, is the same number as allocated and active
     * (non-waiting) channels
     *
     * @since S60 3.2
     * @return Number of allocated channels
     */
    IMPORT_C TInt NumberOfAllocatedChannels();

    /**
     * Number of waiting channels, is the same number as allocated and inactive
     * (waiting) channels
     *
     * @since S60 3.2
     * @return Number of waiting channels
     */
    IMPORT_C TInt NumberOfWaitingChannels();

    /**
     * Gets the number of allocated channels by owner UID, is the same number
     * as allocated and active (non-waiting) channels
     *
     * @since S60 5.0
     * @param aOwnerUid UID of the channel owner
     * @return Number of allocated channels by UID
     */
    IMPORT_C TInt GetNumberOfAllocatedChannelsByUid( TUid aOwnerUid );

    /**
     * Gets the number of waiting channels by owner UID, is the same number
     * as allocated and inactive (waiting) channels
     *
     * @since S60 5.0
     * @param aOwnerUid UID of the channel owner
     * @return Number of waiting channels by UID
     */
    IMPORT_C TInt GetNumberOfWaitingChannelsByUid( TUid aOwnerUid );

    /**
     * Service advertisement status
     *
     * @since S60 5.0
     * @return ETrue if OK to advertise, EFalse otherwise
     */
    IMPORT_C TBool AdvertisementStatus();

    /**
     * Creates a channel of communication between local media (aComm) and
     * network
     * Local media object pointer also works as a connection ID for the
     * allocated channel
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @param aOwnerUid UID of the channel owner
     * @param aName Name for the channel
     * @param aEnqueuedFail ETrue if enqueued failure, EFalse otherwise
     * @param aCorrection Pointer to object implementing buffer correction
     * @return None
     */
    IMPORT_C void AllocateChannelL(
        RComm* aComm,
        TUid aOwnerUid,
        const TDesC8& aName,
        TBool aEnqueuedFail,
        MDunBufferCorrection* aCorrection=NULL );

    /**
     * Creates a channel of communication between local media (aSocket) and
     * network
     * Local media object pointer also works as a connection ID for the
     * allocated channel
     *
     * @since S60 3.2
     * @param aSocket Pointer to opened local media RSocket ID object
     * @param aOwnerUid UID of the channel owner
     * @param aName Name for the channel
     * @param aEnqueuedFail ETrue if enqueued failure, EFalse otherwise
     * @param aNoFreeChans ETrue if no free channels, EFalse otherwise
     * @return None
     */
    IMPORT_C void AllocateChannelL(
        RSocket* aSocket,
        TUid aOwnerUid,
        const TDesC8& aName,
        TBool aEnqueuedFail,
        TBool& aNoFreeChans );

    /**
     * Frees an allocated channel by local media (aComm) connection ID
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt FreeChannel( RComm* aComm );

    /**
     * Frees an allocated channel by local media (aSocket) connection ID
     *
     * @since S60 3.2
     * @param aSocket Pointer to opened local media RSocket ID object
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt FreeChannel( RSocket* aSocket );

    /**
     * Issues transfer requests for all transfer objects by local media
     * (aComm) connection ID
     * This will cause the Transporter by be ready for transferring data
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return None
     */
    IMPORT_C void IssueTransferRequestsL( RComm* aComm );

    /**
     * Issues transfer requests for all transfer objects by local media
     * (aSocket) connection ID
     * This will cause the Transporter by be ready for transferring data
     *
     * @since S60 3.2
     * @param aSocket Pointer to opened local media RSocket ID object
     * @return None
     */
    IMPORT_C void IssueTransferRequestsL( RSocket* aSocket );

    /**
     * Stops transfers for all transfer objects by local media (aComm)
     * connection ID
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt StopTransfers( RComm* aComm );

    /**
     * Stops transfers for all transfer objects by local media (aSocket)
     * connection ID
     *
     * @since S60 3.2
     * @param aSocket Pointer to opened local media RSocket ID object
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt StopTransfers( RSocket* aSocket );

    /**
     * Adds connection monitor callback for either local media or network side
     * by connection ID
     * Callbacks will be called read/write error is detected during endpoint
     * operation
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @param aCallback Pointer to object whose callbacks will be called
     * @param aDirection Direction of operation to monitor for read/write error
     * @param aSignal ETrue if also signal notification wanted from network side
     * @return None
     */
    IMPORT_C void AddConnMonCallbackL( RComm* aComm,
                                       MDunConnMon* aCallback,
                                       TDunDirection aDirection,
                                       TBool aSignal );

    /**
     * Adds connection monitor callback for either local media or network side
     * by connection ID
     * Callbacks will be called when line status switches to high or low
     *
     * @since S60 3.2
     * @param aSocket Pointer to opened local media RSocket ID object
     * @param aCallback Pointer to object whose callbacks will be called
     * @param aDirection Direction of operation to monitor for read/write error
     * @param aSignal ETrue if also signal notification wanted from network side
     * @return None
     */
    IMPORT_C void AddConnMonCallbackL( RSocket* aSocket,
                                       MDunConnMon* aCallback,
                                       TDunDirection aDirection,
                                       TBool aSignal );

    /**
     * Adds error to consider as no error condition when doing any of the four
     * endpoint's read/writer operation
     *
     * @since S60 3.2
     * @param aError Error code to consider as "no error"
     * @param aComm Pointer to opened local media RComm ID object
     * @param aDirection One of four data transfer endpoints where to add a
     *                   skipped error code
     * @return None
     */
    IMPORT_C void AddSkippedErrorL( TInt aError,
                                    RComm* aComm,
                                    TDunDirection aDirection );

    /**
     * Adds error to consider as no error condition when doing any of the four
     * endpoint's read/writer operation
     *
     * @since S60 3.2
     * @param aError Error code to consider as "no error"
     * @param aSocket Pointer to opened local media RSocket ID object
     * @param aDirection One of four data transfer endpoints where to add a
     *                   skipped error code
     * @return None
     */
    IMPORT_C void AddSkippedErrorL( TInt aError,
                                    RSocket* aSocket,
                                    TDunDirection aDirection );

    /**
     * Sets service advertisement monitor callback by owner UID
     * Callbacks will be called when advertisement status changes.
     * The callbacks are updated with every successfully completed
     * channel allocation/free (and allocation failure) so it is recommended
     * to call this method after AllocateChannelL().
     *
     * @since S60 5.0
     * @param aOwnerUid Owner's UID for which to have the monitor
     * @param aCallback Pointer to object whose callbacks will be called
     * @return None
     */
    IMPORT_C void SetAdvertisementMonitorL( TUid aOwnerUid,
                                            MDunServAdvMon* aCallback );

    /**
     * Frees service advertisement monitor callback by owner UID
     *
     * @since S60 5.0
     * @param aOwnerUid Owner's UID for which to have the monitor
     * @param aCallback Pointer to object whose callbacks will be called
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt FreeAdvertisementMonitor( TUid aOwnerUid,
                                            MDunServAdvMon* aCallback );

private:

    CDunTransporter( MDunPluginManager* aPluginManager,
                     TInt aNumOfMaxChannels );

    void ConstructL();

    /**
     * Initializes the transporter, must be called as the first operation
     *
     * @since S60 3.2
     * @return KErrAlreadyExists = already initialized,
     *         KErrGeneral = network initialization failed,
     *         KErrNone otherwise
     */
    TInt InitializeL();

    /**
     * UnInitializes the transporter, can be called as the last operation
     *
     * @since S60 3.2
     * @return None
     */
    void UnInitialize();

    /**
     * Initialize the transporter
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitializeOnDemand();

    /**
     * UnInitialize the transporter
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt UnInitializeOnDemand();

    /**
     * Returns index of media for connection ID
     *
     * @since S60 3.2
     * @param aConnId Connection ID for media
     * @param aMediaContext Context of media to search for; either local or
     *                      network
     * @return Symbian error code on error, found index otherwise
     */
    TInt GetMediaIndex( TConnId aConnId,
                        TDunMediaContext aMediaContext=EDunMediaContextLocal );

    /**
     * Returns index of media for connection ID
     *
     * @since S60 3.2
     * @param aConnId Connection ID for media
     * @param aMediaContext Context of media to search for; either local or
     *                      network
     * @return Found index
     */
    TInt GetMediaIndexL( TConnId aConnId,
                         TDunMediaContext aMediaContext=EDunMediaContextLocal );

    /**
     * Checks initialization and RSubSessionBase() handle
     *
     * @since S60 3.2
     * @param aConnId Connection ID for media
     * @return KErrNotReady if not initialized, KErrBadHandle if no handle
     */
    TInt CheckInitAndHandle( TConnId aConnId );

    /**
     * Deletes own internal data
     *
     * @since S60 3.2
     * @return None
     */
    void DeleteTransporter();

private:  // data

    /**
     * Pointer to common utility class
     * Own.
     */
    MDunTransporterUtility* iUtility;

    /**
     * Pointer to RComm channel manager class
     * Own.
     */
    CDunChanMan* iChanMan;

    /**
     * Pointer to note handler class
     * Own.
     */
    CDunNoteHandler* iNoteHandler;

    /**
     * Array of abstractions of each "channel"
     * One TDunChannelData entry contains objects needed for one channel
     */
    RArray<TDunChannelData> iChannelData;

    /**
     * Array of service advertisement data for each plugin
     * One TDunServAdvData entry contains objects needed for one plugin
     */
    RArray<TDunServAdvData> iServAdvData;

    /**
     * Callback to call when notification via MDunPluginManager to be made
     * Not own.
     */
    MDunPluginManager* iPluginManager;

    /**
     * Number of channels that are active
     * Used for updating KPSUidDialupConnStatus Pub&Sub key
     */
    TInt iActiveChannels;

    /**
     * Number of wanted maximum channels (can be omitted with 0)
     * Used for initialization of this class
     */
    TInt iNumOfMaxChannels;

    /**
     * Indicates whether this class is initialized or not
     */
    TBool iInitialized;

    /**
     * Indicates whether or not it is OK to advertise services
     */
    TBool iAdvertise;

    /**
     * Instance of network side abstraction
     * Own.
     */
    CDunNetDataport* iNetwork;

    };

#endif  // C_CDUNTRANSPORTER_H
