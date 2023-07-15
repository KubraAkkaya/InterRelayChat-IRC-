

#include <string.h>

#include <pthread.h>

#include <sys/types.h>

#include <arpa/inet.h>

#include <stdio.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <unistd.h>

#include <errno.h>

#include <stdlib.h>



static int clientCounter = 0;

static int userid = 10;





#define MAX_ROOM 8

#define MAX_BUFFER_SIZE 2048

#define MAX_KULLANICI 25

#define MAX_GRUP_KAPASITESI 10





typedef struct

{

	struct sockaddr_in address;

	int socket;

	int userid;

	char name[32];

	

} clientStruct;





typedef struct

{

	clientStruct *clientsInGroup[MAX_GRUP_KAPASITESI];

	char name[10];

	char password[10];

	

} roomStruct;





//ture gore adres olusturma



clientStruct *clients[MAX_KULLANICI];



roomStruct *rooms[MAX_ROOM];






//oda kapatma

void closeRoom(char* grupName)

{



    int index = -1;

    for(int i = 0; i < MAX_ROOM; i++)

    {

        if(rooms[i] != NULL && strcmp(rooms[i]->name, grupName) == 0)

        {

            index = i;

            break;

        }

    }



    // İndeks bulunamazsa fonksiyon işlemez

    if(index == -1)

    {

        return;

    }



    // Önce odada bulunan tüm kullanıcıları çıkarın

    for(int i = 0; i < MAX_GRUP_KAPASITESI; i++)

    {

        if(rooms[index]->clientsInGroup[i])

        {

            int userid = rooms[index]->clientsInGroup[i]->userid;

            leftClientGroup(userid, index);

        }

    }



    // Odadaki tüm kullanıcılar kaldırıldıktan sonra, odayı kapatın

    rooms[index] = NULL;

}

//oda yaratma islemei

void creatRoom(roomStruct *group)

{

	for (int i = 0; i < MAX_ROOM; ++i)

	{

		if (!rooms[i])

		{

			rooms[i] = group;

			break;

		}

	}

}







//ana ekrana mesaj verme

void sendMsg(char *s, int userid)

{

	for (int i = 0; i < MAX_KULLANICI; ++i)

	{

		if (clients[i])

		{

			if (clients[i]->userid == userid)

			{

				if (send(clients[i]->socket, s, strlen(s), 0) < 0)

				{

					break;

				}

			}

		}

	}

}

//server'a client ekleme islemi

void addCliToServ(clientStruct *usr)

{

	for (int i = 0; i < MAX_KULLANICI; ++i)

	{

		if (!clients[i])

		{

			clients[i] = usr;

			break;

		}

	}

}


//clinet'a gore grup bulma islemi

int findGroupIndexByClientId(int userid)

{

	int index = -1;

	for (int i = 0; i < MAX_ROOM; i++)

	{

		if (rooms[i])

		{

			for (int j = 0; j < MAX_GRUP_KAPASITESI; j++)

			{

				if (rooms[i]->clientsInGroup[j])

				{

					//kullanıcı grup icindeyse

					if (rooms[i]->clientsInGroup[j]->userid == userid)

					{

						index = i;

						break;

					}

				}

			}

		}

	}

	return index;

}

//Odaya client ekleme islemi

void joinClientGroup(clientStruct *cl, int index)

{

	if (rooms[index])

	{

		for (int i = 0; i < MAX_GRUP_KAPASITESI; i++)

		{

			if (!rooms[index]->clientsInGroup[i])

			{

				rooms[index]->clientsInGroup[i] = cl;

				break;

			}

		}

	}

}





//client'i mevcut bulundugu odadan cikarma islemi -left

void leftClientGroup(int userid, int index)

{

	int clientCnt = 0;

	for (int i = 0; i < MAX_GRUP_KAPASITESI; i++)

	{

		if (rooms[index]->clientsInGroup[i])

		{

			if (rooms[index]->clientsInGroup[i]->userid == userid)

			{

				rooms[index]->clientsInGroup[i] = NULL;

				break;

			}

		}

	}

	//odada kac kisi var konrtol et

	for (int i = 0; i < MAX_GRUP_KAPASITESI; i++)

	{

		if (rooms[index]->clientsInGroup[i])

		{

			clientCnt++;

		}

	}

	// oda bossa odayı kapat

	if (clientCnt == 0)

	{

		rooms[index] = NULL;

	}

}






//client'lar arasi iletisim

