/*
 * Soubor:  proj1.c
 * Datum:   2010/11/07
 * Autor:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Projekt: Jednoducha komprese textu, projekt c. 1 pro predmet IZP
 */

// prace se vstupem/vystupem
#include <stdio.h>

// obecne funkce jazyka C
#include <stdlib.h>

// kvuli funkci strcmp
#include <string.h>

// testovani znaku - isalpha, isdigit, atd.
#include <ctype.h>

// pro testovani programu
// #include <assert.h>

// limity ciselnych typu
#include <limits.h>

// typ bool, konstanty true, false
#include <stdbool.h>

// promenna errno
#include <errno.h>


/** Kody chyb programu */
enum tecodes
{
  EOK = 0,     /**< Bez chyby */
  ECLWRONG,    /**< Chybny prikazovy radek. */
  ECLNPAR,     /**< Chybny parameter N */
  EINWRONG,    /**< Nelegalni vstup */
  ENERROR,     /**< N nema legalnu hodnotu */
  EMEM,        /**< Nedostatek pameti */
  EEOFERROR,   /**< Neocakavany konec vstupu */
  EUNKNOWN,    /**< Neznama chyba */
};

/** Stavove kody programu */
enum tstates
{
  CHELP,       /**< Napoveda */
  CCOMP,       /**< Komprimovat. */
  CDECOMP,     /**< Dekomprimovat. */
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  /* EOK */
  "Vse v poradku.\n",
  /* ECLWRONG */
  "Chybne parametry prikazoveho radku!\n",
  /* ECLNPAR */
  "Chybny parameter N!\n",
  /* EINWRONG */
  "Nelegalni (neocakavany) znak na vstupu!\n",
  /* ENERROR */
  "N musi mat hodnotu vetsi nez 0!\n",
  /* EMEM */
  "Nedostatek pameti!\n",
  /* EEOFERROR */
  "Neocakavany konec vstupu!\n",
  /* EUNKNOWN */
  "Neznama chyba!\n",
};

const char *HELPMSG =
  "Program Jednoducha komprese\n"
  "Autor: David Molnar (c) 2010\n"
  "Pouziti: proj1 -h\n"
  "         proj1 -c N\n"
  "         proj1 -d N\n"
  "Popis parametru:\n"
  "  -h: zobrazeni napovedi\n"
  "  -c N: komprimace vstupni text po blocich delky N\n"
  "  -d N: dekomprimace, predpoklada, ze vstupni text byl\n"
  "        komprimovan se stejnou hodnotou N\n";

/**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  unsigned int N;   /**< Hodnota N z prikazove radky. */
  int ecode;        /**< Chybovy kod programu, odpovida vvctu tecodes. */
  int state;        /**< Stavovy kod programu, odpovida vyctu tstates. */
} TParams;


/**
 * Vytiskne hlaseni odpovidajici chybovemu kodu.
 * @param ecode kod chyby programu
 */
void printECode(int ecode)
{
  if (ecode < EOK || ecode > EUNKNOWN)
  { 
    ecode = EUNKNOWN; 
  }

  fprintf(stderr, "%s", ECODEMSG[ecode]);
}

/**
 * Zpracuje argumenty prikazoveho radku a vrati je ve strukture TParams.
 * Pokud je format argumentu chybny, ukonci program s chybovym kodem.
 * @param argc Pocet argumentu.
 * @param argv Pole textovych retezcu s argumenty.
 * @return Vraci analyzovane argumenty prikazoveho radku.
 */
TParams getParams(int argc, char *argv[])
{
  TParams result =
  { // inicializace struktury
    .N = 0,
    .ecode = EOK,
    .state = CCOMP,
  };

  if (argc == 2 && strcmp("-h", argv[1]) == 0)
  { // tisk napovedy
    result.state = CHELP;
  }
  else if (argc == 3)
  { // dva parametry
    if (strcmp("-c", argv[1]) == 0)
    { // komprimace
      result.state = CCOMP;
    }
    else if (strcmp("-d", argv[1]) == 0)
    { //dekomprimace
      result.state = CDECOMP;
    }
    else 
    { //nieco ine --> chyba
      result.ecode = ECLWRONG;
    }
 
    //konvertujeme N na unsigned int
    errno = 0; //aby sme mohli vediet nas stav po volani funkce strtoul
    char *str = argv[2]; //N
    char *endptr;
    unsigned long val = strtoul(str, &endptr, 10); //konverzia
	 
    if ((errno == ERANGE && val == ULONG_MAX) //vetsi cislo nez ULONG_MAX
      || val > UINT_MAX //my chceme unsigned int, tede nesmi byt vetsi nez UINT_MAX
      || strchr(str, '-') != NULL //unsigned, teda nemoze byt zaporni --> neobsahuje '-'
      || str == endptr) //ma nejake cisla, c99 nepodporuje chybu EINVAL
    { //hlasime chybu
      result.ecode = ECLNPAR; 
    }
    else
    { //vsetko v poradku
      result.N = (unsigned int)val; 
     }
  }
  else
  { // prilis mnoho parametru
    result.ecode = ECLWRONG;
  }

  return result;
}

/**
 * Vrati index nasledujiciho prvku v kruhovom poli
 * @param index aktualni index prvku
 * @param N pocet prvku
 */
