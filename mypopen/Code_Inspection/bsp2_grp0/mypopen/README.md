# mypopen

Schreiben Sie eine kleine Library mit den zwei Funktionen mypopen() und mypclose(), die sich wie popen(3) bzw. pclose(3) verhalten (siehe Skriptum oder Online-Manual). Implementieren Sie diese beiden Funktionen in einer eigenen Übersetzungseinheit namens mypopen.c und exportieren Sie alle nötigen Informationen (z.B., die extern Deklarationen) in einem zugehörigen Headerfile namens mypopen.h.

Mit Hilfe der Funktionen mypopen() und mypclose() können Sie relativ einfach ein Shell-Kommando ausführen und das Ergebnis direkt in ein Programm einlesen und weiterverarbeiten bzw. Daten, aus einem Programm heraus, an dieses Kommando übergeben.

popen(command, "r") liefert einen Filepointer auf eine pipe, von der die Ausgabe des Befehls command gelesen werden kann. popen(command, "w") liefert einen Filepointer auf eine pipe, auf die die Eingabe für den Befehl command geschrieben werden kann.
