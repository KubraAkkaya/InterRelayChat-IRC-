

#include <string.h>

#include <signal.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <pthread.h>

#include <stdio.h>

#include <stdlib.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>





#define len 2048  //buffer size













pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;



void menuyeGit()

{

	//menu islemleri

	printf("--------------YAPABILECEGINIZ ISLEMLER VERILMISITR-----------------\n");

  	printf("- Mevcut odalari listelemek icin:______________________-listRooms \n");

  	printf("- Mevcut kullanicilari listelemek icin:________________-listUsers \n");

  	printf("- Odada bulunan mevcut kullanicilari listelemek icin:__-usersInRoom roomName \n");

  	printf("- Lobi olusturmak icin:________________________________-open lobi \n");

  	printf("- Mevcut oday� kapatmak(ancak oda sahibi yapabilir):___-closeThisRoom userName \n");

  	printf("- Gruba kat�lmak ya da ozel sohbet yaratmak icin:______-join \n");

  	printf("- Grupta mesaj gonderebilmek icin:_____________________-send \n");

  	printf("- Hangi kimlikle girdiginizi gormek icin:______________-whoami \n");

  	printf("- Hangi odada oldugunuzu gormek icin:__________________-whereami \n");

  	printf("- Bulundugunuz gruptan ayrilmak icin :_________________-left \n");

  	printf("- Mevcut programdan cikmak icin:_______________________-exit \n");

  

}


////sonraki sat�r� yok saymak ve daha p�r�zs�z bir g�r�n�m sa�lamak i�in, /n'yi kald�rmak i�in k�rpma i�levi

void kirpmaIslevi(char *diz, int lent)

{

  	int j;

  	for (j = 0; j < lent; j++)

  	{

    	if (diz[j] == '\n')

    	{

      		diz[j] = '\0';

      		break;

    	}

  	}

}





void baslangicImleci()

{

  printf("%s", "--> ");

  fflush(stdout); //tampondaki(ge�ici olarak verileri depolamak i�in kullan�lan bellek b�lgesi) verilerin ��k�� ayg�t�na hemen yaz�lmas�n� sa�lar ve tamponu bo�alt�r.

}






int soket = 0;

void recv_message() 	//alma

{



  char mesg[len] = {};

  

   	while(1)

  	{

    		int rcv = recv(soket, mesg, len, 0);


			if (rcv == 0)

    		{

    			//printf("HATA | ");
    			//printf("Iletilmedi\n")

    		  break;

    		}


    		else if (rcv > 0)

    		{

    		  printf("%s ", mesg);

    		  baslangicImleci();

    		}


    

   		 memset(mesg, 0, sizeof(mesg));

  	}

}

int istemciKontrol = 0;



void send_message()	//g�nderme

{

  	char mesg[len] = {};

  	

	  while (1)

 	 {

 	 

 		   baslangicImleci();

    

    		fgets(mesg, len, stdin);

    

   		 kirpmaIslevi(mesg, len);

    

  
			 if(strcmp(mesg, "-menu") == 0) //menu istiyorsa

    			 {

    			 	menuyeGit();

    			 }
   			 else if (strcmp(mesg, "-exit") == 0)//exit ise kullanicinin baglantisini tamamen kes

   			 {

    			  	istemciKontrol = 1;

    			  	break;

    			 }

    

    			else

    			{

      				send(soket, mesg, strlen(mesg), 0);

    			}



    		bzero(mesg, len); //Bayt alan�n�n ilk n bayt�n� s'den ba�layarak s�f�ra ayarlar.

 	 }

}



char bilgi[32]; //max length



int main(int argumCounter, char **argumVector)

{

  	printf("Adinizi Girin : ");

  	

  	fgets(bilgi, 32, stdin);

  	

  	kirpmaIslevi(bilgi, strlen(bilgi));

  	

  	struct sockaddr_in server_addr;

  

  	// Socket olusturma

  	soket = socket(AF_INET, SOCK_STREAM, 0);

  	server_addr.sin_family = AF_INET;

  	server_addr.sin_addr.s_addr = INADDR_ANY;

  	server_addr.sin_port = htons(4444);

  

  	//host'a baglanma

  	int cnt = connect(soket, (struct sockaddr *)&server_addr, sizeof(server_addr));

  	

  		if (cnt == -1)

  		{

   		 	printf("HATA | Baglanti Basarisiz!\n");

    			return 0;

  		}

  

  	send(soket, bilgi, 32, 0);

  

   	//Menu

  	printf("\n*-*-*-*-*-*-*  Su an ChatApp'de online durumdas�n�z  *-*-*-*-*-*-*\n\n");

	menuyeGit();

  	printf("- Tekrar menuyu gormek icin:___________________________-menu \n");

  

  

  	

  	// send islemi icin msg thread yaratma

  	pthread_t send_m_thread;

  

  		if (pthread_create(&send_m_thread, NULL, (void *)send_message, NULL) != 0)

  		{

   			 printf("HATA | Thread-send Yaratilmadi!\n");

    			return 0;

  		}

  		

  		

  	// recive islemi icin msg thread yaratma

  	pthread_t recv_m_thread;

  	

  		if (pthread_create(&recv_m_thread, NULL, (void *)recv_message, NULL) != 0)

  		{

  		 	 printf("HATA | Thread-recv Yaratilmadi!\n");

    			return 0;

  		}

  		

  		while (1)

 		{

   			if (istemciKontrol)

   		 	{

     			 	printf("Bu Kullanici Oturumdan Ayrildi!\n");

      				break;

    			}

    			

  		}

  		

  	close(soket); // olusturdugumuz soketi kapat�yoruz.

  	

  	return 1;

}






