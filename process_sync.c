/* 
 * File:   main.c
 * Author: Panagiotis
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/ptrace.h>
//#include <linux/user.h>

/* Union semun */
union semun {
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;  /* array for GETALL, SETALL */

};

/* Semaphore down operation, using semop */
int sem_down(int sem_id) {

    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = -1;
    sem_d.sem_flg = 0;
    
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror("Semaphore Down Operation ");
        return -1;
    }
    
    return 0;

}

/* Semaphore up operation, using semop */
int sem_up(int sem_id) {

    struct sembuf sem_d;

    sem_d.sem_num = 0;
    sem_d.sem_op = 1;
    sem_d.sem_flg = 0;
    
    if (semop(sem_id, &sem_d, 1) == -1) {
        perror("Semaphore Up Operation ");
        return -1;
    }
    
    return 0;

}

/* Semaphore Init - set a semaphore's value to val */
int sem_Init(int sem_id, int val) {

    union semun arg;

    arg.val = val;
    
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("Semaphore Setting Value ");
        return -1;
    }
    
    return 0;

}

struct INPUT {      // this is the struct of input shared memory
    int pid;        // process id
    int Num;        // file number to read from
    int sem_ID;     // semaphore id
};

struct OUTPUT {     // this is the struct of the output shared memory
    int pids;       // process id
    char buff[80];  // 80 characters buffer to carry the message
};

