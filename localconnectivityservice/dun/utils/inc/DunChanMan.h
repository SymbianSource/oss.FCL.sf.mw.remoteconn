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
* Description:  RComm channel management related functionality (waiter)
*
*/


#ifndef C_CDUNCHANMAN_H
#define C_CDUNCHANMAN_H

#include "DunTransUtils.h"
#include "DunTransporter.h"

class CDunSignalWaiter;
class CDunDataWaiter;

/**
 *  Class for CDunDataWaiter's and CDunSignalWaiter's connection monitoring
 *  support
 *  When CDunDataWaiter or CDunSignalWaiter detects activity, data in this
 *  class is used as helper data to create subsequent calls to
 *  CDunTransporter::AddConnMonCallbackL
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunConnMonCallback )
    {

public:

    /**
     * Callback to call when notification via MDunConnMon to be made
     * (CDunTransporter::AddConnMonCallbackL, aCallback parameter)
     * Not own.
     */
    MDunConnMon* iCallback;

    /**
     * Direction where to add connection monitor
     * (CDunTransporter::AddConnMonCallbackL, aDirection parameter)
     */
    TDunDirection iDirection;

    };

/**
 *  Class for CDunDataWaiter's and CDunSignalWaiter's connection monitoring
 *  support
 *  When CDunConnWaiter or CDunSignalWaiter detects activity, data in this
 *  class is used as helper data to create subsequent calls to
 *  CDunTransporter::AddSkippedErrorL
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunSkippedError )
    {

public:

    /**
     * Error code of read/write condition that should be skipped
     * (CDunTransporter::AddSkippedErrorL, aError parameter)
     */
    TInt iError;

    /**
     * Direction of read/write error condition
     * (CDunTransporter::AddSkippedErrorL, aDirection parameter)
     */
    TDunDirection iDirection;

    };

/**
 *  Class for CDunDataWaiter's or CDunSignalWaiter's connection monitoring
 *  support
 *  When CDunDataWaiter or CDunSignalWaiter detects activity, data in this
 *  class is used as helper
 *  data to create subsequent calls to the following APIs
 *  CDunTransporter::AddConnMonCallbackL
 *  CDunTransporter::AddSkippedErrorL
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunWaiterData )
    {

public:

    /**
     * RComm object of local media side
     * Not own.
     */
    RComm* iComm;

    /**
     * Name for the channel
     */
    HBufC8* iChannelName;

    /**
     * Signal waiter object for a channel
     * Not own.
     */
    CDunSignalWaiter* iSignalWaiter;

    /**
     * Data waiter object for a channel
     * Not own.
     */
    CDunDataWaiter* iDataWaiter;

    /**
     * Pointer to object implementing buffer correction
     * Not own.
     */
    MDunBufferCorrection* iCorrection;

    /**
     * Connection monitor raw data that will be added to TDunChannelData array
     * after activity detected by iDataWaiter or iSignalWaiter
     */
    RArray<TDunConnMonCallback> iConnMons;

    /**
     * Skipped error raw data that will be added to TDunChannelData array
     * after activity detected by iDataWaiter or iSignalWaiter
     */
    RArray<TDunSkippedError> iOkErrors;

    /**
     * Flag to indicate whether enqueued channel allocation required.
     * Second option is to use service advertisement API.
     */
    TBool iEnqueuedFail;

    /**
     * UID of plugin that allocated this channel
     * Used for making notification via MDunPluginManager callback
     * Added to TDunChannelData array after activity detected by iDataWaiter
     */
    TUid iOwnerUid;

    };

