/***************************************************************************
 *  file: act_comm.c , Implementation of commands.         Part of DIKUMUD *
 *  Usage : Communication.                                                 *
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

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern const char *color_table[];

void do_say(struct char_data *ch, char *argument, int cmd)
{
    int i;
    struct char_data *to;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
    else if (CAN_SEE(to, ch))
    {
        sprintf(buf,"%s says '%s'",
	   (!IS_MOB(ch) ? GET_NAME(ch) : ch->player.short_descr), argument + i);
	to=world[ch->in_room].people;
	for(;to;to=to->next_in_room) {
	  sprintf(buf2,"%s%s%s\n\r", 
		  (IS_SET(to->player.kalbits,KAL_COLOR)?
		   color_table[to->player.col[CHA_SAY]]:""),
		buf,
		  (IS_SET(to->player.kalbits,KAL_COLOR)?
		   color_table[to->player.col[CHA_NORMAL]]:""));
	  if(to != ch)
	     send_to_char(buf, to);
        }
    }
    else  /* "Someone says 'argument'", added by Talen (Rick) */
    {
        sprintf(buf, "Someone says '%s'", argument + i);
        to=world[ch->in_room].people;
        for(;to;to=to->next_in_room) {
           sprintf(buf2, "%s%s%s/n/r",
                   (IS_SET(to->player.kalbits, KAL_COLOR)?
                    color_table[to->player.col[CHA_SAY]]:""),
                  buf,
		    (IS_SET(to->player.kalbits,KAL_COLOR)?
		     color_table[to->player.col[CHA_NORMAL]]:""));
	   if (to != ch)
               act(buf,FALSE,ch,0,0,TO_ROOM);
        }
    }
    sprintf(buf,"You say '%s'", argument + i);
    act(buf,FALSE,ch,0,0,TO_CHAR);
}

void do_shout(struct char_data *ch, char *argument, int cmd)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct descriptor_data *i;


    if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT))
    {
	send_to_char("You can't shout!!\n\r", ch);
	return;
    }

    if (GET_MOVE(ch) < GET_MAX_MOVE(ch)/3 && cmd!=10 && cmd!=11 && cmd!=12)
    {
	send_to_char("You are too exhausted to shout.  Use gossip or auction instead!\n\r", ch);
	return;
    }

    if(cmd!=10&&cmd!=11&&cmd!=12) GET_MOVE(ch) -= GET_MAX_MOVE(ch)/3;

    for (; *argument == ' '; argument++);

    if (!(*argument)) {
      if (cmd==10)
	{
	if (IS_SET(ch->specials.act, PLR_GOSSIP))
	  REMOVE_BIT(ch->specials.act, PLR_GOSSIP);
	else
	  SET_BIT(ch->specials.act, PLR_GOSSIP);
	sprintf(buf1, "Gossip channel is now %s.\n\r",
		(IS_SET(ch->specials.act, PLR_GOSSIP)?"on":"off"));
	act(buf1, 0, ch, 0, 0, TO_CHAR);
        }
      else if (cmd==11)
	{
	if (IS_SET(ch->specials.act, PLR_AUCTION))
          REMOVE_BIT(ch->specials.act, PLR_AUCTION);
        else
          SET_BIT(ch->specials.act, PLR_AUCTION);
        sprintf(buf1, "Auction channel is now %s.\n\r",
                (IS_SET(ch->specials.act, PLR_AUCTION)?"on":"off"));
        act(buf1, 0, ch, 0, 0, TO_CHAR);
        }
       else if (cmd==12)
         {
         if (IS_SET(ch->player.kalbits, KAL_WIZLINE))
           REMOVE_BIT(ch->player.kalbits, KAL_WIZLINE);
         else
           SET_BIT(ch->player.kalbits, KAL_WIZLINE);
         sprintf(buf1, "The immortal line is now %s.\n\r",
                 (IS_SET(ch->player.kalbits, KAL_WIZLINE)?"on":"off"));
         act(buf1, 0, ch, 0, 0, TO_CHAR);
         }
      else
	act("Perhaps you could specify what you want to shout?\n\r"
	    ,0,ch,0,0,TO_CHAR);
    }
    else
    {
      if(cmd==10)
	{
        if (CAN_SEE(i->character, ch))
	   sprintf(buf1, "%s gossips-- '%s'",
	           (!IS_MOB(ch) ? GET_NAME(ch) : ch->player.short_descr), argument);
        else if (!(CAN_SEE(i->character, ch)))
                sprintf(buf1,"Someone gossips-- '%s'", argument);
	sprintf(buf2, "You gossip-- '%s'", argument);
        }
      else if (cmd==11)
	{
        if (CAN_SEE(i->character, ch))
	   sprintf(buf1, "%s auctions-- '%s'",
                   (!IS_MOB(ch) ? GET_NAME(ch) : ch->player.short_descr), argument);
	else if (!(CAN_SEE(i->character, ch)))	/*What if they are can't see? Someone...*/
                sprintf(buf1,"Someone auctions-- '%s'", argument);
        sprintf(buf2, "You auction-- '%s'", argument);
        }
      else if (cmd==12)
        {
        if (CAN_SEE(i->character, ch))
           sprintf(buf1, "%s: '%s'", GET_NAME(ch), argument);/* Mobs can't use*/
        else if (!(CAN_SEE(i->character, ch)))		     /*wizline        */
 	     sprintf(buf1, "Someone: '%s'", argument);
        sprintf(buf2, "Over the wizline you say: '%s'", argument);
        }
      else
	{
        if (CAN_SEE(i->character, ch))
	   sprintf(buf1, "%s shouts '%s'",
	          (!IS_MOB(ch)?GET_NAME(ch):ch->player.short_descr), argument);
        else if (!(CAN_SEE(i->character, ch)))
		sprintf(buf1, "Someone shouts '%s'", argument);
	sprintf(buf2, "USE GOSSIP AND AUCTION INSTEAD.\n\rYou shout '%s'",
                argument);
        }
      act(buf2, 0, ch, 0, 0, TO_CHAR);
      
      for (i = descriptor_list; i; i = i->next)
	if (i->character != ch && !i->connected &&
	    !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	    ((cmd==10&&IS_SET(i->character->specials.act, PLR_GOSSIP)) ||
	     (cmd==11&&IS_SET(i->character->specials.act, PLR_AUCTION)) ||
             (cmd==12&&IS_SET(i->character->player.kalbits, KAL_WIZLINE)) ||
	     (cmd!=10&&cmd!=11&&cmd!=12))) {
	  sprintf(buf2,"%s%s%s\n\r",
		  (IS_SET(i->character->player.kalbits,KAL_COLOR)?
		   color_table[i->character->player.col[
		   (cmd==9?CHA_SHOUT:(cmd==10?CHA_GOSSIP:CHA_AUCTION))]]:""),
		buf1,
		  (IS_SET(i->character->player.kalbits,KAL_COLOR)?
		   color_table[i->character->player.col[CHA_NORMAL]]:""));
	  send_to_char(buf2,i->character);
	}
    }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you wish to tell what??\n\r", ch);
    else if (!(vict = get_char_vis(ch, name)))
	send_to_char("No-one by that name here.\n\r", ch);
    else if (ch == vict)
	send_to_char("You try to tell yourself something.\n\r", ch);
    else if ( ( GET_POS(vict) == POSITION_SLEEPING  ) && GET_LEVEL(ch)<31 )
    {
	act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
    }
    else
    {
     if (CAN_SEE(vict, ch))
	sprintf(buf,"%s%s tells you, '%s'%s\n\r",
		(IS_SET(vict->player.kalbits,KAL_COLOR)?
		 color_table[vict->player.col[CHA_TELL]]:""),
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message,
		(IS_SET(vict->player.kalbits,KAL_COLOR)?
		 color_table[vict->player.col[CHA_NORMAL]]:""));
     else if (!(CAN_SEE(vict, ch))) {
             sprintf(buf,"Someone tells you, '%s'\n\r", message); 
	     send_to_char(buf, vict);
     }
     sprintf(buf,"You tell %s, '%s'\n\r",
            (IS_MOB(ch)?ch->player.short_descr:GET_NAME(ch)), message);
     send_to_char(buf, ch);
    }
}



