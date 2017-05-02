/*
 * Soubor:  proj2.c
 * Datum:   2010/11/21
 * Autor:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Projekt: Iteracni vypocty, projekt c. 2 pro predmet IZP
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

// matematicke funkcie, konstanta NAN
#include <math.h>

/** Konstanty */
const double IZP_E = 2.7182818284590452354;        // e
const double IZP_PI = 3.14159265358979323846;      // pi
const double IZP_2PI = 6.28318530717958647692;     // 2*pi
const double IZP_PI_2 = 1.57079632679489661923;    // pi/2
const double IZP_PI_4 = 0.78539816339744830962;    // pi/4

/** Kody chyb programu */
enum tecodes
{
  EOK = 0,     /**< Bez chyby */
  ECLWRONG,    /**< Chybny prikazovy radek. */
  ESIG,        /**< Chybny sigdig. */
  EBASE,       /**< Chybny zaklad logaritmu. */
  EEND,        /**< Neocakavany konec vstupu. */
  ECONV,       /**< Konverzacna chyba! */
  EUNKNOWN,    /**< Neznama chyba */
};

/** Stavove kody programu */
enum tstates
{
  CHELP,       /**< Napoveda */
  CTANH,       /**< Hyperbolicky tangens */
  CLOGAX,      /**< Obecny logaritmus */
  CWAM,        /**< Vazeny aritmeticky prumer */
  CWQM,        /**< Vazeny kvadraticky prumer */
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  /* EOK */
  "Vse v poradku.\n",
  /* ECLWRONG */
  "Chybne parametry prikazoveho radku!\n",
  /* ESIG */
  "Chybny sigdig!\n",
  /* EBASE */
  "Chybny zaklad logaritmu!\n",
  /* EEND */
  "Neocakavany konec vstupu!\n",
  /* ECONV */
  "Chyba pri konverzii!\n",
  /* EUNKNOWN */
  "Neznama chyba!\n",
};

const char *HELPMSG =
  "Program Iteracni vypocty\n"
  "Autor: David Molnar (c) 2010\n"
  "Pouziti: proj2 -h\n"
  "         proj2 --tanh sigdig\n"
  "         proj2 --logax sigdig a\n"
  "         proj2 --wam\n"
  "         proj2 --wqm\n"
  "Popis parametru:\n"
  "  -h: zobrazeni napovedi\n"
  "  --tanh: hyperbolicky tangens\n"
  "  --logax: obecny logaritmus s zakladom a\n"
  "  --wam: vazeny aritmeticky prumer\n"
  "   -wqm: vazeny kvadraticky prumer\n"
  "   sigdig: presnost zadana jako pocet platnych cifer (significant digits)\n";

/**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  unsigned int sigdig;  /**< Pocet platnych cifer. */
  double base;          /**< Zaklad logaritmu. */
  int ecode;            /**< Chybovy kod programu, odpovida vvctu tecodes. */
  int state;            /**< Stavovy kod programu, odpovida vyctu tstates. */
} TParams;

/**
 * Struktura obsahujici hodnoty pre statisticke funke wam a wqm
 */
typedef struct avParams
{
  double num;         /**< Prvek posloupnosti. */
  double h;           /**< Vaha prvku posloupnosti. */
  double sum1;
  double sum2;
} TAvParams;

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
 * Konvertuje posloupnost znaku na unsigned int
 * V pripade, ze vse probehlo v poradku, nastavi
 * errcode na EOK.
 *@param str posloupnost znaku, z ktere ziskame cislo
 *@param errcode
 */
unsigned int char2uint(char *str, int *errcode)
{
  errno = 0; // errno na nulu
  *errcode = EOK; // errcode na OK
  char *endptr;
  unsigned long val = strtoul(str, &endptr, 10); //konverzia
  unsigned int result = 0;

  if ((errno == ERANGE && val == ULONG_MAX)
   || val > UINT_MAX
   || strchr(str, '-') != NULL 
   || str == endptr)
  { // nastala chyba
    *errcode = ECONV; 
  }
  else
  { 
    result = (unsigned int)val; 
  }

  return result;
}

