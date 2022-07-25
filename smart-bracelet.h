#ifndef PRELOADED_KEY
	#define PRELOADED_KEY "aaaaaaaaaaaaaaaaaaaa"
#endif

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "string.h"
#include "stdbool.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#define DIM_KEY 20

static char key[DIM_KEY] = PRELOADED_KEY;
static bool pairing = true;
static const linkaddr_t* addr;

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

/*------------------------------- BROADCAST --------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	if(strcmp((char *)packetbuf_dataptr(), key) == 0){
		printf("Paired device -> key found, Address: %d.%d\n", from->u8[0], from->u8[1]);
		pairing = false;
		linkaddr_copy(addr, from);
	//	addr = from;
	}
	else
		printf("This is not my key\n");
		
}
/* In this way I set the callback function on receive to the one just written above */
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


//------------------------------ UNICAST --------------------------------------
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  	//if(strcmp((char *)packetbuf_dataptr(), "Stop Pairing") == 0){
		printf("Paired device -> unicast message received, Address: %d.%d\n", from->u8[0], from->u8[1]);
		pairing = false;
		linkaddr_copy(addr, from);
	//}
}

/*---------------------------------------------------------------------------*/

static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{

	printf("Unicast sent\n");
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
 // if(linkaddr_cmp(dest, &linkaddr_null)) {
 //   return;
 // }
  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
    dest->u8[0], dest->u8[1], status, num_tx);
}

/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/




static void sendPairingMessage(){
    
    packetbuf_copyfrom(key, 21);
    broadcast_send(&broadcast);
    printf("Broadcast message sent: %s\n", key);
	
}

static void operationMode(){
	while(1){
	;
		//printf("work\n");
	}
}

/*--------------------------- PARENT ------------------------------------------------*/

PROCESS_THREAD(Parent_bracelet_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();
  
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	
  	unicast_open(&uc, 146, &unicast_callbacks);
	printf("Open unicast connection\n");
  	
	while(pairing){
		/* Delay 2 seconds */
    	etimer_set(&et, CLOCK_SECOND * 2);

    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		sendPairingMessage();
	}
	broadcast_close(&broadcast);
	printf("Closed broadcast connection\n");
	
	packetbuf_copyfrom(key, 21);
	printf("Copy message in buffer \n");
	unicast_send(&uc, addr);
	printf("Message unicast sent\n");
	
	PROCESS_WAIT_EVENT();
	printf("Start op mode\n");
	operationMode();

  PROCESS_END();
}

//---------------------------- CHILD -----------------------------------------------

PROCESS_THREAD(Child_bracelet_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();
  
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	
  	unicast_open(&uc, 146, &unicast_callbacks);
	printf("Open unicast connection\n");
  	
	while(pairing){
		/* Delay 2 seconds */
    	etimer_set(&et, CLOCK_SECOND * 2);

    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		sendPairingMessage();
	}
	broadcast_close(&broadcast);
	printf("Closed broadcast connection\n");
	
	packetbuf_copyfrom(key, 21);
	printf("Copy message in buffer \n");
	unicast_send(&uc, addr);
	printf("Message unicast sent\n");
	
	PROCESS_WAIT_EVENT();
	printf("Start op mode\n");
	operationMode();

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
