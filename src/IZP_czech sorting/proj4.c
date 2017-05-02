 /*
 * Soubor:  proj4.c
 * Datum:   2010/12/16
 * Autor:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Projekt: Ceske razeni, projekt c. 4 pro predmet IZP
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
  EFILEOPEN,   /**< Nelze otevrit soubor. */
  EMEM,        /**< Nedostatek pameti. */
  ETEARLY,     /**< Chybaju data. */
  ETLATE,      /**< Vela dat. */
  EMISSCOL,    /**< Nenasiel som sloupec. */
  EUNKNOWN,    /**< Neznama chyba */
};

/** Stavove kody programu */
enum tstates
{
  CHELP,       /**< Napoveda */
  CBEFORE,     /**<  */
  CAFTER,      /**<  */
  CSORT,       /**<  */
  CEMPTY       /**<  */
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  /* EOK */
  "Vse v poradku.\n",
  /* ECLWRONG */
  "Chybne parametry prikazoveho radku.\n",
  /* EFILEOPEN */
  "Nelze otevrit soubor: ",
  /* EMEM */
  "Nedostatek pameti.\n",
  /* Chybaju data. */                                         
  "Chybaju data ve sloupci.\n",
  /* Vela dat. */
  "Vela dat ve sloupci.\n",
  /* EMISSCOL */
  "Nenasiel som sloupec.\n",
  /* EUNKNOWN */
  "Neznama chyba!\n"
};

const char *HELPMSG =
  "Program Ceske razeni\n"
  "Autor: David Molnar (c) 2010\n"
  "Pouziti: proj4 [parametry] vstupni_soubor vystupni_soubor\n"
  "         proj4 -h\n"
  "Popis parametru:\n"
  "  -h: zobrazeni napovedi\n"
  "  --before SLOUPEC RETEZEC: kriterium je pred (nepovinny)\n"
  "  --after SLOUPEC RETEZEC: kriterium je za (nepovinny)\n"
  "  --print SLOUPEC: vyber sloupce pro tisk\n"
  "  --sort: seradi hodnoty urcene pro tisk (nepovinny)\n";
  
const int BLOCK_INCREMENT = 32;

const char *abc =      "abcèdefgh#ijklmnopqrøs¹tuvwxyz¾0123456789";
const char *abcUpper = "ABCÈDEFGH#IJKLMNOPQRØS©TUVWXYZ®0123456789";

const char *dia =      "áïéìíóò»úùý";
const char *wodia =    "adeeinotuuy";
const char *diaUpper = "ÁÏÉÌÍÒÓ«ÚÙÝ";

/**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  char *inputFile;      /**<  */
  char *outputFile;     /**<  */
  char *printCol;       /**<  */
  char *filterCol;      /**<  */
  char *filterValue;    /**<  */
  int ecode;            /**< Chybovy kod programu, odpovida vvctu tecodes. */
  int state;            /**< Stavovy kod programu, odpovida vyctu tstates. */
  int filter;           /**< */
} tParams;

/**
 * Struktura jednoho prvku seznamu
 */
typedef struct tItem
{
  struct tItem *nextPtr;
  struct tItem *prevPtr;
  char *str;
} *tItemPtr;

/**
 * Struktura dvojsmerneho linearniho seznamu
 */
typedef struct
{
  tItemPtr actItem;
  tItemPtr firstItem;
  tItemPtr lastItem;
} tList; 

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
 * Vytiskne hlaseni o spatnem souboru.
 * @param fileName jmeno souboru 
 */
void printFileError(char *fileName)
{
  printECode(EFILEOPEN);
  
  fprintf(stderr, "%s.\n", fileName);
}  

/**
 * Zpracuje argumenty prikazoveho radku a vrati je ve strukture tParams.
 * Pokud je format argumentu chybny, ukonci program s chybovym kodem.
 * @param argc Pocet argumentu.
 * @param argv Pole textovych retezcu s argumenty.
 * @return Vraci analyzovane argumenty prikazoveho radku.
 */
