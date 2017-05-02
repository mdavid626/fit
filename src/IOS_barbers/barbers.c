/*
 * File:     barbers.c
 * Date:     2011-05-01
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Sleeping barber problem
 */


// Header files
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>

// shared memory
#include <sys/shm.h>

// semaphores
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

// fork, wait, kill
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


// Macros
// -----------------------------------------------------------------------------

#define STDOUT_SWITCH "-"
#define SEM_COUNT 6

// Used in function ftok() to generate unique key for shared memory
#define SHMKEY_PATH "/dev/null"
#define SHMKEY_ID 56

/**
 * Correctly end work with shared memory (deallocate)
 */
#define clean_shm           shmctl(shm_id, IPC_RMID, NULL);

/**
 * Correctly end work with semaphores (close and unlink)
 */
#define clean_sem           for (int j = 0; j < SEM_COUNT; j++)\
                            { if (sem_ids[j] != NULL) sem_close(sem_ids[j]);\
                              sem_unlink(SEM_NAME[j]); }

/**
 * Only *close* semaphores - used in child processes
 */
#define close_sem           for (int j = 0; j < SEM_COUNT; j++)\
                            { if (sem_ids[j] != NULL) sem_close(sem_ids[j]); }

/**
 * Close ouptut file (if file pointer is NULL, file is not open, do nothing)
 */
#define close_file          if (file != NULL) { fclose(file); }

/**
 * Correctly end work with shared memory and semaphores
 */
#define clean_shm_sem       clean_shm clean_sem

/**
 * Correctly end work with shared memory and semaphores, and close output file
 */
#define clean_shm_sem_file  clean_shm clean_sem close_file                            

/**
 * Terminate all hanging processes
 * @param count number of created child processes
 */
#define clean_proc(count)   for (int j = 0; j < count; j++)\
                            { kill(pids[j], SIGTERM); }\
                            free2(pids);

/**
 * Detach shared memory (used in child processes)
 */
#define detach_shm          if ((shmdt(shm_p)) < 0) { return ESHM_UMOUNT; }

/**
 * Generate random number between 0 and val
 * If val is 0, return 0
 * @param val
 */
#define rand_safe(val)      (((val) != 0) ? rand() % (val) : 0)     

/**
 * Lock semaphore, used in *child* processes
 * If locking is not successfull, close output file, detach shared memory
 * and return with error code
 * @param sem semaphore
 */
#define lock(sem)           if (sem_wait(sem_ids[sem]) < 0)\
                            { close_file detach_shm return ESEM_LOCK; }

/**
 * Unlock semaphore, used in *child* processes
 * If unlocking is not successfull, close output file, detach shared memory
 * and return with error code
 * @param sem semaphore 
 */
#define unlock(sem)         if (sem_post(sem_ids[sem]) < 0)\
                            { close_file detach_shm return ESEM_UNLOCK; }

/**
 * Lock semaphore sem, do action, unlock semaphore sem
 * @param val
 */
#define sync(sem, val) lock(sem) val unlock(sem)


// Enums, structures, messages, etc.
// -----------------------------------------------------------------------------

/** Program error codes */
enum tecodes
{
    EOK = 0,         /**< Without error */
    EPARAM,          /**< Bad command line parameters */
    EPAR_NUM,        /**< Parameter is not number, but it should be */
    EPAR_NUM_NEG,    /**< Paramter is a negative number, it should'nt be */

    ESHM_KEY,        /**< Shared memory: unable to create key */
    ESHM_ALLOC,      /**< Shared memory: unable to allocate shared memory */
    ESHM_MOUNT,      /**< Shared memory: unable to mount shared memory */
    ESHM_UMOUNT,     /**< Shared memory: unable to umount shared memory */

    ESEM_CREATE,     /**< Semapohre: unable to create */
    ESEM_LOCK,       /**< Semaphore: unable to lock */
    ESEM_UNLOCK,     /**< Semaphore: unable to unlock */

    EPROC_CREATE,    /**< Child process: unable to create */
    EPROC_WAIT,      /**< Child process: unable to wait for child processes*/

    EFILE,           /**< Unable to open file for writing */
    EOUT_MEM,        /**< Out of memory */

