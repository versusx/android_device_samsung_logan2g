#include <stdio.h>
#include <stdlib.h>
#include "eut_opt.h"
#include <fcntl.h>

#ifndef NUM_ELEMS(x)
#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))
#endif
#define COUNTER_BUF_SIZE 2048

static int wifieut_state;
static int wifiwork_mode;
static float ratio_p;
static int channel_p;
static int tx_power_factor;
static int tx_power_factor_p;
static int tx_state;

static int rx_state;
static int rx_ratio;
static int rx_channel;
static int rx_power;
static long rx_packcount;
static long rxmfrmocast_priv;
static long rxmfrmocast_next;
static long rxdfrmocast;

char cmd_set_ratio[20];
char counter_respon[COUNTER_BUF_SIZE];


struct eng_wifi_eutops wifi_eutops ={   wifieut,
					 wifieut_req,
					 wifi_tx,
					 set_wifi_tx_factor,
					 wifi_rx,
					 set_wifi_mode,
					 set_wifi_ratio,
					 set_wifi_ch,
					 wifi_tx_req,
					wifi_tx_factor_req,
					wifi_rx_req,
					wifi_ratio_req,
					wifi_ch_req,
					wifi_rxpackcount,
					wifi_clr_rxpackcount
					};

struct wifi_ratio2mode{
	int mode;
	float ratios[10];
};

struct wifi_ratio2mode mode_list[]={
		{WIFI_MODE_11B,{1, 2, 5.5, 11}},
		{WIFI_MODE_11G,{6,9,12,18,24,36,48,54}},
		{WIFI_MODE_11N,{6.5,13,19.5,26,39,52,58.5,65}}
};

