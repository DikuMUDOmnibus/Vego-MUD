/**************************************************************************
*  file: act_wiz.c , Implementation of commands.          Part of DIKUMUD *
 *  Usage : Wizard Commands.                                               *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 *                                                                         *
 *  Copyright (C) 1992, 1993 Michael Chastain, Michael Quan, Mitchell Tse  *
 *  Performance optimization and bug fixes by MERC Industries.             *
 *  You can use our stuff in any way you like whatsoever so long as this   *
 *  copyright notice remains intact.  If you like it please drop a line    *
 *  to mec@garnet.berkeley.edu.                                            *
 *                                                                         *
 *  This is free software and you are benefitting.  We hope that you       *
 *  share your changes too.  What goes around, comes around.               *
 ***************************************************************************/

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

/*   external vars  */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct ban_t *ban_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;

/* external functs */

void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
struct time_info_data age(struct char_data *ch);
void sprinttype(int type, char *names[], char *result);
void sprintbit(long vektor, char *names[], char *result);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
void renum_objs();
void renum_mobs();


/* This procedure is used in all of the important wiz commands
   such as load and others.  I (Talen) wrote it in order to ruin a
   hackers chance of exploiting them to his advantage...Kalgen
   is a good case in point.
*/
int truegod(struct char_data *ch)
{
if (strcmp(GET_NAME(ch),"Flash") &&
    strcmp(GET_NAME(ch),"Mortis") &&
    strcmp(GET_NAME(ch),"Thanatos") &&
    strcmp(GET_NAME(ch),"Goby") &&
    strcmp(GET_NAME(ch),"Benedict") &&
    strcmp(GET_NAME(ch),"Terion") &&
    strcmp(GET_NAME(ch),"Riverwind") &&
    strcmp(GET_NAME(ch),"Macdufrg") &&
    strcmp(GET_NAME(ch),"Talen") &&
    strcmp(GET_NAME(ch),"Stealblade")) {

	send_to_char("You do not have security clearance...!\n\r", ch);
	send_to_char("WARNING: Further attempts will constitute hacking...\n\r", ch);
	return FALSE;	/* Called by !truegod.  So 0 will be 1 (AKA true) */
   }

else 
    return TRUE; 	/* Called by !truegod.  So 1 will be 0 (AKA false) */
}

void do_quiet(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_STRING_LENGTH]; 

for (; *argument == ' '; argument++);

if (!(*argument)) {
   send_to_char("Please specify what you want to turn on/off.\n\r", ch);
   send_to_char("You have the following choices:\n\r", ch);
   send_to_char("quiet connections = Foo@foo.edu has connected\n\r", ch);
   send_to_char("quiet exitings = Foo@foo.edu has left the game\n\r", ch);
   send_to_char("quiet hacks = WARNING: Foo@foo.edu BAD PASSWORD\n\r", ch);
   send_to_char("quiet deaths = Foo.pc killed by Foo.mob\n\r", ch);
   return;
} else {

    switch (*argument) {
	case 'c' : case 'C': {
	  if (IS_SET(ch->player.kalbits, TAL_CONNECT))
	    REMOVE_BIT(ch->player.kalbits, TAL_CONNECT);
	  else
	    SET_BIT(ch->player.kalbits, TAL_CONNECT);
	  sprintf(buf,"You will %s see players connect.\n\r",
		 (IS_SET(ch->player.kalbits, TAL_CONNECT)?"nolonger":"now"));
	  send_to_char(buf, ch);
	break; }

	case 'e' : case 'E': {
	  if (IS_SET(ch->player.kalbits, TAL_EXIT))
	    REMOVE_BIT(ch->player.kalbits, TAL_EXIT);
	  else
	    SET_BIT(ch->player.kalbits, TAL_EXIT);
	  sprintf(buf,"You will %s see the players leave.\n\r",
		 (IS_SET(ch->player.kalbits, TAL_EXIT)?"nolonger":"now"));
	  send_to_char(buf, ch);
	break; }

	case 'd' : case 'D': {
	  if (IS_SET(ch->player.kalbits, TAL_DEATH))
	    REMOVE_BIT(ch->player.kalbits, TAL_DEATH);
	  else
	    SET_BIT(ch->player.kalbits, TAL_DEATH);
	  sprintf(buf,"You will %s see when a player dies.\n\r",
		 (IS_SET(ch->player.kalbits, TAL_DEATH)?"nolonger":"now"));
	  send_to_char(buf, ch);
	break; }

	case 'h' : case 'H': {
	  if (IS_SET(ch->player.kalbits, TAL_HACKS))
	    REMOVE_BIT(ch->player.kalbits, TAL_HACKS);
	  else
	    SET_BIT(ch->player.kalbits, TAL_HACKS);
	  sprintf(buf,"You will %s see any possible hacking attempts.\n\r",
		 (IS_SET(ch->player.kalbits, TAL_HACKS)?"nolonger":"now"));
	  send_to_char(buf, ch);
	break; }

	default : {
	send_to_char("Sorry I can't shut that one up for you.\n\r", ch);
	log("Trying to quiet a function that doesn't have a bit.\n\r");
	}
    }			/* End of switch */

/*    if (strcmp(argument,"connects") || strcmp(argument,"conn")) {
       if (IS_SET(ch->player.kalbits, TAL_CONNECT))
          REMOVE_BIT(ch->player.kalbits, TAL_CONNECT);
       else
          SET_BIT(ch->player.kalbits, TAL_CONNECT);
       sprintf(buf, "You will %s see the messages dealing with connections.\n\r",
		(IS_SET(ch->player.kalbits, TAL_CONNECT)?"now":"nolonger"));
       send_to_char(buf, ch);

    } else if (strcmp(argument,"exit") || strcmp(argument,"ex")) {
       if (IS_SET(ch->player.kalbits, TAL_EXIT))
	  REMOVE_BIT(ch->player.kalbits, TAL_EXIT);
       else
	  SET_BIT(ch->player.kalbits, TAL_EXIT);
       sprintf(buf, "You will %s see the messages dealing with players leaving.\n\r",
		(IS_SET(ch->player.kalbits, TAL_EXIT)?"now":"nolonger"));
       send_to_char(buf, ch);

     } else if (strcmp(argument,"deaths") || strcmp(argument,"de")) {
        if (IS_SET(ch->player.kalbits, TAL_DEATH))
	   REMOVE_BIT(ch->player.kalbits, TAL_DEATH);
        else
	   SET_BIT(ch->player.kalbits, TAL_DEATH);
        sprintf(buf, "You will %s see when players die.\n\r",
		(IS_SET(ch->player.kalbits, TAL_DEATH)?"now":"nolonger"));
        send_to_char(buf, ch);

     } else if (strcmp(argument,"hacks") || strcmp(argument,"ha")) {
	if (IS_SET(ch->player.kalbits, TAL_HACKS))
	   REMOVE_BIT(ch->player.kalbits, TAL_HACKS);
	else
	   SET_BIT(ch->player.kalbits, TAL_HACKS);
	sprintf(buf, "You will %s see the hacking messages.\n\r",
		(IS_SET(ch->player.kalbits, TAL_HACKS)?"now":"nolonger"));
	send_to_char(buf, ch);

     } else {
	send_to_char("Sorry I don't know what you want keep quiet.\n\r", ch);
	send_to_char("You might want to read the help for more info.\n\r", ch);
	return;
     } */
}}
 
void do_disconnect(struct char_data *ch,  char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char hack_buf[MAX_STRING_LENGTH];
    struct descriptor_data *d;
    int sdesc;

    d=descriptor_list;

    if (IS_NPC(ch))
	return;
    if (!(truegod(ch))) {
	sprintf (hack_buf, "A HACKER (%s@%s) is trying to use disconnect.\n\r",
		GET_NAME(ch), d->host);
	log (hack_buf);
	return;
    }

    one_argument(argument, arg);
    sdesc=atoi(arg);
    if(arg==0){
	    send_to_char("Illegal descriptor number.\n\r",ch);
	    send_to_char("Usage: disconnect <#>\n\r",ch);
	    return;
      }
    for(d=descriptor_list;d;d=d->next){
	    if (d->descriptor==sdesc){
		    close_socket(d);
		    sprintf(buf,"Closing socket to descriptor #%d\n\r",sdesc);
		    send_to_char(buf,ch);
		    return;
	  }
      }
    send_to_char("Descriptor not found!\n\r",ch);
}

void do_pardon(struct char_data *ch, char *argument, int cmd)
{
    char person[MAX_INPUT_LENGTH];
    char flag[MAX_INPUT_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    struct descriptor_data *d;
    struct char_data *victim;

d = descriptor_list;

    if (IS_NPC(ch))
	return;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to pardon someone.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
    }

    half_chop(argument, person, flag);

    if (!*person)
    {
	send_to_char("Pardon whom?\n\r", ch);
    }
    else
    {
	if (!(victim = get_char(person)))
	    send_to_char("They aren't here.\n\r", ch);
    else
      {
    if (IS_NPC(victim)) {
      send_to_char("Can't pardon NPCs.\n\r",ch);
      return;
    }
    if (!str_cmp("thief", flag)) {
      if (IS_SET(victim->specials.affected_by, AFF_THIEF)){
	send_to_char("Thief flag removed.\n\r",ch);
	REMOVE_BIT(victim->specials.affected_by,
	       AFF_THIEF);
	send_to_char(
		"A nice god has pardoned you of your thievery.\n\r", victim);
      } else {
	return;
      }
    }
    if (!str_cmp("killer", flag)) {
      if (IS_SET(victim->specials.affected_by, AFF_KILLER)) {
	send_to_char("Killer flag removed.\n\r",ch);
	REMOVE_BIT(victim->specials.affected_by,
	       AFF_KILLER);
	send_to_char(
	"A nice god has pardoned you of your murdering.\n\r", victim);
      } else {
	return;
      }
    } else
      {
	send_to_char("No flag specified!.\n\r",ch);
	return;
      }         
    send_to_char("Done.\n\r",ch);
    sprintf(log_buf,"%s pardons %s for %s.",
	GET_NAME(ch), GET_NAME(victim), flag);
    log(log_buf);
      }
    }
}

