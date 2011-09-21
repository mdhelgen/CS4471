#include "fritter.h"

int aclGetLine(char* buf, int fd);

int main(int argc, char** argv){

	uid_t rUid, eUid, sUid;

	int fdAcl;
	int fdFile;
	char* aclFileName;
	char buf[80];

	int a;

	if(argc < 3){
		printf("  usage -- fritter <logfile> <entry>\n\n");
		exit(1);
	}
	//get the real and effective uid
	getresuid(&rUid, &eUid, &sUid);

	//set the effective UID down to the real UID value
	seteuid(rUid);

	printf("argv[1] length: %d\n", strlen(argv[1]));

	//allocate space for filename
	aclFileName = malloc(sizeof(char)*(strlen(argv[1])+5));
	
	//did the malloc fail?
	if(aclFileName==NULL){
		perror("aclFileName malloc");
		exit(1);
	}

	//copy the filename argument with .acl appended
	snprintf(aclFileName,sizeof(char)*(strlen(argv[1])+5),"%s.acl",argv[1]);

	//turn the effective permissions back up
	seteuid(sUid);

	//open the acl file for reading
	fdAcl = open(aclFileName, O_RDONLY | O_NOFOLLOW);

	//did the open fail?
	if(fdAcl == -1){
		if(errno == ENOENT){
			printf("Error: %s does not exist\n", aclFileName);
			exit(1);
		}
		else{
			perror("aclFile open()");
			exit(1);
		}
	}

	a = 0;
	//read the users from the .acl file
	do{
		a = aclGetLine(buf, fdAcl);
		if(a==0)
		printf("%s (%d)\n",buf, strlen(buf));
	}while(a==0);
	
	//open the logfile
	fdFile = open(argv[1], O_RDWR | O_NOFOLLOW);

	//did the open fail?
	if(fdFile == -1){
		if(errno == ENOENT){
			printf("Error: %s does not exist\n", argv[1]);
			exit(1);
		}
		else{	
			perror("logfile open()");
			exit(1);
		}
	}
	

	printf("aclFileName= %s\n", aclFileName);
	
	
	//check to see if the file exists
	//check to see if the .acl file exists
	//attempt to open the acl file
	//loop for 

	return 1;

}
/**
 * int aclGetLine(char* buf, int fd)
 *
 * Gets a line of text from the file described by fd, and places it in buf
 *
 * The file is read character by character, and non-whitespace characters are copied
 * to buf. When a newline or length limit is hit, the function returns, leaving data in buf.
 * When the end of the file is hit, the function returns with a different value.
 *
 * @param buf The memory location of the character buffer to put the read data
 * @param fd The file descriptor of an opened .acl file
 *
 * @returns 0 when a newline character is hit (more data is left) or -1 on EOF 
 */
int aclGetLine(char* buf, int fd){
	int i;
	int j;
	int ret;
	char ch;

	//clear out buf before reading
	memset(buf, '\0', 40);

	//read a character
	ret = read(fd, &ch, 1);

	//has the end of the file been reached?
	if(ret == 0)
		return -1;
	//did the read fail?
	if(ret < 0){
		perror("read");
	}
	
	i = 0;
	j = 0;

	//pass over preceding whitespace
	while(ch == '\t' || ch == ' '){
		ret = read(fd, &ch, 1);
		//end of file?
		if (ret == 0)
			return -1;
		//error?
		if (ret < 0)
			perror("read");
	}

	//read characters until a newline is hit
	for(; ch != '\n' && i < 30; i++){

		//check that the read character is a non-numeric printable character
		if( !(ch >= 65 && ch <= 90) && !(ch >= 97 && ch <= 122)){
			printf("Error: malformed line in acl file (character %d encountered)\n", ch);
			exit(1);
		}

		//copy the character into the buffer
		strncpy(&buf[j++], &ch, 1);
		
		//read another character
		ret=read(fd, &ch, 1);

		//has the end of the file been reached?
		if(ret == 0)
			return -1;
		//did the read fail?
		if(ret < 0){
			perror("read");
		}
	}

	//return 0 on successful read of a line
	return 0;

}
