# Install / Run

The repo ships a pre-built fat JAR — easiest path is to just run that. The
source is two files and assumes Java 1.8 + Apache PDFBox 3.0.0 + FlatLaf on
the classpath.

## Just run the JAR

Requires a Java Runtime (8 or newer).

```powershell
java -jar repos-folder\pdf-font-extractor\PdfFontExtractor.jar
```

The Swing window opens (~725 × 485). No CLI flags, no config file.

If your default `java` is too old, install [Adoptium Temurin
17](https://adoptium.net/) and use the full path.

## Build from source

There is no build script in the repo (no `pom.xml`, no `build.gradle`). You'd
need to wire it up yourself. Minimal deps:

| Lib          | Version         | License    |
|--------------|-----------------|------------|
| Apache PDFBox| 3.0.0           | Apache 2.0 |
| FlatLaf      | 3.x (any recent)| Apache 2.0 |
| JDK          | 1.8 +           | —          |

Quick `javac` build (assuming jars in `lib/`):

```powershell
$cp = "lib\pdfbox-3.0.0.jar;lib\fontbox-3.0.0.jar;lib\flatlaf-3.4.jar"
javac -cp $cp -d out src\app\*.java
java  -cp "$cp;out" app.Main
```

To rebuild the fat JAR you'd need to merge all the deps into one — easiest
via Maven assembly plugin or `jar -c` after extracting the dep jars.

## Where to put the JAR

Anywhere — it doesn't write config or read environment variables. State
that does persist (last-used directory) is held only inside the running
`SettingsManager.DEFAULT_DIR` field, which resets to `user.home` each
launch.
