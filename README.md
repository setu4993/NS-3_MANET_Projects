# NS-3 MANET Projects

All of the programs written for implementation of various Mobile Ad-Hoc Networks in Network Simulator-3 for ECE 69500 Mobile Wireless Networking at IUPUI (Fall 2016).

- **Project 1:** This project focused on understanding how NS-3 works and basic Wi-Fi and LAN networks, and how they communicate among one another.

 1. *third_edited.cc:* Modifications were to be made to the *third.cc* example file of NS-3 to change the number of clients and how many packets each one of them sends to the server. Additional command line variables like simulation time and output PCAP file name were added.
 2. *thrpt_rtt.cpp:* This was used to read and analyze the PCAP files generated to measure the throughput and round trip time. The variations were plotted against the number of Wi-Fi client stations to observe the effect of increase in number of Wi-Fi stations on throughput and round trip time of each UDP Echo packet.

- **Project 2:** This project was to understand the working of AODV as a protocol, and how the network reacts to varying transmission ranges of each packet.

 1. *manet-28.cc:* This file is edited from the AODV example source file for a network of 100 nodes, whose X-Y positions are determined by the *manet100.csv* file. The packet delivery ratio and throughput are measured by using the ns3::FlowMonitor class.
 2. *number_of_neighbors.cpp:* This file is used to calculate the mean neighbors for each Ad-Hoc Wi-Fi node.

- **Project 3:** This project focuses on comparing the performances of various Ad-Hoc Routing Protocols like Ad Hoc On-Demand Distance Vector (AODV), Destination-Sequenced Distance-Vector (DSDV), Optimized Link State Routing (OLSR) and Dynamic Source Routing (DSR) based on their throughput performance.

 1. *manet-28.cc:* This file is the same as the one from Project 2.
 2. *dsdv-manet-08.cc:* Here, we implement the DSDV protocol on the same network topology as defined in *manet100.csv*, with the same parameters as used for the *manet-28.cc*.
 3. *olsr-08.cc:* Here, we implement the OLSR protocol on the same network topology as defined in *manet100.csv*, with the same parameters as used for the *manet-28.cc*.
 4. *dsr-08.cc:* Here, we implement the DSR protocol on the same network topology as defined in *manet100.csv*, with the same parameters as used for the *manet-28.cc*.
 5. *compare.cc:* At the end, we use the *manet-routing-compare.cc* from the examples and modify it to our requirement. We use it to verify the results obtained.

- **Project 4:** This project focuses on comparing AODV, DSDV and OLSR in an entirely different topology that consists of one LAN server, connected to a Gateway, which relays messages to and from the 100 clients distributed over a space. These mobile nodes communicate via Ad-Hoc Wi-Fi protocols. Comparison between point-to-point communication to the clients from the server, and broadcast communication from the server to all the clients is also done. (File names for this project are self-explanatory.)

- **Project 5:**
 This project focuses on combining Greedy Perimeter Stateless Routing (GPSR) and the Bird Flocking Algorithm (BFA) to create a new bio-inspired routing algorithm that performs better than conventional routing algorithms. The routing algorithm deployed is tested for various network topologies and the performance of AODV, simple GPSR and GPSR with BFA are compared based on the throughput, delay and jitter. The GPSR implementation for NS-3 was obtained from http://www.mehic.info/2016/04/greedy-perimeter-stateless-routing-gpsr-in-ns3-25/.

 The edits made to the GPSR files included changes made to route perimeter nodes through both left and right most nodes alternately, instead of GPSR's right-most node selection, the time between 'HELLO' packets, the addition of the mobility model (*ns3::RandomDirection2dMobilityModel*) that would provide the direction and velocity of the nodes, and update the neighbor nodes each time either of them changed, and the addition of a energy module to simulate a real-life application.

 The network topologies compared were simulated from real world probably Wireless Sensor Network topologies. The topologies tested were:
 1. *...-sq-corner:* The server is in the left top corner, with nodes distributed in a square.
 2. *...-sq-center:* The server is in the center, with nodes distributed in a square.
 3. *...-rect-center:* The server is in the center, with nodes distributed in a rectangle.
 4. *...-rect-void:* The nodes are distributed in a rectangle with a void in the center. The server placed at the center of the void.
 5. *...-rect-void-2-servers:* The nodes are distributed in a rectangle with a void in the center. Two servers are placed at the center of the edges of the void.
