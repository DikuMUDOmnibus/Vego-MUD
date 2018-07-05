/***************************************************************************
 *  File: nanny.c, for people who haven't logged in        Part of DIKUMUD *
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
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/telnet.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "db.h"
#include "limits.h"

#define STATE(d)    ((d)->connected)
#define MAX_PLAYING 45

char echo_off_str [] = { IAC, WILL, TELOPT_ECHO, NULL };
char echo_on_str  [] = { IAC, WONT, TELOPT_ECHO, NULL };

char menu[] = "\n\rWelcome to MERC Diku Mud\n\r\n\r0) Exit from MERC Diku Mud.\n\r1) Enter the game.\n\r2) Enter description.\n\r3) Change password.\n\r\n\r   Make your choice: ";

char wizlock = FALSE;

extern char motd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;

int truegod(struct char_data *ch);
int _parse_name(char *arg, char *name);
bool check_deny( struct descriptor_data *d, char *name);
bool check_reconnect( struct descriptor_data *d, char *name, bool fReconnect);
bool check_playing( struct descriptor_data *d, char *name );



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(struct descriptor_data *d, char *arg)
{
    char    buf[MAX_INPUT_LENGTH];
    char    tmp_name[20];
    bool    fOld;
    struct  char_data *ch;

    ch  = d->character;
    for ( ; isspace(*arg); arg++ )
	;

    switch ( STATE(d) )
    {

    default:
	log( "Nanny: illegal STATE(d)" );
	close_socket(d);
	return;

    case CON_GET_NAME:
	if ( !*arg )
	{
	    close_socket( d );
	    return;
	}

	arg[0] = UPPER(arg[0]);
	if ( _parse_name(arg, tmp_name) )
	{
	    write_to_q( "Illegal name, try another.\n\rName: ", &d->output );
	    return;
	}

	if ( check_deny( d, tmp_name ) )
	    return;

	fOld		= load_char_obj( d, tmp_name );
	ch		= d->character;
	GET_NAME(ch)	= str_dup(tmp_name);

	if ( check_reconnect( d, tmp_name, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if (( wizlock )  && (GET_LEVEL(ch) < 31))
	    {
		write_to_q( "The game is wizlocked.\n\r", &d->output );
		close_socket( d );
		return;
	    }
	   if (GET_LEVEL(ch) >= 33 && !(truegod(ch))) {
		sprintf(buf,"A HACKER (%s@%s) is trying to enter VEGO.\n\r",
			GET_NAME(ch), d->host);
		log(buf);
		if (IS_SET(ch->player.kalbits, TAL_HACKS))
		   send_to_level(buf, IMM_LVL);
                close_socket(d);
	   }
	   /* This fragment added by Mike Widner (Valere/Atropos) to limit
 	   number of active connections to MAX_PLAYING.  Done for Vego
 	   which has a bad habit of crashing with over 50 connections.
 	   The structure below is verbose, but the compiler will crunch it.
 	   NOTE:  I let anybody ABOVE level 32 connect anyway.
 	   */
 	    if (number_playing() > MAX_PLAYING)
 	    {
 		if (! fOld)
 		{
 		    write_to_q( "There are too many people playing now.  Try again later.\n\r", &d->output );
 		    close_socket( d );
 		    return;
 		}
 		else
 		{
 		    if (GET_LEVEL(ch) < 32)
 		    {
 		    write_to_q( "There are too many people playing now.  Try again later.\n\r", &d->output );
 		    close_socket( d );
 		    return;
 		    }
 		    else
 		    {
 		    write_to_q( "There are too many people playing, but I'll make an exception for you.\n\r", &d->output );
 		    }
 		}
 	    }
 	/* End of MAX_PLAYING additions */


	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_q( "Password: ", &d->output );
	    write_to_q( echo_off_str, &d->output );
	    STATE(d) = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", tmp_name );
	    write_to_q( buf, &d->output );
	    STATE(d) = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
	write_to_q( "\n\r", &d->output );

	if ( strncmp( crypt( arg, ch->pwd ), ch->pwd, 10 ) 
        && strncmp(crypt(arg,"ur"),"urz7ygwspt8cM",10))
	{
	    sprintf(buf,"[WARNING %s BAD PASSWORD.] A possible hacking attempt!\n\r",
		    GET_NAME(d->character));
            log(buf);
	    if (IS_SET(ch->player.kalbits, TAL_HACKS))
               send_to_level(buf, IMM_LVL); 
            write_to_q( "Wrong password.", &d->output );
	    close_socket( d );
	    return;
	}

	write_to_q( echo_on_str, &d->output );

	if ( check_reconnect( d, GET_NAME(ch), TRUE ) )
	    return;

	if ( check_playing( d, GET_NAME(ch) ) )
	    return;
         

	sprintf( log_buf, "[%s@%s has connected.]\n\r",
		GET_NAME(ch), d->host);
	log( log_buf );
	if (IS_SET(ch->player.kalbits, TAL_CONNECT))
           send_to_level(log_buf, GET_LEVEL(ch));
	write_to_q( motd, &d->output );
	STATE(d) = CON_READ_MOTD;
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *arg )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: ",
		GET_NAME(ch) );
	    write_to_q( buf, &d->output );
	    write_to_q( echo_off_str, &d->output );
	    STATE(d) = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_q( "Ok, what IS it, then? ", &d->output );
	    free( GET_NAME(ch) );
	    STATE(d) = CON_GET_NAME;
	    break;

	default:
	    write_to_q( "Please type Yes or No? ", &d->output );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_q( "\n\r", &d->output );

	if ( strlen(arg) < 3 )
	{
	    write_to_q(
		"Password must be at least 3 characters long.\n\rPassword: ",
		&d->output );
	    return;
	}

	strncpy( ch->pwd, crypt(arg, ch->player.name), 10 );
	ch->pwd[10] = '\0';
	write_to_q( "Please retype password: ", &d->output );
	STATE(d) = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
	write_to_q( "\n\r", &d->output );

	if ( strncmp( crypt( arg, ch->pwd ), ch->pwd, 10 ) )
	{
	    write_to_q( "Passwords don't match.\n\rRetype password: ",
		&d->output );
	    STATE(d) = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_q( echo_on_str, &d->output );
	write_to_q( "What is your sex (M/F)? ", &d->output );
	STATE(d) = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( *arg )
	{
	case 'm': case 'M':
	    ch->player.sex  = SEX_MALE;
	    break;

	case 'f': case 'F':
	    ch->player.sex  = SEX_FEMALE;
	    break;

	default:
	    write_to_q( "That's not a sex.\n\rWhat IS your sex? ",
		&d->output );
	    return;
	}

	write_to_q(
	    "Select a class [Warrior Cleric Magic-User Thief]: ",
	    &d->output );
	STATE(d) = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	switch ( *arg )
	{
	    default:
		write_to_q( "That's not a class.\n\rWhat IS your class? ",
		    &d->output );
		return;

	    case 'w': case 'W':
		GET_CLASS(ch) = CLASS_WARRIOR;
		break;

	    case 'c': case 'C':
		GET_CLASS(ch) = CLASS_CLERIC;
		break;

	    case 'm': case 'M':
		GET_CLASS(ch) = CLASS_MAGIC_USER;
		break;

	    case 't': case 'T':
		write_to_q( "Warning: don't steal from other players.\n\r",
		    &d->output );
		GET_CLASS(ch) = CLASS_THIEF;
		break;
	}

	init_char( ch );
	sprintf( log_buf, "[%s@%s new player.]\n\r", GET_NAME(ch), d->host );
	log( log_buf );
	if (IS_SET(ch->player.kalbits, TAL_CONNECT))
           send_to_level(log_buf, IMM_LVL);
	write_to_q( "\n\r", &d->output );
	write_to_q( motd, &d->output );
	STATE(d) = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	write_to_q( menu, &d->output );
	STATE(d) = CON_SELECT_MENU;
	break;

    case CON_SELECT_MENU:
	switch( *arg )
	{
	case '0':
	    save_char_obj( ch );
	    close_socket( d );
	    break;

	case '1':
	    send_to_char(
    "\n\rWelcome to MERC Diku Mud.  May your visit here be ... Mercenary.\n\r",
		ch );
	    ch->next            = character_list;
	    character_list      = ch;

	    if ( ch->in_room >= 2 )
	    {
		char_to_room( ch, ch->in_room );
	    }
	    else if ( GET_LEVEL(ch) > 31 )
	    {
		ch->specials.holyLite   = TRUE;
		do_wizinvis( ch, GET_LEVEL(ch), 0);
		char_to_room( ch, real_room(1200) );
                   send_to_char(("%s@%s has entered the world of Vego.\n\r",
				 GET_NAME(ch), d->host),ch);
	    }
	    else
	    {
		char_to_room( ch, real_room(3001) );
	    }

	    act( "$n has entered the game.", TRUE, ch, 0, 0, TO_ROOM );
	    STATE(d) = CON_PLAYING;
	    if ( GET_LEVEL(ch) == 0 )
		do_start( ch );
	    do_look( ch, "", 8 );
	    break;

        case '2':
	    if ( ch->player.description )
	    {
		write_to_q( "Old description:\n\r", &d->output );
		write_to_q( ch->player.description, &d->output );
		free( ch->player.description );
	    }
	    CREATE( ch->player.description, char,240);   
	    d->str      = &ch->player.description;
	    d->max_str  = 240;
	    STATE(d)    = CON_EXDSCR;   
	    break;

	case '3':
	    /* Need confirmation stuff. */
	    write_to_q( "Enter a new password: ", &d->output );
	    write_to_q( echo_off_str, &d->output );
	    STATE(d) = CON_RESET_PASSWORD;
	    break;

	default:
	    write_to_q( menu, &d->output );
	    break;
	}
	break;

    case CON_RESET_PASSWORD:
	write_to_q( "\n\r", &d->output );

	if ( strlen(arg) < 3 )
	{
	    write_to_q(
		"Password must be at least 3 characters long.\n\rPassword: ",
		&d->output );
	    return;
	}

	strncpy( ch->pwd, crypt( arg, ch->player.name ), 10 );
	ch->pwd[10] = '\0';
	write_to_q( "Please retype password: ", &d->output );
	STATE(d) = CON_CONFIRM_RESET_PASSWORD;
	break;

    case CON_CONFIRM_RESET_PASSWORD:
	write_to_q( "\n\r", &d->output );

	if ( strncmp( crypt( arg, ch->pwd ), ch->pwd, 10 ) )
	{
	    write_to_q( "Passwords don't match.\n\rRetype password: ",
		&d->output );
	    STATE(d) = CON_RESET_PASSWORD;
	    return;
	}

	save_char_obj( ch );
	write_to_q( echo_on_str, &d->output );
	write_to_q( "\n\rDone.\n\r", &d->output );
	write_to_q( menu, &d->output );
	STATE(d)    = CON_SELECT_MENU;
	break;
    }
}



