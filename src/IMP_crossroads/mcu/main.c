/*
 * File:     main.c
 * Date:     2011-12-05
 * Encoding: ISO-8859-2
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Model riadenia prevadzky na svetelnej krizovatke
 */

// Includes
// ----------------------------------------------------------------------------

#include <fitkitlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include <keyboard/keyboard.h>
#include <lcd/display.h>

#include <string.h>

#include "utils.h"
#include "charmap.h"


// Constants, enums, chars
// ----------------------------------------------------------------------------

// How many walkers do we have
#define WALKER_DELAY_LEN 4

// Waiting times in seconds
#define T_STOP      1
#define T_ATT_1     2
#define T_ATT_2     3
#define T_GO        10
#define T_W_GO      7
#define T_GO_DIFF   3 // Time before the end of phase GO to deactivate walker 

// Some strings
char str_mode_day[] = "DAY";
char str_mode_night[] = "NGT";
char str_mode_off[] = "OFF";

char str_rtype_nor[] = "NOR";
char str_rtype_urg[] = "URG";

char str_way0[] = "NORTH, SOUTH";
char str_way1[] = "EAST, WEST";

const char *str_way[] = 
{
    "NORTH",
    "SOUTH",
    "EAST",
    "WEST"
};

// Mode enum
enum e_mode
{
    M_DAY,
    M_NIGHT,
    M_OFF
};

// Reactivity typ (NORMAL/URGENT)
enum e_rtype
{
    R_NOR,
    R_URG
};

// Car semaphore states
enum e_car
{
    C_STOP,      // STOP
    C_ATT,       // ATTENTION
    C_GO,        // GO
    C_STOP_ATT,  // ATTENTION
    C_NIGHT,     // NIGHT (Blink orange)
    C_OFF        // OFF
};

// Walker semaphore states
enum e_walker
{
    W_STOP,     // STOP
    W_GO,       // GO
    W_NIGHT,    // NIGHT
    W_OFF       // OFF
};

// Orange light for cars states
enum e_oran
{
    O_ATT,      // ATTENTION (ON)
    O_OFF       // OFF
};

// Phases in simulation
enum e_phase
{
    F_STOP,
    F_ATT,
    F_GO,
    F_STOP_ATT,
    F_BLINK,
    F_OFF
};

// Ways
enum e_way
{
    W_NORTH,
    W_SOUTH,
    W_EAST,
    W_WEST
};


// Variables
// ----------------------------------------------------------------------------

volatile char last_ch;

// States
unsigned short mode, last_mode;
unsigned short rtype;
unsigned short phase;

// Time management
unsigned long time;
unsigned long task_delay;
unsigned long walker_delay[WALKER_DELAY_LEN];

// Semaphores for cars
unsigned short sem_c_n, sem_c_s, sem_c_e, sem_c_w;
unsigned short sem_c_n_oran, sem_c_s_oran, sem_c_e_oran, sem_c_w_oran;

// Semaphores for walkers
unsigned short sem_w_n_l, sem_w_s_l, sem_w_e_l, sem_w_w_l;
unsigned short sem_w_n_r, sem_w_s_r, sem_w_e_r, sem_w_w_r;

// Signals from walkers
unsigned short sig_w_n_l, sig_w_s_l, sig_w_e_l, sig_w_w_l;

// FreeRTOS handle for task X
xTaskHandle xHandle;

// Hour, minute, second
unsigned short clk_h, clk_m, clk_s;

// Flags
unsigned short flag_timer, flag_clock, flag_way;


// Functions
// ----------------------------------------------------------------------------

/**
 * Display time on LCD (format HH:MM:SS)
 */
