#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int main (int argc,char *argv[])
{
	int             fd=-1;
	char            buf[128];
	char            *ptr=NULL;
	float           temp;
	DIR             *dirp = NULL;
	char            w1_path[64]="/sys/bus/w1/devices/";
	struct dirent   *direntp = NULL;
        char            chip_sn[32];
	char            ds18b20_path[64];
	int             found = 0;



	dirp=opendir(w1_path);
	if(!dirp)
	{
		printf("open folder %s failure: %s\n",w1_path,strerror(errno));
		return -1; 
	}

	while(NULL != (direntp=readdir(dirp)))
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
			found = 1;
		}
	
	}

	closedir(dirp);


        if(!found)
	{
		printf("can not find ds18b20 chipset\n");
		return -2;	
	}

	snprintf(ds18b20_path,sizeof(ds18b20_path),"%s%s/w1_slave",w1_path,chip_sn);
	printf("w1_path:%s\n",ds18b20_path);

	
	fd=open("/sys/bus/w1/devices/28-0317320a8aff/w1_slave",O_RDONLY);
        if(fd < 0)
	{
		printf("open file failure: %s\n",strerror(errno));
                return -1;
	}


	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf))<0)
	{
		printf("read data from fd=%d failure: %s\n",fd,strerror(errno));
		return -2;
	}
	printf("buf:%s\n",buf);

	ptr=strstr(buf,"t=");
        if(!ptr)
{
	printf("can not find t= strstr\n");
	return -1;
}

        ptr+= 2;

	temp = atof(ptr)/1000;
	printf("temprature: %f\n",temp);




	close(fd); 

}
