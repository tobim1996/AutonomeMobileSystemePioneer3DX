#include "stdafx.h"
#include "Thread.h"
#include "PioneerBV.h"
#include "PioneerBVDlg.h"
#include "Image.h"
#include "cv.hpp"
#include "Shared_Memory.h"
#include <math.h>
#include <cmath>

using namespace cv;
using namespace std;

extern volatile bool bEndThread;	// Wunsch der GUI an den Thread, 
// sich zu beenden

double dThreadFrameCount = 0.0;		// Bestimmung der Framerate

// Anzahl der Hilfsbilder
#define ANZ_BILD 2
int iBild_Index = 4;			// Standardanzeige = gegrabbtes Bild



// Farbkan‰le
#define ROT	2
#define GRUEN	1
#define BLAU	0

// Subtraktion zweier Farbkan‰le: Fkt: Choose_Diff
int minuend=ROT, subtrahend=GRUEN;

// Init der Werte-Felder
int iValue_0 = 0;
int iValue_1 = 0;
int iValue_2 = 0;
int iValue_3 = 5;
int iValue_4 = 0;
int iValue_5 = 0;

// Namen der Variablenfelder
#define FIELD_VALUE_0 0
#define FIELD_VALUE_1 1
#define FIELD_VALUE_2 2
#define FIELD_VALUE_3 3
#define FIELD_VALUE_4 4
#define FIELD_VALUE_5 5

void zeichne_kreuz(long x,long y, long breite, CImage *pImage) ;

/* Die Funktion InitButtons initialisiert die angezeigten Texte 
und Werte der Variablenfelder, sowie die Beschriftung der OwnButtons */
void InitButtons()
{
	// Zeiger auf GUI besorgen
	CPioneerBVDlg* pDlg = ((CPioneerBVApp*)AfxGetApp())->GetDialog();

	// Init-Texte der OwnButtons und ihrer Werte
	pDlg->SetButtonDesc( 0, _T("R-G" ));
	minuend=ROT, subtrahend=GRUEN;

	pDlg->SetButtonDesc( 1, _T("Original" ));
	iBild_Index = ANZ_BILD;

	// Init-Texte und -Werte der Variablenfelder
	pDlg->SetFieldValue( FIELD_VALUE_0, iValue_0);
	pDlg->SetFieldDesc( FIELD_VALUE_0, _T("bv1 (x)") );

	pDlg->SetFieldValue( FIELD_VALUE_1, iValue_1 );
	pDlg->SetFieldDesc( FIELD_VALUE_1, _T("bv2 (y)" ));

	pDlg->SetFieldValue( FIELD_VALUE_2, iValue_2 );
	pDlg->SetFieldDesc( FIELD_VALUE_2, _T("bv3 (flag)" ));

	pDlg->SetFieldValue( FIELD_VALUE_3, iValue_3 );
	pDlg->SetFieldDesc( FIELD_VALUE_3, _T("Filterparm" ));

	pDlg->SetFieldValue( FIELD_VALUE_4, iValue_4 );
	pDlg->SetFieldDesc( FIELD_VALUE_4, _T("Schwelle" ));

	pDlg->SetFieldValue( FIELD_VALUE_5, iValue_5 );
	pDlg->SetFieldDesc( FIELD_VALUE_5, _T("Rahmendicke" ));
}