void display_clock() 
{
    if (flag_clock)
    {
        LCD_send_cmd('0', 1);
        LCD_send_cmd('0', 1);
        LCD_send_cmd(':', 1);
        LCD_send_cmd('0', 1);
        LCD_send_cmd('0', 1);
        LCD_send_cmd(':', 1);
        
        int t = (task_delay > 0) ? (task_delay - time) : 0;
        LCD_send_cmd((unsigned char)(t / 10) + 48, 1);
        LCD_send_cmd((unsigned char)(t % 10) + 48, 1);
        
        return;
    }

    // Hours
    LCD_send_cmd((unsigned char)(clk_h / 10) + 48, 1); 
    LCD_send_cmd((unsigned char)(clk_h % 10) + 48, 1);
    LCD_send_cmd(0x3A, 1);
    
    // Minutes
    LCD_send_cmd((unsigned char)(clk_m / 10) + 48, 1);
    LCD_send_cmd((unsigned char)(clk_m % 10) + 48, 1); 
    LCD_send_cmd(0x3A, 1);
    
    // Seconds
    LCD_send_cmd((unsigned char)(clk_s / 10) + 48, 1);
    LCD_send_cmd((unsigned char)(clk_s % 10) + 48, 1);  
}

/**
 * Display current mode on LCD (DAY/NGT/OFF)
 */
void display_mode()
{
    char *str = str_mode_day;
    
    switch (mode)
    {
        case M_DAY: str = str_mode_day; break;
        case M_NIGHT: str = str_mode_night; break;
        case M_OFF: str = str_mode_off; break;
    }
    
    int i;
    for (i = 0; i < 3; i++)
    {
        LCD_send_cmd(str[i], 1);
    }
}

/**
 * Display current reactivity type on LCD (NOR/URG)
 */
void display_rtype()
{
    char *str = str_rtype_nor;
    
    if (rtype == R_URG)
    {
        str = str_rtype_urg;
    }
    
    int i;
    for (i = 0; i < 3; i++)
    {
        LCD_send_cmd(str[i], 1);
    }
}

/**
 * Display one semaphore on LCD (car, orange, walker)
 */
void display_sem(unsigned short car, unsigned short walker, unsigned short oran)
{
    char c;
    
    // Car
    switch (car)
    {
        case C_STOP:     c = '\x1'; break;
        case C_ATT:      c = '\x2'; break;
        case C_GO:       c = '\x3'; break;
        case C_STOP_ATT: c = '\x6'; break;
        case C_NIGHT:    c = (P1OUT & 0x01) ? '\x6' : ' '; break;
        case C_OFF:      c = ' ';   break;
    }
    
    LCD_send_cmd(c, 1);
    
    // Orange light for cars
    switch (oran)
    {
        case O_ATT: c = '\x6'; break;
        case O_OFF: c = ' ';   break;
    }
    
    LCD_send_cmd(c, 1);
    
    // Walker
    switch (walker)
    {
        case W_STOP:  c = '\x4'; break;
        case W_GO:    c = '\x5'; break;
        case W_NIGHT: c = ' ';   break;
        case W_OFF:   c = ' ';   break;
    }
    
    LCD_send_cmd(c, 1);
}

/**
 * Display all semaphores on LCD
 */
void display_all_sem()
{
    // NORTH
    display_sem(sem_c_n, sem_w_n_l, sem_c_n_oran);
    LCD_send_cmd(' ', 1);
    
    // SOUTH
    display_sem(sem_c_s, sem_w_s_l, sem_c_s_oran);
    LCD_send_cmd(' ', 1);
    LCD_send_cmd(' ', 1);
    
    // EAST
    display_sem(sem_c_e, sem_w_e_l, sem_c_e_oran);
    LCD_send_cmd(' ', 1);
    
    // WEST
    display_sem(sem_c_w, sem_w_w_l, sem_c_w_oran);
}

/**
 * Display info on LCD
 */
void display_info()
{
    // Set cursor to first line
    LCD_send_cmd(LCD_SET_DDRAM_ADDR, 0);
    
    display_all_sem();
    
    // Set cursor to second line
    LCD_send_cmd(LCD_SET_DDRAM_ADDR | LCD_SECOND_HALF_OFS, 0);
    
    display_mode();
    
    LCD_send_cmd(' ', 1);
    
    display_clock();
    
    LCD_send_cmd(' ', 1); 
    
    display_rtype();
}