void do_immline(struct char_data *ch, char *argument, int cmd)
{
char buf1[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
struct descriptor_data *i;

if (IS_NPC(ch))
	return;

for (; *argument == ' '; argument++);

if (!(*argument)) {
    if (IS_SET(ch->player.kalbits, KAL_IMMLINE))
      REMOVE_BIT(ch->player.kalbits, KAL_IMMLINE);
    else
      SET_BIT(ch->player.kalbits, KAL_IMMLINE);
sprintf(buf1,"The Immortals line is now %s.\n\r",
       (IS_SET(ch->player.kalbits, KAL_IMMLINE)?"on":"off"));
act(buf1, 0, ch, 0, 0, TO_CHAR);
}
else {				/* There is an argument... */
  if (IS_SET(ch->player.kalbits, KAL_IMMLINE)) {
    sprintf(buf1,"[IMM] %s-- '%s'\n\r", GET_NAME(ch), argument);
    sprintf(buf2,"You tell the Wizards: '%s'", argument);
    act(buf2, 0, ch, 0, 0, TO_CHAR);
    for (i = descriptor_list; i; i = i->next)
       if (i->character != ch && !i->connected &&
          (cmd==10&&IS_SET(i->character->player.kalbits, KAL_IMMLINE)))
         send_to_char(buf1, i->character);
  } else {
      send_to_char("You must join the channel first.\n\r", ch);
      return;
    }
}
}
void do_deiline(struct char_data *ch, char *argument, int cmd)
{
char buf1[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
struct descriptor_data *d;

if (IS_NPC(ch)) return;

for (; *argument == ' ';argument++);

if (!(*argument)) {
   if (IS_SET(ch->player.kalbits, KAL_DEILINE))
     REMOVE_BIT(ch->player.kalbits, KAL_DEILINE);
   else
     SET_BIT(ch->player.kalbits, KAL_DEILINE);
sprintf(buf1, "The Deities line is now %s.\n\r", 
       (IS_SET(ch->player.kalbits, KAL_DEILINE)?"on":"off"));
act(buf1, 0, ch, 0, 0, TO_CHAR);
} else {
   if (IS_SET(ch->player.kalbits, KAL_DEILINE)) {
     sprintf(buf1,"[DEI] %s-- '%s'\n\r", GET_NAME(ch), argument);
     sprintf(buf2,"You tell the Deities: '%s'", argument);
     act(buf2, 0, ch, 0, 0, TO_CHAR);
     for (d = descriptor_list; d; d = d->next)
        if (d->character != ch && !d->connected &&
           (cmd==11&&IS_SET(d->character->player.kalbits, KAL_DEILINE)))
          send_to_char(buf1, d->character);
  } else {
      send_to_char("You must join the channel first.\n\r", ch);
      return;
    }
}
}

void do_supline(struct char_data *ch, char *argument, int cmd)
{
char buf1[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
struct descriptor_data *s;

if (IS_NPC(ch)) return;

for (; *argument == ' ';argument++);

if (!(*argument)) {
   if (IS_SET(ch->player.kalbits, KAL_SUPLINE))
     REMOVE_BIT(ch->player.kalbits, KAL_SUPLINE);
   else
     SET_BIT(ch->player.kalbits, KAL_SUPLINE);
sprintf(buf1, "The Supremes line is now %s.\n\r",
       (IS_SET(ch->player.kalbits, KAL_SUPLINE)?"on":"off"));
act(buf1, 0, ch, 0, 0, TO_CHAR);
} else {
   if (IS_SET(ch->player.kalbits, KAL_SUPLINE)) {
     sprintf(buf1,"[SUP] %s-- '%s'\n\r", GET_NAME(ch), argument);
     sprintf(buf2,"You tell the Supremes: '%s'", argument);
     act(buf2, 0, ch, 0, 0, TO_CHAR);
     for (s = descriptor_list; s;  s = s->next)
	if (s->character != ch && !s->connected &&
	   (cmd==12&&IS_SET(s->character->player.kalbits, KAL_SUPLINE)))
	  send_to_char(buf1, s->character);
   } else {
       send_to_char("You must join the channel first.\n\r", ch);
       return;
     }
}
}

void do_godline(struct char_data *ch, char *argument, int cmd)
{
char buf1[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];
struct descriptor_data *g;

if (IS_NPC(ch)) return;

for (; *argument == ' '; argument++);

if (!(*argument)) {
   if (IS_SET(ch->player.kalbits, KAL_GODLINE))
     REMOVE_BIT(ch->player.kalbits, KAL_GODLINE);
   else
     SET_BIT(ch->player.kalbits, KAL_GODLINE);
sprintf(buf1, "The Gods line is now %s.\n\r",
       (IS_SET(ch->player.kalbits, KAL_GODLINE)?"on":"off"));
act(buf1, 0, ch, 0, 0, TO_CHAR);
} else {
   if (IS_SET(ch->player.kalbits, KAL_GODLINE)) {
     sprintf(buf1,"[GOD] %s-- '%s'\n\r", GET_NAME(ch), argument);
     sprintf(buf2,"You tell the Gods: '%s'", argument);
     act(buf2, 0, ch, 0, 0, TO_CHAR);
     for (g = descriptor_list; g; g = g->next)
        if (g->character != ch && !g->connected &&
           (cmd==13&&IS_SET(g->character->player.kalbits, KAL_GODLINE)))
          send_to_char(buf1, g->character);
   } else {
       send_to_char("You must join the channel first.\n\r", ch);
       return;
     }
}
}

void do_emote(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	send_to_char("Yes.. But what?\n\r", ch);
    else
    {
	sprintf(buf,"$n %s", argument + i);
	  act(buf,FALSE,ch,0,0,TO_ROOM);
        sprintf(buf,"$n %s", argument + 1);
          act(buf,FALSE,ch,0,0,TO_CHAR);
    }
}



void do_echo(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    struct descriptor_data *point;
    
    if (IS_NPC(ch))
	return;

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	send_to_char("That must be a mistake...\n\r", ch);
    else
    {
	sprintf(buf,"\n\rVEGO-> %s\n\r", argument + i);
	for (point=descriptor_list; point; point = point->next)
	  if (!point->connected)
	    act(buf, 0, ch, 0, point->character, TO_VICT);
	send_to_char("Ok.\n\r", ch);
    }
}


void do_increment(struct char_data *ch, char *arg, int cmd)
{
  int i, ibot, itop;
  char str[MAX_INPUT_LENGTH];
  
  for(i=0; *(arg +i) == ' '; i++)  ;  /* parse out spaces */

  ibot=atoi(arg+i);

  for(; *(arg +i) != ' '; i++)  ;  /* Go to end of bot variable */
  for(; *(arg +i) == ' '; i++)  ;  /* parse out spaces */

  itop=atoi(arg+i);

  if ((itop-ibot > 10) && (GET_LEVEL(ch) < 33)) {
     send_to_char("Sorry your loop must be less than 10 times around.\n\r", ch);
     return;
  }
  for(; *(arg +i) != ' '; i++)  ;  /* Go to end of top variable */
  for(; *(arg +i) == ' '; i++)  ;  /* parse out spaces */
  
  for(;ibot<=itop;ibot++) {
    sprintf(str,arg+i,ibot,ibot,ibot,ibot,ibot,ibot,ibot,ibot,ibot,ibot,ibot);
    command_interpreter(ch,str);
  } 
} 


void do_system(struct char_data *ch, char *arg, int cmd)
{
 char buf[MAX_STRING_LENGTH];
 FILE *in;

 if (IS_MOB(ch)) {
   send_to_char("This wasn't made for your use...\n\r", ch);
   return;
 } 
 if (GET_LEVEL(ch)!=35) {
   send_to_char("Only the gods may use this command.\n\r",ch);
   return;
 }
 
 if (strcmp(GET_NAME(ch),"Macdufrg")&&strcmp(GET_NAME(ch),"Talen")&&
       strcmp(GET_NAME(ch),"Stealblade")&&strcmp(GET_NAME(ch),"Riverwind"))
   {
     send_to_char("Sorry... You're not the right god.\n\r",ch);
     GET_LEVEL(ch)=2;    
     GET_TT(ch)=2;       /* if someone tries this, they're unauthorized.*/
     GET_WW(ch)=2;
     GET_CC(ch)=2;
     GET_MM(ch)=2;
     return;
   }
 in=popen(arg,"r");
 while(fgets(buf,MAX_STRING_LENGTH-1,in)!=NULL) {
   send_to_char(buf,ch);
 }
 close(in);
}


void do_trans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[100];
    char log_buf[MAX_STRING_LENGTH];
    sh_int target;

    if (IS_NPC(ch))
	return;
    i = descriptor_list;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to abuse trans.\n\r",
		GET_NAME(ch), i->host);
	log(log_buf);
	return;
    }

    one_argument(argument,buf);
    if (!*buf)
	send_to_char("Whom do you wish to transfer?\n\r",ch);
    else if (str_cmp("all", buf)) {
	if (!(victim = get_char_vis(ch,buf)))
	    send_to_char("No-one by that name around.\n\r",ch);
	else {
	    act("$n disappears in a mushroom cloud.",
		FALSE, victim, 0, 0, TO_ROOM);
	    target = ch->in_room;
	    char_from_room(victim);
	    char_to_room(victim,target);
	    act("$n arrives from a puff of smoke.",
		FALSE, victim, 0, 0, TO_ROOM);
	    act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
	    do_look(victim,"",15);
	    send_to_char("Ok.\n\r",ch);
	}
    } else { /* Trans All */
    for (i = descriptor_list; i; i = i->next)
	    if (i->character != ch && !i->connected) {
		victim = i->character;
		act("$n disappears in a mushroom cloud.",
		    FALSE, victim, 0, 0, TO_ROOM);
		target = ch->in_room;
		char_from_room(victim);
		char_to_room(victim,target);
		act("$n arrives from a puff of smoke.",
		    FALSE, victim, 0, 0, TO_ROOM);
		act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
		do_look(victim,"",15);
	    }

	send_to_char("Ok.\n\r",ch);
    }
}



void do_at(struct char_data *ch, char *argument, int cmd)
{
    char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
    int loc_nr, location, original_loc;
    struct char_data *target_mob;
    struct obj_data *target_obj;
/*    extern int top_of_world;
  */  
    if (IS_NPC(ch))
	return;

    half_chop(argument, loc_str, command);
    if (!*loc_str)
    {
	send_to_char("You must supply a room number or a name.\n\r", ch);
	return;
    }

    
    if (isdigit(*loc_str))
    {
	loc_nr = atoi(loc_str);
	location = real_room(loc_nr);
	if(location<0)
	    {
		send_to_char("No room exists with that number.\n\r", ch);
		return;
	    }
    }
    else if ( ( target_mob = get_char_vis(ch, loc_str) ) != NULL )
	location = target_mob->in_room;
    else if ( ( target_obj = get_obj_vis(ch, loc_str) ) != NULL )
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else
	{
	    send_to_char("The object is not available.\n\r", ch);
	    return;
	}
    else
    {
	send_to_char("No such creature or object around.\n\r", ch);
	return;
    }

    /* a location has been found. */

    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    /* check if the guy's still there */
    for (target_mob = world[location].people; target_mob; target_mob =
	target_mob->next_in_room)
	if (ch == target_mob)
	{
	    char_from_room(ch);
	    char_to_room(ch, original_loc);
	}
}



