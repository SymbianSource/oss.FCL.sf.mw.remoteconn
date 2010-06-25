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

#include "cmtpplaybackmap.h"
#include "cmtpplaybackproperty.h"
#include "mtpplaybackcontrolpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"MTPPlaybackProperty");)

const TInt32 KMTPDefaultPlaybackRate = 0;

const TUint32 KMTPMaxPlaybackVolume = 100;
const TUint32 KMTPMinPlaybackVolume = 0;
const TUint32 KMTPDefaultPlaybackVolume = 40;
const TUint32 KMTPCurrentPlaybackVolume = 40;
const TUint32 KMTPVolumeStep = 1;

const TUint32 KMTPDefaultPlaybackObject = 0;
const TUint32 KMTPDefaultPlaybackIndex = 0;
const TUint32 KMTPDefaultPlaybackPosition = 0;

/**
Two-phase constructor.
@param aPlugin The data provider plugin
@return a pointer to the created playback checker object
*/  
CMTPPlaybackProperty* CMTPPlaybackProperty::NewL()
    {
    CMTPPlaybackProperty* self = new (ELeave) CMTPPlaybackProperty();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackProperty::~CMTPPlaybackProperty()
    {    
    __FLOG(_L8("~CMTPPlaybackProperty - Entry"));
    delete iPlaybackVolumeData;
    __FLOG(_L8("~CMTPPlaybackProperty - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPPlaybackProperty::CMTPPlaybackProperty()
    {    
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackProperty::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    __FLOG(_L8("ConstructL - Exit")); 
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TInt32& aValue)
    {
    __FLOG(_L8("GetDefaultPropertyValueL - Entry"));
    
    __ASSERT_ALWAYS((aProp == EMTPDevicePropCodePlaybackRate), User::Leave(KErrArgument));
    aValue = KMTPDefaultPlaybackRate;

    __FLOG(_L8("GetDefaultPropertyValueL - Exit")); 
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TUint32& aValue)
    {
    __FLOG(_L8("GetDefaultPropertyValueL - Entry"));
    switch(aProp)
        {
    case EMTPDevicePropCodeVolume:
        {
        if(iPlaybackVolumeData != NULL)
            {
            aValue = iPlaybackVolumeData->DefaultVolume();            
            }
        else
            {
            aValue = KMTPDefaultPlaybackVolume;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackObject:
        {
        aValue = KMTPDefaultPlaybackObject;
        }
        break;
        
    case EMTPDevicePropCodePlaybackContainerIndex:
        {
        aValue = KMTPDefaultPlaybackIndex;
        }
        break;
        
    case EMTPDevicePropCodePlaybackPosition:
        {
        aValue = KMTPDefaultPlaybackPosition;
        }
        break;
        
    default:
        User::Leave(KErrArgument);   
        }
    __FLOG(_L8("GetDefaultPropertyValueL - Exit"));
    }

void CMTPPlaybackProperty::GetDefaultVolSet(TMTPPbDataVolume& aValue)
    {
    if(iPlaybackVolumeData == NULL)
        {
        aValue.SetVolume(KMTPMaxPlaybackVolume,
                         KMTPMinPlaybackVolume,
                         KMTPDefaultPlaybackVolume,
                         KMTPCurrentPlaybackVolume,
                         KMTPVolumeStep);
        }
    else
        {
        aValue = (*iPlaybackVolumeData);
        }
    }

void CMTPPlaybackProperty::SetDefaultVolSetL(const TMTPPbDataVolume& aValue)
    {
    if(iPlaybackVolumeData == NULL)
        {
        iPlaybackVolumeData = new (ELeave) TMTPPbDataVolume(aValue);
        }
    else
        {
        (*iPlaybackVolumeData) = aValue;
        }
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPPbCtrlData& aValue)
    {
    __FLOG(_L8("GetDefaultPropertyValueL - Entry"));
    __ASSERT_DEBUG((aValue.iOptCode == EMTPOpCodeResetDevicePropValue), Panic(EMTPPBArgumentErr));
    
    switch(aValue.iDevPropCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        TInt32 val;
        GetDefaultPropertyValueL(aValue.iDevPropCode, val);
        aValue.iPropValInt32.Set(val);
        }
        break;

    case EMTPDevicePropCodeVolume:
    case EMTPDevicePropCodePlaybackObject:
    case EMTPDevicePropCodePlaybackContainerIndex:
    case EMTPDevicePropCodePlaybackPosition:
        {
        TUint32 val;
        GetDefaultPropertyValueL(aValue.iDevPropCode, val);
        aValue.iPropValUint32.Set(val);
        }
        break;
        
    default:
        User::Leave(KErrArgument);
        }
    __FLOG(_L8("GetDefaultPropertyValueL - Exit"));
    }

TBool CMTPPlaybackProperty::IsDefaultPropertyValueL(const TMTPPbCtrlData& aValue) const
    {
    __FLOG(_L8("EqualToDefaultPropertyValueL - Entry"));
    
    TInt result(EFalse);

    switch(aValue.iDevPropCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        if(aValue.iPropValInt32.Value() == KMTPDefaultPlaybackRate)
            {
            result = ETrue;
            }
        }
        break;
            
    case EMTPDevicePropCodeVolume:
        {
        if(iPlaybackVolumeData == NULL)
            {
            if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackVolume)
                {
                result = ETrue;
                }
            }
        else
            {
            if(aValue.iPropValUint32.Value() == iPlaybackVolumeData->DefaultVolume())
                {
                result = ETrue;
                }
            }
        }
        break;

    case EMTPDevicePropCodePlaybackObject:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackObject)
            {
            result = ETrue;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackContainerIndex:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackIndex)
            {
            result = ETrue;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackPosition:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackPosition)
            {
            result = ETrue;
            }
        }
        break;
        
    default:
        User::Leave(KErrArgument);
        }

    __FLOG(_L8("EqualToDefaultPropertyValueL - Exit"));
    
    return result;
    }

