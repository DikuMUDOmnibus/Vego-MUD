#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>

#include "structs.h"
#include "mob.h"
#include "utils.h"

extern struct room_data *world;

#define MAX_MSGS 40                 /* Max number of messages.          */
#define SAVE_FILE "boards/board"     /* Name of file for saving messages */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */

extern int atoi(char*);

struct board
{
  struct board *next;
  int          room;
  char         filename[MAX_INPUT_LENGTH];
  int          has_loaded;
  int          has_saved;
  char         *msgs[MAX_MSGS];		/* message */
  char         *head[MAX_MSGS];		/* heading line */
  int          msg_num;
  int          flag;                    /* 1=being written to; 0=safe */
} *brd=0;

struct board * gpointer=0;


void board_write_msg(struct board *,CHAR_DATA *, char *);
int board_display_msg(struct board *brd,CHAR_DATA *ch, char *arg);
int board_remove_msg(struct board *brd,CHAR_DATA *ch, char *arg);
void board_save_board(struct board *brd);
void board_load_board(struct board *brd);
void board_reset_board(struct board *brd);
void error_log();
void board_fix_long_desc(int num, char *headers[MAX_MSGS]);
int board_show_board(struct board *brd,CHAR_DATA *ch, char *arg);


/* I have used cmd number 180-182 as the cmd numbers here. */
/* The commands would be, in order : NOTE <header>         */
/* READ <message number>, REMOVE <message number>          */
/* LOOK AT BOARD should give the long desc of the board    */
/* and that should equal a list of message numbers and     */
/* headers. This is done by calling a function that sets   */
/* the long desc in the board object. This function is     */
/* called when someone does a REMOVE or NOTE command.      */
/* I have named the function board_fix_long_desc(). In the */
/* board_write_msg() function there is a part that should  */
/* be replaced with a call to some dreadful routine used   */
/* by the STRING command to receive player input. It is    */
/* reputed to lurk somewhere within the limits of an evil  */
/* file named act.comm.c...  or is that modify.c?..*/

/* saving the board after the addition of a new messg      */
/* poses a slight problem, since the text isn't actually   */
/* entered in board_write. What I'll do is to let board()  */
/* save the first time a 'look' is issued in the room..    */
/* ugh! that's ugly - gotta think of something better.     */
/* -quinn                                                  */

/* And here is the board...correct me if I'm wrong. */

char *strdup(char *source)
{
   char *new;

   CREATE(new, char, strlen(source)+1);
   return(strcpy(new, source));
}


struct board *create_board(int room,int fix)
{
  struct board *board;
  int a;
  log("creating board");
  board=(struct board *)malloc(sizeof (struct board));
  if(board==0)
    log("cannot allocate board structure.");
  else
    {
    board->next=brd;
    board->room=room;
    board->has_loaded=0;
    board->has_saved=1;
    if(fix)
      {
	sprintf(board->filename,"%s.global",SAVE_FILE);
	gpointer=board;
      }
    else
      sprintf(board->filename,"%s.%d",SAVE_FILE,room);
    log(board->filename);
    for(a=0;a<MAX_MSGS;a++)
      {
      board->head[a]=0;
      board->msgs[a]=0;
      }
    board->msg_num=0;
    board->flag=0;
    brd=board;
    }
  return(board);
}

struct board *get_board(CHAR_DATA *ch, int fix)
{
  struct board *board;
  if(brd==0 || (fix && !gpointer))
    {
      if(fix)
	return(gpointer=create_board(world[ch->in_room].number,fix));
      else
	return(create_board(world[ch->in_room].number,fix));
    }
   
  if(fix) return(gpointer);

  for(board=brd;board!=0;board=board->next)
    if(board->room == world[ch->in_room].number) return(board);
  return(create_board(world[ch->in_room].number,fix));
}

int being_written(struct board *board)
{
  return(board->flag);
}
  
