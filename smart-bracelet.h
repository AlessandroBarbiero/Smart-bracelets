#ifndef PRELOADED_KEY
	#define PRELOADED_KEY "aaaaaaaaaaaaaaaaaaaa"
#endif

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "string.h"
#include "stdbool.h"

#include <stdio.h>
#define DIM_KEY 20

static char key[DIM_KEY] = PRELOADED_KEY;
static bool pairing = true;
static bool unicastReceived = false;
static bool operationMode = false;
static bool parent = false;
static bool messageArrived = false;
static linkaddr_t* addr;

struct position_t{
	int x;
	int y;
};

static struct position_t childPosition;

enum STATUS{
	STANDING,
	WALKING,
	RUNNING,
	FALLING
	};
	
static inline const char *toString(enum STATUS s){
    static const char *strings[] = { "STANDING", "WALKING",	"RUNNING", "FALLING" };

    return strings[s];
}

static struct message_t{
	struct position_t position;
	enum STATUS status;
} message;


PROCESS(key_generation_process, "Key generation");
PROCESS(Parent_bracelet_process, "Parent Bracelet");
PROCESS(Child_bracelet_process, "Child Bracelet");
//--------------------------------- KEY GENERATION ----------------------------------
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
	if(unicastReceived || !pairing)
		return;	
	
	if(strcmp((char *)packetbuf_dataptr(), key) == 0){
		printf("Paired device -> key found, Address: %d.%d\n", from->u8[0], from->u8[1]);
		pairing = false;
		linkaddr_copy(addr, from);
	}
	else
		printf("This (%s) is NOT my key\n", (char *)packetbuf_dataptr());
		
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


//------------------------------ UNICAST --------------------------------------
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{

	if(pairing && strcmp((char *)packetbuf_dataptr(), "Stop Pairing") == 0){
		printf("Paired device -> unicast message received, Address: %d.%d\n", from->u8[0], from->u8[1]);
		pairing = false;
		unicastReceived = true;
		linkaddr_copy(addr, from);
	}
	else if (operationMode && parent){
		if (linkaddr_cmp(from, addr)){
			messageArrived = true;
			printf("A message arrived from the child\n");
			message = *((struct message_t*) packetbuf_dataptr());
			printf("x = %d, y = %d, status = %s\n", message.position.x, message.position.y, toString(message.status));
			childPosition = message.position;
			if(message.status == FALLING)
				printf("ALARM: FALL x = %d, y = %d\n", message.position.x, message.position.y);
			
		}
	}
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/

static void sendStopPairing(){
	if(!unicastReceived){
		packetbuf_copyfrom("Stop Pairing", 13);
		unicast_send(&uc, addr);
		printf("Message unicast sent\n");
	}
}


static void sendPairingMessage(){
    
    packetbuf_copyfrom(key, 21);
    broadcast_send(&broadcast);
    printf("Broadcast message sent: %s\n", key);
	
}

static void sendInfo(){
	
	message.position.x = random_rand();
	message.position.y = random_rand();
	
	int rand = random_rand()%100;
	if (rand < 30)       //  30%
    	message.status = 0;
	else if (rand < 60)  //  30%
    	message.status = 1;
	else if (rand < 90)  //  30%
    	message.status = 2;
	else				 //	 10%
    	message.status = 3;
	
	
	
	packetbuf_copyfrom(&message, sizeof(struct message_t));
    unicast_send(&uc, addr);
    printf("Unicast message sent: x = %d, y = %d, status = %s\n", message.position.x, message.position.y, toString(message.status));
}


/*--------------------------- PARENT ------------------------------------------------*/

PROCESS_THREAD(Parent_bracelet_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();
  	parent = true;
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	
  	unicast_open(&uc, 146, &unicast_callbacks);
	printf("Open unicast connection\n");
  	
	while(pairing){
	
		sendPairingMessage();
		/* Delay 2 seconds */
    	etimer_set(&et, CLOCK_SECOND * 2);

    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		
	}
	broadcast_close(&broadcast);
	printf("Closed broadcast connection\n");
	
	sendStopPairing();
	
	printf("Start op mode\n");
	operationMode = true;
	

	while(1){
		messageArrived = false;
		etimer_set(&et, CLOCK_SECOND * 60);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		if(!messageArrived)
			printf("ALARM: MISSING last position -> x = %d, y = %d\n", childPosition.x, childPosition.y);
    }

  PROCESS_END();
}








//---------------------------- CHILD -----------------------------------------------

PROCESS_THREAD(Child_bracelet_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();
  	parent = false;
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	
  	unicast_open(&uc, 146, &unicast_callbacks);
	printf("Open unicast connection\n");
  	
	while(pairing){
	
		sendPairingMessage();
		/* Delay 2 seconds */
    	etimer_set(&et, CLOCK_SECOND * 2);

    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	}
	broadcast_close(&broadcast);
	printf("Closed broadcast connection\n");
	
	sendStopPairing();
	
	
	printf("Start op mode\n");
	operationMode = true;

	while(1){
		etimer_set(&et, CLOCK_SECOND * 10);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		sendInfo();
	}

  PROCESS_END();
}

