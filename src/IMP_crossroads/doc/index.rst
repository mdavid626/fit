.. article:: apps_demo_msp_krizovatka
    :author: Dávid Molnár <xmolna02 AT stud.fit.vutbr.cz>
    :updated: 20111205

    Model riadenia prevádzky na svetelnej kriovatke

============================================================
Model riadenia prevádzky na svetelnej kriovatke (FreeRTOS)
============================================================

.. contents:: Obsah

Popis aplikácie
==================

S vyuitím prostriedka RT jadra FreeRTOS je pre MSP430 na FITkitu v jazyku C implementovanı model svetelnej kriovatke (``Kriovatka`` - CZ, ``Crossroad`` - EN)

tvorená 

1. ``tlaèidlami`` (sú vyuité nasledujúce tlaèidlá z maticovej klávesnice FITkitu: ``A``, ``B``, ``C``, ``D``, ``*``, ``0``, ``#``, ``1`` popísané nišie), 

2. ``riadiacou èasou`` (implementované v MSP430 pomocou prostriedka FreeRTOS) reagujúce na zmenu èasu a na stlaèenie tlaèidla a

3. ``riadenou èasou`` reprezentovanou pre jednoduchos premennımi v pamäti FITkitu.

Aplikáciu je mono slovne ``specifikova`` nasledovne:

Kriovatka v tvaru "X" s obojsmernou prevádzkou motorovıch vozidel (vzájomne kolmé silnice v smeroch svetovıch strán S, J, V, Z).
V blízkosti kríení ciest vedie cez kadú z ciest prechod pre chodce (zebra). 
U kadej zebry je semafor (svetelné signalizaèné zariadenie, SSZ) pre vozidla vjazdiace do kriovatky (trojfarebná sústava s plnımi signálmi - èervená, ltá, zelená - urèenımi k signalizácii povelu POZOR (èervená + ltá), VO¼NO (zelená), STOJ (èervená)). 

Ïalej je na kadej strane zebry semafor pre chodce (dvojfarebná sústava svetelnıch signálov èervená/STOJ, zelená/VO¼NO) vybavenı tlaèidlom pre (signalizáciu poiadavku) chodcov (na prechod zebrou) a schopnı generova doprovodné akustické signály STOJ/VO¾NO slúiace okrem inıch k orientácii nevidomıch.
Na kadom semaforu pre chodce je tie umiestenı signál ltého svetla v tvaru chodca upozoròujúci vodièov na to, e sa blíi k prechodu, prejazdom èoho by krioval volnı smer chodcov.
 
Sú podporované nasledujúce reimy èinnosti: denná prevádzka (DEÒ, aktivovaná v dobe od 04:00 do 23:00 vrátane), noèná prevádzka (NOC: prerušovanı svit ltıch plnıch signálov v dobe od 23:01 do 03:59 vrátane) a vypnutie signalizácie (OFF: vypnutie všetkıch signálov na základe vonkajšieho podnetu na zvláštnom riadiacom vstupu - tlaèidlo ``0``). 

Signálny cyklus (plán) reimu je rozloenı nasledovne:
 
``Reim DEÒ``: fáza 0 [1s] - STOJ pre vozidla S, J, V, Z a chodce S, J, V, Z, vypnutie signálu ltého svetla v tvaru chodca, fáza 1 [2s] - POZOR pre vozidla S, J, fáza 2 [10s] - VO¼NO pre vozidla S, J, (fáza [8s] pod¾a signalizácie od chodca: VO¼NO pre chodca S, J, zapnutie signálu ltého svetla v tvaru chodca), (fáza [0s] STOP pre chodca S, J, vypnutie signálu ltého svetla v tvaru chodca), fáza 3 [3s] - POZOR pre vozidla S, J, fáza 4 [1s] - STOP pre vozidla S, J. Fázy 0 a 4 opakujú, striedajú sa smery S, J a V, Z.

Signál VO¼NO pre chodca je do signálneho cyklu kriovatky zaradenı a na základe predchodzieho stlaèenia príslušného tlaèidla chodca.

Prechody medzi reimami DEN, NOC a OFF sú urèené nastavením typu reaktivity (RTYPE), ktorı môe nabıva hodnoty NORMAL alebo URGENT. Ak je RTYPE=NORMAL, sú prechody medzi reimami a zmeny v signálnom plánu moné len na konci cyklu tıchto reimov. Ak je RTYPE=URGENT, potom sú spomenuté prechody a zmeny moné i v ostatnıch fáziach reimu.

.. figure:: krizovatka.png
   :align: center

   Schéma kriovatky
   
   Pozn.: èervené: SZZ pre vozidla, lté: signál ltého svetla v tvaru chodca, zelené: SZZ pre chodce
   
Popis tlaèidiel a symbolov na LCD
==================================

Tlaèidlá z maticovej klávesnice FITkitu:

1. ``A`` - signalizácia od chodca SEVER ¾avı
2. ``B`` - signalizácia od chodca JUH ¾avı
3. ``C`` - signalizácia od chodca VİCHOD ¾avı
4. ``D`` - signalizácia od chodca ZÁPAD ¾avı
5. ``*`` - preskoèenie aktuálnej fázy (len v reime DEN a pri rtyspe=URGENT)
6. ``#`` - prepínanie reimu: DEN alebo NOC
7. ``1`` - prepínanie módu zobrazenia èasu: aktuálny èas alebo zostávajúci èas v aktuálnej fáze
8. ``0`` - prepnutie do reimu OFF, následnym stlaèením prepne naspä

Symboly na LCD display (5x8 pixel):

  Symbol trojfarebnej SZZ pre vozidla: STOP, POZOR a VO¼NO
  
::
  
  XXXXX   XXXXX   .....
  XX.XX   XX.XX   .....
  XXXXX   XXXXX   .....
  .....   XX.XX   .....
  .....   XXXXX   .....
  .....   .....   XXXXX
  .....   .....   XX.XX
  .....   .....   XXXXX
  
Symbol dvojfarebnej SZZ pre chodcov: STOP a VO¼NO

::

  XXXXX   .....
  XX.XX   .....
  XX.XX   .....
  XXXXX   .....
  .....   XXXXX
  .....   XX.XX
  .....   XX.XX
  .....   XXXXX
  
Symbol signálu ltého svetla v tvaru chodca: POZOR

::

  .....
  .....
  XXXXX
  XX.XX
  XX.XX
  XXXXX
  .....
  .....
  
Keï je svetlo vypnuté, na LCD sa zobrazí prázdne miesto.
  
Prvı riadok LCD:

::

  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
  | S | O | C |   | S | O | C |       | S  | O  | C  |    | S  | O  | C  |
  
1. ``Znaky 1-3``: severnı SZZ
2. ``Znaky 5-6``: junı SZZ
3. ``Znaky 10-12``: vıchodnı SZZ
4. ``Znaky 14-16``: západnı SZZ
  
1. ``S``: SZZ pre vozidla
2. ``O``: signál ltého svetla v tvaru chodca
3. ``C``: SZZ pre chodce

Druhı riadok LCD:

::

  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
  | D | A | Y |   | H | H | : | M | M | :  | S  | S  |    | U  | R  | G  |
  
1. ``Znaky 1-3``: aktuálny reim, môe by: DAY/NGT - DEN alebo NOC
2. ``Znaky 5-12``: aktuálny èas vo formáte HH:MM:SS (HH - hodiny, MM - minúty, SS - sekundy)
3. ``Znaky 14-16``: aktuálny typ reaktivity, môe by: NOR/URG - NORMAL alebo URGENT

Realizácie aplikácie pomocou prostriedku RT jadra FreeRTOS
===========================================================

Problém rozloíme na menšie èasti, urobíme tzv. ``RT úlohy``. 

1. ``terminal_task`` - realizované vo funkcii:

::
  
  static void terminal_task(void *param) 

V tejto RT úlohe riešime spracovanie terminálu a aktualizáciu údajov na LCD (funkcia ``display_info``)

2. ``keyboard_task`` - realizované vo funkcii: 

::
  
  static void keyboard_task(void *param) 

Táto úloha slúi k spracovaniu stlaèení tlaèidiel na FITkitu. Stlaèenie tlaèidla je signalizované nastavením globálnej premennej.

3. ``clock_task`` - realizované vo funkcii:

::
  
  static void clock_task(void *param) 

Reprezentuje hodinu kriovatky. Reim (DEN/NOC) meníme pod¾a èasu v tejto funkcii.

4. ``x_task`` - realizované vo funkcii: 

::
  
  static void x_task(void *param) 

Tu sa odohráva vlastné riadenie kriovatky.

Príkazovı riadok aplikácie
===========================

O prípadnú obsluhu príkazového riadku sa stará úloha ``terminal_task``, ktorá je volaná s periódou 300 ms. Táto úloha zároveò aktualizuje informácie na LCD. ``flag_timer`` indikuje, e informácie na LCD majú by aktualizované.

::
  
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

Po naviazaniu komunikácie medzi PC a FITkitem je moné poui nasledujúce príkazy:

::

  SET HH:MM:SS ... nastavenie èasu beiaceho vo FITkitu, HH - hodiny, MM - minúty, SS - sekundy
               ... príklad SET 13:55:01
               ... príklad zle: SET 12:5:2, spávne: SET 12:05:02

	  
Inicializácia a spustenie aplikácie
====================================

Viz ``main.c`` (kostra):

::

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


Zdrojové kódy
===============

Kompletné zdrojové kódy je moné nájst v súboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprevádzkovanie aplikácie
==========================
1. prelote aplikáciu

2. naprogramujte MCU a FPGA a spuste terminálovı program

3. nastavte èas v terminálu pomocou príkazu SET HH:MM:SS, napr.: SET 13:25:30

Aplikácia nevyaduje k svojej èinnosti nastavit okrem prepojiek umoòujúcich programovanie dalšie prepojky.

