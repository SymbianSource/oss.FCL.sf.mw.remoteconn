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

#ifndef __MTPPROXYDPPROCESSOR_H__
#define __MTPPROXYDPPROCESSOR_H__

//forward declaration
class MMTPConnection;
class TMTPTypeRequest;
class CMTPDataProviderPlugin;
class MMTPDataProviderFramework;
class MMTPRequestProcessor;


/** 
Defines proxy data provider request processor

@internalTechnology
  
*/
class MTPProxyDpProcessor
	{
public:
	static MMTPRequestProcessor* CreateL(
										MMTPDataProviderFramework& aFramework,
										const TMTPTypeRequest& aRequest, 
										MMTPConnection& aConnection);	
	};
	
#endif // __MTPPROXYDPPROCESSOR_H__