void sendMessgClins(char *s, int userid)

{

	int index = findGroupIndexByClientId(userid);

	for (int i = 0; i < MAX_GRUP_KAPASITESI; i++)

	{

		if (rooms[index]->clientsInGroup[i])

		{

			if (rooms[index]->clientsInGroup[i]->userid != userid)

			{

				if (write(rooms[index]->clientsInGroup[i]->socket, s, strlen(s)) < 0)

				{

					break;

				}

			}

		}

	}

}







//gruba gore  grup bulma

int findGroupIndexByName(char *name)

{

	int index = -1;

	for (int i = 0; i < MAX_ROOM; i++)

	{

		if (rooms[i])

		{

			if (rooms[i]->name)

			{

				if (strcmp(rooms[i]->name, name) == 0)

				{

					index = i;

					break;

				}

			}

		}

	}

	return index;

}

//kullanici ile kullanici kimligi bulma

int findClientIndexByName(char *name)

{

	int index = -1;

	for (int i = 0; i < MAX_KULLANICI; i++)

	{

		if (clients[i])

		{

			if (clients[i]->name)

			{

				if (strcmp(clients[i]->name, name) == 0)

				{

					index = i;

					break;

				}

			}

		}

	}

	return index;

}







//server iletisim

void *server_client(void *arg)

