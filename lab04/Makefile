DIR_SOURCE 	= src
DIR_BIN		= bin/
MAKE 		= make

compile:
	- mkdir $(DIR_BIN)
	$(MAKE) -C $(DIR_SOURCE)
clean:
	$(MAKE) -C $(DIR_SOURCE) clean
	- rm -r *~
	- rm ~*
	- rm -rf $(DIR_BIN)