/* Die Funktion OnButton wird mit dem Index des gedrueckten
Buttons gerufen, hierbei handelt es sich um die +/- - Buttons
der Variablenfelder, die Variablenwerte werden dabei in der Regel 
auf [0..255] beschr‰nkt  */
void OnButton( CPioneerBVDlg* pDlg, int iButtonIndex )
{
	switch( iButtonIndex )
	{
		// Wert 0 wird ge‰ndert
	case 0:
		if (iValue_0 < 255)
		{	iValue_0++;
		pDlg->SetFieldValue( FIELD_VALUE_0, iValue_0 );
		}
		break;
	case 1:
		if (iValue_0 > 0)
		{	iValue_0--;
		pDlg->SetFieldValue( FIELD_VALUE_0, iValue_0 );
		}
		break;

		// Wert 1 wird ge‰ndert
	case 2:
		if (iValue_1 < 255)
		{	iValue_1++;
		pDlg->SetFieldValue( FIELD_VALUE_1, iValue_1 );
		}
		break;
	case 3:
		if (iValue_1 > 0)
		{	iValue_1--;
		pDlg->SetFieldValue( FIELD_VALUE_1, iValue_1 );
		}
		break;

		// Wert 2 wird ge‰ndert
	case 4:
		if (iValue_2 < 255)
		{	iValue_2++;
		pDlg->SetFieldValue( FIELD_VALUE_2, iValue_2 );
		}
		break;
	case 5:
		if (iValue_2 > 0)
		{	iValue_2--;
		pDlg->SetFieldValue( FIELD_VALUE_2, iValue_2 );
		}
		break;

		// Wert 3 wird ge‰ndert
	case 6:
		if (iValue_3 < 255)
		{	iValue_3++;
		pDlg->SetFieldValue( FIELD_VALUE_3, iValue_3 );
		}
		break;
	case 7:
		if (iValue_3 > 0)
		{	iValue_3--;
		pDlg->SetFieldValue( FIELD_VALUE_3, iValue_3 );
		}
		break;

		// Wert 4 wird ge‰ndert
	case 8:
		if (iValue_4 < 255)
		{	iValue_4++;
		pDlg->SetFieldValue( FIELD_VALUE_4, iValue_4 );
		}
		break;
	case 9:
		if (iValue_4 > 0)
		{	iValue_4--;
		pDlg->SetFieldValue( FIELD_VALUE_4, iValue_4 );
		}
		break;

		// Wert 5 wird ge‰ndert
	case 10:
		if (iValue_5 < 255)
		{	iValue_5++;
		pDlg->SetFieldValue( FIELD_VALUE_5, iValue_5 );
		}
		break;
	case 11:
		if (iValue_5 > 0)
		{	iValue_5--;
		pDlg->SetFieldValue( FIELD_VALUE_5, iValue_5 );
		}
		break;
	}
}


/* Die Funktion Choose_Diff kann von einem OwnButton gerufen werden 
und schaltet die globalen Variablen minuend, subtrahend um. Die 
Belegung der Variablen kann im Thread zur Binarisierung mittels
einer Farbkanal-Differenz benutzt werden */

void Choose_Diff(CPioneerBVDlg* pDlg, int OwnButton) {
	// zwischen den 6 mˆglichen Farbanteildifferenzen zirkulieren
	// (R-G, R-B, G-R, G-B, B-R, B-G)

	if ((minuend == ROT) && (subtrahend == GRUEN)) {
		minuend = ROT;
		subtrahend = BLAU;
		pDlg->Say(_T("Differenzbild: Rot-Blau"));
		pDlg->SetButtonDesc(OwnButton, _T("R - B"));
	}
	else if ((minuend == ROT) && (subtrahend == BLAU)) {
		minuend = GRUEN;
		subtrahend = ROT;
		pDlg->Say(_T("Differenzbild: Gr¸n-Rot"));
		pDlg->SetButtonDesc(OwnButton, _T("G - R"));
	}
	else  if((minuend == GRUEN) && (subtrahend == ROT)) {
		minuend = GRUEN;
		subtrahend = BLAU;
		pDlg->Say(_T("Differenzbild: Gr¸n-Blau"));
		pDlg->SetButtonDesc(OwnButton, _T("G - B"));
	}
	else if ((minuend == GRUEN) && (subtrahend == BLAU)) {
		minuend = BLAU;
		subtrahend = ROT;
		pDlg->Say(_T("Differenzbild: Blau-Rot"));
		pDlg->SetButtonDesc(OwnButton, _T("B - R"));
	}
	else if ((minuend == BLAU) && (subtrahend == ROT)) {
		minuend = BLAU;
		subtrahend = GRUEN;
		pDlg->Say(_T("Differenzbild: Blau-Gr¸n"));
		pDlg->SetButtonDesc(OwnButton, _T("B - G"));
	}
	else  if((minuend == BLAU) && (subtrahend == GRUEN)) {
		minuend = ROT;
		subtrahend = GRUEN;
		pDlg->Say(_T("Differenzbild: Rot-Gr¸n"));
		pDlg->SetButtonDesc(OwnButton, _T("R - G"));
	}
}

