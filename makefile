all : lsbtw.c
	@echo compiling...
	@gcc -Wall -Wextra lsbtw.c -o lsbtw

clean : lsbtw
	@echo cleaning...
	@rm lsbtw
