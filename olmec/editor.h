#include "common.h"
/* the raw-mode editor */

int inputtobase(int c, int mode);
char *basetooutput(int c);

enum cursor {blockblink, blockblink_, block, underblink, under, barblink, bar};
void setcursor(enum cursor cursor);

/* setup special raw terminal mode and save restore variable */
void specialtty();

/* use restore variable to reset terminal to normal mode */
void restoretty();

/* get input line as int array of internal codes */
int *get_line(char *prompt, int **bufref, int *buflen, int *expn);

