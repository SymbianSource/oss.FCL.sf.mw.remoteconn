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

/**
 @file
 @internalTechnology
*/

#ifndef MTPFRAMEWORKCONST_H
#define MTPFRAMEWORKCONST_H

/**
The device data provider implementation UID.
@internalTechnology
 
*/
const TUint KMTPImplementationUidDeviceDp(0x102827AF);

/**
The file data provider implementation UID.
@internalTechnology
 
*/
const TUint KMTPImplementationUidFileDp(0x102827B0);

/**
The proxy data provider implementation UID.
@internalTechnology
 
*/
const TUint KMTPImplementationUidProxyDp(0x102827B1);

/**
The maximum number of concurrently enumerating data providers.
@internalTechnology
 
*/
const TUint KMTPMaxEnumeratingDataProviders(4);

/** 
The maximum SQL statement length.
@internalTechnology
 
*/
const TInt KMTPMaxSqlStatementLen = 384;

/**
The maximum SUID length.
@internalTechnology
 
*/
const TInt KMTPMaxSuidLen = 255;

/**
The deivce data provider DPID.
@internalTechnology
*/
const TUint KMTPDeviceDPID = 0;

/**
The file data provider DPID.
@internalTechnology
*/
const TUint KMTPFileDPID = 1;

/**
The proxy data provider DPID.
@internalTechnology
*/
const TUint KMTPProxyDPID = 2; 

//MTP should reserve some disk space to prevent OOD(Out of Disk) monitor 
//popup 'Out of memory' note.When syncing music through ovi player,
//sometimes device screen get freeze with this note
//Be default, this value is read from Central Respository, if error while
//reading, use this one
const TInt KFreeSpaceThreshHoldDefaultValue(20*1024*1024);//20M bytes

//Beside the OOD threshold value, we need to reserve extra disk space
//for harvest server do the harvest, set this as 1M
const TInt KFreeSpaceExtraReserved(1024*1024);//1M bytes
#endif // MTPFRAMEWORKCONST_H
