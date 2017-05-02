* ------------------------------------------------------------------------------
* File:     readme.txt
* Date:     2011-04-29
* Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
* Project:  IPK Projekt 3 README
* ------------------------------------------------------------------------------


Popis RDT protokolu
-------------------

- Sliding window algoritmus, velkost okna: 10

- Ked server ma kazdy paket v okne a client dostal kazdy ACK, posune sa okno.

- Klient nacita data (jedno okno) a hned to posle. Nepoziva ziadne "hello" packety.

- Pozitivny potvrzovani: server posle ACK, ked dostane bezchybny packet.

- Ked client nedostane ACK paketu v danom case (300 ms), posle paket este raz. 
  Teraz uz caka 1.5x viac.

- Ked server zisti, ze dostal packet, ktoreho uz ma, ignoruje ho, ale posle ACK.

- Server na konci prenosu caka 1000 ms: mozno nejake ACK pakety stratili

- Fletcher algoritmus na pocitanie checksumu.

- Kazdy paket ma svoje cislo, pocitane od 1 (sequence number)


Struktura data packetu:
+-----------------------------------+
| sequence number | data | checksum |
+-----------------+------+----------+
| 4 bytes         |      | 4 bytes  |        
+-----------------------------------+

Struktura ACK packetu:
+----------------------------+
| sequence number | checksum |
+-----------------+----------+
| 4 bytes         | 4 bytes  |
+----------------------------+

Struktura last_packet_flag packetu: indikuje, ze aktualne okno je posledne
                                    a posledny index je last_packet_index.
                                    Server posle ako ACK to iste.
+------------------------------+
| last_packet_index | checksum |
+-------------------+----------+
| 1 byte            | checksum |
+------------------------------+

