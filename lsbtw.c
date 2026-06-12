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


typedef struct  {
	char* name ;
	off_t size ;  //size in bytes
	mode_t mode; // file type and mode
	nlink_t links ; // number of hard links
	ino_t inode ; //the inode number



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
}

void mode_string(file_info* fi ,char* str ){
	if	 	(S_ISREG(fi->mode)) str[0] = 'r';
	else if (S_ISBLK(fi->mode)) str[0] = 'b';
	else if (S_ISSOCK(fi->mode))str[0] = 's';
	else if (S_ISLNK(fi->mode)) str[0] = 'l';
	else if (S_ISDIR(fi->mode)) str[0] = 'd';
	else if (S_ISFIFO(fi->mode))str[0] = 'f';
	else if (S_ISCHR(fi->mode)) str[0] = 'c';
	str[1] = '\0';

}

void print_entry(char * path , char* name ){
	char mode_str[30];
	file_info fi ;
	fi.name = name;
	if(flags.long_detail){
		get_file_into(path, &fi) ;
		mode_string(&fi , mode_str) ;
		printf("%s:%s\n"  ,mode_str, fi.name  ) ;


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
