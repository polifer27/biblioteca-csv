#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

static void reset(void);
static int endofline(FILE *fin, int c);
static int split(void);
static char *advquoted(char *p);

/** csvtest main: prueba de la biblioteca CSV **/
int main(int argc, char *argv[])
{
    int i;
    char *line;
    FILE *fp;

    if(argc != 2){
        printf("Has olvidade introducir el nombre del archivo.\n");
        exit(1);
    }
    /** Funciona con r, r+, rb, ab, a+, a solo no funciona **/
    if((fp = fopen(argv[1], "rb")) == NULL){
        printf("No se puede abrir el archivo.\n");
        exit(1);
    }
    while((line = csvgetline(fp)) != NULL){
        printf("line = '%s'\n", line);
        for(i = 0; i < csvnfield(); i++)
            printf("campo[%d] = '%s'\n", i, csvfield(i));
    }
    return 0;
}
/** csvgetline: lee una línea, crece según se requiera **/
/** entrada de muestra: "LU",86.25,"11/4/1998","2:19PM",+4.0625 **/
char *csvgetline(FILE *fin)
{
    int i, c;
    char *newl, *news;
    if(line == NULL){ /** Se asigna en la primera llamada **/
        maxline = maxfield = 1;
        line = (char *)malloc(maxline);
        sline = (char *)malloc(maxline);
        field = (char **)malloc(maxfield*sizeof(field[0]));
        if(line == NULL || sline == NULL || field  == NULL){
            reset();
            return NULL;   /** Memoria agotada **/
        }
    }
    //getc: devuelve el sig. caracter de la entrada como un entero

    for(i =0; (c = getc(fin))!=EOF && !endofline(fin, c); i++){
        if(i >= maxline-1){ /** Crece la línea **/
            maxline *= 2;   /** Duplica el tamaño actual **/
            newl = (char *)realloc(line, maxline);
            news = (char *)realloc(sline, maxline);
            if(newl == NULL || news == NULL){
                reset();
                return NULL;   /** Memoria agotada **/
            }
            line = newl;
            sline = news;
        }
        line[i] = c;
    }
    line[i] = '\0';
    if(split() == NOMEM){
        reset();
        return NULL;   /** Memoria agotada **/
    }
    return (c == EOF && i == 0) ? NULL : line;
}
/** reset: establece las variables a sus valores de inicio **/
static void reset(void)
{
    free(line); /** Free(NULL) esta permitido por el ANSI C **/
    free(sline);
    free(field);
    line = NULL;
    sline = NULL;
    field = NULL;
    maxline = maxfield = nfield = 0;
}
/** endofline: detecta y consume '\r', '\n', '\r\n' o EOF **/
static int endofline(FILE *fin, int c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if(c == '\r'){
        c = getc(fin);
        if(c != '\n' && c !=EOF)
            ungetc(c, fin); /** Se leyo demasiado; regresa c **/
    }
    return eol;
}
/** split: divide la línea en campos **/
static int split(void)
{
    char *p, **newf;
    char *sepp; /** apuntador a un caracter separador temporal **/
    int sepc; /** caracter separador temporal **/

    nfield = 0;
    if(line[0] == '\0')
        return 0;
    strcpy(sline, line);
    p = sline;

    do{
        if(nfield >= maxfield){
            maxfield *= 2;      /** duplica el tamaño actual **/
            newf = (char **)realloc(field, maxfield * sizeof(field[0]));
            if(newf == NULL)
                return NOMEM;
            field = newf;
        }
        if(*p == '"')
            sepp = advquoted(++p); /** Ignora la comilla inicial **/
        else
            sepp = p + strcspn(p, fieldsep);
        sepc = sepp[0];
        sepp[0] = '\0';  /** Termina el campo **/
        field[nfield++] = p;
        p = sepp + 1;
    } while(sepc == ',');

    return nfield;
}
/** advquoted: campo entrecomillas; devuelve un apuntador al siguiente separador **/
static char *advquoted(char *p)
{
    int i, j;
    for(i = j = 0; p[j] != '\0'; i++, j++){
        if(p[j] == '"' && p[++j] != '"'){
            /** copia hasta el siguiente separador '\0' **/
            int k = strcspn(p+j, fieldsep);

            memmove(p+i, p+j, k);
            i += k;
            j += k;
            break;
        }
        p[i] = p[j];
    }
    p[i] = '\0';
    return p + j;
}
/** csvfield: devuelve un apuntador al n-esimo campo **/
char *csvfield(int n)
{
    if(n < 0 || n >= nfield)
        return NULL;
    return field[n];
}
/** csvnfield: devuelve el numero de campos **/
int csvnfield(void)
{
    return nfield;
}

