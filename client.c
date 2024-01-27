#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX 30

int main() {
    int clientSocket, flag = 0, contactCount, j;
    struct sockaddr_in serverAddr;
    char message[BUFFER_SIZE];
    int IDtemp, result;
    int id;
    int cont;
    int opt;
    char Cont[] = "Cont";
    char* messages = "Messages";
    char* unread = "Unread";
    char tempConvert[MAX], temp[MAX], Messages[MAX], Unread[MAX];
    FILE* fp;
    int friendID;

    // Socket oluþtur
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Sunucu adresini ayarla
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Sunucuya baðlan
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server...\n");
    
    //Kullanýcýdan ID al
    printf("Enter the ID: ");
    scanf("%d", &id);
    
    //ID'yi gonder
    if (send(clientSocket, &id, sizeof(id), 0) == -1) {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
    //ID kontrol sonucunu al
    if (recv(clientSocket, &cont, sizeof(cont), 0) == -1) {
        perror("Error receiving response");
        exit(EXIT_FAILURE);
    }
    
    if(cont==1)
		flag = 1;
	else
		flag = 0;
    
    //ID GECERLIYSE DEVAM ET
    while(flag){
    	printf("1. List Contacts\n2. Add User\n3. Delete User\n4. Send Message\n5. Check Messages\n6. Display all message\n7. Quit ");
    	scanf("%d", &opt);
		if (send(clientSocket, &opt, sizeof(opt), 0) == -1) {
	        perror("Error sending message");
	        exit(EXIT_FAILURE);
	    }
	    switch(opt){
	    	case 1: //List contact
	    		int tempID, numOfLines, ii;
	    		//Kisi sayisini al
			    if (recv(clientSocket, &numOfLines, sizeof(numOfLines), 0) == -1) {
			        perror("Error receiving response");
			        exit(EXIT_FAILURE);
			    }
			    for(ii=0;ii<numOfLines;ii++){
			    	if (recv(clientSocket, &tempID, sizeof(tempID), 0) == -1) {
				        perror("Error receiving response");
				        exit(EXIT_FAILURE);
			    	}
			    	printf("%d\n", tempID);
				}
		    	/*while (recv(clientSocket, &tempID, sizeof(tempID), 0) > 0) {
		    		printf("%d\n", tempID);
			    }*/
	    	break;
	    	
	    	case 2: //Kiþi ekleme
				printf("Enter the ID do you want to add: ");
				scanf("%d", &IDtemp);
				if (send(clientSocket, &IDtemp, sizeof(IDtemp), 0) == -1) {
			        perror("Error sending message");
			        exit(EXIT_FAILURE);
			    }
			    //Bu kisinin zaten ekli olup olmadýgý sonucu
			    if (recv(clientSocket, &result, sizeof(result), 0) == -1) {
				    perror("Error receiving response");
				    exit(EXIT_FAILURE);
				}
				if(result){
					printf("This user is already in contact list!\n");
				}
	    	break;
	    	
	    	case 3: //Kiþi silme
				printf("Enter the ID do you want to delete: ");
				scanf("%d", &IDtemp);
				if (send(clientSocket, &IDtemp, sizeof(IDtemp), 0) == -1) {
			        perror("Error sending message");
			        exit(EXIT_FAILURE);
			    }
			    //Bu kisinin ekli olup olmadýgý sonucu
			    if (recv(clientSocket, &result, sizeof(result), 0) == -1) {
				    perror("Error receiving response");
				    exit(EXIT_FAILURE);
				}
				if(!result){
					printf("This user is not already in contact list!\n");
				}
	    	break;
	    	
	    	case 4: //Mesaj gönder
	    		int innerFlag = 1, result;
				printf("Who do you want to send a message to? ");
				scanf("%d", &friendID);
				if (send(clientSocket, &friendID, sizeof(friendID), 0) == -1) {
				    perror("Error sending message");
				    exit(EXIT_FAILURE);
				}
				//Serverden donen result deðeri kiþinin ekli olup olmadýgýný soyler
				if (recv(clientSocket, &result, sizeof(result), 0) == -1) {
				    perror("Error receiving response");
				    exit(EXIT_FAILURE);
				}
				if(result){
					while(innerFlag){
						memset(message, 0, BUFFER_SIZE);
						printf("Type: ");
					    scanf(" %[^\n]", message);
						if (send(clientSocket, message, sizeof(message), 0) == -1) {
							perror("Error receiving response");
							exit(EXIT_FAILURE);
						}
						if(strncmp(message, "exit", 4)==0)
					    	innerFlag = 0;
					}
				}
				else
					printf("This id is not in your contact list!\n");
	    	break;
	    	
	    	case 5: //Check messages
	    		strcpy(Unread, unread);
				memset(tempConvert, 0, MAX);
				sprintf(tempConvert, "%d", id);
				strcat(Unread, tempConvert);
				strcat(Unread, ".txt");
				fp = fopen(Unread, "r");
				if(fp==NULL) {printf("File error"); exit(1);}
				fseek(fp, 0, SEEK_END);
				long fileSize = ftell(fp);
				if(fileSize==0)
					printf("There is no unread message\n");
				else{
					printf("You have unread messages:\n");
					fclose(fp);
					fp = fopen(Unread, "r");
					if(fp==NULL) {printf("File error"); exit(1);}
					char temptemp[MAX];
					//Unread.txt'nin içini yazdýrma
					while(!feof(fp)){
						fscanf(fp, " %[^\n]s", temptemp);
						printf("%s\n", temptemp);
					}
					fclose(fp);
					//Unread.txt içindekileri silme
					fp = fopen(Unread, "w");
					if(fp==NULL) {printf("File error"); exit(1);}
					fclose(fp);
				}
	    	break;
	    	
	    	case 6: //Display message history
	    		int bigUser, smallUser;
				printf("Whose messages do you want to read? ");
				scanf("%d", &friendID);
				if(id>friendID){
					bigUser = id;
					smallUser = friendID;
				}
				else{
					bigUser = friendID;
					smallUser = id;
				}
				memset(tempConvert, 0, MAX);
				strcpy(Messages, messages);
				sprintf(tempConvert, "%d", smallUser);
				strcat(Messages, tempConvert);
				strcat(Messages, "-");
				memset(tempConvert, 0, MAX);
				sprintf(tempConvert, "%d", bigUser);
				strcat(Messages, tempConvert);
				strcat(Messages, ".txt");
				FILE* ffpp = fopen(Messages, "r");
				if(ffpp==NULL) {printf("File error"); exit(1);}
				char readed[MAX];
				while(!feof(ffpp)){
					fscanf(ffpp, " %[^\n]s", readed);
					printf("%s\n", readed);
				}
				fclose(ffpp);
	    	break;
	    	
	    	case 7:
	    		flag = 0;
	    	break;
		}
	}
	
	if(cont==0)
		printf("Invalid ID\n");
	
    printf("Disconnected.\n");

    // Soketi kapat
    close(clientSocket);

    return 0;
}

