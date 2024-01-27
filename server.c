#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX 30

//id.txt içinde var mý
int idTxtControl(int id){
	char* c = "id.txt";
	FILE* fp = fopen(c, "r");
	if(fp==NULL){printf("File error"); exit(1);}
	int flag = 0, idtemp;
	while(!feof(fp) && !flag){
		fscanf(fp, "%d", &idtemp);
		if(idtemp==id)
			flag = 1;
	}
	//Varsa 1 yoksa 0
	return flag;
}

//Kullanýcýnýn kontaðýnda bu ID var mý yok mu
int contControl(int id, int dest){
	char* c = "Cont";
	char file[MAX], temp[MAX];
	sprintf(temp, "%d", id);
	strcpy(file, c);
	strcat(file, temp);
	strcat(file, ".txt");
	FILE* fp = fopen(file, "r");
	if(fp==NULL){printf("File error"); exit(1);}
	int flag = 0, idtemp;
	while(!feof(fp) && !flag){
		fscanf(fp, "%d", &idtemp);
		if(idtemp==dest)
			flag = 1;
	}
	fclose(fp);
	//Varsa 1 yoksa 0
	return flag;
}

void appendToTxt(char* fileName, int id){
	FILE* fp13 = fopen(fileName, "a");
	if(fp13==NULL){printf("File error"); exit(1);}
	fseek(fp13, 0, SEEK_END);
	long fileSize = ftell(fp13);
	if(fileSize==0)
		fprintf(fp13, "%d", id);
	else
		fprintf(fp13, "\n%d", id);
	fclose(fp13);
}

void deleteContact(char* fileName, int idToDelete){
	int arr[MAX];
	int arrSize = 0;
	FILE* fp = fopen(fileName, "r");
	if(fp==NULL){printf("File error"); exit(1);}
	int tempid;
	while(!feof(fp)){
		fscanf(fp, "%d", &tempid);
		if(tempid!=idToDelete)
			arr[arrSize++] = tempid;
	}
	int i;
	fclose(fp);
	fp = fopen(fileName, "w");
	if(fp==NULL){printf("File error"); exit(1);}
	fprintf(fp, "%d", arr[0]);
	for(i=1;i<arrSize;i++)
		fprintf(fp, "\n%d", arr[i]);
	fclose(fp);
}

//Tarih - Saat al
char* getTime() {
    char* formattedTime = (char*)calloc(MAX, sizeof(char));
    time_t currentTime;
    struct tm *timeInfo;
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(formattedTime, MAX, "%d-%m-%Y %H:%M:%S", timeInfo);
    return formattedTime;
}

int getNumOfLines(char* fileName){
	FILE* fp = fopen(fileName, "r");
	if(fp==NULL){printf("File Error!"); exit(1);}
	
	int lines=0;
	char line[256];
	
	while(fgets(line, sizeof(line), fp)!=NULL)
		lines++;
	
	fclose(fp);
	return lines;
}

