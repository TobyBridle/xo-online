# Build the executable and link all files
# Needs to link all files within `src/lib`
build: src/server.c src/client.c src/utils.c src/lib/*.h
	@echo "Building project and linking..."
	@make clean
	@mkdir bin
	@gcc src/utils.c src/server.c -o bin/server -pthread
	@gcc src/utils.c src/client.c -o bin/client -pthread

# Same as build but with -g flags
dbb: src/server.c src/client.c src/utils.c src/lib/*.h
	@echo "Building project and linking..."
	@make clean
	@mkdir bin
	@gcc -g src/utils.c src/server.c -o bin/server -pthread
	@gcc -g src/utils.c src/client.c -o bin/client -pthread

# Clean the object files and executable
clean:
	@echo "Cleaning up..."
	@rm -rf ./bin
