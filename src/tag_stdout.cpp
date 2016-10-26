#include <sys/wait.h>
#include <unistd.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <signal.h>


// Convienience Types
using pipe_type = std::array<int, 2>;
using pipe_array = std::array<pipe_type, 2>;


// Constants
const int READ = 0;
const int WRITE = 1;
const int STDOUT = 0;
const int STDERR = 1;


/*
 * Displays help messages
 */
void print_usage(std::string prog_name) {
	std::cout << "Usage:\n\n"
			  << prog_name << " TAG executable [arg1 arg2 arg3...]\n" << std::endl;
}


/** parses argv
 *
 * @param argc
 * @param argv
 * @param tag - populated by function
 *
 * @returns 1 if arguments could not be parsed
 */
int parse_args(int argc,
		char** argv,
		std::string& tag) {
	if (argc < 3 || std::strcmp(argv[1],"--help") == 0) {
		// Can't parse
		return 1;
	}

	tag = argv[1];
	return 0;
}


/*
 * Creates a pair of pipes. stdout and stderr.
 */
void create_pipes(pipe_array& pipe_array) {
	for (auto& pipe_pair : pipe_array) {
		if (pipe(pipe_pair.data()) == -1) {
			throw std::runtime_error("Failed to create pipe");
		}
	}
}



/*
 * Actually does the work of printing to the terminal
 */
void _echo(const std::string& line,
		std::ostream& sink,
		const std::string& stream_name,
		const std::string& tag,
		std::mutex& print_lock) {
	std::lock_guard<std::mutex> guard(print_lock);
	sink << stream_name << ":" << tag << ": ";
	sink << line << '\n';
}


/*
 * Reads from the pipe and prints to the sink line by line.
 */
void echo(int pipe_fd, std::ostream& sink,
		const std::string& stream_name,
		const std::string& tag,
		std::mutex& print_lock) {

	// Capture pipe as input stream
	std::array<unsigned char, BUFSIZ> buff;

	// Read from the pipe until it is closed
	std::string line;
	ssize_t size;
	while ((size = read(pipe_fd, buff.data(), BUFSIZ)) > 0) {
		for (auto i = 0; i != size; i++) {
			auto char_ = buff[i];
			if (char_ == '\n') {
				// print line to terminal
				_echo(line, sink, stream_name, tag, print_lock);
				line.clear();
			} else {
				line += char_;
			}
		}
	}
	// cleanup
	close(pipe_fd);
	// Print the reminants if the last line didn't end with \n
	if (line.length()) {
		_echo(line, sink, stream_name, tag, print_lock);
	}
}


/*
 * Child process.
 *
 * Redirects the stdout and stderr to the pipes in pipe_array. Then
 * spawns a new process given by argv
 */
int child(pipe_array& pipe_array, char* const* argv) {
	// Redirect Stdout and Stderr to the pipes in pipe_array
	if (dup2(pipe_array[STDOUT][WRITE], STDOUT_FILENO) < 0) {
		throw std::runtime_error("Failed to dup2 pipe stdout");
	};
	if (dup2(pipe_array[STDERR][WRITE], STDERR_FILENO) < 0) {
		throw std::runtime_error("Failed to dup2 pipe stderr");
	};

	// Close parent side of the pipes
	for (auto& pipe_pair : pipe_array) {
		for (auto& pipe_ : pipe_pair) {
			if (close(pipe_) < 0) {
				throw std::runtime_error("Failed to close pipe");
			};
		}
	}

	// start the program
	// Remove non-child arguments
	auto child_argv = argv + 2;
	execvp(child_argv[0], child_argv);

	// We only get here if there's an error
	std::string estream(argv[0] + std::string(" failed to run ") + argv[2]);
	perror(estream.c_str());
	return 1;
}


int main(int argc, char **argv) {
	// Parse args
	std::string tag;
	if (parse_args(argc, argv, tag)) {
		// failed to parse, print help
		print_usage(std::string(argv[0]));
		return 1;
	}

	// Create pipes for stdin and stdout
	pipe_array pipe_array;
	create_pipes(pipe_array);

	// Fork new process
	pid_t pid = fork();
	if (pid < 0) {
		throw std::runtime_error("Failed to spawn process");
	}

	else if (pid == 0) {
		//In the child process
		return child(pipe_array, argv);
	}

	else {
		// In the parent process

		// Block all signals so they are handled by the child process
		sigset_t mask;
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, nullptr);

		// Close child side of the pipes
		for (auto& pipe_pair : pipe_array) {
			if (close(pipe_pair[WRITE]) < 0) {
				throw std::runtime_error("Failed to close pipe");
			};
		}

		// Start a thread to echo from the stdout pipe
		std::mutex print_lock;
		std::thread stdout_thread(echo, pipe_array[STDOUT][READ], std::ref(std::cout), "[O]", std::ref(tag), std::ref(print_lock));

		// The main thread echos from the stderr pipe
		echo(pipe_array[STDERR][READ], std::cerr, "[E]", tag, print_lock);

		// Wait for stdout thread to finish
		stdout_thread.join();

		// Get exit status of child process
		int status;
		waitpid(pid, &status, 0);

		// Return child process return code
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		}

		// child killed by signal
		else if (WIFSIGNALED(status)) {
			return WTERMSIG(status);
		}

		// Shouldn't reach here
		throw std::runtime_error("Couldn't get child process exit code");
	}
}
