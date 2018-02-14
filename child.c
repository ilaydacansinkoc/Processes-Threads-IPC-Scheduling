#include <windows.h>
#include <stdio.h>

int number_of_running_process;
// Burst time is the signal that impacts the parent process sent the burst time to the child process.
float burst_time;
// End signal is the signal that impacts the pipe is written that signal the end of process to parent process.
int end_signal = 1;

int main(int argc, char *argv[])
{
	HANDLE hStdin, hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);

	if ((hStdout == INVALID_HANDLE_VALUE) || (hStdin == INVALID_HANDLE_VALUE))
		ExitProcess(EXIT_FAILURE);


	if (!ReadFile(hStdin, &number_of_running_process, sizeof(int), NULL, NULL))
	{
		printf("Error while reading data from pipe.");
		system("pause");
		ExitProcess(EXIT_FAILURE);
	}

	for (int i = 0; i < number_of_running_process; i++) {

		//Reading from pipe.
		if (!ReadFile(hStdin, &burst_time, sizeof(float), NULL, NULL))
		{
			printf("Error while reading data from pipe.");
			system("pause");
			ExitProcess(EXIT_FAILURE);
		}

		//Wait along burst time of process.
		Sleep((int)burst_time);

		//Writing to pipe.
		if (!WriteFile(hStdout, &end_signal, sizeof(int), NULL, NULL))
		{
			printf("unable to write pipe");
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
