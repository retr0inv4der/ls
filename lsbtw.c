#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

int show_all = 0 ;
int main(int argc , char* argv[]){
	//parse argv
	int opt ;
	while((opt = getopt(argc, argv, "a")) != -1){
		switch (opt) {
			case 'a' :
				show_all = 1;
				break;

			default:
				fprintf(stderr, "usage: %s [-a] [path]" , argv[0]);
				return -1;

		}
	}


	const char* path = (optind  < argc) ? argv[optind] : "." ;

	DIR* directory = opendir(path) ;
	if(!directory){
		perror("opendir") ;
		return 1 ;
	}


	struct dirent* entry ;
	while((entry = readdir(directory))!= NULL){
		if( !show_all  && entry->d_name[0] == '.') continue;
		printf("%s    ", entry->d_name) ;
	}
	printf("\n");
	return 0 ;


}
