/*
 *  Thread Queue Handler
 *
 *
 *  COPYRIGHT (c) 1989-2008.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/score/chain.h>
#include <rtems/score/isr.h>
#include <rtems/score/object.h>
#include <rtems/score/states.h>
#include <rtems/score/thread.h>
#include <rtems/score/threadq.h>
#include <rtems/score/tqdata.h>

/*
 *  _Thread_queue_Extract_fifo
 *
 *  This routine removes a specific thread from the specified threadq,
 *  deletes any timeout, and unblocks the thread.
 *
 *  Input parameters:
 *    the_thread_queue - pointer to a threadq header
 *    the_thread       - pointer to the thread to block
 *
 *  Output parameters: NONE
 *
 *  INTERRUPT LATENCY:
 *    EXTRACT_FIFO
 */

void _Thread_queue_Extract_fifo(
  Thread_queue_Control *the_thread_queue __attribute__((unused)),
  Thread_Control       *the_thread
)
{
  ISR_Level level;

  _ISR_Disable( level );

  if ( !_States_Is_waiting_on_thread_queue( the_thread->current_state ) ) {
    _ISR_Enable( level );
    return;
  }

  _Chain_Extract_unprotected( &the_thread->Object.Node );

  the_thread->Wait.queue = NULL;

  if ( !_Watchdog_Is_active( &the_thread->Timer ) ) {
    _ISR_Enable( level );
  } else {
    _Watchdog_Deactivate( &the_thread->Timer );
    _ISR_Enable( level );
    (void) _Watchdog_Remove( &the_thread->Timer );
  }

  _Thread_Unblock( the_thread );

#if defined(RTEMS_MULTIPROCESSING)
  if ( !_Objects_Is_local_id( the_thread->Object.id ) )
    _Thread_MP_Free_proxy( the_thread );
#endif

}
