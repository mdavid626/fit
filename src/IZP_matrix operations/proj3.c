/*
 * Soubor:  proj3.c
 * Datum:   2010/11/21
 * Autor:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Projekt: Maticove operace, projekt c. 3 pro predmet IZP
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
  EFILEOP,     /**< Nelze otvorit soubor. */
  EOUTMEM,     /**< Nedostatok pameti. */
  ETYPERR,     /**< Nevhodny typ. */
  EDIMEN,      /**< Spatne zadane rozmery. */
  EILLCH,      /**< Matica obsahuje nelegalne znaky. */
  ETOOCH,      /**< Matica obsahuje vela znakov. */
  ENOTENCH,    /**< Matica obsahuje malo znakov. */
  EUNKNOWN     /**< Neznama chyba */
};

/** Stavove kody programu */
enum tstates
{
  CHELP,       /**< Napoveda */
  CTEST,       /**< Testovaci vystup */
  CVADD,       /**< Vektorovy soucet */
  CVSCAL,      /**< Skalarni soucin vektoru */
  CMMULT,      /**< Soucin dvou matic */
  CMEXPR,      /**< Maticovy vyraz A*B*A */
  CEIGHT,      /**< Osmismerka */
  CBUBBLES,    /**< Bubliny */
};

/** */
enum ttypes
{
  TVECTOR = 1,     /**< Vektor */
  TMATRIX = 2,     /**< Matice */
  TVMATRIX = 3,    /**< Vektor matic */
  TTERROR = 4      /**< Chybne cislo*/
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  /* EOK */
  "Vse v poradku.\n",
  /* ECLWRONG */
  "Chybne parametry prikazoveho radku!\n",
  /* EFILEOP */
  "Nelze otvorit soubor.\n",
  /* EOUTMEM */
  "Nedostatok pameti.\n",
  /* ETYPERR */
  "Nevhodny typ.\n",
  /* EDIMEN */
  "Spatne zadane rozmery.\n",
  /* EILLCH */
  "Matica obsahuje nelegalne znaky.\n",
  /* ETOOCH */
  "Matica obsahuje vela znakov.\n",
  /* ENOTENCH */
  "Matica obsahuje malo znakov.\n",
  /* EUNKNOWN */
  "Neznama chyba!\n"
};

const char *HELPMSG =
  "Program Maticove operace\n"
  "Autor: David Molnar (c) 2010\n"
  "Pouziti: proj3 -h\n"
  "         proj3 --test <data.txt>\n"
  "         proj3 --vadd <a.txt> <b.txt>\n"
  "         proj3 --vscal <a.txt> <b.txt>\n"
  "         proj3 --mmult <a.txt> <b.txt>\n"
  "         proj3 --mexpr <a.txt> <b.txt>\n"
  "         proj3 --eight <v.txt> <m.txt>\n"
  "         proj3 --maze <m.txt>\n"
  "Popis parametru:\n"
  "  -h: zobrazeni napovedi\n"
  "  --test: testovaci vystup\n"
  "  --vadd: vektorovy soucet\n"
  "  --vscal: skalarni soucin vektoru\n"
  "  --mmult: soucin dvou matic\n"
  "  --mexpr: maticovy vyraz A*B*A\n"
  "  --eight: osmismerka, vyhleda v matici m vektor v\n"
  "  --maze: bludiste, vyhleda cestu z bludiste v matici m\n";

/** 8 smerov, pouzivane v osmismerke */
const int step[16] = { 0, -1, 0, 1, -1, 0, 1, 0, -1, 1, 1, 1, -1, -1, 1, -1 };

/**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  char *filename1;      /**< Meno souboru 1 */
  char *filename2;      /**< Meno souboru 2 */
  int ecode;            /**< Chybovy kod programu, odpovida vvctu tecodes. */
  int state;            /**< Stavovy kod programu, odpovida vyctu tstates. */
} TParams;

/**
 * Struktura, na ulozenie vektor matic
 */
