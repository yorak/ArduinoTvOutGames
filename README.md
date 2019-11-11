# Arduino pelejä

Videopelit on rakennettu Arduino-alustalle, jolloin siitä tulee tavallaan alkeellinen videopelikonsoli. Osat maksavat Kiinasta tilattuna ja kotiinkuljetettuina alle 3 euroa. Peli käyttää [TVout-kirjastoa](https://github.com/Avamander/arduino-tvout), jolla tuotetaan komposiittivideosignaali ja ääntä. Peli on siis kytkettävissä tavallisimpiin televisoihin (joissa on pyöreä keltainen RCA-pistoke videosisääntulolle).

Alla kolme neljä toteutettua videopeliä.

| Peli | nimi | kuvaus |
| ---- | ---- | ------ |
| [![Video Ti-ti -videopelistä](https://raw.githubusercontent.com/juherask/TitiArduinoPeli/master/images/titi_yt_play.png)](https://www.youtube.com/watch?v=6_DM1g7a03M) | titi | Videopeli jossa ohjaat Ti-ti nallea, joka yrittää pyydystää taivaalta putoavia rusetteja. Peli nopeutuu mitä pidemmälle pelaat, joten ole tarkkana. |
| [![Ruudunkaappaus skorssista](https://raw.githubusercontent.com/juherask/TitiArduinoPeli/master/images/skorssi.png)]() | skorssi | 8-bittinen kaksinpeliversio klassisesta Scorched earth DOS-pelistä. Tuhoutuva maasto, mittarit ja monenlaista muuta hienoa. |

## Rauta

Tarvitset Arduino-alustan ATmega 328P -mikrokontrollerilla ja USB-liittimellä. Käytännössä siis Arduno Unon tai Nanon. Lisäksi:
* 3 kpl 470 ohmin vastusta
* 1 kpl 1 kilohmin vastus
* 2 kpl uros-RCA-liitintä, ruuvi- tai painoterminaaleilla
* 1 kpl potentiometriä (säätövastus)
* 3 kpl painonappeja
* johdonpätkiä
* USB-johto

### Hinta-arvio

(hinnat ebaystä tilaten)

| Nimike | Hinta | Huomioita |
| ------ | ----- | --------- |
| Arduino /w 328p | 2,17 €	|	Mini-USB Nano V3.0 ATmega328P Arduino board |
| Säätövastus | 0,23 € | 10 kpl erässä |
| Vastuksia (470, 1k) |	0,04 € | 300 kpl erässä |
| 2 x RCA littimiä | 0,20 € | 10 kpl erässä |
| johdonpätkiä | 0,24 €  | 40 kpl erässä |
| **yhteensä** | **2,87 €**	 | |


### Kytkentä
Piirrän kytkennästä kuvan joskus jos kerkeän tai viimeistään joku kysyy, mutta sitä odotellessa, laitan alkuperäislähteet omalle kytkennälleni. Niilläkin jo pärjää:
* https://code.google.com/p/arduino-tvout/
* https://noperation.wordpress.com/2012/11/27/arduino-starfield/
* https://www.arduino.cc/en/Tutorial/Potentiometer

Yhteenvetona: 
* Komposiittivideon RCA-liittimeen kolme johtoa:
  * vaippaan GND
  * sisemmästä kaksi: 1 kohm - PIN_D9 ja 470 ohm - PIN_D7
* Audio-RCA-liittimeen kaksi johtoa:
  * vaippaan GND
  * sisemmästä 470 ohm - PIN_D11
* Säätövastukselta järjestyksessä kolme johtoa:
  * 5V
  * 470 ohm - PIN_A2
  * GND

## Softa

Pelien lähdekoodi (n. 100-200 riviä per peli) löytyy tästä säilöstä (eng. *repository*) `games`-kansiosta. Olen yrittänyt käyttää kuvaavia muuttujannimiä ja kommentteja hyvien ohjelmointikäytäntöjen mukaisesti, joten koodista pitäisi saada hyvin selvää. Lähdekoodin kieli tosin on englanti, mutta mitään vaikeaa ammattislangia ei pitäisi olla käytetty.

Joitain huomioita ja opittua:
* Bittikartat ja musiikkidata tulee laittaa PROGMEM-makrolla flash-muistiin, jotta ne eivät turhaan vie SRAM:ia, joka on melkein kokonaisuudessaan ruudulle piirrettävän ruutupuskurin (eng. *framebuffer*) käytössä. Jos tämän unohtaa tehdä, saattaa muisti loppua ja peli toimia odottamattomalla tavalla.
* Oma säätövastukseni ei ollut lineaarinen tai sitten lineaarisuus menetettiin analogisen arvon digitalisoinnissa. Etsin säätövastukselta miltei lineaarisen alueen ja rajoitin hahmon liikkumaan vain säätimen ollessa ko. alueella.
* Piirrettäessä ruudun reunojen lähelle tulee olla tarkkana, ettei TVout-kirjaston piirtokomennot ylikirjoita muistista jotain muuta.