void Choose_Image(CPioneerBVDlg* pDlg, int OwnButton) {
	// schaltet die Anzeige der Bilder um
	// 0,1 .. ANZ-BILD-1, GrabBild
	TCHAR sStr[100];

	// Umschalten
	iBild_Index++;
	if (iBild_Index >ANZ_BILD) iBild_Index = 0;

	// Ausgabe
	if (iBild_Index == ANZ_BILD) {
		pDlg->Say(_T("Anzeige Originalbild"));
		pDlg->SetButtonDesc(OwnButton, _T("Original"));
	}
	else {
		_stprintf_s(sStr, 100, _T("Anzeige Bild %d"), iBild_Index);
		pDlg->Say(sStr);
		_stprintf_s(sStr, 100, _T("Bild %d"), iBild_Index);

		pDlg->SetButtonDesc(OwnButton,sStr);
	}
}

/* Die Funktion OnOwnButton wird mit dem Index des 
gedr¸ckten OwnButtons gerufen */

void OnButtonOwn( CPioneerBVDlg* pDlg, int iButtonIndex )
{
	switch( iButtonIndex )
	{
		// Knopf 0 gedrueckt
	case 0:
		//pDlg->Say("Knˆpche 0");
		Choose_Diff(pDlg,0);
		break;
		// Knopf 1 gedrueckt
	case 1:
		//pDlg->Say("Knˆpche 1");
		Choose_Image(pDlg,1);
		break;
		// Knopf 2 gedrueckt
	case 2:
		pDlg->Say(_T("Knˆpche 2"));
		break;
		// Knopf 3 gedrueckt
	case 3:
		pDlg->Say(_T("Knˆpche 3"));
		break;
		// Knopf 4 gedrueckt
	case 4:
		pDlg->Say(_T("Knˆpche 4"));
		break;
	}
}

/* HIER wird gearbeitet ! */

