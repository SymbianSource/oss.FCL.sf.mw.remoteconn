// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __OBEXTRANSPORTFAULTS_H
#define __OBEXTRANSPORTFAULTS_H
/**
@file 
@internalComponent
*/

enum TObexTransportFaultCode
	{
	ETransportNullPointer					= 0,
	EStringOverflow							= 1,
	EPacketOverrun							= 2,
	EInvalidHeaderSize						= 3,
	EInvalidPacketSize						= 4,
	EDataLimitLargerThanBuffer				= 5,
	EOwnerAlreadySet						= 6,
	};
	
#endif
