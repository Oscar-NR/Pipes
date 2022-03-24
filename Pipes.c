

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/types.h>


//This program create a child, the father and the child will comunicate through two pipes to have a full-duplex comunication.

//The funcionality consist in you write a message, the father read the message and send it to the child though the pipe, then the child reverse the 
//	message and reverse the same and write in the console, when the max number of char are reached the child will 
//	ask the father to end the chat.


int main(int argc, char* argv[])
{
	if(argc<2)
	{
		printf("Error: you need to send the limit of the size of the message\n");	
		exit(1);
	}

	int fd_pipe_Father_to_Child [2];
	int fd_pipe_Child_to_Father [2];
	int maxChar = atoi(argv[1]);
	char message [512];
	
	
	//Character to control the flow of the program
	char control;
	
	//Instead of wait to the end of the child we are going to work until the chat ends
	signal(SIGCLD, SIG_IGN);
	
	//Open the pipes
	int pipe1 = pipe(fd_pipe_Father_to_Child);
	int pipe2 = pipe(fd_pipe_Child_to_Father);
	
	//Check that the pipes were opened well
	if( pipe1<0 || pipe2<0 )
	{
		printf("Failure in the creation of the pipes");
		exit(1);	
	}
	
	//Now made the fork so both process will keep the two pipes opened
	pid_t pId = fork();
	
	if(pId>0)
	{
		//Father code, because the the value of pId is bigger than 0
		
		//Now close the file descriptores of the pipes that we are not going to use
		close(fd_pipe_Father_to_Child[0]);	//the reading from father to child
		close(fd_pipe_Child_to_Father[1]);	//the write from child to father
		
		//bucle of chat
		do
		{
			printf("I'm the father write the message:\n");
			int readed = read(0, message, 512);
			if(readed == -1)
			{
				perror("Error in the read of the keyboard");
				exit(1);
			}
			
			//Write the message in the pipe to the child
			int writed = write(fd_pipe_Father_to_Child[1], message, readed);
			if(writed == -1)
			{
				perror("Error writing in the pipe");
				exit(1);
			}
			
			//read if the father should continue or not, this is one character sended by the child
			int received = read(fd_pipe_Child_to_Father[0], &control, 1);	
			
			if(received == -1)
			{
				perror("Error in the reading of pipe of the child");
				exit(1);
			}
			
			printf("The program will continue? %c \n", control);
			
		}while(control !='n');
		
		//Here would be the moment of made the wait, but we ignore that signal and control the process with the character control
		//wait(0);
		
		//close the file descriptors that remains opened
		close(fd_pipe_Father_to_Child[1]);	//the writing from father to child
		close(fd_pipe_Child_to_Father[0]);	//the reading from child to father
		
		//end of the code of the father
	
	}else
	{
		//check to be sure that this code will be executed by the child, pId could be -1 when an error happen
		if(pId == 0)
		{
		
			//Close the file descriptor that will no be used, same in father but close the oposite ones
			close(fd_pipe_Father_to_Child[1]);	//the write from father to child
			close(fd_pipe_Child_to_Father[0]);	//the read from child to father

			//Keep the number of characters
			int total = 0;
			while(1)
			{
				int readed = read(fd_pipe_Father_to_Child[0], message, 512);
				if(readed == -1)
				{
					perror("error reading the pipe");
					exit(1);
				}
				
				//revert the message
				char revertedMessage [512];
				for(int i=0; i<=readed ;i++)
				{
					//message[readed-i-2] this is because the first char is the 0 char to readed-1 but also we need to avoid the char '\0'
					revertedMessage[i]= message[readed-i-2];				
				}				
				printf("I'm the child the reverse word is:\n%s \n",revertedMessage);	
				
				total+=readed;
				if(total >= maxChar)
				{
					control = 'n';
					//send the message 'n' to close the program to end the father
					int w = write(fd_pipe_Child_to_Father[1], &control, 1);
					if(w== -1)
					{
						perror("Error to send the close character to the father");
						exit(1);
					}
					
					break;					
				}else
				{
					control = 'y';
					int w = write(fd_pipe_Child_to_Father[1], &control, 1);
					if(w== -1)
					{
						perror("Error to send the close character to the father");
						exit(1);
					}
				
				}			
			}
			//Close the reaming pipes
			close(fd_pipe_Father_to_Child[0]);	//the read from father to child
			close(fd_pipe_Child_to_Father[1]);	//the write from child to father	
			
			//End the program as expected
			exit (0);
		
		}else
		{
			perror("Error in the fork");
			exit(1);		
		}
	}

	return 0;
}