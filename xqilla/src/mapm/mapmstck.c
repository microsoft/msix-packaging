
/* 
 *  M_APM  -  mapmstck.c
 *
 *  Copyright (C) 1999 - 2003   Michael C. Ring
 *
 *  Permission to use, copy, and distribute this software and its
 *  documentation for any purpose with or without fee is hereby granted, 
 *  provided that the above copyright notice appear in all copies and 
 *  that both that copyright notice and this permission notice appear 
 *  in supporting documentation.
 *
 *	Permission to modify the software is granted. Permission to distribute
 *	the modified code is granted. Modifications are to be distributed
 *	by using the file 'license.txt' as a template to modify the file header.
 *	'license.txt' is available in the official MAPM distribution.
 *
 *	This software is provided "as is" without express or implied warranty.
 *
 *	THIS FILE HAS BEEN MODIFIED FROM THE OFFICIAL MAPM DISTRIBUTION BY
 *	'XQilla project' on 2005/11/03.
 *	THIS FILE IS ORIGINALLY FROM MAPM VERSION 4.6.1.
 */

#include "m_apm_lc.h"

static	int	M_stack_ptr  = -1;
static	int	M_last_init  = -1;
static	int	M_stack_size = 0;

static  char    *M_stack_err_msg = "\'M_get_stack_var\', Out of memory";

static	M_APM	*M_stack_array;

/****************************************************************************/
void	M_free_all_stck()
{
int	k;

if (M_last_init >= 0)
  {
   for (k=0; k <= M_last_init; k++)
     m_apm_free(M_stack_array[k]);

   M_stack_ptr  = -1;
   M_last_init  = -1;
   M_stack_size = 0;

   MAPM_FREE(M_stack_array);
  }
}
/****************************************************************************/
M_APM	M_get_stack_var()
{
void    *vp;

if (++M_stack_ptr > M_last_init)
  {
   if (M_stack_size == 0)
     {
      M_stack_size = 18;
      if ((vp = MAPM_MALLOC(M_stack_size * sizeof(M_APM))) == NULL)
        {
         /* fatal, this does not return */

         M_apm_log_error_msg(M_APM_EXIT, M_stack_err_msg);
        }

      M_stack_array = (M_APM *)vp;
     }

   if ((M_last_init + 4) >= M_stack_size)
     {
      M_stack_size += 12;
      if ((vp = MAPM_REALLOC(M_stack_array, 
      			    (M_stack_size * sizeof(M_APM)))) == NULL)
        {
         /* fatal, this does not return */

         M_apm_log_error_msg(M_APM_EXIT, M_stack_err_msg);
        }

      M_stack_array = (M_APM *)vp;
     }

   M_stack_array[M_stack_ptr]     = m_apm_init();
   M_stack_array[M_stack_ptr + 1] = m_apm_init();
   M_stack_array[M_stack_ptr + 2] = m_apm_init();
   M_stack_array[M_stack_ptr + 3] = m_apm_init();

   M_last_init = M_stack_ptr + 3;

   /* printf("M_last_init = %d \n",M_last_init); */
  }

return(M_stack_array[M_stack_ptr]);
}
/****************************************************************************/
void	M_restore_stack(int count)
{
M_stack_ptr -= count;
}
/****************************************************************************/

