#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<sys/fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<ctype.h>

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
    long int unencrypted_msg[50000],encrypted_msg[50000],i;
    char str[50000]={'\0'},str_len=0, encrypted_file[]="encrypt.txt";

    //Reading Public Key
    FILE *fp;
    fp=fopen(filename,"r");
    printf("pub_expo:%ld\n",PUBLIC_EXPO);   //Reading Public Key.
    printf("mod:%ld\n",MOD);
    if(!fp)
        printf("Error opening file!\n");    

    //fscanf(fp,"%s",str);
    fgets(str,50000,fp);
    str_len=strlen(str);
    printf("File content:");
	
    for(i=0;i<str_len;i++)
        unencrypted_msg[i]=(long int)str[i];

    printf("\n\nEncrypted Message:\n");
    for(i=0;i<str_len;i++)
        encrypted_msg[i]=modular_expo(unencrypted_msg[i],PUBLIC_EXPO,MOD);
    fclose(fp);
    //Writing Encypted Message on screen as well as on file to be sent.
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

void always_on(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen, pid;
     struct sockaddr_in serv_addr, cli_addr;
	
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
	
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("sockfd out:%d\n",sockfd);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);		//activating socket
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             always_on(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     return 0; /* we never get here */
}

/******** ALWAYS_ON() *********************
 It handles all communication
 once a connnection has been established.
 *****************************************/
void always_on (int sock)
{
	int n,fd,i,j; long int pub_expo,mod;
	char buffer[256],requested_file[256]={'\0'},pub_expo_str[50]={'\0'},mod_str[50]={'\0'},*endptr,*file_name;
	
	bzero(buffer,256) ;
	n=read(sock,buffer,256);	//Will receive content sent by client i.e. filename + public key

        if (n < 0) 
		error("ERROR reading from socket");
	
	/*****************************************/
	//detaching filename from public key into separate arrays
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
	/*****************************************/

	/*************************************
	Encryption function goes here   
	**************************************/
	
	file_name = encrypt(pub_expo,mod,requested_file); 
	// 'file_name' contains encrypted data
	printf("\nfilename after bob:%s\n",file_name);
	/**************************************/

   fd=open(file_name,O_RDONLY);
   if(fd<0)
	error("file open error ");
   while(1)
      {
	n=read(fd,file_name,256);
	if(n<=0)
	   exit(0);
	n=write(sock,file_name,n);
	if (n < 0)
	   error("ERROR writing to socket");
	else
	    printf("Transfer Completed\n");
      }     
}