static int get_reflect_factor(int factor,int mode);
static long parse_packcount(char *filename);

 int wifieut(int command_code,char *rsp)
{
	ALOGI("wifieut");
	if(command_code == 1)
	start_wifieut(rsp);
	else if(command_code == 0)
	end_wifieut(rsp);
	return 0;
}
 int wifieut_req(char *rsp)
 {
 	sprintf(rsp,"%s%d",EUT_WIFI_REQ,wifieut_state);
	return 0;
 }
 int wifi_tx(int command_code,char *rsp)
{
	ALOGI("wifi_tx");
	if(command_code == 1)
	start_wifi_tx(rsp);
	else if(command_code == 0)
	end_wifi_tx(rsp);
	return 0;
}
 int wifi_rx(int command_code,char *rsp)
{
	ALOGI("wifi_rx");
	if(command_code == 1)
	start_wifi_rx(rsp);
	else if(command_code == 0)
	end_wifi_rx(rsp);
	return 0;
}
 int start_wifieut(char *result)
{
	ALOGI("start_wifieut----------------------");
	int error = system("ifconfig wlan0 down");
	if(error == -1 || error ==127){
	ALOGE("=== start_wifieut test failed on cmd : ifconfig wlan0 down ===\n");
	sprintf(result,"%s%d",EUT_WIFI_ERROR,error);	
	}else{
		error = system("echo -n /system/etc/wifi/sdio-g-mfgtest.bin > /sys/module/bcmdhd/parameters/firmware_path");
		if(error == -1 || error ==127){
			ALOGE("=== start_wifieut test failed on cmd : echo -n \"/system/etc/sdio-g-mfgtest.bin\" > /sys/module/bcmdhd/parameters/firmware_path ===\n");
			sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
			}else{
				error = system("ifconfig wlan0 up");
				if(error == -1 || error ==127){
					ALOGE("=== start_wifieut test failed on cmd : ifconfig wlan0 down ===\n");
					sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
					}else{
					ALOGI("=== WIFIEUT test succeed! ===\n");
					wifieut_state=1;
					strcpy(result,EUT_WIFI_OK);
						}
				}
		
		
		}
	return 0;
}
 

 int end_wifieut(char *result)
{
	int error = system("ifconfig wlan0 down");
	if(error == -1 || error ==127){
		ALOGE("=== start_wifieut test failed on cmd : ifconfig wlan0 down ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
	wifieut_state=0;
	strcpy(result,EUT_WIFI_OK);
		}
	return 0;
}

 int start_wifi_tx(char *result)
{
	ALOGI("start_wifi_tx-----------------");
	char* cmd_set_channel[20]={0};
	char* cmd_set_factor[25]={0};
	
	sprintf(cmd_set_channel,"wl channel %d",channel_p);
	sprintf(cmd_set_factor,"wl txpwr1 -o -d %d",tx_power_factor);
	
	int error =system("wl down");
	ALOGI("wl down");
	if(error == -1 || error == 127){
		ALOGE("=== start_wifi_tx test failed on cmd : wl down ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
	error = system("wl mpc 0");
	ALOGI("wl mpc 0");
	if(error == -1 || error == 127){
		ALOGE("=== start_wifi_tx test failed on cmd : wl mpc 0 ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
		}else{
				error = system("wl up");
				ALOGI("wl up");
				if(error == -1 || error ==127){
					ALOGE("=== start_wifi_tx test failed on cmd : wl up ===\n");
					strcpy(result,"WIFI_TX,fail");
					}else{
							error = system("wl PM 0");
							ALOGI("wl PM 0");
							if(error == -1 || error == 127){
							  ALOGE("=== start_wifi_tx test failed on cmd : wl PM 0 ===\n");
							  sprintf(result,"%s=%d",EUT_WIFI_ERROR,error);
							}else{
							   error = system("wl scansuppress 1");
							   ALOGI("wl scansuppress 1");
							  if(error == -1 || error == 127){
							  	ALOGE("=== start_wifi_tx test failed on cmd : wl scansuppress 1===\n");
							    sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
							  	}else{
							  	error = system("wl country ALL");
								ALOGI("wl country ALL");
								if(error == -1 || error == 127){
							    ALOGE("=== start_wifi_tx test failed on cmd : wl country ALL 1===\n");
							    sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
							}else{
							     error = system(cmd_set_channel);
								 ALOGI("wl channel %d",channel_p);
								 if(error == -1 || error == 127){
								 	ALOGE("=== start_wifi_tx test failed on cmd : wl country ALL 1===\n");
							    	sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
								 	}else{
								 	     error = system(cmd_set_ratio);
										 ALOGI("wl rate  %s %f",cmd_set_ratio,ratio_p);
										 if(error == -1 || error == 127){
										 	ALOGE("=== start_wifi_tx test failed on cmd : wl rate===\n");
							    			sprintf(result,"%s=%d",EUT_WIFI_ERROR,error);
										 	}else{
										 	 error = system(cmd_set_factor);
											 ALOGI("wl txpwr1 -o -d %d",tx_power_factor);
											    if(error == -1 || error == 127){
													ALOGE("=== start_wifi_tx test failed on cmd : wl txpwr1 -o -d===\n");
							    					sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
											    	}else{
											    	error=system("wl pkteng_start 00:11:22:33:44:55 tx 100 1500 0");
													ALOGI("wl pkteng_start 00:11:22:33:44:55 tx 100 1500 0");
													if(error == -1 || error == 127){
														ALOGE("=== start_wifi_tx test failed on cmd : wl pkteng_start 00:11:22:33:44:55 tx 100 1500 0===\n");
							    						sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
														}else{
															error = system("wl phy_forcecal 1");
															if(error == -1 || error == 127){
																ALOGE("=== start_wifi_tx test failed on cmd : wl phy_forcecal 1===\n");
							    								sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
																}else{
																	ALOGI("=== WIFIEUT test succeed! ===\n");
																	strcpy(result,EUT_WIFI_OK);
																	tx_state=1;
																	}
															}
											    		}
										 		}
								 		}
								}
							  		}
									}
						}
			}
			}
	return 0;
}

 int end_wifi_tx(char *result)
{
	int error = system("wl pkteng_stop tx");
	ALOGI("end_wifi_tx");
	if(error == -1 || error == 127){
		ALOGE("=== start_wifi_rx test failed on cmd : wl pkteng_stop tx ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
	     ALOGI("=== WIFI_TX test succeed! ===\n");
		 strcpy(result,EUT_WIFI_OK);
		 tx_state=0;
			}
	return 0;
}  
                                  
int start_wifi_rx(char *result)
{
	ALOGI("start_wifi_rx-----------------");
	char* cmd_set_channel[20]={0};
	sprintf(cmd_set_channel,"wl channel %d",channel_p);

	int error =system("wl down");
	ALOGI("wl down");
	if(error == -1 || error == 127){
		ALOGE("=== start_wifi_rx test failed on cmd : wl down ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
	error = system("wl mpc 0");
	ALOGI("wl mpc 0");
	if(error == -1 || error == 127){
		ALOGE("=== start_wifi_rx test failed on cmd : wl mpc 0 ===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
		}else{
				error = system("wl up");
				ALOGI("wl up");
				if(error == -1 || error ==127){
					ALOGE("=== start_wifi_rx test failed on cmd : wl up ===\n");
					sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
					}else{
							error = system("wl PM 0");
							ALOGI("wl PM 0");
							if(error == -1 || error == 127){
							  ALOGE("=== start_wifi_rx test failed on cmd : wl PM 0 ===\n");
							  sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
							}else{
							   error = system("wl scansuppress 1");
							   ALOGI("wl scansuppress 1");
							  if(error == -1 || error == 127){
							  	ALOGE("=== start_wifi_rx test failed on cmd : wl scansuppress 1===\n");
							    sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
							  	}else{
							  	error = system("wl country ALL");
								ALOGI("wl country ALL");
								if(error == -1 || error == 127){
							    ALOGE("=== start_wifi_tx test failed on cmd : wl country ALL 1===\n");
							    sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
							}else{
							     error = system(cmd_set_channel);
								 ALOGI("wl channel %d",channel_p);
								 if(error == -1 || error == 127){
								 	ALOGE("=== start_wifi_tx test failed on cmd : wl channel =\n");
							    	sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
								 	}else{
								 	     error = system(cmd_set_ratio);
										 ALOGI("wl rate %s %f",cmd_set_ratio,ratio_p);
										 if(error == -1 || error == 127){
										 	ALOGE("=== start_wifi_rx test failed on cmd : wl rate ===\n");
							    			sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
										 	}else{
													error = system("wl pkteng_start 00:11:22:33:44:55 rx");
													if(error == -1 || error == 127){
														ALOGE("=== start_wifi_rx test failed on cmd : wl pkteng_start 00:11:22:33:44:55 rx ===\n");
														sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
														}else{
															rx_state=1;
															strcpy(result,EUT_WIFI_OK);
															}
														}
								 		}
								}
							  		}
									}
						}
			}
	return 0;
}

	return 0;
}
 int end_wifi_rx(char *result)
{

	ALOGE("=== end_wifi_rx test failed on cmd : wl counter===\n");
	rx_state=0;
	strcpy(result,EUT_WIFI_OK);
	return 0;
}                                         
 int set_wifi_ratio(float ratio,char *result)
{
	ratio_p = ratio;
	ALOGI("=== set_wifi_ratio----> %f === %f",ratio_p,ratio);
	int i,m,j,n;
	int break_tag = 0; 
	n = (int)NUM_ELEMS(mode_list);
	for(i=0;i<n;i++){
		for(j = 0;j<(int)NUM_ELEMS(mode_list[i].ratios);j++){
			if(ratio == mode_list[i].ratios[j])
				{
					wifiwork_mode=mode_list[i].mode;
					m = j;
					ALOGI("find the ratio %f  wifiwork_mode=%d",ratio,wifiwork_mode);
					break_tag=1;
					break;
				}
			}
			if(break_tag)
			break;
			if(i==n-1){
				sprintf(result,"%s%d",EUT_WIFI_ERROR,EUT_WIFIERR_RATIO);
				ratio_p=0;
				
				goto fun_out;
				}
			}
	switch(wifiwork_mode){
		case WIFI_MODE_11B:
		case WIFI_MODE_11G:
			sprintf(cmd_set_ratio,"wl rate %f",ratio_p);
			ALOGI("set ratio cmd is %s",cmd_set_ratio);
			break;
		case WIFI_MODE_11N:			
			sprintf(cmd_set_ratio,"wl nrate -m %d",m);
			ALOGI("set ratio cmd is %s",cmd_set_ratio);
			break;
		}
	strcpy(result,EUT_WIFI_OK);
fun_out:	
			return 0;
}
 int set_wifi_ch(int ch,char *result)
{
	channel_p = ch;
	ALOGI("=== set_wifi_ch ----> %d ===\n",channel_p);
	strcpy(result,"+SPWIFITEST:OK");
	return 0;
}

  // for *#*#83780#*#*
  int  set_wifi_tx_factor_83780(int factor,char *result)
{
	tx_power_factor = factor;
	ALOGI("=== set_wifi_tx_factor----> %d===\n",tx_power_factor);
	if(tx_power_factor){
		strcpy(result,EUT_WIFI_OK);
	}else{
		sprintf(result,"%s%d",EUT_WIFI_ERROR,EUT_WIFIERR_TXFAC_SETRATIOFIRST);
	}

	return 0;
}

  int  set_wifi_tx_factor(long factor,char *result)
{
	if((factor>=1)&&(factor<=32767)){
	tx_power_factor = get_reflect_factor(factor,wifiwork_mode);
	tx_power_factor_p = factor;
	ALOGI("=== set_wifi_tx_factor----> %d===\n",tx_power_factor);
	if(tx_power_factor){
	strcpy(result,EUT_WIFI_OK);
		}else{
			sprintf(result,"%s%d",EUT_WIFI_ERROR,EUT_WIFIERR_TXFAC_SETRATIOFIRST);
			}
		}else{
			sprintf(result,"%s%d",EUT_WIFI_ERROR,EUT_WIFIERR_TXFAC);
		}
	return 0;
} 
  static int get_reflect_factor(int factor,int mode)
{
	int reflect_factor_p;
	ALOGI("get_reflect_factor factor=%d mode=%d",factor,mode);
	switch(mode){
		case WIFI_MODE_11B:
			reflect_factor_p=reflect_factor(6,16,factor);
			break;
		case WIFI_MODE_11G:
			reflect_factor_p=reflect_factor(6,10,factor);
			break;
		case WIFI_MODE_11N:
			reflect_factor_p=reflect_factor(6,15,factor);
			break;
		default:
			return -1;
	}
        return reflect_factor_p;
}  	
  static int reflect_factor(int start,int end,int factor)
{	
	int n = end-start+1;
	return  (factor/32767)*n+start;
}
  
 int set_wifi_mode(char * mode, char * result)
{
	return 0;
}
 static int wifi_tx_req(char *result)
{
	sprintf(result,"%s%d",EUT_WIFI_TX_REQ,tx_state);
	return 0;
}
static int wifi_tx_factor_req(char *result)
{
	sprintf(result,"%s%d",EUT_WIFI_TXFAC_REQ,tx_power_factor_p);
	return 0;
}	
static int wifi_rx_req(char *result)
{
	sprintf(result,"%s%d",EUT_WIFI_RX_REQ,rx_state);
	return 0;
}
static int wifi_ratio_req(char *result)
{
	sprintf(result,"%s%f",EUT_WIFI_RATIO_REQ,ratio_p);
	return 0;
}
static int wifi_ch_req(char *result)
{
	sprintf(result,"%s%d",EUT_WIFI_CH_REQ,channel_p);
	return 0;
}
int wifi_rxpackcount(char * result)
{
	int error = system("wl counters > /data/data/wlancounters.txt");
	ALOGI("wifi_rxpackcount");
	if(error == -1 || error == 127){
		ALOGE("=== wifi_rxpackcount on cmd : wl counter===\n");
		sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
	     ALOGI("=== wifi_rxpackcount! ===\n");
		 strcpy(result,EUT_WIFI_OK);
		 rxmfrmocast_next=parse_packcount("/data/data/wlancounters.txt");
		 rx_packcount = rxmfrmocast_next-rxmfrmocast_priv;
		 system("rm /data/data/wlancounter.txt");
		 sprintf(result,"%s%d",EUT_WIFI_RXPACKCOUNT_REQ,rx_packcount);
			}
	return 0;
}
int wifi_clr_rxpackcount(char * result)
{
	rx_packcount = 0;
	int error=system("wl counters > /data/data/wlancounters.txt");
	ALOGI("wl counters");
	if(error == -1 || error == 127){
	ALOGE("=== start_wifi_rx test failed on cmd : wl counters===\n");
	sprintf(result,"%s%d",EUT_WIFI_ERROR,error);
	}else{
		rxmfrmocast_priv=parse_packcount("/data/data/wlancounters.txt");
		}
	strcpy(result,EUT_WIFI_OK);
	return 0;
}
static long parse_packcount(char * filename)
{
	int fd,n,len;
	char packcount[20];
	memset(counter_respon,0,COUNTER_BUF_SIZE);
	fd = open(filename, O_RDWR|O_NONBLOCK);
	n = read(fd,counter_respon,COUNTER_BUF_SIZE);
	char *p=strstr(counter_respon,"pktengrxducast");
	if(p != NULL){
	char *q=strstr(p," ");
	char *s=strstr(q," pktengrxdmcast");
	len= s-q-1;
	memcpy(packcount,q+1,len);
	return atol(packcount);
	}else{
			return 0;
		}
}
