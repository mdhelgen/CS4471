/**
 * Matt Helgen (mdhelgen)
 * CS4471 project 1
 * 9/25/2011
 *
 */

#include "fritter.h"

int aclGetLine(char* buf, int fd);

int main(int argc, char** argv){

	uid_t rUid, eUid, sUid;

	struct passwd *passwd;
	int fdAcl;
	int fdFile;
	char* aclFileName;
	char buf[80];
	int aclMatch = 0;
	int ret=0;

	int totalWritten;

	int a;

	if(argc < 3){
		printf("  usage -- fritter <logfile> <entry>\n\n");
		exit(-1);
	}

	if( strlen(argv[2]) > 82){
		printf("Error: the entry cannot exceed 81 characters\n");
		exit(-1);
	}

	//get info from the /etc/passwd file
	passwd = getpwuid ( getuid());  
 

	//get the real and effective uid
	ret = getresuid(&rUid, &eUid, &sUid);
	
	if(ret == -1){
		perror("getresuid()");
		exit(ret);
	}


	//get info from the /etc/passwd file about the realuid running the process
	passwd = getpwuid(rUid);
	
	//set the effective UID down to the real UID value
	ret = seteuid(rUid);

	if(ret == -1){
		perror("seteuid()");
		exit(ret);
	}

	//allocate space for filename
	aclFileName = malloc(sizeof(char)*(strlen(argv[1])+5));
	
	//did the malloc fail?
	if(aclFileName==NULL){
		perror("aclFileName malloc");
		exit(-1);
	}

	//copy the filename argument with .acl appended
	ret = snprintf(aclFileName,sizeof(char)*(strlen(argv[1])+5),"%s.acl",argv[1]);

	if(ret < 0){
		perror("snprintf");
		exit(ret);
	}

	//turn the effective permissions back up
	ret = seteuid(sUid);

	if (ret == -1){
		perror("seteuid");
		exit(ret);
	}

	//open the acl file for reading
	fdAcl = open(aclFileName, O_RDONLY | O_NOFOLLOW);

	//did the open fail?
	if(fdAcl == -1){
		if(errno == ENOENT){
			printf("Error: %s does not exist\n", aclFileName);
			exit(-1);
		}
		else{
			perror("aclFile open()");
			exit(-1);
		}
	}

	a = 0;
	//read the users from the .acl file
	do{
		a = aclGetLine(buf, fdAcl);
		if(a==0){

			//does the acl entry match the rUid's login?
			if (strcmp(buf, passwd->pw_name) == 0){
				aclMatch = 1;
			}
		}
	}while(a == 0 && aclMatch == 0);

	//all of the acl entries have been read
	//was there a match to the real user?
	if(aclMatch == 0){
		printf("Error: the acl file does not provide permissions for you to access the file.\n");
		exit(-1);
	}

	//open the logfile
	fdFile = open(argv[1], O_WRONLY | O_NOFOLLOW | O_APPEND);

	//turn the permissions back down once the file has been opened successfuly
	ret = seteuid(rUid);

	if(ret == -1){
		perror("seteuid");
		exit(ret);
	}

	//did the open fail?
	if(fdFile == -1){
		if(errno == ENOENT){
			printf("Error: %s does not exist\n", argv[1]);
			exit(-1);
		}
		else{	
			perror("logfile open()");
			exit(-1);
		}
	}
	

	totalWritten = 0;
	//write until the entire argument is written
	while(totalWritten < strlen(argv[2])){
	
		//write the message to the file
		ret = write(fdFile, &argv[2][totalWritten], strlen(argv[2])-totalWritten);
		totalWritten += ret;
	
		if (ret == -1){
			perror("write");
			exit(ret);
		}
	}
	

	//successful completion
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
	int ret =0;
	char ch;

	//clear out buf before reading
	memset(buf, '\0', 40);


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
	for(; (ch != '\n' && ch != ' ' && ch != '\t') && i < 30; i++){

		//check that the read character is a non-numeric printable character
		if( !(ch >= 65 && ch <= 90) && !(ch >= 97 && ch <= 122)){
			printf("Error: malformed line in acl file (character %d encountered)\n", ch);
			exit(-1);
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
	

	//if whitepace was hit, only whitespace should be seen until a newline
	if( ch == '\t' || ch == ' '){
		while(ch == '\t' || ch == ' '){
			ret = read(fd, &ch, 1);
			//end of file?
			if (ret == 0)
				return -1;
			//error?
			if (ret < 0)
				perror("read");
			//if the character isn't whitespace
			if (ch != '\t' && ch != '\n' && ch != ' '){

				printf("Error: malformed line in acl file (character %d encountered)\n", ch);
				exit(-1);
			}
		}
	}

	

	//return 0 on successful read of a line
	return 0;

}
