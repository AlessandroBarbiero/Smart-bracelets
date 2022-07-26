# Smart-bracelets

## Description

In this work, we designed, implemented and tested a software prototype for smart bracelets. These braceletes are worn by a child and her/his parent to keep track of the child's position and trigger alerts when a child goes too far or falls. 

## Prerequisities

* A Virtual Machine. We used [Oracle's VirtualBox](https://www.virtualbox.org/)
* To have installed in the VM, [Contiki](http://www.contiki-os.org/) Operating System
* To have installed in the VM [Cooja](https://github.com/contiki-os/contiki/wiki/An-Introduction-to-Cooja) Network Simulator
* To have installed in the VM [Node-RED](https://nodered.org/) Low-Code, Event-Driven Programming Tool 

## How to use

* The software can be used as-is, however is possible to modify the behavior. You can edit the ".c" and ".h" files depending on your goal
* We created just two parents and two childs, but if you want to simulate a more complex system you just need to create more "parent[i].c" and "child[i].c" files. These files should be identical, with just a different key hardcoded.

* To simulate the system you just have to open Cooja and follow these steps
* * Create a new simulation
* * Add as many as sky-motes per type you want. We used two childs and two parents.
* * Start the simulation and see how the behavior changes as you change motes postion.

## How it operates

* **Pairing phase**: at startup, the parent’s bracelet and the child’s bracelet broadcast a 20-char random key used to uniquely couple the two devices. The same random key is pre-loaded at production time on the two devices: upon reception of a random key, a device checks whetherthe received random key is equal to the stored one; if yes, it stores theaddress of the source device in memory. Then, a special message is transmitted (in unicast) to the source device to stop the pairing phase and move to the next step

* **Operation mode**: in this phase, the parent’s bracelet listen for messages on the radio and accepts only messages coming from the child’s bracelet. The child’s bracelet periodically transmits INFO messages (one message every 10 seconds), containing the position (X, Y) of the child and an estimate of his/her kinematic status (STANDING, WALKING, RUNNING, FALLING)

* **Alert Mode** : upon reception of an INFO message, the parent’s bracelet reads the content of the message. If the kinematic status is FALLING, the bracelet sends a FALL alarm, reporting the position (X, Y) of the children. If the parent’s bracelet does not receive any message, after one minute from the last received message, a MISSING alarm is sent reporting the last position received.

### Requirements Implementation

* The X, Y coordinates are random numbers
* The kinematic status is randomly selected according to the following probability distribution 
* * P(STANDING) = P(WALKING) = P(RUNNING) = 0.3
* * P(FALLING) = 0.1

## Contacts

* Alessandri Roberto: roberto.alessandri@mail.polimi.it
* Barbiero Alessandro: alessandro.barbiero@mail.polimi.it
