# Synopsis
In this work we examine processes and synchronization issues interprocess communication. We have developed both programs (C and S) in the following sense. The C program in random intervals cloned. The strands C 'produce applications to the program S. These applications contain an identifier (integer 1 ... N, with random selection) referring to text files that manages to S as and the identifier of the process C '. The communication of the strands C 'to S is implemented through shared memory (incoming post, "IN"). The S program receiving a message from the incoming position cloned and "feeds" the message to the newly born strand (S '). The clone process after the message draws the contents of the indicated File (1 ... N), manufactures reply message in which loads the file content. The message is stored in shared memory (position outgoing, "OUT") for receipt by clone C 'who expect it. After On receipt of the message process C'termatizei after informing a log file for details dosolopsias (when Requested when returned, response content). Text files moving with above would have a maximum size of 80 characters (eg, files with 1 line text - string). After sending the reply message clone S 'ends.

# Background
The IN and OUT locations can accommodate only one message every time
time. The locations are rewritten after the extraction of the last complete
the registered message. The number of transactions between C and S (ie on the number of strands C 'and S'
to be produced) is parameter of the program. Parameters of the program will be also the number of files. The C and S processes can have a parent-child relationship. The time between successive cloning for C is exponentially distributed with parameter argument of the program.

# Running the project
The program was developed and tested in a linux environment. It is compiled with the command make, otherwise gcc -o  process_sync process_sync.c -lm.
It also needs three parameters, a possible execution is (main 40 50 10). The first parameter indicates the number (N) of process C 'to be created, and the second argument is the (L) which is the parameter of the exponential distribution and finally the third (F) indicates the number of the available Files to choose a message from.
