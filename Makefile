# Build the executable and link all files
# Needs to link all files within `src/lib`
build: src/server.c src/client.c src/lib/*.h
	@echo "Building project and linking..."
	@make clean
	@mkdir bin
	@gcc -o bin/server src/server.c -Isrc/lib
	@gcc -o bin/client src/client.c -Isrc/lib

# Clean the object files and executable
clean:
	@echo "Cleaning up..."
	@rm -rf ./bin