int board(CHAR_DATA *ch, int cmd, char *arg, struct obj_data *object)
{
	struct board *board;
	static int isbusy=0;

	if (isbusy) return(FALSE);
	if (!ch->desc)
		return(FALSE);
 /* By MS or all NPC's will be trapped at the board */

	if(!(board=get_board(ch,FALSE))) return(FALSE);
/* note: I'll let display and remove return 0 if the arg was non-board- */
/* related. Thus, it'll be possible to read other things than the board */
/* while you're in the room. Conceiveably, you could do this for write, */
/* too, but I'm not in the mood for such hacking.                       */

	if (!(board->has_loaded)) board_load_board(board);
	if (!(board->has_saved)) board_save_board(board);

	switch (cmd) {
		case 15:  /* look */
			return(board_show_board(board, ch, arg));
		case 149: /* write */
			isbusy=1;
			board_write_msg(board, ch, arg);
			isbusy=0;
		return 1;
		case 63: /* read */
			return(board_display_msg(board, ch, arg));
		case 66: /* remove */
			return(board_remove_msg(board, ch, arg));
		default:
			return 0;
	}
}

int board_general(CHAR_DATA *ch, int cmd, char *arg, struct obj_data *object)
{
	struct board *board;
	static int isbusy=0;

	if (isbusy) return(FALSE);
	if (!ch->desc)
		return(FALSE);
 /* By MS or all NPC's will be trapped at the board */

	if(!(board=get_board(ch,TRUE))) return(FALSE);
/* note: I'll let display and remove return 0 if the arg was non-board- */
/* related. Thus, it'll be possible to read other things than the board */
/* while you're in the room. Conceiveably, you could do this for write, */
/* too, but I'm not in the mood for such hacking.                       */

	if (!(board->has_loaded)) board_load_board(board);
	if (!(board->has_saved)) board_save_board(board);

	switch (cmd) 
	  {
	  case 15:  /* look */
	    if (being_written(board)) 
	      {
		send_to_char("Someone else is writing... Wait your turn.\n\r", ch);
		return(FALSE);
	      }
	    board_load_board(board);
	    return(board_show_board(board, ch, arg));
	  case 149: /* write */
	    isbusy=1;
	    board_write_msg(board, ch, arg);
	    isbusy=0;
	    return 1;
	  case 63: /* read */
	    if (being_written(board)) 
	      {
		send_to_char("Someone else is writing... Wait your turn.\n\r", ch);
		return(TRUE);
	      }
	    return(board_display_msg(board, ch, arg));
	  case 66: /* remove */
	    if (being_written(board)) 
	      {
		send_to_char("Someone else is writing... Wait your turn.\n\r", ch);
		return(FALSE);
	      }
	    return(board_remove_msg(board, ch, arg));
	  default:
	    return 0;
	}
}


void board_write_msg(struct board *brd, CHAR_DATA *ch, char *arg)
{
	if (being_written(brd)) {
		send_to_char("Someone else is writing... Wait your turn.\n\r", ch);
		return;
	}
	if (brd->msg_num > MAX_MSGS - 1) {
		send_to_char("The board is full already.\n\r", ch);
		return;
	}
	/* skip blanks */
	for(; isspace(*arg); arg++);
	if (!*arg) {
		send_to_char("We must have a headline!\n\r", ch);
		return;
	}
	brd->head[brd->msg_num] = (char *)malloc(strlen(arg) + strlen(GET_NAME(ch)) + 4);
	/* +4 is for a space and '()' around the character name. */
	if (!brd->head[brd->msg_num]) {
		error_log("Malloc for board header failed.\n\r");
		send_to_char("The board is malfunctioning - sorry.\n\r", ch);
		return;
	}
	strcpy(brd->head[brd->msg_num], arg);
	/* Is this clumsy?  - four strcat() in a row I mean..*/
	strcat(brd->head[brd->msg_num], " (");
	strcat(brd->head[brd->msg_num], GET_NAME(ch));
	strcat(brd->head[brd->msg_num], ")");
	brd->msgs[brd->msg_num] = NULL;

	send_to_char("Write your message. Terminate with a @.\n\r\n\r", ch);
	act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

	ch->desc->str = &(brd->msgs[brd->msg_num]);
	ch->desc->max_str = MAX_MESSAGE_LENGTH;
	ch->desc->flag = &(brd->flag);

	brd->msg_num++;
	brd->has_saved=0;
	brd->flag = 1;
}


