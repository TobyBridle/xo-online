# Build the executable and link all files
# Needs to link all files within `src/lib`
build: src/server.c src/lib/*.h
	@echo "Building project and linking..."
	@gcc -o server.out src/*.c

# Clean the object files and executable
clean:
	@echo "Cleaning up..."
	@rm -f *.o *.out