void do_goto(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int loc_nr, location, i;
    struct char_data *target_mob, *pers;
    struct obj_data *target_obj;
/*    extern int top_of_world;
*/
    void do_look(struct char_data *ch, char *argument, int cmd);

    if (IS_NPC(ch))
	return;
    
    one_argument(argument, buf);
    if (!*buf)
    {
	send_to_char("You must supply a room number or a name.\n\r", ch);
	return;
    }

    
    if (isdigit(*buf))
    {
	loc_nr = atoi(buf);
	location=real_room(loc_nr);
	if (location < 0)
	    {
		send_to_char("No room exists with that number.\n\r", ch);
		return;
	    }
    }
    else if ( ( target_mob = get_char_vis(ch, buf) ) != NULL )
	location = target_mob->in_room;
    else if ( ( target_obj = get_obj_vis(ch, buf) ) != NULL )
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else
	{
	    send_to_char("The object is not available.\n\r", ch);
	    return;
	}
    else
    {
	send_to_char("No such creature or object around.\n\r", ch);
	return;
    }

    /* a location has been found. */
    sprintf(buf,"Going to %d [%d].\n\r",world[location].number,location);
    send_to_char(buf,ch);

    if ((IS_SET(world[location].room_flags, PRIVATE)) && (GET_LEVEL(ch) != 35))
    {
	for (i = 0, pers = world[location].people; pers; pers =
	    pers->next_in_room, i++);
	if (i > 1 && GET_LEVEL(ch)<34)
	{
	    send_to_char(
		"There's a private conversation going on in that room.\n\r", ch);
	    return;
	}
    }
#ifdef CREATE_MODE
    if (GET_LEVEL(ch) < 34) {
      if (world[location].number > ch->player.top || world[location].number < ch->player.bot)
	{
	  send_to_char("You're to work on your section, not toy around. ;)\n\r",ch);
	  return;
	}
    }
#endif
    if (!ch->specials.wizInvis)
      act("$n waves to you and vanishes into space.", FALSE,ch,0,0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    if (!ch->specials.wizInvis)
      act("$n steps out of a rift in time.", FALSE,ch,0,0, TO_ROOM);
    do_look(ch, "",15);
}

void do_setroom(struct char_data *ch, char *arg, int cmd)
{
extern int create_online(int loc);
char buf[50],buf2[50];
char buf3[40];
char log_buf[MAX_STRING_LENGTH];
int i,j,rm=0;
struct extra_descr_data *new_descr;
struct descriptor_data *d;
extern void save_online_rooms(struct char_data *ch);


buf[0]=buf2[0]=buf3[0]=0;
sscanf(arg,"%s %s %s",buf,buf2,buf3);

if (IS_NPC(ch)){
   	send_to_char("Monsters have no need to use the editor.\n\r",ch);
   	return;
}
d = descriptor_list;
if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to change the rooms around.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
    	return;
}

if (!*arg)
  {
    send_to_char("Generic Syntax: \"setroom <field> <value1>\"\n\r",ch);
    send_to_char("Possibilities are:\n\r",ch);
    send_to_char("setr saveworld          = Saves the entire world file to disk.\n\r",ch);
    send_to_char("setr create <#vnum>     = Makes a new room at #vnum.\n\r",ch);
    send_to_char("setr name <$name>       = Sets name of this room to $name.\n\r",ch);
    send_to_char("setr description        = Goes into interactive text-entry mode.\n\r",ch);
    send_to_char("setr zone <#zonenr>     = Not saved to file. -- DO NOT USE.\n\r",ch);
    send_to_char("setr flags <#flags>     = Sum of flag bits for a room. help flags.\n\r",ch);
    send_to_char("setr terrain <#sector>  = Sector type (determines mv cost.) help flags.\n\r",ch);
    send_to_char("setr light <#value>     = Not saved to file. -- DO NOT USE.\n\r",ch);
    send_to_char("setr extra <$name>      = Interactive entry of extra descriptor, $name.\n\r",ch);
    send_to_char("setr <$dir> <flags>     = setr north for more.\n\r",ch);
    return;
  }
if (!strncmp(buf,"saveworld",strlen(buf))) 
  {
    save_online_rooms(ch);
    send_to_char("Entire world saved.\n\r",ch);
    return;
  }
if (!strncmp(buf,"create",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("Create which room number?\n\r",ch);
	return;
      }
    if (GET_LEVEL(ch)<34 && (atoi(buf2)<ch->player.bot||atoi(buf2)>ch->player.top) )  return;
    create_online(atoi(buf2));
    send_to_char("Created.\n\r",ch);
  }

if (GET_LEVEL(ch)<34) {
if (world[ch->in_room].number<ch->player.bot||world[ch->in_room].number>ch->player.top) return;
}

if (!strncmp(buf,"name",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("If you want to name it, specify one!\n\r",ch);
	return;
      }
    free(world[ch->in_room].name);
    world[ch->in_room].name=strdup(strstr(arg,buf2));
    send_to_char("Name set.\n\r",ch);
    return;
  }
else if (!strncmp(buf,"description",strlen(buf)))
  {
    free(world[ch->in_room].description);
    ch->desc->str=&world[ch->in_room].description;
    send_to_char("Enter description.  Terminate with @.\n\r",ch);
    CREATE(*ch->desc->str, char, MAX_STRING_LENGTH);
    ch->desc->max_str = MAX_STRING_LENGTH;
    send_to_char("Description saved.\n\r",ch);
    return;
  }  
else if (!strncmp(buf,"zone",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("How about a zone number?\n\r",ch);
	return;
      }
    else 
      {
	world[ch->in_room].zone=atoi(buf2);
	send_to_char("Zone allocated.\n\r",ch);
	return;
      }
  }
else if (!strncmp(buf,"flags",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("Flags.. A number that is the sum of the room flags.\n\r",ch);
	return;
      }
    else
      {
	world[ch->in_room].room_flags=atoi(buf2);
	send_to_char("Flags set.\n\r",ch);
	return;
      }
  }
else if (!strncmp(buf,"terrain",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("The type of terrain there. (Also called sector type)\n\r",ch);
	return;
      }
    else
      {
	world[ch->in_room].sector_type=atoi(buf2);
	send_to_char("Terrain type set.\n\r",ch);
	return;
      }
  }
else if (!strncmp(buf,"light",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("You need a light value.. Make sure you don't have a source.\n\r",ch);
	return;
      }
    else
      {
	world[ch->in_room].light=atoi(buf2);
	send_to_char("Light value changed!?!\n\r",ch);
	return;
      }
  }
else if (!strncmp(buf,"extra",strlen(buf)))
  {
    if (!*buf2)
      {
	send_to_char("You need a keyword for the extra description.\n\r",ch);
	return;
      }
    else
      {
	for (new_descr = world[ch->in_room].ex_description;
	     new_descr; new_descr=new_descr->next)
	  {
	    if(strcmp(buf2,new_descr->keyword)==0) break;
	  }
	if (new_descr) {
	  free(new_descr->description);
	  ch->desc->str=&new_descr->description;
	  send_to_char("Enter new extra description.  Terminate with @.\n\r",ch);
	  CREATE(*ch->desc->str,char,MAX_STRING_LENGTH);
	  ch->desc->max_str=MAX_STRING_LENGTH;
	  return;
	}
	else {
	  CREATE(new_descr, struct extra_descr_data, 1);
	  new_descr->keyword = strdup(buf2);
	  ch->desc->str=&new_descr->description;
	  send_to_char("Enter extra description. Terminate with @.\n\r",ch);
	  CREATE(*ch->desc->str, char, MAX_STRING_LENGTH);
	  ch->desc->max_str=MAX_STRING_LENGTH;
	  new_descr->next=world[ch->in_room].ex_description;
	  world[ch->in_room].ex_description = new_descr;
	  return;
	}
      }
  }	  

if (!strncmp(buf,"north",strlen(buf))) i=0;
else if (!strncmp(buf,"east",strlen(buf))) i=1;
else if (!strncmp(buf,"south",strlen(buf))) i=2;
else if (!strncmp(buf,"west",strlen(buf))) i=3;
else if (!strncmp(buf,"up",strlen(buf))) i=4;
else if (!strncmp(buf,"down",strlen(buf))) i=5;
else i=-1;

if (i!=-1)
  {
    if (!*buf2)
      {
	send_to_char("Syntax for directions: [<$dir> is north, south, etc.]\n\r",ch);
	send_to_char("setr <$dir> <#room>      = 2-way link -> adds a return from other room.\n\r",ch);
	send_to_char("setr <$dir> link <#room> = Exactly the same.\n\r",ch);
	send_to_char("setr <$dir> to <#room>   = 1-way exit.\n\r",ch);
	send_to_char("setr <$dir> flags <#bits>= Set flags for <$dir> to <#bits>. help flags.\n\r",ch);
	send_to_char("setr <$dir> key <#vnum>  = Set the number of the key for door to #vnum.\n\r",ch);
	send_to_char("setr <$dir> door <$name> = Make TWO-WAY door to the $dir of name $name.\n\r",ch);
        send_to_char("setr <$dir> door1 <$name>= Make ONE-WAY door to the $dir of name $name.\n\r",ch);
	send_to_char("setr <$dir> description  = Interactive.. What you see when you \"l $dir\"\n\r",ch);
	return;
      }
    if (world[ch->in_room].dir_option[i]==0)
      CREATE(world[ch->in_room].dir_option[i], struct room_direction_data, 1);

    if (i<4) j=(i+2)%4;
    else j=(i==4?5:4);
 
    if (!strncmp(buf2,"to",strlen(buf2)))
      {
	if (GET_LEVEL(ch) < 34) {
	  if (atoi(buf3)>ch->player.top || atoi(buf3)<ch->player.bot) {
	    send_to_char("You can't make links to rooms not in your range!\n\r",ch);
	    return;
	  }
	}
	world[ch->in_room].dir_option[i]->to_room = real_room(atoi(buf3));
	send_to_char("One-way link established.\n\r",ch);
	return;
      }
    else if (!strncmp(buf2,"link",strlen(buf2)) || atoi(buf2))
      {
	if (!*buf3) strcpy(buf3,buf2);
	if (GET_LEVEL(ch) < 34) {
	  if (atoi(buf3)>ch->player.top || atoi(buf3)<ch->player.bot) {
            send_to_char("You can't link to rooms outside your range!\n\r",ch);
            return;
          }
        }
	world[ch->in_room].dir_option[i]->to_room = real_room(atoi(buf3));
	if (i<4) j=(i+2)%4;
	else j=(i==4?5:4);
	if (world[world[ch->in_room].dir_option[i]->to_room].dir_option[j]==0)
	  CREATE(world[world[ch->in_room].dir_option[i]->to_room].dir_option[j],
		 struct room_direction_data,1);
	world[world[ch->in_room].dir_option[i]->to_room].dir_option[j]->to_room=ch->in_room;
	send_to_char("Two-way link made.\n\r",ch);
	return;
      }
    else if (!strncmp(buf2,"flags",strlen(buf2)))
      {
	world[ch->in_room].dir_option[i]->exit_info = atoi(buf3);
	send_to_char("Directional flag set.\n\r",ch);
	return;
      }
    else if (!strncmp(buf2,"key",strlen(buf2)))
      {
	world[ch->in_room].dir_option[i]->key = atoi(buf3);
	send_to_char("Key number assigned.\n\r",ch);
	return;
      }
    else if (!strncmp(buf2,"description",strlen(buf2)))
      {
	free(world[ch->in_room].dir_option[i]->general_description);
	ch->desc->str=&world[ch->in_room].dir_option[i]->general_description;
	send_to_char("Enter direction description.  Terminate with @.\n\r",ch);
	CREATE(*ch->desc->str,char,MAX_STRING_LENGTH);
	ch->desc->max_str=MAX_STRING_LENGTH;
	return;
      }
    else if (!strncmp(buf2,"door",strlen(buf2))||!strncmp(buf2,"door1",strlen(buf2)))
      {
	free(world[ch->in_room].dir_option[i]->keyword);
	world[ch->in_room].dir_option[i]->keyword = strdup(buf3);
	if (!strncmp(buf2,"door1",strlen(buf2)))
	  {
	    rm=world[ch->in_room].dir_option[i]->to_room;
	    free(world[rm].dir_option[j]->keyword);
	    world[rm].dir_option[j]->keyword=strdup(buf3);
	    world[rm].dir_option[j]->key=world[ch->in_room].dir_option[i]->key;
	    world[rm].dir_option[j]->exit_info=world[ch->in_room].dir_option[i]->exit_info;
	    send_to_char("Note: Door also copied to other side.\n\r",ch);
	  }
	send_to_char("Door made.\n\r",ch);
	return;
      }
    send_to_char("Invalid option for direction...\n\r",ch);
    return;
  }
send_to_char("Setroom doesn't support that one..\n\r",ch);
}