unsigned long nextIndex(unsigned long index, unsigned long N)
{
  return (index + 1) % N;
}

/**
 * Komprimace textu z stdin, vystup pise na stdout
 * @param N delka blocich
 */
int compress(unsigned int N)
{
  if (N < 1)
  { //N nemoze byt mensi nez 1
    return ENERROR;
  }
  
  unsigned long bN = N + 1; //pocet prvku v kruhovom poli
  int* buffer = (int*)malloc(bN * sizeof(int)); //buffer
  if (buffer == NULL)
  { //neni dostatek pameti
    return EMEM;
  }
  
  int c; //do toho ulozime charaktery z stdin
  unsigned long fi = 0; //first index
  unsigned long li = 0; //last index
  unsigned int eqCount = 0; 
  unsigned int repeatCount = 0;
  
  while ((c = getchar()) != EOF)
  {
    if (isdigit(c) != 0)
    { //nacetli sme cislo, koncime
      free(buffer);
      return EINWRONG;
    }
	 
    if (fi != nextIndex(li, bN))
    { //buffer nie plny
      buffer[li] = c; //naplnime
      li = nextIndex(li, bN); //nastavime last index na nasled. prvek
    }
    else
    { //buffer plny
      if (buffer[(fi + eqCount) % bN] != c)
      { //prisiel znak, ktory nezhoduje s aktualnim v poli
        if (repeatCount > 0)
        { //pocitali sme repeatCount vyskytnuti
          putchar(repeatCount + '0' + 1);
		    
          int index = fi;
          for (unsigned int i = 0; i < N; i++)
          { //vypysem co mame v buffer
            putchar(buffer[index]);
	          index = nextIndex(index, bN);
          }
			 
          buffer[(fi + eqCount) % bN] = c; //pridame novy prvek
          li = (fi + eqCount + 1) % bN; //na zaciatok
			 
          repeatCount = 0;
          eqCount = 0;
        }
        else
	      { //nemali sme ani raz blok
	        putchar(buffer[fi]); //vypyseme prvy prvek
	        buffer[li] = c; //pridame nove
			 
	        fi = nextIndex(fi, bN); //nastavime indexi
	        li = nextIndex(li, bN);
	      }
      }
      else
      {
        eqCount += 1; //mame zhodu
	  
        if (eqCount == N)
        { //nasli sme cely blok
          repeatCount += 1;
          eqCount = 0; //zacneme znova pocitat
        }
      }
    }
  }
  
  //vypyseme co mame v buffer
  if (repeatCount > 0)
  {
    putchar(repeatCount + '0' + 1);
  }		 
  
  while (fi != li)
  {
    putchar(buffer[fi]);
    fi = nextIndex(fi, bN);
  }
  
  free(buffer);
   
  return EOK;
}

/**
 * Decomprimace vstupu ze stdin, vystup na stdout
 * @param N delka blocich
 */
int decompress(unsigned int N)
{
  if (N < 1)
  { //N nemuze byt mensi nez 1
    return ENERROR;
  }
  
  int* buffer = (int*)malloc(N * sizeof(int));
  if (buffer == NULL)
  { //nedostatek pameti
    return EMEM;
  }
  
  int c;
  while ((c = getchar()) != EOF)
  {
    if (isdigit(c) != 0 && c != '0' && c != '1')
    { //nacteni znak nemuze byt cislo 0 alebo 1
      int count = (char)c - '0'; //konverzia znaku na cislo :)
		
      for (unsigned int i = 0; i < N; i++)
      { //nacitame N znaku ze stdin
        c = getchar();
        if (c != EOF)
        {
          buffer[i] = c; //ulozime do buffer
	      }
        else
        { //ked uz nemame dostatek znaku, hlasime chybu
          free(buffer);
          return EEOFERROR;
        }
      }
		
      for (int j = 0; j < count; j++)
      { //vypiseme buffer count krat
        for (unsigned int k = 0; k < N; k++)
        { 
          putchar(buffer[k]);
        }
      }
    }
    else if (c == '0' || c == '1')
    { //precetli sme 0 nebo 1, co nema smysl
      free(buffer);
      return EINWRONG;
    }
    else
    { //ostatne char.
      putchar(c);
    }
  }
  
  free(buffer);
  
  return EOK;
}

/////////////////////////////////////////////////////////////////
/**
 * Hlavni program.
 */
int main(int argc, char *argv[])
{
  TParams params = getParams(argc, argv);
  if (params.ecode != EOK)
  { // nico nestandardniho
    printECode(params.ecode);
    return EXIT_FAILURE;
  }

  if (params.state == CHELP)
  {
    printf("%s", HELPMSG);
    return EXIT_SUCCESS;
  }

  int errcode = EOK;
  
  if (params.state == CCOMP)
  { //komprese
    errcode = compress(params.N);
  }
  else if (params.state == CDECOMP)
  { //dekomprese
    errcode = decompress(params.N);
  }
  
  if (errcode != EOK)
  { //nastala chyba pri kompr. nebo dekomp.
    printECode(errcode);
    return EXIT_FAILURE;
  }

  //ziadna chyba, vsetko v poradku
  return EXIT_SUCCESS;
}

/* konec proj1.c */