/**
 *  Notification interface class for creating new channel
 *  Used by CDunDataWaiter ro CDunSignalWaiter to create new channel when
 *  activity is detected
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunChannelAllocator )
    {

public:

    /**
     * CDunTransporter callback: gets called when waiter wants new channel
     *
     * @since S60 3.2
     * @param aComm Connection where new data exists
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyNewChannelRequest( RComm* aComm ) = 0;

    };

/**
 *  Class for RComm channel management related functionality (waiter)
 *  activity is detected by CDunDataWaiter or CDunSignalWaiter objects
 *  Also takes care of waiter object creation.
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunChanMan ) : public CBase,
                                   public MDunChannelAllocator
    {

public:

    /**
     * Two-phased constructor.
     * @param aParent Parent class owning this friend class (CDunTransporter)
     * @param aUtility Pointer to common utility class
     * @param aUtilityAux Pointer to other than CDunTransporter utility class
     * @param aPluginManager Callback to call when notification via
     *                       MDunPluginManager to be made
     * @return Instance of self
     */
	static CDunChanMan* NewL( CDunTransporter& aParent,
	                          MDunTransporterUtility* aUtility,
	                          MDunTransporterUtilityAux* aUtilityAux,
	                          MDunPluginManager* aPluginManager );

    /**
    * Destructor.
    */
    virtual ~CDunChanMan();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Number of waiters
     *
     * @since S60 3.2
     * @return Number of waiters
     */
    TInt NumberOfWaiters();

    /**
     * Gets number of waiters by owner UID
     *
     * @since S60 5.0
     * @param aOwnerUid UID of the channel owner
     * @return Number of waiters
     */
    TInt GetNumberOfWaitersByUid( TUid aOwnerUid );

    /**
     * Adds new connection waiter to connection waiter array
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @param aOwnerUid UID of the channel owner
     * @param aName Name for the channel
     * @param aEnqueuedFail ETrue if enqueued failure, EFalse otherwise
     * @param aCorrection Pointer to object implementing buffer correction
     * @return None
     */
    void AddConnWaiterL( RComm* aComm,
                         TUid aOwnerUid,
                         const TDesC8& aName,
                         TBool aEnqueuedFail,
                         MDunBufferCorrection* aCorrection );

    /**
     * Removes existing waiter from connection waiter array
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return KErrNotFound if waiter not found, KErrNone if removed
     */
    TInt RemoveConnWaiter( RComm* aComm );

    /**
     * Makes CDunDataWaiter and CDunSignalWaiter ready to detect activity
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return KErrNotFound if waiter not found, KErrNone if request issued
     */
    TInt IssueConnWaiterRequest( RComm* aComm );

    /**
     * Stops CDunDataWaiter and CDunSignalWaiter to detect new data
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @return KErrNotFound if waiter not found, KErrNone if stopped
     */
    TInt StopConnWaiter( RComm* aComm );

    /**
     * Saves waiter's connection monitor callback data
     *
     * @since S60 3.2
     * @param aComm Pointer to opened local media RComm ID object
     * @param aCallback Pointer to object whose callbacks will be called
     * @param aDirection Direction of operation to monitor for read/write
     *                   error
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SaveWaiterConnMonCallbackL( RComm* aComm,
                                     MDunConnMon* aCallback,
                                     TDunDirection aDirection );

    /**
     * Saves waiter's skipped error data
     *
     * @since S60 3.2
     * @param aError Error code to consider as "no error"
     * @param aComm Pointer to opened local media RComm ID object
     * @param aDirection One of four data transfer endpoints where to add a
     *                   skipped error code
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SaveWaiterSkippedErrorL( TInt aError,
                                  RComm* aComm,
                                  TDunDirection aDirection );

private:

    CDunChanMan( CDunTransporter& aParent,
                 MDunTransporterUtility* aUtility,
                 MDunTransporterUtilityAux* aUtilityAux,
                 MDunPluginManager* aPluginManager );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

     /**
      * Fills data for channel created by waiter
      *
      * @since S60 3.2
      * @param aComm Pointer to opened local media RComm ID object
      * @param aFirstFree Index to first free channel
      * @return Symbian error code on error, KErrNone otherwise
      */
    TInt FillNewWaiterChannelData(RComm* aComm, TInt aFirstFree);

    /**
     * Restores saved waiter data to connection data
     *
     * @since S60 3.2
     * @param aWaiterIndex Index to waiter data
     * @param aChannelIndex Index to connection data
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt RestoreWaiterData( TInt aWaiterIndex, TInt aChannelIndex );

    /**
     * Deletes waiter objects of aIndex:th waiters
     *
     * @since S60 3.2
     * @param aIndex Index of waiters to delete
     * @param aNewOwnership ETrue if ownership transferred, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DeleteWaiters( TInt aIndex, TBool aNewOwnership=EFalse );

// from base class MDunChannelAllocator

    /**
     * From MDunChannelAllocator.
     * Notifies when new channel is wanted
     *
     * @since S60 3.2
     * @param aComm Connection where new data exists
     * @return KErrNone
     */
    TInt NotifyNewChannelRequest( RComm* aComm );

private:  // data

    /**
     * Parent class owning this friend class (CDunTransporter)
     */
    CDunTransporter& iParent;

    /**
     * Pointer to common utility class
     * Not own.
     */
    MDunTransporterUtility* iUtility;

    /**
     * Pointer to other than CDunTransporter utility class
     * Not own.
     */
    MDunTransporterUtilityAux* iUtilityAux;

    /**
     * Callback to call when notification via MDunPluginManager to be made
     * Not own.
     */
    MDunPluginManager* iPluginManager;

    /**
     * Waiter's data that will be added to iChannelData when waiter detects
     * read activity. After copying to iChannelData the data will be removed
     * from this array.
     */
    RArray<TDunWaiterData> iWaiterData;

    };

#endif  // C_CDUNCHANMAN_H