tParams getParams(int argc, char *argv[])
{
  tParams result =
  { // inicializace struktury
    .ecode = EOK,
    .state = CEMPTY,
    .filter = CEMPTY,
    .inputFile = NULL,
    .outputFile = NULL,
    .printCol = NULL,
    .filterCol = NULL,
    .filterValue = NULL
  };

  if (argc == 2)
  { // jeden parameter
    if (strcmp("-h", argv[1]) == 0)
    {
      result.state = CHELP;
    }
    else 
    {
      result.ecode = ECLWRONG;
    }
  }
  else if (argc == 5)
  { // 4 parametry
    if (strcmp("--print", argv[1]) == 0)
    {
      result.printCol = argv[2];
      result.inputFile = argv[3];
      result.outputFile = argv[4];
    }
    else 
    {
      result.ecode = ECLWRONG;
    }
  }
  else if (argc == 6)
  { // 5 parametry
    if ((strcmp("--print", argv[1]) == 0) && (strcmp("--sort", argv[3]) == 0))
    {
      result.printCol = argv[2];
      result.inputFile = argv[4];
      result.outputFile = argv[5];
      result.state = CSORT;
    }
    else 
    {
      result.ecode = ECLWRONG;
    } 
  }
  else if (argc == 8)
  { // 7 parametry
    if (strcmp("--print", argv[4]) == 0)
    {
      result.filterCol = argv[2];
      result.filterValue = argv[3];
      result.printCol = argv[5];
      result.inputFile = argv[6];
      result.outputFile = argv[7];
   
      if (strcmp("--after", argv[1]) == 0)
      {
        result.filter = CAFTER;
      }
      else if (strcmp("--before", argv[1]) == 0)
      {
        result.filter = CBEFORE;
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
  }
  else if (argc == 9)
  { // 8 parametry
    if (strcmp("--print", argv[4]) == 0)
    {
      result.filterCol = argv[2];
      result.filterValue = argv[3];
      result.printCol = argv[5];
      result.inputFile = argv[7];
      result.outputFile = argv[8];
   
      if ((strcmp("--after", argv[1]) == 0) && (strcmp("--sort", argv[6]) == 0))
      {
        result.filter = CAFTER;
        result.state = CSORT;
      }
      else if ((strcmp("--before", argv[1]) == 0) && (strcmp("--sort", argv[6]) == 0))
      {
        result.filter = CBEFORE;
        result.state = CSORT;
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
  }
  else
  {
    result.ecode = ECLWRONG;
  }

  return result;
}

/**
 * Inicializace seznamu pred prvnim pouzitim.
 * @param list seznam 
 */ 
void initList(tList *list)
{
  list->actItem = NULL;
  list->firstItem = NULL;
  list->lastItem = NULL;
}

/**
 * Zruseni vsech prvku seznamu.
 * @param list seznam
 */ 
void disposeList(tList *list)
{
  tItemPtr workPtr = list->firstItem;
  
  while (workPtr != NULL)
  {
    tItemPtr temp = workPtr;
    workPtr = workPtr->nextPtr;
    
    free(temp->str);
    free(temp);
  }
  
  list->actItem = NULL;
  list->firstItem = NULL;
  list->lastItem = NULL;
}

/**
 * Vlozeni prvku na zacatek seznamu.
 * @param list seznam
 * @param str hodnota noveho prvku  
 */  
int insertFirst(tList *list, char *str)
{
  tItemPtr item = (tItemPtr)malloc(sizeof(struct tItem));
  
  if (item == NULL)
  {
    return EMEM;
  }
  
  item->str = str;
  item->prevPtr = NULL;
  item->nextPtr = list->firstItem;
  
  if (list->firstItem != NULL)
  { 
    list->firstItem->prevPtr = item;
  }
  else
  {
    list->lastItem = item;
  }
  
  list->firstItem = item;
  
  return EOK;  
}

/**
 * Vlozi novy prvek za aktivni prvek.
 * @param list seznam
 * @param str hodnota prvku  
 */ 
int postInsert(tList *list, char *str)
{
  if (list->actItem == NULL)
  {
    return EOK;
  }
  
  tItemPtr item = (tItemPtr)malloc(sizeof(struct tItem));
  
  if (item == NULL)
  {
    return EMEM;
  }
  
  item->str = str;
  item->prevPtr = list->actItem;
  item->nextPtr = list->actItem->nextPtr;
  list->actItem->nextPtr = item;
  
  if (list->actItem != list->lastItem)
  { 
    item->prevPtr->nextPtr = item;
  }
  else
  {
    list->lastItem = item;
  }
  
  return EOK; 
}

/**
 * Nastaveni aktivity na prvni prvek.
 * @param list seznam 
 */
void first(tList *list)
{
  list->actItem = list->firstItem;
} 

/**
 * Posune aktivitu na dalsi prvek seznamu.
 * @param list seznam 
 */                                                           
void succ(tList *list)
{
  if (list->actItem == NULL)
  {
    return;
  }
  
  list->actItem = list->actItem->nextPtr; 
}

/**
 * Zjistuje aktivitu sesnamu.
 * @param list seznam 
 */ 
bool active(tList *list)
{
  return (list->actItem == NULL) ? false : true; 
}

/**
 * Konvertuje c na male pismo a vysledok ulozi v c.
 * Vraci 1, ked c je jeden z charakter z pole dia.
 * @param c charakter  
 */ 
int tolower_locale(char *c)
{  
  int length = strlen(abc);
  int result = 0;
  
  for (int i = 0; i < length; i++)
  {
    if (*c == abcUpper[i])
    {
      *c = abc[i];
    }
  }
  
  length = strlen(dia);
  
  for (int i = 0; i < length; i++)
  {
    if (*c == dia[i] || *c == diaUpper[i])
    {
      *c = wodia[i];
      result = 1;      
    }
  }
  
  return result;
} 

/**
 * Porovna dva charaktre podla ceskej normy
 * Vraci 0, su stejny, negativni cislo, ked c1 je mensi nez c2 
 * alebo pozitivni cislo ked c1 je vetsi nez c2. 
 * @param c1 charakter1
 * @param c2 charakter2  
 */
int cmp_locale(char c1, char c2)
{
  int length = strlen(abc);
  
  int i1 = -1;
  int i2 = -1;
  
  for (int i = 0; i < length; i++)
  {
    if (abc[i] == c1)
    {
      i1 = i;
    }
    else if (abc[i] == c2)
    {
      i2 = i;
    }  
  } 
  
  return (i1 - i2);
} 

/**
 * Porovna dva pole charaktru podla ceskej normy
 * Vraci 0, su stejny, negativni cislo, ked c1 je mensi nez c2 
 * alebo pozitivni cislo ked c1 je vetsi nez c2. 
 * @param str1 pole charakterov1
 * @param str2 pole charakterov2  
 */ 
int strcmp_locale(char *str1, char *str2)
{ 
  if (str1 == NULL || str2 == NULL)
  {
    return 0;
  }
    
  int i = 0;  // indexovani str1
  int j = 0;  // indexovani str2
  
  char c1 = str1[i];
  char c2 = str2[j];
    
  int f1 = tolower_locale(&c1); // konvertujeme na male pismena
  int f2 = tolower_locale(&c2); 
  
  while (c1 == c2)
  {
    if (c1 == '\0')
    { // str1 == str2
      break;
    }
    
    i += 1;
    j += 1;
    
    c1 = str1[i];
    c2 = str2[j];
    
    if (tolower_locale(&c1) != 0)
    { // niekde ve str1 se nachazi charakter z dia
      f1 = 1;
    }
    
    if (tolower_locale(&c2) != 0)
    { // niekde ve str2 se nachazi charakter z dia
      f2 = 1;
    }
  }                                                
  
  // hledame ch
  char c11 = tolower((str1[i] != '\0') ? str1[i + 1] : 'a');  
  char c12 = tolower((i > 1) ? str1[i - 1] : 'a');
  
  if ((c1 == 'c' && c11 == 'h') || (c12 == 'c' && c1 == 'h'))
  { // nasli sme ch
    c1 = '#';  
  }
  
  char c21 = tolower((str2[j] != '\0') ? str2[j + 1] : 'a');
  char c22 = tolower((j > 1) ? str2[j - 1] : 'a');
  
  if ((c2 == 'c' && c21 == 'h') || (c22 == 'c' && c2 == 'h'))
  { // nasli sme ch
    c2 = '#';  
  }
  
  int m = cmp_locale(c1, c2);
  
  return (m != 0) ? m : (f1 - f2);
}

/**
 * Seraduje sezname vzestupne (bubble sort)
 * @param seznam 
 */ 
void sortList(tList *list)
{ 
  if (list->firstItem == NULL)
  {
    return;
  }

  tItemPtr workPtr;
  tItemPtr endPtr = list->firstItem;
  int done = 0;
  
  do
  {
    done = 1;
    
    workPtr = list->lastItem;
    
    while (workPtr != endPtr)
    {
      int result = strcmp_locale(workPtr->prevPtr->str, workPtr->str);
      if (result > 0)
      {
        char *temp = workPtr->prevPtr->str;
        workPtr->prevPtr->str = workPtr->str;
        workPtr->str = temp;
        
        done = 0;
      }
      
      workPtr = workPtr->prevPtr;
    }
    
    endPtr = endPtr->nextPtr;
  }
  while ((done == 0) && (endPtr != list->lastItem));
}

/**
 * Vytiskne seznam do souboru
 * @param list seznam
 * @param file soubor  
 */ 
void writeList(tList *list, FILE *file)
{
  tItemPtr workPtr = list->firstItem;
  
  while (workPtr != NULL)
  {
    fprintf(file, "%s\n", workPtr->str);
    
    workPtr = workPtr->nextPtr;
  }
}

/**
 * Nacita data ze souboru a ulozi do seznamu podla filtru, vraci kod chybu
 * @param list seznam
 * @param file soubor
 * @param filter filter
 * @param pc sloupec, ktery ma byt vytisteni
 * @param fc sloupec, podla ktereho mame filtrovat
 * @param fv pole charakterov, podla ktereho mama filtrovat      
 */ 
int readList(tList *list, FILE *file, int filter, char *pc, char *fc, char *fv)
{   
  int flag = 0;
  int i = 0;
  int maxCol = 0;
  int printCol = -1;
  int filterCol = -1;
  int add = 0;
  int size = 0;
  int hasCol = 0;
  int errCode = EOK;
  
  int c = fgetc(file);
  char* temp = NULL;
  
  while (c != EOF)
  {
    while (c == ' ' || c == '\t')
    {
      c = fgetc(file);
    }
    
    int j = 0;
    size = 32;
    char* word = (char*)malloc(size * sizeof(char));
    
    while (c != ' ' && c != '\t' && c != '\n' && c != EOF)
    {
      if (j == size)
      { // nestaci buffer, musime zvetsit
        size += BLOCK_INCREMENT;
        char *rebuf = (char*)realloc(word, size * sizeof(char));
        
        if (rebuf == NULL)
        {
          errCode = EMEM;
          if (word != NULL)
          {
            free(word);
          }
          
          break;
        } 
        
        word = rebuf;
      }
    
      word[j] = c;
      j += 1;
      
      c = fgetc(file);
    }
    
    word[j] = '\0';
    
    if (flag == 0)
    { // sme v prvnim radku
      if (strcmp_locale(word, pc)== 0)
      {
        printCol = i;
      }
       
      if (filter != CEMPTY && strcmp_locale(word, fc) == 0)
      {
        filterCol = i;
      }
    }
    else if (j != 0)
    { 
      if (filter != CEMPTY)
      { // mame definovani nejaky filter
        if (i == filterCol && 
            ((filter == CBEFORE && (strcmp_locale(word, fv) < 0)) ||
             (filter == CAFTER && (strcmp_locale(word, fv) > 0))))  
        {
          add = 1;
        }
      }
      else
      {
        add = 1;
      }
    
      if (i == printCol)
      { 
        temp = (char*)malloc(size * sizeof(char));
        
        if (temp == NULL)
        {
          free(word);
          errCode = EMEM;
          break;
        }
        
        strcpy(temp, word); // urobime kopiu
        
        hasCol = 1;
      }
    }
    
    free(word); // word uz nepotrebujeme
    
    if (j > 0)
    {
      i += 1;
    } 
    
    if (c == '\n')
    { // sme na konci radku   
      if (add != 0 && hasCol == 1)
      {
        if (!active(list))
        {
          insertFirst(list, temp);
          first(list);
        }
        else
        {
          postInsert(list, temp);
          succ(list);
        } 
        
        temp = NULL; 
      }
      else if (hasCol == 1)
      { 
         free(temp);
      }
    
      if (flag == 0)
      {
        maxCol = i;
      }
      else
      {
        if (i != 0 && i < maxCol)
        {
          errCode = ETEARLY;
          break;
        }
        else if (i != 0 && i > maxCol)
        {
          errCode = ETLATE;
          break;
        }
      }
      
      if (printCol == -1 || (filter != CEMPTY && filterCol == -1))
      {
        errCode = EMISSCOL;
        break;  
      }
      
      i = 0;
      flag = 1;
      add = 0;
      hasCol = 0;
    }
    
    c = fgetc(file); 
  }
  
  return errCode;  
}

/////////////////////////////////////////////////////////////////
/**
 * Hlavni program.
 */
int main(int argc, char *argv[])
{
  tParams params = getParams(argc, argv);
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
  
  FILE *iFile = fopen(params.inputFile, "r");
  
  if (iFile == NULL)
  {
    printFileError(params.inputFile);
    return EXIT_FAILURE;  
  }

  tList list;
  initList(&list);
  
  int errCode = readList(&list, iFile, params.filter, params.printCol, 
                         params.filterCol, params.filterValue);
  
  fclose(iFile);
  
  if (errCode != EOK)
  {
    disposeList(&list);
    printECode(errCode);
    return EXIT_FAILURE;
  }
  
  FILE *oFile = fopen(params.outputFile, "w");
  
  if (oFile == NULL)
  {
    disposeList(&list);
    printFileError(params.outputFile);
    return EXIT_FAILURE;  
  }
  
  if (params.state == CSORT)
  {
    sortList(&list);
  }
  
  writeList(&list, oFile);
  
  fclose(oFile);
  
  disposeList(&list);

  return EXIT_SUCCESS;
}

/* konec proj4.c */