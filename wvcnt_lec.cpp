// wvcnt_lec.cpp : wavelength tuning software

// For Continuum seeder 2011 version (The Rock)
// Piezo Voltage & Heater handled
// Piezo : controlling the wavelength shift of the narrow band etalon (corresponds to "TNB" for the previous version)  
// Heater: controlling the wavelength shift of the wide band FBG (corresponds to "TWB" for the previous version)

// Heater folder program : Control Heater temperature (Piezo V is fixed) : Use this program for the Rock
// PiezoV folder program : Control Piezo V (Heater temp. is fixed)

// Note:
// Seeder wavelength increases as Heater temp. [HT] increases.
//                             as Piezo V. [PV] decreases.
// This program treats: Wavelength increases: Forward (i.e., HT (PV) increases (decreases))
//                                 decreases: Backward (i.e., HT (PV) decreases (indcreases))            

// This program controls Heater temperature

// 2011 Heater version v1 : control heater temp & Piezo (as function of heater temp)		
//                     v2 : automatically tuning function: search a center wavelength of the iodine absorption line
//                          switch "t". After finding the best position, set switch "g" 

// ch1 -> pp
// ch2 -> pcp
// ch3 -> pcm
// ch4 -> pm

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include "Decl-32.h"
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <time.h>
#include "erslib.h"
#include <signal.h>
#include <visa.h>
#include <memory.h>
using namespace std;

int flag;
struct tm *tm_now;
time_t now;

static float pp, pm, pc, pcp, pcm;
char tfname[20], dayname[20], atimefile[30], adirname[30];

int Dev;
int        Num_Instruments,            // Number of instruments on GPIB
	PAD,                        // Primary address
	SAD,                        // Secondary address
	loop;                       // Loop counter
Addr4882_t Instruments[32],            // Array of primary addresses
	Result[31];                 // Array of listen addresses
char ErrorMnemonic[21][5] = {"EDVR", "ECIC", "ENOL", "EADR", "EARG",
	"ESAC", "EABO", "ENEB", "EDMA", "",
	"EOIP", "ECAP", "EFSO", "", "EBUS",
	"ESTB", "ESRQ", "", "", "", "ETAB"};

void sig_int(int)
{
	printf("Push Cntl-C!!\n");
	flag =0;
}

void getparam(char *dayname, char *tfname, short *prmtab)
{
	time(&now);
	tm_now = gmtime(&now);
	strftime(tfname, 11, "%y%m%d%H%M", tm_now);
	strftime(dayname, 7, "%y%m%d", tm_now);
	prmtab[0] = 25;
	prmtab[1] = 10;
	prmtab[2] = 100;
	prmtab[3] = tm_now->tm_year-100;
	prmtab[4] = tm_now->tm_mon+1;
	prmtab[5] = tm_now->tm_mday;
	prmtab[6] = tm_now->tm_hour;
	prmtab[7] = tm_now->tm_min;
	prmtab[8] = tm_now->tm_sec;
	prmtab[9] = 00;
	return;
}

