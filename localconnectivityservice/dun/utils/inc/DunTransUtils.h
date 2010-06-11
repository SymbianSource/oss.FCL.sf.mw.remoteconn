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
* Description:  Utility class for other CDunTransporter classes
*
*/


#ifndef C_CDUNTRANSUTILS_H
#define C_CDUNTRANSUTILS_H

#include "DunTransporter.h"

/**
 *  Notification interface class for managing channel's activity
 *  This class is needed to update KPSUidDialupConnStatus pub&sub key
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunActivityManager )
    {

public:

    /**
     * CDunTransporter callback: gets called when activity is detected on a
     * channel
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyChannelActivity() = 0;

    /**
     * CDunTransporter callback: gets called when inactivity is detected on a
     * channel
     *
     * @since S60 3.2
     * @return KErrGeneral if mismatch in channel activity, KErrNone otherwise
     */
    virtual TInt NotifyChannelInactivity() = 0;

    };

/**
 *  Utility accessor class for CDunTransporter class itself
 *  Shares basic functionality of CDunTransporter by simplifying it
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunTransporterUtility )
    {

public:

    /**
     * Initializes first free channel
     *
     * @since S60 3.2
     * @param aLocalIdNew Identifier of the new local endpoint
     * @return Symbian error code on error, found index otherwise
     */
    virtual TInt InitializeFirstFreeChannel( TConnId aLocalIdNew ) = 0;

    /**
     * Allocates a channel by creating and setting local media independent
     * objects
     * This is a common method used by exported local media dependent methods
     *
     * @since S60 3.2
     * @param aBufferLength Buffer length selected for this channel
     * @param aFirstFree Index to first free channel data
     * @param aCorrection Pointer to object implementing buffer correction
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual void DoAllocateChannelL(
        RComm* aComm,
        TInt& aBufferLength,
        TInt aFirstFree,
        MDunBufferCorrection* aCorrection ) = 0;

    /**
     * Adds connection monitor callback for either local media or network side
     * by connection ID
     * Error will be added to aIndex:th endpoint
     *
     * @since S60 3.2
     * @param aIndex Index where to add new connection monitor callback
     * @param aCallback Pointer to object whose callbacks will be called
     * @param aDirection Direction of operation to monitor for read/write error
     * @param aSignal Receive also signal change if ETrue
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoAddConnMonCallback( TInt aIndex,
                                       MDunConnMon* aCallback,
                                       TDunDirection aDirection,
                                       TBool aSignal ) = 0;

    /**
     * Adds error to consider as no error condition when doing any of the four
     * endpoint's read/writer operation
     * Error will be added to aIndex:th endpoint
     *
     * @since S60 3.2
     * @param aIndex Index where to add new "no error" code
     * @param aError Error code to consider as "no error"
     * @param aDirection One of four data transfer endpoints where to add a
     *                   skipped error code
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoAddSkippedError( TInt aIndex,
                                    TInt aError,
                                    TDunDirection aDirection ) = 0;

    /**
     * Issues transfers requests for aIndex:th transfer objects
     *
     * @since S60 3.2
     * @param aIndex Index to transfer objects that will be activated
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoIssueTransferRequests( TInt aIndex ) = 0;

    /**
     * Stops transfers for aIndex:th transfer objects
     *
     * @since S60 3.2
     * @param aIndex Index to transfer objects that will be stopped
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoStopTransfers( TInt aIndex ) = 0;

    /**
     * Free aIndex:th channel's objects
     *
     * @since S60 3.2
     * @param aIndex Index of channel to free
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoFreeChannel( TInt aIndex ) = 0;

    };

/**
 *  Utility accessor class for other than CDunTransporter classes
 *  Basically a collection of miscellaneous helper methods
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunTransporterUtilityAux )
    {

public:

    /**
     * Gets local ID counterpart of a network ID
     *
     * @since S60 3.2
     * @param aComm Network ID to search for
     * @return Null if ID not found, otherwise found ID
     */
    virtual TConnId GetLocalId( RComm* aComm ) = 0;

    /**
     * Notifies when serious read/write error is detected on a connection
     *
     * @since S60 3.2
     * @param aComm Non-null if error on RComm
     * @param aSocket Non-null if error on RSocket
     * @param aConnReason Reason of connection error
     * @param aCallbacks Callbacks to notify on connections error
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoNotifyConnectionNotOk(
        RComm* aComm,
        RSocket* aSocket,
        TDunConnectionReason& aConnReason,
        RPointerArray<MDunConnMon>& aCallbacks) = 0;

    };

/**
 *  Utility class for other CDunTransporter classes
 *  This class implements basic functionality that is shared by different
 *  CDunTransporter classes, also for simplifying CDunTransporter itself
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunTransUtils ) : public CBase,
                                      public MDunActivityManager,
                                      public MDunTransporterUtility,
                                      public MDunTransporterUtilityAux
    {

public:

    /**
     * Two-phased constructor.
     * @param aParent Parent class owning this friend class (CDunTransporter)
     * @param aPluginManager Callback to call when notification via
     *                       MDunPluginManager to be made
     * @return Instance of self
     */
	static CDunTransUtils* NewL( CDunTransporter& aParent,
	                             MDunPluginManager* aPluginManager );

    /**
    * Destructor.
    */
    virtual ~CDunTransUtils();

