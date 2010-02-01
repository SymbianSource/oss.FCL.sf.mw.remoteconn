/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
* Declaration of CSecureBUREncryptKeySource
* 
*
*/



/**
 @file
*/
#ifndef __SBENCRYPT_H__
#define __SBENCRYPT_H__
#include <e32base.h>
#include <f32file.h>

namespace conn
  { 
class CSecureBURKeySourceImpl;

class CSecureBUREncryptKeySource : public CBase
	/**
    This class is responsible for providing keys for encryption of backup data
    or decryption of restore data based on drive and SID of the data owner.

    The class will be included in a separate key-providing DLL provided by licensees
    at build time.  The default Symbian implementation will not provide keys.

    It is permissible to not provide any keys and to not encrypt data.  It is also
    permissible for one key to be common to all or a set of SIDs or for one key to be 
    common to a set of drives.

    The class can provide a data buffer with backup keys.  If it does then the buffer
    will be stored (un-encrypted!) with the backup and will be provided when keys are
    requested for a restore operation.  The class supports a default data buffer for a
    whole backup plus the ability to override it with buffers for specific SIDs.  If 
    the buffer is used then the implementor must be aware that is is not encrypted in the
    backup and so must not contain any sensitive data or any data that would allow an
    attacker to recreate the key.  If a buffer is provided then it is returned at restore
    time on a per-SID basis - there is no provision to provide a default buffer at restore
    time.

    If keys are provided then the implementor needs to consider a number of factors:
    
    @li If the key is entered by the user then it should be assumed that the user can 
    decrypt data off the device.

    @li If the key is local to the device then the data cannot be restored to a new device.

    @li If the key depends on the drive being backed up or restored then the implementor
    needs to consider whether a drive may have its letter changed (e.g. if a device has
    multiple slots for removable media).

    This class owns a CSecureBURKeySourceImpl instance and publishes the 
    public API to the outside world. The reason for this facade class is twofold:

    @li Hiding the implementation details of CSecureBURKeySourceImpl 

    @li Future binary compatibility
    
    @released
    @publishedPartner
	*/	
    {
public:
	/**
	Static factory method (two phase construction)

    @return  Pointer to the CSecureBUREncryptKeySource instance
	*/	
    IMPORT_C static CSecureBUREncryptKeySource* NewL();
    /**
    Standard virtual destructor
    */
    IMPORT_C virtual ~CSecureBUREncryptKeySource();

    /**
    Get a default data buffer for all backups of a specified drive.

    @param aDrive the drive being backed up - may be ignored
    @param aGotBuffer set to ETrue on return if a buffer is supplied
    @param aBuffer if aGotBuffer is set to ETrue then this is a buffer of data to be
    included with backups.
    */
    IMPORT_C void GetDefaultBufferForBackupL(TDriveNumber aDrive, TBool& aGotBuffer, TDes& aBuffer);

    /**
    Provides a key to use to encrypt backup data for a specific data owner from a specific
    drive.  It is permissible to provide the same key for some or all data owners.  It is 
    permissible to provide the same key for some or all drives.
    
    @param aDrive the drive (EDrive A to EDriveZ) which is being backed up
    @param aSID the secure id of the data owner
    @param aDoEncrypt returns ETrue if a key is provided, EFalse if data is not to be encrypted
    @param aKey the key to use to encrypt data - ignored if aDoEncrypt is set to EFalse
    @param aGotBuffer returns ETrue if a buffer is returned that is specific to this SID
    @param aBuffer if aGotBuffer is set to ETrue then this is a buffer of data to be
    included with backups.
    */
    IMPORT_C void GetBackupKeyL(TDriveNumber aDrive, TSecureId aSID,
                                TBool &aDoEncrypt, TDes8& aKey,
                                TBool& aGotBuffer, TDes& aBuffer);

    /**
    Provides a key to use to decrypt backup data for a specific data owner from a specific
    drive.  It is permissible to provide the same key for some or all data owners.  It is 
    permissible to provide the same key for some or all drives.
    
    @param aDrive the drive (EDrive A to EDriveZ) which is being restored
    @param aSID the secure id of the data owner
    @param aGotBuffer set to ETrue if a buffer is provided
    @param aBuffer if aGotBuffer is set to ETrue then this is a buffer of data that was 
    provided with the key for the backup (or the default buffer)
    @param aGotKey returns ETrue if a key is provided, EFalse if data is not to be decrypted
    @param aKey the key to use to decrypt data - ignored if aGotKey is set to EFalse
    */
    IMPORT_C void GetRestoreKeyL(TDriveNumber aDrive, TSecureId aSID, 
                                 TBool aGotBuffer, TDes& aBuffer,
                                 TBool &aGotKey, TDes8& aKey);

private:
    /**
    Standard C++ Constructor
    */
    CSecureBUREncryptKeySource();
    /**
    Symbian Second phase constructor
    */
    void ConstructL();
 
private:
	/** Pointer the the CSecureBURKeySourceImpl implementation */
	CSecureBURKeySourceImpl* iImpl;
    };

  } // end namespace
#endif