void plotterN(FILE* gp, float x[], float y[], float y2[], float y3[], float y4[], float y5[], float y6[], int n)
{
	int i;

	// Set Gnu parameters

	fprintf(gp, "unset key\n");
	fprintf(gp, "set multiplot\n");
	fprintf(gp, "set size nosquare\n");
	fprintf(gp, "set size 1, 0.5\n");

	// Upper figure
	fprintf(gp, "set origin 0, 0.5\n");
	//fprintf(gp, "set title 'Red=HeaterHOLA, Green=PiezoV*5, Blue=50.0+3.0*prt'\n");
	fprintf(gp, "set title 'Red=log10(p+/p-)'\n");
	fprintf(gp, "set xrange [61.5:63.5]\n"); //1st HSRL
	fprintf(gp, "set yrange [-1.5:1.5]\n");
	fprintf(gp, "set ytics  0.5\n");
	//fprintf(gp, "set xrange [59.0:59.5]\n"); //2nd HSRL
	//fprintf(gp, "set xrange [58.0:59.5]\n"); //2nd HSRL - 1111 and 1110	
	fprintf(gp, "plot '-' \n");
	for(i=0;i<n;i++)
		fprintf(gp, "%f %f\n", y[i], (log10)(y3[i]/y6[i]));
	fprintf(gp, "end\n");

	// Lower figure
	fprintf(gp, "set origin 0, 0.0\n");
	fprintf(gp, "set title 'Magenta=P+, Green=PC+, Blue=PC-, Orange=P-'\n");
	fprintf(gp, "set xrange [61.5:63.5]\n"); //1st HSRL
	//fprintf(gp, "set xrange [1.0:6.0]\n");
	fprintf(gp, "set xtics 1\n");
	fprintf(gp, "set mxtics 5\n");
	fprintf(gp, "set ytics  0.5\n");
	fprintf(gp, "set mytics 5\n");
	fprintf(gp, "set nologscale y\n");
	//fprintf(gp, "set yrange [50:60]\n");  
	//fprintf(gp, "set yrange [-1.75:1.75]\n");  

	/*
	fprintf(gp, "plot '-', '-', '-' \n");
	for(i=0;i<n;i++)
	fprintf(gp, "%f %f\n", y[i], y[i]);
	fprintf(gp, "end\n");
	for(i=0;i<n;i++)
	fprintf(gp, "%f %f\n", y[i], y2[i]);
	//fprintf(gp, "%f %f\n", y2[i], 50.0+y2[i]);
	fprintf(gp, "end\n");
	*/
	//fprintf(gp, "set xrange [59.0:59.5]\n"); //2nd HSRL
	//fprintf(gp, "set xrange [58.0:59.5]\n"); //2nd HSRL 1111 and 1110
	//fprintf(gp, "set xrange [1.0:6.0]\n");

	fprintf(gp, "set xtics 1\n");
	fprintf(gp, "set mxtics 5\n");
	fprintf(gp, "set ytics 1000.0\n");
	fprintf(gp, "set mytics 2\n");
	fprintf(gp, "set nologscale y\n");
	//fprintf(gp, "set logscale y\n"); 
	fprintf(gp, "set yrange [-100:3000]\n");  
	fprintf(gp, "plot '-', '-', '-', '-' \n");
	for(i=0;i<n;i++)
		//fprintf(gp, "%f %f\n", y[i], 20.0*y3[i]);
		//fprintf(gp, "%f %f\n", y2[i], 120.0*y3[i]);
		fprintf(gp, "%f %f\n", y[i], 1000*y3[i]); //channel 1
	fprintf(gp, "end\n");
	for(i=0;i<n;i++)
		fprintf(gp, "%f %f\n", y[i], 1000*y4[i]/5.0); //channel 2
	fprintf(gp, "end\n");
	for(i=0;i<n;i++)
		fprintf(gp, "%f %f\n", y[i], 1000*y5[i]/5.0); //channel 3
	fprintf(gp, "end\n");
	for(i=0;i<n;i++)
		fprintf(gp, "%f %f\n", y[i], 1000*y6[i]); //channel 4
	fprintf(gp, "end\n");

	fflush(gp);
}

