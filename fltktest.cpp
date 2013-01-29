// fltktest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <fltk.h>
#include <FL/Fl.H>  
#include <FL/Fl_Window.H>  
#include <FL/Fl_Box.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Button.H>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <Windows.h>
#include <FL\fl_draw.H>
#include <FL\math.h>
#include <FL\Fl_Text_Buffer.H>
#include <FL\Fl_Text_Display.H>
#include <vector>
#include <fstream>

using namespace std;

extern "C" { 
	#include <LightStoneManager.h>
}

#define BUFSIZE 16

// *** Original Packet Function
void printPacket(LightStoneControlPacket *packetPtr)
{
	switch (packetPtr->type)
	{
	case LightStoneControlPacketTypeData:

		if (LightStoneDataPacketTypeRaw ==  packetPtr->dataPacket.type)
		{
			printf("Packet type:%d first value:%lx second value: %lx\n", packetPtr->dataPacket.type, packetPtr->dataPacket.data1, packetPtr->dataPacket.data2);
		}
		else
		{
			printf("Packet type:%d  value:%lx \n", packetPtr->dataPacket.type, packetPtr->dataPacket.data1);
		}
		break;

	case LightStoneControlPacketTypeMalformedData:
		printf(" Received LightStoneControlPacketTypeMalformedData packet\n");
		break;

	case LightStoneControlPacketTypeDeviceDataError:
		printf("Received LightStoneControlPacketTypeDeviceDataError packet\n");
		break;

	case LightStoneControlPacketTypeDeviceOpened:
		printf("Received LightStoneControlPacketTypeDeviceOpened packet\n");
		break;
	case LightStoneControlPacketTypeDeviceClosed:
		printf("Received LightStoneControlPacketTypeDeviceClosed packet\n");
		break;
	case LightStoneControlPacketTypeDeviceOpenFailed:
		printf("Received LightStoneControlPacketTypeDeviceOpenFailed packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneDeviceAdded:
		printf("Received LightStoneControlPacketTypeLightStoneDeviceAdded packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneDeviceRemoved:
		printf("Received LightStoneControlPacketTypeLightStoneDeviceRemoved packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneStartedRunning:
		printf("Received LightStoneControlPacketTypeLightStoneStartedRunning packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneStoppedRunning:
		printf("Received LightStoneControlPacketTypeLightStoneStoppedRunning packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneStartedReading:
		printf("Received LightStoneControlPacketTypeLightStoneStartedReading packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneStoppedReading:
		printf("Received LightStoneControlPacketTypeLightStoneStoppedReading packet\n");
		break;
	case LightStoneControlPacketTypeLightStoneGeneralError:
		printf("Received LightStoneControlPacketTypeLightStoneGeneralError packet\n");
		break;
	default:
		printf("Received mystery packet of type:%d\n", packetPtr->type);
		break;

	}

	return;
}

void writeSCLdata(vector<float> vec)
{

	ofstream myfile;
	myfile.open ("SCLdata.txt");
	for ( int i =0; i< vec.size(); i++)
		myfile << vec.at(i) << endl;

	myfile.close();
}


// *** Return the Skin Conductance Value
double condValue(LightStoneControlPacket *packetPtr)
{
	switch (packetPtr->type)
	{
	case LightStoneControlPacketTypeData:

		if (LightStoneDataPacketTypeRaw ==  packetPtr->dataPacket.type)
		{
			printf("Packet type:%d SC Level:%d Raw Heart: %d\n", packetPtr->dataPacket.type, packetPtr->dataPacket.data1, packetPtr->dataPacket.data2);
			return packetPtr->dataPacket.data1*0.01;
		}
		else
		{
			//printf("Packet type:%d  value:%lx \n", packetPtr->dataPacket.type, packetPtr->dataPacket.data1);
			//if(packetPtr->dataPacket.type != 2)
			//	return packetPtr->dataPacket.data1;
			//else
				return -1;
		}
		break;

	
	default:
		printf("Packet has no value:%d\n", packetPtr->type);
		return -1;
		break;
	}
}

// *** Return the Heart Rate Value
double heartValue(LightStoneControlPacket *packetPtr)
{
	switch (packetPtr->type)
	{
	case LightStoneControlPacketTypeData:

		if (LightStoneDataPacketTypeRaw ==  packetPtr->dataPacket.type)
		{
			return packetPtr->dataPacket.data2;
		}
		else
		{
				return -1;
		}
		break;

	default:
		return -1;
		break;
	}
}

// *** Draw the Chart and Window Here
int main(int argc, char **argv)  
{  
  char data[BUFSIZE];
  int nextChar = 0;
  
	vector<float> dataVector;
  LightStoneControlPacket packet;

	LightStoneManagerInit();
	LightStoneManagerSetMaxNumLightStones(1);
	LightStoneManagerCheckForLightStones();
	if (!LightStoneManagerNumLightStones())
	{
		printf("No lightstones found - is there a lightstone plugged in?\n");
		system("Pause");
	}
	else
	{
		// the device is opened and a device manager thread is fired up when it's found by LightStoneManagerCheckForLightStones()
		// Go grab 1000 packets from the device. We'll get back a 'NoPackets' packet if the device has nothing to report yet.

	Fl_Window *window = new Fl_Window(1000,700,"Biometric_Display_Graph v0.01.a");
	int MAX_ELEM = 250;
	Fl_Chart *SCLchart = new Fl_Chart(5,5, 995, 500, "Skin Conductance"); 
	SCLchart->maxsize(MAX_ELEM);
	SCLchart->type(FL_LINE_CHART);
//	for(int i=0;i<250;i++)
//		SCLchart->insert(i,0,"",0);

	Fl_Chart *HRchart = new Fl_Chart(5,525, 500, 155, "Raw Heart Data"); 
	HRchart->maxsize(100);
	HRchart->type(FL_LINE_CHART);

	Fl_Button *pause = new Fl_Button(900,650,50,30,"Pause");
	pause->type(FL_TOGGLE_BUTTON);
	Fl_Button *close = new Fl_Button(950,650,50,30,"Quit");

	Fl_Text_Buffer  *sclbuff = new Fl_Text_Buffer();
    Fl_Text_Display *scldisp = new Fl_Text_Display(575, 560, 100, 40, "Skin Conductance Level");
    scldisp->buffer(sclbuff);

	double SCLVALUE = 0; double min = 0;	double max = 0;
	double HR_VALUE = 0; double hr_min = 0; double hr_max = 0;
	int    x = 0;
	char   buff[5];

	window->show(argc, argv);

	while(1)
		{


			Fl::check(); // Updates the Graphics

			// Quit Statement
			if(close->value() == 1){
				window->~Fl_Window();
				break;
			}

			// Resume from Paused
			if(pause->value() == 1){
				pause->label("Start");
				continue;
			}
			else
				pause->label("Pause");

			memset(data, 0, BUFSIZE);
			nextChar = 0;
			
			LightStoneManagerPacketResult result = LightStoneManagerNoPackets;
			while (LightStoneManagerNoPackets == result)
			{
			//	Sleep(30);
				result = LightStoneManagerGetNextPacket(&packet);
			} 

			// printPacket(&packet);
			// Get the Skin Conductance Value
			SCLVALUE = condValue(&packet);
			if(SCLVALUE==-1){
				continue;
			}

			// Get the Heart Rate value
			HR_VALUE = heartValue(&packet);
			if(HR_VALUE==-1){
				continue;
			}

			// Set the parameters for the HR Graphic
			if( HR_VALUE-1 < hr_min) 
					hr_min = HR_VALUE - 1.0;
			if( HR_VALUE+1 > hr_max) 
					hr_max = HR_VALUE + 1.0;

			
			// Set the Parameters for the SCL Graphic
			min = SCLVALUE - 1.0;
			max = SCLVALUE + 1.0;

			// cout << "SCL Bounds are: "<< min <<", "<< max <<'\n';

			// Data graphic
			char SCLstring[22];   //To hold '.' and null
			sprintf(SCLstring,"%20.4f",SCLVALUE);
			sclbuff->text(SCLstring);

			
			if(x%10 == 0)
			{
				SCLchart->insert(1, SCLVALUE, SCLstring, 2); // "" is the String associated with the data
				SCLchart->bounds(min, max);

				HRchart->insert(1, HR_VALUE, "", 1);
				HRchart->bounds(hr_min,hr_max);
			}
			else
			{
				
				SCLchart->insert(1, SCLVALUE, "", 2); //  itoa(VALUE,buff,10)
				HRchart->insert( 1, HR_VALUE, "", 1);
			}
			x++;
			if(x % MAX_ELEM == 0)
				x = 0;

			dataVector.push_back(SCLVALUE);
			//Sleep(100);

		}
	}


	LightStoneManagerShutdown();
	writeSCLdata(dataVector);
	return Fl::run();  
} 
