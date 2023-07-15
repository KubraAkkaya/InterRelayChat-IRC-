

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

  	printf("- Mevcut odayý kapatmak(ancak oda sahibi yapabilir):___-closeThisRoom userName \n");

  	printf("- Gruba katýlmak ya da ozel sohbet yaratmak icin:______-join \n");

  	printf("- Grupta mesaj gonderebilmek icin:_____________________-send \n");

  	printf("- Hangi kimlikle girdiginizi gormek icin:______________-whoami \n");

  	printf("- Hangi odada oldugunuzu gormek icin:__________________-whereami \n");

  	printf("- Bulundugunuz gruptan ayrilmak icin :_________________-left \n");

  	printf("- Mevcut programdan cikmak icin:_______________________-exit \n");

  

}


////sonraki satýrý yok saymak ve daha pürüzsüz bir görünüm saðlamak için, /n'yi kaldýrmak için kýrpma iþlevi

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

  fflush(stdout); //tampondaki(geçici olarak verileri depolamak için kullanýlan bellek bölgesi) verilerin çýkýþ aygýtýna hemen yazýlmasýný saðlar ve tamponu boþaltýr.

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



void send_message()	//gönderme

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



    		bzero(mesg, len); //Bayt alanýnýn ilk n baytýný s'den baþlayarak sýfýra ayarlar.

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

  	printf("\n*-*-*-*-*-*-*  Su an ChatApp'de online durumdasýnýz  *-*-*-*-*-*-*\n\n");

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

  		

  	close(soket); // olusturdugumuz soketi kapatýyoruz.

  	

  	return 1;

}