    EUNKNOWN         /**< Unknown error */
};

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Bad command line parameters!\n",
    /* EPAR_NUM */
    "Error: Q or GenC or GenB or N is not a number!\n",
    /* EPAR_NUM_NEG */
    "Error: Q or GenC or GenB or N can not be negative!\n",

    /* ESHM_KEY */
    "Error: Unable to create key for shared memory!\n",
    /* ESHM_ALLOC */
    "Error: Unable to allocate shared memory!\n",
    /* ESHM_MOUNT */
    "Error: Unable to mount shared memory!\n",
    /* ESHM_UMOUNT */
    "Error: Unable to unmount shared memory!\n",

    /* ESEM_CREATE */
    "Error: Unable to create semaphore: \"%s\"!\n",
    /* ESEM_LOCK */
    "Error: Unable to lock semaphore!\n",
    /* ESEM_UNLOCK */
    "Error: Unable to unlock semaphore!\n",

    /* EPROC_CREATE */
    "Error: Unable to create new process!\n",
    /* EPROC_WAIT */
    "Error: Unable to wait for process - %d!\n",

    /* EFILE */
    "Error: Unable to open file (\"%s\") for writing!\n",
    /* EOUT_MEM */
    "Error: Out of memory. Sorry!\n",

    /* EUNKNOWN */
    "Error: Unknown!\n"
};

/** Status messages enum */
enum statcodes
{
    BAR_CHECKS = 0,    /**< Barber cheks the waiting room */
    BAR_READY,         /**< Barber ready to serve customer */
    BAR_FINISHED,      /**< Barber has finished serving the customer */
    CUS_CREATED,       /**< Customer created */
    CUS_ENTERS,        /**< Customer enters the waiting room */
    CUS_READY,         /**< Customer is ready to be served */
    CUS_SERVED,        /**< Customer has been served */
    CUS_REFUSED        /**< Customer has been refused */
};

/** Status messages */
const char *STATMSG[] =
{
    /* holic kontroluje cekarnu, zda je dalsi zakaznik */
    "%u: barber: checks\n", 

    /* holic je pripraven prijmout zakaznika z cekarny */
    "%u: barber: ready\n",

    /* holic ostrihal zakaznika */
    "%u: barber: finished\n",

    /* zakaznik je vytvoren */
    "%u: customer %u: created\n",

    /* zakaznik vstupuje do cekerny */
    "%u: customer %u: enters\n",

    /* zakaznik useda do kresla holice; 
       tiskne se aa pote, co je holic pripraven (barber: ready) */
    "%u: customer %u: ready\n",

    /* zakaznik odchazi ostrihan;
       tiskne se az pote, co holic dokoncil strihani(barber: finished):*/
    "%u: customer %u: served\n",

    /* zakaznik odchazi neobslouzen */
    "%u: customer %u: refused\n"
};

/** Semaphore names enum */
enum semcodes
{
    SEM_CC,      /**< Semaphore for access customer count */
    SEM_FILE,    /**< Semaphore for output file */
    SEM_CUS,     /**< Semaphore for customer */
    SEM_BAR,     /**< Semaphore for barber */
    SEM_SEAT ,   /**< Semaphore for access seats */
    SEM_SERVING  /**< Semaphore for serving customer */
};

/** Semaphore names */
const char *SEM_NAME[] =
{
    /* Customer count */
    "XMOLNA02_CC_10",
    /* Output file*/
    "XMOLNA02_OUTPUT_FILE_20",
    /* Customers */
    "XMOLNA02_CUSTOMERS_30",
    /* Barber */
    "XMOLNA02_BARBER_40",
    /* Access seats*/
    "XMOLNA02_SEATS_50",
    /* Serving customer */
    "XMOLNA02_SERVING_60"
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
    unsigned int q;      /**<  Free seats in waiting room */
    unsigned int gen_c;  /**<  Max time waiting between generating customers */
    unsigned int gen_b;  /**<  Max time barber serves the customer */
    unsigned int n;      /**<  Total count of customeres (served + refused) */
    char *filename;      /**<  Output files name */
    int ecode;           /**<  Error code (enum tecodes) */
} tParams;

/**
 * Structure used in shared memory
 */
typedef struct shared_data_struct
{
    unsigned int stat_counter;     /**< Status counter (A) */
    unsigned int free_seat_count;  /**< Free seat counter */
    unsigned int customer_count;   /**< Customer counter */
} tSd_struct;


// Functions
// -----------------------------------------------------------------------------

/**
 * Prints error message to stderr
 * @param ecode error code
 */
