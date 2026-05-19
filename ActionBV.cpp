
#include "ActionBV.h"
#include <string>
#include <string.h>

ActionBV::ActionBV(double border, ArPTZ *camera) :
  ArAction("PioneerBV")
{
	m_Border = border;
	m_Camera = camera;

	//reset pan and tilt
	m_Camera->tilt(0);
	m_Camera->pan(0);

	m_SharedMemory = new Shared_Memory();
	m_SharedMemory->SM_Init();

	if (m_SharedMemory == NULL)
    {
        deactivate();
		Aria::exit(1);
    }

	m_X = m_SharedMemory->SM_GetFloat(INDEX_X);
	m_Y = m_SharedMemory->SM_GetFloat(INDEX_Y);
}

ArActionDesired *ActionBV::fire(ArActionDesired currentDesired)
{
	m_Desired.reset();
	//track general position for logs
	bool top;
	bool left;
	bool visible;
	bool center;

	if (m_Camera != NULL){
		center = false;
		
		//map the current y-Value to a range of [-2,2] for tilt speed of the camera
		float tiltMapping = output_start + m_Slope * (*m_Y - input_start);
		if (*m_Y > m_Border)
		{
			//object is in the top half of the image
			top = true;
			m_Camera->tiltRel(tiltMapping);
		}
		else if (*m_Y < m_Border * -1)
		{
			//object is in the bottom half of the image
			top = false;
			m_Camera->tiltRel(tiltMapping);
		}
		else
		{
			//object is inside specified center
			center = true;
		}
	}


	//prüfe ob der Schwellwert überschritten ist
	//und lege die Drehgeschwindigkeit, abhängig der Objektposition fest


	float* m_Visible = m_SharedMemory->SM_GetFloat(INDEX_VISIBLE);

	if (*m_Visible == 0){
		//object is not visible
		//rotate in the direction the object was last seen to look for it
		center = false;
		visible = false;
		if (*m_X < 0){
			//object was last seen in the left half of the image
			left = true;
			m_Desired.setRotVel(15);
		}
		else
		{
			//object was last seen in the right half of the image
			left = false;
			m_Desired.setRotVel(-15);
		}
	}
	else {
		//object is visible
		visible = true;
		if (*m_X > m_Border || *m_X < m_Border * -1)
		{
			//object is not inside the specified center
			center = false;
			//calculate speed based on x-Value
			float rotVel = *m_X / 5 * -1;

			//check on which side the object is
			if (rotVel > 0){
				left = true;
			}
			else
			{
				left = false;
			}
			m_Desired.setRotVel(rotVel);
		}
		else
		{
			//object is inside specified center
		center = true;
		}
	}

	if (center){
		ArLog::log(ArLog::Terse, "Object is near the center");
	}
	else
	{
	string vPosition = string((top ? "top" : "bottom")); 
	string hPosition = string(left ? "left" : "right");
	string vis1 = string(visible ? "visible" : "not visible");
	string vis2 = string(visible ? "is" : "was last seen");
	//TODO center on 1 axis
	string msg = string("Object is ") + string(visible ? "visible" : "not visible") + string(" and ") + string(visible ? "is" : "was last seen") + string(" in the ") + string(top ? "top" : "bottom") + string(left ? " left" : " right");
	cout << msg << "\n";
	}

	return &m_Desired;
}


int main(int argc, char** argv)
{

  double border = 20;
  
  Aria::init();

  ArSimpleConnector conn(&argc, argv);
  ArRobot robot;
  ArArgumentParser parser(&argc, argv);
  ArPTZConnector ptzConnector(&parser, &robot);
  ArPTZ *ptz;
  //Erzeugen der Actions
  ArActionStallRecover recover;
    
  // Parsen der Argumente
  if(!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::exit(1);
  }
  
  // Verbindung mit dem Roboter herstellen
  if(!conn.connectRobot(&robot))
  {
    ArLog::log(ArLog::Terse, "actionExample: Could not connect to robot! Exiting.");
    Aria::logOptions();
    Aria::exit(1);
  }

  //connect the camera
  ptzConnector.connect();

  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);

  // Key-Handler hinzufügen, um die Anwendung beenden zu können
  robot.attachKeyHandler(&keyHandler);
  printf("You may press escape to exit\n");

  //get actual camera (device) from connector
  ptz = ptzConnector.getPTZ();
  ActionBV pioneerBV(border,ptz);

  // Actions mit bestimmter Priorität zum Roboter hinzufügen
  robot.addAction(&recover, 100);
  robot.addAction(&pioneerBV, 50);  

  // Anschalten der Motoren
  robot.enableMotors();

  // Prozessschleife des Roboters starten
  robot.runAsync(true);

  //auf Abbruchbedingung warten
  robot.waitForRunExit();
  Aria::exit(0);
  return 0;
}