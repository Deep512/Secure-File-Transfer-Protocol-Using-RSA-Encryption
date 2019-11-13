#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include<arpa/inet.h>
#define PORT 4444

long int modular_expo(long int b,long int e,long int m)
{
    long long int r=1,base=b;
    base %= m;
    while(e)
        {
            if( e%2 ==1 )
                r = (r*base) % m;
            e = e >> 1;
            base = (base * base) % m;
        }
    return r;
}

char * encrypt(long int PUBLIC_EXPO,long int MOD,char *filename)
{
    long int unencrypted_msg[50000],encrypted_msg[50000];
	int i,str_len=0;
    char str[50000]={'\0'}, encrypted_file[]="encrypt.txt";

    //Reading Public Key
    FILE *fp;
    fp=fopen(filename,"r");
    printf("pub_expo:%ld\n",PUBLIC_EXPO);   //Reading Public Key.
    printf("mod:%ld\n",MOD);
    if(!fp)
        printf("Error opening file!\n");    

    //fscanf(fp,"%s",str);
	printf("\nContent of file:\n\n");
    fgets(str,10000,fp);
    fputs(str,stdout);
    str_len=strlen(str);
	
    for(i=0;i<str_len;i++)
    {	
		unencrypted_msg[i]=(long int)str[i];
	}
    printf("\nEncrypted Message:\n\n");
    for(i=0;i<str_len;i++)
        encrypted_msg[i]=modular_expo(unencrypted_msg[i],PUBLIC_EXPO,MOD);
	fclose(fp);
    FILE *encrypt_fp;
    encrypt_fp=fopen(encrypted_file,"w+");
    for(i=0;i<str_len;i++)
        {
            printf("%ld:",encrypted_msg[i]);
            fprintf(encrypt_fp,"%ld\n",encrypted_msg[i]);
        }
    fclose(encrypt_fp);
    filename = encrypted_file;
}

void always_on (int sock)
{
	int n,fd,i,j; long int pub_expo,mod;
	char buffer[256],requested_file[256]={'\0'},pub_expo_str[50]={'\0'},mod_str[50]={'\0'},*endptr,*file_name;
	
	bzero(buffer,256) ;
	n=read(sock,buffer,256);	//Will receive content sent by client i.e. filename + public key

	for(i=0;buffer[i]!='#';i++)
		requested_file[i]=buffer[i];	
	requested_file[i]='\0';
	
	//Converting private key from string type into long integer
	for(i=i+1,j=0;buffer[i]!='#';i++,j++)
		pub_expo_str[j]=buffer[i];
	pub_expo_str[j]='\0';
	pub_expo = strtol(pub_expo_str,&endptr,10);	//strtol() function converts string to long integer
	
	for(i=i+1,j=0;i<n;i++,j++)
		mod_str[j]=buffer[i];
	mod_str[j]='\0';
	mod = strtol(mod_str,&endptr,10);	
	
	file_name = encrypt(pub_expo,mod,requested_file); 
	// 'file_name' contains encrypted data
	printf("\n\nFile containing encrypted data:%s\n",file_name);

   fd=open(file_name,O_RDONLY);
   while(1)
    {
		n=read(fd,file_name,50000);
		if(n<=0)
			exit(0);
		n=write(sock,file_name,n);
	}
}

int main(int argc,char **argv)
{
int i,j;
ssize_t n;
FILE *fp;
char s[80],f[80];
struct sockaddr_in servaddr,cliaddr;
int listenfd,connfd,clilen;
listenfd=socket(AF_INET,SOCK_STREAM,0);
bzero(&servaddr,sizeof(servaddr));
servaddr.sin_family=AF_INET;
servaddr.sin_port=htons(PORT);
bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
listen(listenfd,1);
clilen=sizeof(cliaddr);
connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
printf("client connected\n");
always_on(connfd);
close(listenfd);
fclose(fp);
}