void print_ecode(int ecode, ...)
{
    if (ecode < EOK || ecode > EUNKNOWN)
    { 
        ecode = EUNKNOWN; 
    }

    va_list arg;

    va_start(arg, ecode);

    vfprintf(stderr, ECODEMSG[ecode], arg);

    va_end(arg);
}

/**
 * Frees a pointer if it's not NULL
 * @param *p pointer to free
 */
void free2(void *p)
{
    if (p != NULL)
    {
        free(p);
    }
}

/**
 * Convert char* to an unsigned int
 * If an error occured, the return value will contain the error code
 * @param *str
 * @param *result
 */
int convert2int(char *str, unsigned int *result)
{
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || 
        (errno != 0 && val == 0) || endptr == str)
    {
        return EPAR_NUM;
    }
    
    if (val < 0)
    {
        return EPAR_NUM_NEG;
    }

    *result = (unsigned int)val;

    return EOK;
}

/**
 * Processes arguments of command line
 * @param argc Argument count
 * @param argv Array of string with arguments
 */
tParams get_params(int argc, char *argv[])
{
    tParams result = 
    {  // init structure
        .q = 0,
        .gen_c = 0,
        .gen_b = 0,
        .n = 0,
        .filename = NULL,
        .ecode = EOK
    };

    if (argc == 6)
    {
        // Q
        result.ecode = convert2int(argv[1], &result.q);
    
        // GenC
        if (result.ecode == EOK)
        {
            result.ecode = convert2int(argv[2], &result.gen_c);
        }

        // GenB
        if (result.ecode == EOK)
        {
            result.ecode = convert2int(argv[3], &result.gen_b);
        }

        // N
        if (result.ecode == EOK)
        {
            result.ecode = convert2int(argv[4], &result.n);
        }

        // filename
        if (result.ecode == EOK)
        {
            result.filename = argv[5];
        }
    }
    else
    {
        result.ecode = EPARAM;
    }

    return result;
}

/**
 * Prints a message about current status
 * @param *file
 * @param *format
 */
void print_status(FILE *file, const char *format, ...)
{
    FILE *stream = (file != NULL) ? file : stdout;

    va_list arg;

    va_start(arg, format);

    vfprintf(stream, format, arg);

    va_end(arg);
}

/**
 * Barber's process
 * @param shm_id shared memory id
 * @param **sem_ids sem_t* array
 * @param *file output file
 * @param max_wait max time in ms serving the customer
 */
int do_barber(int shm_id, sem_t** sem_ids, FILE *file, int max_wait, int q)
{
    // Init random number generator
    srand(time(NULL));

    // Mount shared memory
    tSd_struct *shm_p;

    if ((shm_p = (tSd_struct*)shmat(shm_id, NULL, 0)) == NULL)
    {
        return ESHM_MOUNT;
    }

    // Endless loop - serving customers
    // Breaks only when there are no more customers (served or refused)
    while (1)
    {
        // check if there are unsatisfied customers
        sync(SEM_CC,
             if (shm_p->customer_count <= 0)
             {
                 unlock(SEM_CC)
                 break;
             }
        )
    
        // *** BARBER CHECKS ***
        sync(SEM_FILE,
             print_status(file, STATMSG[BAR_CHECKS], shm_p->stat_counter);
             shm_p->stat_counter += 1;
        )

        // waiting room has no chairs, exit...        
        if (q == 0)
        {
            break;
        }

        // Wiki: tries to acquire a customer
        //       if none is available he goes to sleep
        lock(SEM_CUS)

        // *** BARBER READY ***
        sync(SEM_FILE,
             print_status(file, STATMSG[BAR_READY], shm_p->stat_counter);
             shm_p->stat_counter += 1;
        )

        // Wiki: at this time he has been awakened
        //       want to modify the number of available seats
        lock(SEM_SEAT)

        // Wiki: one chair gets free
        shm_p->free_seat_count += 1;

        sync(SEM_SERVING,

            // Wiki: the barber is ready to cut
            unlock(SEM_BAR)

            // Wiki: we don't need the lock on the chairs anymore
            unlock(SEM_SEAT)
                       
            // Wiki: here the barber is cutting hair
            usleep(1000 * rand_safe(max_wait));
            //max_wait = 0;
            //usleep(1000 * 1000 * 10);
            
            // One customer served, decrement the counter
            sync(SEM_CC,
                 shm_p->customer_count -= 1;        
            )

            // *** BARBER FINISHED ***
            sync(SEM_FILE,
                 print_status(file, STATMSG[BAR_FINISHED], shm_p->stat_counter);
                 shm_p->stat_counter += 1;
            )
        )    
    }

    // Close output file
    close_file

    // Unmount shared memory
    detach_shm
    
    return EOK;
}

