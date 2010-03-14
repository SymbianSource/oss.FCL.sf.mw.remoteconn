// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @test
*/
#if !defined(__CMAPPGroupVIEW_H__)
#define __CMAPPGroupVIEW_H__

#include "CMLSTVW.H"


class CContactsManAppGroupView : public CContactsManListView
	{
public:
	static CContactsManAppGroupView* NewL(CCoeControl& aContainer,CContactsAppModel& aModel);
	TInt CurrentItemIndex();
	TInt NumberOfItems();
private:
	CContactsManAppGroupView(CContactsAppModel& aModel,TContactsListViewPrefs* aPreferences);
	void NewGroupL();
// from CContactsManViewBase
	virtual void HandleModelChangeL(TBool aCountChanged);
	virtual TBool HandleCommandL(TInt aCommand);		
	virtual TUid ViewUid() const;
//from 	MCoeView
	virtual void ViewActivatedL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId, const TDesC8& aCustomMessage);
	virtual void ViewDeactivated();
// from CCoeControl
	virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	void ConstructL(CCoeControl& aContainer);
	};

#endif // __CMAPPGroupVIEW_H__
