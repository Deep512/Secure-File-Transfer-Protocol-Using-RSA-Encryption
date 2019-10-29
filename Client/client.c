#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/fcntl.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#define MIN 16500
#define MAX 32500


long int generate_primes(long int prime_array[])
{
    long int i,j,count=0;
    for(i=MIN+1;i<MAX;i+=2)
    {
	for(j=2;j<i/2;j++)
	    {
		if(i%j == 0)
		    break;
	    }
	if(j>=i/2)
	    prime_array[count++]=i;
    }
    return count;
}

long int mul_inv(long int a,long int b)
{
	long int b0 = b, t, q;
	long int x0 = 0, x1 = 1;
	if (b == 1) return 1;
	while (a > 1) {
		q = a / b;
		t = b, b = a % b, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0) x1 += b0;
	return x1;
}

void key_generate(long int *d,long int *e,long int *n)
{
    long int PRIME_P,PRIME_Q,count,MODULUS,PHI;
    long int prime1_index,prime2_index;
    long int prime_array[1600];

count=generate_primes(prime_array);
//There are total 1575(which is value of 'count' variable at this PRIME_Point)
//primes between this range, And the program will randomly select any
//2-primes from those generated primes.

//randomize();     //This intializes seed for random numbers from clock.
                   //That's why 'time.h' is included. :)

prime1_index=prime2_index=2;
int low=1000;
while(prime1_index==prime2_index)
    {   srand(time(NULL));
	prime2_index = (rand() % (count-low) ) + low;
	prime1_index = rand() % low;
    }

PRIME_P=prime_array[prime1_index];
PRIME_Q=prime_array[prime2_index];

printf("PRIME_P:%10ld ( 0x%lx )\n",PRIME_P,PRIME_P);
printf("PRIME_Q:%10ld ( 0x%lx )\n",PRIME_Q,PRIME_Q);
//Here 2-primes numbers of same bit-length(15 bit) are choosen randomly

MODULUS=PRIME_P*PRIME_Q;   //This is our MODULUS for 'private' and 'public' key generation
PHI=(PRIME_P-1)*(PRIME_Q-1);  //This is 'Euler's Totient Function
printf("\nMODULUS:%10ld ( 0x%lx )\n",MODULUS,MODULUS);
printf("PHI(n) =%10ld ( 0x%lx )\n\nNOTE:phi(n) = phi(p)*phi(q) = (p-1)*(q-1)\n",PHI,PHI);

//Now, let public Key Exponent(e)=65537
//Thus, e is coprime to phi(n).(i.e. gcd(e,phi(n)) = 1)
//So, the public key is declared as: (e,n)
long int PUBLIC_KEY_EXPONENT=65537,PRIVATE_KEY_EXPONENT;
//FILE *fp;
//fp=fopen("public_key.txt","w+");
PRIVATE_KEY_EXPONENT=mul_inv(PUBLIC_KEY_EXPONENT,PHI);
printf("\nThe Public Key(e,n) is: ( %ld ( 0x%lx ), %ld ( 0x%lx ) )\n",PUBLIC_KEY_EXPONENT,PUBLIC_KEY_EXPONENT,MODULUS,MODULUS);
printf("\nThe Private Key(d,n) is: ( %ld ( 0x%lx ), %ld ( 0x%lx ) )\n",PRIVATE_KEY_EXPONENT,PRIVATE_KEY_EXPONENT,MODULUS,MODULUS);

//Putting Public Key for Public access.
//fprintf(fp,"%ld\n%ld",PUBLIC_KEY_EXPONENT,MODULUS);
//fclose(fp);
*e = PUBLIC_KEY_EXPONENT;
*d=PRIVATE_KEY_EXPONENT;  //Giving up private key as a
*n=MODULUS;               //Result of this function.
}

long long int modular_expo(long int b,long int e,long int m)
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

void decryption(long int PRIVATE_EXPO,long int MOD,char *encrypted_str,int n,char *filename)
{
    long int encrypted_values[5000],decrypted_values[5000],i,j,value;
    long long int decrypted_value;
    char decrypted_msg[5000],value_str[20]={'\0'}, *endptr;
	FILE *fp=fopen(filename,"w+");
    //printf("\n\nEncrypted str:%s",encrypted_str);
    for(i=0,j=0;i<n;i++,j++){
	if(encrypted_str[i]!='\n'){
		value_str[j] = encrypted_str[i];
	}
	else{
		value_str[j]='\0';
		j=-1;
		value = strtol(value_str,&endptr,10);
		decrypted_value = modular_expo(value,PRIVATE_EXPO,MOD);
		fprintf(fp,"%c",(char)decrypted_value);
		}
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	int sockfd, portno,n;
	long int pub_expo,pri_expo,mod;
	struct sockaddr_in serv_addr;
	char buffer[256], *servip,pub_expo_str[256]={'\0'},mod_str[256]={'\0'},filename[256]={'\0'},content[50000]={'\0'};
	if(argc<4)
	   {
		printf("Not enough parameters.");
		exit(0);
	   }
	servip=argv[1] ;   //server ip is being delievered
	portno=atoi(argv[3]); //port number is being delievered
	strcpy(filename,argv[2]);	//requested file name is being kept and stored
	
	//Key generation is being done here
	key_generate(&pri_expo,&pub_expo,&mod);
	
	//Converting long integer public key into string to append it after the filename
	sprintf(pub_expo_str,"%ld",pub_expo);   
	sprintf(mod_str,"%ld",mod);

	strcat(argv[2],"#");	strcat(argv[2],pub_expo_str);  //argv[2] = filename + pub_expo_str, now
	strcat(argv[2],"#");	strcat(argv[2],mod_str);   //argv[2] = filename + pub_expo_str + mod_str, now

	sockfd=socket(AF_INET, SOCK_STREAM,0);
	if(sockfd<0)
		perror("error opening socket");
	printf("client online\n");
	bzero((char *)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(servip);
	serv_addr.sin_port = htons(portno);
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		perror("error connecting");

	//to request file 'filename' we must write its name in buffer, So	
	//write argv[2](filename + pub_expo_str + mod_str) in the buffer
	write(sockfd,argv[2],strlen(argv[2])+1);	
	
	//file received by this point, So we need to read data
	n=read(sockfd,content,50000);	//moves content from sockfd --> 'content[]'
	
	printf("Content Received:\n");	
	write(1,content,n);     //write content of n length from 'content[]' to the terminal
	if(n<=0)
	  { 
		perror("file not found");
	 	exit(0);
	   }

/***********************************************
Decryption function goes here   
************************************************/

decryption(pri_expo,mod,content,n,filename);

/*************************************************/	

printf("File Received.\n");		
return 0;
}
