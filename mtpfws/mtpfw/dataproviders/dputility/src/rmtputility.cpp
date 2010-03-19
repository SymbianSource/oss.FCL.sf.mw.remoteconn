
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

#include <bautils.h>
#include <f32file.h>
#include <e32math.h>
#include <e32def.h>
#include <caf/content.h>

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtpdataproviderplugin.h>

#include "rmtputility.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpextensionmapping.h"
#include "cmtpdataprovider.h"

using namespace ContentAccess;
// Class constants.
const TInt KMTPDateStringLength = 15;
const TInt KMTPDateStringTIndex = 8;
const TInt KMimeTypeMaxLength = 76;
const TInt KMAXPackageIDStringLen = 32;

_LIT( KTxtBackSlash, "\\" );
    
_LIT( KTxtExtensionODF, ".odf" );
    
_LIT( KMimeTypeAudio3gpp, "audio/3gpp" );
_LIT( KMimeTypeVideo3gpp, "video/3gpp" );
_LIT( KMimeTypeAudioMp4, "audio/mp4" );
_LIT( KMimeTypeVideoMp4, "video/mp4" );

__FLOG_STMT(_LIT8(KComponent,"RMTPUtility");)

RMTPUtility::RMTPUtility():
	iFramework(NULL)
	{
	}

void RMTPUtility::OpenL(MMTPDataProviderFramework& aFramework)
	{
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    
	iFramework = &aFramework;
	iSingleton.OpenL();
	
    __FLOG(_L8("OpenL - Exit"));
	}

void RMTPUtility::Close()
	{
	iSingleton.Close();
	iFramework = NULL;
	iFormatMappings.ResetAndDestroy();
	__FLOG_CLOSE;
	}

/*
 * Convert the TTime to the MTP datatime string
 * 
 * 	MTP datatime string format: YYYYMMDDThhmmss.s  Optional(.s)
 *  TTime string format       : YYYYMMDD:HHMMSS.MMMMMM
 *  
 */