/**
 * Suspend x task for delay
 * If delay is zero, don't set task_delay
 * @param delay seconds to suspend x task
 */
void do_delay(int delay)
{
    if (delay != 0)
    {
        task_delay = time + delay;
    }
    
    vTaskSuspend(xHandle);
}

/**
 * Resume x task
 */
void skip()
{
    task_delay = 0;
    vTaskResume(xHandle);
}

/**
 * Turn on orange light for cars
 */
void orange_on(unsigned short walker)
{
    char *str;
    switch (walker)
    {
        case W_NORTH: sem_c_n_oran = sem_c_s_oran = O_ATT; str = str_way0; break;
        case W_SOUTH: sem_c_n_oran = sem_c_s_oran = O_ATT; str = str_way0; break;
        case W_EAST:  sem_c_e_oran = sem_c_w_oran = O_ATT; str = str_way1; break;
        case W_WEST:  sem_c_e_oran = sem_c_w_oran = O_ATT; str = str_way1; break;
    }

    term_send_str("[DAY]: ON orange light for cars ");
    term_send_str_crlf(str);
}

/**
 * Turn on one walker
 * @param walker which walker to turn on
 */
void seize_walker(unsigned short walker)
{
    if ((task_delay - time) < T_GO_DIFF)
    {
        // Not enough time
        return;
    }

    switch (walker)
    {
        case W_NORTH: sem_w_n_l = sem_w_n_r = W_GO; break;
        case W_SOUTH: sem_w_s_l = sem_w_s_r = W_GO; break;
        case W_EAST:  sem_w_e_l = sem_w_e_r = W_GO; break;
        case W_WEST:  sem_w_w_l = sem_w_w_r = W_GO; break;
    }
    
    // Schedule turn off
    if (task_delay > 0)
    {
        walker_delay[walker] = task_delay - T_GO_DIFF;
    }
    else
    {
        walker_delay[walker] = time + T_W_GO;
    }
   
    term_send_str("[DAY] 2: GO for walkers (LEFT and RIGHT) ");
    term_send_str_crlf(str_way[walker]);
    
    term_send_str("[DAY] 2: GO audio signals for walkers (both LEFT and RIGHT) ");
    term_send_str_crlf(str_way[walker]);
}

/**
 * Turn off one walker
 * @param walker which walker to turn off
 */
void release_walker(unsigned short walker)
{
    switch (walker)
    {
        case W_NORTH: sem_w_n_l = sem_w_n_r = W_STOP; sem_c_e_oran = sem_c_w_oran = O_OFF; sig_w_n_l = 0; break;
        case W_SOUTH: sem_w_s_l = sem_w_s_r = W_STOP; sem_c_e_oran = sem_c_w_oran = O_OFF; sig_w_s_l = 0; break;
        case W_EAST:  sem_w_e_l = sem_w_e_r = W_STOP; sem_c_s_oran = sem_c_n_oran = O_OFF; sig_w_e_l = 0; break;
        case W_WEST:  sem_w_w_l = sem_w_w_r = W_STOP; sem_c_s_oran = sem_c_n_oran = O_OFF; sig_w_w_l = 0; break;
    }
   
    walker_delay[walker] = 0;

    term_send_str("[DAY] 2: STOP for walkers (LEFT and RIGHT) ");
    term_send_str_crlf(str_way[walker]);
    
    term_send_str("[DAY] 2: STOP audio signals for walkers (both LEFT and RIGHT) ");
    term_send_str_crlf(str_way[walker]);
    
    term_send_str("[DAY] 2: OFF orange light for cars ");
    term_send_str_crlf(!flag_way ? str_way0 : str_way1);
}


// Bodies of the RT tasts running over FreeRTOS
// ----------------------------------------------------------------------------

/**
 * Keyboard service task
 * @param *param parameters
 */