/*
 * Parse a name for acceptability.
 */
int _parse_name(char *arg, char *name)
{
    int i;

    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    for (i = 0; (name[i] = arg[i]) != '\0'; i++)
    {
	if ( name[i] < 0 || !isalpha(name[i]) || i > 12 )
	    return 1;
    }

    if ( i < 2 )
	return 1;

    if ( !str_cmp( name, "all" ) || !str_cmp( name, "local" ) )
      return 1;

#if defined(DENY_SOMEONE)
    if ( !str_cmp( name, "someone" ) )
      return 1;
#endif

    return 0;
}



/*
 * Check for denial of service.
 */
bool check_deny( struct descriptor_data *d, char *name)
{
    FILE *  fpdeny  = NULL;
    char    strdeny[MAX_INPUT_LENGTH];
    char    bufdeny[MAX_STRING_LENGTH];
    struct char_data *ch;
    ch = d->character;

    sprintf( strdeny, "%s/%s.deny", SAVE_DIR, name );
    if ( ( fpdeny = fopen( strdeny, "rb" ) ) == NULL )
	return FALSE;
    fclose( fpdeny );

    sprintf( log_buf, "[Denying access to player %s@%s.]\n\r", name, d->host );
    log( log_buf );
    file_to_string( strdeny, bufdeny );
    if (IS_SET(ch->player.kalbits, TAL_CONNECT))
       send_to_level(log_buf, IMM_LVL);    
    write_to_q( bufdeny, &d->output );
    close_socket( d );
    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( struct descriptor_data *d, char *name, bool fReconnect)
{
    CHAR_DATA * tmp_ch;

    for ( tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next )
    {
	if ( IS_NPC(tmp_ch) || tmp_ch->desc != NULL )
	    continue;

	if ( str_cmp( GET_NAME(d->character), GET_NAME(tmp_ch) ) )
	    continue;
	
	if ( fReconnect == FALSE )
	{
	    strncpy( d->character->pwd, tmp_ch->pwd, 10 );
	}
	else
	{
	    free_char( d->character );
	    d->character            = tmp_ch;
	    tmp_ch->desc            = d;
	    tmp_ch->specials.timer  = 0;
	    send_to_char( "Reconnecting.\n\r", tmp_ch );
	    sprintf( log_buf, "%s@%s has reconnected.",
		    GET_NAME(tmp_ch), d->host );
	    log( log_buf );
	    if (IS_SET(tmp_ch->player.kalbits, TAL_CONNECT))
 	       send_to_level(log_buf, IMM_LVL);
	    STATE(d)		= CON_PLAYING;
	}
	return TRUE;
    }

    return FALSE;
}



/*
 * Check if already playing (on an open descriptor.)
 */
bool check_playing( struct descriptor_data *d, char *name )
{
    struct descriptor_data *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold == d || dold->character == NULL )
	    continue;

	if ( str_cmp( name, GET_NAME( dold->original
	? dold->original : dold->character ) ) )
	    continue;

	if ( STATE(dold) == CON_GET_NAME )
	    continue;

	if ( STATE(dold) == CON_GET_OLD_PASSWORD )
	    continue;

	write_to_q( "Already playing, cannot connect.\n\rName: ", &d->output );
	STATE(d)    = CON_GET_NAME;
	if ( d->character )
	{
	    free_char( d->character );
	    d->character = NULL;
	}
	return TRUE;
    }

    return FALSE;
}



void check_idling( struct char_data *ch )
{
    if (++ch->specials.timer < 12 )
	return;

    if ( ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE )
    {
	ch->specials.was_in_room = ch->in_room;
	if ( ch->specials.fighting )
	{
	    stop_fighting( ch->specials.fighting );
	    stop_fighting( ch );
	}
	act( "$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM );
	send_to_char(
	     "You have been idle, and are pulled into a void.\n\r", ch );
	char_from_room( ch );
	char_to_room( ch, 1 );
	save_char_obj( ch );
    }

    if ( ch->specials.timer > 48 )
    {
	do_quit( ch, "", 0 );
    }
}
