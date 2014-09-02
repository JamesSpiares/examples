examples/nameserver
===================

An example of a nameserver/server/client combination.
It provides and basic mathematics library as a network service.


On Linux:
To compile the code, type "make server client name" and the three executables will be compiled for you.
To run the name server, type "./name address1 address2 ..." with the addresses of the math library servers.
To run the math library server, type "./server switch" where "switch" is "TCP" or "UDP".
To run the math library client, type "./client switch" where "switch" is "TCP" or "UDP".

If you wish to list more than 4 math library servers on your name server, change the number after "LISTSIZE"
to the number you want, save, and make.