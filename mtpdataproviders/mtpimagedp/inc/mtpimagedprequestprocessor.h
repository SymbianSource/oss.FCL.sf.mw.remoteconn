// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef MTPIMAGEDPPROCESSOR_H_
#define MTPIMAGEDPPROCESSOR_H_

class MMTPConnection;
class TMTPTypeRequest;
class CMTPDataProviderPlugin;
class MMTPDataProviderFramework;
class MMTPRequestProcessor;
class CMTPImageDataProvider;

/** 
Defines task data provider request processor factory

@internalTechnology
*/
class MTPImageDpProcessor
	{
public:
	static MMTPRequestProcessor* CreateL(
	                                    MMTPDataProviderFramework& aFramework,
	                                    const TMTPTypeRequest& aRequest, 
	                                    MMTPConnection& aConnection,
	                                    CMTPImageDataProvider& aDataProvider);    
	};

#endif /*MTPIMAGEDPPROCESSOR_H_*/