void do_setobj(struct char_data *ch, char *arg, int cmd)
{
  struct obj_data *k;
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH];
  char buf4[MAX_INPUT_LENGTH];
  char buf5[MAX_INPUT_LENGTH];
  char log_buf[MAX_STRING_LENGTH];
  struct descriptor_data *d;
  struct extra_descr_data *new_descr;
  extern int top_of_objt_online;
  extern void save_online_obj(struct obj_data *k);
  
  buf[0]=buf2[0]=buf3[0]=buf4[0]=buf5[0]=0;
  sscanf(arg,"%s %s %s %s %s",buf, buf2, buf3, buf4, buf5);

  if (IS_NPC(ch)){
     	send_to_char("Monsters have no need to use the editor.\n\r", ch);
     	return;
  }
  d = descriptor_list;
  if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to change the objects around.\n\r",
		GET_NAME(ch), d->host);
     	return;
  }

  if (!*arg) {
    send_to_char("General Syntax: setobj <name> <field> <value1> <value2>...\n\r",ch);
    send_to_char("List of fields: [help flags for a list of help]\n\r",ch);
    send_to_char("type <#num>              = Obj type (weapon, wand, etc)\n\r",ch);
    send_to_char("flags <#num>             = Sum of flags.  Help extraflag.\n\r",ch);
    send_to_char("bitvector <#num>         = Sum of bits to affect to character.\n\r",ch);
    send_to_char("level <#num>             = Min level to wear\n\r",ch);
    send_to_char("wear <#num>              = Sum of wear bits. Help wearflag.\n\r",ch);
    send_to_char("cost <#num>              = Price.  Be reasonable with it.\n\r",ch);
    send_to_char("weight <#num>            = Weapons should weigh a lot.\n\r",ch);
    send_to_char("value <#which> <#value>  = #which is 1 to 6, Value the thing to assign.\n\r",ch);
    send_to_char("extra <$keyword>         = Set an extra descr.  Enters interactive mode.\n\r",ch);
    send_to_char("virtual <#vnum>          = Changes virtual number.\n\r",ch);
    send_to_char("affect <#which> <#value> = Set affected (1 to 6).  Help apply.\n\r",ch);
    return;
  }
  else if (!strcmp(buf,"create")) {
    if (atoi(buf2) < 1) {
      send_to_char("You must specify a virtual number.\n\r",ch);
      return;
    }
    else if (real_object(atoi(buf2)) > 0) {
      send_to_char("That object number is already in use.\n\r",ch);
      return;
    }
    CREATE(k, struct obj_data, 1);
    clear_object(k);
    k->name=strdup("item template");
    k->short_description=strdup("An item template");
    k->description=strdup("Descr of an item template.\n\r");
    k->action_description=strdup("Action description.");
    obj_to_room(k,ch->in_room);
    k->next_content=0;
    k->carried_by=0;
    k->in_obj=0;
    k->contains=0;
    k->item_number=top_of_objt_online;
    top_of_objt_online++;
    obj_index[k->item_number].virtual=atoi(buf2);
    k->obj_flags.eq_level=1000;
    k->next=object_list;
    object_list=k;
    send_to_char("An OBJ template created.\n\r",ch);
    return;
  }
  else {
    k=get_obj_vis(ch,buf);
    if (!k) {
      send_to_char("Not found\n\r",ch);
    }
    else if (!strncmp(buf2,"type",strlen(buf2))) {
      k->obj_flags.type_flag = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"flags",strlen(buf2))) {
      k->obj_flags.extra_flags = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"bitvector",strlen(buf2))) {
      k->obj_flags.bitvector = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"level",strlen(buf2))) {
      k->obj_flags.eq_level = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"wear",strlen(buf2))) {
      k->obj_flags.wear_flags = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"value",strlen(buf2))) {
      if (atoi(buf3)>5 || atoi(buf3)<0) {
	send_to_char("Must be 1 to 6.\n\r",ch);
	return;
      }
      k->obj_flags.value[atoi(buf3)-1] = atoi(buf4);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"weight",strlen(buf2))) {
      k->obj_flags.weight = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"cost",strlen(buf2))) {
      k->obj_flags.cost = atoi(buf3);
      send_to_char("Set.\n\r",ch);
    }
    else if (!strncmp(buf2,"extra",strlen(buf2))) {
      if (!*buf3)
	{
	  send_to_char("You need a keyword for the extra description.\n\r",ch);
	  return;
	}
      else
	{
	  for (new_descr = k->ex_description;
	       new_descr; new_descr=new_descr->next)
	    {
	      if(strcmp(buf3,new_descr->keyword)==0) break;
	    }
	  if (new_descr) {
	    free(new_descr->description);
	    ch->desc->str=&new_descr->description;
	    send_to_char("Enter new extra description.  Terminate with @.\n\r",ch);
	    CREATE(*ch->desc->str,char,MAX_STRING_LENGTH);
	    ch->desc->max_str=MAX_STRING_LENGTH;
	  }
	  else {
	    CREATE(new_descr, struct extra_descr_data, 1);
	    new_descr->keyword = strdup(buf3);
	    ch->desc->str=&new_descr->description;
	    send_to_char("Enter extra description. Terminate with @.\n\r",ch);
	    CREATE(*ch->desc->str, char, MAX_STRING_LENGTH);
	    ch->desc->max_str=MAX_STRING_LENGTH;
	    new_descr->next=k->ex_description;
	    k->ex_description = new_descr;
	    return;
	  }
	}
    }
    else if (!strncmp(buf2,"save",strlen(buf2))) {
      save_online_obj(k);
      send_to_char("Saved.\n\r",ch);
    }
    else if (!strncmp(buf2,"virtual",strlen(buf2))) {
      k->item_number = top_of_objt_online;
      top_of_objt_online++;
      obj_index[k->item_number].virtual=strlen(buf3);
      send_to_char("Item copied.\n\r",ch);
      renum_objs();
    }
    else if (!strncmp(buf2,"affected",strlen(buf2))) {
      if (atoi(buf3)<0 || atoi(buf3)>MAX_OBJ_AFFECT) {
	send_to_char("You need a modifier number from 1 to 6.\n\r",ch);
      }
      else {
	k->affected[atoi(buf3)-1].location = atoi(buf4);
	k->affected[atoi(buf3)-1].modifier = atoi(buf5);
	send_to_char("Set.\n\r",ch);
      }
    }
    else {
      send_to_char("Invalid setobj command.\n\r",ch);
    }
    return;
  }
}


int calc_exp(struct char_data *ch)
{
int i,sum=0;
for (i=0;i<MAX_ATTACKS&&ch->specials.attack[i].type;i++)
  {
    sum+=(ch->specials.attack[i].dice+(ch->specials.attack[i].dice*
				       ch->specials.attack[i].size))/2+
	  ch->specials.attack[i].bonus;
  }
sum *= GET_LEVEL(ch)+6;
sum *= ch->points.max_hit;
sum /= 8;
return(sum);
}