/**
 * Customer's process
 * @param index unique id of customer
 * @param shm_id shared memory id
 * @param **sem_ids sem_t* array
 * @param *file output file
 */
int do_customer(int index, int shm_id, sem_t** sem_ids, FILE *file)
{
    // Mount shared memory
    tSd_struct *shm_p;

    if ((shm_p = (tSd_struct*)shmat(shm_id, NULL, 0)) == NULL)
    {
        return ESHM_MOUNT;
    }

    // *** CUSTOMER CREATED ***
    sync(SEM_FILE,
         print_status(file, STATMSG[CUS_CREATED], shm_p->stat_counter, index);
         shm_p->stat_counter += 1;
    )

    // lock customer counter too! it's very important when waiting room is
    // full and customer is refused
    lock(SEM_CC)

    // Wiki: tries to get access to the chairs
    lock(SEM_SEAT)

    // *** CUSTOMER ENTERS ***
    sync(SEM_FILE,
         print_status(file, STATMSG[CUS_ENTERS], shm_p->stat_counter, index);
         shm_p->stat_counter += 1;
    )
    
    if (shm_p->free_seat_count > 0)
    { 
        // Wiki: if there are any free seats

        // Don't need to lock customer counter anymore
        unlock(SEM_CC)

        // Wiki: sitting down on a chair
        shm_p->free_seat_count -= 1;

        // Wiki: notify the barber, who's waiting that there is a customer
        unlock(SEM_CUS)
        
        // Wiki: don't need to lock the chairs anymore
        unlock(SEM_SEAT)
       
        // Wiki: now it's this customer's turn, but wait if the barber is busy
        lock(SEM_BAR)

        // *** CUSTOMER READY ***
        sync(SEM_FILE,
             print_status(file, STATMSG[CUS_READY], shm_p->stat_counter, index);
             shm_p->stat_counter += 1;
        )

        // Wiki: here the customer is having his hair cut
        // Waiting for barber
        sync(SEM_SERVING, ;)

        // *** CUSTOMER SERVED ***
        sync(SEM_FILE,
             print_status(file, STATMSG[CUS_SERVED], 
                                shm_p->stat_counter, index);
             shm_p->stat_counter += 1;
        )
    }
    else
    {      
        // Wiki: there are no free seats

        // One customer refused, decrement the counter
        shm_p->customer_count -= 1;

        // Don't need customer counter anymore
        unlock(SEM_CC)

        // Wiki: but don't forget to release the lock on the seats
        unlock(SEM_SEAT)

        // Wiki: customer leaves without a haircut

        // *** CUSTOMER REFUSED ***
        sync(SEM_FILE,
           print_status(file, STATMSG[CUS_REFUSED], shm_p->stat_counter, index);
           shm_p->stat_counter += 1;
        )      
    } 

    // Close output file
    close_file

    // Unmount shared memory
    detach_shm

    return EOK;
}


// Function main
// -----------------------------------------------------------------------------

/**
 * Main program
 */
