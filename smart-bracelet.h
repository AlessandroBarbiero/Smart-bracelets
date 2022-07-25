
#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#define DIM_KEY 20

static char key[DIM_KEY];

PROCESS(key_generation_process, "Key generation");
PROCESS(Parent_bracelet_process, "Parent Bracelet");
PROCESS(Child_bracelet_process, "Child Bracelet");
//-------------------------------------------------------------------
PROCESS_THREAD(key_generation_process, ev, data)
{
  PROCESS_BEGIN();

  static int i;
  
	for(i = 0; i<DIM_KEY; i++){
		key[i] = random_rand()%95 + 32;
	}
	
	printf("%s \n", key);

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
/* In this way I set the callback function on receive to the one just written above */
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(Parent_bracelet_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

// Broadcast_call will be our function custom defined to handle reeception o fbroadcast messages
  broadcast_open(&broadcast, 129, &broadcast_call);
  while(1) {
    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    // printf("broadcast message sent\n");
     printf("Number %s \n", key);
  }

  PROCESS_END();
}


PROCESS_THREAD(Child_bracelet_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

// Broadcast_call will be our function custom defined to handle reeception o fbroadcast messages
  broadcast_open(&broadcast, 129, &broadcast_call);
  while(1) {
    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    // printf("broadcast message sent\n");
     printf("Number %s \n", key);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