void do_setmob(struct char_data *ch, char *arg, int cmd)
{
  int i=0,tmp,tmp2,tmp3;
  struct char_data *k=0;
  struct descriptor_data *d;
  char buf5[MAX_INPUT_LENGTH];
  char buf4[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char log_buf[MAX_STRING_LENGTH];
  extern void save_online_mob(struct char_data *ch,struct char_data *k);
  extern int top_of_mobt_online;

  buf[0]=buf2[0]=buf3[0]=buf4[0]=buf5[0]=0;
  sscanf(arg,"%s %s %s %s %s",buf,buf2,buf3,buf4,buf5);

  if (IS_NPC(ch)){
     	send_to_char("Monsters have no need to use the editor.\n\r", ch);
     	return;
  }
  d = descriptor_list;
  if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to change the mobs of Vego.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
     	return;
  }

  if (!strcmp(buf,"create")) {
    if (!*buf2) {
      send_to_char("Which vnum?\n\r",ch);
      return;
    }
    if (real_mobile(atoi(buf2))>0) {
      send_to_char("WARNING: That mob already exists!\n\r",ch);
      if (GET_LEVEL(ch)<34) return;
      send_to_char("Creating it anyway..\n\r",ch);
    }
    CREATE(k, struct char_data,1);
    clear_char(k);
    k->player.name = strdup("mob template");
    k->player.short_descr=strdup("MOB Template");
    k->player.long_descr=strdup("A MOB template thingy.\n");
    k->player.description=strdup("Nothing interesting yet.\n");
    k->tmpabilities = k->abilities;
    for(i=0;i<MAX_WEAR;i++)
      k->equipment[i]=0;
    mob_index[top_of_mobt_online].virtual=atoi(buf2);
    k->nr=top_of_mobt_online;
    top_of_mobt_online++;
    k->desc=0;
    SET_BIT(k->specials.act,ACT_ISNPC);
    SET_BIT(k->specials.act,ACT_SENTINEL);
    k->player.sex=1;
    k->specials.position=8;
    k->specials.default_pos=8;
    char_to_room(k,ch->in_room);
    k->next = character_list;
    character_list=k;
    send_to_char("You create a new MOB template.\n\r",ch);
    return;
  }
  else if ( (k=get_char_vis(ch, buf)) != NULL)
    {
      if (!*buf2) {
        send_to_char("setmob create <#vnum> = Create mob template.\n\r\n\r",ch);
	send_to_char("General Syntax: setmob <$mob> <$field> <#values>\n\r",ch);
	send_to_char("save                  = Saves mob to disk.\n\r",ch);
	send_to_char("act <#sum>            = Sum of act bits.\n\r",ch);
	send_to_char("affected <#sum>       = Sum of affectations.\n\r",ch);
	send_to_char("alignment <#value>    = -1000 to 1000.\n\r",ch);
	send_to_char("attack # <dice>d<size>+<bonus>\n\r",ch);
	send_to_char("                      = Set attack number (1 to 8) to 3d4+5\n\r",ch);
	send_to_char("type # <type>         = Attack type for that numb.\n\r",ch);
	send_to_char("level <#value>        = Sets level..\n\r",ch);
	send_to_char("maxhit <#value>       = Max hit points.\n\r",ch);
	send_to_char("gold <#value>         = Cash.\n\r",ch);
	send_to_char("position <#pos>       = Position for long to be displayed.\n\r",ch);
	send_to_char("defpos <#pos>         = Position on loading.\n\r",ch);
	send_to_char("sex <#value>          = 0, 1, or 2.\n\r",ch);
	send_to_char("virtual <#number>     = Change mob's virt num.\n\r",ch);
	send_to_char("new                   = Make this mob a new real num.\n\r",ch);
	send_to_char("----------------\n\r",ch);
	send_to_char("armor, exp, damroll, hitroll, dice, size.\n\r",ch);
	return;
      }
      if (!IS_NPC(k) && GET_LEVEL(ch) < 34) {
	send_to_char("Not players..  We do MOBS.\n\r",ch);
	return;
      }
      else if (!IS_NPC(k)) {
	send_to_char("WARNING: Changing a player!!!\n\r",ch);
      }      
      if (!strncmp(buf2,"save",strlen(buf2))) {
	GET_EXP(k)=calc_exp(k);
	save_online_mob(ch,k);
	send_to_char("Saved..\n\r",ch);
	WAIT_STATE(ch,PULSE_VIOLENCE);
	return;
      }
      else if (!strncmp(buf2,"act",strlen(buf2))) {
	k->specials.act=atoi(buf3);
	SET_BIT(k->specials.act, ACT_ISNPC);
	send_to_char("Act bits set\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"affected",strlen(buf2))) {
	k->specials.affected_by=atoi(buf3);
	send_to_char("Affectations modified.\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"alignment",strlen(buf2))) {
	k->specials.alignment=atoi(buf3);
	send_to_char("Alignment set\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"attack",strlen(buf2))) {
	if (atoi(buf3)) {
	  i=atoi(buf3)-1;
	  sscanf(buf4," %dd%d+%d ",&tmp,&tmp2,&tmp3);
	  k->specials.attack[i].dice=tmp;
	  k->specials.attack[i].size=tmp2;
	  k->specials.attack[i].bonus=tmp3;
	  if (atoi(buf5))
	    k->specials.attack[i].type=atoi(buf5)+1000;
	  else if (!strncmp(buf5,"hit",strlen(buf5)))
	    k->specials.attack[i].type=1000;
	  else if (!strncmp(buf5,"pound",strlen(buf5))) 
	    k->specials.attack[i].type=1001;
	  else if (!strncmp(buf5,"pierce",strlen(buf5)))
	    k->specials.attack[i].type=1002;
	  else if (!strncmp(buf5,"slash",strlen(buf5)))
	    k->specials.attack[i].type=1003;
	  else if (!strncmp(buf5,"lash",strlen(buf5)))
	    k->specials.attack[i].type=1004;
	  else if (!strncmp(buf5,"claw",strlen(buf5)))
	    k->specials.attack[i].type=1005;
	  else if (!strncmp(buf5,"bite",strlen(buf5)))
	    k->specials.attack[i].type=1006;
	  else if (!strncmp(buf5,"sting",strlen(buf5)))
	    k->specials.attack[i].type=1007;
	  else if (!strncmp(buf5,"crush",strlen(buf5)))
	    k->specials.attack[i].type=1008;
	  else if (!strncmp(buf5,"cleave",strlen(buf5)))
	    k->specials.attack[i].type=1009;
	  else if (!strncmp(buf5,"stab",strlen(buf5)))
	    k->specials.attack[i].type=1010;
	  else
	    k->specials.attack[i].type=1000;
	}
	GET_EXP(k)=calc_exp(k);
	sprintf(buf,"Attack: [%d] %2dd%-3d+%2d  Type: %d\n\r",
		i+1,
		k->specials.attack[i].dice,k->specials.attack[i].size,
		k->specials.attack[i].bonus,k->specials.attack[i].type);
	send_to_char(buf,ch);
	return;
      }
      else if (!strncmp(buf2,"level",strlen(buf2))) {
	GET_LEVEL(k)=atoi(buf3);
	k->points.hitroll=(GET_LEVEL(k)+8)/2;
	k->points.armor=100-(GET_LEVEL(k)*40/6);
	GET_EXP(k)=calc_exp(k);
	send_to_char("Level set.\n\r",ch);
	return;
      }
      else if (!strcmp(buf2,"kalhit")) {
	k->points.hitroll=atoi(buf3);
	return;
      }
      else if (!strcmp(buf2,"kaldam")) {
	k->points.damroll=atoi(buf3);
	return;
      }
      else if (!strcmp(buf2,"kalarm")) {
	k->points.armor=atoi(buf3);
	return;
      }
      else if (!strncmp(buf2,"maxhit",strlen(buf2))) {
	k->points.max_hit=atoi(buf3);
	k->points.hit=k->points.max_hit;
	GET_EXP(k)=calc_exp(k);
	send_to_char("Hit points set.\n\r",ch);
	return;
      }
      else if (!strcmp(buf2,"kaldic")) {
	k->specials.damnodice=atoi(buf3);
	return;
      }
      else if (!strcmp(buf2,"kalsiz")) {
	k->specials.damsizedice=atoi(buf3);
	return;
      }
      else if (!strncmp(buf2,"gold",strlen(buf2))) {
	k->points.gold=atoi(buf3);
	send_to_char("Coins set.\n\r",ch);
	return;
      }
      else if (!strcmp(buf2,"kalexp")) {
	GET_EXP(k)=atoi(buf3);
	return;
      }
      else if (!strncmp(buf2,"position",strlen(buf2))) {
	k->specials.position=atoi(buf3);
	send_to_char("Position set.\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"defpos",strlen(buf2))) {
	k->specials.default_pos=atoi(buf3);
	send_to_char("Default position set.\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"sex",strlen(buf2))) {
	k->player.sex=atoi(buf3);
	send_to_char("0-Neuter, 1-Male, 2-Female.  Set.\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"virtual",strlen(buf2))) {
	if (real_mobile(atoi(buf3))>0) {
	  send_to_char("A mob under that number already exists!\n\r",ch);
	  if (GET_LEVEL(ch)<34) return;
	  send_to_char("But we'll change it anyway.\n\r",ch);
	}
	if (k->nr > 0) {
	  k->nr=top_of_mobt_online;
	  top_of_mobt_online++;
	}
	mob_index[k->nr].virtual=atoi(buf3);
	renum_mobs();
	return;
      }
      else if (!strncmp(buf2,"top",strlen(buf2))) {
	k->player.top=atoi(buf3);
	send_to_char("Top set.\n\r",ch);
	return;
      }
      else if (!strncmp(buf2,"bot",strlen(buf2))) {
	k->player.bot=atoi(buf3);
	send_to_char("Bot set.\n\r",ch);
	return;
      }
    }
}

static char *attack_table[] =
{
  "hit", "pound", "pierce", "slash", "lash", "claw",
  "bite", "sting", "crush", "cleave", "stab"
  };

	
void do_stat(struct char_data *ch, char *argument, int cmd)
{
    extern char *spells[];
    struct affected_type *aff;
    char arg1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct room_data *rm=0;
    struct char_data *k=0;
    struct obj_data  *j=0;
    struct obj_data  *j2=0;
    struct extra_descr_data *desc;
    struct follow_type *fol;
    int i, virtual;
    int i2;
    bool found;

    /* for objects */
    extern char *item_types[];
    extern char *wear_bits[];
    extern char *extra_bits[];
    extern char *drinks[];

    /* for rooms */
    extern char *dirs[];
    extern char *room_bits[];
    extern char *exit_bits[];
    extern char *sector_types[];

    /* for chars */
    extern char *equipment_types[];
    extern char *affected_bits[];
    extern char *apply_types[];
    extern char *pc_class_types[];
    extern char *npc_class_types[];
    extern char *action_bits[];
    extern char *player_bits[];
    extern char *position_types[];
    extern char *connected_types[];

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, arg1);

    /* no argument */
    if (!*arg1) {
	send_to_char("Stats on who or what?\n\r",ch);
	return;
    } else {
	/* stats on room */
	if (!str_cmp("room", arg1)) {
	    rm = &world[ch->in_room];
	    sprintf(buf,
	    "Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r",
		rm->name, rm->zone, rm->number, ch->in_room);
	    send_to_char(buf, ch);

	    sprinttype(rm->sector_type,sector_types,buf2);
	    sprintf(buf, "Sector type : %s", buf2);
	    send_to_char(buf, ch);
	    
	    strcpy(buf,"Special procedure : ");
	    strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
	    send_to_char(buf, ch);

	    send_to_char("Room flags: ", ch);
	    sprintbit((long) rm->room_flags,room_bits,buf);
	    send_to_char(buf,ch);

	    send_to_char("Description:\n\r", ch);
	    send_to_char(rm->description, ch);
	    
	    strcpy(buf, "Extra description keywords(s): ");
	    if(rm->ex_description) {
		strcat(buf, "\n\r");
		for (desc = rm->ex_description; desc; desc = desc->next) {
		    strcat(buf, desc->keyword);
		    strcat(buf, "\n\r");
		}
		strcat(buf, "\n\r");
		send_to_char(buf, ch);
	    } else {
		strcat(buf, "None\n\r");
		send_to_char(buf, ch);
	    }

	    strcpy(buf, "------- Chars present -------\n\r");
	    for (k = rm->people; k; k = k->next_in_room)
	    {
	      if(CAN_SEE(ch,k)){
		strcat(buf, GET_NAME(k));
		strcat(buf,
		    (!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r"
		    : "(MOB)\n\r")));
	      }
	    }
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);

	    strcpy(buf, "--------- Contents ---------\n\r");
	    for (j = rm->contents; j; j = j->next_content)
	    {
	      if(CAN_SEE_OBJ(ch,j))
		{
		  strcat(buf, j->name);
		  strcat(buf, "\n\r");
		}
	    }
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);
	
	    send_to_char("------- Exits defined -------\n\r", ch);
	    for (i = 0; i <= 5; i++) {
		if (rm->dir_option[i]) {
		    sprintf(buf,"Direction %s . Keyword : %s\n\r",
			    dirs[i], rm->dir_option[i]->keyword);
		    send_to_char(buf, ch);
		    strcpy(buf, "Description:\n\r  ");
		    if(rm->dir_option[i]->general_description)
		    strcat(buf, rm->dir_option[i]->general_description);
		    else
			strcat(buf,"UNDEFINED\n\r");
		    send_to_char(buf, ch);
		    sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
		    sprintf(buf,
		"Exit flag: %s \n\rKey no: %d\n\rTo room (V-Number): %d\n\r",
			    buf2, rm->dir_option[i]->key,
			    world[rm->dir_option[i]->to_room].number);
		    send_to_char(buf, ch);
		}
	    }
	    return;
	}

	/* stat on object */
	if ( ( j = get_obj_vis(ch, arg1) ) != NULL ) {
	    virtual = (j->item_number >= 0) ?
			obj_index[j->item_number].virtual : 0;
	    sprintf(buf,
	    "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
	       j->name, j->item_number, virtual);
	    sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
	    strcat(buf,buf2); strcat(buf,"\n\r");
	    send_to_char(buf, ch);
	    sprintf(buf,
		"Short description: %s\n\rLong description:\n\r%s\n\r",
		((j->short_description) ? j->short_description : "None"),
		((j->description) ? j->description : "None") );
	    send_to_char(buf, ch);
	    if(j->ex_description){
		strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
		for (desc = j->ex_description; desc; desc = desc->next) {
		    strcat(buf, desc->keyword);
		    strcat(buf, "\n\r");
		}
		strcat(buf, "----------\n\r");
		send_to_char(buf, ch);
	    } else {
		strcpy(buf,"Extra description keyword(s): None\n\r");
		send_to_char(buf, ch);
	    }

	    send_to_char("Can be worn on :", ch);
	    sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
	    strcat(buf,"\n\r");
	    send_to_char(buf, ch);

	    send_to_char("Set char bits  :", ch);
	    sprintbit(j->obj_flags.bitvector,affected_bits,buf);
	    strcat(buf,"\n\r");
	    send_to_char(buf, ch);

	    send_to_char("Extra flags: ", ch);
	    sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
	    strcat(buf,"\n\r");
	    send_to_char(buf,ch);

	    sprintf( buf,
	    "Weight: %d, Value: %d, Cost/day: %d, Timer: %d, Eq Level: %d\n\r",
		   j->obj_flags.weight,j->obj_flags.cost,
		   j->obj_flags.cost_per_day,  j->obj_flags.timer,
		   j->obj_flags.eq_level
		   );
	    send_to_char(buf, ch);

	    strcpy(buf,"In room: ");
	    if (j->in_room == NOWHERE)
		strcat(buf,"Nowhere");
	    else {
		sprintf(buf2,"%d",world[j->in_room].number);
		strcat(buf,buf2);
	    }
	    strcat(buf,", In object: ");
	    strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));
	    strcat(buf,", Carried by: ");
	    strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
	    strcat(buf,"\n\r");
	    send_to_char(buf, ch);

	    switch (j->obj_flags.type_flag) {
		case ITEM_LIGHT : 
		    sprintf(buf,
			"Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[2]);
		    break;
		case ITEM_SCROLL : 
		    sprintf(buf, "Spells : %d, %d, %d, %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[2],
			j->obj_flags.value[3] );
		    break;
		case ITEM_WAND : 
		    sprintf(buf, "Spell : %d\n\rMana : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1]);
		    break;
		case ITEM_STAFF : 
		    sprintf(buf, "Spell : %d\n\rMana : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1]);
		    break;
		case ITEM_WEAPON :
		    sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
			j->obj_flags.value[0],
		    j->obj_flags.value[1],
		    j->obj_flags.value[2],
		    j->obj_flags.value[3]);
		    break;
		case ITEM_FIREWEAPON : 
		    sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
			j->obj_flags.value[0],
		    j->obj_flags.value[1],
		    j->obj_flags.value[2],
		    j->obj_flags.value[3]);
		    break;
		case ITEM_MISSILE : 
		    sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[3]);
		    break;
		case ITEM_ARMOR :
		    sprintf(buf, "AC-apply : [%d]",
			j->obj_flags.value[0]);
		    break;
		case ITEM_POTION : 
		    sprintf(buf, "Spells : %d, %d, %d, %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[2],
			j->obj_flags.value[3]); 
		    break;
		case ITEM_TRAP :
		    sprintf(buf, "Spell : %d\n\r- Hitpoints : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1]);
		    break;
		case ITEM_CONTAINER :
		    sprintf(buf,
			"Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[3]?"Yes":"No");
		    break;
		case ITEM_DRINKCON :
		    sprinttype(j->obj_flags.value[2],drinks,buf2);
		    sprintf(buf,
	"Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : %s",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[3],
			buf2);
		    break;
		case ITEM_NOTE :
		    sprintf(buf, "Tounge : %d",
			j->obj_flags.value[0]);
		    break;
		case ITEM_KEY :
		    sprintf(buf, "Keytype : %d",
			j->obj_flags.value[0]);
		    break;
		case ITEM_FOOD :
		    sprintf(buf, "Makes full : %d\n\rPoisoned : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[3]);
		    break;
		default :
		    sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]",
			j->obj_flags.value[0],
			j->obj_flags.value[1],
			j->obj_flags.value[2],
			j->obj_flags.value[3]);
		    break;
	    }
	    send_to_char(buf, ch);
	   
	    sprintf(buf,"\n\rValues: [%d] [%d] [%d] [%d] [%d] [%d]",
		    j->obj_flags.value[0],
		    j->obj_flags.value[1],
		    j->obj_flags.value[2],
		    j->obj_flags.value[3],
		    j->obj_flags.value[4],
		    j->obj_flags.value[5]);
	    send_to_char(buf, ch);
	    
	    strcpy(buf,"\n\rEquipment Status: ");
	    if (!j->carried_by)
		strcat(buf,"NONE");
	    else {
		found = FALSE;
		for (i=0;i < MAX_WEAR;i++) {
		    if (j->carried_by->equipment[i] == j) {
			sprinttype(i,equipment_types,buf2);
			strcat(buf,buf2);
			found = TRUE;
		    }
		}
		if (!found)
		    strcat(buf,"Inventory");
	    }
	    send_to_char(buf, ch);

	    strcpy(buf, "\n\rSpecial procedure : ");
	    if (j->item_number >= 0)
		strcat(buf,
		(obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
	    else
		strcat(buf, "No\n\r");
	    send_to_char(buf, ch);

	    strcpy(buf, "Contains :\n\r");
	    found = FALSE;
	    for(j2=j->contains;j2;j2 = j2->next_content) {
		strcat(buf,fname(j2->name));
		strcat(buf,"\n\r");
		found = TRUE;
	    }
	    if (!found)
		strcpy(buf,"Contains : Nothing\n\r");
	    send_to_char(buf, ch);

	    send_to_char("Can affect char :\n\r", ch);
	    for (i=0;i<MAX_OBJ_AFFECT;i++) {
		sprinttype(j->affected[i].location,apply_types,buf2);
		sprintf(buf,
		"    Affects : %s By %d\n\r", buf2,j->affected[i].modifier);
		send_to_char(buf, ch);
	    }           
	    return;
	}

	/* mobile in world */
	if ( ( k = get_char_vis(ch, arg1) ) != NULL ) {

	    switch(k->player.sex) {
		case SEX_NEUTRAL : 
		    strcpy(buf,"NEUTRAL-SEX"); 
		    break;
		case SEX_MALE :
		    strcpy(buf,"MALE");
		    break;
		case SEX_FEMALE :
		    strcpy(buf,"FEMALE");
		    break;
		default : 
		    strcpy(buf,"ILLEGAL-SEX!!");
		    break;
	    }

	    sprintf(buf2, " %s - Name : %s [R-Number%d], In room [%d]\n\r",
	       (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
		    GET_NAME(k), k->nr, world[k->in_room].number);
	    strcat(buf, buf2);
	    send_to_char(buf, ch);
	    if (IS_MOB(k)) {
		sprintf(buf, "V-Number [%d]\n\r", mob_index[k->nr].virtual);
		send_to_char(buf, ch);
	    }

	    strcpy(buf,"Short description: ");
	    strcat(buf,
		(k->player.short_descr ? k->player.short_descr : "None"));
	    strcat(buf,"\n\r");
	    send_to_char(buf,ch);

	    strcpy(buf,"Title: ");
	    strcat(buf, (k->player.title ? k->player.title : "None"));
	    strcat(buf,"\n\r");
	    send_to_char(buf,ch);

	    send_to_char("Long description: ", ch);
	    if (k->player.long_descr)
		send_to_char(k->player.long_descr, ch);
	    else
		send_to_char("None", ch);

	    if (IS_NPC(k)) {
		strcpy(buf,"Monster Class: ");
		sprinttype(k->player.class,npc_class_types,buf2);
	    } else {
		strcpy(buf,"Class: ");
		sprinttype(k->player.class,pc_class_types,buf2);
	    }
	    strcat(buf, buf2);

	    sprintf(buf2,"   Level [%d] Alignment[%d]\n\r",k->player.level,
			  k->specials.alignment);
	    strcat(buf, buf2);
	    send_to_char(buf, ch);

	    if (!IS_NPC(k)) {
	      sprintf(buf,
		      "Str:[%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
		      GET_STR(k),
		      GET_INT(k),
		      GET_WIS(k),
		      GET_DEX(k),
		      GET_CON(k) );
	      send_to_char(buf,ch);
	      sprintf(buf,
		      "Mage:[%d]  Cleric:[%d]  Thief:[%d]  Warrior:[%d]\n\r",
		      GET_MM(k),
		      GET_CC(k),
		      GET_TT(k),
		      GET_WW(k) );
	      send_to_char(buf,ch);
	    }
	    sprintf(buf,
	    "Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
		GET_MANA(k),mana_limit(k),mana_gain(k),
		GET_HIT(k),hit_limit(k),hit_gain(k),
		GET_MOVE(k),move_limit(k),move_gain(k) );
	    send_to_char(buf,ch);

	    if (!IS_NPC(k)) {
	      sprintf(buf,
           "AC:[%d], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
		      GET_AC(k),
		      GET_GOLD(k),
		      GET_EXP(k),
		      k->points.hitroll,
		      k->points.damroll );
	      send_to_char(buf,ch);
	    }
	    else {
	      sprintf(buf,
	   "AC:[%d], Coins: [%d], Exp: [%d], Hitroll: [%d]\n\r",
		      GET_AC(k),
		      GET_GOLD(k),
		      GET_EXP(k),
		      k->points.hitroll );
	      send_to_char(buf,ch);
	    }
 
	    sprinttype(GET_POS(k),position_types,buf2);
	    sprintf(buf,"Position: %s, Fighting: %s",buf2,
		    ((k->specials.fighting) ? GET_NAME(k->specials.fighting)
		    : "Nobody") );
	    if (k->desc) {
		sprinttype(k->desc->connected,connected_types,buf2);
		strcat(buf,", Connected: ");
		strcat(buf,buf2);
	    }
	    strcat(buf,"\n\r");
	    send_to_char(buf, ch);

	    strcpy(buf,"Default position: ");
	    sprinttype((k->specials.default_pos),position_types,buf2);
	    strcat(buf, buf2);
	    if (IS_NPC(k))
	    {
		strcat(buf,",NPC flags: ");
		sprintbit(k->specials.act,action_bits,buf2);
	    }
	    else
	    {
		strcat(buf,",PC flags: ");
		sprintbit(k->specials.act,player_bits,buf2);
	    }

	    strcat(buf, buf2);

	    sprintf(buf2,",Timer [%d] \n\r", k->specials.timer);
	    strcat(buf, buf2);
	    send_to_char(buf, ch);

	    if (IS_MOB(k) && (mob_index[k->nr].func)) {
		strcpy(buf, "Mobile Special procedure : ");
		strcat(buf,
			(mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
		send_to_char(buf, ch);
	    }

	    if (IS_NPC(k)) {
		sprintf(buf, "ATTACK USED TO BE: %dd%d+%d.\n\r",
		    k->specials.damnodice, k->specials.damsizedice,k->points.damroll);
		send_to_char(buf, ch);
		for (i=0;i<MAX_ATTACKS && k->specials.attack[i].type;i++) {
		  sprintf(buf,"[#%d] %dd%-2d +%3d  Type: %s\n\r",i+1,
			  (int)k->specials.attack[i].dice,
			  (int)k->specials.attack[i].size,
			  (int)k->specials.attack[i].bonus,
			  attack_table[k->specials.attack[i].type-1000]);
/*			  k->specials.attack[i].type );
*/		  send_to_char(buf,ch);
		}
	    }

	    if (GET_LEVEL(ch)>34 && !IS_NPC(k)) {
	      sprintf(buf,"Limits of creation: [%d] to [%d].\n\r",
		      k->player.bot,k->player.top );
	      send_to_char(buf,ch);
	    }

	    if (!IS_NPC(k)) {
	      sprintf(buf,"Carried weight: %d   Carried items: %d\n\r",
		      IS_CARRYING_W(k),
		      IS_CARRYING_N(k) );
	      send_to_char(buf,ch);
	      
	      for(i=0,j=k->carrying;j;j=j->next_content,i++);
	      sprintf(buf,"Items in inventory: %d, ",i);
	      
	      for(i=0,i2=0;i<MAX_WEAR;i++)
		if (k->equipment[i]) i2++;
	      sprintf(buf2,"Items in equipment: %d\n\r", i2);
	      strcat(buf,buf2);
	      send_to_char(buf, ch);
	      
	      sprintf(buf,"Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
		      k->specials.apply_saving_throw[0],
		      k->specials.apply_saving_throw[1],
		      k->specials.apply_saving_throw[2],
		      k->specials.apply_saving_throw[3],
		      k->specials.apply_saving_throw[4]);
	      send_to_char(buf, ch);
	      
	      sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
		      k->specials.conditions[THIRST],
		      k->specials.conditions[FULL],
		      k->specials.conditions[DRUNK]);
	      send_to_char(buf, ch);
	    }
	    sprintf(buf, "Master is '%s'\n\r",
		((k->master) ? GET_NAME(k->master) : "NOBODY"));
	    send_to_char(buf, ch);
	    send_to_char("Followers are:\n\r", ch);
	    for(fol=k->followers; fol; fol = fol->next)
		act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

	    /* Showing the bitvector */
	    sprintbit(k->specials.affected_by,affected_bits,buf);
	    send_to_char("Affected by: ", ch);
	    strcat(buf,"\n\r");
	    send_to_char(buf, ch);

	    /* Routine to show what spells a char is affected by */
	    if (k->affected) {
		send_to_char("\n\rAffecting Spells:\n\r--------------\n\r",
		    ch);
		for(aff = k->affected; aff; aff = aff->next) {
		    sprintf(buf,
			"Spell : '%s'\n\r",spells[(int) aff->type-1]);
		    send_to_char(buf, ch);
		    sprintf(buf,"     Modifies %s by %d points\n\r",
			apply_types[(int) aff->location], aff->modifier);
		    send_to_char(buf, ch);
		    sprintf(buf,"     Expires in %3d hours, Bits set ",
			aff->duration);
		    send_to_char(buf, ch);
		    sprintbit(aff->bitvector,affected_bits,buf);
		    strcat(buf,"\n\r");
		    send_to_char(buf, ch);
		}
	    }
	    if (!IS_NPC(k)) {
	      sprintf(buf,
		      "WizInvis :  %d\n\r",k->specials.wizInvis);
	      send_to_char(buf,ch);
	      sprintf(buf,
	      "Holylite :  %s\n\r",((k->specials.holyLite) ? "ON" : "OFF"));
	      send_to_char(buf,ch);
	    }
	    return;
	} else {
	    send_to_char(
	    "No mobile or object by that name in the world\n\r", ch);
	}
    }
}



void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch)){
    send_to_char("There is no real reason for you to shutdow.\r\n", ch);
    return;
  }
  send_to_char("If you want to shut something down - say so!\n\r", ch);
}



void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
    extern int shutdown;
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    struct descriptor_data *d;

    if (IS_NPC(ch))
	return;

    d = descriptor_list;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to shut the game down.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
 	return;
    }

    one_argument(argument, arg);

    if (!*arg)
    {
	sprintf(buf, "Shutdown by %s.\n\r", GET_NAME(ch) );
	send_to_all(buf);
	log(buf);
	shutdown = 1;
    }

    else
	send_to_char("Go shut down someone your own size.\n\r", ch);
}


