# Koder

Koder is a code editor built using Scintilla editing component.

## Scintilla for Haiku

Haiku port of Scintilla is not upstreamed yet. You can find it [here](http://bitbucket.org/KapiX/scintilla).

In order to build the editor, you need to create *Scintilla* directory and copy Scintilla's files (*scintilla.a*, *ScintillaView.h*, *Scintilla.h*, *SciLexer.h*) there.
Then you can invoke `make` and application should build.