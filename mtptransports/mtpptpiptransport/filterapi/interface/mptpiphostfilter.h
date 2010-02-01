
// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Provides an interface to implement a filter(that filters the incoming requests), by mentioning 
// a basic behaviour any filter has to exhibit. There has to be a method to authenticate or validate
// an incoming request.This can be done by examining the Host GUID, or also, by prompting the user
// to accept or reject the request, by giving the user the details of connection request.
// 
//

/**
 @publishedPartner
 @released
*/


#ifndef MPTPIPHOSTFILTER_H_
#define MPTPIPHOSTFILTER_H_
 
 #include <e32def.h>
 #include <e32cmn.h>


class MPTPIPHostFilter
{
   public:
    		
   			/*Examine the parameters ,from PTPIP Controller i.e.,Host GUID,Host Friendly Name 
   			& filter out the requests that cannot be authenticated.The User can also be prompted using 
   			a dialog box asking whether to accept or reject the request by giving the details of connection request
   			@param HostFriendly Name, Host GUID and iStatus of the Caller,i.e PTP/IP Controller
   			 */
   			virtual void Accept(const TDesC& aHostFriendlyName,TDesC8& aHostGUID,TRequestStatus& aStatus)=0;
};

#endif
    		