private:

    CDunTransUtils( CDunTransporter& aParent,
                    MDunPluginManager* aPluginManager );

    void ConstructL();

    /**
     * Manages service advertisement status changes
     *
     * @since S60 5.0
     * @param aAdvertise New advertisement status
     * @param aCreation ETrue if channel creation
     *                  EFalse if channel free
     * @return None
     */
    void ManageAdvertisementStatusChange( TBool aAdvertise,
                                          TBool aCreation=EFalse );

    /**
     * Creates empty channel data
     *
     * @since S60 3.2
     * @param aChannel Channel data to set empty
     * @return None
     */
    void CreateEmptyChannelData( TDunChannelData& aChannel );

    /**
     * Creates new buffer if source buffer defined, otherwise already existing
     * buffer will be used
     *
     * @since S60 3.2
     * @param aSrcBuffer Source buffer
     * @param aSrcPtr Pointer to source buffer
     * @param aDstBuffer Destination buffer
     * @param aDstPtr Pointer to destination buffer
     * @param aItemsInCs Items in cleanup stack, will be incremented if
     *                   necessary
     * @return None
     */
    void DoCreateBufferLC( TUint8* aSrcBuffer,
                           TPtr8* aSrcPtr,
                           TUint8*& aDstBuffer,
                           TPtr8*& aDstPtr,
                           TInt aBufferLength,
                           TInt& aItemsInCs );

    /**
     * Creates new signal copy object if source defined, otherwise
     * already existing will be used
     *
     * @since S60 3.2
     * @param aSrcSignalCopy Source signal copy object
     * @param aDstSignalCopy Destination signal copy object
     * @param aItemsInCs Items in cleanup stack, will be incremented if
     *                   necessary
     * @return None
     */
    void DoCreateSignalCopyLC( CDunSignalCopy* aSrcSignalCopy,
                               CDunSignalCopy*& aDstSignalCopy,
                               TInt& aItemsInCs );

    /**
     * Creates new signal notify object if source defined, otherwise
     * already existing will be used
     *
     * @since S60 3.2
     * @param aSrcSignalNotify Source signal notify object
     * @param aDstSignalNotify Destination signal notify object
     * @param aItemsInCs Items in cleanup stack, will be incremented if
     *                   necessary
     * @return None
     */
    void DoCreateSignalNotifyLC( CDunSignalNotify* aSrcSignalNotify,
                                 CDunSignalNotify*& aDstSignalNotify,
                                 TInt& aItemsInCs );

    /**
     * Creates transfer objects for reader and writer if sources defined,
     * otherwise already existing ones will be used
     *
     * @since S60 3.2
     * @param aSrcReader Source reader object
     * @param aDstReader Destination reader object
     * @param aItemsInCs Items in cleanup stack, will be incremented if
     *                   necessary
     * @return None
     */
    void DoCreateUpTransferObjectL( CDunUpstream* aSrcReader,
                                    CDunUpstream*& aDstReader,
                                    TInt& aItemsInCs );

    /**
     * Creates transfer objects for reader and writer if sources defined,
     * otherwise already existing ones will be used
     *
     * @since S60 3.2
     * @param aSrcReader Source reader object
     * @param aDstReader Destination reader object
     * @param aItemsInCs Items in cleanup stack, will be incremented if
     *                   necessary
     * @return None
     */
    void DoCreateDownTransferObjectL( CDunDownstream* aSrcReader,
                                      CDunDownstream*& aDstReader,
                                      TInt& aItemsInCs );

    /**
     * Resets/frees network data of aIndex:th channel
     *
     * @since S60 3.2
     * @param aIndex Index of channel to reset/free
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DeleteOneNetworkData( TInt aIndex );

    /**
     * Deletes local data of aIndex:th channel
     *
     * @since S60 3.2
     * @param aIndex Index of channel to delete
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DeleteOneLocalData( TInt aIndex );

    /**
     * Deletes buffering objects of aIndex:th channel
     *
     * @since S60 3.2
     * @param aIndex Index of channel to delete
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DeleteBuffering( TInt aIndex );

// from base class MDunTransporterUtility

    /**
     * From MDunTransporterUtility.
     * Initializes first free channel
     *
     * @since S60 3.2
     * @param aLocalIdNew Identifier of the new local endpoint
     * @return Symbian error code on error, found index otherwise
     */
    TInt InitializeFirstFreeChannel( TConnId aLocalIdNew );

    /**
     * From MDunTransporterUtility.
     * Allocates a channel by creating and setting local media independent
     * objects
     * This is a common method used by exported local media dependent methods
     *
     * @since S60 3.2
     * @param aBufferLength Buffer length selected for this channel
     * @param aFirstFree Index to first free channel data
     * @param aCorrection Pointer to object implementing buffer correction
     * @return Symbian error code on error, KErrNone otherwise
     */
    void DoAllocateChannelL( RComm* aComm,
                             TInt& aBufferLength,
                             TInt aFirstFree,
                             MDunBufferCorrection* aCorrection );

    /**
     * From MDunTransporterUtility.
     * Adds connection monitor callback for either local media or network side
     * by connection ID
     * Connection monitor will be added to aIndex:th endpoint
     *
     * @since S60 3.2
     * @param aIndex Index where to add new connection monitor callback
     * @param aCallback Pointer to object whose callbacks will be called
     * @param aDirection Direction of operation to monitor for read/write error
     * @param aSignal Receive also signal change if ETrue
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoAddConnMonCallback( TInt aIndex,
                               MDunConnMon* aCallback,
                               TDunDirection aDirection,
                               TBool aSignal );

    /**
     * From MDunTransporterUtility.
     * Adds error to consider as no error condition when doing any of the four
     * endpoint's read/writer operation
     * Error will be added to aIndex:th endpoint
     *
     * @since S60 3.2
     * @param aIndex Index where to add new "no error" code
     * @param aError Error code to consider as "no error"
     * @param aDirection One of four data transfer endpoints where to add a
     *                   skipped error code
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoAddSkippedError( TInt aIndex,
                            TInt aError,
                            TDunDirection aDirection );

    /**
     * From MDunTransporterUtility.
     * Issues transfers requests for aIndex:th transfer objects
     *
     * @since S60 3.2
     * @param aIndex Index to transfer objects that will be activated
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoIssueTransferRequests( TInt aIndex );

    /**
     * From MDunTransporterUtility.
     * Stops transfers for aIndex:th transfer objects
     *
     * @since S60 3.2
     * @param aIndex Index to transfer objects that will be stopped
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoStopTransfers( TInt aIndex );

    /**
     * From MDunTransporterUtility.
     * Free aIndex:th channel's objects
     *
     * @since S60 3.2
     * @param aIndex Index of channel to free
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoFreeChannel( TInt aIndex );

// from base class MDunTransporterUtilityAux

    /**
     * From MDunTransporterUtilityAux.
     * Gets local ID counterpart of a network ID
     *
     * @since S60 3.2
     * @param aComm Network ID to search for
     * @return Null if ID not found, otherwise found ID
     */
    TConnId GetLocalId( RComm* aComm );

    /**
     * From MDunTransporterUtilityAux.
     * Notifies when serious read/write error is detected on a connection
     *
     * @since S60 3.2
     * @param aComm Non-null if error on RComm
     * @param aSocket Non-null if error on RSocket
     * @param aConnReason Reason of connection error
     * @param aCallbacks Callbacks to notify on connections error
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoNotifyConnectionNotOk(
        RComm* aComm,
        RSocket* aSocket,
        TDunConnectionReason& aConnReason,
        RPointerArray<MDunConnMon>& aCallbacks);

// from base class MDunActivityManager

    /**
     * From MDunActivityManager.
     * Notifies about activity on a channel
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyChannelActivity();

    /**
     * From MDunActivityManager.
     * Notifies about inactivity on a channel
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyChannelInactivity();

private:  // data

    /**
     * Parent class owning this friend class (CDunTransporter)
     */
    CDunTransporter& iParent;

    /**
     * Array of abstractions of each "channel" (from parent iParent)
     * One TDunChannelData entry contains objects needed for one channel
     */
    RArray<TDunChannelData>& iChannelData;

    /**
     * Array of service advertisement data for each plugin (from parent iParent)
     * One TDunServAdvData entry contains objects needed for one plugin
     */
    RArray<TDunServAdvData>& iServAdvData;

    /**
     * Callback to call when notification via MDunPluginManager to be made
     * Not own.
     */
    MDunPluginManager* iPluginManager;

    };

#endif  // C_CDUNTRANSUTILS_H