EXPORT_C TBool  RMTPUtility::TTime2MTPTimeStr(const TTime& aTime, TDes& aRet ) const
	{
	_LIT(KMTPDateStringFormat,"%F%Y%M%DT%H%T%S");		
	TRAPD(err, aTime.FormatL(aRet,KMTPDateStringFormat));
	if(err == KErrNone)
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

/*
 * Convert the MTP datatime string to TTime:
 * 
 * 	MTP datatime string format: YYYYMMDDThhmmss.s  Optional(.s)
 *  TTime string format       : YYYYMMDD:HHMMSS.MMMMMM
 *  
 */
EXPORT_C TBool RMTPUtility::MTPTimeStr2TTime(const TDesC& aTimeString, TTime& aRet) const
	{
    __FLOG(_L8("ConvertMTPTimeStr2TTimeL - Entry"));

	TBool result = EFalse;
	TInt year = 0;
	TMonth month = EJanuary;
	TInt day = 0;
	TInt hour = 0;
	TInt minute  = 0;
	TInt second = 0;
	TInt tenthSecond = 0;
	TBool positiveTimeZone = ETrue;
	TInt timeZoneInHour = 0;
	TInt timeZoneInMinute = 0;

    const TChar KCharT = 'T';
	if( aTimeString.Length() >= KMTPDateStringLength && ( aTimeString.Locate(KCharT) == KMTPDateStringTIndex ) && GetYear(aTimeString, year) && GetMonth(aTimeString, month) && 
		GetDay(aTimeString, day) && GetHour(aTimeString, hour) && GetMinute(aTimeString, minute) && 
		GetSecond(aTimeString, second) && GetTenthSecond(aTimeString, tenthSecond) && GetTimeZone(aTimeString, positiveTimeZone, timeZoneInHour, timeZoneInMinute))
		{
		TDateTime dateTime(year, month, day, hour, minute, second, tenthSecond * 100000);
		
		TTime dateTimeInTTime(dateTime);
		if(positiveTimeZone)
			{
			dateTimeInTTime += TTimeIntervalHours(timeZoneInHour);
			dateTimeInTTime += TTimeIntervalMinutes(timeZoneInMinute);
			}
		else
			{
			dateTimeInTTime -= TTimeIntervalHours(timeZoneInHour);
			dateTimeInTTime -= TTimeIntervalMinutes(timeZoneInMinute);
			}		
		
		aRet = dateTimeInTTime.Int64();
		result = ETrue;
		}
	
	return result;
	}

TBool RMTPUtility::GetYear(const TDesC& aTimeString, TInt& aYear) const
	{
	aYear = 0;
	TLex dateBuf(aTimeString.Left(4));
	return dateBuf.Val(aYear) == KErrNone;
	}

TBool RMTPUtility::GetMonth(const TDesC& aTimeString, TMonth& aMonth) const
	{
	TBool result = EFalse;
	aMonth = EJanuary;
	TInt month = 0;
	TLex dateBuf(aTimeString.Mid(4, 2));
	if(dateBuf.Val(month) == KErrNone && month > 0 && month < 13)
		{
		month--;
		aMonth = (TMonth)month;
		result = ETrue;
		}
	return result;
	}

TBool RMTPUtility::GetDay(const TDesC& aTimeString, TInt& aDay) const
	{
	TBool result = EFalse;
	aDay = 0;
	TLex dateBuf(aTimeString.Mid(6, 2));
	if(dateBuf.Val(aDay) == KErrNone && aDay > 0 && aDay < 32)
		{
		aDay--;
		result = ETrue;
		}
	return result;	
	}

TBool RMTPUtility::GetHour(const TDesC& aTimeString, TInt& aHour) const
	{
	aHour = 0;
	TLex dateBuf(aTimeString.Mid(9, 2));
	return (dateBuf.Val(aHour) == KErrNone && aHour >=0 && aHour < 60);
	}
				
TBool RMTPUtility::GetMinute(const TDesC& aTimeString, TInt& aMinute) const
	{
	aMinute = 0;
	TLex dateBuf(aTimeString.Mid(11, 2));
	return (dateBuf.Val(aMinute) == KErrNone && aMinute >=0 && aMinute < 60);
	}

TBool RMTPUtility::GetSecond(const TDesC& aTimeString, TInt& aSecond) const
	{
	aSecond = 0;
	TLex dateBuf(aTimeString.Mid(13, 2));
	return (dateBuf.Val(aSecond) == KErrNone && aSecond >= 0 && aSecond < 60);
	}

TBool RMTPUtility::GetTenthSecond(const TDesC& aTimeString, TInt& aTenthSecond) const
	{
	TBool result = EFalse;
	aTenthSecond = 0;
	TInt dotPos = aTimeString.Find(_L("."));
	if(dotPos != KErrNotFound && dotPos == KMTPDateStringLength && aTimeString.Length() > KMTPDateStringLength + 1)
		{
		TLex dateBuf(aTimeString.Mid(dotPos + 1, 1));
		result = (dateBuf.Val(aTenthSecond) == KErrNone && aTenthSecond >=0 && aTenthSecond < 10);
		}
	else if(dotPos == KErrNotFound)
		{
		result = ETrue;
		}
	
	return result;	
	}

TBool RMTPUtility::GetTimeZone(const TDesC& aTimeString, TBool& aPositiveTimeZone, TInt& aTimeZoneInHour, TInt& aTimeZoneInMinute) const	 		
	{
	TBool result = EFalse;
	aTimeZoneInHour = 0;
	aTimeZoneInMinute = 0;
	TInt plusTimeZonePos = aTimeString.Find(_L("+"));
	TInt minusTimeZonePos = aTimeString.Find(_L("-"));
	TInt timeZoneIndicatorPos = Max(plusTimeZonePos, minusTimeZonePos);		
	aPositiveTimeZone = (plusTimeZonePos != KErrNotFound);
	if(timeZoneIndicatorPos != KErrNotFound)
		{
		if(aTimeString.Length() - timeZoneIndicatorPos == 5)
			{
			TLex dateBuf(aTimeString.Mid(timeZoneIndicatorPos + 1, 2));
			if(dateBuf.Val(aTimeZoneInHour) == KErrNone)
				{
				dateBuf.Assign(aTimeString.Mid(timeZoneIndicatorPos + 3, 2));
				if(dateBuf.Val(aTimeZoneInMinute) == KErrNone)
					{
					result = ETrue;
					}
				}
			}
		}
	else
		{
		result = ETrue;
		}
	return result;
	}

EXPORT_C void RMTPUtility::RenameObjectL( TUint aObjectHandle, const TDesC& aNewName )
	{
    __FLOG(_L8("RenameAssocationObjectL - Entry"));
    
    CMTPObjectMetaData* meta = CMTPObjectMetaData::NewLC();
       
    if( !iFramework->ObjectMgr().ObjectL(aObjectHandle, *meta) )
    	{
    	User::Leave(KErrNotFound);
    	}
			
   if( !BaflUtils::FileExists( iFramework->Fs(), meta->DesC(CMTPObjectMetaData::ESuid) ) )
	   {
	   User::Leave(KErrNotFound);
	   }
	
	TFileName filename;
	User::LeaveIfError(BaflUtils::MostSignificantPartOfFullName(meta->DesC(CMTPObjectMetaData::ESuid), filename));
	RBuf oldFullName;
	oldFullName.CleanupClosePushL();
	
	TInt maxLen = (KMaxFileName > meta->DesC(CMTPObjectMetaData::ESuid).Length()? KMaxFileName: meta->DesC(CMTPObjectMetaData::ESuid).Length());
	oldFullName.CreateL(maxLen);
	oldFullName = meta->DesC(CMTPObjectMetaData::ESuid);
		
	// Update the folder name using the new passed value.
	RBuf newFullName;
	newFullName.CleanupClosePushL();
	newFullName.CreateL(maxLen);
	newFullName.Append(oldFullName);
	if(meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
		{
		newFullName.SetLength(newFullName.Length() - filename.Length() - 1);
		}
	else
		{
		newFullName.SetLength(newFullName.Length() - filename.Length());
		}
	
	maxLen = newFullName.Length() + aNewName.Length() + 1;
	if(maxLen > newFullName.MaxLength())
		{
		newFullName.ReAllocL(maxLen);
		}
	newFullName.Append(aNewName);
	
	if(meta->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeAssociation)
		{
		// Modify the filename
		User::LeaveIfError( iFramework->Fs().Rename(meta->DesC(CMTPObjectMetaData::ESuid), newFullName) );
		
		meta->SetDesCL( CMTPObjectMetaData::ESuid, newFullName );
		iFramework->ObjectMgr().ModifyObjectL(*meta);
		}
	else
		{
		// Add backslash.
		_LIT(KBackSlash, "\\");
		newFullName.Append(KBackSlash);
		// Modify the filename
		User::LeaveIfError( iFramework->Fs().Rename(meta->DesC(CMTPObjectMetaData::ESuid), newFullName) );
		
		meta->SetDesCL( CMTPObjectMetaData::ESuid, newFullName );
		iFramework->ObjectMgr().ModifyObjectL(*meta);
		
		RenameAllChildrenL( meta->Uint(CMTPObjectMetaData::EStorageId), meta->Uint(CMTPObjectMetaData::EHandle), newFullName, oldFullName);
		
		if(meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
			{
			TMTPNotificationParamsHandle param = { meta->Uint(CMTPObjectMetaData::EHandle) ,oldFullName};
			iSingleton.DpController().NotifyDataProvidersL(EMTPRenameObject, static_cast<TAny*>(&param));
			}
		}

	CleanupStack::PopAndDestroy(3);//oldFullName, newFullName,meta
    __FLOG(_L8("RenameAssocationObjectL - Exit"));
	}

EXPORT_C TMTPFormatCode RMTPUtility::FormatFromFilename( const TDesC& aFullFileName )
    {
    if ( aFullFileName.Right( 1 ).CompareF( KTxtBackSlash ) == 0 ) 
        {
        return EMTPFormatCodeAssociation;
        }

    TParsePtrC file( aFullFileName );

    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        HBufC* mime =ContainerMimeType( file.FullName() );

        // 3GP
        if ( mime->CompareF( KMimeTypeAudio3gpp ) == 0 || mime->CompareF( KMimeTypeVideo3gpp ) == 0)
            {
            delete mime;
            mime = NULL;
            return EMTPFormatCode3GPContainer;
            }
        else if (  mime->CompareF( KMimeTypeAudioMp4 ) == 0 || mime->CompareF( KMimeTypeVideoMp4 ) == 0 )
            {
            delete mime;
            mime = NULL;
            return EMTPFormatCodeMP4Container;
            }
        if ( mime != NULL )
            {
            delete mime;
            mime = NULL;
            }
        }

    return EMTPFormatCodeUndefined;
    }

EXPORT_C HBufC* RMTPUtility::ContainerMimeType( const TDesC& aFullPath )
    {

    TParsePtrC file( aFullPath );
    HBufC* mime = NULL;
    TInt err = KErrNone;
    
    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        TRAP( err, mime = OdfMimeTypeL( aFullPath ) );
        }

    return mime;
    }

EXPORT_C void RMTPUtility::FormatExtensionMapping()
    {
     TInt count = iSingleton.DpController().Count();
    
    while(count--)
        {
        CDesCArraySeg* FormatExtensionMapping = new (ELeave) CDesCArraySeg(4);
        CleanupStack::PushL(FormatExtensionMapping);
        TRAP_IGNORE(iSingleton.DpController().DataProviderByIndexL(count).Plugin().SupportedL(EFormatExtensionSets,*FormatExtensionMapping));
        AppendFormatExtensionMapping(*FormatExtensionMapping,iSingleton.DpController().DataProviderByIndexL(count).DataProviderId());
        CleanupStack::PopAndDestroy(FormatExtensionMapping);
        }
    }

EXPORT_C TMTPFormatCode RMTPUtility::GetFormatByExtension(const TDesC& aExtension)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::Compare));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->FormatCode();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return EMTPFormatCodeUndefined;
    }

