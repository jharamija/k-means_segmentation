# Segmentacija slike metodom k-srednjih vrijednosti

## Opis projekta
Ovaj projekt predstavlja implementaciju metode k-srednjih vrijednosti (K-means) za segmentaciju slika u programskom jeziku C. Segmentacija slika je proces podjele slike na više regija, kako bi se olakšala analiza i obrada slike. Metoda k-srednjih vrijednosti je popularna tehnika za grupiranje podataka, koja se također može primijeniti na segmentaciju slika.

## Kako koristiti projekt
- odaberite želite li sliku segmentirati u RGB ili HSV prostoru boja, za RGB koristite datoteku: k_means.c, a za HSV: HSV_k_means.c
- na vrhu odabrane datoteke definirajte konstante:
  - _FILENAME_ -> ime datoteke koja će se segmentirati (mora biti u .ppm formatu)
  - _NEW_IMG_ -> ime datoteke u koju će se spremiti segmentirana slika
  - _MIN_K_, _MAX_K_ -> minimalna i maksimalna vrijednost parametra _k_ ukoliko koristimo metodu lakta (treba ju doraditi, preporučam manualni odabir _k_)
  - _FIRST_CENTR_ -> odabir prvog centroida; treba upisati redni broj piksela za koji želimo da bude odabran kao prvi centroid, ukoliko je -1, prvi centroid biti će nasumično odabran
  - _K_ -> odabir parametra k; ukoliko je manji od 1, k će biti nasumično odabran
- slike koje se koriste moraju biti u _.ppm_ formatu, a najlakše je ako su locirane u istoj mapi kao i .c datoteke, ukoliko su locirane negdje drugdje, u .c datoteci je potrebno upisati put do .ppm datoteke

## Izvođenje koda
### Na Windowsu
- ukoliko nemate, instalirajte C kompajler
- otvorite commant prompt i navigirajte u direktorij gdje se nalazi C kod pomoću naredbi _cd_
- kompajlirajte kod naredbom: `gcc k_means.c -o k_means.exe`
- pokrenite izvršnu datoteku naredbom: `k_means.exe`

### Na Linuxu
- otvorite commant prompt i navigirajte u direktorij gdje se nalazi C kod pomoću naredbi _cd_
- kompajlirajte kod koristeći GCC kompajler naredbom: `gcc k_means.c -o k_means`
- nakon kompajliranja, program se može pokrenuti naredbom: `./moj_program`