/**
 * Konvertuje posloupnost znaku na double
 * V pripade, ze vse probehlo v poradku, nastavi
 * errcode na EOK.
 *@param str posloupnost znaku, z ktere ziskame cislo
 *@param errcode
 */
double char2double(char *str, int *errcode)
{
  errno = 0; // errno na nulu
  *errcode = EOK; // errcode na OK
  char *endptr;
  double result = strtod(str, &endptr); //konverzia
  
  if (errno == ERANGE || errno == EINVAL
    || (str == endptr && *endptr == '\0'))
  { // nastala chyba
    *errcode = ECONV; 
  }

  return result;
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
    .ecode = EOK,
    .state = 0,
    .sigdig = 0,
    .base = NAN
  };

  if (argc == 2)
  { // jeden parameter
    if (strcmp("-h", argv[1]) == 0)
    {
      result.state = CHELP;
    }
    else if (strcmp("--wam", argv[1]) == 0)
    {
      result.state = CWAM;
    }
    else if (strcmp("--wqm", argv[1]) == 0)
    {
      result.state = CWQM;
    }
    else 
    {
      result.ecode = ECLWRONG;
    }
  }
  else if (argc == 3)
  { // dva parametry
    if (strcmp("--tanh", argv[1]) == 0)
    {
      result.state = CTANH;
		
      int errcode = EOK;
      int sigdig = char2uint(argv[2], &errcode);
		
      if (errcode != EOK || sigdig == 0)
      { // nastala chyba
        result.ecode = ESIG;
      }
      else
      {
        result.sigdig = sigdig;
      }
    }
    else 
    {
      result.ecode = ECLWRONG;
    }
  }
  else if (argc == 4)
  { // try parametry
    if (strcmp("--logax", argv[1]) == 0)
    {
      result.state = CLOGAX;
		
      int errcode = EOK;
      int sigdig = char2uint(argv[2], &errcode);
		
      if (errcode != EOK || sigdig == 0)
      { // nastala chyba
        result.ecode = ESIG;
      }
      else
      {
        result.sigdig = sigdig;
      }

      if (errcode == EOK)
      { 
        errcode = EOK;
	double base = char2double(argv[3], &errcode);

	if (errcode != EOK || isnan(base)|| isinf(base)
	   || base <= 0.0 || base == 1.0)
	{ // nastala chyba
	  result.ecode = EBASE;
	}
	else
	{
	  result.base = base;
	}
      }
    }
    else
    {
      result.ecode = ECLWRONG;
    }
  }
  else
  {
    result.ecode = ECLWRONG;
  }

  return result;
}

/**
 * Z pocet platnych cifer ziskame epsilon
 * Vraci 0.1^sigdig
 *@param sigdig pocet platnych cifer
 */
double sd2eps(unsigned int sigdig)
{
  double eps = 1;
  
  while (sigdig > 0)
  {
    eps *= 0.1;
    sigdig -= 1;
  }
  
  return eps;
}

/**
 * Hyperbolicky sinus
 *@param num cislo, z ktere ziskame sinh
 *@param sigdig pocet platnych cifer
 */
double sinush(double num, unsigned int sigdig)
{
  if (sigdig < 1)
  {
    return NAN;
  }

  if (isnan(num)|| isinf(num))
  {
    return num;
  }
  
  double x = num;
  double x2 = num * num;
  double result = x;
  double presult = 0;
  
  double k = 0;
  double fakt = 1;
  
  double eps = sd2eps(sigdig);
  
  while (fabs(result - presult) > eps)
  {
    x *= x2; 
    k += 2;
    fakt *= k * (k + 1);
	
    presult = result;
    result = presult + x / fakt;
  }
  
  return result;
}

/**
 * Hyperbolicky cosinus
 *@param num cislo, z ktere ziskame cosh
 *@param sigdig pocet platnych cifer
 */
double cosinush(double num, unsigned int sigdig)
{
  if (sigdig < 1)
  {
    return NAN;
  }

  if (isnan(num)|| isinf(num))
  {
    return num;
  }
  
  double x = 1;
  double x2 = num * num;
  double result = x;
  double presult = 0;
  
  double k = 1;
  double fakt = 1;
  
  double eps = sd2eps(sigdig);
  
  while (fabs(result - presult) > eps)
  {
    x *= x2;
    fakt *= k * (k + 1);
    k += 2;
	
    presult = result;
    result = presult + x / fakt;
  }
  
  return result;
}