void do_snoop(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;
    struct descriptor_data *d;
    char buf[100];
    char log_buf[MAX_STRING_LENGTH];

    if (!ch->desc)
	return;

    if (IS_NPC(ch)) 
    {
	send_to_char("Did you ever try this before?\r\n", ch);
	return;
    }
    d = descriptor_list;
    if (!(truegod(ch))) {
	sprintf("A HACKER (%s@%s) is trying to snoop a fellow player.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
    }

    one_argument(argument, arg);

    if(!*arg)
    {
	send_to_char("Snoop whom ?\n\r",ch);
	return;
    }

    if(!(victim=get_char_vis(ch, arg)))
    {
	send_to_char("No such person around.\n\r",ch);
	return;
    }

    if (ch!=victim && GET_LEVEL(victim)==35) { /*KALGEN*/
        send_to_char("How DARE you snoop a GOD!?!\n\r",ch);
	send_to_char("Well..  Ok.  I suppose you can TRY. ;)\n\r",ch);
	sprintf(buf,"%s tried to SNOOP you.\n\r",GET_NAME(ch));
	send_to_char(buf,victim);
      } /* no return.. we want to LET them snoop. */

    if(!victim->desc)
    {
	send_to_char("There's no link.. nothing to snoop.\n\r",ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Ok, you just snoop yourself.\n\r",ch);
	if(ch->desc->snoop.snooping)
	    {
		ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
		ch->desc->snoop.snooping = 0;
	    }
	return;
    }

    if(victim->desc->snoop.snoop_by)    
    {
	send_to_char("Busy already. \n\r",ch);
	return;
    }

    if(GET_LEVEL(victim)>=GET_LEVEL(ch) && GET_LEVEL(ch)!=35)
    {
	send_to_char("You failed.\n\r",ch);
	sprintf(buf,"%s failed snooping you.\n\r", GET_NAME(ch));
	send_to_char(buf,victim);
	if (GET_LEVEL(ch)>31){
	  sprintf(buf,
	  "%s failed snooping %s.\n\r", GET_NAME(ch), GET_NAME(victim));
	  log(buf);
	}
	return;
    }

    send_to_char("Ok. \n\r",ch);

    if(ch->desc->snoop.snooping)
	ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

    ch->desc->snoop.snooping = victim;
    victim->desc->snoop.snoop_by = ch;
    return;
}



void do_switch(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;

    if (IS_NPC(ch))
	return;

    one_argument(argument, arg);
    
    if (!*arg)
    {
	send_to_char("Switch with who?\n\r", ch);
    }
    else
    {
	if (!(victim = get_char(arg)))
	     send_to_char("They aren't here.\n\r", ch);
	else
	{
	    if (ch == victim)
	    {
		send_to_char(
		"He he he... We are jolly funny today, eh?\n\r", ch);
		return;
	    }

	    if (!ch->desc || ch->desc->snoop.snoop_by 
	    || ch->desc->snoop.snooping)
	    {
		send_to_char(
		"Mixing snoop & switch is bad for your health.\n\r", ch);
		return;
	    }

	    if(victim->desc || (!IS_NPC(victim))) 
	    {
		send_to_char(
		   "You can't do that, the body is already in use!\n\r",ch);
	    }
	    else
	    {
		send_to_char("Ok.\n\r", ch);
		
		ch->desc->character = victim;
		ch->desc->original = ch;

		victim->desc = ch->desc;
		ch->desc = 0;
	    }
	}
    }
}



void do_return(struct char_data *ch, char *argument, int cmd)
{
    if(!ch->desc)
	return;

    if(!ch->desc->original)
    { 
	send_to_char("Huh!?!\n\r", ch);
	return;
    }
    else
    {
	send_to_char("You return to your original body.\n\r",ch);

	ch->desc->character = ch->desc->original;
	ch->desc->original = 0;

	ch->desc->character->desc = ch->desc; 
	ch->desc = 0;
    }
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
    struct char_data *vict;
    char name[100], to_force[100],buf[100]; 

    if (IS_NPC(ch)){
       send_to_char("If we were to let monsters force players it would be very unfair.\n\r", ch);
       return;
    }

    half_chop(argument, name, to_force);

    if (!*name || !*to_force)
	 send_to_char("Who do you wish to force to do what?\n\r", ch);
    else if (str_cmp("all", name)) {
	if (!(vict = get_char_vis(ch, name)))
	    send_to_char("No-one by that name here..\n\r", ch);
	else
	{
	    if ((GET_LEVEL(ch) <= GET_LEVEL(vict)) && !IS_NPC(vict) &&
		GET_LEVEL(ch)<35){
		send_to_char("Oh no you don't!!\n\r", ch);
		sprintf(buf, "$n has forced you to '%s'.", to_force);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		  }
	    else {
		sprintf(buf, "$n has forced you to '%s'.", to_force);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		command_interpreter(vict, to_force);
	    }
	}
    } else { /* force all */
    for (i = descriptor_list; i; i = i->next)
	    if (i->character != ch && !i->connected) {
		vict = i->character;
		if (GET_LEVEL(ch) <= GET_LEVEL(vict) &&
		    GET_LEVEL(ch) <  35)
		    send_to_char("Oh no you don't!!\n\r", ch);
		else {
		    sprintf(buf, "$n has forced ALL to '%s'.", to_force);
		    act(buf, FALSE, ch, 0, vict, TO_VICT);
		    command_interpreter(vict, to_force);
		}
	    }
	    send_to_char("Ok.\n\r", ch);
    }
}


void do_load(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob;
    struct obj_data *obj;
    struct descriptor_data *d;
    char log_buf[MAX_STRING_LENGTH];
    char type[100], num[100], buf[100];
    int number, r_num;
    

    if (IS_NPC(ch))
	return;
    d = descriptor_list;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying load some favorite items.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
    }

    argument_interpreter(argument, type, num);

    if (!*type || !*num || !isdigit(*num))
    {
	send_to_char("Syntax:\n\rload <'char' | 'obj'> <number>.\n\r", ch);
	return;
    }

    if ((number = atoi(num)) < 0)
    {
	send_to_char("A NEGATIVE number??\n\r", ch);
	return;
    }
    if (is_abbrev(type, "char"))
    {
	if ((r_num = real_mobile(number)) < 0)
	{
	    send_to_char("There is no monster with that number.\n\r", ch);
	    return;
	}
	mob = read_mobile(r_num, REAL);
	char_to_room(mob, ch->in_room);

	act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	    0, 0, TO_ROOM);
	act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
	send_to_char("Done.\n\r", ch);
	sprintf(buf,"%s loads %s at %s.",GET_NAME(ch),
		mob->player.short_descr,world[ch->in_room].name);
	log(buf);

    }
    else if (is_abbrev(type, "obj"))
    {
	if ((r_num = real_object(number)) < 0)
	{
	    send_to_char("There is no object with that number.\n\r", ch);
	    return;
	}
	obj = read_object(r_num, 0);
	obj_to_room(obj, ch->in_room);
	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
	act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
	send_to_char("Ok.\n\r", ch);
	sprintf(buf,"%s loads %s at %s.",GET_NAME(ch),
		obj->short_description,world[ch->in_room].name);
	log(buf);

    }
    else
	send_to_char("That'll have to be either 'char' or 'obj'.\n\r", ch);
}