EXPORT_C TUint32 RMTPUtility::GetDpId(const TDesC& aExtension,const TDesC& aMIMEType)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined,aMIMEType);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->DpId();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return 255;
    }

EXPORT_C TUint RMTPUtility::GetEnumerationFlag(const TDesC& aExtension)
    {
    CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(aExtension, EMTPFormatCodeUndefined);
    CleanupStack::PushL(extensionMapping);
    TInt  found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::Compare));
    if ( KErrNotFound != found)
        {
        CleanupStack::PopAndDestroy(extensionMapping);
        return iFormatMappings[found]->EnumerationFlag();
        }
    CleanupStack::PopAndDestroy(extensionMapping);
    return 1;
    }

void RMTPUtility::RenameAllChildrenL(TUint32 aStorageId, TUint32 aParentHandle, const TDesC& aNewFolderName, const TDesC& aOldFolderName)
	{
    __FLOG(_L8("RenameAllChildrenL - Entry"));
    
    CMTPObjectMetaData* objectInfo(CMTPObjectMetaData::NewLC());
    TInt count = 0; 
    RArray<TUint>               handles;
    CleanupClosePushL(handles);
    GetAllDecendents(aStorageId, aParentHandle, handles);
    count = handles.Count();
    
    TEntry entry;
    for(TInt i(0); (i < count); ++i)
        {
        if (!iFramework->ObjectMgr().ObjectL(handles[i], *objectInfo))
            {
            User::Leave(KErrCorrupt);
            }
        
        /**
         * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
         * but it does not really own the format 0x3002.
         * 
         * Make the same behavior betwen 0x3000 and 0x3002.
         */
        if( (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeAssociation)
            && (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeUndefined)
            && (objectInfo->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeScript) )
           continue;

        RBuf entryName; 
        entryName.CreateL(KMaxFileName);
        entryName.CleanupClosePushL();
        entryName = objectInfo->DesC(CMTPObjectMetaData::ESuid);
        
        RBuf rightPartName;
        rightPartName.CreateL(KMaxFileName);
        rightPartName.CleanupClosePushL();
        rightPartName = entryName.Right(entryName.Length() - aOldFolderName.Length());
        
        if ((aNewFolderName.Length() + rightPartName.Length()) > entryName.MaxLength())
            {
            entryName.ReAllocL(aNewFolderName.Length() + rightPartName.Length());
            }
        
        entryName.Zero();
        entryName.Append(aNewFolderName);
        entryName.Append(rightPartName);
        
        if (KErrNone != iFramework->Fs().Entry(entryName, entry))
            {
            // Skip objects that don't use the file path as SUID.
            CleanupStack::PopAndDestroy(&entryName);
            continue;
            }        
        
        TFileName oldfilename(objectInfo->DesC(CMTPObjectMetaData::ESuid));
        objectInfo->SetDesCL(CMTPObjectMetaData::ESuid, entryName);
        iFramework->ObjectMgr().ModifyObjectL(*objectInfo);
        
        if(objectInfo->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation)
            {
            //Send the Rename notification 
            TMTPNotificationParamsHandle param = { handles[i], oldfilename};
            iSingleton.DpController().NotifyDataProvidersL(EMTPRenameObject, static_cast<TAny*>(&param));
            }
            
        CleanupStack::PopAndDestroy(2); // rightPartName, entryName             
        }
    
    CleanupStack::PopAndDestroy(2); //objectInfo; &handles; 
	
    __FLOG(_L8("RenameAllChildrenL - Exit"));
	}