//thread
void* handle_client(void *client_socket){
	int socket = *(int*)client_socket;
	int numOfLines;
    char buffer[BUFFER_SIZE];
    FILE* fp;
    FILE* fp2;
    char* dosyaID = "id.txt";
    int temp, clientID, destID, opt, result;
    char* cont = "Cont";
    char Cont[MAX];
    char* messages = "Messages";
    char Messages[MAX];
    char* unread = "Unread";
    char Unread[MAX], tempConvert[MAX], charTemp[MAX], time[MAX];
    char originalMessage[BUFFER_SIZE];
    
    if (recv(socket, &clientID, sizeof(clientID), 0) == -1) {
	    perror("Error receiving message");
	    exit(EXIT_FAILURE);
	}
	
	//ID kontrol
	fp = fopen(dosyaID, "r");
	if(fp==NULL){
	    printf("File error.");
	    exit(1);
	}
	
	int flag;
	int isValid = 0;
	
	while(!feof(fp) && !isValid){
		fscanf(fp, "%d", &temp);
		if(clientID == temp)
			isValid = 1;
	}
	
	fclose(fp);
		
	if(isValid)
		flag = 1;
	else
		flag = 0;
	
	//Kullanýcýya ID kontrol sonucunu döndür
	if (send(socket, &flag, sizeof(flag), 0) == -1) {
	    perror("Error sending message");
	    exit(EXIT_FAILURE);
	}
	
	while(flag){
		//Islemi al
		if (recv(socket, &opt, sizeof(opt), 0) == -1) {
	        perror("Error receiving response");
	        exit(EXIT_FAILURE);
    	}
    	int j;
    	switch(opt){
    		case 1: //Contactlarý tek tek gonder
	    		memset(tempConvert, 0, MAX);
				sprintf(tempConvert, "%d", clientID);
				strcpy(Cont, cont);
				strcat(Cont, tempConvert);
				strcat(Cont, ".txt");
				numOfLines = getNumOfLines(Cont);
				//Ilk once kisi sayisini gonder
				if (send(socket, &numOfLines, sizeof(numOfLines), 0) == -1) {
				    perror("Error sending message");
				    exit(EXIT_FAILURE);
				}
				int tempID;
	    		fp = fopen(Cont, "r");
				if(fp==NULL) {printf("File error"); exit(1);}
				while(!feof(fp)){
					fscanf(fp, "%d", &tempID);
					if (send(socket, &tempID, sizeof(tempID), 0) == -1) {
						perror("Error sending message");
						exit(EXIT_FAILURE);
					}
				}
				fclose(fp);
    		break;
    		
    		case 2: //Add user
				if (recv(socket, &destID, sizeof(destID), 0) == -1) {
			        perror("Error receiving response");
			        exit(EXIT_FAILURE);
		    	}
		    	result = contControl(clientID, destID);
		    	//Result deðerini kullanýcýya dondur
				if (send(socket, &result, sizeof(result), 0) == -1) {
				    perror("Error sending message");
				    exit(EXIT_FAILURE);
				}
				//Ekli deðilse ekle
				if(!result){
					strcpy(Cont, cont);
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", clientID);
					strcat(Cont, tempConvert);
					strcat(Cont, ".txt");
					appendToTxt(Cont, destID);
					
					strcpy(Cont, cont);
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", destID);
					strcat(Cont, tempConvert);
					strcat(Cont, ".txt");
					appendToTxt(Cont, clientID);
					
					int resultTemp = idTxtControl(destID);
					if(!resultTemp)
						appendToTxt("id.txt", destID);
				}
    		break;
    		
    		case 3: //Delete user
				if (recv(socket, &destID, sizeof(destID), 0) == -1) {
			        perror("Error receiving response");
			        exit(EXIT_FAILURE);
		    	}
		    	result = contControl(clientID, destID);
		    	//Result deðerini kullanýcýya dondur
				if (send(socket, &result, sizeof(result), 0) == -1) {
				    perror("Error sending message");
				    exit(EXIT_FAILURE);
				}
				//Ekliyse Sil
				if(result){
					strcpy(Cont, cont);
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", clientID);
					strcat(Cont, tempConvert);
					strcat(Cont, ".txt");
					deleteContact(Cont, destID);
					
					strcpy(Cont, cont);
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", destID);
					strcat(Cont, tempConvert);
					strcat(Cont, ".txt");
					deleteContact(Cont, clientID);
				}
    		break;
    		
    		case 4: //Send Message
	    		int smallUser, bigUser;
				memset(originalMessage, 0, BUFFER_SIZE);
				int innerFlag = 1, friendID;
				//kime gonderilceginin id'si
				if (recv(socket, &friendID, sizeof(friendID), 0) == -1) {
				    perror("Error receiving message");
				    exit(EXIT_FAILURE);
				}
				//Bu id senin kontagýn mý??
				result = contControl(clientID, friendID);
				//Result deðerini kullanýcýya dondur
				if (send(socket, &result, sizeof(result), 0) == -1) {
				    perror("Error sending message");
				    exit(EXIT_FAILURE);
				}
				
				if(result){
					//IDleri sýralama
					if(clientID>friendID){
						bigUser = clientID;
						smallUser = friendID;
					}
					else{
						bigUser = friendID;
						smallUser = clientID;
					}
					//acýlacak txt uzantýlý dosyaya erisme
					memset(tempConvert, 0, MAX);
					strcpy(Messages, messages);
					sprintf(tempConvert, "%d", smallUser);
					strcat(Messages, tempConvert);
					strcat(Messages, "-");
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", bigUser);
					strcat(Messages, tempConvert);
					strcat(Messages, ".txt");
					fp = fopen(Messages, "a");
					if(fp==NULL) {printf("File error"); exit(1);}
					strcpy(Unread, unread);
					memset(tempConvert, 0, MAX);
					sprintf(tempConvert, "%d", friendID);
					strcat(Unread, tempConvert);
					strcat(Unread, ".txt");
					fp2 = fopen(Unread, "a");
					if(fp2==NULL) {printf("File error"); exit(1);}
					//while icinde mesajlarý alýyoruz
					while(innerFlag){
					    memset(buffer, 0, BUFFER_SIZE);
					    if (recv(socket, buffer, BUFFER_SIZE, 0) == -1) {
					        perror("Error receiving message");
					        exit(EXIT_FAILURE);
					    }
					    if(strncmp(buffer, "exit", 4)==0)
					    	innerFlag = 0;
					    strcpy(time, getTime());
					    strcat(time, "->");
					    memset(tempConvert, 0, MAX);
						sprintf(tempConvert, "%d", clientID);
						strcat(time, "[");
						strcat(time, tempConvert);
						strcat(time, "]");
						strcat(time, ":");
					    
					    strcat(originalMessage, time);
					    strcat(originalMessage, buffer);
					    fprintf(fp, "%s\n", originalMessage);
					    fprintf(fp2, "%s\n", originalMessage);
					
						if(innerFlag)
					        printf("%d sent a message\n", clientID);
					    else
					        printf("%d exited from message section.\n", clientID);
					    memset(originalMessage, 0, BUFFER_SIZE);
					}
					fclose(fp);
					fclose(fp2);
				}
			break;
			
    		case 7: //Quit
    			flag = 0;
    		break;
		}
	}
	
	printf("%d is disconnected\n", clientID);
	
	close(socket);
	pthread_exit(NULL);
}	

//main
int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];
    FILE* fp;
    char* dosyaID = "id.txt";
    int temp;
    int clientID;
    pthread_t thread_id[10];
    int i = 0;

    // Socket oluþtur
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Sunucu adresi ayarla
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Socket'i baðla
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Socket'i dinle
    if (listen(serverSocket, 10) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
	
	
	while(1){
	    // Baðlantý bekleniyor
	    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
	    if (clientSocket == -1) {
	        perror("Error accepting connection");
	        exit(EXIT_FAILURE);
	    }
	    
	    printf("New connection\n");
	
		if(pthread_create(&thread_id[i++], NULL, handle_client, (void *)&clientSocket)!=0){
			perror("Thread error");
			exit(1);
		}
	}
    // Soketleri kapat
    close(serverSocket);

    return 0;
}