int main(int argc, char** argv) {

    pid_t pid;
    int N, L, F;
    int shm_id, shm_id2;
    int sem_id1, mutexc, mutexs, sem1, sem2;
    int i, j;
    int FileNumber, FileNumber2;
    FILE* fp;
    FILE* fp2;
    struct tm *full1, *full2;
    time_t start1, start2;
    char *event1, *event2;
    int exponential;
    double temp;

    struct INPUT *sh;                   // pointers to shared memory in and out
    struct OUTPUT *sh2;
    struct INPUT in_buffer, in_buffer2; // assistant buffers for write and read, to and from shared memories
    struct OUTPUT out_buffer, out_buffer2;

    srand(time(NULL));                  // seed for rand to produce random numbers

    if (argc != 4) {
        printf("All Arguments Needed\n");
    }

    N = atoi(argv[1]);                  // setting arguments
    L = atoi(argv[2]);
    F = atoi(argv[3]);
    
    /* Create a new shared memory segment for Input */
    shm_id = shmget(IPC_PRIVATE, sizeof (struct INPUT), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("Input Shared Memory Creation");
        shmctl(shm_id, IPC_RMID, 0);
        exit(EXIT_FAILURE);
    }

    /* Attach the shared memory segments */
    sh = (struct INPUT*) shmat(shm_id, (struct INPUT*) 0, 0);
    if (sh == (struct INPUT*) - 1) {
        perror("Shared Memory Attach");
        shmctl(shm_id, IPC_RMID, 0);
        exit(EXIT_FAILURE);
    }

    /* Create a new shared memory segment for Output */
    shm_id2 = shmget(IPC_PRIVATE, sizeof (struct OUTPUT), 0666 | IPC_CREAT);
    if (shm_id2 == -1) {
        perror("Output Shared Memory Creation");
        shmctl(shm_id2, IPC_RMID, 0);
        exit(EXIT_FAILURE);
    }

    /* Attach the shared memory segments */
    sh2 = (struct OUTPUT*) shmat(shm_id2, (struct OUTPUT*) 0, 0);
    if (sh2 == (struct OUTPUT*) - 1) {
        perror("Shared Memory Attach ");
        shmctl(shm_id2, IPC_RMID, 0);
        exit(EXIT_FAILURE);
    }

    /* Create and Initialize Global Semaphores */
    mutexc = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    if (mutexc == -1) {
        perror("Semaphore Creation");
        semctl(mutexc, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    if (sem_Init(mutexc, 1) == -1) {
        perror("Semaphore Initialize");
        semctl(mutexc, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    mutexs = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    if (mutexs == -1) {
        perror("Semaphore Creation");
        semctl(mutexs, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    if (sem_Init(mutexs, 1) == -1) {
        perror("Semaphore Initialize");
        semctl(mutexs, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    sem1 = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    if (sem1 == -1) {
        perror("Semaphore Creation");
        semctl(sem1, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    if (sem_Init(sem1, 1) == -1) {
        perror("Semaphore Initialize");
        semctl(sem1, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    sem2 = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    if (sem2 == -1) {
        perror("Semaphore Creation");
        semctl(sem2, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    if (sem_Init(sem2, 0) == -1) {
        perror("Semaphore Initialize");
        semctl(sem2, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    /* New process */
    if ((pid = fork()) == -1) {
        perror("Fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { /* Child process S*/

        printf("I Am The Child Process S With Process id: %d\n", getpid());

        for (j = 0; j < N; j++) { // S will run exact N times

            /*enter the OUTPUT shared memory and read the message*/
            sem_down(sem2);     // this prevents other processes from writing to a full memory
            sem_down(mutexc);   //this prevents other processes entering the shared memory at the same time with an existing one (mutex)

            memcpy(&in_buffer, sh, sizeof (struct INPUT));  // take the message from shared memory and put it to in_buffer

            sem_up(mutexc);     // up the mutex
            sem_up(sem1);       // up this semaphore to let other processes that memory is empty

            /*After you read create an s' to read from the wright file and make a package to answer*/
            if ((pid = fork()) == -1) { //at this point we have read the message and we must create an s'
                perror("Fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {     //S'

                printf("I Am The Child Process S' With Process id: %d\n", getpid());

                FileNumber = in_buffer.Num; // the file that s' will read from

                char fnumber[15];           //convert int to string
                sprintf(fnumber, "%d", FileNumber);

                char txtfile[80];           //append the whole file name
                strcpy(txtfile, fnumber);
                strcat(txtfile, ".txt");
                puts(txtfile);

		int found_file = 0;

                for (i = 1; i <= F; i++) {
                    
                    if (FileNumber == i) {
                        if ((fp = fopen(txtfile, "r")) == NULL) {
                            perror("Could Not Read From File");
                            //printf("File:%d\n", FileNumber);
                            exit(EXIT_FAILURE);
                        }
                        fgets(out_buffer.buff, 80, fp); // take the line (max 80 chars) and place it to out_buffer
                        fclose(fp);
                        found_file = 1;
                    }
                                  
                }
		
		if (found_file == 0) {
                	printf("No Valid File Number\n");
                }
                
                out_buffer.pids = getpid(); // keeping s' pid

                /*s' enters OUTUP shared memory and writes the reply message for c'*/
                sem_down(mutexs);   // this prevents s' processes from each other from writing to out memory at the same time

                memcpy(sh2, &out_buffer, sizeof (struct OUTPUT));   // place out_buffer elements to output shared memory from the c' to read
                printf("I Read For Process C' With Process id: %d\n", in_buffer.pid);

                sem_up(in_buffer.sem_ID);   // wake the wright c' toread the message that belongs to her
                exit(EXIT_SUCCESS);
            
            }
        
        }
        
        while (wait(NULL) > 0) { // wait all s' to end before S exits
        }
        
        exit(EXIT_SUCCESS); // end process S
    
    } 
    else { /* Parent process C*/
        
        printf("I Am The Parent process C With Process id: %d\n", getpid());

        for (i = 0; i < N; i++) {   // c' number = N
            FileNumber2 = rand() % 10 + 1;

            temp = (double) log((double) (((double) (rand() % 1000 + 1)) / (1000 + 1)));
            exponential = (int) (-temp / L);

            if ((pid = fork()) == -1) {
                perror("Fork");
                exit(EXIT_FAILURE);
            }
            
            if (pid == 0) { /* Child process C'*/
                
                printf("I Am The Child Process C' With Process id: %d\n", getpid());
                
                /* Create a new semaphore id */
                sem_id1 = semget((key_t) getpid(), 1, 0600 | IPC_CREAT);
                if (sem_id1 == -1) {
                    perror("Semaphore Creation");
                    semctl(sem_id1, 0, IPC_RMID);
                    exit(EXIT_FAILURE);
                }
                
                /* Initialize C' semaphore */
                if (sem_Init(sem_id1, 0) == -1) {
                    perror("Semaphore Initialize");
                    semctl(sem_id1, 0, IPC_RMID);
                    exit(EXIT_FAILURE);
                }
                
                /*Setting up the message */
                in_buffer2.sem_ID = sem_id1;    // keep the shem_id to pass it to S process
                in_buffer2.Num = FileNumber2;   // decide file number
                in_buffer2.pid = getpid();
                
                /*Trying to enter INPUT shared memory to write the message for s to read*/
                sem_down(sem1);                 // this prevents other processes form writing in an empty memory
                sem_down(mutexc);               // mutex
                
                /*Taking the full shape of date and current time of writing the request*/
                start1 = time(NULL);
                full1 = localtime(&start1);
                event1 = asctime(full1);

                memcpy(sh, &in_buffer2, sizeof (struct INPUT)); // place in_buffer2 into input shared memory

                sem_up(mutexc);                 // setting free the mutex
                sem_up(sem2);                   // tell other processes that memory is full
                sem_down(sem_id1);              // c' blocks herself until an s' process wakes her to deliver her messsage

                /*When s' wakes this specific c' that previously was down,c' will read for OUTPUT shared memory*/
                memcpy(&out_buffer2, sh2, sizeof (struct OUTPUT));  // at this point c' has waken up and read output memory and places the message to out_buffer2

                /*Taking the full shape of date and current time of reading the reply*/
                start2 = time(NULL);
                full2 = localtime(&start2);
                event2 = asctime(full2);

                sem_up(mutexs);     //free s' mutex
                printf("I Am The Child Process C' With Process id: %d And I Recieved Answer From S' With Process id: %d\n", getpid(), out_buffer2.pids);

                if (semctl(sem_id1, 0, IPC_RMID) == -1) {   // c' semaphore not needed any more
                    perror("Semaphore Deleting");
                    exit(EXIT_FAILURE);
                }
                
                /*Write the info to log file*/
                if ((fp2 = fopen("logfile.txt", "a")) == NULL) {
                    perror("Could Not Write to logfile");
                    exit(EXIT_FAILURE);
                }
                
                fprintf(fp2, "Message: %s\n", out_buffer2.buff);
                fprintf(fp2, "Time of Request: %s\n", event1);
                fprintf(fp2, "Time of Reply: %s\n", event2);
                fclose(fp2);
                exit(EXIT_SUCCESS); // c' ends
                
            }
            
            sleep(exponential);
        
        }
        
        while (wait(NULL) > 0) { //wait all c' to end before C exits
        }
        
        /*Deleting Shared Memories INPUT,OUTPUT And Global Semaphores*/
        shmdt((char*) sh); // dettach
        if (shmctl(shm_id, IPC_RMID, 0) == -1) {
            perror("Failed To Delete INPUT Shared Memory");
            exit(EXIT_FAILURE);
        }

        shmdt((char*) sh2); // dettach
        if (shmctl(shm_id2, IPC_RMID, 0) == -1) {
            perror("Failed To Delete OUTPUT Shared Memory");
            exit(EXIT_FAILURE);
        }

        if (semctl(mutexc, 0, IPC_RMID) == -1) {
            perror("Semaphore Deleting");
            exit(EXIT_FAILURE);
        }
        if (semctl(mutexs, 0, IPC_RMID) == -1) {
            perror("Semaphore Deleting");
            exit(EXIT_FAILURE);
        }
        if (semctl(sem1, 0, IPC_RMID) == -1) {
            perror("Semaphore Deleting");
            exit(EXIT_FAILURE);
        }
        if (semctl(sem2, 0, IPC_RMID) == -1) {
            perror("Semaphore Deleting");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);     // end process C
    
    }

}