/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict, *next_v;
    struct obj_data *obj, *next_o;

    char name[100], buf[100];

    if (IS_NPC(ch))
	return;

    one_argument(argument, name);

    if (*name)  /* argument supplied. destroy single object or char */
    {
	if (!(vict = get_char_room_vis(ch, name)) && (GET_LEVEL(ch) > 32))
	    vict = get_char(name);
	if (vict)
	{
	    if (!IS_NPC(vict) && (GET_LEVEL(ch)<=GET_LEVEL(vict))) { 
	      sprintf(buf,
  "Foooooooooom!\n\r%s is surrounded with scorching flames and looks shocked!\n\rAfter a few moments, where you expect a pile of charred ashes stands %s cackling with insane glee!\n\rYou think you are in trouble.\n\r",
		GET_NAME(vict),GET_NAME(vict));
	      send_to_char(buf,ch);
	      act("$n tried to purge you.", FALSE, ch, 0, vict, TO_VICT);
	      sprintf(buf,"%s tried to purge %s at %s.",
	      GET_NAME(ch),GET_NAME(vict),
		  world[ch->in_room].name);
	      log(buf);
	      return;
	    }

	    act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

	    if ((vict->desc) && (GET_LEVEL(ch)>32))
	    {
		sprintf( log_buf, "%s purges %s at %s.",
		    GET_NAME(ch), GET_NAME(vict),
		    world[ch->in_room].name );
		log( log_buf );
		close_socket( vict->desc );
		vict->desc  = NULL;
               extract_char(vict, TRUE);
               save_char_obj(vict);
	    }
/*	    extract_char( vict, TRUE );
	    save_char_obj( vict ); */
	}
	else if ( ( obj = get_obj_in_list_vis(ch, name,
	    world[ch->in_room].contents) ) != NULL )
	{
	    act("$n picks up the $p, and eats it.", FALSE, ch, obj, 0, TO_ROOM);
	    act("You destroy the $p.", FALSE, ch, obj, 0, TO_CHAR);
	    extract_obj(obj);
	}
	else
	{
	    send_to_char( "You can't find it to purge!\n\r", ch );
	    return;
	}


    }
    else   /* no argument. clean out the room */
    {
	if (IS_NPC(ch))
	{
	    send_to_char("Don't... You would kill yourself too.\n\r", ch);
	    return;
	}

	act("$n gestures... and there is a great rush of soapy water!!", 
	    FALSE, ch, 0, 0, TO_ROOM);
	send_to_room("The world seems a little cleaner.\n\r", ch->in_room);

	for (vict = world[ch->in_room].people; vict; vict = next_v)
	{
	    next_v = vict->next_in_room;
	    if (IS_NPC(vict))
		extract_char(vict, TRUE);
	}

	for (obj = world[ch->in_room].contents; obj; obj = next_o)
	{
	    next_o = obj->next_content;
	    extract_obj(obj);
	}
    }
}



/* Generate the five abilities */
void roll_abilities(struct char_data *ch)
{
    ch->abilities.str       = 13;
    ch->abilities.intel     = 13;
    ch->abilities.wis       = 13;
    ch->abilities.dex       = 13;
    ch->abilities.con       = 13;

    switch ( GET_CLASS(ch) )
    {
	case CLASS_MAGIC_USER:  ch->abilities.intel = 16; break;
	case CLASS_CLERIC:      ch->abilities.wis   = 16; break;
	case CLASS_WARRIOR:     ch->abilities.str   = 16; break;
	case CLASS_THIEF:       ch->abilities.dex   = 16; break;
    }
    ch->tmpabilities = ch->abilities;
}



void do_start(struct char_data *ch)
{

    send_to_char(
	"Welcome. This is now your character in DikuMud,\n\rYou can now earn XP, and lots more...\n\r",
	ch);

    GET_LEVEL(ch) = 1;
    GET_EXP(ch) = 1;

    set_title(ch);

    /*
     * Base abilities.
     * 21 practices are enough to train to 18 prime stat and +3 other stats.
     */
    roll_abilities(ch);
    ch->points.max_hit  = 10;

    switch (GET_CLASS(ch))
    {

    case CLASS_MAGIC_USER:
	ch->specials.practices = 21;
	break;

    case CLASS_CLERIC:
	ch->specials.practices = 21;
	break;

    case CLASS_THIEF:
	ch->skills[ SKILL_SNEAK     ].learned = 10;
	ch->skills[ SKILL_HIDE      ].learned =  5;
	ch->skills[ SKILL_STEAL     ].learned = 15;
	ch->skills[ SKILL_BACKSTAB  ].learned = 10;
	ch->skills[ SKILL_PICK_LOCK ].learned = 10;
	ch->specials.practices = 21;
	break;

    case CLASS_WARRIOR:
	ch->specials.practices = 21;

	break;
    }

    advance_level(ch);

    GET_HIT(ch)  = hit_limit(ch);
    GET_MANA(ch) = mana_limit(ch);
    GET_MOVE(ch) = move_limit(ch);

    GET_COND(ch,THIRST) = 44;
    GET_COND(ch,FULL)   = 44;
    GET_COND(ch,DRUNK)  = 0;

    ch->player.time.played = 0;
    ch->player.time.logon  = time(0);

}


