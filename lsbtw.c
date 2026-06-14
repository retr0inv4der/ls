#include <stdalign.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

typedef struct  {
	char* name ; //the name of the file
	off_t size ;  //size in bytes
	mode_t mode; // file type and mode
	nlink_t links ; // number of hard links
	ino_t inode ; //the inode number
	char* username ; // the name of the owner
	char* gidname ;
	time_t mtime;
} file_info ;


struct flags {
	bool show_all;
	bool long_detail;
};
struct flags flags ;

void prinf_err(const char* msg){
	fprintf(stderr,  "%s: %s\n", msg , strerror(errno)) ;
	fflush(stderr) ;
}

char*  set_path(int argc , char** argv) {
	char* res ;
	res= (argc > optind) ? argv[optind] : "." ;
	return res ;
}


bool parse_flags(int argc , char** argv){
	//returns true on success and false on failure
	bool res = 1 ;
	char opt ;
	while((opt = getopt(argc, argv, "al")) != -1){
		switch (opt) {
			case 'a' :
			flags.show_all = true;
			break;

			case 'l' :
			flags.long_detail = true;
			break;

			default:
			fprintf(stderr, "usage: %s [-a] [-l] [path] "  , argv[0]);
			fflush(stderr) ;
			res = 0 ;
			break;
			}
	}
	return res ;
}


bool should_show(char* name){
	if(!flags.show_all && name[0]=='.' ) return 0 ;
	return 1 ;
}

void get_file_into(char* path,  file_info* fi){
	char full_path[1024] ;
	snprintf(full_path, sizeof(full_path), "%s/%s", path , fi->name);

	struct stat st ;
	if(stat(full_path, &st) != 0  ){
		perror(full_path) ;
			return;
		}
	//extract from the st to the file_info struct

	fi->size = st.st_size ;
	fi->inode  = st.st_ino ;
	fi->links = st.st_nlink ;
	fi->mode = st.st_mode ;
	fi->mtime = st.st_mtime ;




	//extract the username
	struct passwd* pw = getpwuid(st.st_uid) ;
	if(pw == NULL){
		strcpy(fi->username, "?\0") ;
	}
	fi->username = pw->pw_name ;

	//extract the gid name
	struct group* gr  = getgrgid(st.st_gid) ;
	if(gr == NULL ){
		strcpy(fi->gidname , "?\0") ;

	}
	fi->gidname = gr->gr_name ;



}


void get_time_string(char* buf  , size_t buf_len, time_t mtime ){
	struct tm* time_info  = localtime(&mtime) ;
	strftime(buf, buf_len, "%b %e %H:%M", time_info);
}





void perm_string(file_info* fi ,char* str ){
	if (S_ISBLK(fi->mode)) str[0] = 'b';
	else if (S_ISSOCK(fi->mode))str[0] = 's';
	else if (S_ISLNK(fi->mode)) str[0] = 'l';
	else if (S_ISDIR(fi->mode)) str[0] = 'd';
	else if (S_ISFIFO(fi->mode))str[0] = 'f';
	else if (S_ISCHR(fi->mode)) str[0] = 'c';
	else						str[0] = '-';
	//set the perms
	for(int i =1 ; i<= 9 ; i++)str[i] ='-' ;
	if(fi->mode & S_IRUSR) str[1]='r' ;
	if(fi->mode & S_IWUSR) str[2]='w' ;
	if(fi->mode & S_IXUSR) str[3]='x' ;

	if(fi->mode & S_IRGRP) str[4]='r';
	if(fi->mode & S_IWGRP) str[5]='w';
	if(fi->mode & S_IXGRP) str[6] = 'x' ;

	if(fi->mode & S_IROTH) str[7]='r';
	if(fi->mode & S_IWOTH) str[8] = 'w' ;
	if(fi->mode & S_IXOTH) str[9] = 'x' ;
	str[10]='\0';

}




void print_entry(char * path , char* name ){
	char mode_str[11];
	file_info fi ;
	fi.name = name;
	char time[100] ;
	if(flags.long_detail){
		get_file_into(path, &fi) ;
		perm_string(&fi , mode_str) ;
		get_time_string(time , 100 , fi.mtime) ;
		printf("%s %lu %s %s %s   %s\n"   ,mode_str , fi.links ,fi.username , fi.gidname ,time, fi.name  ) ;


	}else{
		printf("%s    " , fi.name) ;
	}

}


void walk_directory(char* path , void (*callback)( char* , char* )){
	DIR* directory = opendir(path);
	if(!directory){
		perror("opendir") ;
		return;
	}
	struct dirent* entry ;
	while ((entry = readdir(directory)) != NULL) {

		if(!should_show(entry->d_name)) continue;
		callback(path , entry->d_name ) ;
	}


	closedir(directory) ;
}


int main(int argc , char* argv[]){

	if(!parse_flags(argc, argv)){
		exit(1) ;
	}
	//create path after parsing the flags acording to the optind var
	char* path = set_path(argc , argv ) ;
	walk_directory(path  ,print_entry);

	return 0 ;


}