void RMTPUtility::GetAllDecendents(TUint32 aStorageId, TUint aParentHandle, RArray<TUint>& aHandles) const
    {
    TInt index = 0; 
    TBool firstLevel = ETrue;
    
    do
        {
        TUint parentHandle;
        if (firstLevel)
            {
            parentHandle = aParentHandle; //Get the first level children handles
            firstLevel = EFalse;
            }        
        else
            {
            parentHandle = aHandles[index];
            ++index;
            }        
        
        RMTPObjectMgrQueryContext   context;
        RArray<TUint>               childrenHandles;
        TMTPObjectMgrQueryParams    params(aStorageId, KMTPFormatsAll, parentHandle);
        CleanupClosePushL(context);
        CleanupClosePushL(childrenHandles);
        
        do
            {
            iFramework->ObjectMgr().GetObjectHandlesL(params, context, childrenHandles);
            TInt count = childrenHandles.Count(); 
            for (TUint i = 0; i < count; ++i)
                {
                aHandles.Append(childrenHandles[i]);
                }
            }
        while (!context.QueryComplete());
        CleanupStack::PopAndDestroy(2); //&childrenHandles; &context
        }
    while(index < aHandles.Count());

    }

HBufC* RMTPUtility::OdfMimeTypeL( const TDesC& aFullPath )
    {
    HBufC* mimebuf = NULL;
    
    TParsePtrC file( aFullPath );
        
    if ( file.Ext().CompareF( KTxtExtensionODF ) == 0 )
        {
        RFs tempFsSession; 
        User::LeaveIfError(tempFsSession.Connect());     
        CleanupClosePushL(tempFsSession);
        User::LeaveIfError(tempFsSession.ShareProtected());  
        
        RFile tempFile; 
        User::LeaveIfError(tempFile.Open(tempFsSession, aFullPath, EFileRead|EFileShareAny)); 
        CleanupClosePushL(tempFile); 
        
        //CContent* content = CContent::NewL( aFullPath );
        CContent* content = CContent::NewL( tempFile );
        CleanupStack::PushL( content ); // + content
        
        HBufC* buffer = HBufC::NewL( KMimeTypeMaxLength );
        CleanupStack::PushL( buffer ); // + buffer
        
        TPtr data = buffer->Des();
        TInt err = content->GetStringAttribute( EMimeType, data );
                
        if ( err == KErrNone )
            {
            mimebuf = HBufC::New( buffer->Length() );
    
            if (mimebuf == NULL)
                {
                User::LeaveIfError( KErrNotFound );
                }
            
            mimebuf->Des().Copy( *buffer );
            }
        
        // leave if NULL
        if ( mimebuf == NULL )
            {
            User::Leave( KErrNotFound );
            }
        
        CleanupStack::PopAndDestroy( buffer ); // - buffer
        CleanupStack::PopAndDestroy( content ); // - content
        CleanupStack::PopAndDestroy(&tempFile); // close 
        CleanupStack::PopAndDestroy(&tempFsSession);    // close 
        }
    else
        {
        User::Leave( KErrNotSupported );
        }
    
    return mimebuf;
    }

