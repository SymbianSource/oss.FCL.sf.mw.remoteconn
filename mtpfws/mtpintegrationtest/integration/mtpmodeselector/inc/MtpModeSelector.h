// This material, including documentation and any related computer
// programs, is protected by copyright controlled by Nokia. All
// rights are reserved. Copying, including reproducing, storing
// adapting or translating, any or all of this material requires the
// prior written consent of Nokia. This material also contains
// confidential information which may not be disclosed to others
// without the prior written consent of Nokia.



/**
 @file
 @internalComponent
 @test
*/
#ifndef __MTPMODESELECTOR_H__
#define __MTPMODESELECTOR_H__

#include <eikdialg.h>
#include <eikchlst.h>
#include "CtransportInitiator.h"


class CMtpModeSelectorDlg : public CEikDialog
	{
public:	
	static CMtpModeSelectorDlg* NewL();
	CMtpModeSelectorDlg();
   
private:
    void ConstructL();
	~CMtpModeSelectorDlg();
	// from CEikDialog
	TBool OkToExitL(TInt aButtonId);
	void PreLayoutDynInitL();
public :
    CtransportInitiator* iTransportInitiator;	
	
	};
	

	
/**
 *	CModeSelectorAppView for creating a window and to draw the text
 *	
 */  
class CModeSelectorAppView : public CCoeControl
    {
public:
	// creates a CModeSelectorAppView object
	static CModeSelectorAppView* NewL(const TRect& aRect);
	~CModeSelectorAppView();
private:
	CModeSelectorAppView();
	void ConstructL(const TRect& aRect);
	//Draws the text on the screen	           
	void Draw(const TRect& /*aRect*/) const;

private:
	//contains the text needs to be drawn
	HBufC*  iExampleText;
    };
    
    
/**
 *	CModeSelectorAppUi handles the system events and menu events
 *	
 */  
class CModeSelectorAppUi : public CEikAppUi
    {
public:
    void ConstructL();
	~CModeSelectorAppUi();

private:
    // Inherirted from class CEikAppUi for handling menu events
	void HandleCommandL(TInt aCommand);
	
	// From CCoeAppUi to handle system events
	void HandleSystemEventL(const TWsEvent& aEvent);

private:
	CCoeControl* iAppView;
	};


/**
 *	CExampleDocument for creating the application UI
 *	
 */  
class CModeSelectorDocument : public CEikDocument
	{
public:
	// creates a CExampleDocument object
	static CModeSelectorDocument* NewL(CEikApplication& aApp);
	CModeSelectorDocument(CEikApplication& aApp);
	void ConstructL();
private: 
	// Inherited from CEikDocument for creating the AppUI
	CEikAppUi* CreateAppUiL();
	};
	
	
	
/**
 *	CExampleApplication creates a new instance of the document 
 *   associated with this application
 *	
 */  
class CModeSelectorApplication : public CEikApplication
	{
private: 
	// Inherited from class CApaApplication to create a new instance of the document
	CApaDocument* CreateDocumentL();
	//gets teh Application's UID
	TUid AppDllUid() const;
	};	 
	
#endif	   
	

	