int board_remove_msg(struct board *brd,CHAR_DATA *ch, char *arg)
{
	int ind, msg;
	char buf[256],number[MAX_INPUT_LENGTH];

	if (being_written(brd)) {
		send_to_char("Someone is writing on it now... Please wait.\n\r", ch);
		return(TRUE);
	}

	one_argument(arg, number);

	if (!*number || !isdigit(*number))
		return(0);
	if (!(msg = atoi(number))) return(0);
	if (!brd->msg_num) {
		send_to_char("The board is empty!\n\r", ch);
		return(1);
	}
	if (msg < 1 || msg > brd->msg_num)
	  {
	  send_to_char("That message exists only in your imagination.\n\r",ch);
	  return(1);
	  }

	if (!strncmp(brd->head[msg-1], "to:", 3))
	  {
	  send_to_char("Private message.\n\r", ch);
	  if(IS_NPC(ch) ||
	     (GET_LEVEL(ch)<33 && !isname(GET_NAME(ch), brd->head[msg-1]+3)))
	    {
	    send_to_char("And it is for someone else!\n\r", ch);
	    return(1);
	    }
	  }
	else if (GET_LEVEL(ch)<31 && !isname(GET_NAME(ch),brd->head[msg-1]) )
	  {
	  send_to_char("Only 10th level characters and higher may\n\r", ch);
	  send_to_char("remove messages which they did not write.\n\r", ch);
	  return(TRUE);
	  }

	ind = msg;
	free(brd->head[--ind]);
	sprintf(buf, "$n just removed message %d.", ind + 1);
	if (brd->msgs[ind])
		free(brd->msgs[ind]);
	for (; ind < brd->msg_num -1; ind++) {
		brd->head[ind] = brd->head[ind + 1];
		brd->msgs[ind] = brd->msgs[ind + 1];
	}
	brd->msg_num--;
	send_to_char("Message removed.\n\r", ch);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	brd->has_saved=0;
	board_save_board(brd);

	return(1);
}

void board_save_board(struct board *brd)
{
	FILE *the_file;		
	int ind, len;
	if (being_written(brd)) return;
	log("saving board");
	if (!brd->msg_num) {
		error_log("No messages to save.\n\r");
		brd->has_saved=1;
		return;
	}
	the_file = fopen(brd->filename, "wb");
	if (!the_file) {
		error_log("Unable to open/create savefile..\n\r");
		return;
	}
	fwrite(&(brd->msg_num), sizeof(int), 1, the_file);
	for (ind = 0; ind < brd->msg_num; ind++) {
		len = strlen(brd->head[ind]) + 1;
		fwrite(&len, sizeof(int), 1, the_file);
		fwrite(brd->head[ind], sizeof(char), len, the_file);
		len = strlen(brd->msgs[ind]) + 1;
		fwrite(&len, sizeof(int), 1, the_file);
		fwrite(brd->msgs[ind], sizeof(char), len, the_file);
	}
	fclose(the_file);
	board_fix_long_desc(brd->msg_num, brd->head);
	brd->has_saved=1;
	return;
}

void board_load_board(struct board *brd)
{
	FILE *the_file;
	int ind, len = 0;
	log("loading board");
	board_reset_board(brd);
	the_file = fopen(brd->filename, "rb");
	if (!the_file) {
	       error_log("Can't open message file. Board will be empty.\n\r",0);
		brd->has_loaded=1;
		return;
	}
	fread(&(brd->msg_num), sizeof(int), 1, the_file);
	
	if (brd->msg_num < 1 || brd->msg_num > MAX_MSGS || feof(the_file)) {
		error_log("Board-message file corrupt or nonexistent.\n\r");
		fclose(the_file);
		brd->msg_num=0;
		brd->has_loaded=1;
		return;
	}
	for (ind = 0; ind < brd->msg_num; ind++) {
		fread(&len, sizeof(int), 1, the_file);
		brd->head[ind] = (char *)malloc(len + 1);
		if (!brd->head[ind]) {
			error_log("Malloc for board header failed.\n\r");
			board_reset_board(brd);
			fclose(the_file);
			brd->has_loaded=1;
			return;
		}
		fread(brd->head[ind], sizeof(char), len, the_file);
		fread(&len, sizeof(int), 1, the_file);
		brd->msgs[ind] = (char *)malloc(len + 1);
		if (!brd->msgs[ind]) {
			error_log("Malloc for board msg failed..\n\r");
			board_reset_board(brd);
			fclose(the_file);
			brd->has_loaded=1;
			return;
		}
		fread(brd->msgs[ind], sizeof(char), len, the_file);
	}
	fclose(the_file);
	board_fix_long_desc(brd->msg_num, brd->head);
	brd->has_loaded=1;
	return;
}