UINT WorkerThread( LPVOID pParam ){ 
	HWND hWnd = (HWND)pParam;
	dThreadFrameCount = 0.0;

	Shared_Memory * shared_mem = new Shared_Memory();
	shared_mem->SM_Init();
	shared_mem->SM_SetFloat(3,0);
	double PI = 4.0*atan(1.0);
	TCHAR sStr[100];		// String f¸r Ausgabe 

	int xmax,ymax;		// Auflˆsung des Bildes	

	//Threshold values HSV
	int iLowH = 95;
	int iHighH = 130;

	int iLowS = 15;
	int iHighS = 255;

	int iLowV = 55;
	int iHighV = 255;

	CPioneerBVDlg* pDlg;	// GUI
	CCamera* pCamera;		
	CImage* pImage;			// Zeiger auf das Image der Camera
	CImage aImage[ANZ_BILD];// Array mit vollst‰ndigen Bildern
	IplImage *p,*ph[ANZ_BILD];	// pIplImage, um ipl-Funktionen zu nutzen
	//Mat mat_p,mat_ph[ANZ_BILD];	// pMat, f¸r zugriff mit OpenCV2-funktionen, die Mat benˆtigen

	// Holen des GUI-Zeigers
	pDlg = ((CPioneerBVApp*)AfxGetApp())->GetDialog();

	InitButtons();

	// Holen der Camera und des Bildes der Camera
	pCamera = pDlg->GetCamera();
	//pCamera ->Start();

	// Viermal ein Bild holen, damit CImage seine beiden Bilder auf die eventuell neue Auflˆsung umstellen kann
	pImage = pCamera->GetFrame();	pCamera->ReleaseFrame( pImage ); 
	pImage = pCamera->GetFrame();	pCamera->ReleaseFrame( pImage ); 
	pImage = pCamera->GetFrame();	pCamera->ReleaseFrame( pImage ); 
	pImage = pCamera->GetFrame();	pCamera->ReleaseFrame( pImage ); 

	p = pImage->GetImage();

	// Erstelle ein Bild vom Datentyp Mat mit den gleichen Dimensionen wie das IplImage
	Mat mat_p = cvarrToMat(p);
	Mat mat_ph[ANZ_BILD];

	// Erzeugen gleichgroﬂer Bildkopien und ihrer pIplImage (Zeiger auf Ipl-Bild)
	for(int b=0;b<ANZ_BILD;b++) {
		aImage[b].Create( pImage->Width(), pImage->Height(), 24 );
		ph[b] = aImage[b].GetImage();		
		// Konvertiere Bild-Metadaten von *IplImage zu cv::Mat, Bilddaten sind gemeinsam! (siehe Operator IplImage der Klasse Mat)
		mat_ph[b] = cvarrToMat(ph[b]);
	}

	// Erstelle eine Bild vom Datentyp Mat mit den gleichen Dimensionen wie das IplImage mit Kopie des Bildinhaltes
	// Mat mat_image2(p,true); // hier nicht

	_stprintf_s(sStr, 100, _T("%d %dx%d Bilder angelegt ..."), ANZ_BILD, p->width, p->height);
	pDlg->Say(sStr);

	pCamera->ReleaseFrame( pImage ); 

	pDlg->Say(_T("Thread gestartet ..."));

	// und los ...
	while( !bEndThread ){

		//variables to save values of biggest circle
		int maxCircle = -1;
		double maxArea = 0;
		Point maxCenter;
		int maxRadius;

		Mat hsv, mask;
		vector<Vec3f> circles;

		// Holen aktuelles Bild
		pImage = pCamera->GetFrame();		// Zeiger auf CImage
		p = pImage->GetImage();				// Zeiger auf IplImage
		// Konvertiere Bild-Metadaten von *IplImage zu cv::Mat, Bilddaten sind gemeinsam
		// Konvertierung erst in der Hauptschleife, da p durch das Grabben zwischen zwei Bildpuffern wechselt

		mat_p = cvarrToMat(p); 

		// Auflˆsung des Bildes bestimmen
		xmax = pImage->Width();
		ymax = pImage->Height();

		cvtColor(mat_p,hsv, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		inRange(hsv, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), mask); //Threshold the image

		//morphological opening (removes small objects)
		erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(10, 10)));
		dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(10, 10)));

		//morphological closing (removes small holes)
		dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		GaussianBlur(mask, mask, cv::Size(9, 9), 2, 2);

		//find circles in mask (greyscale image)
		HoughCircles(mask, circles, CV_HOUGH_GRADIENT, 1, mask.rows / 4, 200, 15, 0, 0);

		//check which of the detected circles is biggest
		for (uint i = 0; i < circles.size(); i++)
		{
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			double area = PI * radius * radius;
			if (area > maxArea){
				//current circle is bigger than the currently biggest one (or the first to be checked)
				maxArea = area;
				maxCircle = i;
				maxCenter = center;
				maxRadius = radius;
			}
		}

		cvtColor(mask, mat_ph[0], COLOR_GRAY2BGR);

		if (maxCircle != -1 && maxArea > 500){
			//we have found at least 1 circle
			//that circles area is > 500 to exclude small fragments
			//-> use it for tracking
			circle(mat_p, maxCenter, maxRadius, Scalar(0, 255, 0));
			shared_mem->SM_SetFloat(1, (float)maxCenter.x - xmax / 2);
			shared_mem->SM_SetFloat(2, (float)maxCenter.y - xmax / 2);
			shared_mem->SM_SetFloat(3, (float)1);
			Sleep(50);
		}
		else
		{
			//no circle found
			shared_mem->SM_SetFloat(3, (float)0);
		}

		// Das gew¸nschte Bild anzeigen
		if (iBild_Index == ANZ_BILD) pDlg->ShowImage( pImage );	
		else	pDlg->ShowImage( &(aImage[iBild_Index]));

		// Man muss auch loslassen koennen
		pCamera->ReleaseFrame( pImage );							// Frame freigeben

		dThreadFrameCount += 1;		
	}

	shared_mem->SM_Close();

	::PostMessage( hWnd, WM_WORKER_THREAD_FINISHED, 0, 0 );
	pDlg->Say(_T("Thread gestoppt ..."));
	return 0;
}

void zeichne_kreuz(long x,long y, long breite, CImage *pImage) {
	long i,j,sx,sy,BytePerPixel;
	long size;

	BYTE *pix = (BYTE*)pImage->GetImage()->imageData; 

	sx = pImage->Width();
	sy = pImage->Height();
	BytePerPixel = pImage->Channels();

	size = sx*sy*BytePerPixel;
	// horizontale Linie
	for(i=(y*sx+x-breite)*BytePerPixel+BLAU;i<=(y*sx+x+breite)*BytePerPixel+BLAU;i+=BytePerPixel) 
		if (i>=0  && i< size) pix[i] = 255;
	// vertikale Linie
	for(i=y-breite;i<=y+breite;i++)   {
		j = (i*sx+x)*BytePerPixel+BLAU;
		if (j>=0  && j< size) pix[j] = 255;
	}

	// Zentrum des Kreuzes weiﬂ zeichnen
	pix[(sx*y+x)*BytePerPixel+ROT] = 255;
	pix[(sx*y+x)*BytePerPixel+GRUEN] = 255;
	pix[(sx*y+x)*BytePerPixel+BLAU] = 255;
}