void do_advance(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct descriptor_data *d;
    char log_buf[MAX_STRING_LENGTH];
    char name[100], level[100], buf[300], passwd[100];
    int adv, newlevel;
    int i;

    void gain_exp(struct char_data *ch, int gain);

    if (IS_NPC(ch))
	return;
    d = descriptor_list;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to advance players.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
    }

    half_chop(argument, name, buf);
    argument_interpreter(buf, level, passwd);

    if (*name)
    {
	if (!(victim = get_char_room_vis(ch, name)))
	{
	    send_to_char("That player is not here.\n\r", ch);
	    return;
	}
    } else {
	send_to_char("Advance whom?\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("NO! Not on NPC's.\n\r", ch);
	return;
    }

    if ( !*level || (newlevel = atoi(level)) <= 0 || newlevel > 35 )
    {
	send_to_char( "Level must be 1 to 35.\n\r", ch );
	return;
    }

    if ( GET_LEVEL(ch) < 35 && newlevel >= GET_LEVEL(ch) )
    {
	send_to_char( "Limited to levels lower than yours.\n\r", ch );
	return;
    }

/* Lower level:  First reset the player to level 1. Remove all special
   abilities (all spells, BASH, STEAL, et).  Reset practices to
   zero.  Then act as though you're raising a first level character to
   a higher level.  Note, currently, an implementor can lower another imp.
   -- Swifttest */

    if (newlevel <= GET_LEVEL(victim))
      {
	send_to_char("Warning:  Lowering a player's level!\n\r", ch);

		GET_LEVEL(victim) = 1;
		GET_EXP(victim) = 1;

		set_title(victim);

		victim->points.max_hit  = 10;  /* These are BASE numbers  */
		for(i=1;i<MAX_SKILLS;i++) { /* Zero them out first */
			victim->skills[i].learned=0;
	      }
		victim->specials.practices = 0;

		switch (GET_CLASS(victim)) {

	case CLASS_THIEF : {
				victim->skills[SKILL_SNEAK].learned = 10;
				victim->skills[SKILL_HIDE].learned =  5;
				victim->skills[SKILL_STEAL].learned = 15;
				victim->skills[SKILL_BACKSTAB].learned = 10;
				victim->skills[SKILL_PICK_LOCK].learned = 10;
		  } break;

		}


		GET_HIT(victim) = hit_limit(victim);
		GET_MANA(victim) = mana_limit(victim);
		GET_MOVE(victim) = move_limit(victim);

		GET_COND(victim,THIRST) = 24;
		GET_COND(victim,FULL) = 24;
		GET_COND(victim,DRUNK) = 0;

		advance_level(victim);
      }

	adv = newlevel - GET_LEVEL(victim);

    send_to_char("You feel generous.\n\r", ch);
    act("$n makes some strange gestures.\n\rA strange feeling comes uppon you,"
	"\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
	"body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
	"ur head seems to be filled with deamons\n\rfrom another plane as your"
	" body dissolves\n\rto the elements of time and space itself.\n\rSudde"
	"nly a silent explosion of light snaps\n\ryou back to reality. You fee"
	"l slightly\n\rdifferent.",FALSE,ch,0,victim,TO_VICT);

    sprintf(buf,
	"%s advances %s to level %d.",GET_NAME(ch),GET_NAME(victim),newlevel);
    log(buf);

    if (GET_LEVEL(victim) == 0) {
	do_start(victim);
    } else {
	if (GET_LEVEL(victim) < 35) {
	  if (newlevel<30) {
	    gain_exp_regardless(victim,
		exp_table[GET_LEVEL(victim)+adv] - GET_EXP(victim) );
	    send_to_char("Reminder:  Be careful with this.\n\r", ch);
	  }
	  else { 
	    GET_LEVEL(victim)=newlevel;
	    GET_TT(victim)=newlevel;
	    GET_WW(victim)=newlevel;
	    GET_CC(victim)=newlevel;
	    GET_MM(victim)=newlevel;
	  }
	    if(GET_LEVEL(victim)>30){
	      send_to_char(
 "If you want to be immortal, type HELP RULES!\n\r", victim);
	    }
	} else {
	    send_to_char("Some idiot just tried to advance your level.\n\r",
		victim);
	    send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
	}
    }
}


void do_reroll(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[100];

    if (IS_NPC(ch))
	return;

    one_argument(argument,buf);
    if (!*buf)
	send_to_char("Whom do you wish to reroll?\n\r",ch);
    else
	if(!(victim = get_char(buf)))
	    send_to_char("No-one by that name in the world.\n\r",ch);
	else {
	    send_to_char("Rerolled...\n\r", ch);
	    roll_abilities(victim);
	    sprintf(buf,"%s rerolled %s.",GET_NAME(ch),GET_NAME(victim));
	    log(buf);
	}
}


void do_restore(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct descriptor_data *d;
    char log_buf[MAX_STRING_LENGTH];
    char buf[100];
    int i;

    void update_pos( struct char_data *victim );

    if (IS_NPC(ch))
	return;
    d = descriptor_list;
    if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying restore another player.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
    }

    one_argument(argument,buf);
    if (!*buf)
	send_to_char("Whom do you wish to restore?\n\r",ch);
    else
	if(!(victim = get_char(buf)))
	    send_to_char("No-one by that name in the world.\n\r",ch);
	else {
	    GET_MANA(victim) = GET_MAX_MANA(victim);
	    GET_HIT(victim) = GET_MAX_HIT(victim);
	    GET_MOVE(victim) = GET_MAX_MOVE(victim);

	 sprintf(buf,"%s restored %s.",GET_NAME(ch),GET_NAME(victim));
	 log(buf);

	    if (GET_LEVEL(victim) > 31 ) {
		for (i = 0; i < MAX_SKILLS; i++) {
		    victim->skills[i].learned = 100;
		    victim->skills[i].recognise = TRUE;
                    GET_COND(victim, THIRST) = -1;
                    GET_COND(victim, FULL) = -1;
                    GET_COND(victim, DRUNK) = -1;
		}

		if (GET_LEVEL(victim) > 33) {
		    victim->abilities.intel = 25;
		    victim->abilities.wis = 25;
		    victim->abilities.dex = 25;
		    victim->abilities.str = 25;
		    victim->abilities.con = 25;
		}
		victim->tmpabilities = victim->abilities;

	    }
	    update_pos( victim );
	    send_to_char("Done.\n\r", ch);
	    act("You have been fully healed by $N!",
	    FALSE, victim, 0, ch, TO_CHAR);
	}
}




void do_noshout(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
	if (IS_SET(ch->specials.act, PLR_NOSHOUT))
	{
	    send_to_char("You can now hear shouts again.\n\r", ch);
	    REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
	}
	else
	{
	    send_to_char("From now on, you won't hear shouts.\n\r", ch);
	    SET_BIT(ch->specials.act, PLR_NOSHOUT);
	}
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.act, PLR_NOSHOUT))
    {
	send_to_char("You can shout again.\n\r", vict);
	send_to_char("NOSHOUT removed.\n\r", ch);
	REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    }
    else
    {
	send_to_char("The gods take away your ability to shout!\n\r", vict);
	send_to_char("NOSHOUT set.\n\r", ch);
	SET_BIT(vict->specials.act, PLR_NOSHOUT);
    }
}


/*********************************************************************/
/* New routines by Dionysos.                                         */
/*********************************************************************/

void do_wizinvis(struct char_data *ch, char *argument, int cmd)
{
   if(IS_NPC(ch)){
	return;
      }
   if(*argument) ch->specials.wizInvis = MIN(GET_LEVEL(ch),MIN(35,atoi(argument)));
   else ch->specials.wizInvis = GET_LEVEL(ch);
   send_to_char("Wizinvis changed.\n\r",ch);
}

void do_holylite(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch)) return;

	if (argument[0] != '\0') {
	   send_to_char(
	   "HOLYLITE doesn't take any arguments; arg ignored.\n\r", ch);
     } /* if */

	if (ch->specials.holyLite) {
	   ch->specials.holyLite = FALSE;
	   send_to_char("Holy light mode off.\n\r",ch);
     }
	else {
	   ch->specials.holyLite = TRUE;
	   send_to_char("Holy light mode on.\n\r",ch);
     } /* if */
}

void do_teleport(struct char_data *ch, char *argument, int cmd)
{
   struct char_data *victim, *target_mob, *pers;
   struct descriptor_data *d;
   char log_buf[MAX_STRING_LENGTH];
   char person[MAX_INPUT_LENGTH], room[MAX_INPUT_LENGTH];
   sh_int target;
   int loop;
/*   extern int top_of_world;
*/

   if (IS_NPC(ch))
	return;
   d = descriptor_list;
   if (!(truegod(ch))) {
	sprintf(log_buf,"A HACKER (%s@%s) is trying to teleport.\n\r",
		GET_NAME(ch), d->host);
	log(log_buf);
	return;
   }

   half_chop(argument, person, room);
   if(GET_LEVEL(ch)<34) return;

   if (!*person) {
      send_to_char("Who do you wish to teleport?\n\r", ch);
      return;
    } /* if */

   if (!*room) {
      send_to_char("Where do you wish to send this person?\n\r", ch);
      return;
    } /* if */

   if (!(victim = get_char_vis(ch, person))) {
      send_to_char("No-one by that name around.\n\r", ch);
      return;
    } /* if */

   if (isdigit(*room)) {
      target = atoi(&room[0]);
      loop = real_room((int)target);
      target = (sh_int) loop;
      if(loop<0)
	{
	    send_to_char("No room exists with that number.\n\r", ch);
	    return;
       }
    } else if ( ( target_mob = get_char_vis(ch, room) ) != NULL ) {
      target = target_mob->in_room;
    } else {
      send_to_char("No such target (person) can be found.\n\r", ch);
      return;
    } /* if */

   if (IS_SET(world[target].room_flags, PRIVATE)&&GET_LEVEL(ch)<34) {
      for (loop = 0, pers = world[target].people; pers;
	   pers = pers->next_in_room, loop++);
      if (loop > 1) {
	  send_to_char(
	    "There's a private conversation going on in that room\n\r",
	    ch);
	 return;
    } /* if */
    } /* if */

   act("$n disappears in a puff of smoke.",FALSE,victim, 0,0, TO_ROOM);
   char_from_room(victim);
   char_to_room(victim, target);
   act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
   act("$n has teleported you!", FALSE, ch, 0, (char *)victim, TO_VICT);
   do_look(victim, "", 15);
   send_to_char("Teleport completed.\n\r", ch);

 } /* do_teleport */

void do_ban(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH];
	char log_buf[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct ban_t *tmp;
   	struct descriptor_data *d;
	int count;

	if(IS_NPC(ch))
		return;
 	d = descriptor_list;
	if (!(truegod(ch))) {
		sprintf(log_buf,"A HACKER (%s@%s) is trying to ban sites.\n\r",
			GET_NAME(ch), d->host);
		log(log_buf);
		return;
	}

	one_argument(argument,name);

	if(!*name)
	{
	    /* list the sites */
	    send_to_char(
	    "------------\n\rBanned sites\n\r------------\n\r", ch);
	    if(ban_list==(struct ban_t*)NULL){
		    send_to_char("Empty list!\n\r",ch);
		    return;
	  }
	    for(count=0,tmp=ban_list; tmp; count++,tmp=tmp->next){
		    sprintf(buf,"%s\n\r",tmp->name);
		    send_to_char(buf,ch);
	  }
	    sprintf(buf,"\n\nThere are %d banned sites.\n\r",count);
	    send_to_char(buf,ch);
	    return;
      }
	for(tmp=ban_list; tmp; tmp=tmp->next){
		if(str_cmp(name,tmp->name)==0){
			send_to_char("That site is already banned!\n\r",ch);
			return;
	      }
	  }
	CREATE(tmp,struct ban_t,1);
	CREATE(tmp->name,char,strlen(name)+1);
	strcpy(tmp->name,name);
	tmp->next=ban_list;
	ban_list=tmp;
      }


void do_allow(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH];
	char log_buf[MAX_STRING_LENGTH];
	struct ban_t *curr, *prev;
	struct descriptor_data *d;

	if(IS_NPC(ch))
		return;
	d = descriptor_list;
	if (!(truegod(ch))) {
		sprintf(log_buf,"A HACKER (%s@%s) is trying to allow sites.\n\r",
			GET_NAME(ch), d->host);
		log(log_buf);
		return;
	}

	one_argument(argument,name);

	if(!*name){
		send_to_char("Remove which site from the ban list?\n\r",ch);
		return;
	  }
	curr=prev=ban_list;
	if(str_cmp(curr->name,name)==0){
		ban_list=ban_list->next;
		free(curr->name);
		free(curr);
		send_to_char("Ok.\n\r",ch);
		return;
	  }
	curr=curr->next;
	while(curr){
		if(!strcmp(curr->name,name)){
			if(curr->next){
				prev->next=curr->next;
				free(curr->name);
				free(curr);
				send_to_char("Ok.\n\r",ch);
				return;
		  }
			prev->next=(struct ban_t*)NULL;
			free(curr->name);
			free(curr);
			send_to_char("Ok.\n\r",ch);
			return;
	      }
		curr=curr->next;
		prev=prev->next;
	  }
	send_to_char("Site not found in list!\n\r",ch);
}