void  RMTPUtility::AppendFormatExtensionMapping(const CDesCArray& aFormatExtensionMapping,TUint32 aDpId)
    {
    //Parse the descriptor formatcode,fileextension, e.g. 0x3009:mp3
     TBuf<KMAXPackageIDStringLen> stringSeg;
     TInt  splitter1(0);
     TInt  splitter2(0);
     TInt  found(0);
     TUint formatCode = 0;
     TUint isNeedFileDp = 1;
     
     for(TInt i=0; i < aFormatExtensionMapping.Count(); ++i)
         {
         CMTPExtensionMapping* extensionMapping = CMTPExtensionMapping::NewL(KNullDesC, EMTPFormatCodeUndefined);
         CleanupStack::PushL(extensionMapping);
         _LIT(KSPLITTER,":");
         splitter1 = aFormatExtensionMapping[i].FindF(KSPLITTER);
         //Skip "0x", 2 is the length of "0x"
         stringSeg = aFormatExtensionMapping[i].Mid(2, 4);
         TLex lex(stringSeg);
         User::LeaveIfError(lex.Val(formatCode, EHex));
         //Skip ":"
         stringSeg = aFormatExtensionMapping[i].Mid(splitter1 + 1);
         splitter2 = stringSeg.FindF(KSPLITTER);
         if ( splitter2 == KErrNotFound )
             {
             extensionMapping->SetExtensionL(stringSeg);
             }
         else
             {
             extensionMapping->SetExtensionL(aFormatExtensionMapping[i].Mid(splitter1+1,splitter2));
             stringSeg = stringSeg.Mid(splitter2+1);
             splitter1 = stringSeg.FindF(KSPLITTER);
             if ( splitter1==KErrNotFound )
                 {
                 extensionMapping->SetMIMETypeL(stringSeg);
                 }
             else if ( splitter1==0 )
                 {
                 TLex lex1(stringSeg.Mid(splitter1+1));
                 User::LeaveIfError(lex1.Val(isNeedFileDp, EDecimal));                 
                 }
             else
                 {
                 extensionMapping->SetMIMETypeL(stringSeg.Mid(0,splitter1));
                 TLex lex2(stringSeg.Mid(splitter1+1));
                 User::LeaveIfError(lex2.Val(isNeedFileDp, EDecimal));
                 }
             
             }

         found = iFormatMappings.FindInOrder(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
         if (KErrNotFound == found)
             {
             extensionMapping->SetFormatCode((TMTPFormatCode)formatCode);
             extensionMapping->SetDpId(aDpId);
             extensionMapping->SetEnumerationFlag(isNeedFileDp);
             iFormatMappings.InsertInOrderL(extensionMapping, TLinearOrder<CMTPExtensionMapping>(CMTPExtensionMapping::ComparewithMIME));
             }
         CleanupStack::Pop(extensionMapping);
         }    
    }
