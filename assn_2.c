//
//  main.c
//  ADSAssignment2
//
//  Created by Shreyas Zagade on 2/12/18.
//  Copyright © 2018 Shreyas Zagade. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* studentFileName = "";
char* stratergy = "--first-fit";

char* indexFileName = "primaryIndexFile.db";
char* availabilityFileName = "availabilityFile.db";

char* getFromFile(long primaryKey,FILE* studentFile);
void compactPrimaryIndex();
void compactAvailableList();

//Primary Index
typedef struct { int key; /* Record's key */ long off; /* Record's offset in file */ } index_S;
index_S *primaryIndex;// = (index_S*)malloc(sizeof(index_S) * 10);
int primaryIndexLength;// = 10;
int primaryIndexCurrentIndex;// = 0;

//Student File
int studentFileEndOffset = 0;

//Availability List
typedef struct { int siz; /* Hole's size */ long off; /* Hole's offset in file */ } avail_S;
avail_S *availableList;// = (index_S*)malloc(sizeof(index_S) * 10);
int availableListLength;// = 10;
int availableListCurrentIndex;// = 0;

void initPrimaryIndex(){
    primaryIndex = (index_S*)malloc(sizeof(index_S) * 10);
    primaryIndexLength = 10;
    primaryIndexCurrentIndex = 0;
}

void initAvailableList(){
    availableList = (avail_S*)malloc(sizeof(avail_S) * 10);
    availableListLength = 10;
    availableListCurrentIndex = 0;
}

int ascendingCompareFunction(const void * a, const void * b) {
    return ( (*(index_S*)a).key - (*(index_S*)b).key );
}
int ascendingCompareForAvailability(const void * a, const void * b) {
    return ( (*(avail_S*)a).siz - (*(avail_S*)b).siz );
}
int descendingCompareForAvailability(const void * a, const void * b) {
    return ( (*(avail_S*)b).siz - (*(avail_S*)a).siz );
}

void sortAppropriately(){
    if (strcmp(stratergy,"--first-fit") == 0) {
    }else if (strcmp(stratergy,"--best-fit") == 0){
        qsort(availableList, availableListCurrentIndex, sizeof(avail_S), ascendingCompareForAvailability);
    }else if(strcmp(stratergy,"--worst-fit") == 0){
        qsort(availableList, availableListCurrentIndex, sizeof(avail_S), descendingCompareForAvailability);
    }
}

void addToPrimaryIndex(int primaryKey, long offset){
    if (primaryIndexCurrentIndex == primaryIndexLength) {
        primaryIndex = realloc(primaryIndex, sizeof(index_S) * (primaryIndexLength + 20));
        if(primaryIndex == NULL){
            printf("Realloc issue ");
            return;
        }
        primaryIndexLength += 20;
    }
    
    index_S newElement;
    newElement.key = primaryKey;
    newElement.off = offset;
    
    primaryIndex[primaryIndexCurrentIndex++] = newElement;
    
    qsort(primaryIndex, primaryIndexCurrentIndex, sizeof(index_S), ascendingCompareFunction);
}

void addToAvailableList(int size, long offset){
    if (availableListCurrentIndex == availableListLength) {
        availableList = realloc(availableList, sizeof(avail_S) * (availableListLength + 20));
        if(availableList == NULL){
            printf("Realloc issue ");
            return;
        }
        availableListLength += 20;
    }
    
    avail_S newElement;
    newElement.siz = size;
    newElement.off = offset;
    
    availableList[availableListCurrentIndex++] = newElement;
    
}

void loadPrimaryIndex(){
    initPrimaryIndex();
    FILE* primaryIndexFile = fopen(indexFileName, "rb+");
    index_S entry;
    fread(&studentFileEndOffset, sizeof(int), 1, primaryIndexFile);
    while (fread(&entry, sizeof(index_S), 1, primaryIndexFile)) {
        addToPrimaryIndex(entry.key, entry.off);
    }
}

void loadAvailabilityIndex(){
    initAvailableList();
    FILE* availabilityFile = fopen(availabilityFileName, "rb+");
    avail_S entry;
    while (fread(&entry, sizeof(avail_S), 1, availabilityFile)) {
        addToAvailableList(entry.siz, entry.off);
    }
}

