#include <windows.h>
#include <stdio.h>
#include <time.h>

//Number of processes.
#define NUMBER_OF_PROCESSES 5
//The count of how many times processes will be executed.
#define NUMBER_OF_RUNNING 5
//Alpha value that we will use to predict next cpu burst time.
#define ALPHA 0.5f

int main(int argc, char *argv[])
{
	//Required for randomness.
	srand((unsigned int)time(NULL));


	SECURITY_ATTRIBUTES sa[NUMBER_OF_PROCESSES];
	HANDLE hWritePipeForSending[NUMBER_OF_PROCESSES], hReadPipeForSending[NUMBER_OF_PROCESSES];
	HANDLE hWritePipeForReceiving[NUMBER_OF_PROCESSES], hReadPipeForReceiving[NUMBER_OF_PROCESSES];

	//Creating Pipes.
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		SecureZeroMemory(&sa[i], sizeof(SECURITY_ATTRIBUTES));
		sa[i].bInheritHandle = TRUE;
		sa[i].lpSecurityDescriptor = NULL;
		sa[i].nLength = sizeof(SECURITY_ATTRIBUTES);

		if (!CreatePipe(&hReadPipeForSending[i], &hWritePipeForSending[i], &sa[i], 0) ||
			!CreatePipe(&hReadPipeForReceiving[i], &hWritePipeForReceiving[i], &sa[i], 0))
		{
			printf("unable to create pipe.\n");
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	STARTUPINFO si[NUMBER_OF_PROCESSES];
	PROCESS_INFORMATION pi[NUMBER_OF_PROCESSES];
	HANDLE process_handles[NUMBER_OF_PROCESSES];

	//Creating Child Processes.
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{

		SecureZeroMemory(&si[i], sizeof(STARTUPINFO));
		SecureZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));
		si[i].cb = sizeof(STARTUPINFO);

		//Initialize your Pipe handles here.
		si[i].hStdInput = hReadPipeForSending[i];
		si[i].hStdOutput = hWritePipeForReceiving[i];
		si[i].dwFlags = STARTF_USESTDHANDLES;

		// Start the child process. 
		if (!CreateProcess(NULL,	// No module name (use command line)
			"Child.exe",			// Command line
			NULL,					// Process handle not inheritable
			NULL,				    // Thread handle not inheritable
			TRUE,					// Set handle inheritance to FALSE
			0,					    // No creation flags
			NULL,					// Use parent's environment block
			NULL,					// Use parent's starting directory 
			&si[i],					// Pointer to STARTUPINFO structure
			&pi[i])					// Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			system("pause");
			exit(EXIT_FAILURE);
		}

		process_handles[i] = pi[i].hProcess;
	}


	int number_of_running = NUMBER_OF_RUNNING;
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
	{
		if (!WriteFile(hWritePipeForSending[i], &number_of_running, sizeof(int), NULL, NULL))
		{
			printf("unable to write pipe");
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	//Array to store burst times of processes.
	float burst_time[NUMBER_OF_PROCESSES];

	//Initializing first CPU burst times of processes.
	burst_time[0] = 300;
	burst_time[1] = 220;
	burst_time[2] = 180;
	burst_time[3] = 45;
	burst_time[4] = 225;

	//Process order keeps the working order of processes.
	int process_order[NUMBER_OF_PROCESSES];
	int end_signal[NUMBER_OF_PROCESSES];

	for (int j = 0; j < NUMBER_OF_RUNNING; j++)
	{
		printf("***********************************\n");
		printf("\n%dth Burst Time Order  <P%d = %0.1fms,", j + 1, 1, burst_time[0]);				//Printing burst time order of processes.

		for (int i = 1; i < NUMBER_OF_PROCESSES - 1; i++)
			printf("P%d = %0.1fms,", i + 1, burst_time[i]);
		printf("P%d = %0.1fms>\n", NUMBER_OF_PROCESSES, burst_time[NUMBER_OF_PROCESSES - 1]);

		for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
			process_order[i] = i;

		for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
		{
			for (int k = i + 1; k < NUMBER_OF_PROCESSES; k++)
			{
				//Sorting processes orders' accordingly their burst times.
				if (burst_time[process_order[i]] > burst_time[process_order[k]])
				{
					int temp_index = process_order[i];
					process_order[i] = process_order[k];
					process_order[k] = temp_index;
				}
			}
		}

		printf("%dth Execution Order<P%d,", j + 1, process_order[0] + 1);					//Printing the execution order of processes.
		for (int i = 1; i < NUMBER_OF_PROCESSES - 1; i++)
			printf("P%d,", process_order[i] + 1);
		printf("P%d>\n", process_order[NUMBER_OF_PROCESSES - 1] + 1);

		for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
		{
			printf("P%d started.\n", process_order[i] + 1);

			if (!WriteFile(hWritePipeForSending[process_order[i]], &burst_time[process_order[i]], sizeof(float), NULL, NULL))
			{
				printf("unable to write pipe");
				system("pause");
				exit(EXIT_FAILURE);
			}

			if (!ReadFile(hReadPipeForReceiving[process_order[i]], &end_signal[process_order[i]], sizeof(float), NULL, NULL))
			{
				printf("Error while reading data from pipe.");
				system("pause");
				ExitProcess(EXIT_FAILURE);
			}
			printf("P%d ended.\n", process_order[i] + 1);
		}


		printf("\n--------------------------------------------------------------\n");
		printf("Process||CPU Burst Time||Actual Lenght||Predicted CPU Burst\n");
		//Next Burst Time Calculation
		for (int i = 0; i < NUMBER_OF_PROCESSES; i++) {
			float tau_n_1;									//Predicted value of next CPU burst.
			int tn;											//Actual length of next CPU burst.
			float tau_n;									//Initial estimated value of CPU burst.
			tau_n = burst_time[i];
			tn = (rand() % 251) + 50;						//Random value between 50 and 300
			tau_n_1 = (ALPHA)*tn + (1 - (ALPHA)) / tau_n;
			burst_time[i] = tau_n_1;
			printf("   %d   ||     %0.1f     ||     %d     ||    %0.1f   \n", process_order[i] + 1, burst_time[process_order[i]],tn,tau_n_1);

		}
		printf("--------------------------------------------------------------\n\n");
		
		
		
	}

	


	// Close process handles. 
	for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
		CloseHandle(pi[i].hProcess);

	system("pause");
	return EXIT_SUCCESS;
}