#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "parser.h"

static char* magic_to_str(u1 *magic,int size) {
	int i = 0;
	char *buf = (char *) malloc((size+1) * sizeof(char));
	for(;i<size;i++) {
		buf[i] = magic[i];
	}
	buf[size] = '\0';
	return buf;
}

static void resetFilePtr(FILE *fp) {
	fseek(fp,0,SEEK_SET);
}

static void setFilePtr(FILE* fp,long offset) {
	resetFilePtr(fp);
	fseek(fp,offset,SEEK_CUR);
}

static const char* dexGetStringData(u1* baseAddress,u4 offset) {
	const u1* ptr = baseAddress + offset;
	while(*(ptr++) > 0x7f) /* empty body */;
	return (const char* ) ptr;
}

int main(int argc,char ** argv) {
	struct DexHeader header;
	struct DexMapList mapList;
	struct DexMapItem strItem;
	struct stat filestat;
	FILE * fp;
	int i;
	size_t count; 
	const char * filename = argv[1];
	int fd = open(filename,O_RDONLY);	
	fp = fopen(filename,"rb");
	if(fp == NULL) {
		puts("Error Open File");
		exit(1);
	}
	/** parse the dexHeader */
	fstat(fd,&filestat);
	off_t pos = filestat.st_size; 
	fseek(fp,0,SEEK_SET);
	u1 *baseAddress = mmap(0,pos,PROT_READ,MAP_SHARED,fd,0);
	memset(&header,0,sizeof(struct DexHeader));
	count = fread(&header,sizeof(struct DexHeader),1,fp);
	char *magic = magic_to_str(header.magic,8);
	printf("%s\n", magic);
	free(magic);
	printf("%d\n", header.mapOff);
	setFilePtr(fp,header.mapOff);
	fread(&mapList,sizeof(u4)+sizeof(struct DexMapItem),1,fp);
	printf("mapListSize:%d\n", mapList.size);
	printf("itemType:%d\n",mapList.list[0].type);
	struct DexMapItem item;
	for(int i = 0;i<mapList.size-1;i++) {
		fread(&item,sizeof(struct DexMapItem),1,fp);
		printf("itemType:%d\n",item.type);
		if(item.type == kDexTypeStringIdItem) {
			strItem = item;
		}
	}
	setFilePtr(fp,strItem.offset);
	struct DexStringId strIdItem;
	for(int i=0;i<strItem.size;i++) {
		fread(&strIdItem,sizeof(struct DexStringId),1,fp);
		const char * str = dexGetStringData(baseAddress,strIdItem.stringDataOff);
		printf("stringDataOff:%d %s\n",strIdItem.stringDataOff,str);
	}
	/** close fp,fd */
	munmap(baseAddress,pos);
	i = fclose(fp);
	close(fd);
	if(i != 0) {
		puts("Close Fail");
		return 1;
	}
	return 0;
}
