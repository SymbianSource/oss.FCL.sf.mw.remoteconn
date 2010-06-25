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

#ifndef CMTPPLAYBACKPROPERTY_H
#define CMTPPLAYBACKPROPERTY_H

#include "mtpplaybackcontroldpconst.h"
#include "mtpdebug.h"
#include "cmtpplaybackcommand.h"

class CMTPPlaybackControlDataProvider;
class TMTPPbCtrlData;

/** 
Implements the CMTPPlaybackProperty.
@internalComponent
*/
class CMTPPlaybackProperty : public CBase
    {

public:

    static CMTPPlaybackProperty* NewL();    
    ~CMTPPlaybackProperty();

    /**
     Get the devide property value by property code,
     @param aProp,  DevicePropertyCode,
     @param aValue, Return value of the property,
    */
    void GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TInt32& aValue);

    /**
     Get the devide property value by property code,
     @param aProp,  DevicePropertyCode,
     @param aValue, Return value of the property,
    */
    void GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TUint32& aValue);
    
    /**
     Get the default devide property value,
     @param aValue, Value of the property,
    */
    void GetDefaultPropertyValueL(TMTPPbCtrlData& aValue);

    /**
     Get the default volume data set,
     @param aValue, Value of the property,
    */
    void GetDefaultVolSet(TMTPPbDataVolume& aValue);
    
    /**
     Set the default volume data set,
     @param aValue, Value of the property,
    */
    void SetDefaultVolSetL(const TMTPPbDataVolume& aValue);
	
	/**
     Check if the input is equalt to the default devide property value,
     @param aValue, Value of the property,
     @return ETrue if equal.
    */
    TBool IsDefaultPropertyValueL(const TMTPPbCtrlData& aValue) const;

private:
    CMTPPlaybackProperty();
    void ConstructL();

private: // Owned.

    __FLOG_DECLARATION_MEMBER_MUTABLE;
    TMTPPbDataVolume* iPlaybackVolumeData;
    };
    
#endif //CMTPPLAYBACKPROPERTY_H