typedef struct TVMatrix
{
  int row;  // radek
  int col;  // slupec
  int z;    // pocet matic
  int ***m;
  int type; // typ
} *TVMatrixPtr;

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
    .filename1 = NULL,
    .filename2 = NULL,
    .ecode = EOK,
    .state = 0
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
  else if (argc == 3)
  { // dva parametry
    result.filename1 = argv[2];  
  
    if (strcmp("--test", argv[1]) == 0)
    {
      result.state = CTEST;  
    }
    else if (strcmp("--bubbles", argv[1]) == 0)
    {
      result.state = CBUBBLES;
    }
    else 
    {
      result.ecode = ECLWRONG;
    }
  }
  else if (argc == 4)
  { // try parametry
    result.filename1 = argv[2]; 
    result.filename2 = argv[3]; 
  
    if (strcmp("--vadd", argv[1]) == 0)
    {
      result.state = CVADD;  
    }
    else if (strcmp("--vscal", argv[1]) == 0)
    {
      result.state = CVSCAL;
    }
    else if (strcmp("--mmult", argv[1]) == 0)
    {
      result.state = CMMULT; 
    }
    else if (strcmp("--mexpr", argv[1]) == 0)
    {
      result.state = CMEXPR; 
    }
    else if (strcmp("--eight", argv[1]) == 0)
    {
      result.state = CEIGHT; 
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
 * Alokuje vektor matic podla parametrov
 */
int allocateVMatrix(TVMatrixPtr *vmatrix, int z, int row, int col, int type)
{
  *vmatrix = (TVMatrixPtr)malloc(sizeof(struct TVMatrix));

  if (*vmatrix == NULL)
  { // nedostatek pameti
    return EOUTMEM;
  }
  
  (*vmatrix)->z = z;
  (*vmatrix)->row = row;
  (*vmatrix)->col = col;
  (*vmatrix)->m = (int***)malloc(z * sizeof(int**));
  (*vmatrix)->type = type;
  
  if ((*vmatrix)->m == NULL)
  { // nedostatek pameti, uvolnime vsetko potrebne
    free(*vmatrix);
	 *vmatrix = NULL;
	 
    return EOUTMEM;
  }
  
  for (int i = 0; i < z; i++)
  {
    int** matrix = (int**)malloc(row * sizeof(int*));

    if (matrix == NULL)
    {	// nedostatek pameti, uvolnime vsetko potrebne	  
	   for (int k = 0; k < i; k++)
		{
		  for (int m = 0; m < (*vmatrix)->row; m++)
		  {
		    free((*vmatrix)->m[k][m]);
		  }

		  free((*vmatrix)->m[k]);
		}
		
		free((*vmatrix)->m);
      free(*vmatrix);
		*vmatrix = NULL;
		
      return EOUTMEM;
    }

    for (int j = 0; j < row; j++)
    {
      matrix[j] = (int*)malloc(col * sizeof(int));
	 
      if (matrix[j] == NULL)
      { // nedostatek pameti, uvolnime vsetko potrebne	  
		  free(matrix);
		  
		  for (int k = 0; k < i; k++)
		  {
		    for (int m = 0; m < (*vmatrix)->row; m++)
		    {
		      free((*vmatrix)->m[k][m]);
		    }
			 
		    free((*vmatrix)->m[k]);
		  }
		  
		  free((*vmatrix)->m);
		  free(*vmatrix);
		  *vmatrix = NULL;
		  
        return EOUTMEM;
      }
    }
 
	 (*vmatrix)->m[i] = matrix;
  }

  return EOK;
}

/**
 * Uvolni alokovanu vektor matic
 */
void freeVMatrix(TVMatrixPtr *vmatrix)
{
  if (*vmatrix == NULL)
  {
    return;
  }
  
  if ((*vmatrix)->m != NULL)
  {
    for (int i = 0; i < (*vmatrix)->z; i++)
	 {
	   for (int j = 0; j < (*vmatrix)->row; j++)
		{
		  free((*vmatrix)->m[i][j]);
		}
		
	   free((*vmatrix)->m[i]);
	 }
	 
    free((*vmatrix)->m);
  }
  
  free(*vmatrix);
  *vmatrix = NULL;
}

/**
 * Alokuje vektor matic, a naplni hodnotami nacitane ze souboru
 */
int createVMatrix(FILE *file, TVMatrixPtr *vmatrix)
{
  if (file == NULL)
  {
    return EUNKNOWN;
  }
  
  int type = TTERROR;
  int z = 1;
  int row = 1;
  int col = 1;
  
  fscanf(file, "%d", &type); // nacitani typ ze souboru
  
  if (type < TVECTOR || type > TVMATRIX)
  { // nevhodny typ
    return TTERROR;
  }
  
  if (type == TVECTOR)
  {
    if (fscanf(file, "%d", &col) != 1 || col <= 0)
    {
      return EDIMEN;
    }
  }
  else if (type == TMATRIX)
  {
    if (fscanf(file, "%d %d", &row, &col) != 2 || row <= 0 || col <= 0)
    {
      return EDIMEN;
    }
  }
  else if (type == TVMATRIX)
  {
    if (fscanf(file, "%d %d %d", &z, &row, &col) != 3 || z <= 0 || row <= 0 || col <= 0)
    {
      return EDIMEN;
    }
  }
  
  int result = allocateVMatrix(vmatrix, z, row, col, type); // alokujeme miesto v pameti
  if (result != EOK)
  { // nastala chyba, koncime
    return result;
  }
  
  int readed;
  int status;
  int i = 0;
  int j = 0;
  int k = 0;

  while (((status = fscanf(file, "%d", &readed)) != EOF))
  { // nacitame cisla ze souboru
    if (status != 1)
	 { // nelegalni znak
	   return EILLCH;
	 }
	 
	 if (k >= z)
	 { // vela znakov
	   return ETOOCH;
	 }

    (*vmatrix)->m[k][i][j] = readed; // ulozime do matice
	 j += 1;

    if (j >= col)
    {
		 i += 1;
       j = 0;
    }
	 
	 if (i >= row)
	 {
	   k += 1;
		i = 0;
	 }
  }

  if (k != z || i != 0|| j != 0)
  { // malo znakov
    return ENOTENCH;
  }

  return EOK;
}

/**
 * Vytiskne vektor matic na standardni vystup
 */
void printVMatrix(TVMatrixPtr vmatrix)
{
  if (vmatrix == NULL) 
  {
    return;
  }
  
  if (vmatrix->type == TVECTOR)
  {
    printf("%d\n%d\n", vmatrix->type, vmatrix->col);
  }
  else if (vmatrix->type == TMATRIX)
  {
    printf("%d\n%d %d\n", vmatrix->type, vmatrix->row, vmatrix->col);
  }
  else if (vmatrix->type == TVMATRIX)
  {
    printf("%d\n%d %d %d\n", vmatrix->type, vmatrix->z, vmatrix->row, vmatrix->col);
  }
 
  for (int k = 0; k < vmatrix->z; k++)
  {
    for (int i = 0; i < vmatrix->row; i++)
    {
      for (int j = 0; j < vmatrix->col; j++)
      {
        printf("%d ", vmatrix->m[k][i][j]);
      }

      printf("\n");
    }
  }
}

/**
 * Vytiskne na stand. vystup "false"
 */
void printFalse()
{
  printf("false\n");
}

/**
 * Secte dva vektory (a+b) a osetri pripadne chyby.
 */
int addVectors(TVMatrixPtr vector1, TVMatrixPtr vector2)
{
  if (vector1->type != TVECTOR || vector2->type != TVECTOR)
  { // musime mat typ vektor
    return ETYPERR;  
  }
  
  if (vector1->col != vector2->col)
  { // pocet cisel musi byt stejne
    return EDIMEN;
  }
  
  for (int i = 0; i < vector1->col; i++)
  {
    vector1->m[0][0][i] += vector2->m[0][0][i];
  }
  
  return EOK;
}

/**
 * Vypocte skalarni soucin (a*b) dvou vektoru a osetri pripadne chyby.
 */
int multVectors(TVMatrixPtr vector1, TVMatrixPtr vector2, int *result)
{
  if (vector1->type != TVECTOR || vector2->type != TVECTOR)
  { // musime mat dva vektory
    return ETYPERR;  
  }
  
  if (vector1->col != vector2->col)
  { // pocet cisel musi byt stejny
    return EDIMEN;
  }
  
  int sum = 0;
  for (int i = 0; i < vector1->col; i++)
  {
    sum += vector1->m[0][0][i] * vector2->m[0][0][i];
  }
  
  *result = sum;
  
  return EOK;
}

/**
 * Vypocte soucin dvou matic v danem poradi (A*B) a osetri pripadne chyby.
 */
int multMatrix(TVMatrixPtr matrix1, TVMatrixPtr matrix2, TVMatrixPtr result)
{
  if (matrix1->type != TMATRIX || matrix1->type != TMATRIX || result->type != TMATRIX)
  { // musime mat matrix :)
    return ETYPERR;  
  }
  
  if (matrix1->row != matrix2->col  || 
      result->row != matrix1->row || result->col != matrix2->col)
  { // nevhodne dimenzie matic
    return EDIMEN;
  }

  for (int i = 0; i < matrix1->row; i++)
  {
    for (int j = 0; j < matrix2->col; j++)
    {
	   int sum = 0;

      for (int k = 0; k < matrix1->col; k++)
      {
        sum += matrix1->m[0][i][k] * matrix2->m[0][k][j];
      }
		
		result->m[0][i][j] = sum;
    }
  }

  return EOK;
}

/**
 * Vypocte maticovy vyraz (A*B*A) a osetri pripadne chyby.
 */
int exprMatrix(TVMatrixPtr matrix1, TVMatrixPtr matrix2, TVMatrixPtr result)
{
  if (matrix1->type != TMATRIX || matrix1->type != TMATRIX || result->type != TMATRIX)
  { // nevhodny typ
    return ETYPERR;  
  }
  
  int errcode;
  
  TVMatrixPtr temp; // na ulozeni medzivysledku
  if ((errcode = allocateVMatrix(&temp, 1, matrix1->row, matrix2->col, TMATRIX)) != EOK)
  {
    return errcode;
  }
  
  if ((errcode = multMatrix(matrix1, matrix2, temp)) != EOK)
  {
    return errcode;
  }
  
  errcode = multMatrix(temp, matrix1, result);
  
  freeVMatrix(&temp);
  
  return errcode;
}

/**
 * Vyhleda v matici M vektor v.
 */
int eight(TVMatrixPtr vector, TVMatrixPtr matrix, int *result)
{
  if (vector->type != TVECTOR || matrix->type == TVECTOR)
  {
    return ETYPERR;
  }
  
  for (int i = 0; i < matrix->z; i++)
  { // inicializacia vysledku
    result[i] = 0;
  }
  
  int res = true;
  
  for (int z = 0; z < matrix->z; z++)
  {
    for (int i = 0; i < matrix->row; i++)
    {
      for (int j = 0; j < matrix->col; j++)
	   { 
		  for (int h = 0; h < 8; h++)
		  {
		    res = 1;
		    int r = i;
		    int s = j;
	
		    for (int k = 0; k < vector->z; k++)
		    {
		      if (matrix->m[z][r][s] != vector->m[0][0][k])
		      {
		        res = 0;
				  break;
		      }
			 
			   r += step[h * 2]; // next step
			   s += step[h * 2 + 1];
			 
			   if (r < 0 || r >= matrix->row || s < 0 || s >= matrix->col)
			   { 
			     if (k < (vector->z - 1)) 
				  {
				    res = 0; // nenasli sme
				  }
				
			     break;
			   }
		    }
		  
		    if (res != 0)
		    {
		      result[z] = 1; // ok nasli sme
			   return EOK;
		    }
		  }
	   }
    }
  }
  
  return EOK;
}

/**
 * Pomocna funkce pro bubles, hlada nuly, kdyz najde, nastavi flag
 */
void findZero(TVMatrixPtr matrix, TVMatrixPtr flag, int z, int i, int j)
{
  if (i < 0 || i >= matrix->row || j < 0 || j >= matrix->col)
  {
    return;
  }
  
  if (flag->m[z][i][j] != 1 && matrix->m[z][i][j] == 0)
  {
    flag->m[z][i][j] = 1; // nastavime flag
	 
	 findZero(matrix, flag, z, i, j + 1); // pozrime 4 smery
	 findZero(matrix, flag, z, i, j - 1);
	 findZero(matrix, flag, z, i + 1, j);
	 findZero(matrix, flag, z, i - 1, j);
  }
}
 
/**
 * Spocita pocet bublin v matici M.
 */
int bubbles(TVMatrixPtr matrix, int *result)
{
  TVMatrixPtr flag; // pomocna promenna
  int errcode;
  if ((errcode = allocateVMatrix(&flag, matrix->z, matrix->row, matrix->col, matrix->type)) != EOK)
  { // asi nedostatek pameti
    return errcode;
  }
  
  int count = 0;
  
  for (int z = 0; z < flag->z; z++)
  {
    for (int i = 0; i < flag->row; i++)
    {
      for (int j = 0; j < flag->col; j++)
	   {
	     flag->m[z][i][j] = 0; // inicializacia flagu
	   }
    }
  }
  
  for (int z = 0; z < flag->z; z++)
  {
    count = 0; // vysledek
	 
    for (int i = 0; i < matrix->row; i++)
    {
      for (int j = 0; j < matrix->col; j++)
	   {
	     if (matrix->m[z][i][j] < 0)
		  { // nevhodny cislo v matici, koncime s chybou
		    freeVMatrix(&flag);
		    return EILLCH;
		  }
		
	     if (flag->m[z][i][j] != 1 && matrix->m[z][i][j] == 0)
		  {
		    findZero(matrix, flag, z, i, j);
		  
		    count += 1; // nasli sme jeden
		  }
	   }
    }
	 
	 result[z] = count;
  }
  
  freeVMatrix(&flag);
  
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
  { // tisk napovedi
    printf("%s", HELPMSG);
    return EXIT_SUCCESS;
  }
  
  int errcode = EOK;
  FILE *file1;
  FILE *file2;
  
  if ((file1 = fopen(params.filename1, "r")) == NULL)
  { // nastala chyba pri otevreni soubor1
	   printECode(EFILEOP);
      return EXIT_FAILURE;
  }
  
  if (params.state != CTEST && params.state != CBUBBLES)
  { // nastala chyba pri otevreni soubor2
    if ((file2 = fopen(params.filename2, "r")) == NULL)
	 {
	   printECode(EFILEOP);
      return EXIT_FAILURE;  
	 }
  }
  
  if (params.state == CTEST)
  { // TEST
	 TVMatrixPtr vmatrix;
	 errcode = createVMatrix(file1, &vmatrix);
	 
	 if (errcode == EOK)
	 {
	   printVMatrix(vmatrix);
	 }
	 else
	 {
	   printFalse();
	 }
	 
	 freeVMatrix(&vmatrix);
  }
  else if (params.state == CVADD)
  { // Vektor soucet
    TVMatrixPtr vector1;
	 TVMatrixPtr vector2;
	 
	 if ((errcode = createVMatrix(file1, &vector1)) == EOK && (errcode = createVMatrix(file2, &vector2)) == EOK && 
	     (errcode = addVectors(vector1, vector2)) == EOK)
	 {
	   printVMatrix(vector1);
	 }
	 else
	 {
		printFalse();
	 }
	 
	 freeVMatrix(&vector1);
	 freeVMatrix(&vector2);
  }
  else if (params.state == CVSCAL)
  { // Vektor skalarni soucin
    TVMatrixPtr vector1;
	 TVMatrixPtr vector2;

	 int result;
	 
	 if ((errcode = createVMatrix(file1, &vector1)) == EOK && (errcode = createVMatrix(file2, &vector2)) == EOK && 
	     (errcode = multVectors(vector1, vector2, &result)) == EOK)
	 {
	   printf("%d\n", result);
	 }
	 else
	 {
		printFalse();
	 }
	 
	 freeVMatrix(&vector1);
	 freeVMatrix(&vector2);
  }
  else if (params.state == CMMULT)
  { // Nasobeni matic
    TVMatrixPtr matrix1;
	 TVMatrixPtr matrix2;
	 TVMatrixPtr resultMatrix;
	 
	 if ((errcode = createVMatrix(file1, &matrix1)) == EOK && (errcode = createVMatrix(file2, &matrix2)) == EOK &&
	     (errcode = allocateVMatrix(&resultMatrix, 1, matrix1->row, matrix2->col, matrix1->type)) == EOK && 
		  (errcode = multMatrix(matrix1, matrix2, resultMatrix)) == EOK)
	 {
		printVMatrix(resultMatrix);
	 }
	 else
	 {
		printFalse();
    }
	
	 freeVMatrix(&matrix1);	
	 freeVMatrix(&matrix2);	
	 freeVMatrix(&resultMatrix);
  }
  else if (params.state == CMEXPR)
  { // Vyraz A*B*A
    TVMatrixPtr matrix1;
	 TVMatrixPtr matrix2;
	 TVMatrixPtr resultMatrix;
	 
	 if ((errcode = createVMatrix(file1, &matrix1)) == EOK && (errcode = createVMatrix(file2, &matrix2)) == EOK &&
	     (errcode = allocateVMatrix(&resultMatrix, 1, matrix1->row, matrix2->col, matrix1->type)) == EOK &&
		  (errcode = exprMatrix(matrix1, matrix2, resultMatrix)) == EOK)
	 {
		printVMatrix(resultMatrix);
	 }
	 else
	 {
		printFalse();
    }
	
	 freeVMatrix(&matrix1);	
	 freeVMatrix(&matrix2);	
	 freeVMatrix(&resultMatrix);
  }
  else if (params.state == CEIGHT)
  { // Osmismerka
    TVMatrixPtr vector;
	 TVMatrixPtr matrix;
	 errcode = createVMatrix(file2, &matrix);
	 int *result = (int*)malloc(matrix->z * sizeof(int));
	 
	 if (result == NULL)
	 {
	   errcode = EOUTMEM;
	 }
	 
	 if (errcode == EOK && (errcode = createVMatrix(file1, &vector)) == EOK && 
	    (errcode = eight(vector, matrix, result)) == EOK)
	 {
	   for (int i = 0; i < matrix->z; i++)
		{
		  if (result[i] != 0)
		  {
		    printf("true\n");
		  }
		  else
		  {
		    printf("false\n");
		  }
		}
	 }
	 else
	 {
		printFalse();
	 }
	 
	 if (result != NULL)
	 {
	   free(result);
	 }
	 
	 freeVMatrix(&vector);
	 freeVMatrix(&matrix);
  }
  else if (params.state == CBUBBLES)
  { // Bubliny
	 TVMatrixPtr matrix;
	 errcode = createVMatrix(file1, &matrix);
	 int *result = (int*)malloc(matrix->z * sizeof(int));
	 
	 if (result == NULL)
	 {
	   errcode = EOUTMEM;
	 }
	 
	 if (errcode == EOK && (errcode = bubbles(matrix, result)) == EOK)
	 {
	   for (int i = 0; i < matrix->z; i++)
		{
	     printf("%d\n", result[i]);
		}
	 }
	 else
	 {
		printFalse();
	 }
	 
	 if (result != NULL)
	 {
	   free(result);
	 }
	 
	 freeVMatrix(&matrix);
  }
  
  fclose(file1);
  
  if (params.state != CTEST && params.state != CBUBBLES)
  {
    fclose(file2);
  }
  
  if (errcode != EOK)
  {
    printECode(errcode);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* konec proj2.c */