void board_reset_board(struct board *brd)
{
	int ind;
	for (ind = 0; ind < MAX_MSGS; ind++)
	  {
	  if(brd->head[ind]!=0) free(brd->head[ind]);
	  if(brd->msgs[ind]!=0) free(brd->msgs[ind]);
	  }
	brd->msg_num = 0;
	board_fix_long_desc(0, 0);
	return;
}

void error_log(char *str)
{				/* The original error-handling was MUCH */
	fputs("Board : ", stderr); /* more competent than the current but  */
	fputs(str, stderr);	/* I got the advice to cut it out..;)   */
	return;
}

int board_display_msg(struct board *brd,CHAR_DATA *ch, char *arg)
{
	char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
	int msg;
	if (being_written(brd)) {
	  send_to_char("Someone is writing on it... Please wait.\n\r",ch);
	  return(1);
	}
	one_argument(arg, number);
	if (!*number || !isdigit(*number))
		return(0);
	if (!(msg = atoi(number))) return(0);
	if (!brd->msg_num) {
		send_to_char("The board is empty!\n\r", ch);
		return(1);
	}
	if (msg < 1 || msg > brd->msg_num) {
	  send_to_char("That message exists only in your imagination.\n\r",ch);
	  return(1);
	}

	if (!strncmp(brd->head[msg-1], "to:", 3))
	  {
	  send_to_char("Private message.\n\r", ch);
	  if(IS_NPC(ch) ||
	     (GET_LEVEL(ch)<32 && !isname(GET_NAME(ch), brd->head[msg-1]+3)))
	    {
	    send_to_char("And it is for someone else!\n\r", ch);
	    return(1);
	    }
	  }

	sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg,brd->head[msg - 1],
		brd->msgs[msg - 1]);
	page_string(ch->desc, buffer, 1);
	return(1);
}


		
void board_fix_long_desc(int num, char *headers[MAX_MSGS])
{

	/**** Assign the right value to this pointer..how? ****/
	/**** It should point to the bulletin board object ****/
	/**** Then make ob.description point to a malloced ****/
	/**** space containing itoa(msg_num) and all the   ****/
	/**** headers. In the format :
	This is a bulletin board. Usage : READ/REMOVE <message #>, NOTE <header>
	There are 12 messages on the board.
	1	: Re : Whatever and something else too.
	2	: I don't agree with Rainbird.
	3	: Me neither.
	4 	: Groo got hungry again - bug or sabotage?

	Well...something like that..;) 			   ****/
	
	/**** It is always to contain the first line and   ****/
	/**** the second line will vary in how many notes  ****/
	/**** the board has. Then the headers and message  ****/
	/**** numbers will be listed. 			   ****/
	return;
}




int board_show_board(struct board *brd,CHAR_DATA *ch, char *arg)
{
	int i;
	char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

	one_argument(arg, tmp);
	if (!*tmp || !isname(tmp, "board bulletin"))
		return(0);

	act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

	strcpy(buf,
"This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\n\rUse WRITE TO: <name> to write a private message.\n\r");
	if (!brd->msg_num)
		strcat(buf, "The board is empty.\n\r");
	else
	{
		sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
			brd->msg_num);
		for (i = 0; i < brd->msg_num; i++)
			sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1,
				 brd->head[i]);
 	}
	if(being_written(brd)) strcat(buf, "It is currently being written on.\n\r");
	page_string(ch->desc, buf, 1);

	return(1);
}

