#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
PB29 nCONFIG
PB30 CONF_DONE
PB23 DCLK
PB24 DATA
PD12 nSTATUS
*/

int read_config(char argv[])
{
	FILE *fp;
	static int Fcon_device = -1;
	int CONF_DONE = 0;
	unsigned char a;

Reconfig:
	if((fp = fopen(argv,"rb")) == NULL)
		printf("open error \n");
	
	Fcon_device = open("/dev/myFcon",O_RDWR);
	if (Fcon_device < 0 ){
		perror("open:Fcon_device");
		exit(-1);
	}
	
	a = fgetc(fp);

	while(!feof(fp)){
			
			if(!write(Fcon_device, &a, sizeof(unsigned char))){
				fclose(fp);
				close(Fcon_device);
				goto Reconfig;
			}
				
			a = fgetc(fp);
		}
		
	printf("config done 1?\n");
	read(Fcon_device,&CONF_DONE,sizeof(int));
	if(CONF_DONE)
		printf("config done 2?\n");

	fclose(fp);
	close(Fcon_device);
	return 1;
}