{



	int argIn; //argument index'i

	int grpIn; //grup index'i

	

	int clientInRoom[10];

	char clientInRoomStr[10];



	char odaSahibi[10];



	char *argument;

	char *arguments[10];

	char name[32];



	char grupName[20];



	roomStruct *group = (roomStruct *)arg;



	char grupSay[20];

	

	clientCounter++;

	clientStruct *usr = (clientStruct *)arg;



	

	char outputFromClient[MAX_BUFFER_SIZE];



	int istemciKontrol = 0;

	

	//isim girme islemi 

	if (recv(usr->socket, name, 32, 0) <= 0)

	{

		printf("Bir kullanıcı adı girmediniz!!\n");

		istemciKontrol = 1;

	}

	else

	{

		strcpy(usr->name, name);

		sprintf(outputFromClient, "%s baglandi.\n", usr->name);

		printf("%s", outputFromClient);

	}

	bzero(outputFromClient, MAX_BUFFER_SIZE); // bufferi temizler

	

		char recv_bisey[MAX_BUFFER_SIZE];

		char password[MAX_BUFFER_SIZE];



	while (1)	//-exit olasıya kadar devam et

	{

		if (istemciKontrol)

		{

			break;

		}

		

		int recv = recv(usr->socket, outputFromClient, MAX_BUFFER_SIZE, 0);

		strcpy(recv_bisey, outputFromClient);

		

		if (recv > 0) 

		{

			if (strlen(outputFromClient) > 0)

			{

				if (strncmp(outputFromClient, "-", 1) == 0)

				{

					argIn = 0;

					argument = strtok(outputFromClient, " ");

					

					while (argument != NULL)// "\n" girilene kadar devam 

					{

						arguments[argIn] = argument;

						argIn++;

						argument = strtok(NULL, "\n");

					}

					

		

					

				

					if (strcmp(arguments[0], "-open") == 0 && arguments[1] != NULL)	   // oda yaratma

					{

						if (strcmp(arguments[1], "lobi") == 0)

    						{

        						roomStruct *lobi = (roomStruct*)malloc(sizeof(roomStruct));

        						strcpy(lobi->name, arguments[1]);



        						creatRoom(lobi);



        						grpIn = findGroupIndexByName(lobi->name);



        						joinClientGroup(usr, grpIn);

							strcpy(grupName, lobi->name);

       						 	char infoMsg[MAX_BUFFER_SIZE];

        						strcpy(infoMsg, "Lobi Sohbet Odasina hosgeldiniz.\n");

        						sendMsg(infoMsg, usr->userid);



    						}

    						else

    						{

							char msg3[MAX_BUFFER_SIZE];

							strcpy(msg3, "Sıfre olusuturun : ");

							sendMsg(msg3, usr->userid);

							int receivePassword = recv(usr->socket, password, MAX_BUFFER_SIZE, 0);

							if (receivePassword > 0)

							{

								if (strlen(password) > 0)

								{

								roomStruct *privateGroup = (roomStruct *)malloc(sizeof(roomStruct));

								strcpy(privateGroup->name, arguments[1]);

								strcpy(privateGroup->password, password);

								creatRoom(privateGroup);

								grpIn = findGroupIndexByName(privateGroup->name);

								joinClientGroup(usr, grpIn);

								

								char infoMsg[MAX_BUFFER_SIZE];

								strcpy(infoMsg, "Ozel \t~");

								strcat(infoMsg, privateGroup->name);

								strcpy(grupName, privateGroup->name);

								strcat(infoMsg, "~\todasına hosgeldiniz.\n");

								

								//oda sahibi ata

								strcpy(odaSahibi, usr->name);

								

								sendMsg(infoMsg, usr->userid);

								bzero(password, MAX_BUFFER_SIZE);

								

								/*

								//odaya göre mevcut kullanıcı sayısı bulma

								for(int i =0; i<MAX_ROOM; ++i)

								{ 

								if(strcmp(rooms[i]->name, grupName) != 0)

								{

								clientInRoom[i]++ ;

								}

								}*/

								

								}

								else

								{

								char pswMsg[MAX_BUFFER_SIZE];

								strcpy(pswMsg, "Sifre uzunlugu 0'dan buyuk olmalıdır!!\n");

								sendMsg(pswMsg, usr->userid);

								}

							}

						}

					}

					

					

					else if (strcmp(arguments[0], "-join") == 0 && arguments[1] != NULL )   //mevcut odaya katilma

					{

						

						char msg3[MAX_BUFFER_SIZE];

						

						if (findGroupIndexByName(arguments[1]) != -1)

						{

							

							strcpy(msg3, "Sifre Gririn : ");

							sendMsg(msg3, usr->userid);

							int receivePassword = recv(usr->socket, password, MAX_BUFFER_SIZE, 0);

							

							if (strcmp(rooms[findGroupIndexByName(arguments[1])]->password, password) == 0)  //sifre dogruysa

							{

								joinClientGroup(usr, findGroupIndexByName(arguments[1]));

								strcpy(msg3, "Oda ");

								strcat(msg3, rooms[findGroupIndexByName(arguments[1])]->name);

								strcpy(grupName, rooms[findGroupIndexByName(arguments[1])]->name);

								strcat(msg3, " 'ya Hosgeldiniz... \n");

								sendMsg(msg3, usr->userid);

								bzero(msg3, MAX_BUFFER_SIZE);

								strcpy(msg3, "\t\t ~ ");

								strcat(msg3, usr->name);

								strcat(msg3, " ~ Gruba Katildi!\n");

								sendMessgClins(msg3, usr->userid);

								bzero(password, MAX_BUFFER_SIZE);

								

								/*

								//odaya göre mevcut kullanıcı sayısı bulma

								for(int i =0; i<MAX_ROOM; ++i)

								{ 

								if(strcmp(rooms[i]->name, grupName) == 0)

								{

								clientInRoom[i]++ ;

								sprintf(clientInRoomStr, "%d", clientInRoom[i]);

								strcpy(msg3, " *-*-* Bu odada mevcut kullanici sayısı: ");

								strcat(msg3, clientInRoomStr);

								

								strcat(msg3, " \n");

								sendMsg(msg3, usr->userid);

								sendMessgClins(msg3, usr->userid);

								}

								}*/

								

							}



							else

							{

								bzero(msg3, MAX_BUFFER_SIZE);

								bzero(password, MAX_BUFFER_SIZE);

								strcpy(msg3, "Hatali sifre girdiniz!!\n");

								sendMsg(msg3, usr->userid);

							}

						}



						

						else  //grup yoksa

						{

							strcpy(msg3, "Bu isimde bir grup bulunmamaktadir!\n");

							sendMsg(msg3, usr->userid);

						}

					}

					

					

					//bulundugun odada kimler var sorgula

					else if (strcmp(arguments[0], "-usersInRoom") == 0 && arguments[1] != NULL )

					{

		

						char mesg[MAX_BUFFER_SIZE];

						

						if (findGroupIndexByName(arguments[1]) != -1)

						{

							

							strcpy(mesg, "Oda ");

							strcat(mesg, rooms[findGroupIndexByName(arguments[1])]->name);

							strcat(mesg, " 'da bulunan kisiler: \n");

							bzero(mesg, MAX_BUFFER_SIZE);

								

					

							//odaya göre mevcut kullanıcı sayısı bulma

							for(int i =0; i<MAX_GRUP_KAPASITESI; ++i)

							{ 

							strcpy(mesg,"\0");

								if(rooms[findGroupIndexByName(arguments[1])]->clientsInGroup[i]->userid != usr->userid)

								{

								strcat(mesg, "\n --- Kullanici Adı :  ");

            							strcat(mesg, clients[i]->name);

            							strcat(mesg, "\n");

            							

            							sendMsg(mesg, usr->userid);

            							bzero(mesg, MAX_BUFFER_SIZE);

            							

								}

								else if(clients[0]==NULL)

								{

								strcpy(mesg, "Bu odada mevcut kullanici yoktur.\n");

								sendMsg(mesg, usr->userid);

								bzero(mesg, MAX_BUFFER_SIZE);

								break;

								}

							}

								

						}

						else  //grup yoksa

						{

							strcpy(mesg, "Bu isimde bir grup bulunmamaktadir!\n");

							sendMsg(mesg, usr->userid);

							bzero(mesg, MAX_BUFFER_SIZE);

						}

					}

	



					//Kullanici listele

					else if (strcmp(arguments[0], "-listUsers") == 0 )

					{

						

						char messg[MAX_BUFFER_SIZE]; 

						

						for (int i = 0; i < MAX_KULLANICI; ++i)

						{

						strcpy(messg,"\0");

							if (clients[i] != NULL)

							{

								

								strcat(messg, "\n --- Kullanici Adı :  ");

            							strcat(messg, clients[i]->name);

            							strcat(messg, "\n");

            							sendMsg(messg, usr->userid);

            							bzero(messg, MAX_BUFFER_SIZE);

							}

							else if(clients[0]==NULL)

							{

								strcpy(messg, "Mevcut kullanici yoktur.\n");

								sendMsg(messg, usr->userid);

								bzero(messg, MAX_BUFFER_SIZE);

								break;

							}

						}

						

						

					}

					

					

					//Oda listele

					else if (strcmp(arguments[0], "-listRooms") == 0 )

					{

						

						char messg[MAX_BUFFER_SIZE];

						strcpy(messg,"\0");

							 for (int i = 0; i < MAX_ROOM; ++i)

    							{

    							

      								if (rooms[i] != NULL)

     								{

            								//printf("%d.Oda Adı: %s\n", i+1, rooms[i]->name);

            								strcat(messg, "\n --- Oda Adı :  ");

            								strcat(messg, rooms[i]->name);

            								strcat(messg, "\n");

            								sendMsg(messg, usr->userid);

            								bzero(messg, MAX_BUFFER_SIZE);

       								}

       								else if(rooms[0]==NULL)

								{

									strcpy(messg, "Mevcut oda yoktur.\n");

									sendMsg(messg, usr->userid);

									bzero(messg, MAX_BUFFER_SIZE);

									break;

								}

  							}

					}

					

					//mevcut odan cikma islemi

					else if (strcmp(arguments[0], "-left") == 0)  

					{

						grpIn = findGroupIndexByClientId(usr->userid);

						char infoMsg[MAX_BUFFER_SIZE];

						strcpy(infoMsg, "Sohbetten ayrildin!\n");

						sendMsg(infoMsg, usr->userid);

						bzero(infoMsg, MAX_BUFFER_SIZE);

						strcpy(infoMsg, "\t\t");

						strcat(infoMsg, usr->name);

						strcat(infoMsg, " sohbetten ayrildi!");

						strcat(infoMsg, "\n");

						strcpy(grupName, "0");

						sendMessgClins(infoMsg, usr->userid);

						leftClientGroup(usr->userid, grpIn);

						

						/*clientInRoom--;

						strcpy(infoMsg, "  *-*-* Bu odada mevcut kalan kişi sayısı: ");

						sprintf(clientInRoomStr, "%d", clientInRoom);

						strcat(infoMsg, clientInRoomStr);

						strcat(infoMsg, " \n");

						sendMessgClins(infoMsg, usr->userid);*/

						

					}

					

					 //mevcut odayi kapatma

					else if (strcmp(arguments[0], "-closeThisRoom") == 0 && strcmp(arguments[1], odaSahibi) == 0 ) 

					{

		

		 			grpIn = findGroupIndexByClientId(usr->userid);



    					// Odanın indeksini bulun

   					 int roomIndex = grpIn;

	

    						if (roomIndex != -1)

    						{

        					char infoMsg[MAX_BUFFER_SIZE];

        					strcpy(infoMsg, "Odayi kapattın!\n");

        					sendMsg(infoMsg, usr->userid);



        						// Odadaki tüm kullanıcıları çıkarın

        						for (int i = 0; i < MAX_GRUP_KAPASITESI; i++)

       						 	{

            							if (rooms[roomIndex]->clientsInGroup[i])

            							{

               							 int userid = rooms[roomIndex]->clientsInGroup[i]->userid;

             							   leftClientGroup(userid, roomIndex);

            							}

       						 	}



       						 // Odayı kapatın

    	   					closeRoom(rooms[roomIndex]->name);

   						 }

   					 

    						else

    						{

       						 // Odanın indeksi bulunamadı

       						 char errorMsg[MAX_BUFFER_SIZE];

       						 strcpy(errorMsg, "Oda bulunamadi!\n");

       						 sendMsg(errorMsg, usr->userid);

  						  }

						

					}

	

					//mesaj gonderme islemi

					else if (strcmp(arguments[0], "-send") == 0 && arguments[1] != NULL )

					{

						char msg[MAX_BUFFER_SIZE];

						strcpy(msg, "\t\t");

						strcat(msg, usr->name);

						strcat(msg, " : ");

						strcat(msg, arguments[1]);

						strcat(msg, "\n");

						sendMessgClins(msg, usr->userid);

					}

					

					//hangi odadasın sorgula

					else if (strcmp(arguments[0], "-whereami") == 0 ) 

					{

						char msg2[MAX_BUFFER_SIZE]; 

						if(strcmp(grupName, "0") !=0 && strcmp(grupName, "NULL") != 0 )

						{

							strcpy(msg2, "Bulundugun oda adi  : ");

							strcat(msg2, grupName);

							strcat(msg2, "\n");

							sendMsg(msg2, usr->userid);

						}

						else 

						{

							strcpy(msg2, "Bir gruba dahil degilsiniz!\n");

							sendMsg(msg2, usr->userid);

						}

						

					}

					

					//kim oldugunu sorgula

					else if (strcmp(arguments[0], "-whoami") == 0)

					{

						char msg2[MAX_BUFFER_SIZE];

						strcpy(msg2, "Katildigin kimligin  : ");

						strcat(msg2, usr->name);

						strcat(msg2, "\n");

						sendMsg(msg2, usr->userid);

					}

					





				}

			}

		}



		else if (recv == 0) //odadan ciktiginda kontrol saglar

		{

			char msg[MAX_BUFFER_SIZE];

			strcpy(msg, usr->name);

			strcat(msg, " ayrildi!\n");

			printf("%s", msg);

			istemciKontrol = 1;

		}

		bzero(outputFromClient, MAX_BUFFER_SIZE);

	}

	

	close(usr->socket);

	

	

	 //diziden client silme islemi

	for (int i = 0; i < MAX_KULLANICI; ++i)

	{

		if (clients[i])

		{

			if (clients[i]->userid == usr->userid)

			{

				clients[i] = NULL;

				break;

			}

		}

	}

	

	free(usr);

	clientCounter--;

	return NULL;

}



