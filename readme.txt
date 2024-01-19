!!! Ultima versiune: proxy 

Link-uri testare:
http://www.testingmcafeesites.com/
http://testphp.vulnweb.com/login.php

Mozzila: 
about:preferences
Manual proxy config:  localhost:9097                   

Analiza trafic:
https://www.tcpdump.org/pcap.html

=====ProjectPlan=====

- testare folosind firefox  
Cerințe:  
  - Modificare Headere: Adăugare/Ștergere/Modificare header existent 
 - Modificările:  
         - Interactiv  
- O listă de IP-uri pe care sa le blocheze

======Utilizare aplicatie=====
1. Setarea browser-ului sa foloseasca proxy-ul(localhost, port 9097)
2. Deschiderea aplicatiei.
            -proxy-ul asculta pe portul 9097 pentru conexiuni
    Modul listening activ(default):  se incarca o pagina si se inspecteaza traficul 
            -socket-ul pentru care se face bind la adresa clientului va fi transmis catre un thread(ProxyThread ce deriveaza clasa QThread)
            -se citeste din socket request-ul, se cauta adresa hostname-ului tinta
            -se creeaza un target socket care va astepta raspunsul
            -traficul este transmis catre interfata si poate fi inspectat
            -*se pastreaza o conexiune keep-alive pana la deconectarea clientului
    Modul listening oprit:
            -nu se mai trateaza request-ul
    Modul intercepting(listening oprit automat): se incarca o pagina, se afiseaza request-ul in tab-ul Request, se poate modifica continutul, iar prin apasarea butonului next
                                                 se trimite catre target si se intoarce raspunsul
            -thread-ul va intra in starea busy-waiting pana la apasarea butonului Next
    Traficul va fi afisat in fereastra de jos prin selectarea host-ului din tabelul din stanga.
    
Cerinte partial functionale:
-adaugarea unei adrese IP intr-un black list pentru a opri traficul (functia gethostbyname() intoarce o lista de adrese ip a serverului tinta, iar in aplicatie se compara doar cu prima din acea lista)
            
            
 
