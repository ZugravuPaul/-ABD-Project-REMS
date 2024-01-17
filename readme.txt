https://jigsaw.w3.org/HTTP/
http://www.testingmcafeesites.com/
http://testphp.vulnweb.com/login.php

===========extract redirects js
var urls = document.getElementsByTagName('a');

for (url in urls) {
    console.log ( urls[url].href );
}
===========

Mozzila: 
about:preferences
Manual proxy config:  localhost:9097                   

Analiza trafic:
https://www.tcpdump.org/pcap.html
libpcap 

Http proxy:
https://github.com/vmsandeeprao/HTTP-proxy-server/blob/master/csapp.h

=====ProjectPlan=====

- testare folosind firefox  
Cerințe:  
  - Modificare Headere: Adăugare/Ștergere/Modificare header existent - Header de autentificarte de ex.  
  - Modificare conținut HTTP: rescriere link-uri, rescriere conțiut pe bază de reguli, blocare imagini  
              - Să ai un endpoint de tipul https://my-intercept.ro/  
 - Modificările:  
         - Automat pe baza unui fișier de reguli  
         - Interactiv  
- O listă de domenii/IP-uri și să le prindă doar pe alea  
- Grupare după sesiune HTTP (cookie based)  
- Suport pentru HTTPS*  
