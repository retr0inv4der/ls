#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>


int show_all = 0 ;
int long_detail=0;

int should_show(struct dirent* entry){
	if(!show_all && entry->d_name[0]=='.' ) return 0 ;
	return 1 ;
}

void print_entry(char* path ,struct dirent* entry){
	if(long_detail){
		char fullpath[1024] ;
		snprintf(fullpath, sizeof(fullpath), "%s/%s", path , entry->d_name);

		struct stat st ;
		if(stat(fullpath, &st) != 0  ){
			perror(fullpath) ;
			return;
		}

		printf("%lu %s\n" , (unsigned long)st.st_size , entry->d_name ) ;


	}else{
		printf("%s    " , entry->d_name) ;
	}

}

void walk_directory(char* path , void (*callback)(char* , struct dirent* )){
	DIR* directory = opendir(path);
	if(!directory){
		perror("opendir") ;
		return;
	}


	struct dirent* entry ;
	while ((entry = readdir(directory)) != NULL) {
		if(!should_show(entry)) continue;
		callback(path , entry) ;
	}
}

int main(int argc , char* argv[]){
	//parse argv
	int opt ;
	while((opt = getopt(argc, argv, "al")) != -1){
		switch (opt) {
			case 'a' :
				show_all = 1;
				break;

			case 'l' :
				long_detail = 1 ;
				break;

			default:
				fprintf(stderr, "usage: %s [-a] [path]" , argv[0]);
				return -1;

		}
	}

	char* path ;
	if(optind < argc){
		path = argv[optind] ;
	}else {
		path = "." ;
	}
	walk_directory(path, print_entry);

	return 0 ;


}
