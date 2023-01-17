# AFLoRa (v0.9)

AFLoRa  (Advanced Framework for LoRa) is a simulation framework for carrying out end-to-end simulations for LoRa networks, including advanced characteristics, such as confirmed transmission, and propose 3 new SF management methods in order to avoid SF12 Well situation. 

We are happy to share the AFLoRa source code. Please consider citing the following paper as the main reference for AFLoRa:

L. Casals,  C. Gomez, R. Vidal, "The SF12 Well in LoRaWAN: Problem and End-Device-Based Solutions", 21(19):6478, Sensors, September 2021.
https://doi.org/10.3390/s21196478

AFLoRa is based on FLoRa (Framework for LoRa), version 0.8, a simulation framework for LoRa networks (http://flora.aalto.fi/), which, in torn, is based on the [OMNeT++](https://omnetpp.org/) network simulator and uses components from the [INET framework](https://inet.omnetpp.org/) as well.

AFloRa improve the original FloRa, implementing more LoRaWAN features (confirmed transmissions, Duty-cycle control), includes transmission queues, new Application-MAC relationship, and correct several FLoRa problems or mistakes. 


Originally, FLoRa allows the creation of LoRa networks with modules for LoRa nodes, gateway(s) and a network server. 
Application logic can be deployed as independent modules that are connected with the network server. 
The network server and nodes support dynamic management of configuration parameters through Adaptive Data Rate (ADR). 
Finally, the energy consumption statistics are collected in every node.
More information about FloRa: [flora.aalto.fi](http://flora.aalto.fi/)