void printPrimaryIndex(){
    printf("Index:\n");
    int  i = 0;
    for(i = 0; i < primaryIndexCurrentIndex; i++){
        printf("key=%d: offset=%ld\n",primaryIndex[i].key,primaryIndex[i].off);
    }
}

void printAvailableList(){
    sortAppropriately();
    printf("Availability:\n");
    int  i = 0;
    int holes = 0;
    int size = 0;
    for(i = 0; i < availableListCurrentIndex; i++){
        printf("size=%d: offset=%ld\n",availableList[i].siz,availableList[i].off);
        holes++;
        size += availableList[i].siz;
    }
    printf("Number of holes: %d\n",holes);
    printf("Hole space: %d\n",size);
}

index_S* binarySearch(long key, long left, long right, index_S* primaryIndex){
    if(left > right){
        return NULL;
    }else{
        long mid = (left + right) / 2;
        if(primaryIndex[mid].key == key){
            return &primaryIndex[mid];
        }else if(key < primaryIndex[mid].key){
            return binarySearch(key, left, mid-1, primaryIndex);
        }else{
            return binarySearch(key, mid+1, right, primaryIndex);
        }
    }
    return NULL;
}

index_S* findInPrimaryIndex(long key){
    return binarySearch(key,0,primaryIndexCurrentIndex,primaryIndex);
}

char* savePrimaryIndex(){
    remove(indexFileName);
    FILE* primaryIndexFile = fopen(indexFileName, "ab+");
    if(primaryIndexFile == NULL){
        printf("Not able to create new File\n");
        return NULL;
    }
    fseek(primaryIndexFile, 0, SEEK_SET);
    
    fwrite(&studentFileEndOffset, sizeof(int), 1, primaryIndexFile);
    
    int i = 0;
    for (i = 0; i < primaryIndexCurrentIndex; i++) {
        fwrite(&primaryIndex[i], sizeof(index_S), 1, primaryIndexFile);
    }
    fclose(primaryIndexFile);
    return indexFileName;
}

char* saveAvability(){
    remove(availabilityFileName);
    FILE* availabitiyFile = fopen(availabilityFileName, "ab+");
    if(availabitiyFile == NULL){
        printf("Not able to create new File\n");
        return NULL;
    }
    fseek(availabitiyFile, 0, SEEK_SET);
    
    int i = 0;
    for (i = 0; i < availableListCurrentIndex; i++) {
        fwrite(&availableList[i], sizeof(avail_S), 1, availabitiyFile);
    }
    fclose(availabitiyFile);
    return indexFileName;
}

long firstFit(long sizeRequired){
    int i =0;
    for (i = 0; i < availableListCurrentIndex; i++) {
        if(availableList[i].siz >= sizeRequired){
            long offset = availableList[i].off;
            int sizeAvailable = availableList[i].siz;
            availableList[i].off = -1;
            availableList[i].siz = -1;
            compactAvailableList();
            
            avail_S leftOverSpace;
            leftOverSpace.siz = sizeAvailable - sizeRequired;
            leftOverSpace.off = offset + sizeRequired;
            addToAvailableList(leftOverSpace.siz, leftOverSpace.off);
            return offset;
        }
    }
    return studentFileEndOffset;
}

long getFreeSpace(long sizeRequired){
    sortAppropriately();
    return firstFit(sizeRequired);
}

int addToFile(FILE *studentFile,int primaryKey, char entry[], long sizeOfEntry){
    if(studentFile == NULL){
        printf("File Not found or created");
        return 0;
    }else if(findInPrimaryIndex(primaryKey) != NULL){
        printf("Record with SID=%d exists\n",primaryKey);
        return 0;
    }else{
        
        long offsetToAdd = getFreeSpace(sizeOfEntry * sizeof(char) + sizeof(int));//studentFileEndOffset;
        if (offsetToAdd == studentFileEndOffset) {
            studentFileEndOffset += sizeOfEntry + sizeof(int);//If End of File Increase Offset of the end of file
        }
        
        fseek(studentFile, offsetToAdd, SEEK_SET);//Replacement Policy
        
        int retVal = fwrite(&sizeOfEntry,sizeof(int),1,studentFile);
        int someVal = fwrite(entry,sizeof(char),strlen(entry),studentFile);
        addToPrimaryIndex(primaryKey, offsetToAdd);
        
        return 1;
    }
}

