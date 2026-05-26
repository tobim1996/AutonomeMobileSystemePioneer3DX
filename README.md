# 🤖 Autonome Mobile Systeme 
# Projekt „Aufmerksamer Roboter“

## Vorwort

Diese Dokumentation beschreibt die Projektabschlussarbeit im Modul **Autonome Mobile Systeme**.  
Ziel des Projekts ist die Entwicklung eines intelligenten Robotersystems, das ein definiertes Objekt im Kamerabild erkennt und dieses zuverlässig im Zentrum des Sichtfeldes hält.

Dabei kombiniert das System Methoden der **Bildverarbeitung (OpenCV)** mit der **Robotersteuerung (Aria Framework)**.  
Der Roboter soll nicht aktiv auf das Objekt zufahren, sondern dieses lediglich beobachten und seine Ausrichtung entsprechend anpassen.

Im Mittelpunkt steht die Umsetzung eines stabilen Zusammenspiels aus Softwarearchitektur, Echtzeitverarbeitung und Roboterbewegung.

## 📚 Inhaltsverzeichnis

1. Analyse der Aufgabenstellung  
2. Anforderungen & Randbedingungen  
3. Diskussion von Lösungsvarianten  
4. Konzeption und Lösungsideen  
5. Implementierung  
6. Einschätzung der Lösung & aufgetretene Probleme  
7. Zusammenfassung  
8. Vollständig kommentierter Quelltext  
9. Literaturverzeichnis  


## 1. Analyse der Aufgabenstellung

Es soll ein Software-System entwickelt werden, bei dem der Roboter ein Objekt, z. B. einen roten Ball, zuverlässig im Bildzentrum hält.  
Dies erfolgt durch Steuerung der Kamera sowie Rotation des Roboters.

Der Roboter soll dem Objekt **aufmerksam folgen**, ohne aktiv darauf zuzufahren. Stattdessen bleibt er stationär und richtet sich dynamisch nach dem Objekt aus.

Das Ziel ist es, das Objekt dauerhaft im Zentrum des Kamerabildes zu halten.


## 2. Anforderungen & Randbedingungen

- Teamgröße: 2 Mitglieder  
- Nutzung der Roboter aus dem KI-Labor  
- Programmiersprache: **C++**  
- IDE: Microsoft Visual Studio  
- Bildverarbeitung: **OpenCV**  
- Robotersteuerung: **Aria Framework**  

Weitere Anforderungen:
- Ständige informative Ausgaben im System  
- Weiche und langsame Bewegungen  
- Keine hektischen Bewegungen  
- Eigene individuelle Lösung pro Team  


## 3. Diskussion von Lösungsvarianten

### Variante 1: Schachbrettmuster
- Objekt: Schachbrett  
- Problem: Sehr rechenintensiv  
- Ergebnis: nur ca. 1 FPS  
- Fazit: Nicht geeignet für Echtzeitsteuerung  

### Variante 2: Farbiger Ball
- Objekt: einfacher farbiger Ball  
- Vorteil: deutlich bessere Performance  
- Ergebnis: ca. 6× höhere Bildrate  
- Fazit: Beste Lösung für schwache Hardware  


## 4. Konzeption und Lösungsideen

Das System besteht aus zwei Hauptkomponenten:

- Bewegungssteuerung (Aria)
- Bildverarbeitung (OpenCV)

### Bewegungsstrategie:
- Roboter übernimmt horizontale Rotation  
- Kamera übernimmt vertikale Bewegung (Tilt)  
- Geschwindigkeit abhängig von Abstand zum Bildzentrum  

### Bildverarbeitung:
- Farbraumkonvertierung (HSV)
- Thresholding (inRange)
- Morphologische Operationen
- Formenerkennung
- Positionsberechnung


## 5. Implementierung

Grundlage: ActionBV-Beispielprojekt

### Umsetzung:
- Anpassung von `ActionBV.cpp`
- Anpassung von `Thread.cpp`
- Kombination aus Kamera- und Robotersteuerung

### Steuerung:
- Objekt im Zentrum → keine Bewegung  
- Objekt außerhalb → Kamera oder Roboter reagieren  
- Geschwindigkeit abhängig von Distanz zum Zentrum  

### Bildverarbeitung:
- HSV-Konvertierung  
- Filterung über inRange()  
- Noise-Reduktion (Blur + Morphologie)  
- HoughCircles zur Objekterkennung  
- Größter erkannter Kreis wird verwendet  


## 6. Einschätzung der Lösung & Probleme

### Probleme:
- Automatische Helligkeitsanpassung der Kamera  
- Schlechte Performance bei starken Lichtquellen  
- Nur ca. 3 FPS erreichbar  
- Synchronisationsprobleme (100–1000 ms Delay)  

### Auswirkungen:
- Verzögerte Reaktion des Roboters  
- Teilweise nicht zeitgerechte Steuerung  


## 7. Zusammenfassung

Das Projekt „Aufmerksamer Roboter“ implementiert ein System, das ein Objekt im Sichtfeld verfolgt und im Zentrum hält.

Der Roboter reagiert aufmerksam auf Bewegungen, ohne aktiv zu verfolgen oder zu beschleunigen.  
Die Kombination aus Bildverarbeitung und Bewegungssteuerung ermöglicht ein stabiles Tracking-System.

Trotz Performanceproblemen wurde das Projekt erfolgreich umgesetzt.


## 8. Vollständig kommentierter Quelltext

Die vollständigen Quelltexte befinden sich im Projektverzeichnis (Anhang).


## 9. Literaturverzeichnis

- Aufgabenblatt des Projekts  
- Aria Dokumentation (lokal im Labor)  
- Moodle Materialien: https://moodle.th-brandenburg.de/  
- OpenCV Dokumentation: https://docs.opencv.org/4.0.1/index.html  
- Redemittel wissenschaftliches Schreiben:  
  https://webcache.googleusercontent.com/search?q=cache:NdhkTD66bGAJ:https://www.ph-freiburg.de/fileadmin/dateien/zentral/schreibzentrum/typo3content/Lehre_SS13/Redemittel_f%25C3%2583_r_schriftliche_wissenschaftliche_Texte.pdf
