.. article:: apps_demo_msp_krizovatka
    :author: D�vid Moln�r <xmolna02 AT stud.fit.vutbr.cz>
    :updated: 20111205

    Model riadenia prev�dzky na svetelnej kri�ovatke

============================================================
Model riadenia prev�dzky na svetelnej kri�ovatke (FreeRTOS)
============================================================

.. contents:: Obsah

Popis aplik�cie
==================

S vyu�it�m prostriedka RT jadra FreeRTOS je pre MSP430 na FITkitu v jazyku C implementovan� model svetelnej kri�ovatke (``Kri�ovatka`` - CZ, ``Crossroad`` - EN)

tvoren� 

1. ``tla�idlami`` (s� vyu�it� nasleduj�ce tla�idl� z maticovej kl�vesnice FITkitu: ``A``, ``B``, ``C``, ``D``, ``*``, ``0``, ``#``, ``1`` pop�san� ni��ie), 

2. ``riadiacou �as�ou`` (implementovan� v MSP430 pomocou prostriedka FreeRTOS) reaguj�ce na zmenu �asu a na stla�enie tla�idla a

3. ``riadenou �as�ou`` reprezentovanou pre jednoduchos� premenn�mi v pam�ti FITkitu.

Aplik�ciu je mo�no slovne ``specifikova�`` nasledovne:

Kri�ovatka v tvaru "X" s obojsmernou prev�dzkou motorov�ch vozidel (vz�jomne kolm� silnice v smeroch svetov�ch str�n S, J, V, Z).
V bl�zkosti kr�en� ciest vedie cez ka�d� z ciest prechod pre chodce (zebra). 
U ka�dej zebry je semafor (sveteln� signaliza�n� zariadenie, SSZ) pre vozidla vjazdiace do kri�ovatky (trojfarebn� s�stava s pln�mi sign�lmi - �erven�, �lt�, zelen� - ur�en�mi k signaliz�cii povelu POZOR (�erven� + �lt�), VO�NO (zelen�), STOJ (�erven�)). 

�alej je na ka�dej strane zebry semafor pre chodce (dvojfarebn� s�stava sveteln�ch sign�lov �erven�/STOJ, zelen�/VO�NO) vybaven� tla�idlom pre (signaliz�ciu po�iadavku) chodcov (na prechod zebrou) a schopn� generova� doprovodn� akustick� sign�ly STOJ/VO�NO sl��iace okrem in�ch k orient�cii nevidom�ch.
Na ka�dom semaforu pre chodce je tie� umiesten� sign�l �lt�ho svetla v tvaru chodca upozor�uj�ci vodi�ov na to, �e sa bl�i k prechodu, prejazdom �oho by kri�oval voln� smer chodcov.
 
S� podporovan� nasleduj�ce re�imy �innosti: denn� prev�dzka (DE�, aktivovan� v dobe od 04:00 do 23:00 vr�tane), no�n� prev�dzka (NOC: preru�ovan� svit �lt�ch pln�ch sign�lov v dobe od 23:01 do 03:59 vr�tane) a vypnutie signaliz�cie (OFF: vypnutie v�etk�ch sign�lov na z�klade vonkaj�ieho podnetu na zvl�tnom riadiacom vstupu - tla�idlo ``0``). 

Sign�lny cyklus (pl�n) re�imu je rozlo�en� nasledovne:
 
``Re�im DE�``: f�za 0 [1s] - STOJ pre vozidla S, J, V, Z a chodce S, J, V, Z, vypnutie sign�lu �lt�ho svetla v tvaru chodca, f�za 1 [2s] - POZOR pre vozidla S, J, f�za 2 [10s] - VO�NO pre vozidla S, J, (f�za [8s] pod�a signaliz�cie od chodca: VO�NO pre chodca S, J, zapnutie sign�lu �lt�ho svetla v tvaru chodca), (f�za [0s] STOP pre chodca S, J, vypnutie sign�lu �lt�ho svetla v tvaru chodca), f�za 3 [3s] - POZOR pre vozidla S, J, f�za 4 [1s] - STOP pre vozidla S, J. F�zy 0 a� 4 opakuj�, striedaj� sa smery S, J a V, Z.

Sign�l VO�NO pre chodca je do sign�lneho cyklu kri�ovatky zaraden� a� na z�klade predchodzieho stla�enia pr�slu�n�ho tla�idla chodca.

Prechody medzi re�imami DEN, NOC a OFF s� ur�en� nastaven�m typu reaktivity (RTYPE), ktor� m��e nab�va� hodnoty NORMAL alebo URGENT. Ak je RTYPE=NORMAL, s� prechody medzi re�imami a zmeny v sign�lnom pl�nu mo�n� len na konci cyklu t�chto re�imov. Ak je RTYPE=URGENT, potom s� spomenut� prechody a zmeny mo�n� i v ostatn�ch f�ziach re�imu.

.. figure:: krizovatka.png
   :align: center

   Sch�ma kri�ovatky
   
   Pozn.: �erven�: SZZ pre vozidla, �lt�: sign�l �lt�ho svetla v tvaru chodca, zelen�: SZZ pre chodce
   
Popis tla�idiel a symbolov na LCD
==================================

Tla�idl� z maticovej kl�vesnice FITkitu:

1. ``A`` - signaliz�cia od chodca SEVER �av�
2. ``B`` - signaliz�cia od chodca JUH �av�
3. ``C`` - signaliz�cia od chodca V�CHOD �av�
4. ``D`` - signaliz�cia od chodca Z�PAD �av�
5. ``*`` - presko�enie aktu�lnej f�zy (len v re�ime DEN a pri rtyspe=URGENT)
6. ``#`` - prep�nanie re�imu: DEN alebo NOC
7. ``1`` - prep�nanie m�du zobrazenia �asu: aktu�lny �as alebo zost�vaj�ci �as v aktu�lnej f�ze
8. ``0`` - prepnutie do re�imu OFF, n�slednym stla�en�m prepne nasp�

Symboly na LCD display (5x8 pixel):

  Symbol trojfarebnej SZZ pre vozidla: STOP, POZOR a VO�NO
  
::
  
  XXXXX   XXXXX   .....
  XX.XX   XX.XX   .....
  XXXXX   XXXXX   .....
  .....   XX.XX   .....
  .....   XXXXX   .....
  .....   .....   XXXXX
  .....   .....   XX.XX
  .....   .....   XXXXX
  
Symbol dvojfarebnej SZZ pre chodcov: STOP a VO�NO

::

  XXXXX   .....
  XX.XX   .....
  XX.XX   .....
  XXXXX   .....
  .....   XXXXX
  .....   XX.XX
  .....   XX.XX
  .....   XXXXX
  
Symbol sign�lu �lt�ho svetla v tvaru chodca: POZOR

::

  .....
  .....
  XXXXX
  XX.XX
  XX.XX
  XXXXX
  .....
  .....
  
Ke� je svetlo vypnut�, na LCD sa zobraz� pr�zdne miesto.
  
Prv� riadok LCD:

::

  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
  | S | O | C |   | S | O | C |       | S  | O  | C  |    | S  | O  | C  |
  
1. ``Znaky 1-3``: severn� SZZ
2. ``Znaky 5-6``: ju�n� SZZ
3. ``Znaky 10-12``: v�chodn� SZZ
4. ``Znaky 14-16``: z�padn� SZZ
  
1. ``S``: SZZ pre vozidla
2. ``O``: sign�l �lt�ho svetla v tvaru chodca
3. ``C``: SZZ pre chodce

Druh� riadok LCD:

::

  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
  | D | A | Y |   | H | H | : | M | M | :  | S  | S  |    | U  | R  | G  |
  
1. ``Znaky 1-3``: aktu�lny re�im, m��e by�: DAY/NGT - DEN alebo NOC
2. ``Znaky 5-12``: aktu�lny �as vo form�te HH:MM:SS (HH - hodiny, MM - min�ty, SS - sekundy)
3. ``Znaky 14-16``: aktu�lny typ reaktivity, m��e by�: NOR/URG - NORMAL alebo URGENT

Realiz�cie aplik�cie pomocou prostriedku RT jadra FreeRTOS
===========================================================

Probl�m rozlo��me na men�ie �asti, urob�me tzv. ``RT �lohy``. 

1. ``terminal_task`` - realizovan� vo funkcii:

::
  
  static void terminal_task(void *param) 

V tejto RT �lohe rie�ime spracovanie termin�lu a aktualiz�ciu �dajov na LCD (funkcia ``display_info``)

2. ``keyboard_task`` - realizovan� vo funkcii: 

::
  
  static void keyboard_task(void *param) 

T�to �loha sl��i k spracovaniu stla�en� tla�idiel na FITkitu. Stla�enie tla�idla je signalizovan� nastaven�m glob�lnej premennej.

3. ``clock_task`` - realizovan� vo funkcii:

::
  
  static void clock_task(void *param) 

Reprezentuje hodinu kri�ovatky. Re�im (DEN/NOC) men�me pod�a �asu v tejto funkcii.

4. ``x_task`` - realizovan� vo funkcii: 

::
  
  static void x_task(void *param) 

Tu sa odohr�va vlastn� riadenie kri�ovatky.

Pr�kazov� riadok aplik�cie
===========================

O pr�padn� obsluhu pr�kazov�ho riadku sa star� �loha ``terminal_task``, ktor� je volan� s peri�dou 300 ms. T�to �loha z�rove� aktualizuje inform�cie na LCD. ``flag_timer`` indikuje, �e inform�cie na LCD maj� by� aktualizovan�.

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

Po naviazaniu komunik�cie medzi PC a FITkitem je mo�n� pou�i� nasleduj�ce pr�kazy:

::

  SET HH:MM:SS ... nastavenie �asu be�iaceho vo FITkitu, HH - hodiny, MM - min�ty, SS - sekundy
               ... pr�klad SET 13:55:01
               ... pr�klad zle: SET 12:5:2, sp�vne: SET 12:05:02

	  
Inicializ�cia a spustenie aplik�cie
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


Zdrojov� k�dy
===============

Kompletn� zdrojov� k�dy je mo�n� n�jst v s�boru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprev�dzkovanie aplik�cie
==========================
1. prelo�te aplik�ciu

2. naprogramujte MCU a FPGA a spus�te termin�lov� program

3. nastavte �as v termin�lu pomocou pr�kazu SET HH:MM:SS, napr.: SET 13:25:30

Aplik�cia nevy�aduje k svojej �innosti nastavit okrem prepojiek umo��uj�cich programovanie dal�ie prepojky.