/**
 * Hyperbolicky tangens
 *@param num cislo, z ktere ziskame tanh
 *@param sigdig pocet platnych cifer
 */
double tangensh(double num, unsigned int sigdig)
{
  if (sigdig < 1)
  {
    return NAN;
  }

  if (isnan(num)|| isinf(num))
  {
    return num;
  }

  double s = sinush(num, sigdig);
  double c = cosinush(num, sigdig);
  
  return s / c;
}

/**
 * Naturalny logaritmus
 *@param num cislo, z ktere ziskame ln
 *@param sigdig pocet platnych cifer
 */
double logex(double num, unsigned int sigdig)
{
  if (sigdig < 1 || num < 0)
  {
    return NAN;
  }
  
  if (num == 0)
  {
    return INFINITY;
  }
  
  if (isnan(num) || isinf(num))
  {
    return num;
  }

  double f = (num - 1) / (num + 1);
  double f2 = f * f;
  double sum = f;
  double presult = 0;
  double result = 2 * f;
  
  double k = 1;
  double eps = sd2eps(sigdig + 1);

  while (fabs(result - presult) > (presult * eps))
  {
    k += 2;
    f *= f2;
    sum += f / k;
	 
    presult = result;
    result = 2 * sum;
  }

  return result;
}

/**
 * Obecny logaritmus sinus
 *@param num cislo, z ktere ziskame log
 *@param sigdig pocet platnych cifer
 */
double logax(double num, double base, unsigned int sigdig)
{
  if (num <= 0 || base < 0 || base == 1) 
  {
    return NAN;
  }
  
  if (base == 0)
  {
    return INFINITY;
  }
  
  if (isnan(num) || isinf(num))
  {
    return num;
  }
  
  double log1 = logex(num, sigdig);
  double log2 = logex(base, sigdig);

  return log1 / log2;
}

/**
 * Vazeny aritmeticky prumer
 *@param avParams
 */
double wam(TAvParams *avParams)
{
  if (avParams->h < 0)
  { // vaha nemoze byt zaporna
    return NAN;
  }
  
  avParams->sum1 += avParams->num * avParams->h;
  avParams->sum2 += avParams->h;
  
  return avParams->sum1 / avParams->sum2;
}

/**
 * Vazeny kvadraticky prumer
 *@param avParams
 */
double wqm(TAvParams *avParams)
{
  if (avParams->h < 0)
  { // vaha nemoze byt zaporna
    return NAN;
  }
  
  avParams->sum1 += avParams->num * avParams->num * avParams->h;
  avParams->sum2 += avParams->h;
  
  return sqrt(avParams->sum1 / avParams->sum2);
}

/**
 * Vypise cislo v danej formate na stdout
 *@param num cislo, ktere vypise
 */
void printNum(double num)
{
  printf("%.10e\n", num);
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
  { // tisk napovedi
    printf("%s", HELPMSG);
    return EXIT_SUCCESS;
  }
  
  double num;
  int status;
  bool next = false;
  
  TAvParams avParams = 
  { // inicializace
    .num = 0,
    .h = 0,
    .sum1 = 0,
    .sum2 = 0
  };

  while((status = scanf("%lf", &num)) != EOF)
  {
    if (status != 1)
    { // nejaka chyba pri nacitani cisla, vypiseme NAN
      printNum(NAN);
      scanf("%*s");
      continue;
    }

    double result;
	 
    if (params.state == CTANH)
    { // hyperbolicky tangens
      result = tangensh(num, params.sigdig);
      printNum(result);
    }
    else if (params.state == CLOGAX)
    { // obecny logaritmus
      result = logax(num, params.base, params.sigdig);
      printNum(result);
    }
    else if (params.state == CWAM || params.state == CWQM)
    { //prumeri
      if (next)
      {
        avParams.h = num;
        result = (params.state == CWAM) ? wam(&avParams) : wqm(&avParams);
      }
      else
      {
        avParams.num = num;
      }
		
      if (next)
      {
        printNum(result);
      }
	 
      next = !next;
    }
  }
  
  if (next)
  {
    printECode(EEND);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* konec proj2.c */