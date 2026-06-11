#include <stdalign.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>


struct file_info {
	char* name ;
	size_t size ;
};


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

void get_file_into(char* path, struct file_info* fi){
	char full_path[1024] ;
	snprintf(full_path, sizeof(full_path), "%s/%s", path , fi->name);

	struct stat st ;
	if(stat(full_path, &st) != 0  ){
		perror(full_path) ;
			return;
		}
	fi->size = st.st_size ;
}

void print_entry(char * path , char* name ){
	struct file_info fi ;
	fi.name = name;
	if(flags.long_detail){
		get_file_into(path, &fi) ;
		printf("%lu %s\n" , (unsigned long)fi.size , fi.name  ) ;


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
