/*****************************************************************************/
/*		This sample demonstrates how to continuously acquire pictures        */
/*      using a high level grab operation								     */
/*																			 */
/*		The display is done using the function imaqDisplayImage, 		 	 */																	
/*																			 */
/*****************************************************************************/     


#include <cvirte.h>		/* Needed if linking in external compiler; harmless otherwise */
#include <userint.h>
#include <stdio.h>
#include <utility.h>
#include <ansi_c.h>

#include "nivision.h"
#include "NIIMAQdx.h"
#include "Grab.h"



// globals
static int panelHandle;
static IMAQdxSession session = 0;
static Image* image = NULL;

// prototypes
void DisplayError(IMAQdxError error);

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)	/* Needed if linking in external compiler; harmless otherwise */
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Grab.uir", PANEL)) < 0)
		return -1;
	
	DisplayPanel (panelHandle);
	RunUserInterface ();
	
	return 0;
}



int CVICALLBACK Grab (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char			camName[64];
    IMAQdxError     error = IMAQdxErrorSuccess;
    
	switch (event)
	{
		case EVENT_COMMIT:
			// Create an image
			image = imaqCreateImage (IMAQ_IMAGE_U8, 0);

			// Get the camera name
			GetCtrlVal (panelHandle, PANEL_CAMNAME, camName);
		
			// Open a session to the selected camera
			error = IMAQdxOpenCamera (camName, IMAQdxCameraControlModeController, &session);
			if (error) 
			{
				break;
			}	
	
			// Configure and start the acquisiton
			error = IMAQdxConfigureGrab (session);
			if (error)
			{
				break;
			}
			
			// Activate the Imaq Loop
			SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, TRUE);
			SetCtrlAttribute (panelHandle, PANEL_STOP, ATTR_DIMMED, FALSE);
			SetCtrlAttribute (panelHandle, PANEL_GRAB, ATTR_DIMMED, TRUE);
			break;
	}
	
	if (error) 
	{
		DisplayError(error);
		
		// Stop the Imaq Loop and free resources
		Stop (panel, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
	}
	
	return 0;
}


int CVICALLBACK Stop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
			// Stop the Imaq Loop
			SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, FALSE);
			SetCtrlAttribute (panelHandle, PANEL_STOP, ATTR_DIMMED, TRUE);
			SetCtrlAttribute (panelHandle, PANEL_GRAB, ATTR_DIMMED, FALSE);
			
			// Close the session
			if (session)
			{
				IMAQdxCloseCamera (session);
				session = 0;
			}
			
			// Destroy the image
			if (image)
			{
				imaqDispose(image);
				image = NULL;
			}
			break;
	}
	return 0;
}

int CVICALLBACK Imaq_loop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	const double frameRateInterval = 0.5;
	static double lastTick = 0;
	double newTick;
	static unsigned int lastBufferNumber = - 1;
	unsigned int bufferNumber;
	IMAQdxError error = IMAQdxErrorSuccess;

	switch (event) 
	{
		case EVENT_TIMER_TICK:
			newTick = Timer();

			// Get the next frame  
			error = IMAQdxGrab (session, image, TRUE, &bufferNumber);
			if (error)
			{
				break;
			}
			
			// Display using NI Vision
			imaqDisplayImage (image, 0, TRUE);
			
			// Display actual buffer number every frame
			SetCtrlVal (panelHandle, PANEL_BUFFERNUMBER, bufferNumber);

			// Calculate the number of frame per seconds
			if ((newTick - lastTick) >= frameRateInterval)
			{
				double frameRate = (double)(bufferNumber - lastBufferNumber) / (newTick - lastTick);
	            lastTick = newTick;
	            lastBufferNumber = bufferNumber;
				
				SetCtrlVal (panelHandle, PANEL_FRAMERATE, frameRate);
			}
			break;
	}
	
	if (error)
	{
		DisplayError(error);
		
		// Stop the Imaq Loop and free resources
		Stop (panel, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
	}
	
	return 0;
}


int CVICALLBACK Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int stopped;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlAttribute (panelHandle, PANEL_STOP, ATTR_DIMMED, &stopped);
			if(!stopped)
			{
				// Stop the Imaq Loop and the acquisition
				Stop (panel, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
			}
			QuitUserInterface (0);
			break;
	}
	return 0;
}


void DisplayError(IMAQdxError error)
{
	// Convert the error to a string and present to user
	char errorString[256];
	IMAQdxGetErrorString(error, errorString, sizeof(errorString));
	MessagePopup("Error", errorString);
}

