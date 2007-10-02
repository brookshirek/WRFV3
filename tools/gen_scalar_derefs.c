#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "protos.h"
#include "registry.h"
#include "data.h"

#define DUMMY 1
#define ACTUAL 2 

int
gen_scalar_derefs ( char * dirname )
{
  int i ;
  
  for ( i = 0 ; i < get_num_cores() ; i++ )
    scalar_derefs ( dirname , get_corename_i(i) ) ; 
  return(0) ;
}

#define DIR_COPY_OUT 1
#define DIR_COPY_IN  2

int
scalar_derefs ( char * dirname , char * corename )
{
  FILE * fp ;
  char  fname[NAMELEN] ;
  char * fn = "_scalar_derefs.inc" ;
  char * p ;
  int linelen ;
  char outstr[64*4096] ;

  if ( dirname == NULL || corename == NULL ) return(1) ;
  if ( strlen(dirname) > 0 ) 
   { sprintf(fname,"%s/%s%s",dirname,corename, fn) ; }
  else                       
   { sprintf(fname,"%s%s",corename,fn) ; }

  if ((fp = fopen( fname , "w" )) == NULL ) return(1) ;
  print_warning(fp,fname) ;
  fprintf(fp,"! BEGIN SCALAR DEREFS\n") ;
  linelen = 0 ;
  if ( sw_limit_args ) {
    fprintf(fp,"#undef CPY\n") ;
    fprintf(fp,"#undef CPYC\n") ;
    fprintf(fp,"#ifdef COPY_OUT\n") ;
    scalar_derefs1 ( fp , corename , &Domain, DIR_COPY_OUT ) ;
    fprintf(fp,"#else\n") ;
    scalar_derefs1 ( fp , corename , &Domain, DIR_COPY_IN ) ;
    fprintf(fp,"#endif\n") ;
  }
  fprintf(fp,"! END SCALAR DEREFS\n") ;
  close_the_file( fp ) ;
  return(0) ;
}

int
scalar_derefs1 ( FILE * fp , char * corename, node_t * node, int direction )
{
  node_t * p ;
  int tag ;
  char fname[NAMELEN] ;

  if ( node == NULL ) return(1) ;
  for ( p = node->fields ; p != NULL ; p = p->next )
  {
    if ( p->node_kind & I1 ) continue ;              /* short circuit any field that is not state */
                                                     /* short circuit DERIVED types */
    if ( p->type->type_type == DERIVED ) continue ;
                                                     /* short circuit non-scalars */
    if ( p->ndims > 0 ) continue ; 

    if (                 (
                                                   /* if it's a core specific field and we're doing that core or... */
          (p->node_kind & FIELD && (!strncmp("dyn_",p->use,4)&&!strcmp(corename,p->use+4)))
                                                   /* it is not a core specific field and it is not a derived type -ajb */
       || (p->node_kind & FIELD && (p->type->type_type != DERIVED) && ( strncmp("dyn_",p->use,4)))
#if 0
                                                   /* it is a state variable */
       || (p->node_kind & RCONFIG )
#endif
                         )
       )
    {
      for ( tag = 1 ; tag <= p->ntl ; tag++ )
      {
        char * x ;
        /* if this is a core-specific variable, prepend the name of the core to */
        /* the variable at the driver level */
        if ((!strncmp("dyn_",p->use,4)&&!strcmp(corename,p->use+4)) ) { x = "C" ; } else { x = "" ; }
        strcpy(fname,field_name(t4,p,(p->ntl>1)?tag:0)) ;
        /* generate deref */
        if ( direction == DIR_COPY_OUT ) {
          if ( (!strncmp("dyn_",p->use,4)&&!strcmp(corename,p->use+4)) ) { fprintf(fp, " grid%%%s_%s = %s\n",corename,fname,fname) ; }
          else                                { fprintf(fp, " grid%%%s    = %s\n",fname,fname ) ; }
        } else {
          if ( (!strncmp("dyn_",p->use,4)&&!strcmp(corename,p->use+4)) ) { fprintf(fp, " %s = grid%%%s_%s\n",fname,corename,fname) ; }
          else                                { fprintf(fp, " %s = grid%%%s\n",fname,fname ) ; }
        }
      }
    }
  }
  return(0) ;
}

