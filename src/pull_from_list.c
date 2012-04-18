#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <errno.h>

#include "pull_from_list.h"
#include "linked_list.h"
#include "file_read.h"
#include "global.h"

extern char const *progname;

void pull_from_list(char *input_file, char *names_file, int min, int max) {
  FILE *fp;
  int i,count=0,l,capacity=80;
  node *n;
  list_t *list;
  char *fasta_name;
  char *line;
  kseq_t *seq;

  fp = fopen(names_file,"r");
  if (!fp) {
    fprintf(stderr,"%s - failed to open file %s\n",progname,names_file);
    exit(EXIT_FAILURE);
  }

  list = (list_t *) malloc(sizeof(list_t)); /* create list for fasta names */
  initialize_list(list);                    /* and initialize to NULL      */
  n = (node *) NULL;

  fprintf(stderr,"reading 1st name from file\n");

  /* get some space for the line */
  line = malloc(sizeof(char) * capacity); /* get memory allocated */
  if (!line) {
    fprintf(stderr, "%s - line malloc: %s\n",progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  while((i = getl(&line,fp)) != -1) {
    fasta_name = parse_name(line);
    if (fasta_name) {
      fprintf(stderr,".");
      n = initnode(fasta_name);             /* create a new node with fasta_name */
      add_to_list(list,n);                  /* add it to the list */
    }
  }

  free(line); /* free up line */
  fclose(fp); /* close file */
  fprintf(stderr,"\n");
  fprintf(stderr,"done reading from %s\n",progname);

  /* open fasta file */
  fp = gzopen(input_file,"r");
  if (!fp) {
    fprintf(stderr,"%s - Couldn't open fasta file %s\n",progname,input_file);
    exit(EXIT_FAILURE);
  }

  seq = kseq_init(fp);
  /* search through list and see if this header matches */
  while((l = kseq_read(seq)) >= 0) {
    if (search_list(list,seq->name.s)) {            /* found name in list */
      if (min > 0 && max > 0) { /* got a min and max */
        if (seq->seq.l >= min && seq->seq.l <= max) {
          count++;
          printf(">%s %s\n%s\n",seq->name.s,seq->comment.s,seq->seq.s);
        }
      } else if (min > 0 || max > 0) { /* either  min or max is 0 */
        if (min > 0 && seq->seq.l >= min) {
          count++;
          printf(">%s %s\n%s\n",seq->name.s,seq->comment.s,seq->seq.s);
        } else if (max > 0 && seq->seq.l <= max) {
          count++;
          printf(">%s %s\n%s\n",seq->name.s,seq->comment.s,seq->seq.s);
        }
      } else {
        count++;
        printf(">%s %s\n%s\n",seq->name.s,seq->comment.s,seq->seq.s);
      }
    }
  }
  kseq_destroy(seq);
  gzclose(fp); /* done reading file */

  delete_list(list,list->head); /* free the list nodes */
  free(list); /* free the list structure */

  fprintf(stderr,"Output contained %i entries\n",count);
  exit(EXIT_SUCCESS);
}
