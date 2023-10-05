Zadání na cvičení (postupně):
------------------
- seznámení s problémem obchodního cestujícího
- implementace sekvenčního a paralelního řešení


Interní poznámky:
-----------------
- v několika cvičeních řešit problém obchodního cestujícího (TSP)
- 1. cvičení: představení problému, vytvoření (načtení) dat, implementace ohodnocení jedné cesty v grafu
- 2. cvičení: generování všech možných cest v grafu (permutace se zafixovaným prvním prvkem), ohodnocení každé cesty, výběr nejkratší. Paralelizace ve více vláknech. [ILUSTRUJE TRIVIÁLNÍ PARALELNÍ ÚLOHU, MINIMÁLNÍ KOMUNIKACE]
- 3. cvičení: jednoduchá verze algoritmu branmch and bound, paralelizace [ILUSTRUJE PARALELNÍ ÚLOHU S KOMUNIKACÍ]

- (na prvním se toho možná stihne víc)
- (pozor na dimenzionalitu, prakticky pujde řešit problémy s n = 5, 6 určitě ne 16 atd.)

Vstupy:
-------
- ulysses22.tsp.txt - soubor ve formátu TSPLIB (BTW, jde o Odyseovy cesty)
- ulysses22.opt.tour.txt - optimální řešení
- tsp_step1.cpp  - příklad načtení souboru ve formátuy TSPLIB
