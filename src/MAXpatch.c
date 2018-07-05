 /* Declaration added for connection number limit.  See nanny() below */
 /* You may have to add extern def for int number_playing() (in comm.c),
    depending on the stupidity of your compiler. */
 #define MAX_PLAYING 3
  
 	/* This fragment added by Mike Widner (Valere/Atropos) to limit
 	   number of active connections to MAX_PLAYING.  Done for Vego
 	   which has a bad habit of crashing with over 50 connections.
 	   The structure below is verbose, but the compiler will crunch it.
 	   NOTE:  I let anybody ABOVE level 31 connect anyway.
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

