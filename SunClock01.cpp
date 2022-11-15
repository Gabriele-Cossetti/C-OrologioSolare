/*-----------------------------------------------------------
   RESOURC2.C -- Icon and Cursor Demonstration Program No. 2
                 (c) Charles Petzold, 1996
  -----------------------------------------------------------*/

#include <windows.h>
#include <math.h>
#include <time.h>

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

char      szAppName[] = "Immagine01" ;
HINSTANCE hInst ;

const double DegRad = 0.017453292;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
     {
     HBITMAP    hBitmap ;
     HBRUSH     hBrush ;
     HWND       hwnd ;
     MSG        msg ;
     WNDCLASSEX wndclass ;

     hBitmap = LoadBitmap (hInstance, szAppName) ;
     hBrush = CreatePatternBrush (hBitmap) ;

     wndclass.cbSize        = sizeof (wndclass) ;
     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (hInstance, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (hInstance, IDC_ARROW) ;
     wndclass.hbrBackground = hBrush ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;
     wndclass.hIconSm       = LoadIcon (hInstance, szAppName) ;

     RegisterClassEx (&wndclass) ;

     hInst = hInstance ;

     hwnd = CreateWindow (szAppName, "Orologio Solare",
                          WS_OVERLAPPEDWINDOW & ~ (WS_THICKFRAME) & ~ (WS_MAXIMIZEBOX),
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          416, 238,
                          NULL, NULL, hInstance, NULL) ;

     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;

     while (GetMessage (&msg, NULL, 0, 0))
          {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
          }
     return msg.wParam ;
     }



double intToLat(int i,int h) {
	return 90.0-((double) i)/((double)h)*180.0;
}

int latToInt(int h,double lat) {
	return -((int)(((lat-90.0)/180.0)*((double)h)));
}

double intToLon(int i,int w) {
	return 180-((double) i)/((double)w)*360.0;
}

int lonToInt(int w,double lon) {
	return -((int)(((lon-180.0)/360.0)*((double)w)));
}

double sunVal(double sunLat,double sunLon,double pLat, double pLon) {
	double sX = cos(sunLat*DegRad)*cos(sunLon*DegRad);
	double sY = cos(sunLat*DegRad)*sin(sunLon*DegRad);
	double sZ = sin(sunLat*DegRad);
	double pX = cos(pLat*DegRad)*cos(pLon*DegRad);
	double pY = cos(pLat*DegRad)*sin(pLon*DegRad);
	double pZ = sin(pLat*DegRad);
	return sX*pX+sY*pY+sZ*pZ;
}

void DrawOmbra (HDC hdc)
{
			// shadow drawing ombra
		time_t ltime;
		time( &ltime );
			int nightfilter = 50;       // (1 <= nightfilter <= 100 ; scurezza della zona d'ombra)
			int h = 200;                // (altezza inferiore della zona d'ombra - altezza della finestra)
			int w = 400;                // (larghezza della finestra)
			// --------------------------------- sun position
			double T = ((((double)ltime)/86400.0)-10957.5)/36525.0;		//secoli dal 1/1/2000
			double M = 357.52910+35999.05030*T-0.0001559*T*T-0.00000048*T*T*T;		// mean anomaly, degree 
			double L0 = 280.46645+36000.76983*T+0.0003032*T*T;		// mean longitude, degree 
			double DL = (1.914600-0.004817*T-0.000014*T*T)*sin(DegRad*M)+(0.019993-0.000101*T)*sin(DegRad*2*M)+0.000290*sin(DegRad*3*M);
			double L = L0 + DL;		 // true longitude, degree
			double eps = 23.43999;		// obliquity of ecliptic
			double X = cos(L*DegRad);
			double Y = cos(eps*DegRad)*sin(L*DegRad);
			double Z = sin(eps*DegRad)*sin(L*DegRad);
			double R = sqrt(1.0-Z*Z);
			double delta = atan2(Z,R)/DegRad;
			double RA = atan2(Y,(X+R))/DegRad;
			double JD = (T*36525.0)+2451545.0;
			double lat = delta;
			double lon = (fmod(ltime,86400.0)-43200.0)*0.004166666;
			double theta0 = lon+RA;
			lat = lat/180.0;
			lat = lat - floor(lat);
			if(lat<0.0) lat += 1.0;
			lat = lat*180.0;
			if(lat>90.0) lat -= 180.0;
			// ----------------------------------end sun position
			lon += 14;   // correttore bordo finestra
			// ----------------------------------
			long int v;
			long int r,g,b;
			long int mr,mg,mb;
			// nightside colors and masks
			long int n_maskrg = 0x154;
			long int n_maskb = 0xff;
			n_maskrg = min(n_maskrg*nightfilter/100,0xff);
			n_maskb = n_maskb*nightfilter/100;
			long int n_mr = n_maskrg;
			long int n_mg = n_maskrg<<8;
			long int n_mb = n_maskb<<16;
			//dayside colors and masks
			long int d_mr = 0x10;
			long int d_mg = 0x1000;
			long int d_mb = 0x130000;
			for(int i = 0; i<h;++i) {
				double pLat = intToLat(i,h);
				for(int j = 0;j<w;++j) {
					double pLon = intToLon(j,w);
					double a = sunVal(lat,lon,pLat,pLon);
					v = GetPixel(hdc,j,i);
					r = v&0xff;
					g = v&0xff00;
					b = v&0xff0000;
					if(a <0.0) {
						//nightside
						a = pow(-a,0.4);
						mr = ((int)((double)n_mr*a)&0xff);
						mg = ((int)((double)n_mg*a)&0xff00);
						mb = ((int)((double)n_mb*a)&0xff0000);
						if(r > mr) r -= mr;
							else r = 0;
						if(g > mg) g -= mg;
							else g = 0;
						if(b > mb) b -= mb;
							else b = 0;
					}
					else {
						//dayside
						a = pow(a,9);
						mr = ((int)((double)d_mr*a)&0xff);
						mg = ((int)((double)d_mg*a)&0xff00);
						mb = ((int)((double)d_mb*a)&0xff0000);
						if((r+mr)<0xff) r += mr;
							else r = 0xff;
						if((g+mg)<0xff00) g += mg;
							else g = 0xff00;
						if((b+mb)<0xff0000) b += mb;
							else b = 0xff0000;
					}
					v = r|g|b;
					SetPixel(hdc,j,i,v);
				}
			}
			// end shadow drawing ombra
}

void DrawSole (HDC hdc)
{
			// shadow drawing sole
			time_t ltime;
			time( &ltime );
			long int oldtime =(long int) ltime;
			int h = 232;                // (altezza della finestra)
			int w = 400;                // (larghezza della finestra)
	//		int bh = 232;
			int bw = 400;
			int scale = 50;             // (1 <= scale <= 100)
			bool haschanged = true;
			int span = 300000;
			static bool f = true;
			if((ltime>=(oldtime+span/1000))||(ltime<=(oldtime-span/1000))||haschanged){
			oldtime = (long int) ltime;
			// calculate sun position
			double T = ((((double)ltime)/86400.0)-10957.5)/36525.0;		//secoli dal 1/1/2000
			double M = 357.52910+35999.05030*T-0.0001559*T*T-0.00000048*T*T*T;		// mean anomaly, degree 
			double L0 = 280.46645+36000.76983*T+0.0003032*T*T;		// mean longitude, degree 
			double DL = (1.914600-0.004817*T-0.000014*T*T)*sin(DegRad*M)+(0.019993-0.000101*T)*sin(DegRad*2*M)+0.000290*sin(DegRad*3*M);
			double L = L0 + DL;		 // true longitude, degree
			double eps = 23.43999;		// obliquity of ecliptic
			double X = cos(L*DegRad);
			double Y = cos(eps*DegRad)*sin(L*DegRad);
			double Z = sin(eps*DegRad)*sin(L*DegRad);
			double R = sqrt(1.0-Z*Z);
			double delta = atan2(Z,R)/DegRad;
			double RA = atan2(Y,(X+R))/DegRad;
			double JD = (T*36525.0)+2451545.0;
			double lat = delta;
			double lon = (fmod(ltime,86400.0)-43200.0)*0.004166666;
			double theta0 = lon+RA;
			lat = lat/180.0;
			lat = lat - floor(lat);
			if(lat<0.0) lat += 1.0;
			lat = lat*180.0;
			if(lat>90.0) lat -= 180.0;
			// end sun position
				// drawing sun
				BITMAP BMP;
				HBITMAP hSunBitmap = 0;
				HBITMAP hSunMaskBitmap = 0;
				hSunBitmap = (HBITMAP)LoadImage( NULL, "sun40.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				hSunMaskBitmap = (HBITMAP)LoadImage( NULL, "sunmask.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				GetObject(hSunBitmap, sizeof(BMP), &BMP);
				HDC BMPDC = CreateCompatibleDC(NULL);
				SelectObject(BMPDC, hSunBitmap);
				int sunx = lonToInt(w,lon) - (BMP.bmWidth/2*0.8);    //con correzione fattore di forma finestra
				int suny = latToInt(h,lat) - (BMP.bmHeight/2*0.7);   //con correzione fattore di forma finestra
				SetStretchBltMode(hdc, HALFTONE);
				int sunsize = ((int)(((double)((((BMP.bmWidth+BMP.bmHeight)/2)*0.6)*scale*bw))/16000.0));
				SelectObject(BMPDC, hSunMaskBitmap);
				StretchBlt(hdc, sunx-(sunsize/2),suny-(sunsize/2),sunsize,sunsize,BMPDC,0,0,40,40,SRCAND);
				SelectObject(BMPDC, hSunBitmap);
				StretchBlt(hdc, sunx-(sunsize/2),suny-(sunsize/2),sunsize,sunsize,BMPDC,0,0,40,40,SRCPAINT);
				// fine tracciamento Sole
				}
}

void DrawLuna (HDC hdc)
{
				time_t ltime;
				time( &ltime );
				int h = 232;                // (altezza della finestra)
				int w = 400;                // (larghezza della finestra)
				int bw = 400;
				int scale = 50;             // (1 <= scale <= 100)
				double eps = 23.43999;		// obliquity of ecliptic
				double T = ((((double)ltime)/86400.0)-10957.5)/36525.0;		//secoli dal 1/1/2000
				double M = 357.52910+35999.05030*T-0.0001559*T*T-0.00000048*T*T*T; // mean anomaly, degree 
				double lon = (fmod(ltime,86400.0)-43200.0)*0.004166666;
				double L0 = 280.46645+36000.76983*T+0.0003032*T*T;		// mean longitude, degree 
				double DL = (1.914600-0.004817*T-0.000014*T*T)*sin(DegRad*M)+(0.019993-0.000101*T)*sin(DegRad*2*M)+0.000290*sin(DegRad*3*M);
				double L = L0 + DL;		 // true longitude, degree
				double X = cos(L*DegRad);
				double Y = cos(eps*DegRad)*sin(L*DegRad);
				double Z = sin(eps*DegRad)*sin(L*DegRad);
				double R = sqrt(1.0-Z*Z);
				double RA = atan2(Y,(X+R))/DegRad;
				// ---------calcolo posizione Luna
				double d = T*36525.0;				
				double MN = 125.1228-0.0529538083*d;
				double Mi = 5.1454;
				double Mw = 318.00634+0.1643573223*d;
				double Ma = 60.2666;
				double Me = 0.054900;
				double MM = 115.3654+13.0649929509*d;
				double ME = MM+Me/DegRad*sin(MM*DegRad)*(1.0+Me*cos(MM*DegRad));
				double Mxv = Ma*(cos(ME*DegRad)-Me);
				double Myv = Ma*(sqrt(1.0-Me*Me)*sin(ME*DegRad));
				double Mv = atan2(Myv,Mxv)/DegRad;
				double Mr = sqrt(Mxv*Mxv+Myv*Myv);
				double Mxh = Mr*(cos(MN*DegRad)*cos((Mv+Mw)*DegRad)-sin(MN*DegRad)*sin((Mv+Mw)*DegRad)*cos(Mi*DegRad));
				double Myh = Mr*(sin(MN*DegRad)*cos((Mv+Mw)*DegRad)+cos(MN*DegRad)*sin((Mv+Mw)*DegRad)*cos(Mi*DegRad));
				double Mzh = Mr*(sin((Mv+Mw)*DegRad)*sin(Mi*DegRad));
				double Mxe = Mxh;
				double Mye = Myh*cos(eps*DegRad)-Mzh*sin(eps*DegRad);
				double Mze = Myh*sin(eps*DegRad)+Mzh*cos(eps*DegRad);
				double MRA = atan2(Mye,Mxe)/DegRad;
				double MDec = atan2(Mze,sqrt(Mxe*Mxe+Mye*Mye))/DegRad;
				double mlon = lon+RA-MRA;
				double mlat = MDec;
				mlon = mlon/360.0;
				mlon = mlon - floor(mlon);
				if(mlon<0.0) mlon += 1.0;
				mlon = mlon*360.0;
				if(mlon>180.0) mlon -= 360.0;
				mlat = mlat/180.0;
				mlat = mlat - floor(mlat);
				if(mlat<0.0) mlat += 1.0;
				mlat = mlat*180.0;
				if(mlat>90.0) mlat -= 180.0;
				// end moon position
				// drawing moon ----------------------------------------
				BITMAP BMP;
				HBITMAP hFullMoonBitmap = 0;
				HBITMAP hNewMoonBitmap = 0;
				HBITMAP hMoonMaskBitmap = 0;
				HBITMAP hFirstMoonBitmap = 0;
				HBITMAP hLastMoonBitmap = 0;
				hFullMoonBitmap = (HBITMAP)LoadImage( NULL, "moon20.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				hNewMoonBitmap = (HBITMAP)LoadImage( NULL, "newmoon1.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				hMoonMaskBitmap = (HBITMAP)LoadImage( NULL, "newmoon.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				hFirstMoonBitmap = (HBITMAP)LoadImage( NULL, "firstmoon.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				hLastMoonBitmap = (HBITMAP)LoadImage( NULL, "bmp00001.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
				GetObject(hFullMoonBitmap, sizeof(BMP), &BMP);
				int moonx = lonToInt(w,mlon) - (BMP.bmWidth/2*1.6);     //con correzione fattore di forma finestra
				int moony = latToInt(h,mlat) - (BMP.bmHeight/2*1.6);    //con correzione fattore di forma finestra
				int moonsize = ((int)(((double)((((BMP.bmWidth+BMP.bmHeight)/2)*0.6)*scale*bw))/16000.0));
				HDC BMPDC = CreateCompatibleDC(NULL);
				SelectObject(BMPDC, hMoonMaskBitmap);
				SetStretchBltMode(hdc, HALFTONE);
				StretchBlt(hdc, moonx-(moonsize/2),moony-(moonsize/2),moonsize,moonsize,BMPDC,0,0,20,20,SRCAND);
				// moon phase to select the right bitmap
				double phase = lon-mlon;
				if(phase>180.0) phase -= 360.0;
				else if(phase<-180.0) phase += 360.0;
				if(phase<-135.0) SelectObject(BMPDC, hFullMoonBitmap);
				else if(phase<-45) SelectObject(BMPDC, hLastMoonBitmap);
				else if(phase<45) SelectObject(BMPDC, hNewMoonBitmap);
				else if(phase<135) SelectObject(BMPDC, hFirstMoonBitmap);
				else SelectObject(BMPDC, hFullMoonBitmap);
				StretchBlt(hdc, moonx-(moonsize/2),moony-(moonsize/2),moonsize,moonsize,BMPDC,0,0,20,20,SRCPAINT);
				// Fine tracciamento Luna
}
LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
     {
     HDC          hdc ;
     PAINTSTRUCT  ps ;

     switch (iMsg)
          {
          case WM_CREATE :
				// To do
               return 0 ;

          case WM_SIZE :
				// To do
               return 0 ;

          case WM_PAINT :
               hdc = BeginPaint (hwnd, &ps) ;

			   DrawOmbra (hdc);
			   DrawSole (hdc);
			   DrawLuna (hdc);

               EndPaint (hwnd, &ps) ;
               return 0 ;

          case WM_DESTROY :
               PostQuitMessage (0) ;
               return 0 ;
          }
     return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
     }