int main(int argumCounter, char **argumVector)

{

	int lst = 0, baglanmaIslemi = 0;

	

	struct sockaddr_in server;

	struct sockaddr_in cli_addr;

	

	pthread_t servertid;

	
	//socet olusturma
	lst = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(4444);

	
	//baglanma
	if (bind(lst, (struct sockaddr *)&server, sizeof(server)) < 0)

	{

		perror("HATA: Socket baglanmadı!");

		return EXIT_FAILURE;

	}

	if (listen(lst, 10) < 0) //dinleme

	{

		perror("HATA: Socket dinlenemedi!");

		return EXIT_FAILURE;

	}

	printf("ChatAPP Online\n");

	

	while (1)

	{

		char outputFromClient[MAX_BUFFER_SIZE];

		char name[32];

		

		socklen_t usrlent = sizeof(cli_addr);

		

		//baglanma islemi

		baglanmaIslemi = accept(lst, (struct sockaddr *)&cli_addr, &usrlent);

		

		// Client init

		clientStruct *usr = (clientStruct *)malloc(sizeof(clientStruct));

		

		usr->address = cli_addr;

		usr->socket = baglanmaIslemi;

		usr->userid = userid++;

		addCliToServ(usr);

		

		//thread yaratma

		pthread_create(&servertid, NULL, &server_client, (void *)usr);

		sleep(1);

	}

	return 1;

}