int measure()
{

	ViSession rm = VI_NULL, vi = VI_NULL;
	ViStatus status;
	ViChar buffer[256];
	ViUInt32 retCnt;
	
	// Open a default session
	status = viOpenDefaultRM(&rm);
	if (status < VI_SUCCESS) goto error;
	
	// Open the USB device
	status = viOpen(rm,"USB0::0x0699::0x3A4::C042138::INSTR", VI_NULL, VI_NULL,&vi); 
	//USB0::0x0699::0x03A4::C042138::INSTR valor desde la instalacion en PILAR a la fecha 2017.06.15

	if (status < VI_SUCCESS)if (status < VI_SUCCESS){
		printf("\nUSB Open error\nCheck that the Oscilloscope is connected\nand the device nr is\nUSB0::0x0699::0x3A4::C042138::INSTR\n");
		goto error;
	}
		
	// Send an ID query.
	status = viWrite(vi, (ViBuf) "*idn?", 5, &retCnt);
	if (status < VI_SUCCESS){
		printf("\nID query error: Command *idn\n");
		goto error;
	}
	
	
	// Clear the buffer and read the response
	memset(buffer, 0, sizeof(buffer));
	status = viRead(vi, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto error;
	// Print the response
	//printf("id: %s\n", buffer);


	// pp=mean(Ch1) pcp=mean(Ch2) pcm=mean(Ch3) pm=mean(Ch4) //

	status = viWrite(vi, (ViBuf) "SELECT:CH1 ON", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "ACQUIRE:STATE ON", 30, &retCnt);
	//status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE PK2PK", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE MEAN", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:SOURCE CH1", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:VALUE?",30 , &retCnt);
	memset(buffer, 0, sizeof(buffer));
	status = viRead(vi, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto error;
	pp = atof(buffer);


	//7printf("pkpk CH1:	%s\n", buffer);
	//status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE PK2PK", 30, &retCnt);

	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE MEAN", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:SOURCE CH2", 30, &retCnt);
	while(status = viWrite(vi, (ViBuf) "BUSY?", 6, &retCnt));
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:VALUE?",30 , &retCnt);
	memset(buffer, 0, sizeof(buffer));
	status = viRead(vi, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto error;
	pcp = atof(buffer);
	// Print the response
	//printf("pkpk CH2:	 %s\n", buffer);

	//printf("pkpk CH1:	%s\n", buffer);
	//status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE PK2PK", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE MEAN", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:SOURCE CH3", 30, &retCnt);
	while(status = viWrite(vi, (ViBuf) "BUSY?", 6, &retCnt));
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:VALUE?",30 , &retCnt);
	memset(buffer, 0, sizeof(buffer));
	status = viRead(vi, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto error;
	pcm = atof(buffer);
	//pcm = 0.0;
	// Print the response
	//printf("pkpk CH2:	 %s\n", buffer);

	//printf("pkpk CH1:	%s\n", buffer);
	//status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE PK2PK", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:TYPE MEAN", 30, &retCnt);
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:SOURCE CH4", 30, &retCnt);
	while(status = viWrite(vi, (ViBuf) "BUSY?", 6, &retCnt));
	status = viWrite(vi, (ViBuf) "MEASUREMENT:IMMED:VALUE?",30 , &retCnt);
	memset(buffer, 0, sizeof(buffer));
	status = viRead(vi, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto error;
	//pm = atof(buffer) + 0.001;
	pm = atof(buffer);
	//pm = 0.0;
	// Print the response
	//printf("pkpk CH2:	 %s\n", buffer);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Clean up 
	viClose(vi); // Not needed, but makes things a bit more
	// understandable
	viClose(rm); // Closes resource manager and any sessions
	// opened with it

	return 0;
error:
	// Report error and clean up
	viStatusDesc(vi, status, buffer);
	fprintf(stderr, "failure: %s\n", buffer);
	if (rm != VI_NULL) {
		viClose(rm);
	}
	_getch();
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	float temp0, piezoV, heater;

	// TEC1, TEC2, PiezoV set point, Heater, PCB
	char tmon0[]=":syst:tmon0:temp?\r\n";
	char tmon1[]=":syst:tmon1:temp?\r\n";
	char tmon2[]=":syst:pdr:volt?\r\n";
	char tmon3[]=":syst:tmon2:temp?\r\n";
	char tmon4[]=":syst:tmon3:temp?\r\n";

	// Make protected command available, status (1 able, 0 unable)
	char pass[]=":syst:pass:cen \"NP\"\r\n";
	char passchk[]=":syst:pass:cen:stat?\r\n";


	char ocm2[30], ocm3[30];
	char kht1[10], kht2[10];
	FILE *fp;
	FILE* gp = _popen("gnuplot", "w");
	short prmtab[10];
	char buf[20], buf_[20], c;  
	int comp, nr, nr_, i, j, s2, s3;
	float x[100], y[100], y2[100], y3[100], y4[100], y5[100], y6[100];
	float oc2, oc2d, oc2s, oc2ss, oc2sss, oc3, oc3s, oc3d, oc3ss, oc3sss;
	float oc3min, oc3max;

	// addN
	int igflg;
	float prt, pcd, ocd, slp, pcdabs, ocdabs, prtd, prtdabs, prtR, prtdth;
	float ppb1, pmb1, pcpb1, pcmb1, prtb1, oc2b1, oc3b1;
	float ppb2, pmb2, pcpb2, pcmb2, prtb2, oc2b2, oc3b2;
	float coe;

	// "t" switch
	int itflg;
	float toc3dif, toc3sum, toc3sumrt, tpcmin, toc2min, toc3min, prtdif;

	char gpout[80];

	// Sweep set --------------------
	//char ssw[]= "200"; // SWEEPS
	//  char ssw[]= "150"; // SWEEPS
	//    char ssw[]= "600"; // SWEEPS

	//-------------------------------

	// Piezo (1064.190-1064.220nm)
	//// Volt 2.607, 3.518, 4.428, 5.339 (1064.220, 210, 200, 190nm)

	//oc2 = 4.701; // temp = 54.439
	oc2 = 2;	//oc2 = 5.5;
	//oc2 = -10.0;
	//// Resolution (0.25pm/0.125pm/00625pm at 1064nm)
	// 190pm-220pm
	//  oc2s = 0.02275;
	//  oc2ss= 0.011375;
	//  oc2sss=0.0056875;
	//oc2s = 0.02595;
	oc2s = 0;
	//oc2s = 0.02275;
	oc2ss= 0.00000000;
	oc2sss=0.00000000;

	// Heater (1064.190-1064.220nm)
	//// Temperature 57.706, 56.286, 54.865, 53.445 (1064.220, 210, 200, 190nm)

	//oc3 = 54.51;
	//oc3 = 54.7;
	//oc3 = 54.72; //Nov'17
	oc3 = 62.14; //50.31


	// Resolution (0.25pm/0.125pm/0.0625pm at 1064nm)
	oc3s = 0.069923;
	//oc3s = 0.0355;
	
	//oc3s = 0.060937;
	//oc3s = 0.01775; //For fine scanning
	//oc3s = 0.1;
	
	//oc3s = 0.008875;
	oc3ss= 0.01775;
	//oc3sss=0.008875;
	oc3sss = 0.0044375;

	// Min & Max for safety
	oc3min = 61.3; //1st HSRL
	oc3max = 63.3; //1st HSRL
	//oc3min = 54.7; //1st HSRL
	//oc3max = 55.3; //1st HSRL
	//oc3min = 54.30; //1st HSRL (PRR)
	//oc3max = 55.00; //1st HSRL (PRR)
	//oc3min = 54.00; //1st HSRL (PRR)
	//oc3max = 55.50; //1st HSRL (PRR)
	//oc3min = 53.50; //1st HSRL (PRR)
	//oc3max = 60.50; //1st HSRL (PRR)


	flag = 1;
	signal(SIGINT, sig_int);

	for(i=0;i<95;i++)
		x[i] = i; 

	// COM number for seeder connection (Serial Port)
	comp=5;
	if(ERS_Open(comp,4096,4096) != 0) {
		cout << "Open_RS232C_error" << endl;
		gets(kht1);
		return 1;
	}
	ERS_Config(comp,ERS_57600|ERS_1|ERS_NO|ERS_8|ERS_X_N|ERS_DTR_N|ERS_RTS_N|ERS_CTS_N|ERS_DSR_N);


	///////////////////////////////// 2016.12.14
	printf("%s ",pass); 
	ERS_Send(comp, pass, 21);
	ERS_Send(comp, passchk, 22);
	ERS_Gets(comp, buf, sizeof(buf));
	printf("Pass stat= %s\n",buf);
	ERS_ClearRecv(comp);

	getparam(dayname, tfname, prmtab);
	sprintf(atimefile, "wvcntdata/%s", tfname);
	printf("filename:%s\n",atimefile);
	fp = fopen(atimefile, "w");

	i = 0;
	c = 's';
	pp = 0.0;
	pm = 0.0;
	pcp = 0.0;
	pcm = 0.0;
	ppb1 = 0.0;
	pmb1 = 0.0;
	pcpb1 = 0.0;
	pcmb1 = 0.0;
	prtb1 = 0.0;
	oc2b1 = 0.0;
	oc3b1 = 0.0;
	prt = 0.0;
	itflg = 0;
	coe = 1.2; //previous 1.2
	do{

		// addN security 190-220pm (1064nm)
		if(oc3 > oc3max) c = 'b';
		if(oc3 < oc3min) c = 'f';

		//addN signal data for two step before
		ppb2=ppb1;
		pmb2=pmb1;
		pcpb2=pcpb1;
		pcmb2=pcmb1;
		prtb2=prtb1;
		oc2b2=oc2b1;
		oc3b2=oc3b1;

		//addN signal data for one step before
		ppb1=pp;
		pmb1=pm;
		pcpb1=pcp;
		pcmb1=pcm;
		prtb1=prt;
		oc2b1=oc2;
		oc3b1=oc3;

		// Check switch
		if(kbhit()) 
			c = getch();  // No need Enter key

		// Primary switch
		if(c == 's'){  //stop
			oc2d = 0.0;
			oc3d = 0.0;
		}
		if(c == 'f'){ // forward
			oc2d = -1.0*oc2s;
			oc3d = oc3s;
		}
		if(c == 'b'){ // backwards
			oc2d = oc2s;
			oc3d = -1.0*oc3s;
		}
		if(c == 'e'){  // end
			flag = 0;
		}  

		//addN New switch (g)
		if(c != 'g') igflg = 0; // lock point 
		if(c == 'g'){

			if(igflg == 0) {
				prtR=prtb2;
				igflg = 1;
				//	   printf("%f %f %f %f \n", prt, prtb1, prtb2, prtR);
			}

			//prtd=prt-prtR;
			//prtdabs=prtd;
			//prtdth=prtR*1.20;
			//if(prtdabs < 0) prtdabs=-prtdabs;
			//if(prtdth > 0.2) prtdth=0.2;

			if(prt > prtR*coe) {
				oc2d = 0.0;
				oc3d = -1.0*oc3sss;
			} else if (prt >= prtR/coe) {
				oc2d = 0.0;
				oc3d = 0.0;
			}

			if(prt < prtR/coe) {
				oc2d = 0.0;
				oc3d = oc3sss;
			}
		}

		// set oc2 & oc3
		oc2 = oc2 + oc2d;
		oc3 = oc3 + oc3d;

		// addN printf
		printf("c=%c %d %f %f %f %f %d \n", c, itflg, oc2, oc3, oc2d, oc3d, i);
		if(c == 'g') printf("prt= %f prtR = %f prt-= %f prt+= %f\n",prt,prtR,prtR/coe,prtR*coe);
		//	if(itflg == 1) printf("tpcmin= %f toc2min= %f toc3min= %f toc3sumrt= %f\n",tpcmin,toc2min,toc3min,toc3sumrt);
		//	if(itflg == 2) printf("toc3min= %f heater= %f Tdif= %f Tres= %f PRTdif= %f PRTres= %f\n",toc3min,heater,heater-toc3min,oc3ss, prtb1-prtb2, 0.1);

		sprintf(ocm2, ":syst:pdr:volt %f\r\n", oc2);
		sprintf(ocm3, ":syst:tcon2:spo %f\r\n", oc3);
		s2 = strlen(ocm2);
		s3 = strlen(ocm3);
		ERS_Send(comp, ocm2, s2);
		ERS_Send(comp, ocm3, s3);

		ERS_Send(comp, tmon2, 17);
		nr = ERS_Gets(comp, buf, sizeof(buf));
		if( nr != 14) nr = ERS_Gets(comp, buf, sizeof(buf));

		piezoV = atof(buf);
		ERS_ClearRecv(comp);

		ERS_Send(comp, tmon3, 19);
		nr = ERS_Gets(comp, buf, sizeof(buf));
		if( nr != 14) nr = ERS_Gets(comp, buf, sizeof(buf));

		heater = atof(buf);
		ERS_ClearRecv(comp);
		//    printf("tm2= %f tm3= %f\n", piezoV, heater);

		if(measure() != 0 ) {
			cout << "measure error" << endl;
			return 1;
		}

		//	Sleep(20000);

		//addN prt
		prt=pp/pm;

		//addN printf
		printf("piezoV= %f  Heater= %f%\n", piezoV, heater);
		printf("T0= %f P0= %f CP0= %f CM0= %f M0= %f R0=%f\n",oc3  ,pp  ,pcp,   pcm,   pm  ,prt  );
		printf("T1= %f P1= %f CP1= %f CM1= %f M1= %f R1=%f\n",oc3b1,ppb1,pcpb1, pcmb1, pmb1,prtb1);
		printf("T2= %f P2= %f CP2= %f CM2= %f M2= %f R2=%f\n",oc3b2,ppb2,pcpb2, pcmb2, pmb2,prtb2);
		printf("\n");

		//!!!!!!!!!!!!! 2016.11.15 Modified by Y.Jin (NIES)
		Sleep(100);
		memset(buf_, 0, sizeof(buf_));
		ERS_Send(comp, ":syst:smod:lev?\r\n", 17);
		ERS_Gets(comp, buf_, sizeof(buf_));
		nr_ = atoi(buf_);
		ERS_ClearRecv(comp);
		printf("%d%\n", nr_);
		//!!!!!!!!!!!!!

		fprintf(fp,"%f %f %f %f %f %f %f %f %d\n",oc2,oc3,piezoV,heater,pp,pcp,pcm,pm,nr_);

		if(i >= 95){
			for(j=0;j<94;j++){
				y[j] = y[j+1];
				y2[j] = y2[j+1];
				y3[j] = y3[j+1];
				y4[j] = y4[j+1];
				y5[j] = y5[j+1];
				y6[j] = y6[j+1];
			}  
			y[94] = heater;
			y2[94] = piezoV;
			y3[94] = pp;
			y4[94] = pcp;
			y5[94] = pcm;
			y6[94] = pm;

		} else {
			y[i] = heater;
			y2[i] = piezoV;
			y3[i] = pp;
			y4[i] = pcp;
			y5[i] = pcm;
			y6[i] = pm;
		}
		i++;
		if(i >= 95) i = 95;
		//   plotter(gp, x, y, y2, y3, y4, y5,100);
		plotterN(gp, x, y, y2, y3, y4, y5, y6, 95);
		if (c != 'e') {
			Sleep(15000);
			//Sleep(14000); // Prev. Value
			//Sleep(5000); //for quick scan
		} else {
			printf("Loop end");
		}
	}while(flag);

	getch();
	_pclose(gp);
	fclose(fp);

	ibsre(Dev, 0);
	return 0;
}