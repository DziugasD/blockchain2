# Programos naudojimo instrukcija

## Įdiegimas

1. Atsisiųskite ir įdiekite [g++](https://gcc.gnu.org/).
2. Atsisiųskite šį projektą iš GitHub arba klonuokite jį naudodami komandą:
    ```sh
    git clone <projekto URL>
    ```
3. Atidarykite terminalą ir eikite į projekto katalogą:


## Kompiliavimas

1. Kompiliuokite projektą naudodami šią komandą:
    ```sh
    g++ -o Blockchain.exe Blockchain.cpp Blockchain.h -std=c++11
    ```

## Paleidimas

1. Paleiskite sukurtą vykdomąjį failą:
    ```sh
    ./Blockchain.exe
    ```

## Naudojimas

1. Programa paleidžiama terminale ir leidžia jums atlikti įvairias operacijas su blokų grandine.
2. Sekite ekrane pateikiamas instrukcijas, kad galėtumėte pridėti naujus blokus, peržiūrėti blokų grandinę ir atlikti kitas operacijas.

## Pavyzdys

1. Paleidus programą, galite pridėti naują bloką įvesdami reikiamus duomenis.
2. Programa automatiškai apskaičiuos bloko maišos reikšmę ir pridės bloką į grandinę.

## Komandos

Paleidus programą, galite naudoti šias komandas:

- `mine`: Iškasa naują bloką su esamomis transakcijomis.
- `mine_all`: Iškasa visus laukiančius blokus su visomis transakcijomis.
- `info`: Parodo informaciją apie blokų grandinę.
- `balances`: Parodo visų vartotojų balansus.
- `utxo`: Parodo visus nepanaudotus transakcijų išvestis (UTXO).
- `new_user <number>`: Sukuria naują vartotoją su nurodytu numeriu.
- `new_transaction <number>`: Sukuria naują transakciją tarp vartotojų su nurodytu numeriu.
- `transaction <transactionID>`: Parodo informaciją apie nurodytą transakciją pagal jos ID.
- `block <blockIdx>`: Parodo informaciją apie nurodytą bloką pagal jo indeksą.
- `exit`: Išeina iš programos.