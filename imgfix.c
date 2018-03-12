#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


static void
process_img(FILE *input, FILE *output)
{
    register c;
    char last = 0;


    while ( (c = fgetc(input)) != EOF && c != '>' ) {
	last = c;
	fputc(c, output);
    }

    if ( c == '>' ) {
	if ( last != '/' )
	    fputc('/', output);
	fputc(c, output);
    }
}


static void
process_a(FILE *input, FILE *output)
{
    static char flickrattribute[]="data-flickr-embed=\"true\"";
    int i = 0;
    register c;

    while ( (c = fgetc(input)) != EOF && c != '>' ) {
	if ( c == flickrattribute[i] ) {
	    if ( ++i == sizeof(flickrattribute)-1 )
		i = 0;	/* matched the Forbidden Attribute */
	}
	else {
	    if ( i > 0 ) {
		fwrite(flickrattribute, i, 1, output);
		i = 0;
	    }
	    fputc(c, output);
	    if ( c == '"' ) {	/* ignore "quoted" "values" */
		while ( (c = fgetc(input)) != EOF && c != '"' )
		    putc(c, output);
		if ( c != EOF )
		    putc(c, output);
	    }
	}
    }
    if ( i > 0 ) fwrite(flickrattribute, i, 1, output);
    if ( c != EOF )
	putc(c, output);
}


static int
imgfix(FILE *input, FILE *output)
{
    register c;
    char tag[8];
    char last;


    while ( (c = fgetc(input)) != EOF ) {
	if ( c == '<' ) {
	    int i = 0;
	    while ( (c = fgetc(input)) != EOF ) {
		if ( isalpha(c) && (i < sizeof(tag)) )
		    tag[i++] = c;
		else
		    break;
	    }
	    fputc('<',output);
	    if ( i > 0 )
		fwrite(tag, i, 1, output);
	    if ( c != EOF )
		fputc(c, output);
	    
	    if ( i > 0 ) {
		if ( strncmp(tag, "img", i) == 0 )
		    process_img(input, output);
		else if ( strncmp(tag, "a", i) == 0 )
		    process_a(input, output);
		i = 0;
	    }
	}
	else
	    fputc(c, output);
    }
    return ferror(output);
}


main(argc,argv)
char **argv;
{
    int i;
    int dryrun = 0;
    int ofd;
    char *outputfile = malloc(1);

    if ( outputfile == 0 ) {
	fprintf(stderr, "memory squeeze! cannot make initial output file\n");
	exit(1);
    }
    
    if ( argc > 1 && strcmp(argv[1], "-n") == 0 ) {
	dryrun = 1;
	++argv, --argc;
    }

    if ( argc < 2 )
	return imgfix(stdin, stdout);


    for ( i=1; i < argc; i++ ) {
	FILE *input;
	FILE *output;

	if ( (input = fopen(argv[i], "r")) == NULL ) {
	    fprintf(stderr, "cannot open <%s>\n", argv[i]);
	    continue;
	}

	if ( dryrun ) {
	    imgfix(input, stdout);
	}
	else {
	    if ( (outputfile = realloc(outputfile, strlen(argv[i])+7)) == 0 ) {
		fprintf(stderr, "out of memory allocating work file name\n");
		exit(1);
	    }
	    strcpy(outputfile, argv[i]);
	    strcat(outputfile, "XXXXXX");

	    if ( (ofd = mkstemp(outputfile)) == EOF ) {
		fprintf(stderr, "cannot create work file <%s>\n", outputfile);
		exit(1);
	    }
	    output = fdopen(ofd, "w");

	    if ( output == NULL ) {
		fprintf(stderr, "cannot open work file <%s>\n", outputfile);
		exit(1);
	    }

	    if ( imgfix(input, output) == 0 ) {
		fclose(output);
		close(ofd);
		if ( rename(outputfile, argv[i]) != 0 ) {
		    perror(argv[i]);
		    unlink(outputfile);
		    exit(1);
		}
	    }
	}
    }
}