int deleteFromFile(FILE *studentFile,int primaryKey){
    index_S* entry = findInPrimaryIndex(primaryKey);
    if(entry == NULL){
        return 0;
    }else{
        char* record = getFromFile(primaryKey, studentFile);
        int totalSize = strlen(record) * sizeof(char) + sizeof(int);
        addToAvailableList(totalSize, (*entry).off);
        (*entry).key = -1;
        (*entry).off = -1;
        compactPrimaryIndex();
        return 1;
    }
}

char* getFromFile(long primaryKey,FILE* studentFile){
    index_S* entry = binarySearch(primaryKey, 0, primaryIndexCurrentIndex, primaryIndex);
    if (entry == NULL) {
        return NULL;
    }
    fseek(studentFile, (*entry).off, SEEK_SET);
    int sizeOfRecord;
    fread( &sizeOfRecord, sizeof(int), 1, studentFile);
    char* readBuffer = (char*) malloc(sizeOfRecord + 4);
    fread( readBuffer, sizeOfRecord, 1, studentFile );
    return readBuffer;
}

void compactPrimaryIndex(){
    int i = 0;
    for (i = 0; i < primaryIndexCurrentIndex; i++) {
        if (primaryIndex[i].key == -1) {
            int j = i;
            while (j >= 0 && j+1 < primaryIndexCurrentIndex) {
                primaryIndex[j] = primaryIndex[j+1];
                j++;
            }
            primaryIndexCurrentIndex -= 1;
            break;
        }
    }
}

void compactAvailableList(){
    int i = 0;
    for (i = 0; i < availableListCurrentIndex; i++) {
        if (availableList[i].siz == -1) {
            int j = i;
            while (j >= 0 && j+1 < availableListCurrentIndex) {
                availableList[j] = availableList[j+1];
                j++;
            }
            availableListCurrentIndex -= 1;
            break;
        }
    }
}

void printFile(FILE *studentFile){
    fseek(studentFile, 0, SEEK_SET);
    char readBuffer[20];
    int sizeOfRecord;
    while(fread( &sizeOfRecord, sizeof(int), 1, studentFile )){
        fread( &readBuffer, sizeOfRecord, 1, studentFile );
        printf("%s \n",readBuffer);
    }
}

void printFileAsIs(FILE *studentFile){
    fseek(studentFile, 0, SEEK_SET);
    char readBuffer[500];
    fread( &readBuffer, 500, 1, studentFile );
    int i = 0;
    for(i = 0; i < 500; i++){
        printf("%c",readBuffer[i]);
    }
}

int checkFileExists(char* filename){
    FILE *file;
    if ((file = fopen(filename, "r"))){
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        printf("Please provide appropriate paramters");
    }else{
        stratergy = argv[1];
        studentFileName = argv[2];
    }

    if (checkFileExists(studentFileName)) {
        loadPrimaryIndex();
        loadAvailabilityIndex();
    }else{
        remove(indexFileName);
        remove(availabilityFileName);
        initPrimaryIndex();
        initAvailableList();
    }
    
    FILE *studentFile = fopen(studentFileName, "ab+");
    fclose(studentFile);
    studentFile = fopen(studentFileName, "rb+");
    while (1) {
        char command[10];
        scanf("%s",command);
        if(strcmp(command, "end") == 0){
            char *fileName = savePrimaryIndex();
            saveAvability();
            break;
        }else if(strcmp(command, "add") == 0){
            int primaryKey;
            char entry[200];
            scanf("%d %s",&primaryKey,entry);
            addToFile(studentFile, primaryKey, entry,strlen(entry));
        }else if(strcmp(command, "find") == 0){
            int primaryKey;
            scanf("%d",&primaryKey);
            char* record = getFromFile(primaryKey,studentFile);
            if(record == NULL){
                printf("No record with SID=%d exists\n",primaryKey);
            }else{
                printf("%s\n",record);
            }
        }else if(strcmp(command, "del") == 0){
            int primaryKey;
            scanf("%d",&primaryKey);
            if (deleteFromFile(studentFile, primaryKey) == 0) {
                printf("No record with SID=%d exists\n",primaryKey);
            }
        }
    }
    printPrimaryIndex();
    printAvailableList();
    fclose(studentFile);
    return 0;
}