static void keyboard_task(void *param) 
{
    char ch;
    char *str;

    last_ch = 0;
    keyboard_init();

    while (1)
    {
        set_led_d6(0);

        ch = key_decode(read_word_keyboard_4x4());
        
        if (ch != last_ch)
        {
            last_ch = ch;
            
            if (ch != 0) 
            {
                switch (ch) 
                {
                    case '1':
                        // Flip timer modes
                        
                        set_led_d6(1);
                        
                        flag_clock = !flag_clock;
                        
                        flag_timer += 1;
                        term_send_str("Timer: ");
                        
                        if (flag_clock)
                        {
                            term_send_str_crlf("remaining time");
                        }
                        else
                        {
                            term_send_str_crlf("current time");
                        }
                        break;
                
                    case 'A':
                        // NORTH walker signal
                        
                        if (mode != M_DAY)
                        {
                            break;
                        }
                    
                        set_led_d6(1);
                        
                        sig_w_n_l = 1;
                        orange_on(W_EAST);

                        if (flag_way && phase == F_GO)
                        {
                            seize_walker(W_NORTH);
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf("Signal from walker NORTH LEFT");
                        break;
                        
                    case 'B':
                        // SOUTH walker signal
                        
                        if (mode != M_DAY)
                        {
                            break;
                        }
                    
                        set_led_d6(1);
                        
                        sig_w_s_l = 1;
                        orange_on(W_WEST);
                        
                        if (flag_way && phase == F_GO)
                        {
                            seize_walker(W_SOUTH);
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf("Signal from walker SOUTH LEFT");
                        break;
                        
                    case 'C':
                        // EAST walker signal
                        
                        if (mode != M_DAY)
                        {
                            break;
                        }
                    
                        set_led_d6(1);
                        
                        sig_w_e_l = 1;
                        orange_on(W_NORTH);
                        
                        if (!flag_way && phase == F_GO)
                        {
                            seize_walker(W_EAST);
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf("Signal from walker EAST LEFT");
                        break;
                        
                    case 'D':
                        // WEST walker signal
                        
                        if (mode != M_DAY)
                        {
                            break;
                        }
                    
                        set_led_d6(1);
                        
                        sig_w_w_l = 1;
                        orange_on(W_SOUTH);
                        
                        if (!flag_way && phase == F_GO)
                        {
                            seize_walker(W_WEST);
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf("Signal from walker WEST LEFT");
                        break;
                        
                    case '*':
                        // Skip current phase (Only in URGENT reactivity)
                        
                        if (rtype != R_URG)
                        {
                            break;
                        }
                    
                        set_led_d6(1);
                        
                        skip();
                        
                        int i;
                        for (i = 0; i < WALKER_DELAY_LEN; i++)
                        {
                            if (walker_delay[i] > 0)
                            {
                                release_walker(i);
                            }
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf("Skipping current phase...");
                        break;
                        
                    case '#':
                        // Flip mode
                        // M_DAY -> M_NIGHT -> M_DAY
                    
                        set_led_d6(1);
                        
                        term_send_str("New mode: ");
                        
                        switch (mode)
                        {
                            case M_DAY:   mode = M_NIGHT; str = str_mode_night; break;
                            case M_NIGHT: mode = M_DAY;   str = str_mode_day; flag_way = 0; vTaskResume(xHandle); break;
                            case M_OFF: str = str_mode_off;
                        }
                        
                        flag_timer += 1;
                        term_send_str_crlf(str);
                        break;
                        
                    case '0':
                        // Turn on/off simulation
                    
                        set_led_d6(1);
                        
                        term_send_str("Simulation is now ");

                        if (mode != M_OFF)
                        {
                            last_mode = mode;
                            mode = M_OFF;
                            term_send_str_crlf("OFF");
                        }
                        else
                        {
                            mode = last_mode;
                            flag_way = 0;
                            vTaskResume(xHandle);
                            term_send_str_crlf("ON");
                        }
                        
                        flag_timer += 1;
                        break;
                        
                    default:
                        break;
                }
            }
        }
        
        // Delay for 10 ms (= btn-press sampling period)
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

/**
 * Terminal service task
 * @param *param parameters
 */
static void terminal_task(void *param) 
{
    while (1)
    {
        terminal_idle();
        
        if (flag_timer > 0)
        {
            display_info();
            flag_timer--;
        }
        
        // Delay for 300 ms 
        vTaskDelay(300 / portTICK_RATE_MS);
    }
}

/**
 * Task controlling crossroad
 * @param *param parameters
 */
static void x_task(void *param) 
{
    // Init semaphores
    // Semaphores for cars
    sem_c_n = sem_c_s = sem_c_e = sem_c_w = C_STOP;
    sem_c_n_oran = sem_c_s_oran = sem_c_e_oran = sem_c_w_oran = O_OFF;

    // Semaphores for walkers
    sem_w_n_l = sem_w_s_l = sem_w_e_l = sem_w_w_l = W_STOP;
    sem_w_n_r = sem_w_s_r = sem_w_e_r = sem_w_w_r = W_STOP;

    // Signals from walkers
    sig_w_n_l = sig_w_s_l = sig_w_e_l = sig_w_w_l = 0;
    
    term_send_str_crlf("[INIT] STOP semaphores for cars NORTH, SOUTH, EAST, WEST");
    term_send_str_crlf("[INIT] STOP semaphores for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHT");
    term_send_str_crlf("[INIT] STOP audio signals for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHTs");
    
    term_send_str_crlf("Starting cycle...");
    term_send_crlf();

    while (1)
    {
        if (mode == M_DAY)
        {
            // -------------------- Phase 0 - STOP ----------------------------
            phase = F_STOP;

            // Semaphores for cars
            if (!flag_way)
            {
                sem_c_n = sem_c_s = C_STOP;
            }
            else
            {
                sem_c_e = sem_c_w = C_STOP;
            }
            
            term_send_str("[DAY] 0: STOP semaphores for cars ");
            term_send_str_crlf(!flag_way ? str_way0 : str_way1);
            
            if (sem_c_n != C_STOP || sem_c_s != C_STOP || sem_c_e != C_STOP || sem_c_w != C_STOP )
            {
                sem_c_n = sem_c_s = sem_c_e = sem_c_w = C_STOP;
                term_send_str("[DAY] 0: STOP semaphores for cars ");
                term_send_str_crlf(flag_way ? str_way0 : str_way1);
            }
            
            if (sem_w_n_l == W_OFF || sem_w_s_l == W_OFF || sem_w_e_l == W_OFF || sem_w_w_l == W_OFF || 
                sem_w_n_r == W_OFF || sem_w_s_r == W_OFF || sem_w_e_r == W_OFF || sem_w_w_r == W_OFF)
            {
                int i;
                for (i = 0; i < WALKER_DELAY_LEN; i++)
                {
                    release_walker(i);
                }
            }

            do_delay(T_STOP);
           
            // -------------------- Phase 1 - ATTENTION -----------------------
            phase = F_ATT;

            // Semaphores for cars
            if (!flag_way)
            {
                sem_c_n = sem_c_s = C_ATT;
            }
            else
            {
                sem_c_e = sem_c_w = C_ATT;
            }
            
            term_send_str("[DAY] 1: ATTENTION semaphores for cars ");
            term_send_str_crlf(!flag_way ? str_way0 : str_way1);
            
            do_delay(T_ATT_1);
            
            // -------------------- Phase 2 - GO ------------------------------
            phase = F_GO;

            task_delay = time + T_GO;
            
            // Semaphores for cars
            if (!flag_way)
            {
                sem_c_n = sem_c_s = C_GO;
                
                // Walkers
                if (sig_w_e_l)
                {
                    seize_walker(W_EAST);
                }
                
                if (sig_w_w_l)
                {
                    seize_walker(W_WEST);
                }
            }
            else
            {
                sem_c_e = sem_c_w = C_GO;
                
                // Walkers
                if (sig_w_n_l)
                {
                    seize_walker(W_NORTH);
                }
                
                if (sig_w_s_l)
                {
                    seize_walker(W_SOUTH);
                }
            }

            term_send_str("[DAY] 2: GO semaphores for cars ");
            term_send_str_crlf(!flag_way ? str_way0 : str_way1);
            
            do_delay(0);
            
            // -------------------- Phase 3 - ATTENTION -----------------------
            phase = F_STOP_ATT;

            // Semaphores for cars
            if (!flag_way)
            {
                sem_c_n = sem_c_s = C_STOP_ATT;
            }
            else
            {
                sem_c_e = sem_c_w = C_STOP_ATT;
            }            

            term_send_str("[DAY] 3: ATTENTION semaphores for cars ");
            term_send_str_crlf(!flag_way ? str_way0 : str_way1);
            
            do_delay(T_ATT_2);
            
            // -------------------- Phase 4 - STOP ----------------------------
            phase = F_STOP;

            if (!flag_way)
            {
                // Semaphores for cars
                sem_c_n = sem_c_s = C_STOP;
            }
            else
            {
                // Semaphores for cars
                sem_c_e = sem_c_w = C_STOP;
            }
            
            term_send_str("[DAY] 4: STOP semaphores for cars ");
            term_send_str_crlf(!flag_way ? str_way0 : str_way1);

            do_delay(T_STOP);
            
            // Flip way            
            flag_way = !flag_way;
            
            // delay for 30 ms 
            vTaskDelay(30 / portTICK_RATE_MS);
        }
        else if (mode == M_NIGHT)
        {
            // -------------------- Phase 0 - BLINK ---------------------------
            phase = F_BLINK;

            // Semaphores for cars
            sem_c_n = sem_c_s = sem_c_e = sem_c_w = C_NIGHT;
            sem_c_n_oran = sem_c_s_oran = sem_c_e_oran = sem_c_w_oran = O_OFF;

            // Semaphores for walkers
            sem_w_n_l = sem_w_s_l = sem_w_e_l = sem_w_w_l = W_OFF;
            sem_w_n_r = sem_w_s_r = sem_w_e_r = sem_w_w_r = W_OFF;

            // Signals from walkers
            sig_w_n_l = sig_w_s_l = sig_w_e_l = sig_w_w_l = 0;
            
            term_send_str_crlf("[NIGHT] 0: BLINK semaphores for cars NORTH, SOUTH, EAST, WEST");
            term_send_str_crlf("[NIGHT] 0: OFF semaphores for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHT");
            term_send_str_crlf("[NIGHT] 0: OFF audio signals for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHTs");
            
            vTaskSuspend(xHandle);
        }
        else if (mode == M_OFF)
        {
            // -------------------- Phase 0 - OFF -----------------------------
            phase = F_OFF;
            
            // Semaphores for cars
            sem_c_n = sem_c_s = sem_c_e = sem_c_w = C_OFF;
            sem_c_n_oran = sem_c_s_oran = sem_c_e_oran = sem_c_w_oran = O_OFF;

            // Semaphores for walkers
            sem_w_n_l = sem_w_s_l = sem_w_e_l = sem_w_w_l = W_OFF;
            sem_w_n_r = sem_w_s_r = sem_w_e_r = sem_w_w_r = W_OFF;

            // Signals from walkers
            sig_w_n_l = sig_w_s_l = sig_w_e_l = sig_w_w_l = 0;
            
            term_send_str_crlf("OFF semaphores for cars NORTH, SOUTH, EAST, WEST");
            term_send_str_crlf("OFF semaphores for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHT");
            term_send_str_crlf("OFF audio signals for walkers NORTH, SOUTH, EAST, WEST both LEFT and RIGHTs");

            vTaskSuspend(xHandle);
        }
    }
}

/**
 * Clock service crossroad
 * @param *param parameters
 */
static void clock_task(void *param) 
{
    while (1)
    {
        // Invert bit P1.0 (every 1/2 s)
        P1OUT ^= 0x01;      

        clk_s++;
       
        // Check x task for waking up
        if ((task_delay > 0) && (time >= task_delay))
        {
            skip();
        }
        
        // Check walkers for stopping
        int i;
        for (i = 0; i < WALKER_DELAY_LEN; i++)
        {
            if ((walker_delay[i] > 0) && (time >= walker_delay[i]))
            {
                release_walker(i);
            }
        }

        if (clk_s == 60) 
        {
            // Every 60. seconds increment minutes
            clk_m++; 
            clk_s = 0; 
        }
        
        if (clk_m == 60) 
        { 
            // Every 60. minutes increment hours
            clk_h++; 
            clk_m = 0; 
        }     
        if (clk_h == 24)                         
        {
            // Every 24 hours clear h, m, s
            clk_h = clk_m = clk_s = 0;
        }
        
        // Toggle mode based on time
        if (clk_h == 23 && clk_m == 0 && clk_s == 1)
        {
            mode = M_NIGHT;
        }
        
        if (clk_h == 4 && clk_m == 0 && clk_s == 0)
        {
            mode = M_DAY;
            flag_way = 0;
            vTaskResume(xHandle);
        }

        flag_timer += 1;
        
        time++;
    
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


// Misc functions
// ----------------------------------------------------------------------------

/**
 * Prints user help to terminal (on command 'help')
 */
void print_user_help() 
{ 
    term_send_str_crlf(" SET HH:MM:SS  ... set time");
}

/**
 * Decodes user input from terminal
 * @param *cmd_ucase
 * @param *cmd
 */
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
    unsigned char data[8];
  
    if (strcmp4(cmd_ucase, "SET ")) 
    {
        term_send_str("Current time: ");
        strcpy(data, get_data(cmd, 8));
        term_send_str_crlf(data);

        // Convert ASCII sstring hh:mm:ss to values hh, mm, ss
        clk_h = (data[0] - 48) * 10 + (data[1] - 48);   // hh
        clk_m = (data[3] - 48) * 10 + (data[4] - 48);   // mm
        clk_s = (data[6] - 48) * 10 + (data[7] - 48);   // ss
    } 
    else 
    {
        return CMD_UNKNOWN;
    }
  
    return USER_COMMAND;
}


// Inits preceding main() call
// ----------------------------------------------------------------------------

void fpga_initialized() 
{
    LCD_init();

    // Load user characters to LCD
    LCD_load_char(1, ch_car_stop);
    LCD_load_char(2, ch_car_att);
    LCD_load_char(3, ch_car_go);
    LCD_load_char(4, ch_walker_stop);
    LCD_load_char(5, ch_walker_go);
    LCD_load_char(6, ch_oran);
    
    // Turn off cursor
    LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);
}


// Top level code
// ----------------------------------------------------------------------------

int main( void ) 
{
    // HW init
    initialize_hardware();
    WDG_stop();
    
    // Init variables
    flag_timer = 0;
    flag_clock = 0;
    flag_clock = 0;

    time = 0;
    task_delay = 0;
    
    int i;
    for (i = 0; i < WALKER_DELAY_LEN; i++)
    {
        walker_delay[i] = 0;
    }

    mode = last_mode = M_DAY;
    rtype = R_URG;
    //rtype = R_NOR;

    // Init hours, minutes and seconds
    clk_h = 12;
    clk_m = clk_s = 0;

    // Set P1.0 to output (we will control green D5 LED
    P1DIR |= 0x01;
    
    // Init P1.0 to 1 (D5 is off)
    P1OUT ^= 0x01;

    term_send_crlf();

    // Install FreeRTOS tasks 
    term_send_str_crlf("Init FreeRTOS tasks...");
    
    xTaskCreate(terminal_task, "TERM", 200, NULL, 1, NULL);
    xTaskCreate(keyboard_task, "KBD", 32, NULL, 1, NULL);
    xTaskCreate(clock_task, "CLOCK", 64, NULL, 1, NULL);
    xTaskCreate(x_task, "X", 64, NULL, 1, &xHandle);

    // Start FreeRTOS kernel
    term_send_str_crlf("Starting FreeRTOS scheduler...\n");
    vTaskStartScheduler();

    return 0;
}
