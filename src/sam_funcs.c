


/*  Riverwinds new implementations for Vego mud.    */

#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char *where[];

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);

void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode,
 bool show);

int str_cmp(char *arg1, char *arg2);
  

/*  Setin function */
/*
void do_setin(struct char_data *ch, char *argument, int cmd)
{
   char buf[100];

      if(!*argument)
        {
           send_to_char("What do you want to set setin to ?\n\r", ch);
              return;
        }
                ch->player.setin = str_dup(argument);  
                   sprintf(buf, "Setin set to : %s\n\r", GET_SETIN(ch));
                      send_to_char(buf, ch);
}

*/

/* Setout function */
/*
void do_setout(struct char_data *ch, char *argument, int cmd)
{
   char buf[100];

     if(!*argument)
       {
          send_to_char("What do you want to set setout to ?\n\r", ch);
             return;
       }
               ch->player.setout = str_dup(argument);
                  sprintf(buf, "Setout set to : %s\n\r", GET_SETOUT(ch));
                     send_to_char(buf, ch);
}
*/

/* Review Function */
/*
void do_review(struct char_data *ch, char *argument, int cmd)
{

char buf[100];

   send_to_char("Here is what you have setin and setout set to.\n\r", ch);
   
   sprintf(buf, "Setin  : %s\n\r", GET_SETIN(ch));
     send_to_char(buf, ch);
   sprintf(buf, "Setout : %s\n\r", GET_SETOUT(ch));
     send_to_char(buf, ch);
}
*/

/* Jail Function */     

   void do_jail(struct char_data *ch, char *argument, int cmd)
{

    struct char_data *vict;
    struct obj_data *needthis; 
    char buf[100];
    int location;


   if (IS_NPC(ch))
     return;

   one_argument(argument, buf);
    
    if (!*buf)      
   {
   send_to_char("Who do you want to send to jail ?\n\r", ch);            
     return;
   }

  if ((location = real_room(9499)) == -1 )   
   {
     send_to_char("Hmm that room is not around...\n\r",ch);
     return;
   }

   else if(!generic_find(argument, FIND_CHAR_WORLD,ch,&vict,&needthis))
      {
    send_to_char("That player could not be found in the world.\n\r",ch);
        return;
      }

     else if(IS_NPC(vict))
       {
      send_to_char("Sorry you can't do that to a mob.\n\r",ch);
        return;
       }

         else if(GET_LEVEL(vict) >= GET_LEVEL(ch))
           {
          send_to_char("Thats not a good idea!\n\r",ch);
             return;
           }
          else
         
        {
          sprintf(buf, "Ok you sent %s to jail.\n\r", GET_NAME(vict));
          send_to_char(buf, ch);
          send_to_char("You are being sent to jail!\n\r",vict);
          char_from_room(vict);
          char_to_room(vict,location); 
          do_look(vict, "", 0);
          return;
        }
}

/* Demote Function */

   void do_demote(struct char_data *ch, char *argument, int cmd) 
{
     
     struct char_data *vict;
     struct obj_data *needthis;

     char buf[100];

if (IS_NPC(ch))
    return;

one_argument(argument, buf);

    if (!*buf) 
     
      {
         send_to_char("Who do you want to demote ?\n\r", ch);
           return;
      }

               if(!generic_find(argument, FIND_CHAR_WORLD,ch,&vict,&needthis))
               
            {
              send_to_char("That player can not be found in the world.\n\r",ch);
               return;
            }

               else if(IS_NPC(vict))
                {
                  send_to_char("You can't demote a mob!\n\r",ch);
                     return;
                }
                  else if(GET_LEVEL(vict) >= GET_LEVEL(ch))
                    {
                     send_to_char("Thats not a good idea!\n\r",ch);
                       return;
                    }
                    else
                        {
         sprintf(buf, "Ok you demoted %s.\n\r",GET_NAME(vict));
               send_to_char(buf, ch);
                          send_to_char("You have been demoted!\n\r",vict);  
                         vict->tmpabilities.str = 1;
                         vict->abilities.str = 1;
                         vict->tmpabilities.intel = 1;
                         vict->abilities.intel = 1;
                         vict->tmpabilities.wis = 1;
                         vict->abilities.wis = 1;
                         vict->tmpabilities.dex = 1;
                         vict->abilities.dex = 1;
                         vict->tmpabilities.con = 1;
                         vict->abilities.con = 1;
                         vict->points.gold = 1;
                         vict->points.exp = 1;
                         vict->points.max_mana = 25;
                         vict->points.max_hit = 25;
                         vict->points.max_move = 25;
                         vict->specials.practices = 0;
                         vict->specials.alignment = 0;
                         vict->player.level = 2;
                         save_char_obj(ch);  
	                 send_to_char("Auto-saving...\n\r", vict);   
                            return; 
                        }
                  }



