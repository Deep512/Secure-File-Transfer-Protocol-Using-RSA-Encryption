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
#include <arpa/inet.h>

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
    FILE *erase_fp;
    erase_fp=fopen(encrypted_file,"w+");
    fclose(erase_fp);
    FILE *fp;
    fp=fopen(filename,"r");
    if(!fp){
        perror("file not found");
        exit(0);
    }
    printf("pub_expo:%ld\n",PUBLIC_EXPO);   
    printf("mod:%ld\n",MOD);    
    printf("\nEncrypted Message:\n\n");
    while(fgets(str,10000,fp))
    {
    str_len=strlen(str);
    for(i=0;i<str_len;i++)
    {	
		unencrypted_msg[i]=(long int)str[i];
	}
    for(i=0;i<str_len;i++)
        encrypted_msg[i]=modular_expo(unencrypted_msg[i],PUBLIC_EXPO,MOD);

    FILE *encrypt_fp;
    encrypt_fp=fopen(encrypted_file,"a+");
    for(i=0;i<str_len;i++)
        {
            printf("%ld:",encrypted_msg[i]);
            fprintf(encrypt_fp,"%ld\n",encrypted_msg[i]);
        }
        fclose(encrypt_fp);
    }
    fclose(fp);
    filename = encrypted_file;
}

void always_on (int sock)
{
	int n,fd,i,j; long int pub_expo,mod;
	char buffer[256],requested_file[256]={'\0'},pub_expo_str[50]={'\0'},mod_str[50]={'\0'},*endptr,*file_name;
	
	bzero(buffer,256) ;
	n=read(sock,buffer,256);

	for(i=0;buffer[i]!='#';i++)
		requested_file[i]=buffer[i];	
	requested_file[i]='\0';
	
	//Converting private key from string type into long integer
	for(i=i+1,j=0;buffer[i]!='#';i++,j++)
		pub_expo_str[j]=buffer[i];
	pub_expo_str[j]='\0';
	pub_expo = strtol(pub_expo_str,&endptr,10);
	
	for(i=i+1,j=0;i<n;i++,j++)
		mod_str[j]=buffer[i];
	mod_str[j]='\0';
	mod = strtol(mod_str,&endptr,10);	
	
	file_name = encrypt(pub_expo,mod,requested_file); 
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
    if(argc<2){
        perror("port not provided");
        exit(0);
    }
    int i,j;
    ssize_t n;
    FILE *fp;
    char s[80],f[80];
    struct sockaddr_in servaddr,cliaddr;
    int listenfd,connfd,clilen; 
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
	    perror("socket not created");
	    exit(0);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;    
    servaddr.sin_port=htons(atoi(argv[1]));
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));     
    if(listen(listenfd,1)==-1){
        perror("connection error");
        exit(0);
    }
    clilen=sizeof(cliaddr); 
    connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
    if(connfd==-1){
        perror("connection error");
        exit(0);
    }
    printf("client connected\n");
    always_on(connfd);  
    close(listenfd);    
    fclose(fp);
}