int main(int argc, char *argv[])
{
    // Process parameters
    // -------------------------------------------------------------------------
    
    tParams params = get_params(argc, argv);

    if (params.ecode != EOK)
    { // wrong parameters
        print_ecode(params.ecode);
        return EXIT_FAILURE;
    }


    // Init random number generator
    // -------------------------------------------------------------------------
    
    srand(time(NULL));


    // Init shared memory
    // -------------------------------------------------------------------------

    // Id of shared memory
    int shm_id;

    // Key for shared memory
    key_t shm_key;

    // Pointer to shared memory
    tSd_struct *shm_p;

    // Init of shared memory key
    if ((shm_key = ftok(SHMKEY_PATH, SHMKEY_ID)) < 0)
    {
        print_ecode(ESHM_KEY);      
        return EXIT_FAILURE;
    }

    // Allocate shared memory
    // Rigths: User, group, others - read and write (0666)
    if ((shm_id = shmget(shm_key, sizeof(tSd_struct), 
                                  0666 | IPC_CREAT | IPC_EXCL)) < 0)
    {
        print_ecode(ESHM_ALLOC);
        return EXIT_FAILURE;
    }

    // Mount shared memory
    if ((shm_p = (tSd_struct*)shmat(shm_id, NULL, 0)) == NULL)
    {        
        print_ecode(ESHM_MOUNT);
        clean_shm
        return EXIT_FAILURE;
    }

    // Init variables in shared memory 	
    shm_p->stat_counter = 1;
    shm_p->free_seat_count = params.q;
    shm_p->customer_count = params.n;

    // Unmount shared memory
    if ((shmdt(shm_p)) < 0)
    {
        print_ecode(ESHM_UMOUNT);
        clean_shm
        return EXIT_FAILURE;
    }


    // Init semaphores
    // -------------------------------------------------------------------------

    sem_t *sem_ids[SEM_COUNT] = { 0 };
    int val;

    for (int i = 0; i < SEM_COUNT; i++)
    {
        val = (i == SEM_CUS || i == SEM_BAR) ? 0 : 1;

        // Allocating semaphore
        // Only owner (user) can read and write the semaphore!
        if ((sem_ids[i] = sem_open(SEM_NAME[i], O_CREAT, S_IRUSR | S_IWUSR, val)
            ) == SEM_FAILED)
        {
            print_ecode(ESEM_CREATE, SEM_NAME[i]);
            clean_shm_sem
            return EXIT_FAILURE;
        } 
    }


    // Init output file
    // -------------------------------------------------------------------------

    FILE *file = NULL;
    if (strcmp(params.filename, STDOUT_SWITCH) != 0)
    {
        if ((file = fopen(params.filename, "w")) == NULL)
        {
            print_ecode(EFILE, params.filename);
            clean_shm_sem
            return EXIT_FAILURE;
        }

        setbuf(file, NULL);
    }
    else
    {
        setbuf(stdout, NULL);
    }
    
    // Init processes
    // -------------------------------------------------------------------------

    pid_t pid;
    int proc_count = params.n + 1;
    pid_t *pids = (pid_t*)malloc(proc_count * sizeof(pid_t));

    if (pids == NULL)
    {
        print_ecode(EOUT_MEM);
        clean_shm_sem_file
        return EXIT_FAILURE;
    }

    for (int i = 0; i < proc_count; i++)
    {
        pid = fork();

        if (pid == 0)
        { // child
            if (i == 0)
            { // barber
                int result = do_barber(shm_id, sem_ids, file, 
                                               params.gen_b, params.q);

                free2(pids);
             
                if (result != EOK)
                {
                    print_ecode(result);
                    clean_shm_sem
                    kill(getppid(), SIGTERM);
                    return EXIT_FAILURE;
                }

                close_sem
                
                return EXIT_SUCCESS;
            }
            else
            { // customer
                int result = do_customer(i, shm_id, sem_ids, file);

                free2(pids);
    
                if (result != EOK)
                {
                    print_ecode(result);
                    clean_shm_sem
                    kill(getppid(), SIGTERM);
                    return EXIT_FAILURE;
                }

                close_sem
                
                return EXIT_SUCCESS;
            }   
        }
        else if (pid < 0)
        { // error
            print_ecode(EPROC_CREATE);
            clean_proc(i)
            clean_shm_sem_file
            return EXIT_FAILURE;
        }
        else 
        { // parent
            pids[i] = pid;   

            // wait only *between* customers
            if (i != 0 && i != (proc_count - 1))
            {
                usleep(1000 * rand_safe(params.gen_c));
            }
        }
    }


    // Waiting child processes to die
    // -------------------------------------------------------------------------

    int status;
    int result;

    for (int i = 0; i < proc_count; i++)
    {
        result = waitpid(pids[i], &status, 0);

        if (result < 0 || !(WIFEXITED(status)) ||
            WEXITSTATUS(status) != EXIT_SUCCESS)
        {            
            print_ecode(EPROC_WAIT, i);

            clean_proc(proc_count)
            clean_shm_sem_file           
            return EXIT_FAILURE;
        }
    }


    // Free resources
    // -------------------------------------------------------------------------
    
    // Kill processes (all processes should be terminated)
    clean_proc(proc_count)

    // Free shared memory, semaphores and close output file
    clean_shm_sem_file

    return EXIT_SUCCESS;
}

/* end of barbers.c */
