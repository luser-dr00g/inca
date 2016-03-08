/* the raw-mode editor */

/* setup special raw terminal mode and save restore variable */
void specialtty();

/* use restore variable to reset terminal to normal mode */
void restoretty();

/* get input line as int array of internal codes */
int *get_line(char *prompt, int **bufref, int *len);