void do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\n\r", ch);
    else if (vict == ch)
    {
	act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
	send_to_char(
	    "You can't seem to get your mouth close enough to your ear...\n\r",
	     ch);
    }
    else
    {
     if (CAN_SEE(vict, ch)) {
	sprintf(buf,"$s whispers to you, '%s'\n\r",
                (IS_MOB(ch)?ch->player.short_descr:GET_NAME(ch)), message);
	send_to_char(buf, vict);
     }
     else if (!(CAN_SEE(vict, ch))) {
	     sprintf(buf,"Someone whispers to you, '%s'\n\r", message);
 	     send_to_char(buf, vict);
     }
    sprintf(buf,"You whisper to $N, '%s'.",message);
    act(buf,FALSE,ch,0,vict,TO_CHAR);
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you want to ask something, and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here.\n\r", ch);
    else if (vict == ch)
    {
	act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
	send_to_char("You think about it for a while...\n\r", ch);
    }
    else
    {
     if (CAN_SEE(vict, ch)) {
	sprintf(buf,"$n asks you, '%s'.\n\r",
               (IS_MOB(ch) ? ch->player.short_descr : GET_NAME(ch)),message);
	send_to_char(buf, vict);
     }
     else if (!(CAN_SEE(vict, ch))) {
             sprintf(buf,"Someone asks you, '%s'\n\r", argument);
             send_to_char(buf, vict);
     }
     sprintf(buf,"You ask $N, '%s'.\n\r",message);
     act(buf, FALSE, ch, 0, vict, TO_CHAR);
    }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0, *pen = 0;
    char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH];

    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
	return;

    if (!*papername)  /* nothing was delivered */
    {   
	send_to_char(
	    "Write? with what? ON what? what are you trying to do??\n\r", ch);
	return;
    }
    if (*penname) /* there were two arguments */
    {
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
    }
    else  /* there was one arg.let's see what we can find */
    {           
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */
	{
	    pen = paper;
	    paper = 0;
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE)
	{
	    send_to_char("That thing has nothing to do with writing.\n\r", ch);
	    return;
	}

	/* one object was found. Now for the other one. */
	if (!ch->equipment[HOLD])
	{
	    sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD]))
	{
	    send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	    return;
	}
	
	if (pen)
	    paper = ch->equipment[HOLD];
	else
	    pen = ch->equipment[HOLD];
    }
	    
    /* ok.. now let's see what kind of stuff we've found */
    if (pen->obj_flags.type_flag != ITEM_PEN)
    {
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE)
    {
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    }
    else if (paper->action_description)
	send_to_char("There's something written on it already.\n\r", ch);
    else
    {
	/* we can write - hooray! */
		
	send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
	    ch);
	act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
	ch->desc->str = &paper->action_description;
	ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}
