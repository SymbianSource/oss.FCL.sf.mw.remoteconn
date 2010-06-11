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


#endif // MTPFRAMEWORKCONST_H