/* pcinv function */

void do_pcinv(struct char_data *ch, char *argument, int cmd)

{
   
     struct char_data *vict;
     struct obj_data *needthis;
     char buf[100];

      if(IS_NPC(ch))
        return;

one_argument(argument, buf);

        if (!*buf)
          {
            send_to_char("Whos' inventory do you want to check ?\n\r",ch);
             return;
          }

     else if(!generic_find(argument, FIND_CHAR_WORLD,ch,&vict,&needthis))
         {
           send_to_char("That player can not be found.\n\r",ch);
            return;
         }

     else if(IS_NPC(vict))
         {
           send_to_char("You don't need to use this command on a mob.\n\r",ch);
            return;
         }

     else if(GET_LEVEL(vict) >= GET_LEVEL(ch))
         {
           send_to_char("Thats not a good idea!\n\r",ch);
            return;
         }

     else
      
       {
      
         sprintf(buf,"%s is carying :\n\r",GET_NAME(vict));
           send_to_char(buf,ch);
         list_obj_to_char(vict->carrying,ch,1,TRUE);     
          return;
       }

}

/* pceq function */

void  do_pceq(struct char_data *ch, char *argument, int cmd)
{
     int j;
     bool found;
     struct char_data *vict;
     struct obj_data *needthis;
     char buf[100];
       
if(IS_NPC(ch))
  return;

one_argument(argument, buf);


   if (!*buf)
    {
      send_to_char("Whos' eq do you want to peek at ?\n\r",ch);
         return;
    }

   else if(!generic_find(argument, FIND_CHAR_WORLD,ch,&vict,&needthis))
    {
      send_to_char("That player can not be found.\n\r",ch);
         return;
    }
   
    else if(IS_NPC(vict))
     {
       send_to_char("You don't need to use this command on a mob!\n\r",ch);
          return; 
      }

     else if(GET_LEVEL(vict) >= GET_LEVEL(ch))
      {
        send_to_char("Thats not a good idea!\n\r",ch);
          return;
      } 

    else
     {
       sprintf(buf,"This is %s's equipment :\n\r",GET_NAME(vict));           
       send_to_char(buf,ch); 
       found = FALSE;
       for (j=0; j< MAX_WEAR; j++) {
         if (vict->equipment[j]) {
            if (CAN_SEE_OBJ(ch,vict->equipment[j])) {
              send_to_char(where[j],ch);
              show_obj_to_char(vict->equipment[j],ch,1);
                 found = TRUE;
          } else {
            send_to_char(where[j],ch);
            send_to_char("Something.\n\r",ch);
               found = TRUE;
         return;
     }
    }
   }
  }
 }



/* sget function */

void do_sget(struct char_data *ch, char *argument, int cmd)

{
     char *arg1;
     char *arg2;
     struct char_data *vict;
     struct obj_data *needthis;
     char buf[100];
     struct obj_data *obj_object;
     int j;


   if(IS_NPC(ch))
      return;

argument_interpreter(argument, arg1, arg2);


      if (!*arg1)
  
   {
     send_to_char("What item do you want to get ?\n\r",ch);
       return;
   }

       if(!generic_find(argument, FIND_CHAR_WORLD,ch,&vict,&needthis))
      {
       send_to_char("That player can not be found in the world.\n\r",ch);
         return;
      }
     
   if (!*arg2)
   {
    send_to_char("Who do you want to get this item from ?\n\r",ch);
      return;
   }
          else if(IS_NPC(vict))
            {
              send_to_char("You don't need to use this on a mob!\n\r",ch);
                return;
            }

            else if(GET_LEVEL(vict) >= GET_LEVEL(ch))
              {
               send_to_char("Thats not a good idea!\n\r",ch);
                return;
              }

               else
           sprintf(buf, "You summon [%s] from %s\n\r",arg1,arg2);
              send_to_char(buf, ch);
            
          obj_object = get_object_in_equip_vis(
          ch,arg1, vict->equipment, &j);

            if(!obj_object) 
            {
            send_to_char("Hmm you can't seen to find that object ?\n\r",ch);
              return;  
            }
                   
           else
          {
          obj_to_char(unequip_char(vict,j),ch);   
            return;
          }

}


   
