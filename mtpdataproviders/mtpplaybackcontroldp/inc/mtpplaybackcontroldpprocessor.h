// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#ifndef MTPPLAYBACKCONTROLDPPROCESSOR_H
#define MTPPLAYBACKCONTROLDPPROCESSOR_H

//forward declaration
class MMTPConnection;
class TMTPTypeRequest;
class CMTPDataProviderPlugin;
class MMTPDataProviderFramework;
class MMTPRequestProcessor;
class CMTPPlaybackControlDataProvider;
/** 
Defines device data provider request processor

@internalTechnology
*/
class MTPPlaybackControlDpProcessor
	{
public:
	static MMTPRequestProcessor* CreateL(
										MMTPDataProviderFramework& aFramework,
										const TMTPTypeRequest& aRequest, 
										MMTPConnection& aConnection,
										CMTPPlaybackControlDataProvider& aDataProvider);	
	};
	
#endif //MTPPLAYBACKCONTROLDPPROCESSOR_H

